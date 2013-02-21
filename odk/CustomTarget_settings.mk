# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/settings))

odk_SETTINGSLIST := settings.mk \
	std.mk \
	stdtarget.mk \
	$(if $(filter WNT,$(OS)),component.uno.def)

define odk_settings
odkcommon_ZIPLIST += settings/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon/settings): $(odk_WORKDIR)/settings/$(1)
$(odk_WORKDIR)/settings/$(1): $(SRCDIR)/odk/settings/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach setting,$(odk_SETTINGSLIST),$(eval $(call odk_settings,$(setting))))

odkcommon_ZIPLIST += settings/dk.mk
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/settings,dk.mk))
$(odk_WORKDIR)/settings/dk.mk: $(SRCDIR)/odk/pack/copying/dk.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,SED,1)
	tr -d "\015" < $< | sed -e 's/@@RELEASE@@/$(PRODUCTVERSION)/' \
		-e 's/@@BUILDID@@/$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)/' > $@

ifneq ($(OS),WNT)
odkcommon_ZIPLIST += settings/component.uno.map
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/settings,component.uno.map))
$(odk_WORKDIR)/settings/component.uno.map: $(SRCDIR)/solenv/bin/addsym.awk $(SRCDIR)/solenv/src/component.map
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,GEN,1)
	tr -d "\015" < $(SRCDIR)/solenv/src/component.map | \
		$(if $(filter MACOSX,$(OS)),\
		tail -n +3 | head -3 | sed -e 's/.*component/_component/g' \
		-e 's/;[ ]*//',\
		awk -f $<) > $@
	chmod 664 $@
endif

# vim: set noet sw=4 ts=4:
