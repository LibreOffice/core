# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/draw))

extras_DRAW_XMLFILES := \
	bpmn/content.xml \
	bpmn/META-INF/manifest.xml \
	bpmn/meta.xml \
	bpmn/styles.xml \
	bpmn/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_DRAW_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_DRAW_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/% : $(SRCDIR)/extras/source/templates/draw/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/$$*).dir
	$(call gb_Output_announce,templates/draw/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/draw/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/draw/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.xml : $(SRCDIR)/extras/source/templates/draw/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/$$*.xml).dir
	$(call gb_Output_announce,templates/draw/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/draw/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/draw/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/%.otg : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/draw)/$$*/,\
            mimetype $$(call extras_DRAW_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/draw/$*.otg,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/draw/$*.otg,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_DRAW_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/draw/$*.otg,ZIP)

# vim: set noet sw=4 ts=4:
