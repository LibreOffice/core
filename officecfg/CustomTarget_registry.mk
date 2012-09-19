##
## Version: MPL 1.1 / GPLv3+ / LGPLv3+
##
## The contents of this file are subject to the Mozilla Public License Version
## 1.1 (the "License"); you may not use this file except in compliance with
## the License or as specified alternatively below. You may obtain a copy of
## the License at http://www.mozilla.org/MPL/
##
## Software distributed under the License is distributed on an "AS IS" basis,
## WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
## for the specific language governing rights and limitations under the
## License.
##
## Major Contributor(s):
## Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
## (initial developer)
##
## All Rights Reserved.
##
## For minor contributions see the git repository.
##
## Alternatively, the contents of this file may be used under the terms of
## either the GNU General Public License Version 3 or later (the "GPLv3+"), or
## the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
## in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
## instead of those above.
##

$(eval $(call gb_CustomTarget_CustomTarget,officecfg/registry))

$(call gb_CustomTarget_get_target,officecfg/registry) : \
	$(foreach i,officecfg_qa_allheaders $(officecfg_XCSFILES),\
		$(call gb_CustomTarget_get_workdir,officecfg/registry)/$(i).hxx)

# auto generated header file for unit test qa/cppheader.cxx
$(call gb_CustomTarget_get_workdir,officecfg/registry)/officecfg_qa_allheaders.hxx: \
		$(SRCDIR)/officecfg/registry/files.mk
	$(call gb_Output_announce,officecfg_qa_allheaders.hxx,$(true),CAT,1)
	mkdir -p $(dir $@) && \
	rm -f $@ \
	$(foreach file,$(officecfg_XCSFILES),\
		&& echo "#include <officecfg/$(file).hxx>" >> $@)

define officecfg_TARGET
$(call gb_CustomTarget_get_workdir,officecfg/registry)/$(if $(1),$(1)/$(if $(2),$(2)/))%.hxx: \
            $(SRCDIR)/officecfg/registry/schema/org/openoffice/$(if $(1),$(1)/$(if $(2),$(2)/))%.xcs \
            $(SRCDIR)/officecfg/registry/cppheader.xsl | $(gb_XSLTPROCTARGET)
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),$(true),XSL,1)
	$$(call gb_Helper_abbreviate_dirs, \
        mkdir -p $$(dir $$@) && \
        $$(gb_XSLTPROC) --nonet --stringparam ns1 \
            $(if $(1), \
                $(1) --stringparam ns2 $(if $(2),$(2) --stringparam ns3)) $$* \
            -o $$@ $(SRCDIR)/officecfg/registry/cppheader.xsl $$<)

endef

# Sort longer paths before their prefixes, as at least GNU Make 3.81 on Mac OS X
# appears to let % span sub-directories, so that the above rule would produce
# unexpected results; sorting this way seems to avoid the problem:
$(eval $(call officecfg_TARGET,Office,DataAccess))
$(eval $(call officecfg_TARGET,Office,OOoImprovement))
$(eval $(call officecfg_TARGET,Office,UI))
$(eval $(call officecfg_TARGET,Office))
$(eval $(call officecfg_TARGET,TypeDetection))
$(eval $(call officecfg_TARGET,ucb))
$(eval $(call officecfg_TARGET))
