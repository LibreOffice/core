# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(BUILD_UNOWINREG),YES)
ifeq ($(COM),MSC)
odk_unowinreg_DIR := $(WORKDIR)/LinkTarget/Library
else
odk_unowinreg_DIR := $(call gb_CustomTarget_get_workdir,odk/unowinreg)
endif
odk_unowinreg_FILE := unowinreg.dll
else # BUILD_UNOWINREG=NO
odk_unowinreg_DIR := $(TARFILE_LOCATION)
odk_unowinreg_FILE := $(UNOWINREG_DLL)
endif

$(eval $(call gb_Package_Package,odk_unowinreg,$(odk_unowinreg_DIR)))

$(eval $(call gb_Package_set_outdir,odk_unowinreg,$(INSTDIR)))

$(eval $(call gb_Package_add_file,odk_unowinreg,$(SDKDIRNAME)/classes/win/unowinreg.dll,$(odk_unowinreg_FILE)))

# vim: set noet sw=4 ts=4:
