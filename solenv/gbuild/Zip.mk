###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



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
