# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Zip class

gb_Zip__get_preparation_target = $(WORKDIR)/Zip/$(1).prepare

gb_Zip_ZIPCOMMAND := zip $(if $(findstring s,$(MAKEFLAGS)),-q)

# remove zip file in workdir and outdir
$(call gb_Zip_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),ZIP,3)
	$(call gb_Helper_abbreviate_dirs,\
		$(if $(CLEAR_LOCATION),rm -rf $(gb_Package_Location_$*) &&) \
		$(if $(INSTALL_NAME),rm -f $(INSTALL_NAME) &&) \
		rm -f $(call gb_Zip_get_target,$*) && \
		rm -f $(call gb_Zip__get_preparation_target,$*))

$(dir $(call gb_Zip_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Zip_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

# rule to create zip package in workdir
# --filesync makes sure that all files in the zip package will be removed that no longer are in $(FILES)
# if there are no files, zip fails; copy empty zip file to target in that case
$(call gb_Zip_get_target,%) :
	$(call gb_Output_announce,$*,$(true),ZIP,3)
	$(call gb_Trace_StartRange,$*,ZIP)
	$(call gb_Helper_abbreviate_dirs,\
		$(if $(FILES),\
			RESPONSEFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),\
                        $(FILES)) && \
			cd $(LOCATION) && \
			cat $${RESPONSEFILE} | tr "[:space:]" "\n" | \
				$(call gb_Helper_wsl_path,$(WSL) $(gb_Zip_ZIPCOMMAND) -@rX --filesync --must-match \
					$(call gb_Zip_get_target,$*)) && \
			rm -f $${RESPONSEFILE} && \
			touch $@\
		,	cp $(SRCDIR)/solenv/gbuild/empty.zip $@)\
		$(if $(INSTALL_NAME),&& cp $(call gb_Zip_get_target,$*) $(INSTALL_NAME)) \
	)
	$(call gb_Trace_EndRange,$*,ZIP)

# the preparation target is here to ensure proper ordering of actions in cases
# when we want to, e.g., create a zip from files created by a custom target
$(call gb_Zip__get_preparation_target,%) :
	touch $@

# clear file list, set location (zipping uses relative paths)
# register target and clean target
# add deliverable
# add dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
# the zip package target requires that all added files have a common root directory (package location)
# names of added files are relative to it; the zip will store them with their complete relative path name
# the location can't be stored in a scoped variable as it is needed in the add_file macro (see rule above)
define gb_Zip_Zip_internal_nodeliver
$(call gb_Zip_get_target,$(1)) : FILES :=
$(call gb_Zip_get_target,$(1)) : INSTALL_NAME :=
$(call gb_Zip_get_target,$(1)) : LOCATION := $(2)
$(call gb_Zip_get_target,$(1)) :| $(dir $(call gb_Zip_get_target,$(1))).dir
$(call gb_Zip__get_preparation_target,$(1)) :| $(dir $(call gb_Zip__get_preparation_target,$(1))).dir
$(call gb_Zip_get_clean_target,$(1)) : CLEAR_LOCATION :=
$(call gb_Zip_get_clean_target,$(1)) : INSTALL_NAME :=
$(eval gb_Package_Location_$(1) := $(2))

endef

define gb_Zip_Zip_internal
$(call gb_Zip_Zip_internal_nodeliver,$(1),$(2))

endef

# depend on makefile to enforce a rebuild if files are removed from the zip
define gb_Zip_Zip
$(call gb_Zip_Zip_internal,$(1),$(2))
$(call gb_Zip_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)

$(eval $(call gb_Module_register_target,$(call gb_Zip_get_target,$(1)),$(call gb_Zip_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Zip,$(call gb_Zip_get_target,$(1)))

endef

# adding a file creates a dependency to it
# the full path name of the file needs access to the package location
# as scoped variables only exist in rules, we use a postfixed name to refer to the location
#
# if package location is in $(WORKDIR) we can specify third parameter and copy file from different place
# then we need also remove the location on make clean
define gb_Zip_add_file
$(call gb_Zip_get_target,$(1)) : FILES += $(2)
$(call gb_Zip_get_target,$(1)) : $(gb_Package_Location_$(1))/$(2)
$(gb_Package_Location_$(1))/$(2) :| $(call gb_Zip__get_preparation_target,$(1))
ifneq ($(3),)
$(call gb_Zip_get_clean_target,$(1)) : CLEAR_LOCATION := TRUE
$(gb_Package_Location_$(1))/$(2) : $(3)
	mkdir -p $$(dir $$@)
	cp -f $$< $$@

endif

endef

# add additional dependency that must exist before the archive can be created
define gb_Zip_add_dependency
$(call gb_Zip__get_preparation_target,$(1)) :| $(2)

endef

define gb_Zip_add_files
$(foreach file,$(2),$(call gb_Zip_add_file,$(1),$(file)))
endef

define gb_Zip_add_dependencies
$(foreach dependency,$(2),$(call gb_Zip_add_dependency,$(1),$(dependency)))

endef

define gb_Zip_add_commandoptions
$(call gb_Zip_get_target,$(1)) : gb_Zip_ZIPCOMMAND += $(2)

endef

define gb_Zip_set_install_name
$(call gb_Zip_get_target,$(1)) : INSTALL_NAME := $(2)
$(call gb_Zip_get_target,$(1)) :| $(dir $(2)).dir
$(call gb_Zip_get_clean_target,$(1)) : INSTALL_NAME := $(2)

endef

define gb_Zip_use_unpacked
$(call gb_Zip__get_preparation_target,$(1)) \
	:| $(call gb_UnpackedTarball_get_final_target,$(2))

endef

# vim: set noet sw=4 ts=4:
