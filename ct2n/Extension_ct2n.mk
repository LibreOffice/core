# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,ConvertTextToNumber,\
	workdir/$(INPATH_FOR_BUILD)/UnpackedTarball/ConvertTextToNumber))

CT2N_DIR := $(call gb_UnpackedTarball_get_dir,ConvertTextToNumber)

# we have our own LICENSE file; force using it instead of the default one
$(call gb_ExtensionTarget_get_target,ConvertTextToNumber) : \
	LICENSE := $(CT2N_DIR)/registration/LICENSE

$(eval $(call gb_Extension_add_files,ConvertTextToNumber,,\
	$(CT2N_DIR)/Addons.xcu \
))

$(eval $(call gb_Extension_add_files,ConvertTextToNumber,CT2N,\
	$(CT2N_DIR)/CT2N/MainCode.xba \
	$(CT2N_DIR)/CT2N/ToClipbord.xba \
	$(CT2N_DIR)/CT2N/Language.xba \
	$(CT2N_DIR)/CT2N/dialog.xlb \
	$(CT2N_DIR)/CT2N/dlgCode.xba \
	$(CT2N_DIR)/CT2N/script.xlb \
	$(CT2N_DIR)/CT2N/dlgCT2N.xdl \
	$(CT2N_DIR)/CT2N/UserSettings.xba \
))

$(eval $(call gb_Extension_add_files,ConvertTextToNumber,Office/UI,\
    $(CT2N_DIR)/Office/UI/BaseWindowState.xcu \
    $(CT2N_DIR)/Office/UI/ImpressWindowState.xcu \
    $(CT2N_DIR)/Office/UI/StartModuleWindowState.xcu \
    $(CT2N_DIR)/Office/UI/BasicIDEWindowState.xcu \
    $(CT2N_DIR)/Office/UI/CalcWindowState.xcu \
    $(CT2N_DIR)/Office/UI/WriterWindowState.xcu \
    $(CT2N_DIR)/Office/UI/DrawWindowState.xcu \
    $(CT2N_DIR)/Office/UI/MathWindowState.xcu \
))

$(eval $(call gb_Extension_add_files,ConvertTextToNumber,icons,\
    $(CT2N_DIR)/icons/image1_26.bmp \
    $(CT2N_DIR)/icons/image1_16.bmp \
))

$(eval $(call gb_Extension_add_files,ConvertTextToNumber,pkg-desc,\
    $(CT2N_DIR)/pkg-desc/pkg-description.txt \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
