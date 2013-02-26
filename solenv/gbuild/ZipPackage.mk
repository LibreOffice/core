# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# ZipPackage class
# this is a bit of a hack, hopefully needed only temporarily because
# scp2 can easily deal with zip files but not so easily with 100s of
# individual files; ideally the Package itself should be sufficient.

# sigh... WTF does that scp2/installer look for these in bin and not pck???
gb_Zip_get_outdir_bin_target = $(OUTDIR)/bin/$(1).zip

define gb_ZipPackage_ZipPackage
$(call gb_Package_Package,$(1),$(2))
$(call gb_Zip_Zip_internal,$(1),$(WORKDIR)/Zip/$(1))
$(call gb_Package_get_target,$(1)) : $(call gb_Zip_get_outdir_bin_target,$(1))
$(call gb_Zip_get_outdir_bin_target,$(1)) : $(call gb_Zip_get_target,$(1))
	cp $$< $$@
$(call gb_Package_get_clean_target,$(1)) : $(call gb_Zip_get_clean_target,$(1))

endef

# sadly Package delivers to inc/ but ODK path is include/ so need to
# copy files twice
# $(1) package
# $(2) dir in package
# $(3) dir in zip
# $(4) file list
define gb_ZipPackage_add_files
$(call gb_Package_add_files,$(1),$(2),$(4))
$(foreach file,$(4),$(call gb_Zip_add_file,$(1),$(3)/$(notdir $(file)),$(gb_Package_SOURCEDIR_$(1))/$(file)))

endef

# vim: set noet sw=4 ts=4:
