# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates))

include $(SRCDIR)/extras/template_files.mk

define run_zip_template_recipe =
$(call gb_Output_announce,$(subst $(gb_CustomTarget_workdir)/extras/source/,,$@),$(true),ZIP,2)
$(call gb_Trace_StartRange,$(subst $(gb_CustomTarget_workdir)/extras/source/,,$@),ZIP)
cd $(dir $<) && \
$(call gb_Helper_wsl_path,\
$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
$(WSL) zip -qrX --must-match $@ $(subst $(dir $<),,$^))
$(call gb_Trace_EndRange,$(subst $(gb_CustomTarget_workdir)/extras/source/,,$@),ZIP)
endef

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(gb_CustomTarget_workdir)/extras/source/templates/% : $(SRCDIR)/extras/source/templates/% \
        | $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/$$*).dir
	$(call gb_Output_announce,templates/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/$*,CPY)

# test and copy xml files
$(gb_CustomTarget_workdir)/extras/source/templates/%.xml : $(SRCDIR)/extras/source/templates/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/extras/source/templates/$$*.xml).dir
	$(call gb_Output_announce,templates/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/$*.xml,XSL)

$(gb_CustomTarget_workdir)/extras/source/templates/%.odt \
$(gb_CustomTarget_workdir)/extras/source/templates/%.otg \
$(gb_CustomTarget_workdir)/extras/source/templates/%.oth \
$(gb_CustomTarget_workdir)/extras/source/templates/%.otp \
$(gb_CustomTarget_workdir)/extras/source/templates/%.ots \
$(gb_CustomTarget_workdir)/extras/source/templates/%.ott : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/$$*/,\
            mimetype META-INF/manifest.xml content.xml meta.xml styles.xml \
            $$(call extra_files_TEMPLATES_RELATIVE,$$*))
	$(run_zip_template_recipe)

# special case for styles/Default,Modern,Simple - no content.xml for those
$(gb_CustomTarget_workdir)/extras/source/templates/styles/%.ott : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/templates/styles/$$*/,\
            mimetype META-INF/manifest.xml meta.xml styles.xml \
            $$(call extra_files_TEMPLATES_RELATIVE,styles/$$*))
	$(run_zip_template_recipe)

# vim: set noet sw=4 ts=4:
