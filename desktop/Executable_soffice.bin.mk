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

ifeq ($(OS),WNT)
sofficebin := soffice_bin
else
sofficebin := soffice.bin
endif

$(eval $(call gb_Executable_Executable,$(sofficebin)))

$(eval $(call gb_Executable_set_targettype_gui,$(sofficebin),YES))

$(eval $(call gb_Executable_set_include,$(sofficebin),\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_add_linked_libs,$(sofficebin),\
    sal \
    sofficeapp \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_cobjects,$(sofficebin),\
    desktop/source/app/main \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_linked_static_libs,$(sofficebin),\
    ooopathutils \
    winextendloaderenv \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,$(sofficebin),\
    /STACK:10000000 \
))

endif

$(call gb_Module_get_target,desktop) : $(OUTDIR)/bin/soffice.bin

$(OUTDIR)/bin/soffice.bin : $(call gb_Executable_get_target,$(sofficebin))
$(call gb_Executable_get_clean_target,$(sofficebin)) : $(WORKDIR)/Clean/OutDir/bin/soffice.bin
$(OUTDIR)/bin/soffice.bin : $(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktargetname,$(sofficebin)))
	$(call gb_Deliver_deliver,$<,$@)

.PHONY : $(WORKDIR)/Clean/OutDir/bin/soffice.bin
$(WORKDIR)/Clean/OutDir/bin/soffice.bin :
	rm -f $(OUTDIR)/bin/soffice.bin

ifeq ($(COM),MSC)
$(call gb_Module_get_target,desktop) : $(OUTDIR)/bin/soffice.bin.manifest

$(OUTDIR)/bin/soffice.bin.manifest : $(call gb_Executable_get_target,$(sofficebin))
$(call gb_Executable_get_clean_target,$(sofficebin)) : $(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest
$(OUTDIR)/bin/soffice.bin.manifest : $(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktargetname,$(sofficebin)))
	$(call gb_Deliver_deliver,$<.manifest,$@)

.PHONY : $(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest
$(WORKDIR)/Clean/OutDir/bin/soffice.bin.manifest :
	rm -f $(OUTDIR)/bin/soffice.bin.manifest

endif

endif

ifeq ($(OS),MACOSX)

$(eval $(call gb_Executable_set_ldflags,\
    $(filter-out -bind_at_load,$$(LDFLAGS)) \
))

endif

$(eval $(call gb_Executable_add_nativeres,$(sofficebin),sofficebin/src))

# vim: set ts=4 sw=4 et:
