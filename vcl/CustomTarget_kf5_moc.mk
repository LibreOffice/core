# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/unx/kf5))

$(call gb_CustomTarget_get_target,vcl/unx/kf5) : \
	$(gb_CustomTarget_workdir)/vcl/unx/kf5/KFFilePicker.moc

$(gb_CustomTarget_workdir)/vcl/unx/kf5/%.moc : \
		$(SRCDIR)/vcl/unx/kf5/%.hxx \
		| $(gb_CustomTarget_workdir)/vcl/unx/kf5/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MOC,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),MOC)
	$(MOC5) $< -o $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),MOC)

# vim: set noet sw=4:
