# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# to block heavy exception handling that try to acquire the solarmutex
export LO_LEAN_EXCEPTION=1

# to avoid flashing windows during tests
export VCL_HIDE_WINDOWS=1

gb_LICENSE := license.txt
gb_README = readme_$(1).txt

gb_Helper_get_rcfile = $(1).ini

gb_Helper_LIBRARY_PATH_VAR := PATH

gb_MKTEMP := mktemp --tmpdir=$(gb_TMPDIR) gbuild.XXXXXX

# define _WIN32_WINNT and WINVER will be derived from it in sdkddkver.h
# current baseline is Windows 7 (NT 6.1)
# for _WIN32_IE, if _WIN32_WINNT >= 0x0600 the derived value from
# sdkddkver.h is sufficient
gb_OSDEFS := \
	-D_WIN32_WINNT=0x0601 \
	-DWIN32 \
	-DWNT \
	-DNOMINMAX \


gb_UWINAPI :=

ifeq ($(VCVER),120)

gb_UWINAPI := uwinapi

endif

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \

# empty to avoid --warn-undefined-functions
gb_Library__set_soversion_script_platform =

# vim:set noexpandtab:
