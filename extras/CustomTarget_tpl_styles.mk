# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/styles))

extras_STYLES_XMLFILES := \
	Default/META-INF/manifest.xml \
	Default/meta.xml \
	Default/styles.xml \
	Default/Thumbnails/thumbnail.png \
	Modern/META-INF/manifest.xml \
	Modern/meta.xml \
	Modern/styles.xml \
	Modern/Thumbnails/thumbnail.png \
	Simple/META-INF/manifest.xml \
	Simple/meta.xml \
	Simple/styles.xml \
	Simple/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_STYLES_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_STYLES_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/% : $(SRCDIR)/extras/source/templates/styles/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/$$*).dir
	$(call gb_Output_announce,templates/styles/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/styles/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/styles/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.xml : $(SRCDIR)/extras/source/templates/styles/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/template/styles)/$$*.xml).dir
	$(call gb_Output_announce,templates/styles/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/styles/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/styles/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/styles)/$$*/,\
            mimetype $$(call extras_STYLES_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/styles/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/styles/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_STYLES_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/styles/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
