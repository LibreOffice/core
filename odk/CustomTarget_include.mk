# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/include))

include $(SRCDIR)/solenv/inc/udkversion.mk

odkcommon_ZIPLIST += include/udkversion.mk
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/include,include/udkversion.mk))
$(odk_WORKDIR)/include/udkversion.mk:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	cp $(BUILDDIR)/config_host/udkversion.mk $@

# vim: set noet sw=4 ts=4:
