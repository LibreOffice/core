# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/idl))

# FIXME: should be pulled in from offapi/udkapi
odk_IDLLIST := $(subst $(OUTDIR)/idl/,,$(shell find $(OUTDIR)/idl/com -type f))

define odk_idl
odkcommon_ZIPLIST += idl/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon/idl): $(odk_WORKDIR)/idl/$(1)
$(odk_WORKDIR)/idl/$(1): $(OUTDIR)/idl/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach idl,$(odk_IDLLIST),$(eval $(call odk_idl,$(idl))))

# vim: set noet sw=4 ts=4:
