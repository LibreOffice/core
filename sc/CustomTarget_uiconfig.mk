# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,sc/uiconfig))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(call gb_CustomTarget_get_target,sc/uiconfig) : \
	$(OUTDIR)/xml/uiconfig/modules/scalc/menubar/menubar.xml

$(OUTDIR)/xml/uiconfig/modules/scalc/menubar/menubar.xml : $(SRCDIR)/sc/uiconfig/scalc/menubar/menubar.xml \
		| $(OUTDIR)/xml/uiconfig/modules/scalc/menubar/.dir
	sed 's/.*.uno:Collaborate.*/            <menu:menuseparator\/>\n            <menu:menuitem menu:id=".uno:Collaborate"\/>/' $< > $@

endif

# vim: set noet sw=4 ts=4:
