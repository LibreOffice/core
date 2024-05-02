# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,wizards/share))

$(call gb_CustomTarget_get_target,wizards/share): \
    $(gb_CustomTarget_workdir)/wizards/share/dialog.xlc \
    $(gb_CustomTarget_workdir)/wizards/share/script.xlc

$(gb_CustomTarget_workdir)/wizards/share/%.xlc: \
    $(SRCDIR)/wizards/source/configshare/%.xlc \
    | $(gb_CustomTarget_workdir)/wizards/share/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),SED)
	sed -e 's/@LIBO_SHARE_FOLDER@/$(LIBO_SHARE_FOLDER)/' $< > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),SED)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:


# vim: set noet sw=4 ts=4:
