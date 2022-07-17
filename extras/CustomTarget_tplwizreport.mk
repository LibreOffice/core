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
	cnt-01/content.xml \
	cnt-01/META-INF/manifest.xml \
	cnt-01/meta.xml \
	cnt-01/settings.xml \
	cnt-01/styles.xml \
	cnt-01/Thumbnails/thumbnail.png \
	cnt-011/content.xml \
	cnt-011/META-INF/manifest.xml \
	cnt-011/meta.xml \
	cnt-011/settings.xml \
	cnt-011/styles.xml \
	cnt-011/Thumbnails/thumbnail.png \
	cnt-012/content.xml \
	cnt-012/META-INF/manifest.xml \
	cnt-012/meta.xml \
	cnt-012/settings.xml \
	cnt-012/styles.xml \
	cnt-012/Thumbnails/thumbnail.png \
	cnt-02/content.xml \
	cnt-02/META-INF/manifest.xml \
	cnt-02/meta.xml \
	cnt-02/settings.xml \
	cnt-02/styles.xml \
	cnt-02/Thumbnails/thumbnail.png \
	cnt-021/content.xml \
	cnt-021/META-INF/manifest.xml \
	cnt-021/meta.xml \
	cnt-021/settings.xml \
	cnt-021/styles.xml \
	cnt-021/Thumbnails/thumbnail.png \
	cnt-022/content.xml \
	cnt-022/META-INF/manifest.xml \
	cnt-022/meta.xml \
	cnt-022/settings.xml \
	cnt-022/styles.xml \
	cnt-022/Thumbnails/thumbnail.png \
	cnt-03/content.xml \
	cnt-03/META-INF/manifest.xml \
	cnt-03/meta.xml \
	cnt-03/settings.xml \
	cnt-03/styles.xml \
	cnt-03/Thumbnails/thumbnail.png \
	cnt-031/content.xml \
	cnt-031/META-INF/manifest.xml \
	cnt-031/meta.xml \
	cnt-031/settings.xml \
	cnt-031/styles.xml \
	cnt-031/Thumbnails/thumbnail.png \
	cnt-032/content.xml \
	cnt-032/META-INF/manifest.xml \
	cnt-032/meta.xml \
	cnt-032/settings.xml \
	cnt-032/styles.xml \
	cnt-032/Thumbnails/thumbnail.png \
	cnt-04/content.xml \
	cnt-04/META-INF/manifest.xml \
	cnt-04/meta.xml \
	cnt-04/settings.xml \
	cnt-04/styles.xml \
	cnt-04/Thumbnails/thumbnail.png \
	cnt-041/content.xml \
	cnt-041/META-INF/manifest.xml \
	cnt-041/meta.xml \
	cnt-041/settings.xml \
	cnt-041/styles.xml \
	cnt-041/Thumbnails/thumbnail.png \
	cnt-042/content.xml \
	cnt-042/META-INF/manifest.xml \
	cnt-042/meta.xml \
	cnt-042/settings.xml \
	cnt-042/styles.xml \
	cnt-042/Thumbnails/thumbnail.png \
	cnt-05/content.xml \
	cnt-05/META-INF/manifest.xml \
	cnt-05/meta.xml \
	cnt-05/settings.xml \
	cnt-05/styles.xml \
	cnt-05/Thumbnails/thumbnail.png \
	cnt-051/content.xml \
	cnt-051/META-INF/manifest.xml \
	cnt-051/meta.xml \
	cnt-051/settings.xml \
	cnt-051/styles.xml \
	cnt-051/Thumbnails/thumbnail.png \
	cnt-052/content.xml \
	cnt-052/META-INF/manifest.xml \
	cnt-052/meta.xml \
	cnt-052/settings.xml \
	cnt-052/styles.xml \
	cnt-052/Thumbnails/thumbnail.png \
	cnt-06/content.xml \
	cnt-06/META-INF/manifest.xml \
	cnt-06/meta.xml \
	cnt-06/settings.xml \
	cnt-06/styles.xml \
	cnt-06/Thumbnails/thumbnail.png \
	cnt-061/content.xml \
	cnt-061/META-INF/manifest.xml \
	cnt-061/meta.xml \
	cnt-061/settings.xml \
	cnt-061/styles.xml \
	cnt-061/Thumbnails/thumbnail.png \
	cnt-062/content.xml \
	cnt-062/META-INF/manifest.xml \
	cnt-062/meta.xml \
	cnt-062/settings.xml \
	cnt-062/styles.xml \
	cnt-062/Thumbnails/thumbnail.png \
	cnt-default/content.xml \
	cnt-default/META-INF/manifest.xml \
	cnt-default/meta.xml \
	cnt-default/settings.xml \
	cnt-default/styles.xml \
	cnt-default/Thumbnails/thumbnail.png \
	stl-01/content.xml \
	stl-01/manifest.rdf \
	stl-01/META-INF/manifest.xml \
	stl-01/meta.xml \
	stl-01/Pictures/100002000000001400000014E87B6AC5.gif \
	stl-01/Pictures/2000018100001CB100002711BFED37A8.wmf \
	stl-01/settings.xml \
	stl-01/styles.xml \
	stl-01/Thumbnails/thumbnail.png \
	stl-02/content.xml \
	stl-02/manifest.rdf \
	stl-02/META-INF/manifest.xml \
	stl-02/meta.xml \
	stl-02/Pictures/2000009E0000380100002657BA019D86.wmf \
	stl-02/settings.xml \
	stl-02/styles.xml \
	stl-02/Thumbnails/thumbnail.png \
	stl-03/content.xml \
	stl-03/manifest.rdf \
	stl-03/META-INF/manifest.xml \
	stl-03/meta.xml \
	stl-03/Pictures/2000024900002711000021F4DCB2DF4A.wmf \
	stl-03/settings.xml \
	stl-03/styles.xml \
	stl-03/Thumbnails/thumbnail.png \
	stl-04/content.xml \
	stl-04/manifest.rdf \
	stl-04/META-INF/manifest.xml \
	stl-04/meta.xml \
	stl-04/Pictures/10000000000000400000004077CDC8F9.png \
	stl-04/Pictures/10000000000001C6000000E44DCA6C94.jpg \
	stl-04/settings.xml \
	stl-04/styles.xml \
	stl-04/Thumbnails/thumbnail.png \
	stl-05/content.xml \
	stl-05/manifest.rdf \
	stl-05/META-INF/manifest.xml \
	stl-05/meta.xml \
	stl-05/Pictures/2000036D000027110000145EB7E93049.wmf \
	stl-05/settings.xml \
	stl-05/styles.xml \
	stl-05/Thumbnails/thumbnail.png \
	stl-06/content.xml \
	stl-06/manifest.rdf \
	stl-06/META-INF/manifest.xml \
	stl-06/meta.xml \
	stl-06/Pictures/100000000000005A000000268DE98CCE.gif \
	stl-06/Pictures/100000000000021B000001E8BE936AB6.gif \
	stl-06/settings.xml \
	stl-06/styles.xml \
	stl-06/Thumbnails/thumbnail.png \
	stl-07/content.xml \
	stl-07/manifest.rdf \
	stl-07/META-INF/manifest.xml \
	stl-07/meta.xml \
	stl-07/settings.xml \
	stl-07/styles.xml \
	stl-07/Thumbnails/thumbnail.png \
	stl-08/content.xml \
	stl-08/manifest.rdf \
	stl-08/META-INF/manifest.xml \
	stl-08/meta.xml \
	stl-08/Pictures/100000000000005E0000005E97FB9EF6.jpg \
	stl-08/settings.xml \
	stl-08/styles.xml \
	stl-08/Thumbnails/thumbnail.png \
	stl-09/content.xml \
	stl-09/manifest.rdf \
	stl-09/META-INF/manifest.xml \
	stl-09/meta.xml \
	stl-09/Pictures/10000000000000050000004D32652675.jpg \
	stl-09/Pictures/100002000000001400000014E87B6AC5.gif \
	stl-09/Pictures/2000025100001D92000017719EB70CFB.wmf \
	stl-09/settings.xml \
	stl-09/styles.xml \
	stl-09/Thumbnails/thumbnail.png \
	stl-default/content.xml \
	stl-default/manifest.rdf \
	stl-default/META-INF/manifest.xml \
	stl-default/meta.xml \
	stl-default/settings.xml \
	stl-default/styles.xml \
	stl-default/Thumbnails/thumbnail.png \

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
            mimetype $$(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/wizard/report/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/report/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_WIZARD_REPORT_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/report/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
