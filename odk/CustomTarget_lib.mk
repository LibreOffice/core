# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/lib))

define odk_lib
odkcommon_ZIPLIST += lib/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon/lib): $(odk_WORKDIR)/lib/$(1)
$(odk_WORKDIR)/lib/$(1): $(call gb_Library_get_target,$(1))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

ifeq ($(COM),MSC)
odk_LIBLIST := store \
	reg \
	sal \
	salhelper \
	cppu \
	cppuhelper \
	purpenvhelper

$(foreach lib,$(odk_LIBLIST),$(eval $(call odk_lib,$(lib))))
else ifeq ($(OS),LINUX)
odkcommon_ZIPLIST += lib/libsalcpprt.a
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/lib,libsalcpprt.a))
$(odk_WORKDIR)/lib/libsalcpprt.a: $(call gb_StaticLibrary_get_target,salcpprt)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@
endif

# vim: set noet sw=4 ts=4:
