# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/personal))

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

# param: style-base (e.g. Modern)
extras_PERSONAL_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_PERSONAL_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/% : $(SRCDIR)/extras/source/templates/personal/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/$$*).dir
	$(call gb_Output_announce,templates/personal/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/personal/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/personal/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.xml : $(SRCDIR)/extras/source/templates/personal/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/$$*.xml).dir
	$(call gb_Output_announce,templates/personal/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/personal/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/personal/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/personal)/$$*/,\
            mimetype $$(call extras_PERSONAL_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/personal/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/personal/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_PERSONAL_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/personal/$*.ott,ZIP)

# vim: set noet sw=4 ts=4:
