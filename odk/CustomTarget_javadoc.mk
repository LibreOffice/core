# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/docs/java/ref))

odkcommon_ZIPLIST += docs/java/ref

JAVADOCREFNAME := "UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) Java API Reference"

odk_JAVAPACKAGES := com.sun.star.comp.helper \
	com.sun.star.lib.uno.helper \
	com.sun.star.lib.unoloader \
	com.sun.star.uno \
	$(odk_WORKDIR)/java_src/com/sun/star/lib/util/UrlToFileMapper.java

$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/docs/java/ref,index.html))

$(odk_WORKDIR)/docs/java/ref/index.html: $(call gb_Jar_get_target,ridl)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JDC,1)
	$(JAVADOC) -J-Xmx120m -use -splitindex \
		-windowtitle "Java UNO Runtime Reference" \
		-header $(JAVADOCREFNAME) -d $(dir $@) \
		-sourcepath $(odk_WORKDIR)/java_src \
		-classpath $(OUTDIR)/bin/ridl.jar \
		-linkoffline ../../common/reg ./uno \
		-linkoffline http://java.sun.com/j2se/1.5/docs/api ./java \
		$(odk_JAVAPACKAGES) \
		$(if $(JAVADOCISGJDOC),,-notimestamp) \
		> $(odk_WORKDIR)/javadoc_log.txt

odk_JAVASRCLIST := juh_src \
	jurt_src \
	ridl_src \
	unoloader_src

define odk_jsrc
$(odk_WORKDIR)/docs/java/ref/index.html: $(odk_WORKDIR)/java_src/$(1).done
$(odk_WORKDIR)/java_src/$(1).done: $(call gb_Zip_get_target,$(1))
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),build,UNZ,1)
	unzip -quo $$< -d $$(dir $$@)
	touch $$@
endef

$(foreach src,$(odk_JAVASRCLIST),$(eval $(call odk_jsrc,$(src))))

# vim: set noet sw=4 ts=4:
