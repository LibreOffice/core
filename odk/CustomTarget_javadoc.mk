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
	$(SRCDIR)/jurt/com/sun/star/lib/util/UrlToFileMapper.java

$(eval $(call gb_CustomTarget_register_targets,odk/docs/java/ref,\
	javadoc_log.txt \
))

$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/package-list : $(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/javadoc_log.txt
	touch $@

$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/%.css \
$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/%.gif \
$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/%.html : \
		$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/javadoc_log.txt
	touch $@

$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)/javadoc_log.txt : $(call gb_Jar_get_outdir_target,ridl)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JDC,1)
	$(JAVADOC) -J-Xmx120m -use -splitindex \
		-windowtitle "Java UNO Runtime Reference" \
		-header "$(PRODUCTNAME) $(PRODUCTVERSION) SDK Java API Reference"\
		-tag attention:a:"Attention:" \
		-d $(dir $@) \
		-sourcepath "$(SRCDIR)/ridljar/source/unoloader$(gb_CLASSPATHSEP)$(SRCDIR)/ridljar$(gb_CLASSPATHSEP)$(SRCDIR)/jurt$(gb_CLASSPATHSEP)$(SRCDIR)/javaunohelper" \
		-classpath $(OUTDIR)/bin/ridl.jar \
		-linkoffline ../../common/reg $(SRCDIR)/odk/docs/java-uno \
		-linkoffline http://java.sun.com/j2se/1.5/docs/api $(SRCDIR)/odk/docs/java \
		$(odk_JAVAPACKAGES) \
		$(if $(JAVADOCISGJDOC),,-notimestamp) \
		> $@

# vim: set noet sw=4 ts=4:
