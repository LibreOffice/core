# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
	
gb_Pagein_get_install_target = $(INSTDIR)/$(gb_PROGRAMDIRNAME)/pagein-$(1)

define gb_Pagein_Pagein
$(call gb_Pagein_get_target,$(1)) : OBJECTS :=
$(call gb_Pagein_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$$(eval $$(call gb_Module_register_target,$(call gb_Pagein_get_outdir_target,$(1)),$(call gb_Pagein_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Pagein,$(call gb_Pagein_get_outdir_target,$(1)))
$(call gb_Deliver_add_deliverable,$(call gb_Pagein_get_outdir_target,$(1)),$(call gb_Pagein_get_target,$(1)),$(1))
$(call gb_Pagein_get_outdir_target,$(1)) : $(call gb_Pagein_get_target,$(1))
$(call gb_Pagein_get_outdir_target,$(1)) :| $(dir $(call gb_Pagein_get_outdir_target,$(1))).dir

$(call gb_Helper_install,$(call gb_Pagein_get_outdir_target,$(1)), \
	$(call gb_Pagein_get_install_target,$(1)), \
	$(call gb_Pagein_get_target,$(1)))

endef

define gb_Pagein_add_object
$(call gb_Pagein_get_target,$(1)) : OBJECTS += $(filter-out $(gb_URELIBS) $(gb_MERGEDLIBS),$(2))

endef

define gb_Pagein_add_objects
$(foreach object,$(2),$(call gb_Pagein_add_object,$(1),$(object)))

endef

# vim: set ts=4 sw=4 noet:
