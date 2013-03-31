# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

gb_Pagein_UREPATH := ../ure-link/lib/

gb_Pagein__is_library = $(filter $(1),$(gb_Library_KNOWNLIBS))

gb_Pagein__get_libdir = $(if $(filter URELIB,$(call gb_Library_get_layer,$(1))),$(call gb_Pagein_UREPATH))

gb_Pagein__make_library_path = $(call gb_Pagein__get_libdir,$(1))$(call gb_Library_get_runtime_filename,$(1))

gb_Pagein__make_path = \
$(if $(call gb_Pagein__is_library,$(1)),$(call gb_Pagein__make_library_path,$(1)),$(1))

define gb_Pagein__command
$(call gb_Output_announce,$(2),$(true),PAG,5)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && rm -f $(1) \
    && touch $(1) \
    $(foreach object,$(OBJECTS),&& echo $(call gb_Pagein__make_path,$(object)) >> $(1)))

endef

.PHONY : $(call gb_Pagein_get_clean_target,%)
$(call gb_Pagein_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PAG,5)
	$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_Pagein_get_target,$*) $(call gb_Pagein_get_outdir_target,$*))

$(call gb_Pagein_get_target,%) :
	$(call gb_Pagein__command,$@,$*,$^)
	
$(call gb_Pagein_get_outdir_target,%) : $(call gb_Pagein_get_target,%)
	$(call gb_Deliver_deliver,$<,$@)

define gb_Pagein_Pagein
$(call gb_Pagein_get_target,$(1)) : OBJECTS :=
$(call gb_Pagein_get_target,$(1)) : $(realpath $(lastword $(MAKEFILE_LIST)))
$$(eval $$(call gb_Module_register_target,$(call gb_Pagein_get_outdir_target,$(1)),$(call gb_Pagein_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Pagein,$(call gb_Pagein_get_outdir_target,$(1)))
$(call gb_Pagein_get_outdir_target,$(1)) : $(call gb_Pagein_get_target,$(1))
$(call gb_Pagein_get_outdir_target,$(1)) :| $(dir $(call gb_Pagein_get_outdir_target,$(1))).dir

endef

define gb_Pagein_add_object
$(call gb_Pagein_get_target,$(1)) : OBJECTS += $(filter-out $(gb_MERGEDLIBS),$(2))

endef

define gb_Pagein_add_objects
$(foreach object,$(2),$(call gb_Pagein_add_object,$(1),$(object)))

endef

# vim: set ts=4 sw=4 noet:
