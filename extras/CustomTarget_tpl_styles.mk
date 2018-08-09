# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/styles))

extras_TEMPLATES_STYLES := \
	Default \
	Modern \


extras_STYLES_XMLFILES := \
	Default/META-INF/manifest.xml \
	Default/meta.xml \
	Default/styles.xml \
	Default/Thumbnails/thumbnail.png \
	Modern/META-INF/manifest.xml \
	Modern/meta.xml \
	Modern/styles.xml \
	Modern/Thumbnails/thumbnail.png \


extras_TPLSTYLES_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_STYLES),$(atexts)/mimetype)


ifneq ($(sort $(foreach file,$(extras_STYLES_XMLFILES),$(wordlist 1, 1, $(subst /, ,$(file))))),$(sort $(extras_TEMPLATES_STYLES)))
$(call gb_Output_error,defined template text do not match existing directories)
endif

$(call gb_CustomTarget_get_target,extras/source/templates/styles) : \
	$(foreach atexts,$(extras_TEMPLATES_STYLES),$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/$(atexts).ott)

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%/mimetype : $(SRCDIR)/extras/source/templates/styles/%/mimetype
	$(call gb_Output_announce,$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.jpg : $(SRCDIR)/extras/source/templates/styles/%.jpg
	$(call gb_Output_announce,$*.jpg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.png : $(SRCDIR)/extras/source/templates/styles/%.png
	$(call gb_Output_announce,$*.png,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.rdf : $(SRCDIR)/extras/source/templates/styles/%.rdf
	$(call gb_Output_announce,$*.rdf,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.svg : $(SRCDIR)/extras/source/templates/styles/%.svg
	$(call gb_Output_announce,$*.svg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.svm : $(SRCDIR)/extras/source/templates/styles/%.svm
	$(call gb_Output_announce,$*.svm,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.xml : $(SRCDIR)/extras/source/templates/styles/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.ott :
	$(call gb_Output_announce,$*.ott,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_STYLES_DIR) && \
		zip -qrX --filesync --must-match $@ $(EXTRAS_STYLES_FILES) \
	)

define extras_Tplstyles_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tplstyles_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_TPLSTYLES_MIMETYPEFILES) $(extras_STYLES_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_STYLES_FILES := $(foreach file,$(filter $(3)/%,$(extras_TPLSTYLES_MIMETYPEFILES) $(extras_STYLES_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_STYLES_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_TPLSTYLES_MIMETYPEFILES) $(extras_STYLES_XMLFILES),\
	$(call extras_Tplstyles_make_zip_deps,extras/source/templates/styles,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_STYLES),\
	$(call extras_Tplstyles_make_zip_deps,extras/source/templates/styles,$(atexts).ott,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
