# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/officorr))

extras_TEMPLATES_OFFICORR := \
	Modern_business_letter_sans_serif \
	Modern_business_letter_serif \


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


extras_TPLOFFICORR_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_OFFICORR),$(atexts)/mimetype)


ifneq ($(sort $(foreach file,$(extras_OFFICORR_XMLFILES),$(wordlist 1, 1, $(subst /, ,$(file))))),$(sort $(extras_TEMPLATES_OFFICORR)))
$(call gb_Output_error,defined template text do not match existing directories)
endif

$(call gb_CustomTarget_get_target,extras/source/templates/officorr) : \
	$(foreach atexts,$(extras_TEMPLATES_OFFICORR),$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/$(atexts).ott)

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%/mimetype : $(SRCDIR)/extras/source/templates/officorr/%/mimetype
	$(call gb_Output_announce,$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.jpg : $(SRCDIR)/extras/source/templates/officorr/%.jpg
	$(call gb_Output_announce,$*.jpg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.rdf : $(SRCDIR)/extras/source/templates/officorr/%.rdf
	$(call gb_Output_announce,$*.rdf,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.png : $(SRCDIR)/extras/source/templates/officorr/%.png
	$(call gb_Output_announce,$*.png,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.svg : $(SRCDIR)/extras/source/templates/officorr/%.svg
	$(call gb_Output_announce,$*.svg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.svm : $(SRCDIR)/extras/source/templates/officorr/%.svm
	$(call gb_Output_announce,$*.svm,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.xml : $(SRCDIR)/extras/source/templates/officorr/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/templates/officorr)/%.ott :
	$(call gb_Output_announce,$*.ott,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_OFFICORR_DIR) && \
		zip -qrX --filesync --must-match $@ $(EXTRAS_OFFICORR_FILES) \
	)

define extras_Tplofficorr_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tplofficorr_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_TPLOFFICORR_MIMETYPEFILES) $(extras_OFFICORR_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_OFFICORR_FILES := $(foreach file,$(filter $(3)/%,$(extras_TPLOFFICORR_MIMETYPEFILES) $(extras_OFFICORR_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_OFFICORR_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_TPLOFFICORR_MIMETYPEFILES) $(extras_OFFICORR_XMLFILES),\
	$(call extras_Tplofficorr_make_zip_deps,extras/source/templates/officorr,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_OFFICORR),\
	$(call extras_Tplofficorr_make_zip_deps,extras/source/templates/officorr,$(atexts).ott,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
