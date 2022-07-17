# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard/agenda))

extras_WIZARD_AGENDA_XMLFILES := \
	10grey/content.xml \
	10grey/manifest.rdf \
	10grey/META-INF/manifest.xml \
	10grey/meta.xml \
	10grey/settings.xml \
	10grey/styles.xml \
	10grey/Thumbnails/thumbnail.png \
	1simple/content.xml \
	1simple/manifest.rdf \
	1simple/META-INF/manifest.xml \
	1simple/meta.xml \
	1simple/settings.xml \
	1simple/styles.xml \
	1simple/Thumbnails/thumbnail.png \
	2elegant/content.xml \
	2elegant/manifest.rdf \
	2elegant/META-INF/manifest.xml \
	2elegant/meta.xml \
	2elegant/Pictures/1000000000000280000000052876A81A.gif \
	2elegant/Pictures/100002000000008F0000000A83659D51.gif \
	2elegant/settings.xml \
	2elegant/styles.xml \
	2elegant/Thumbnails/thumbnail.png \
	3modern/content.xml \
	3modern/manifest.rdf \
	3modern/META-INF/manifest.xml \
	3modern/meta.xml \
	3modern/settings.xml \
	3modern/styles.xml \
	3modern/Thumbnails/thumbnail.png \
	4classic/content.xml \
	4classic/manifest.rdf \
	4classic/META-INF/manifest.xml \
	4classic/meta.xml \
	4classic/settings.xml \
	4classic/styles.xml \
	4classic/Thumbnails/thumbnail.png \
	5blue/content.xml \
	5blue/manifest.rdf \
	5blue/META-INF/manifest.xml \
	5blue/meta.xml \
	5blue/settings.xml \
	5blue/styles.xml \
	5blue/Thumbnails/thumbnail.png \
	6orange/content.xml \
	6orange/manifest.rdf \
	6orange/META-INF/manifest.xml \
	6orange/meta.xml \
	6orange/settings.xml \
	6orange/styles.xml \
	6orange/Thumbnails/thumbnail.png \
	7red/content.xml \
	7red/manifest.rdf \
	7red/META-INF/manifest.xml \
	7red/meta.xml \
	7red/settings.xml \
	7red/styles.xml \
	7red/Thumbnails/thumbnail.png \
	8green/content.xml \
	8green/manifest.rdf \
	8green/META-INF/manifest.xml \
	8green/meta.xml \
	8green/settings.xml \
	8green/styles.xml \
	8green/Thumbnails/thumbnail.png \
	9colorful/content.xml \
	9colorful/manifest.rdf \
	9colorful/META-INF/manifest.xml \
	9colorful/meta.xml \
	9colorful/settings.xml \
	9colorful/styles.xml \
	9colorful/Thumbnails/thumbnail.png \
	aw-10grey/content.xml \
	aw-10grey/manifest.rdf \
	aw-10grey/META-INF/manifest.xml \
	aw-10grey/meta.xml \
	aw-10grey/settings.xml \
	aw-10grey/styles.xml \
	aw-10grey/Thumbnails/thumbnail.png \
	aw-1simple/content.xml \
	aw-1simple/manifest.rdf \
	aw-1simple/META-INF/manifest.xml \
	aw-1simple/meta.xml \
	aw-1simple/settings.xml \
	aw-1simple/styles.xml \
	aw-1simple/Thumbnails/thumbnail.png \
	aw-2elegant/content.xml \
	aw-2elegant/manifest.rdf \
	aw-2elegant/META-INF/manifest.xml \
	aw-2elegant/meta.xml \
	aw-2elegant/Pictures/1000000000000280000000052876A81A.gif \
	aw-2elegant/Pictures/100002000000008F0000000A83659D51.gif \
	aw-2elegant/settings.xml \
	aw-2elegant/styles.xml \
	aw-2elegant/Thumbnails/thumbnail.png \
	aw-3modern/content.xml \
	aw-3modern/manifest.rdf \
	aw-3modern/META-INF/manifest.xml \
	aw-3modern/meta.xml \
	aw-3modern/settings.xml \
	aw-3modern/styles.xml \
	aw-3modern/Thumbnails/thumbnail.png \
	aw-4classic/content.xml \
	aw-4classic/manifest.rdf \
	aw-4classic/META-INF/manifest.xml \
	aw-4classic/meta.xml \
	aw-4classic/settings.xml \
	aw-4classic/styles.xml \
	aw-4classic/Thumbnails/thumbnail.png \
	aw-5blue/content.xml \
	aw-5blue/manifest.rdf \
	aw-5blue/META-INF/manifest.xml \
	aw-5blue/meta.xml \
	aw-5blue/settings.xml \
	aw-5blue/styles.xml \
	aw-5blue/Thumbnails/thumbnail.png \
	aw-6orange/content.xml \
	aw-6orange/manifest.rdf \
	aw-6orange/META-INF/manifest.xml \
	aw-6orange/meta.xml \
	aw-6orange/settings.xml \
	aw-6orange/styles.xml \
	aw-6orange/Thumbnails/thumbnail.png \
	aw-7red/content.xml \
	aw-7red/manifest.rdf \
	aw-7red/META-INF/manifest.xml \
	aw-7red/meta.xml \
	aw-7red/settings.xml \
	aw-7red/styles.xml \
	aw-7red/Thumbnails/thumbnail.png \
	aw-8green/content.xml \
	aw-8green/manifest.rdf \
	aw-8green/META-INF/manifest.xml \
	aw-8green/meta.xml \
	aw-8green/settings.xml \
	aw-8green/styles.xml \
	aw-8green/Thumbnails/thumbnail.png \
	aw-9colorful/content.xml \
	aw-9colorful/manifest.rdf \
	aw-9colorful/META-INF/manifest.xml \
	aw-9colorful/meta.xml \
	aw-9colorful/settings.xml \
	aw-9colorful/styles.xml \
	aw-9colorful/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_WIZARD_AGENDA_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_AGENDA_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/% : $(SRCDIR)/extras/source/templates/wizard/agenda/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/$$*).dir
	$(call gb_Output_announce,templates/wizard/agenda/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/agenda/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/agenda/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/%.xml : $(SRCDIR)/extras/source/templates/wizard/agenda/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/agenda/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/agenda/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/agenda/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/agenda)/$$*/,\
            mimetype $$(call extras_WIZARD_AGENDA_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/wizard/agenda/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/agenda/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_WIZARD_AGENDA_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/agenda/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
