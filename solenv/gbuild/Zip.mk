#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# Zip class

gb_Zip_ZIPCOMMAND := zip

# remove zip file in workdir and outdir
$(call gb_Zip_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),ZIP,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Zip_get_target,$*) && \
		rm -f $(call gb_Zip_get_final_target,$*) && \
		rm -f $(call gb_Zip_get_outdir_target,$*))

# rule to create zip package in workdir
# -FS makes sure that all files in the zip package will be removed that no longer are in $(FILES)
$(call gb_Zip_get_target,%) :
	$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(call gb_Zip_get_target,$*)) && \
	cd $(LOCATION) && $(gb_Zip_ZIPCOMMAND) -rX -FS $(call gb_Zip_get_target,$*) $(FILES) )

# the final target is a touch target; we use it as registered targets should be in workdir, not in outdir
# the outdir target depends on the workdir target and is built by delivering the latter
# the workdir target is created by cd'ing to the target directory and adding/updating the files
$(call gb_Zip_get_final_target,%) : $(call gb_Zip_get_outdir_target,%)
	touch $@

# clear file list, set location (zipping uses relative paths)
# register target and clean target
# add deliverable
# add dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
# the zip package target requires that all added files have a common root directory (package location)
# names of added files are relative to it; the zip will store them with their complete relative path name
# the location can't be stored in a scoped variable as it is needed in the add_file macro (see rule above)
define gb_Zip_Zip
$(call gb_Zip_get_target,$(1)) : FILES :=
$(call gb_Zip_get_target,$(1)) : LOCATION := $(2)
gb_Package_Location_$(1) := $(2)
$(eval $(call gb_Module_register_target,$(call gb_Zip_get_final_target,$(1)),$(call gb_Zip_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Zip_get_outdir_target,$(1)),$(call gb_Zip_get_target,$(1)),$(1))
$(call gb_Zip_get_outdir_target,$(1)) : $(call gb_Zip_get_target,$(1))

endef

# adding a file creates a dependency to it
# the full path name of the file needs access to the package location
# as scoped variables only exist in rules, we use a postfixed name to refer to the location
define gb_Zip_add_file
$(call gb_Zip_get_target,$(1)) : FILES += $(2)
$(call gb_Zip_get_target,$(1)) : $(gb_Package_Location_$(1))/$(2)

endef

define gb_Zip_add_files
$(foreach file,$(2),$(call gb_Zip_add_file,$(1),$(file)))
endef

# vim: set noet sw=4 ts=4:
