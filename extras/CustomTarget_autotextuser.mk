# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/autotext/user))

extras_AUTOTEXTUSER_XMLFILES := \
	mytexts/BlockList.xml \
	mytexts/META-INF/manifest.xml \

# param: autotext-base (e.g. mytexts)
extras_AUTOTEXTUSER_XMLFILES_RELATIVE = $(subst $(1)/,,$(filter $(1)/%,$(extras_AUTOTEXTUSER_XMLFILES)))

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion withdifferent number of elements
$(gb_CustomTarget_workdir)/extras/source/autotext/user/%/mimetype : \
        | $$(dir $(gb_CustomTarget_workdir)/extras/source/autotext/user/$$*/mimetype).dir
	$(call gb_Output_announce,autotext/user/$*/mimetype,$(true),TCH,1)
	$(call gb_Trace_StartRange,autotext/user/$*/mimetype,TCH)
	touch $@
	$(call gb_Trace_EndRange,autotext/user/$*/mimetype,TCH)

$(gb_CustomTarget_workdir)/extras/source/autotext/user/%.xml : $(SRCDIR)/extras/source/autotext/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/extras/source/autotext/user/$$*.xml).dir
	$(call gb_Output_announce,autotext/user/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,autotext/user/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,autotext/user/$*.xml,XSL)

$(gb_CustomTarget_workdir)/extras/source/autotext/user/%.bau : \
        $$(addprefix $(gb_CustomTarget_workdir)/extras/source/autotext/user/$$*/,\
            mimetype $$(call extras_AUTOTEXTUSER_XMLFILES_RELATIVE,$$*))
	$(call gb_Output_announce,autotext/user/$*.bau,$(true),ZIP,2)
	$(call gb_Trace_StartRange,autotext/user/$*.bau,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
	cd $(dir $<) && \
	$(call gb_Helper_wsl_path,\
		$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
		$(WSL) zip -qrX --must-match $@ $(call extras_AUTOTEXTUSER_XMLFILES_RELATIVE,$*)) \
	)
	$(call gb_Trace_EndRange,autotext/user/$*.bau,ZIP)

# vim: set noet sw=4 ts=4:
