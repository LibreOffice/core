# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard/letter))

extras_WIZARD_LETTER_XMLFILES := \
	bus-elegant_l/content.xml \
	bus-elegant_l/manifest.rdf \
	bus-elegant_l/META-INF/manifest.xml \
	bus-elegant_l/meta.xml \
	bus-elegant_l/Pictures/10000000000001B8000000824B7EA208.png \
	bus-elegant_l/settings.xml \
	bus-elegant_l/styles.xml \
	bus-elegant_l/Thumbnails/thumbnail.png \
	bus-modern_l/content.xml \
	bus-modern_l/manifest.rdf \
	bus-modern_l/META-INF/manifest.xml \
	bus-modern_l/meta.xml \
	bus-modern_l/Pictures/10000000000001B8000000824B7EA208.png \
	bus-modern_l/settings.xml \
	bus-modern_l/styles.xml \
	bus-modern_l/Thumbnails/thumbnail.png \
	bus-office_l/content.xml \
	bus-office_l/manifest.rdf \
	bus-office_l/META-INF/manifest.xml \
	bus-office_l/meta.xml \
	bus-office_l/Pictures/10000000000001B8000000824B7EA208.png \
	bus-office_l/settings.xml \
	bus-office_l/styles.xml \
	bus-office_l/Thumbnails/thumbnail.png \
	off-elegant_l/content.xml \
	off-elegant_l/manifest.rdf \
	off-elegant_l/META-INF/manifest.xml \
	off-elegant_l/meta.xml \
	off-elegant_l/Pictures/2000000D00000434000006FD6841386B.svm \
	off-elegant_l/settings.xml \
	off-elegant_l/styles.xml \
	off-elegant_l/Thumbnails/thumbnail.png \
	off-modern_l/content.xml \
	off-modern_l/manifest.rdf \
	off-modern_l/META-INF/manifest.xml \
	off-modern_l/meta.xml \
	off-modern_l/Pictures/10000000000000110000001142D1547F.png \
	off-modern_l/settings.xml \
	off-modern_l/styles.xml \
	off-modern_l/Thumbnails/thumbnail.png \
	off-office_l/content.xml \
	off-office_l/manifest.rdf \
	off-office_l/META-INF/manifest.xml \
	off-office_l/meta.xml \
	off-office_l/settings.xml \
	off-office_l/styles.xml \
	off-office_l/Thumbnails/thumbnail.png \
	pri-bottle_l/content.xml \
	pri-bottle_l/META-INF/manifest.xml \
	pri-bottle_l/meta.xml \
	pri-bottle_l/Pictures/200002EB000022DA000024E1F891208C.wmf \
	pri-bottle_l/settings.xml \
	pri-bottle_l/styles.xml \
	pri-bottle_l/Thumbnails/thumbnail.png \
	pri-mail_l/content.xml \
	pri-mail_l/META-INF/manifest.xml \
	pri-mail_l/meta.xml \
	pri-mail_l/Pictures/200000430000271100001205CAE80946.wmf \
	pri-mail_l/settings.xml \
	pri-mail_l/styles.xml \
	pri-mail_l/Thumbnails/thumbnail.png \
	pri-marine_l/content.xml \
	pri-marine_l/META-INF/manifest.xml \
	pri-marine_l/meta.xml \
	pri-marine_l/Pictures/2000004B0000227F0000227886F3FC0F.wmf \
	pri-marine_l/settings.xml \
	pri-marine_l/styles.xml \
	pri-marine_l/Thumbnails/thumbnail.png \
	pri-redline_l/content.xml \
	pri-redline_l/META-INF/manifest.xml \
	pri-redline_l/meta.xml \
	pri-redline_l/settings.xml \
	pri-redline_l/styles.xml \
	pri-redline_l/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_WIZARD_LETTER_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_LETTER_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/% : $(SRCDIR)/extras/source/templates/wizard/letter/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/$$*).dir
	$(call gb_Output_announce,templates/wizard/letter/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/letter/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/letter/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/%.xml : $(SRCDIR)/extras/source/templates/wizard/letter/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/letter/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/letter/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/letter/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/letter)/$$*/,\
            mimetype $$(call extras_WIZARD_LETTER_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/wizard/letter/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/letter/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_WIZARD_LETTER_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/letter/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
