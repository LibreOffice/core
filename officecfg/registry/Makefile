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

WFDIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
gb_PARTIALBUILD := T
include $(GBUILDDIR)/gbuild_simple.mk
include $(WFDIR)/files.mk

.DEFAULT_GOAL := all
.PHONY: all
all: $(foreach i,$(officecfg_FILES),$(i).hxx)

define my_target
$(if $(1),$(1)/$(if $(2),$(2)/))%.hxx: \
            $(WFDIR)/schema/org/openoffice/$(if $(1),$(1)/$(if $(2),$(2)/))%.xcs \
            $(WFDIR)/cppheader.xsl
	$$(call gb_Helper_abbreviate_dirs_native,mkdir -p $$(dir $$@))
	$$(call gb_Helper_abbreviate_dirs_native, \
            $$(gb_XSLTPROC) --nonet --stringparam ns1 \
            $(if $(1), \
                $(1) --stringparam ns2 $(if $(2),$(2) --stringparam ns3)) $$* \
            -o $$@ $$(WFDIR)/cppheader.xsl $$<)

endef

# Sort longer paths before their prefixes, as at least GNU Make 3.81 on Mac OS X
# appears to let % span sub-directories, so that the above rule would produce
# unexpected results; sorting this way seems to avoid the problem:
$(eval $(call my_target,Office,DataAccess))
$(eval $(call my_target,Office,OOoImprovement))
$(eval $(call my_target,Office,UI))
$(eval $(call my_target,Office))
$(eval $(call my_target,TypeDetection))
$(eval $(call my_target,ucb))
$(eval $(call my_target))
