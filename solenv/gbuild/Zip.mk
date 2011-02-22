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

# clear file list, set location (zipping uses relative paths)
# register target and clean target
# add deliverable
define gb_Zip_Zip

$(call gb_Zip_get_target,$(1)) : FILES :=
$(call gb_Zip_get_target,$(1)) : LOCATION := $(2)
gb_Package_Location_$(1) := $(2)

$(call gb_Zip_get_final_target,$(1)) : $(call gb_Zip_get_outdir_target,$(1))
	touch $$@

$(call gb_Zip_get_outdir_target,$(1)) : $(call gb_Zip_get_target,$(1))
	$(call gb_Helper_abbreviate_dirs,\
	$$(call gb_Deliver_deliver,$$<,$$@))

$(eval $(call gb_Module_register_target,$(call gb_Zip_get_final_target,$(1)),$(call gb_Zip_get_clean_target,$(1))))

$(call gb_Zip_get_target,$(1)) :
	$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $$(dir $$(call gb_Zip_get_target,$(1))) && \
	cd $$(LOCATION) && $$(gb_Zip_ZIPCOMMAND) -rX -FS $$(call gb_Zip_get_target,$(1)) $$(FILES) )

$(call gb_Deliver_add_deliverable,$(call gb_Zip_get_outdir_target,$(1)),$(call gb_Zip_get_target,$(1)))

endef

define gb_Zip_add_file
$(call gb_Zip_get_target,$(1)) : FILES += $(2)
$(call gb_Zip_get_target,$(1)) : $(gb_Package_Location_$(1))/$(2)

endef

define gb_Zip_add_files
$(foreach file,$(2),$(call gb_Zip_add_file,$(1),$(file)))
endef

# vim: set noet sw=4 ts=4:
