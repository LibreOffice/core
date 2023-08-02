# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates))

# # for OTT templates: mimetype, styles.xml, META-INF/manifest.xml and
# Thumbnails/thumbnail.png files are automatically added for each template
# # for OTG templates: mimetype, content.xml, styles.xml, META-INF/manifest.xml and
# Thumbnails/thumbnail.png files are automatically added for each template
# list of meta.xml files (one per template) + other files (content.xml, manifest.rdf, settings.xml, pictures...)
extras_TEMPLATES_XMLFILES := \
	officorr/Modern_business_letter_sans_serif/meta.xml \
	officorr/Modern_business_letter_serif/meta.xml \
	offimisc/Businesscard-with-logo/meta.xml \
	personal/CV/meta.xml \
	personal/Resume1page/meta.xml \
	styles/Default/meta.xml \
	styles/Modern/meta.xml \
	styles/Simple/meta.xml \
	officorr/Modern_business_letter_sans_serif/content.xml \
	officorr/Modern_business_letter_serif/content.xml \
	offimisc/Businesscard-with-logo/content.xml \
	personal/CV/content.xml \
	personal/Resume1page/content.xml \
	officorr/Modern_business_letter_sans_serif/manifest.rdf \
	officorr/Modern_business_letter_serif/manifest.rdf \
	offimisc/Businesscard-with-logo/manifest.rdf \
	personal/CV/manifest.rdf \
	personal/Resume1page/manifest.rdf \
	offimisc/Businesscard-with-logo/settings.xml \
	offimisc/Businesscard-with-logo/Pictures/10000201000001F4000000A0108F3F06.png \
	draw/bpmn/meta.xml \
	l10n/zh_CN_ott_normal/meta.xml \
	l10n/zh_CN_ott_normal/content.xml \
	l10n/ja_ott_normal/meta.xml \
	l10n/ja_ott_normal/content.xml \

# param: style-base (e.g. Modern)
extras_TEMPLATES_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_TEMPLATES_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates)/% : $(SRCDIR)/extras/source/templates/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*).dir
	$(call gb_Output_announce,templates/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/$*,CPY)

# test and copy xml files
$(call gb_CustomTarget_get_workdir,extras/source/templates)/%.xml : $(SRCDIR)/extras/source/templates/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*.xml).dir
	$(call gb_Output_announce,templates/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/$*.xml,XSL)

# zip files to OTT
$(call gb_CustomTarget_get_workdir,extras/source/templates)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            mimetype $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            styles.xml $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            META-INF/manifest.xml $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            Thumbnails/thumbnail.png $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ styles.xml META-INF/manifest.xml Thumbnails/thumbnail.png && \
		$(WSL) zip -qrX --must-match $@ $(call extras_TEMPLATES_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/$*.ott,ZIP)

# zip files to OTG
$(call gb_CustomTarget_get_workdir,extras/source/templates)/%.otg : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            mimetype $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            content.xml $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            styles.xml $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            META-INF/manifest.xml $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates)/$$*/,\
            Thumbnails/thumbnail.png $$(call extras_TEMPLATES_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/$*.otg,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/$*.otg,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml styles.xml META-INF/manifest.xml Thumbnails/thumbnail.png && \
		$(WSL) zip -qrX --must-match $@ $(call extras_TEMPLATES_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/$*.otg,ZIP)

# vim: set noet sw=4 ts=4:
