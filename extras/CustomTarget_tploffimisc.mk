# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/offimisc))

extras_TEMPLATES_OFFIMISC := \
	Businesscard-with-logo \


extras_OFFIMISC_XMLFILES := \
	Businesscard-with-logo/content.xml \
	Businesscard-with-logo/manifest.rdf \
	Businesscard-with-logo/META-INF/manifest.xml \
	Businesscard-with-logo/meta.xml \
	Businesscard-with-logo/Pictures/10000201000001F4000000A0108F3F06.png \
	Businesscard-with-logo/settings.xml \
	Businesscard-with-logo/styles.xml \
	Businesscard-with-logo/Thumbnails/thumbnail.png \


extras_OFFIMISC_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_OFFIMISC),$(atexts)/mimetype)


$(call gb_CustomTarget_get_target,extras/source/templates/offimisc) : \
	$(foreach atexts,$(extras_TEMPLATES_OFFIMISC),$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/$(atexts).ott)

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%/mimetype : $(SRCDIR)/extras/source/templates/offimisc/%/mimetype
	$(call gb_Output_announce,templates/offimisc/$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.jpg : $(SRCDIR)/extras/source/templates/offimisc/%.jpg
	$(call gb_Output_announce,templates/offimisc/$*.jpg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.png : $(SRCDIR)/extras/source/templates/offimisc/%.png
	$(call gb_Output_announce,templates/offimisc/$*.png,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.rdf : $(SRCDIR)/extras/source/templates/offimisc/%.rdf
	$(call gb_Output_announce,templates/offimisc/$*.rdf,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.svg : $(SRCDIR)/extras/source/templates/offimisc/%.svg
	$(call gb_Output_announce,templates/offimisc/$*.svg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.svm : $(SRCDIR)/extras/source/templates/offimisc/%.svm
	$(call gb_Output_announce,templates/offimisc/$*.svm,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.xml : $(SRCDIR)/extras/source/templates/offimisc/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,templates/offimisc/$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.ott :
	$(call gb_Output_announce,templates/offimisc/$*.ott,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_OFFIMISC_DIR) && \
		zip -q0X --filesync --must-match $@ $(EXTRAS_OFFIMISC_MIMEFILES_FILTER) && \
		zip -qrX --must-match $@ $(EXTRAS_OFFIMISC_XMLFILES_FILTER) \
	)

define extras_Tploffimisc_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tploffimisc_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_OFFIMISC_MIMETYPEFILES) $(extras_OFFIMISC_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_OFFIMISC_MIMEFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_OFFIMISC_MIMETYPEFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_OFFIMISC_XMLFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_OFFIMISC_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_OFFIMISC_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_OFFIMISC_MIMETYPEFILES) $(extras_OFFIMISC_XMLFILES),\
	$(call extras_Tploffimisc_make_file_deps,extras/source/templates/offimisc,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_OFFIMISC),\
	$(call extras_Tploffimisc_make_zip_deps,extras/source/templates/offimisc,$(atexts).ott,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
