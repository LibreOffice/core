# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard/fax))

extras_WIZARD_FAX_XMLFILES := \
	bus-classic_f/content.xml \
	bus-classic_f/manifest.rdf \
	bus-classic_f/META-INF/manifest.xml \
	bus-classic_f/meta.xml \
	bus-classic_f/Pictures/200000520000271100001F73A977C564.wmf \
	bus-classic_f/settings.xml \
	bus-classic_f/styles.xml \
	bus-classic_f/Thumbnails/thumbnail.png \
	bus-classic-pri_f/content.xml \
	bus-classic-pri_f/manifest.rdf \
	bus-classic-pri_f/META-INF/manifest.xml \
	bus-classic-pri_f/meta.xml \
	bus-classic-pri_f/settings.xml \
	bus-classic-pri_f/styles.xml \
	bus-classic-pri_f/Thumbnails/thumbnail.png \
	bus-modern_f/content.xml \
	bus-modern_f/manifest.rdf \
	bus-modern_f/META-INF/manifest.xml \
	bus-modern_f/meta.xml \
	bus-modern_f/Pictures/2000004E0000271100001F73772F7F48.wmf \
	bus-modern_f/settings.xml \
	bus-modern_f/styles.xml \
	bus-modern_f/Thumbnails/thumbnail.png \
	bus-modern-pri_f/content.xml \
	bus-modern-pri_f/manifest.rdf \
	bus-modern-pri_f/META-INF/manifest.xml \
	bus-modern-pri_f/meta.xml \
	bus-modern-pri_f/settings.xml \
	bus-modern-pri_f/styles.xml \
	bus-modern-pri_f/Thumbnails/thumbnail.png \
	pri-bottle_f/content.xml \
	pri-bottle_f/META-INF/manifest.xml \
	pri-bottle_f/meta.xml \
	pri-bottle_f/Pictures/200002EB000022DA000024E1F891208C.wmf \
	pri-bottle_f/settings.xml \
	pri-bottle_f/styles.xml \
	pri-bottle_f/Thumbnails/thumbnail.png \
	pri-fax_f/content.xml \
	pri-fax_f/META-INF/manifest.xml \
	pri-fax_f/meta.xml \
	pri-fax_f/Pictures/2000032600002711000026A54A905481.wmf \
	pri-fax_f/settings.xml \
	pri-fax_f/styles.xml \
	pri-fax_f/Thumbnails/thumbnail.png \
	pri-lines_f/content.xml \
	pri-lines_f/META-INF/manifest.xml \
	pri-lines_f/meta.xml \
	pri-lines_f/settings.xml \
	pri-lines_f/styles.xml \
	pri-lines_f/Thumbnails/thumbnail.png \
	pri-marine_f/content.xml \
	pri-marine_f/META-INF/manifest.xml \
	pri-marine_f/meta.xml \
	pri-marine_f/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
	pri-marine_f/settings.xml \
	pri-marine_f/styles.xml \
	pri-marine_f/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_WIZARD_FAX_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_FAX_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/% : $(SRCDIR)/extras/source/templates/wizard/fax/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/$$*).dir
	$(call gb_Output_announce,templates/wizard/fax/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/fax/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/fax/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/%.xml : $(SRCDIR)/extras/source/templates/wizard/fax/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/fax/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/fax/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/fax/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/fax)/$$*/,\
            mimetype $$(call extras_WIZARD_FAX_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/wizard/fax/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/fax/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_WIZARD_FAX_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/fax/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
