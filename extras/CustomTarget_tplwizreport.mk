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
	stl-01/manifest.rdf \
	stl-02/manifest.rdf \
	stl-03/manifest.rdf \
	stl-04/manifest.rdf \
	stl-05/manifest.rdf \
	stl-06/manifest.rdf \
	stl-07/manifest.rdf \
	stl-08/manifest.rdf \
	stl-09/manifest.rdf \
	stl-default/manifest.rdf \
	stl-01/Pictures/100002000000001400000014E87B6AC5.gif \
	stl-01/Pictures/2000018100001CB100002711BFED37A8.wmf \
	stl-02/Pictures/2000009E0000380100002657BA019D86.wmf \
	stl-03/Pictures/2000024900002711000021F4DCB2DF4A.wmf \
	stl-04/Pictures/10000000000000400000004077CDC8F9.png \
	stl-04/Pictures/10000000000001C6000000E44DCA6C94.jpg \
	stl-05/Pictures/2000036D000027110000145EB7E93049.wmf \
	stl-06/Pictures/100000000000005A000000268DE98CCE.gif \
	stl-06/Pictures/100000000000021B000001E8BE936AB6.gif \
	stl-08/Pictures/100000000000005E0000005E97FB9EF6.jpg \
	stl-09/Pictures/10000000000000050000004D32652675.jpg \
	stl-09/Pictures/100002000000001400000014E87B6AC5.gif \
	stl-09/Pictures/2000025100001D92000017719EB70CFB.wmf \

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
