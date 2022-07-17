# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard/report))

extras_WIZARD_REPORT_XMLFILES := \
	cnt-01/mimetype \
	cnt-011/mimetype \
	cnt-012/mimetype \
	cnt-02/mimetype \
	cnt-021/mimetype \
	cnt-022/mimetype \
	cnt-03/mimetype \
	cnt-031/mimetype \
	cnt-032/mimetype \
	cnt-04/mimetype \
	cnt-041/mimetype \
	cnt-042/mimetype \
	cnt-05/mimetype \
	cnt-051/mimetype \
	cnt-052/mimetype \
	cnt-06/mimetype \
	cnt-061/mimetype \
	cnt-062/mimetype \
	cnt-default/mimetype \
	stl-01/mimetype \
	stl-02/mimetype \
	stl-03/mimetype \
	stl-04/mimetype \
	stl-05/mimetype \
	stl-06/mimetype \
	stl-07/mimetype \
	stl-08/mimetype \
	stl-09/mimetype \
	stl-default/mimetype \

# param: style-base (e.g. Modern)
extras_WIZARD_REPORT_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_REPORT_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/% : $(SRCDIR)/extras/source/templates/wizard/report/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*).dir
	$(call gb_Output_announce,templates/wizard/report/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/report/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/report/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/%.xml : $(SRCDIR)/extras/source/templates/wizard/report/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/report/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/report/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/report/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            mimetype $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            content.xml $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) )\
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            meta.xml $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            settings.xml $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            styles.xml $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            META-INF/manifest.xml $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/report)/$$*/,\
            Thumbnails/thumbnail.png $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/wizard/report/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/report/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ content.xml && \
		zip -qrX --must-match $@ meta.xml && \
		zip -qrX --must-match $@ settings.xml && \
		zip -qrX --must-match $@ styles.xml && \
		zip -qrX --must-match $@ META-INF/manifest.xml && \
		zip -qrX --must-match $@ Thumbnails/thumbnail.png && \
		zip -qrX --must-match $@ $(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/report/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
