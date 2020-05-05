# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/draw))

extras_TEMPLATES_DRAW := \
	bpmn \


extras_DRAW_XMLFILES := \
	bpmn/content.xml \
	bpmn/META-INF/manifest.xml \
	bpmn/meta.xml \
	bpmn/styles.xml \
	bpmn/Thumbnails/thumbnail.png \


extras_DRAW_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_DRAW),$(atexts)/mimetype)


$(call gb_CustomTarget_get_target,extras/source/templates/draw) : \
	$(foreach atexts,$(extras_TEMPLATES_DRAW),$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/$(atexts).ott)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%/mimetype : $(SRCDIR)/extras/source/templates/draw/%/mimetype
	$(call gb_Output_announce,templates/draw/$*/mimetype,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*/mimetype,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*/mimetype,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.jpg : $(SRCDIR)/extras/source/templates/draw/%.jpg
	$(call gb_Output_announce,templates/draw/$*.jpg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*.jpg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*.jpg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.png : $(SRCDIR)/extras/source/templates/draw/%.png
	$(call gb_Output_announce,templates/draw/$*.png,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*.png,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*.png,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.rdf : $(SRCDIR)/extras/source/templates/draw/%.rdf
	$(call gb_Output_announce,templates/draw/$*.rdf,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*.rdf,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*.rdf,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.svg : $(SRCDIR)/extras/source/templates/draw/%.svg
	$(call gb_Output_announce,templates/draw/$*.svg,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*.svg,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*.svg,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.svm : $(SRCDIR)/extras/source/templates/draw/%.svm
	$(call gb_Output_announce,templates/draw/$*.svm,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*.svm,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*.svm,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.xml : $(SRCDIR)/extras/source/templates/draw/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,templates/draw/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/draw/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/draw/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.ott :
	$(call gb_Output_announce,templates/draw/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/draw/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_DRAW_DIR) && \
		zip -q0X --filesync --must-match $@ $(EXTRAS_DRAW_MIMEFILES_FILTER) && \
		zip -qrX --must-match $@ $(EXTRAS_DRAW_XMLFILES_FILTER) \
	)
	$(call gb_Trace_EndRange,templates/draw/$*.ott,ZIP)

define extras_Tpldraw_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tpldraw_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_DRAW_MIMETYPEFILES) $(extras_DRAW_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_DRAW_MIMEFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_DRAW_MIMETYPEFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_DRAW_XMLFILES_FILTER := $(foreach file,$(filter $(3)/%,$(extras_DRAW_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_DRAW_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_DRAW_MIMETYPEFILES) $(extras_DRAW_XMLFILES),\
	$(call extras_Tpldraw_make_file_deps,extras/source/templates/draw,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_DRAW),\
	$(call extras_Tpldraw_make_zip_deps,extras/source/templates/draw,$(atexts).ott,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
