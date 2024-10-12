# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,filter/source/docbook))

filter_docbook_files := \
	DocBookTemplate/settings.xml \
	DocBookTemplate/manifest.rdf \


filter_docbook_files_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(filter_docbook_files)))

define run_zip_docbook_recipe =
$(call gb_Output_announce,$(subst $(gb_CustomTarget_workdir)/filter/source/docbook/,,$@),$(true),ZIP,2)
$(call gb_Trace_StartRange,$(subst $(gb_CustomTarget_workdir)/filter/source/docbook/,,$@),ZIP)
cd $(dir $<) && \
$(call gb_Helper_wsl_path,\
$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
$(WSL) zip -qrX --must-match $@ $(subst $(dir $<),,$^)) -x mimetype
$(call gb_Trace_EndRange,$(subst $(gb_CustomTarget_workdir)/filter/source/docbook/,,$@),ZIP)
endef

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(gb_CustomTarget_workdir)/filter/source/docbook/% : $(SRCDIR)/filter/source/docbook/% \
        | $$(dir $(gb_CustomTarget_workdir)/filter/source/docbook/$$*).dir
	$(call gb_Output_announce,docbook/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,docbook/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,docbook/$*,CPY)

# test and copy xml files
$(gb_CustomTarget_workdir)/filter/source/docbook/%.xml : $(SRCDIR)/filter/source/docbook/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/filter/source/docbook/$$*.xml).dir
	$(call gb_Output_announce,docbook/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,docbook/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,docbook/$*.xml,XSL)

$(gb_CustomTarget_workdir)/filter/source/docbook/%.ott : \
        $$(addprefix $(gb_CustomTarget_workdir)/filter/source/docbook/$$*/,\
            mimetype META-INF/manifest.xml styles.xml \
            $$(call filter_docbook_files_RELATIVE,$$*))
	$(run_zip_docbook_recipe)

# vim: set noet sw=4 ts=4:
