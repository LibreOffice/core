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

OCDIR := $(SRCDIR)/officecfg/registry
include $(OCDIR)/files.mk

$(eval $(call gb_CustomTarget_CustomTarget,officecfg/registry,new_style))

OCRG := $(call gb_CustomTarget_get_workdir,officecfg/registry)

$(call gb_CustomTarget_get_target,officecfg/registry) : \
	$(foreach i,$(officecfg_FILES),$(OCRG)/$(i).hxx)

define oc_target
$(OCRG)/$(if $(1),$(1)/$(if $(2),$(2)/))%.hxx: \
            $(OCDIR)/schema/org/openoffice/$(if $(1),$(1)/$(if $(2),$(2)/))%.xcs \
            $(OCDIR)/cppheader.xsl | $(gb_XSLTPROCTARGET)
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),$(true),XSL,1)
	$$(call gb_Helper_abbreviate_dirs_native, \
        mkdir -p $$(dir $$@) && \
        $$(gb_XSLTPROC) --nonet --stringparam ns1 \
            $(if $(1), \
                $(1) --stringparam ns2 $(if $(2),$(2) --stringparam ns3)) $$* \
            -o $$@ $(OCDIR)/cppheader.xsl $$<)

endef

# Sort longer paths before their prefixes, as at least GNU Make 3.81 on Mac OS X
# appears to let % span sub-directories, so that the above rule would produce
# unexpected results; sorting this way seems to avoid the problem:
$(eval $(call oc_target,Office,DataAccess))
$(eval $(call oc_target,Office,OOoImprovement))
$(eval $(call oc_target,Office,UI))
$(eval $(call oc_target,Office))
$(eval $(call oc_target,TypeDetection))
$(eval $(call oc_target,ucb))
$(eval $(call oc_target))
