# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/qt5))

#$(call gb_CustomTarget_get_target,vcl/qt5) : \
#	$(call gb_LinkTarget_get_target,vclplug_qt5).rpath
#	$(call gb_LinkTarget__get_workdir_linktargetname,vclplug_qt5)

#$(call gb_CustomTarget_get_workdir,$(INSTROOT)/program/)/%.moc : \
#		$(SRCDIR)/vcl/inc/qt5/%.hxx \
#		| $(call gb_CustomTarget_get_workdir,vcl/qt5)/.dir
#	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),RPATH,1)
#	install_name_tool -add_rpath $(QT5_LIBDIR)

# vim: set noet sw=4:
