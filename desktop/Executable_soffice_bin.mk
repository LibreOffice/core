# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_bin))

$(eval $(call gb_Executable_set_targettype_gui,soffice_bin,YES))

$(eval $(call gb_Executable_set_include,soffice_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_use_libraries,soffice_bin,\
    sal \
    sofficeapp \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_cobjects,soffice_bin,\
    desktop/source/app/main \
))

$(eval $(call gb_Executable_use_static_libraries,soffice_bin,\
    ooopathutils \
    winextendloaderenv \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
    /STACK:10000000 \
))

# HACK: Copy manifest to $(OUTDIR)/bin under the right name. The
# executable is copied in Package_soffice_bin, but that does not work
# for the manifest, for some reason...
$(call gb_Module_get_target,desktop) : $(OUTDIR)/bin/soffice.bin.manifest

$(OUTDIR)/bin/soffice.bin.manifest : $(call gb_Executable_get_target,soffice_bin)
$(call gb_Executable_get_clean_target,soffice_bin) : $(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest

$(OUTDIR)/bin/soffice.bin.manifest : $(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktarget,soffice_bin))
	$(call gb_Deliver_deliver,$<.manifest,$@)

.PHONY : $(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest
$(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest :
	rm -f $(OUTDIR)/bin/soffice.bin.manifest

endif

$(eval $(call gb_Executable_add_nativeres,soffice_bin,sofficebin/officeloader))

# vim: set ts=4 sw=4 et:
