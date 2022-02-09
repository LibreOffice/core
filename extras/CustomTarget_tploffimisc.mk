# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/offimisc))

extras_OFFIMISC_XMLFILES := \
	Businesscard-with-logo/content.xml \
	Businesscard-with-logo/manifest.rdf \
	Businesscard-with-logo/META-INF/manifest.xml \
	Businesscard-with-logo/meta.xml \
	Businesscard-with-logo/Pictures/10000201000001F4000000A0108F3F06.png \
	Businesscard-with-logo/settings.xml \
	Businesscard-with-logo/styles.xml \
	Businesscard-with-logo/Thumbnails/thumbnail.png \

# param: style-base (e.g. Modern)
extras_OFFIMISC_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_OFFIMISC_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/% : $(SRCDIR)/extras/source/templates/offimisc/% \
        | $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/$$*).dir
	$(call gb_Output_announce,templates/offimisc/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/offimisc/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/offimisc/$*,CPY)

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.xml : $(SRCDIR)/extras/source/templates/offimisc/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/$$*.xml).dir
	$(call gb_Output_announce,templates/offimisc/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/offimisc/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/offimisc/$*.xml,XSL)

$(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/%.ott : \
        $$(addprefix $(call gb_CustomTarget_get_workdir,extras/source/templates/offimisc)/$$*/,\
            mimetype $$(call extras_OFFIMISC_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,templates/offimisc/$*.ott,$(true),ZIP,2)
	$(call gb_Trace_StartRange,templates/offimisc/$*.ott,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		zip -q0X --filesync --must-match $@ mimetype && \
		zip -qrX --must-match $@ $(call extras_OFFIMISC_XMLFILES_RELATIVE,$*) \
	)
	$(call gb_Trace_EndRange,templates/offimisc/$*.ott,ZIP)
extras_OFFIMISC_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_OFFIMISC),$(atexts)/mimetype)

# vim: set noet sw=4 ts=4:
