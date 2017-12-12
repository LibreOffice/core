# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/windows))

$(eval $(call gb_InstallModule_define_if_set,scp2/windows,\
	BUILD_X64 \
))

$(eval $(call gb_InstallModule_add_defs,scp2/windows,\
	$(if $(WINDOWS_SDK_HOME),\
		-DHAVE_WINDOWS_SDK \
	) \
	$(if $(MSM_PATH),\
		-DMSM_PATH \
	) \
	$(if $(VCREDIST_DIR),\
		-DVCREDIST_EXE_NAME="$(VCREDIST_EXE)" \
	) \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/windows,\
    scp2/source/ooo/folder_ooo \
    scp2/source/ooo/vc_redist \
    scp2/source/ooo/windowscustomaction_ooo \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/windows,\
    scp2/source/base/folderitem_base \
    scp2/source/base/registryitem_base \
    scp2/source/calc/folderitem_calc \
    scp2/source/calc/registryitem_calc \
    scp2/source/draw/folderitem_draw \
    scp2/source/draw/registryitem_draw \
    scp2/source/impress/folderitem_impress \
    scp2/source/impress/registryitem_impress \
    scp2/source/math/folderitem_math \
    scp2/source/math/registryitem_math \
    scp2/source/ooo/folderitem_ooo \
    scp2/source/ooo/registryitem_ooo \
    scp2/source/writer/folderitem_writer \
    scp2/source/writer/registryitem_writer \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
