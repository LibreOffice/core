# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

$(eval $(call gb_Executable_add_standard_system_libs,soffice_bin))

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

$(OUTDIR)/bin/soffice.bin.manifest : $(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktargetname,soffice_bin))
	$(call gb_Deliver_deliver,$<.manifest,$@)

.PHONY : $(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest
$(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest :
	rm -f $(OUTDIR)/bin/soffice.bin.manifest

endif

$(eval $(call gb_Executable_add_nativeres,soffice_bin,sofficebin/src))

# vim: set ts=4 sw=4 et:
