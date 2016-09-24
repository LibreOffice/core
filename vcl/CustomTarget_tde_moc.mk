# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/unx/kde/fpicker))

$(call gb_CustomTarget_get_target,vcl/unx/kde/fpicker) : \
	$(call gb_CustomTarget_get_workdir,vcl/unx/kde/fpicker)/kdefilepicker.moc.cxx

$(call gb_CustomTarget_get_workdir,vcl/unx/kde/fpicker)/kdefilepicker.moc.cxx :\
		$(SRCDIR)/vcl/unx/kde/fpicker/kdefilepicker.hxx \
		| $(call gb_CustomTarget_get_workdir,vcl/unx/kde/fpicker)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MOC,1)
	cat $< | grep -v 'FILTER_OUT_KDE' | $(MOC) -o $@ -f$<

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
