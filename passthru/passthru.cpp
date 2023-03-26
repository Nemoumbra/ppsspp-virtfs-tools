#include <cstdio>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <string>
#include <map>
#include <sstream>

#include "../common/virtfs-handler.h"

static LogFunc g_logger;
static void *g_loggerArg;
static std::map<handle_t, std::string> filenames;
static std::stringstream buffer;

// static void LOG(handle_t handle, ErrLevel level, const char *msg)
// {

// 	g_logger(g_loggerArg, handle, level, msg);
// }

static void LOG(handle_t handle) {
	g_logger(g_loggerArg, handle, L_DEBUG, buffer.str().c_str());
	buffer.str(std::string());
}

constexpr const char* FileMove_to_string(FileMove filemove) {
	switch (filemove) {
		case FILEMOVE_BEGIN: {
			return "Begin";
		}
		case FILEMOVE_CURRENT: {
			return "Current";
		}
		case FILEMOVE_END: {
			return "End";
		}
	}
}

#ifdef _MSC_VER
#define open _open
#define close _close
#define lseek _lseek
#define read _read
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#endif

EXPORT bool Init(LogFunc logger, void *loggerArg) {
	g_logger = logger;
	g_loggerArg = loggerArg;

	buffer << "Passthru handler initialized";
	LOG(-1);
	return true;
}

EXPORT void Shutdown() {
	buffer << "Passthru handler shutdown";
	LOG(-1);
}

EXPORT handle_t Open(const char *rootPath, const char *filename) {
	std::string full = std::string(rootPath) + "/" + filename;
	handle_t handle = (handle_t) open(full.c_str(), O_RDONLY | O_BINARY);

	filenames.emplace(handle, full);

	buffer << "Opened " << full << " (handle " << handle << ")";
	LOG(handle);

	return handle;
}

EXPORT void Close(handle_t handle) {
	buffer << "File ";
	auto it = filenames.find(handle);
	if (it != filenames.end()) {
		buffer << it->second;
	}
	else {
		buffer << "with handle " << handle;
	}
	buffer << " closed";

	LOG(handle);
	close((int) handle);
}

EXPORT offset_t Seek(handle_t handle, offset_t offset, FileMove origin) {
	// buffer << "Seek  " << handle << ", offset = " << offset << "\n";

	buffer << "Seek in file ";

	auto it = filenames.find(handle);
	if (it != filenames.end()) {
		buffer << it->second;
	}
	else {
		buffer << "with handle " << handle;
	}

	buffer << " (offset " << std::hex << offset << std::dec << ", seek mode = " << FileMove_to_string(origin) << ")";

	LOG(handle);

	return lseek((int) handle, (long) offset, (int) origin);
}

EXPORT offset_t Read(handle_t handle, void *data, offset_t size) {
	offset_t count = 0;
	int result = 0;
	do {
		count += result;
		result = read((int) handle, (char *)data + count, (unsigned int) (size - count));
	}
	while (result > 0);

	buffer << "Read 0x" << std::hex << size << std::dec << "bytes from file ";

	auto it = filenames.find(handle);
	if (it != filenames.end()) {
		buffer << it->second;
	}
	else {
		buffer << "with handle " << handle;
	}

	LOG(handle);
	return count;
}
