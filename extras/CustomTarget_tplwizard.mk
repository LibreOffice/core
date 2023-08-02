# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard))

# mimetype, content.xml, settings.xml, styles.xml and META-INF/manifest.xml
# files are automatically added for each template
# list of meta.xml files (one per template) + other files (manifest.rdf, pictures...)
extras_WIZARD_XMLFILES := \
	agenda/10grey/meta.xml \
	agenda/1simple/meta.xml \
	agenda/2elegant/meta.xml \
	agenda/3modern/meta.xml \
	agenda/4classic/meta.xml \
	agenda/5blue/meta.xml \
	agenda/6orange/meta.xml \
	agenda/7red/meta.xml \
	agenda/8green/meta.xml \
	agenda/9colorful/meta.xml \
	agenda/aw-10grey/meta.xml \
	agenda/aw-1simple/meta.xml \
	agenda/aw-2elegant/meta.xml \
	agenda/aw-3modern/meta.xml \
	agenda/aw-4classic/meta.xml \
	agenda/aw-5blue/meta.xml \
	agenda/aw-6orange/meta.xml \
	agenda/aw-7red/meta.xml \
	agenda/aw-8green/meta.xml \
	agenda/aw-9colorful/meta.xml \
	agenda/10grey/manifest.rdf \
	agenda/1simple/manifest.rdf \
	agenda/2elegant/manifest.rdf \
	agenda/3modern/manifest.rdf \
	agenda/4classic/manifest.rdf \
	agenda/5blue/manifest.rdf \
	agenda/6orange/manifest.rdf \
	agenda/7red/manifest.rdf \
	agenda/8green/manifest.rdf \
	agenda/9colorful/manifest.rdf \
	agenda/aw-10grey/manifest.rdf \
	agenda/aw-1simple/manifest.rdf \
	agenda/aw-2elegant/manifest.rdf \
	agenda/aw-3modern/manifest.rdf \
	agenda/aw-4classic/manifest.rdf \
	agenda/aw-5blue/manifest.rdf \
	agenda/aw-6orange/manifest.rdf \
	agenda/aw-7red/manifest.rdf \
	agenda/aw-8green/manifest.rdf \
	agenda/aw-9colorful/manifest.rdf \
	agenda/2elegant/Pictures/1000000000000280000000052876A81A.gif \
	agenda/2elegant/Pictures/100002000000008F0000000A83659D51.gif \
	agenda/aw-2elegant/Pictures/1000000000000280000000052876A81A.gif \
	agenda/aw-2elegant/Pictures/100002000000008F0000000A83659D51.gif \
	fax/bus-classic_f/meta.xml \
	fax/bus-classic-pri_f/meta.xml \
	fax/bus-modern_f/meta.xml \
	fax/bus-modern-pri_f/meta.xml \
	fax/pri-bottle_f/meta.xml \
	fax/pri-fax_f/meta.xml \
	fax/pri-lines_f/meta.xml \
	fax/pri-marine_f/meta.xml \
	fax/bus-classic_f/manifest.rdf \
	fax/bus-classic-pri_f/manifest.rdf \
	fax/bus-modern_f/manifest.rdf \
	fax/bus-modern-pri_f/manifest.rdf \
	fax/bus-classic_f/Pictures/200000520000271100001F73A977C564.wmf \
	fax/bus-modern_f/Pictures/2000004E0000271100001F73772F7F48.wmf \
	fax/pri-bottle_f/Pictures/200002EB000022DA000024E1F891208C.wmf \
	fax/pri-fax_f/Pictures/2000032600002711000026A54A905481.wmf \
	fax/pri-marine_f/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
	letter/bus-elegant_l/meta.xml \
	letter/bus-modern_l/meta.xml \
	letter/bus-office_l/meta.xml \
	letter/off-elegant_l/meta.xml \
	letter/off-modern_l/meta.xml \
	letter/off-office_l/meta.xml \
	letter/pri-bottle_l/meta.xml \
	letter/pri-mail_l/meta.xml \
	letter/pri-marine_l/meta.xml \
	letter/pri-redline_l/meta.xml \
	letter/bus-elegant_l/manifest.rdf \
	letter/bus-modern_l/manifest.rdf \
	letter/bus-office_l/manifest.rdf \
	letter/off-elegant_l/manifest.rdf \
	letter/off-modern_l/manifest.rdf \
	letter/off-office_l/manifest.rdf \
	letter/bus-elegant_l/Pictures/10000000000001B8000000824B7EA208.png \
	letter/bus-modern_l/Pictures/10000000000001B8000000824B7EA208.png \
	letter/bus-office_l/Pictures/10000000000001B8000000824B7EA208.png \
	letter/off-elegant_l/Pictures/2000000D00000434000006FD6841386B.svm \
	letter/off-modern_l/Pictures/10000000000000110000001142D1547F.png \
	letter/pri-bottle_l/Pictures/200002EB000022DA000024E1F891208C.wmf \
	letter/pri-mail_l/Pictures/200000430000271100001205CAE80946.wmf \
	letter/pri-marine_l/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
	report/cnt-01/meta.xml \
	report/cnt-011/meta.xml \
	report/cnt-012/meta.xml \
	report/cnt-02/meta.xml \
	report/cnt-021/meta.xml \
	report/cnt-022/meta.xml \
	report/cnt-03/meta.xml \
	report/cnt-031/meta.xml \
	report/cnt-032/meta.xml \
	report/cnt-04/meta.xml \
	report/cnt-041/meta.xml \
	report/cnt-042/meta.xml \
	report/cnt-05/meta.xml \
	report/cnt-051/meta.xml \
	report/cnt-052/meta.xml \
	report/cnt-06/meta.xml \
	report/cnt-061/meta.xml \
	report/cnt-062/meta.xml \
	report/cnt-default/meta.xml \
	report/stl-01/meta.xml \
	report/stl-02/meta.xml \
	report/stl-03/meta.xml \
	report/stl-04/meta.xml \
	report/stl-05/meta.xml \
	report/stl-06/meta.xml \
	report/stl-07/meta.xml \
	report/stl-08/meta.xml \
	report/stl-09/meta.xml \
	report/stl-default/meta.xml \
	report/stl-01/manifest.rdf \
	report/stl-02/manifest.rdf \
	report/stl-03/manifest.rdf \
	report/stl-04/manifest.rdf \
	report/stl-05/manifest.rdf \
	report/stl-06/manifest.rdf \
	report/stl-07/manifest.rdf \
	report/stl-08/manifest.rdf \
	report/stl-09/manifest.rdf \
	report/stl-default/manifest.rdf \
	report/stl-01/Pictures/100002000000001400000014E87B6AC5.gif \
	report/stl-01/Pictures/2000018100001CB100002711BFED37A8.wmf \
	report/stl-02/Pictures/2000009E0000380100002657BA019D86.wmf \
	report/stl-03/Pictures/2000024900002711000021F4DCB2DF4A.wmf \
	report/stl-04/Pictures/10000000000000400000004077CDC8F9.png \
	report/stl-04/Pictures/10000000000001C6000000E44DCA6C94.jpg \
	report/stl-05/Pictures/2000036D000027110000145EB7E93049.wmf \
	report/stl-06/Pictures/100000000000005A000000268DE98CCE.gif \
	report/stl-06/Pictures/100000000000021B000001E8BE936AB6.gif \
	report/stl-08/Pictures/100000000000005E0000005E97FB9EF6.jpg \
	report/stl-09/Pictures/10000000000000050000004D32652675.jpg \
	report/stl-09/Pictures/100002000000001400000014E87B6AC5.gif \
	report/stl-09/Pictures/2000025100001D92000017719EB70CFB.wmf \
	styles/black_white/meta.xml \
	styles/blackberry/meta.xml \
	styles/default/meta.xml \
	styles/diner/meta.xml \
	styles/fall/meta.xml \
	styles/glacier/meta.xml \
	styles/green_grapes/meta.xml \
	styles/jeans/meta.xml \
	styles/marine/meta.xml \
	styles/millennium/meta.xml \
	styles/nature/meta.xml \
	styles/neon/meta.xml \
	styles/night/meta.xml \
	styles/nostalgic/meta.xml \
	styles/pastell/meta.xml \
	styles/pool/meta.xml \
	styles/pumpkin/meta.xml \
	styles/xos/meta.xml \

# param: style-base (e.g. agenda/10grey)
extras_WIZARD_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/% : $(SRCDIR)/extras/source/templates/wizard/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*).dir
	$(call gb_Output_announce,templates/wizard/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/$*,CPY)

# test and copy xml files
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/%.xml : $(SRCDIR)/extras/source/templates/wizard/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/$*.xml,XSL)

# zip files to OTT
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            mimetype $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            content.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            settings.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            styles.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            META-INF/manifest.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/wizard/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml settings.xml styles.xml META-INF/manifest.xml && \
		$(WSL) zip -qrX --must-match $@ $(call extras_WIZARD_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/wizard/$*.ott,ZIP)

# zip files to OTS
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/%.ots : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            mimetype $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            content.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            settings.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            styles.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard)/$$*/,\
            META-INF/manifest.xml $$(call extras_WIZARD_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/wizard/$*.ots,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/$*.ots,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml settings.xml styles.xml META-INF/manifest.xml && \
		$(WSL) zip -qrX --must-match $@ $(call extras_WIZARD_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/wizard/$*.ots,ZIP)

# vim: set noet sw=4 ts=4:
