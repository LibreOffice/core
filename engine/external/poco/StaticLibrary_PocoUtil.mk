# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoUtil))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoUtil,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoUtil,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoUtil))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoUtil,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoUtil,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	$$(INCLUDE) \
))

# PocoUtil pulls in XMLConfiguration (PocoXML) and JSONConfiguration (PocoJSON).
# FastLoggerConfigurator (the bundled quill dependency) is disabled.
$(eval $(call gb_StaticLibrary_add_defs,PocoUtil,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
	-DPOCO_NO_FASTLOGGER \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoUtil,\
	UnpackedTarball/poco/Util/src/AbstractConfiguration \
	UnpackedTarball/poco/Util/src/Application \
	UnpackedTarball/poco/Util/src/ConfigurationMapper \
	UnpackedTarball/poco/Util/src/ConfigurationView \
	UnpackedTarball/poco/Util/src/FilesystemConfiguration \
	UnpackedTarball/poco/Util/src/HelpFormatter \
	UnpackedTarball/poco/Util/src/IniFileConfiguration \
	UnpackedTarball/poco/Util/src/IntValidator \
	UnpackedTarball/poco/Util/src/JSONConfiguration \
	UnpackedTarball/poco/Util/src/LayeredConfiguration \
	UnpackedTarball/poco/Util/src/LocalConfigurationView \
	UnpackedTarball/poco/Util/src/LoggingConfigurator \
	UnpackedTarball/poco/Util/src/LoggingSubsystem \
	UnpackedTarball/poco/Util/src/MapConfiguration \
	UnpackedTarball/poco/Util/src/Option \
	UnpackedTarball/poco/Util/src/OptionCallback \
	UnpackedTarball/poco/Util/src/OptionException \
	UnpackedTarball/poco/Util/src/OptionProcessor \
	UnpackedTarball/poco/Util/src/OptionSet \
	UnpackedTarball/poco/Util/src/PropertyFileConfiguration \
	UnpackedTarball/poco/Util/src/RegExpValidator \
	UnpackedTarball/poco/Util/src/ServerApplication \
	UnpackedTarball/poco/Util/src/Subsystem \
	UnpackedTarball/poco/Util/src/SystemConfiguration \
	UnpackedTarball/poco/Util/src/Timer \
	UnpackedTarball/poco/Util/src/TimerTask \
	UnpackedTarball/poco/Util/src/Validator \
	UnpackedTarball/poco/Util/src/XMLConfiguration \
))

# Windows-only: ServerApplication registers a service via WinService, which also
# uses the Windows registry helpers.
ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoUtil,\
	UnpackedTarball/poco/Util/src/WinRegistryConfiguration \
	UnpackedTarball/poco/Util/src/WinRegistryKey \
	UnpackedTarball/poco/Util/src/WinService \
))
endif

# vim: set noet sw=4 ts=4:
