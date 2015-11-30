# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# to avoid flashing windows during tests
export VCL_HIDE_WINDOWS=1

gb_LICENSE := license.txt
gb_README = readme_$(1).txt

gb_Helper_get_rcfile = $(1).ini

gb_Helper_LIBRARY_PATH_VAR := PATH

gb_MKTEMP := mktemp --tmpdir=$(gb_TMPDIR) gbuild.XXXXXX

# define _WIN32_WINNT and WINVER will be derived from it in sdkddkver.h
# with a 7.1 SDK target Windows XP, with 8.x SDK target Windows Vista
# currently _WIN32_IE is defined to a higher version than would be derived
# in sdkddkver.h from _WIN32_WINNT=0x0502 but if _WIN32_WINNT >= 0x0600
# the derived value is sufficient
gb_OSDEFS := \
	-D_WIN32_WINNT=$(if $(filter 70,$(WINDOWS_SDK_VERSION)),0x0502,0x0600) \
	-D_WIN32_IE=0x0700 \
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

# Prevent warning spamming
# Happens because of the way we link our unit tests with our libraries.
gb_LinkTarget_LDFLAGS += \
	/ignore:4217

# vim:set noexpandtab:
