# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/wizard/styles))

extras_WIZARD_STYLES_XMLFILES := \
	black_white/content.xml \
	black_white/META-INF/manifest.xml \
	black_white/meta.xml \
	black_white/settings.xml \
	black_white/styles.xml \
	black_white/Thumbnails/thumbnail.png \
	blackberry/content.xml \
	blackberry/META-INF/manifest.xml \
	blackberry/meta.xml \
	blackberry/settings.xml \
	blackberry/styles.xml \
	blackberry/Thumbnails/thumbnail.png \
	default/content.xml \
	default/META-INF/manifest.xml \
	default/meta.xml \
	default/settings.xml \
	default/styles.xml \
	default/Thumbnails/thumbnail.png \
	diner/content.xml \
	diner/META-INF/manifest.xml \
	diner/meta.xml \
	diner/settings.xml \
	diner/styles.xml \
	diner/Thumbnails/thumbnail.png \
	fall/content.xml \
	fall/META-INF/manifest.xml \
	fall/meta.xml \
	fall/settings.xml \
	fall/styles.xml \
	fall/Thumbnails/thumbnail.png \
	glacier/content.xml \
	glacier/META-INF/manifest.xml \
	glacier/meta.xml \
	glacier/settings.xml \
	glacier/styles.xml \
	glacier/Thumbnails/thumbnail.png \
	green_grapes/content.xml \
	green_grapes/META-INF/manifest.xml \
	green_grapes/meta.xml \
	green_grapes/settings.xml \
	green_grapes/styles.xml \
	green_grapes/Thumbnails/thumbnail.png \
	jeans/content.xml \
	jeans/META-INF/manifest.xml \
	jeans/meta.xml \
	jeans/settings.xml \
	jeans/styles.xml \
	jeans/Thumbnails/thumbnail.png \
	marine/content.xml \
	marine/META-INF/manifest.xml \
	marine/meta.xml \
	marine/settings.xml \
	marine/styles.xml \
	marine/Thumbnails/thumbnail.png \
	millennium/content.xml \
	millennium/META-INF/manifest.xml \
	millennium/meta.xml \
	millennium/settings.xml \
	millennium/styles.xml \
	millennium/Thumbnails/thumbnail.png \
	nature/content.xml \
	nature/META-INF/manifest.xml \
	nature/meta.xml \
	nature/settings.xml \
	nature/styles.xml \
	nature/Thumbnails/thumbnail.png \
	neon/content.xml \
	neon/META-INF/manifest.xml \
	neon/meta.xml \
	neon/settings.xml \
	neon/styles.xml \
	neon/Thumbnails/thumbnail.png \
	night/content.xml \
	night/META-INF/manifest.xml \
	night/meta.xml \
	night/settings.xml \
	night/styles.xml \
	night/Thumbnails/thumbnail.png \
	nostalgic/content.xml \
	nostalgic/META-INF/manifest.xml \
	nostalgic/meta.xml \
	nostalgic/settings.xml \
	nostalgic/styles.xml \
	nostalgic/Thumbnails/thumbnail.png \
	pastell/content.xml \
	pastell/META-INF/manifest.xml \
	pastell/meta.xml \
	pastell/settings.xml \
	pastell/styles.xml \
	pastell/Thumbnails/thumbnail.png \
	pool/content.xml \
	pool/META-INF/manifest.xml \
	pool/meta.xml \
	pool/settings.xml \
	pool/styles.xml \
	pool/Thumbnails/thumbnail.png \
	pumpkin/content.xml \
	pumpkin/META-INF/manifest.xml \
	pumpkin/meta.xml \
	pumpkin/settings.xml \
	pumpkin/styles.xml \
	pumpkin/Thumbnails/thumbnail.png \
	xos/content.xml \
	xos/META-INF/manifest.xml \
	xos/meta.xml \
	xos/settings.xml \
	xos/styles.xml \
	xos/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_WIZARD_STYLES_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_WIZARD_STYLES_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/% : $(SRCDIR)/extras/source/templates/wizard/styles/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/$$*).dir
	$(call gb_Output_announce,templates/wizard/styles/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/wizard/styles/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/wizard/styles/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/%.xml : $(SRCDIR)/extras/source/templates/wizard/styles/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/$$*.xml).dir
	$(call gb_Output_announce,templates/wizard/styles/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/wizard/styles/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/wizard/styles/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/%.ots : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/wizard/styles)/$$*/,\
            mimetype $$(call extras_WIZARD_STYLES_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/wizard/styles/$*.ots,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/wizard/styles/$*.ots,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_WIZARD_STYLES_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/wizard/styles/$*.ots,ZIP)

# vim: set noet sw=4 ts=4:
