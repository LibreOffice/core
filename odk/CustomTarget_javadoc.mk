# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/docs/java/ref))

odk_JAVAPACKAGES := com.sun.star.comp.helper \
	com.sun.star.lib.uno.helper \
	com.sun.star.lib.unoloader \
	com.sun.star.uno \
	$(SRCDIR)/ridljar/com/sun/star/lib/util/UrlToFileMapper.java

$(eval $(call gb_CustomTarget_register_targets,odk/docs/java/ref,\
	javadoc_log.txt \
))

$(gb_CustomTarget_workdir)/odk/docs/java/ref/package-list : $(gb_CustomTarget_workdir)/odk/docs/java/ref/javadoc_log.txt
	touch $@

$(gb_CustomTarget_workdir)/odk/docs/java/ref/%.css \
$(gb_CustomTarget_workdir)/odk/docs/java/ref/%.gif \
$(gb_CustomTarget_workdir)/odk/docs/java/ref/%.html : \
		$(gb_CustomTarget_workdir)/odk/docs/java/ref/javadoc_log.txt
	touch $@

$(gb_CustomTarget_workdir)/odk/docs/java/ref/javadoc_log.txt : \
		$(call gb_Jar_get_target,libreoffice)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JDC,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),JDC)
	$(call gb_Helper_abbreviate_dirs,\
		$(JAVADOC) -source $(JAVA_SOURCE_VER) -J-Xmx120m -use -splitindex \
		-windowtitle "Java UNO Runtime Reference" \
		-header "$(PRODUCTNAME) $(PRODUCTVERSION) SDK Java API Reference"\
		-tag attention:a:"Attention:" \
		-d $(dir $@) \
		-sourcepath "$(SRCDIR)/ridljar/source/unoloader$(gb_CLASSPATHSEP)$(SRCDIR)/ridljar$(gb_CLASSPATHSEP)$(SRCDIR)/jurt$(gb_CLASSPATHSEP)$(SRCDIR)/javaunohelper" \
		-classpath $(call gb_Jar_get_target,libreoffice) \
		-linkoffline ../../common/reg $(SRCDIR)/odk/docs/java-uno \
		-linkoffline http://java.sun.com/j2se/1.5/docs/api $(SRCDIR)/odk/docs/java \
		$(odk_JAVAPACKAGES) \
		$(if $(JAVADOCISGJDOC),,-notimestamp) \
		> $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),JDC)

# vim: set noet sw=4 ts=4:
