# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard))

# mimetype, content.xml, meta.xml, styles.xml and META-INF/manifest.xml
# files are automatically added for each template
# list of meta.xml files (one per template) + other files (manifest.rdf, pictures...)
extras_WIZDESKTOP_XMLFILES := \
	desktop/html/meta.xml \
	desktop/idxexample/meta.xml \
	desktop/idxexample/Pictures/2000008600001923000012C24E0D0895.wmf \

# param: style-base (e.g. agenda/10grey)
extras_WIZDESKTOP_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZDESKTOP_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(gb_CustomTarget_workdir)/extras/source/templates/wizard/% : $(SRCDIR)/extras/source/templates/wizard/% \
        | $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*).dir
	$(call gb_Output_announce,templates/wizard/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/$*,CPY)

# test and copy xml files
$(gb_CustomTarget_workdir)/extras/source/templates/wizard/%.xml : $(SRCDIR)/extras/source/templates/wizard/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/$*.xml,XSL)

# zip files to OTH
$(gb_CustomTarget_workdir)/extras/source/templates/wizard/%.oth : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            mimetype $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            content.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            styles.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            settings.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            META-INF/manifest.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/wizard/$*.oth,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/$*.oth,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml settings.xml styles.xml META-INF/manifest.xml && \
		$(WSL) zip -qrX --must-match $@ $(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/wizard/$*.oth,ZIP)

# zip files to ODT
$(gb_CustomTarget_workdir)/extras/source/templates/wizard/%.odt : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            mimetype $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            content.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            settings.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            styles.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) ) \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/wizard/$$*/,\
            META-INF/manifest.xml $$(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$$*) )
	$(call gb_Output_announce,templates/wizard/$*.odt,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/$*.odt,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ content.xml settings.xml styles.xml META-INF/manifest.xml && \
		$(WSL) zip -qrX --must-match $@ $(call extras_WIZDESKTOP_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,templates/wizard/$*.odt,ZIP)

# vim: set noet sw=4 ts=4:
