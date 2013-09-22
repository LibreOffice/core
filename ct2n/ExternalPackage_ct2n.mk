# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,ct2n,ConvertTextToNumber))

$(eval $(call gb_ExternalPackage_set_outdir,ct2n,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files_with_dir,ct2n,share/extensions/ConvertTextToNumber,\
    Addons.xcu \
    CT2N/MainCode.xba \
    CT2N/ToClipbord.xba \
    CT2N/Language.xba \
    CT2N/dialog.xlb \
    CT2N/dlgCode.xba \
    CT2N/script.xlb \
    CT2N/dlgCT2N.xdl \
    CT2N/UserSettings.xba \
    META-INF/manifest.xml \
    Office/UI/BaseWindowState.xcu \
    Office/UI/ImpressWindowState.xcu \
    Office/UI/StartModuleWindowState.xcu \
    Office/UI/BasicIDEWindowState.xcu \
    Office/UI/CalcWindowState.xcu \
    Office/UI/WriterWindowState.xcu \
    Office/UI/DrawWindowState.xcu \
    Office/UI/MathWindowState.xcu \
    description.xml \
    icons/image1_26.bmp \
    icons/image1_16.bmp \
    pkg-desc/pkg-description.txt \
    registration/COPYING \
))

# vim: set noet sw=4 ts=4:
