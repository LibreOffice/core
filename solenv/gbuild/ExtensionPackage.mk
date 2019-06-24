# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# ExtensionPackage class
#
# Unpacks a binary .oxt file into the instdir extension directory and
# writes a file-list.

$(call gb_ExtensionPackage_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),OXP,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(INSTROOT)/$(LIBO_SHARE_FOLDER)/extensions/$* \
			$(call gb_ExtensionPackage_get_target,$*))

$(call gb_ExtensionPackage_get_preparation_target,%) :
	mkdir -p $(dir $@) && touch $@

$(call gb_ExtensionPackage_get_target,%) :
	$(call gb_Output_announce,$*,$(true),OXP,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) \
		&& rm -rf $(INSTROOT)/$(LIBO_SHARE_FOLDER)/extensions/$* \
		&& unzip -q $(ZIPFILE) -d $(INSTROOT)/$(LIBO_SHARE_FOLDER)/extensions/$* \
		&& zipinfo -1 $(ZIPFILE) | grep -v '/$$' \
			| sed s+^+$(INSTROOT)/$(LIBO_SHARE_FOLDER)/extensions/$*/+ > $@)

# call gb_ExtensionPackage_ExtensionPackage_internal,package,oxt
define gb_ExtensionPackage_ExtensionPackage_internal
$(if $(2),,$(call gb_Output_error,need 2 parameters))
$(call gb_ExtensionPackage_get_target,$(1)) : ZIPFILE := $(2)
$(call gb_ExtensionPackage_get_target,$(1)) : $(2)
$(call gb_ExtensionPackage_get_target,$(1)) :| \
	$(INSTROOT)/$(LIBO_SHARE_FOLDER)/extensions/.dir

endef

# call gb_ExtensionPackage_ExtensionPackage,package,oxt
define gb_ExtensionPackage_ExtensionPackage
$(call gb_ExtensionPackage_ExtensionPackage_internal,$(1),$(2))
$(2) : $(call gb_ExtensionPackage_get_preparation_target,$(1))
	touch $$@

$$(eval $$(call gb_Module_register_target,ExtensionPackage_$(1),$(call gb_ExtensionPackage_get_target,$(1)),$(call gb_ExtensionPackage_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),ExtensionPackage)

endef

# call gb_ExtensionPackage_use_external_project,package,externalproject
define gb_ExtensionPackage_use_external_project
$(call gb_ExtensionPackage_get_preparation_target,$(1)) : \
	$(call gb_ExternalProject_get_target,$(2))

endef

# ExtensionPackage class
#
# This is less boring than writing a dozen 1-line ExtensionPackage files.

$(call gb_ExtensionPackageSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),OXS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_ExtensionPackageSet_get_target,$*))

$(call gb_ExtensionPackageSet_get_target,%) :
	$(call gb_Output_announce,$*,$(true),OXS,3)
	mkdir -p $(dir $@) && touch $@

# call gb_ExtensionPackageSet_ExtensionPackageSet,set
define gb_ExtensionPackageSet_ExtensionPackageSet
$$(eval $$(call gb_Module_register_target,ExtensionPackage_$(1),$(call gb_ExtensionPackageSet_get_target,$(1)),$(call gb_ExtensionPackageSet_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),ExtensionPackageSet)

endef

# call gb_ExtensionPackageSet_add_extension,set,package,oxt-filename
define gb_ExtensionPackageSet_add_extension
$(call gb_ExtensionPackage_ExtensionPackage_internal,$(2),$(TARFILE_LOCATION)/$(3))

$(call gb_ExtensionPackageSet_get_target,$(1)) : \
	$(call gb_ExtensionPackage_get_target,$(2))
$(call gb_ExtensionPackageSet_get_clean_target,$(1)) : \
	$(call gb_ExtensionPackage_get_clean_target,$(2))

endef

# vim: set noet sw=4 ts=4:
