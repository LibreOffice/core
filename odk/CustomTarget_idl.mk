# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/idl))

odkcommon_ZIPLIST += $(subst $(SRCDIR)/udkapi/,idl/,$(shell find $(SRCDIR)/udkapi/com -type f))
odkcommon_ZIPLIST += $(subst $(SRCDIR)/offapi/,idl/,$(shell find $(SRCDIR)/offapi/com -type f))

$(call gb_CustomTarget_get_target,odk/odkcommon/idl) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	mkdir -p $(odk_WORKDIR)/idl
	cp -rf $(SRCDIR)/udkapi/com $(odk_WORKDIR)/idl
	cp -rf $(SRCDIR)/offapi/com $(odk_WORKDIR)/idl
	touch $@

# vim: set noet sw=4 ts=4:
