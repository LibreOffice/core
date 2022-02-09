# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/officorr))

extras_OFFICORR_XMLFILES := \
	Modern_business_letter_sans_serif/content.xml \
	Modern_business_letter_sans_serif/manifest.rdf \
	Modern_business_letter_sans_serif/META-INF/manifest.xml \
	Modern_business_letter_sans_serif/meta.xml \
	Modern_business_letter_sans_serif/styles.xml \
	Modern_business_letter_sans_serif/Thumbnails/thumbnail.png \
	Modern_business_letter_serif/content.xml \
	Modern_business_letter_serif/manifest.rdf \
	Modern_business_letter_serif/META-INF/manifest.xml \
	Modern_business_letter_serif/meta.xml \
	Modern_business_letter_serif/styles.xml \
	Modern_business_letter_serif/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_OFFICORR_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_OFFICORR_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/% : $(SRCDIR)/extras/source/templates/officorr/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/$$*).dir
	$(call gb_Output_announce,templates/officorr/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/officorr/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/officorr/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.xml : $(SRCDIR)/extras/source/templates/officorr/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/$$*.xml).dir
	$(call gb_Output_announce,templates/officorr/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/officorr/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/officorr/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/$$*/,\
            mimetype $$(call extras_OFFICORR_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/officorr/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/officorr/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_OFFICORR_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/officorr/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
