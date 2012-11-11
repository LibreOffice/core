# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,ct2n,$(call gb_UnpackedTarball_get_dir,ConvertTextToNumber)))

$(eval $(call gb_Zip_use_unpacked,ct2n,ConvertTextToNumber))

$(eval $(call gb_Zip_add_files,ct2n,\
    Addons.xcu \
    CT2N \
    CT2N/MainCode.xba \
    CT2N/ToClipbord.xba \
    CT2N/Language.xba \
    CT2N/dialog.xlb \
    CT2N/dlgCode.xba \
    CT2N/script.xlb \
    CT2N/dlgCT2N.xdl \
    CT2N/UserSettings.xba \
    META-INF \
    META-INF/manifest.xml \
    Office \
    Office/UI \
    Office/UI/BaseWindowState.xcu \
    Office/UI/ImpressWindowState.xcu \
    Office/UI/StartModuleWindowState.xcu \
    Office/UI/BasicIDEWindowState.xcu \
    Office/UI/CalcWindowState.xcu \
    Office/UI/WriterWindowState.xcu \
    Office/UI/DrawWindowState.xcu \
    Office/UI/MathWindowState.xcu \
    description.xml \
    icons \
    icons/image1_26.bmp \
    icons/image1_16.bmp \
    pkg-desc \
    pkg-desc/pkg-description.txt \
    registration \
    registration/COPYING \
))

# UGLY: the Zip target always has .zip suffix, so copy the file to .oxt here...

$(call gb_Zip_get_final_target,ct2n) : $(OUTDIR)/bin/ConvertTextToNumber.oxt
$(call gb_Zip_get_clean_target,ct2n) : Clean_ConvertTextToNumber_oxt

$(OUTDIR)/bin/ConvertTextToNumber.oxt : $(call gb_Zip_get_target,ct2n)
	$(call gb_Deliver_deliver,$<,$@)

.PHONY : Clean_ConvertTextToNumber_oxt
Clean_ConvertTextToNumber_oxt :
	rm -f $(OUTDIR)/bin/ConvertTextToNumber.oxt

# vim: set noet sw=4 ts=4:
