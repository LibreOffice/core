# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/personal))

extras_TEMPLATES_PERSONAL := \
	CV \
	Resume1page \


extras_PERSONAL_XMLFILES := \
	CV/content.xml \
	CV/manifest.rdf \
	CV/META-INF/manifest.xml \
	CV/meta.xml \
	CV/styles.xml \
	CV/Thumbnails/thumbnail.png \
	Resume1page/content.xml \
	Resume1page/manifest.rdf \
	Resume1page/META-INF/manifest.xml \
	Resume1page/meta.xml \
	Resume1page/styles.xml \
	Resume1page/Thumbnails/thumbnail.png \


extras_PERSONAL_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_PERSONAL),$(atexts)/mimetype)


$(call gb_CustomTarget_get_target,extras/source/templates/personal) : \
	$(foreach atexts,$(extras_TEMPLATES_PERSONAL),$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/$(atexts).ott)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%/mimetype : $(SRCDIR)/extras/source/templates/personal/%/mimetype
	$(call gb_Output_announce,templates/personal/$*/mimetype,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*/mimetype,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*/mimetype,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.jpg : $(SRCDIR)/extras/source/templates/personal/%.jpg
	$(call gb_Output_announce,templates/personal/$*.jpg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*.jpg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*.jpg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.png : $(SRCDIR)/extras/source/templates/personal/%.png
	$(call gb_Output_announce,templates/personal/$*.png,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*.png,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*.png,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.rdf : $(SRCDIR)/extras/source/templates/personal/%.rdf
	$(call gb_Output_announce,templates/personal/$*.rdf,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*.rdf,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*.rdf,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.svg : $(SRCDIR)/extras/source/templates/personal/%.svg
	$(call gb_Output_announce,templates/personal/$*.svg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*.svg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*.svg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.svm : $(SRCDIR)/extras/source/templates/personal/%.svm
	$(call gb_Output_announce,templates/personal/$*.svm,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*.svm,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*.svm,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.xml : $(SRCDIR)/extras/source/templates/personal/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,templates/personal/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/personal/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/personal/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.ott :
	$(call gb_Output_announce,templates/personal/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/personal/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_PERSONAL_DIR) && \
		zip -q0X --filesync --must-match $@ $(EXTRAS_PERSONAL_MIMEFILES_FILTER) && \
		zip -qrX --must-match $@ $(EXTRAS_PERSONAL_XMLFILES_FILTER) \
	)
	$(call gb_Trace_EndRange,templates/personal/$*.ott,ZIP)

define extras_Tplpersonal_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tplpersonal_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_PERSONAL_MIMETYPEFILES) $(extras_PERSONAL_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PERSONAL_MIMEFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_PERSONAL_MIMETYPEFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PERSONAL_XMLFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_PERSONAL_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PERSONAL_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_PERSONAL_MIMETYPEFILES) $(extras_PERSONAL_XMLFILES),\
	$(call extras_Tplpersonal_make_file_deps,extras/source/templates/personal,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_PERSONAL),\
	$(call extras_Tplpersonal_make_zip_deps,extras/source/templates/personal,$(atexts).ott,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
