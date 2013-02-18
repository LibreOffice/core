# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

docu_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/gendocu)
docu_SRCDIR := $(SRCDIR)/odk/pack/gendocu

$(eval $(call gb_CustomTarget_CustomTarget,odk/gendocu))

# Doxygen
ifneq ($(DOXYGEN),)
$(eval $(call gb_CustomTarget_register_targets,odk/gendocu,\
	docs/cpp/ref/index.html \
))

CPPDOCREFNAME := "UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) C/C++ API Reference"
JAVADOCREFNAME := "UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) Java API Reference"

# Cygwin Doxygen needs unix paths
DOXY_INPUT := $(docu_SRCDIR)/main.dox $(addprefix $(OUTDIR)/inc/,$(odk_INCDIRLIST) $(odk_INCFILELIST))
DOXY_INPUT := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(DOXY_INPUT)),$(DOXY_INPUT))
DOXY_WORKDIR := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(docu_WORKDIR)/docs/cpp/ref),$(docu_WORKDIR)/docs/cpp/ref)
DOXY_STRIP_PATH := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(OUTDIR)/inc),$(OUTDIR)/inc)
DOXY_DEPS := $(docu_SRCDIR)/Doxyfile \
	$(docu_SRCDIR)/main.dox \
	$(call gb_Package_get_target,sal_inc) \
	$(call gb_Package_get_target,salhelper_inc) \
	$(call gb_Package_get_target,cppu_inc) \
	$(call gb_Package_get_target,cppuhelper_inc) \
	$(call gb_Package_get_target,store_inc) \
	$(call gb_Package_get_target,registry_inc)


odkcommon_ZIPLIST += docs/cpp/ref/index.html
$(docu_WORKDIR)/docs/cpp/ref/index.html: $(DOXY_DEPS)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GEN,1)
	sed $< -e 's!^INPUT = %$$!INPUT = $(DOXY_INPUT)!' \
	-e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(DOXY_WORKDIR)!' \
	-e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = $(CPPDOCREFNAME)!' \
        -e 's!^QUIET = %$$!QUIET = $(if $(VERBOSE),YES,NO)!' \
        -e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(DOXY_STRIP_PATH)!' \
	> $(docu_WORKDIR)/Doxyfile
	$(DOXYGEN) $(docu_WORKDIR)/Doxyfile
endif

# Javadoc
ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_CustomTarget_register_targets,odk/gendocu,\
	docs/java/ref/index.html \
))

JAVA_PACKAGES := com.sun.star.comp.helper \
	com.sun.star.lib.uno.helper \
	com.sun.star.lib.unoloader \
	com.sun.star.uno \
	$(docu_WORKDIR)/java_src/com/sun/star/lib/util/UrlToFileMapper.java

$(docu_WORKDIR)/docs/java/ref/index.html: $(call gb_Jar_get_target,ridl)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JDC,1)
	$(JAVADOC) -J-Xmx120m -use -splitindex \
		-windowtitle "Java UNO Runtime Reference" \
		-header $(JAVADOCREFNAME) -d $(dir $@) \
		-sourcepath $(docu_WORKDIR)/java_src \
		-classpath $(OUTDIR)/bin/ridl.jar \
		-linkoffline ../../common/reg ./uno \
		-linkoffline http://java.sun.com/j2se/1.5/docs/api ./java \
		$(JAVA_PACKAGES) \
		$(if $(JAVADOCISGJDOC),,-notimestamp) \
		> $(docu_WORKDIR)/javadoc_log.txt

odk_JAVASRCLIST := juh_src \
	jurt_src \
	ridl_src \
	unoloader_src

define odk_jsrc
$(docu_WORKDIR)/docs/java/ref/index.html: $(docu_WORKDIR)/java_src/$(1).done
$(docu_WORKDIR)/java_src/$(1).done: $(call gb_Zip_get_target,$(1))
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),build,UNZ,1)
	unzip -quo $$< -d $$(dir $$@)
	touch $$@
endef

$(foreach src,$(odk_JAVASRCLIST),$(eval $(call odk_jsrc,$(src))))

endif

# vim: set noet sw=4 ts=4:
