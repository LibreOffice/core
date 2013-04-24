# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,officecfg/registry))

$(call gb_CustomTarget_get_target,officecfg/registry) : \
	$(foreach i,officecfg_qa_allheaders $(officecfg_XCSFILES),\
		$(call gb_CustomTarget_get_workdir,officecfg/registry)/$(i).hxx)

# auto generated header file for unit test qa/cppheader.cxx
$(call gb_CustomTarget_get_workdir,officecfg/registry)/officecfg_qa_allheaders.hxx: \
		$(SRCDIR)/officecfg/files.mk
	$(call gb_Output_announce,officecfg_qa_allheaders.hxx,$(true),CAT,1)
	mkdir -p $(dir $@) && \
	rm -f $@ \
	$(foreach file,$(officecfg_XCSFILES),\
		&& echo "#include <officecfg/$(file).hxx>" >> $@)

# pass the stem as space separated path elements and get a set of --stringparam ns<level> <element> in return
officecfg_xsltparams=$(if $(filter-out $(lastword $1),$1),$(call officecfg_xsltparams,$(filter-out $(lastword $1),$1))) --stringparam ns$(words $1) $(lastword $1)

$(call gb_CustomTarget_get_workdir,officecfg/registry)/%.hxx: \
            $(SRCDIR)/officecfg/registry/schema/org/openoffice/%.xcs \
            $(SRCDIR)/officecfg/registry/cppheader.xsl\
	    | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Helper_abbreviate_dirs, \
	mkdir -p $(dir $@) && \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet $(call officecfg_xsltparams,$(subst /, ,$*)) \
	    -o $@ $(SRCDIR)/officecfg/registry/cppheader.xsl $<)

# vim: set noet sw=4 ts=4:
