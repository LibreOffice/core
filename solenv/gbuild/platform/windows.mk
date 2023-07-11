# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# to block heavy exception handling that try to acquire the solarmutex
export LO_LEAN_EXCEPTION=1

gb_LICENSE := license.txt
gb_README = readme_$(1).txt

gb_Helper_get_rcfile = $(1).ini

gb_Helper_LIBRARY_PATH_VAR := PATH

gb_MKTEMP := mktemp --tmpdir=$(TMPDIR) gbuild.XXXXXX

gb_RUN_CONFIGURE := CONFIG_SHELL=$(shell cygpath -ms /bin/sh)

# define _WIN32_WINNT and WINVER will be derived from it in sdkddkver.h
# current baseline is Windows 7 (NT 6.1)
# for _WIN32_IE, if _WIN32_WINNT >= 0x0600 the derived value from
# sdkddkver.h is sufficient
gb_WIN_VERSION_DEFS := \
	-D_WIN32_WINNT=0x0601 \
	-DWIN32 \
	-DWNT \

gb_OSDEFS := \
	$(gb_WIN_VERSION_DEFS) \
	-DNOMINMAX \
	$(LFS_CFLAGS) \

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \

# empty to avoid --warn-undefined-functions
gb_Library__set_soversion_script_platform =

# default platform and configuration values used by msbuild
gb_MSBUILD_CONFIG := $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)
gb_MSBUILD_PLATFORM := $(strip \
	$(if $(filter INTEL,$(CPUNAME)),Win32) \
	$(if $(filter X86_64,$(CPUNAME)),x64) \
	$(if $(filter AARCH64,$(CPUNAME)),ARM64) \
	)
gb_MSBUILD_CONFIG_AND_PLATFORM := \
	/p:Configuration=$(gb_MSBUILD_CONFIG) \
	/p:Platform=$(gb_MSBUILD_PLATFORM)

gb_CONFIGURE_PLATFORMS := \
	$(if $(and $(filter i686-pc-cygwin,$(HOST_PLATFORM)),$(filter x86_64-pc-cygwin,$(BUILD_PLATFORM))), \
		--build=$(HOST_PLATFORM),--build=$(BUILD_PLATFORM)) --host=$(HOST_PLATFORM)

# vim:set noexpandtab:
