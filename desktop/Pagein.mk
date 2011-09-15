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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

desktop_Pagein_get_target = $(WORKDIR)/Pagein/desktop/$(1)
desktop_Pagein_get_clean_target = $(WORKDIR)/Clean/Pagein/desktop/$(1)
desktop_Pagein_get_outdir_target = $(OUTDIR)/bin/pagein-$(1)

desktop_Pagein__istype = $(findstring $(2),$(call desktop_Pagein__prefix,$(1)))
desktop_Pagein__prefix = $(firstword $(subst :, ,$(1)))
desktop_Pagein__suffix = $(lastword $(subst :, ,$(1)))
desktop_Pagein__object = $(call desktop_Pagein__suffix,$(1))
desktop_Pagein__dir = $(call desktop_Pagein__prefix,$(1))
desktop_Pagein__libname = $(notdir $(call gb_Library_get_target,$(call desktop_Pagein__suffix,$(1))))
desktop_Pagein__libpath = $(call desktop_Pagein__dir,$(1))/$(call desktop_Pagein__libname,$(1))

desktop_Pagein__make_path = \
$(if $(call desktop_Pagein__istype,$(1),OBJ),\
    $(call desktop_Pagein__object,$(1)),\
    $(if $(call desktop_Pagein__istype,$(1),LIB),\
        $(call desktop_Pagein__libname,$(1)),\
        $(call desktop_Pagein__libpath,$(1))))

define desktop_Pagein__command
$(call gb_Output_announce,$(2),$(true),PAG,5)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && rm -f $(1) \
    $(foreach object,$(OBJECTS),&& echo $(call desktop_Pagein__make_path,$(object)) >> $(1)))

endef

.PHONY : $(call desktop_Pagein_get_clean_target,%)
$(call desktop_Pagein_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PAG,5)
	$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call desktop_Pagein_get_target,$*) $(call desktop_Pagein_get_outdir_target,$*))

$(call desktop_Pagein_get_target,%) :
	$(call desktop_Pagein__command,$@,$*,$^)
	
$(call desktop_Pagein_get_outdir_target,%) : $(call desktop_Pagein_get_target,%)
	$(call gb_Deliver_deliver,$<,$@)

define desktop_Pagein_Pagein
$(call desktop_Pagein_get_target,$(1)) : OBJECTS :=
$$(eval $$(call gb_Module_register_target,$(call desktop_Pagein_get_outdir_target,$(1)),$(call desktop_Pagein_get_clean_target,$(1))))
$(call desktop_Pagein_get_outdir_target,$(1)) : $(call desktop_Pagein_get_target,$(1))

endef

define desktop_Pagein_add_lib
$(call desktop_Pagein_get_target,$(1)) : OBJECTS += LIB:$(2)

endef

define desktop_Pagein_add_lib_with_dir
$(call desktop_Pagein_get_target,$(1)) : OBJECTS += $(strip $(3)):$(2)

endef

define desktop_Pagein_add_object
$(call desktop_Pagein_get_target,$(1)) : OBJECTS += OBJ:$(2)

endef

define desktop_Pagein_add_libs
$(foreach lib,$(2),$(call desktop_Pagein_add_lib,$(1),$(lib)))

endef

define desktop_Pagein_add_libs_with_dir
$(foreach lib,$(2),$(call desktop_Pagein_add_lib_with_dir,$(1),$(lib),$(3)))

endef

define desktop_Pagein_add_objects
$(foreach object,$(2),$(call desktop_Pagein_add_object,$(1),$(object)))

endef

# vim: set ts=4 sw=4 noet:
