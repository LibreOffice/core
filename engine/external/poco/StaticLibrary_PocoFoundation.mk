# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoFoundation))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoFoundation,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoFoundation,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoFoundation))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoFoundation,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoFoundation,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	-I$(gb_UnpackedTarball_workdir)/poco/Foundation/src \
	-I$(gb_UnpackedTarball_workdir)/poco/dependencies/zlib/src \
	-I$(gb_UnpackedTarball_workdir)/poco/dependencies/pcre2/src \
	-I$(gb_UnpackedTarball_workdir)/poco/dependencies/utf8proc/src \
	-I$(gb_UnpackedTarball_workdir)/poco/dependencies/v8_double_conversion/src \
	$$(INCLUDE) \
))

# bundled zlib/pcre2/utf8proc are compiled in; double-conversion is included
# textually by NumericString.cpp.  expat/OpenSSL come from the engine, not from
# POCO's bundled copies, so they are not referenced here.  FastLogger (which
# would pull in the bundled quill library) is disabled.
$(eval $(call gb_StaticLibrary_add_defs,PocoFoundation,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
	-DPOCO_NO_FASTLOGGER \
	-DPCRE2_STATIC \
	-DUTF8PROC_STATIC \
	-DHAVE_CONFIG_H \
))

# Android's Bionic libc has no POSIX shared memory (shm_open/shm_unlink), so use
# POCO's dummy SharedMemory implementation there.
ifeq ($(OS),ANDROID)
$(eval $(call gb_StaticLibrary_add_defs,PocoFoundation,\
	-DPOCO_NO_SHAREDMEMORY \
))
endif

# Emscripten has no inotify; DirectoryWatcher includes <sys/inotify.h> unless
# POCO_NO_INOTIFY is set (it then falls back to a polling implementation).
ifeq ($(OS),EMSCRIPTEN)
$(eval $(call gb_StaticLibrary_add_defs,PocoFoundation,\
	-DPOCO_NO_INOTIFY \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoFoundation,\
	UnpackedTarball/poco/Foundation/src/AbstractObserver \
	UnpackedTarball/poco/Foundation/src/ActiveThreadPool \
	UnpackedTarball/poco/Foundation/src/ArchiveStrategy \
	UnpackedTarball/poco/Foundation/src/Ascii \
	UnpackedTarball/poco/Foundation/src/ASCIIEncoding \
	UnpackedTarball/poco/Foundation/src/AsyncChannel \
	UnpackedTarball/poco/Foundation/src/AsyncNotificationCenter \
	UnpackedTarball/poco/Foundation/src/AtomicCounter \
	UnpackedTarball/poco/Foundation/src/Base32Decoder \
	UnpackedTarball/poco/Foundation/src/Base32Encoder \
	UnpackedTarball/poco/Foundation/src/Base64Decoder \
	UnpackedTarball/poco/Foundation/src/Base64Encoder \
	UnpackedTarball/poco/Foundation/src/BinaryReader \
	UnpackedTarball/poco/Foundation/src/BinaryWriter \
	UnpackedTarball/poco/Foundation/src/BufferedBidirectionalStreamBuf \
	UnpackedTarball/poco/Foundation/src/BufferedStreamBuf \
	UnpackedTarball/poco/Foundation/src/Bugcheck \
	UnpackedTarball/poco/Foundation/src/ByteOrder \
	UnpackedTarball/poco/Foundation/src/Channel \
	UnpackedTarball/poco/Foundation/src/Checksum \
	UnpackedTarball/poco/Foundation/src/Clock \
	UnpackedTarball/poco/Foundation/src/Condition \
	UnpackedTarball/poco/Foundation/src/Configurable \
	UnpackedTarball/poco/Foundation/src/ConsoleChannel \
	UnpackedTarball/poco/Foundation/src/CountingStream \
	UnpackedTarball/poco/Foundation/src/DataURIStream \
	UnpackedTarball/poco/Foundation/src/DataURIStreamFactory \
	UnpackedTarball/poco/Foundation/src/DateTime \
	UnpackedTarball/poco/Foundation/src/DateTimeFormat \
	UnpackedTarball/poco/Foundation/src/DateTimeFormatter \
	UnpackedTarball/poco/Foundation/src/DateTimeParser \
	UnpackedTarball/poco/Foundation/src/Debugger \
	UnpackedTarball/poco/Foundation/src/DeflatingStream \
	UnpackedTarball/poco/Foundation/src/DigestEngine \
	UnpackedTarball/poco/Foundation/src/DigestStream \
	UnpackedTarball/poco/Foundation/src/DirectoryIterator \
	UnpackedTarball/poco/Foundation/src/DirectoryIteratorStrategy \
	UnpackedTarball/poco/Foundation/src/DirectoryWatcher \
	UnpackedTarball/poco/Foundation/src/Environment \
	UnpackedTarball/poco/Foundation/src/Error \
	UnpackedTarball/poco/Foundation/src/ErrorHandler \
	UnpackedTarball/poco/Foundation/src/Event \
	UnpackedTarball/poco/Foundation/src/EventArgs \
	UnpackedTarball/poco/Foundation/src/EventChannel \
	UnpackedTarball/poco/Foundation/src/Exception \
	UnpackedTarball/poco/Foundation/src/FIFOBufferStream \
	UnpackedTarball/poco/Foundation/src/File \
	UnpackedTarball/poco/Foundation/src/FileChannel \
	UnpackedTarball/poco/Foundation/src/FileStream \
	UnpackedTarball/poco/Foundation/src/FileStreamFactory \
	UnpackedTarball/poco/Foundation/src/FileStreamRWLock \
	UnpackedTarball/poco/Foundation/src/Format \
	UnpackedTarball/poco/Foundation/src/Formatter \
	UnpackedTarball/poco/Foundation/src/FormattingChannel \
	UnpackedTarball/poco/Foundation/src/FPEnvironment \
	UnpackedTarball/poco/Foundation/src/Glob \
	UnpackedTarball/poco/Foundation/src/Hash \
	UnpackedTarball/poco/Foundation/src/HashStatistic \
	UnpackedTarball/poco/Foundation/src/HexBinaryDecoder \
	UnpackedTarball/poco/Foundation/src/HexBinaryEncoder \
	UnpackedTarball/poco/Foundation/src/InflatingStream \
	UnpackedTarball/poco/Foundation/src/IOLock \
	UnpackedTarball/poco/Foundation/src/JSONFormatter \
	UnpackedTarball/poco/Foundation/src/JSONString \
	UnpackedTarball/poco/Foundation/src/Latin1Encoding \
	UnpackedTarball/poco/Foundation/src/Latin2Encoding \
	UnpackedTarball/poco/Foundation/src/Latin9Encoding \
	UnpackedTarball/poco/Foundation/src/LineEndingConverter \
	UnpackedTarball/poco/Foundation/src/LocalDateTime \
	UnpackedTarball/poco/Foundation/src/LogFile \
	UnpackedTarball/poco/Foundation/src/Logger \
	UnpackedTarball/poco/Foundation/src/LoggingFactory \
	UnpackedTarball/poco/Foundation/src/LoggingRegistry \
	UnpackedTarball/poco/Foundation/src/LogStream \
	UnpackedTarball/poco/Foundation/src/Manifest \
	UnpackedTarball/poco/Foundation/src/MD4Engine \
	UnpackedTarball/poco/Foundation/src/MD5Engine \
	UnpackedTarball/poco/Foundation/src/MemoryPool \
	UnpackedTarball/poco/Foundation/src/MemoryStream \
	UnpackedTarball/poco/Foundation/src/Message \
	UnpackedTarball/poco/Foundation/src/Mutex \
	UnpackedTarball/poco/Foundation/src/NamedEvent \
	UnpackedTarball/poco/Foundation/src/NamedMutex \
	UnpackedTarball/poco/Foundation/src/NestedDiagnosticContext \
	UnpackedTarball/poco/Foundation/src/Notification \
	UnpackedTarball/poco/Foundation/src/NotificationCenter \
	UnpackedTarball/poco/Foundation/src/NotificationQueue \
	UnpackedTarball/poco/Foundation/src/NullChannel \
	UnpackedTarball/poco/Foundation/src/NullStream \
	UnpackedTarball/poco/Foundation/src/NumberFormatter \
	UnpackedTarball/poco/Foundation/src/NumberParser \
	UnpackedTarball/poco/Foundation/src/NumericString \
	UnpackedTarball/poco/Foundation/src/Path \
	UnpackedTarball/poco/Foundation/src/PatternFormatter \
	UnpackedTarball/poco/Foundation/src/PIDFile \
	UnpackedTarball/poco/Foundation/src/Pipe \
	UnpackedTarball/poco/Foundation/src/PipeImpl \
	UnpackedTarball/poco/Foundation/src/PipeStream \
	UnpackedTarball/poco/Foundation/src/PriorityNotificationQueue \
	UnpackedTarball/poco/Foundation/src/Process \
	UnpackedTarball/poco/Foundation/src/ProcessRunner \
	UnpackedTarball/poco/Foundation/src/PurgeStrategy \
	UnpackedTarball/poco/Foundation/src/Random \
	UnpackedTarball/poco/Foundation/src/RandomStream \
	UnpackedTarball/poco/Foundation/src/RefCountedObject \
	UnpackedTarball/poco/Foundation/src/RegularExpression \
	UnpackedTarball/poco/Foundation/src/RotateStrategy \
	UnpackedTarball/poco/Foundation/src/Runnable \
	UnpackedTarball/poco/Foundation/src/RWLock \
	UnpackedTarball/poco/Foundation/src/SHA1Engine \
	UnpackedTarball/poco/Foundation/src/SHA2Engine \
	UnpackedTarball/poco/Foundation/src/Semaphore \
	UnpackedTarball/poco/Foundation/src/SharedLibrary \
	UnpackedTarball/poco/Foundation/src/SharedMemory \
	UnpackedTarball/poco/Foundation/src/SignalHandler \
	UnpackedTarball/poco/Foundation/src/SimpleFileChannel \
	UnpackedTarball/poco/Foundation/src/SortedDirectoryIterator \
	UnpackedTarball/poco/Foundation/src/SplitterChannel \
	UnpackedTarball/poco/Foundation/src/Stopwatch \
	UnpackedTarball/poco/Foundation/src/StreamChannel \
	UnpackedTarball/poco/Foundation/src/StreamConverter \
	UnpackedTarball/poco/Foundation/src/StreamCopier \
	UnpackedTarball/poco/Foundation/src/StreamTokenizer \
	UnpackedTarball/poco/Foundation/src/String \
	UnpackedTarball/poco/Foundation/src/StringTokenizer \
	UnpackedTarball/poco/Foundation/src/SynchronizedObject \
	UnpackedTarball/poco/Foundation/src/Task \
	UnpackedTarball/poco/Foundation/src/TaskManager \
	UnpackedTarball/poco/Foundation/src/TaskNotification \
	UnpackedTarball/poco/Foundation/src/TeeStream \
	UnpackedTarball/poco/Foundation/src/TemporaryFile \
	UnpackedTarball/poco/Foundation/src/TextBufferIterator \
	UnpackedTarball/poco/Foundation/src/TextConverter \
	UnpackedTarball/poco/Foundation/src/TextEncoding \
	UnpackedTarball/poco/Foundation/src/TextIterator \
	UnpackedTarball/poco/Foundation/src/Thread \
	UnpackedTarball/poco/Foundation/src/ThreadLocal \
	UnpackedTarball/poco/Foundation/src/ThreadPool \
	UnpackedTarball/poco/Foundation/src/ThreadTarget \
	UnpackedTarball/poco/Foundation/src/ActiveDispatcher \
	UnpackedTarball/poco/Foundation/src/Timer \
	UnpackedTarball/poco/Foundation/src/Timespan \
	UnpackedTarball/poco/Foundation/src/Timestamp \
	UnpackedTarball/poco/Foundation/src/Timezone \
	UnpackedTarball/poco/Foundation/src/TimedNotificationQueue \
	UnpackedTarball/poco/Foundation/src/Token \
	UnpackedTarball/poco/Foundation/src/ULID \
	UnpackedTarball/poco/Foundation/src/ULIDGenerator \
	UnpackedTarball/poco/Foundation/src/UnbufferedStreamBuf \
	UnpackedTarball/poco/Foundation/src/Unicode \
	UnpackedTarball/poco/Foundation/src/UnicodeConverter \
	UnpackedTarball/poco/Foundation/src/URI \
	UnpackedTarball/poco/Foundation/src/URIStreamFactory \
	UnpackedTarball/poco/Foundation/src/URIStreamOpener \
	UnpackedTarball/poco/Foundation/src/UTF16Encoding \
	UnpackedTarball/poco/Foundation/src/UTF32Encoding \
	UnpackedTarball/poco/Foundation/src/UTF8Encoding \
	UnpackedTarball/poco/Foundation/src/UTF8String \
	UnpackedTarball/poco/Foundation/src/UUID \
	UnpackedTarball/poco/Foundation/src/UUIDGenerator \
	UnpackedTarball/poco/Foundation/src/Var \
	UnpackedTarball/poco/Foundation/src/VarHolder \
	UnpackedTarball/poco/Foundation/src/VarIterator \
	UnpackedTarball/poco/Foundation/src/VarVisitor \
	UnpackedTarball/poco/Foundation/src/Void \
	UnpackedTarball/poco/Foundation/src/Windows1250Encoding \
	UnpackedTarball/poco/Foundation/src/Windows1251Encoding \
	UnpackedTarball/poco/Foundation/src/Windows1252Encoding \
))

# Platform-specific log channels: Windows has no <syslog.h>, and uses the
# Windows console channels instead.  EventLogChannel is intentionally not built
# (it needs a message-compiler-generated pocomsg.h and is unused); LoggingFactory
# is patched to not reference it on Windows.
ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoFoundation,\
	UnpackedTarball/poco/Foundation/src/WindowsConsoleChannel \
))
else
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoFoundation,\
	UnpackedTarball/poco/Foundation/src/SyslogChannel \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,PocoFoundation,\
	UnpackedTarball/poco/dependencies/zlib/src/adler32 \
	UnpackedTarball/poco/dependencies/zlib/src/compress \
	UnpackedTarball/poco/dependencies/zlib/src/crc32 \
	UnpackedTarball/poco/dependencies/zlib/src/deflate \
	UnpackedTarball/poco/dependencies/zlib/src/infback \
	UnpackedTarball/poco/dependencies/zlib/src/inffast \
	UnpackedTarball/poco/dependencies/zlib/src/inflate \
	UnpackedTarball/poco/dependencies/zlib/src/inftrees \
	UnpackedTarball/poco/dependencies/zlib/src/trees \
	UnpackedTarball/poco/dependencies/zlib/src/zutil \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_auto_possess \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_chartables \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_chkdint \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_compile \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_compile_cgroup \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_compile_class \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_config \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_context \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_convert \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_dfa_match \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_error \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_extuni \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_find_bracket \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_jit_compile \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_maketables \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_match \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_match_data \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_match_next \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_newline \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_ord2utf \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_pattern_info \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_script_run \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_serialize \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_string_utils \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_study \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_substitute \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_substring \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_tables \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_ucd \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_valid_utf \
	UnpackedTarball/poco/dependencies/pcre2/src/pcre2_xclass \
	UnpackedTarball/poco/dependencies/utf8proc/src/utf8proc \
))

# vim: set noet sw=4 ts=4:
