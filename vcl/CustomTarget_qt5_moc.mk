# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/qt5))

$(call gb_CustomTarget_get_target,vcl/qt5) : \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5AccessibleWidget.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Clipboard.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5FilePicker.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Frame.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Instance.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5MainWindow.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Menu.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Object.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Timer.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5Widget.moc \
	$(call gb_CustomTarget_get_workdir,vcl/qt5)/Qt5XAccessible.moc \

$(call gb_CustomTarget_get_workdir,vcl/qt5)/%.moc : \
		$(SRCDIR)/vcl/inc/qt5/%.hxx \
		| $(call gb_CustomTarget_get_workdir,vcl/qt5)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MOC,1)
	$(MOC5) $< -o $@

# vim: set noet sw=4:
