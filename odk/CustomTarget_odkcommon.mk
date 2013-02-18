# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

odk_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/odkcommon)

UDK_MAJOR := 3
UDK_MINOR := 2
UDK_MICRO := 7

odkcommon_ZIPLIST :=

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon))

# Executables

odk_EXELIST := cppumaker \
	regcompare \
	idlc \
	javamaker \
	autodoc \
	unoapploader \
	uno-skeletonmaker \
	$(if $(SYSTEM_UCPP),,ucpp) \
	$(if $(filter WNT,$(OS)),climaker)

define odk_exe
odkcommon_ZIPLIST += bin/$(1)$(gb_Executable_EXT)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/bin/$(1)$(gb_Executable_EXT)
$(call gb_CustomTarget_get_workdir,odk/odkcommon)/bin/$(1)$(gb_Executable_EXT): $(call gb_Executable_get_target,$(1))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach exe,$(odk_EXELIST),$(eval $(call odk_exe,$(exe))))

ifeq ($(OS),MACOSX)
odkcommon_ZIPLIST += bin/addsym-macosx.sh
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,bin/addsym-macosx.sh))
$(odk_WORKDIR)/bin/addsym-macosx.sh: $(SRCDIR)/odk/pack/copying/addsym-macosx.sh
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@
	chmod 755 $@
endif

# CLI assmemblies

ifeq ($(COM),MSC)
odk_CLILIST := cli_basetypes.dll \
		cli_uretypes.dll \
		cli_oootypes.dll \
		cli_ure.dll \
		cli_cppuhelper.dll

define odk_cli
odkcommon_ZIPLIST += cli/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/cli/$(1)
$(odk_WORKDIR)/cli/$(1): $(call gb_CliAssembly_get_target,$(1))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach cli,$(odk_CLILIST),$(eval $(call odk_cli,$(cli))))
endif

# Libraries

define odk_lib
odkcommon_ZIPLIST += lib/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/lib/$(1)
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
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,lib/libsalcpprt.a))
$(odk_WORKDIR)/lib/libsalcpprt.a: $(call gb_StaticLibrary_get_target,salcpprt)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@
endif

# Settings
odk_SETTINGSLIST := settings.mk \
	std.mk \
	stdtarget.mk \
	$(if $(filter WNT,$(OS)),component.uno.def)

define odk_settings
odkcommon_ZIPLIST += settings/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/settings/$(1)
$(odk_WORKDIR)/settings/$(1): $(SRCDIR)/odk/settings/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach setting,$(odk_SETTINGSLIST),$(eval $(call odk_settings,$(setting))))

odkcommon_ZIPLIST += settings/dk.mk
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,settings/dk.mk))
$(odk_WORKDIR)/settings/dk.mk: $(SRCDIR)/odk/pack/copying/dk.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,SED,1)
	tr -d "\015" < $< | sed -e 's/@@RELEASE@@/$(PRODUCTVERSION)/' \
		-e 's/@@BUILDID@@/$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)/' > $@

ifneq ($(OS),WNT)
odkcommon_ZIPLIST += settings/component.uno.map
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,settings/component.uno.map))
$(odk_WORKDIR)/settings/component.uno.map: $(SRCDIR)/solenv/bin/addsym.awk $(SRCDIR)/solenv/src/component.map
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,GEN,1)
	tr -d "\015" < $(SRCDIR)/solenv/src/component.map | \
		$(if $(filter MACOSX,$(OS)),\
		tail -n +3 | head -3 | sed -e 's/.*component/_component/g' \
		-e '/s/;[ ]*//',\
		awk -f $<) > $@
	chmod 664 $@
endif

# Java stuff
ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_JavaClassSet_JavaClassSet,loader))

$(eval $(call gb_JavaClassSet_add_sourcefiles,loader,\
	odk/source/com/sun/star/lib/loader/Loader \
	odk/source/com/sun/star/lib/loader/InstallationFinder \
	odk/source/com/sun/star/lib/loader/WinRegKey \
	odk/source/com/sun/star/lib/loader/WinRegKeyException \
))

odk_CLASSESLIST := com/sun/star/lib/loader/Loader.class \
	com/sun/star/lib/loader/InstallationFinder.class \
	com/sun/star/lib/loader/WinRegKey.class \
	com/sun/star/lib/loader/WinRegKeyException.class \

define odk_class
odkcommon_ZIPLIST += classes/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/classes/$(1)
$(odk_WORKDIR)/classes/$(1): $(call gb_JavaClassSet_get_target,loader)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $(call gb_JavaClassSet_get_classdir,loader)/$(1) $$@
endef

$(foreach class,$(odk_CLASSESLIST),$(eval $(call odk_class,$(class))))

odkcommon_ZIPLIST += classes/win/unowinreg.dll
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,classes/win/unowinreg.dll))
$(odk_WORKDIR)/classes/win/unowinreg.dll: $(call gb_CustomTarget_get_workdir,odk/unowinreg)/unowinreg.dll
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@

endif

# Configuration Scripts
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

# Documentation
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

# Autodoc
odkcommon_ZIPLIST += docs/common/ref/module-ix.html
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,docs/common/ref/module-ix.html))
$(odk_WORKDIR)/docs/common/ref/module-ix.html: $(SRCDIR)/odk/pack/copying/idl_chapter_refs.txt \
	$(SRCDIR)/odk/docs/common/ref/idl.css $(call gb_UnoApi_get_target,offapi) \
	$(call gb_Executable_get_target,autodoc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AUD,1)
	$(call gb_Executable_get_target,autodoc) -html $(dir $@) \
		-dvgroot "http://wiki.services.openoffice.org/wiki" \
		-name "LibreOffice $(PRODUCTVERSION) API" \
		-lg idl -dvgfile $< -t $(OUTDIR)/idl


define odk_doc
odkcommon_ZIPLIST += $(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/$(1)
$(odk_WORKDIR)/$(1): $(call gb_CustomTarget_get_workdir,odk/gendocu)/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

# Documentation
ifneq ($(SOLAR_JAVA),)
$(odk_WORKDIR)/docs.done: $(call gb_CustomTarget_get_workdir,odk/gendocu)/docs/java/ref/index.html
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/docs.done
endif

ifneq ($(DOXYGEN),)
$(odk_WORKDIR)/docs.done: $(call gb_CustomTarget_get_workdir,odk/gendocu)/docs/cpp/ref/index.html
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/docs.done
endif

$(odk_WORKDIR)/docs.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cd $(call gb_CustomTarget_get_workdir,odk/gendocu) && \
		find docs -type f -exec cp --parents {} $(odk_WORKDIR) \;
	touch $@

# IDL files
# FIXME: should be pulled in from offapi/udkapi
odk_IDLLIST := $(subst $(OUTDIR)/idl/,,$(shell find $(OUTDIR)/idl/com -type f))

define odk_idl
odkcommon_ZIPLIST += idl/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/idl/$(1)
$(odk_WORKDIR)/idl/$(1): $(OUTDIR)/idl/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach idl,$(odk_IDLLIST),$(eval $(call odk_idl,$(idl))))

# Include files
odk_INCDIRLIST := sal salhelper rtl osl store typelib uno cppu cppuhelper \
	registry $(if $(filter WNT,$(OS)),systools)
odk_INCFILELIST := com/sun/star/uno/Any.h \
	com/sun/star/uno/Any.hxx \
	com/sun/star/uno/genfunc.h \
	com/sun/star/uno/genfunc.hxx \
	com/sun/star/uno/Reference.h \
	com/sun/star/uno/Reference.hxx \
	com/sun/star/uno/Sequence.h \
	com/sun/star/uno/Sequence.hxx \
	com/sun/star/uno/Type.h \
	com/sun/star/uno/Type.hxx
odk_INCLIST := $(subst $(OUTDIR)/inc/,,$(shell find \
	$(foreach dir,$(odk_INCDIRLIST),$(OUTDIR)/inc/$(dir)) -type f)) \
	$(odk_INCFILELIST)

define odk_inc
odkcommon_ZIPLIST += include/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/include/$(1)
$(odk_WORKDIR)/include/$(1): $(foreach dir,$(odk_INCDIRLIST),$(call gb_Package_get_target,$(dir)_inc))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach inc,$(odk_INCLIST),$(eval $(call odk_inc,$(inc))))

odkcommon_ZIPLIST += include/udkversion.mk
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon,include/udkversion.mk))
$(odk_WORKDIR)/include/udkversion.mk:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo "#UDK version number" > $@
	echo "#major" >>$@
	echo "UDK_MAJOR=$(UDK_MAJOR)" >> $@
	echo "#minor" >>$@
	echo "UDK_MINOR=$(UDK_MINOR)" >> $@
	echo "#micro" >>$@
	echo "UDK_MICRO=$(UDK_MICRO)" >> $@

# Doxygen
ifneq ($(DOXYGEN),)
odkcommon_CPPDOCU = $(shell find $(call gb_CustomTarget_get_workdir,odk/gendocu)/cpp/ref -type f)

define odk_doc
odkcommon_ZIPLIST += docs/cpp/ref/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/docs/cpp/ref/$(1)
$(odk_WORKDIR)/docs/cpp/ref/$(1): $(call gb_CustomTarget_get_workdir,odk/gendocu)/cpp/ref/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach doc,$(odk_CPPDOCU),$(eval $(call odk_doc,$(doc))))

endif

# Javadoc
ifneq ($(SOLAR_JAVA),)
odkcommon_JAVADOCU = $(shell find $(call gb_CustomTarget_get_workdir,odk/gendocu)/java/ref -type f)

define odk_jdoc
odkcommon_ZIPLIST += docs/java/ref/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon): $(odk_WORKDIR)/docs/java/ref/$(1)
$(odk_WORKDIR)/docs/java/ref/$(1): $(call gb_CustomTarget_get_workdir,odk/gendocu)/java/ref/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach doc,$(odk_JAVADOCU),$(eval $(call odk_jdoc,$(doc))))
endif

# vim: set noet sw=4 ts=4:
