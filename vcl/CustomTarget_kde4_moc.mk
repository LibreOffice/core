# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/unx/kde4))

$(call gb_CustomTarget_get_target,vcl/unx/kde4) : \
	$(call gb_CustomTarget_get_workdir,vcl/unx/kde4)/KDEXLib.moc \
	$(call gb_CustomTarget_get_workdir,vcl/unx/kde4)/KDE4FilePicker.moc \
	$(call gb_CustomTarget_get_workdir,vcl/unx/kde4)/tst_exclude_socket_notifiers.moc

$(call gb_CustomTarget_get_workdir,vcl/unx/kde4)/%.moc : \
		$(SRCDIR)/vcl/unx/kde4/%.hxx \
		| $(call gb_CustomTarget_get_workdir,vcl/unx/kde4)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MOC,1)
	$(MOC4) $< -o $@

# vim: set noet sw=4:
