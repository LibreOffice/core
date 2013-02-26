# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

odk_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/odkcommon)
odkcommon_ZIPLIST :=

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon))

$(call gb_CustomTarget_get_target,odk/odkcommon): \
	$(if $(DOXYGEN),$(call gb_CustomTarget_get_target,odk/odkcommon/docs/cpp/ref)) \
	$(if $(SOLAR_JAVA),$(call gb_CustomTarget_get_target,odk/odkcommon/docs/java/ref)) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/docs/common/ref) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/settings) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/classes) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/bin) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/lib) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/idl) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/include) \
	$(if $(filter MSC,$(COM)),$(call gb_CustomTarget_get_target,odk/odkcommon/cli))

ifeq ($(OS),WNT)
odk_CONFIGLIST := cfgWin.js \
	setsdkenv_windows.template \
	setsdkname.bat
else
odk_CONFIGLIST := configure.pl \
	setsdkenv_unix \
	setsdkenv_unix.sh.in \
	setsdkenv_unix.csh.in
endif

define odk_config
odkcommon_ZIPLIST += $(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/$(1)
$(odk_WORKDIR)/$(1): $(SRCDIR)/odk/config/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach config,$(odk_CONFIGLIST),$(eval $(call odk_config,$(config))))

ifeq ($(OS),WNT)
odkcommon_ZIPLIST += setsdkenv_windows.bat
$(eval $(call gb_CustomTarget_register_targets,odk/odkcommon,\
	setsdkenv_windows.bat \
))
$(odk_WORKDIR)/setsdkenv_windows.bat: $(SRCDIR)/odk/config/setsdkenv_windows.bat
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,SED,1)
	sed $< -e 's#__SDKNAME__#libreoffice$(PRODUCTVERSION)_sdk#' > $@
else
odkcommon_ZIPLIST += config.guess config.sub
$(eval $(call gb_CustomTarget_register_targets,odk/odkcommon,\
	config.guess \
	config.sub \
))
$(odk_WORKDIR)/config.%: $(SRCDIR)/config.%
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@
endif

odk_HTMLLIST := index.html \
	docs/tools.html \
	docs/notsupported.html \
	docs/install.html \
	examples/examples.html \
	examples/DevelopersGuide/examples.html

odk_DOCULIST := docs/sdk_styles.css \
	docs/common/ref/idl.css \
	docs/images/nada.gif \
	docs/images/arrow-1.gif \
	docs/images/arrow-2.gif \
	docs/images/arrow-3.gif \
	docs/images/bluball.gif \
	docs/images/ooo-main-app_32.png \
	docs/images/odk-footer-logo.gif \
	docs/images/bg_table.png \
	docs/images/bg_table2.png \
	docs/images/bg_table3.png \
	docs/images/nav_down.png \
	docs/images/nav_home.png \
	docs/images/nav_left.png \
	docs/images/nav_right.png \
	docs/images/nav_up.png \
	docs/images/sdk_head-1.png \
	docs/images/sdk_head-2.png \
	docs/images/sdk_line-1.gif \
	docs/images/sdk_line-2.gif

define odk_html
odkcommon_ZIPLIST += $(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/$(1)
$(odk_WORKDIR)/$(1): $(SRCDIR)/odk/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,SED,1)
	cat $$< | tr -d "\015" | \
		sed -e 's/%PRODUCT_RELEASE%/$(PRODUCTVERSION)/g' > $$@
endef

define odk_docu
odkcommon_ZIPLIST += $(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/$(1)
$(odk_WORKDIR)/$(1): $(SRCDIR)/odk/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach html,$(odk_HTMLLIST),$(eval $(call odk_html,$(html))))
$(foreach doc,$(odk_DOCULIST),$(eval $(call odk_docu,$(doc))))

# vim: set noet sw=4 ts=4:
