# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# AutoInstall class

$(dir $(call gb_AutoInstall_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_AutoInstall_get_target,%) : $(GBUILDDIR)/AutoInstall.mk \
		$(SRCDIR)/Repository.mk $(SRCDIR)/RepositoryExternal.mk \
		$(BUILDDIR)/config_host.mk \
		$(GBUILDDIR)/gen-autoinstall.py \
		$(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Output_announce,$*,$(true),AIN,3)
	SDKLIBFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
	   $(foreach lib,$(gb_SdkLinkLibrary_MODULE_$*),\
			$(lib) \
			$(notdir $(call gb_Library_get_sdk_link_lib,$(lib))) \
			../../program/$(call gb_Library_get_runtime_filename,$(lib)))) \
	&& LIBFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
		$(foreach lib,$(gb_Library_MODULE_$*),\
			$(lib) \
			$(call gb_Library_get_runtime_filename,$(lib)))) \
	&& EXEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
		$(foreach exe,$(gb_Executable_MODULE_$*),\
			$(exe) \
			$(call gb_Executable_get_filename,$(exe)))) \
	&& JARFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,$(gb_Jar_MODULE_$*)) \
	&& PKGFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,$(gb_Package_MODULE_$*)) \
	&& $(call gb_ExternalExecutable_get_command,python) \
			$(GBUILDDIR)/gen-autoinstall.py \
			'$*' '$(SCP2COMPONENTCONDITION)' \
			'$(SCP2LIBTEMPLATE)' '$(SCP2EXETEMPLATE)' '$(SCP2JARTEMPLATE)' \
			'$(SCP2PKGTEMPLATE)' \
			$${SDKLIBFILE} $${LIBFILE} $${EXEFILE} $${JARFILE} $${PKGFILE} \
			> $@ \
	&& rm -f $${SDKLIBFILE} $${LIBFILE} $${EXEFILE} $${JARFILE} $${PKGFILE}


$(call gb_AutoInstall_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),AIL,3)
	rm -f $(call gb_AutoInstall_get_target,$*)

define gb_AutoInstall_AutoInstall
$(call gb_AutoInstall_get_target,all) :| $(dir $(call gb_AutoInstall_get_target,all)).dir

$$(eval $$(call gb_Module_register_target,AutoInstall_$(1),$(call gb_AutoInstall_get_target,all),$(call gb_AutoInstall_get_clean_target,all)))
$(call gb_Helper_make_userfriendly_targets,all,AutoInstall)

endef

# gb_AutoInstall_add_module module lib_template exe_template jar_template package_template componentcondition
define gb_AutoInstall_add_module
$(call gb_AutoInstall_get_target,all) : $(call gb_AutoInstall_get_target,$(1))
$(call gb_AutoInstall_get_clean_target,all) : $(call gb_AutoInstall_get_clean_target,$(1))
$(call gb_Helper_make_userfriendly_targets,$(1),AutoInstall)

$(call gb_AutoInstall_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_AutoInstall_get_target,$(1)) :| $(dir $(call gb_AutoInstall_get_target,$(1))).dir
$(call gb_AutoInstall_get_target,$(1)) : SCP2LIBTEMPLATE := $(2)
$(call gb_AutoInstall_get_target,$(1)) : SCP2EXETEMPLATE := $(3)
$(call gb_AutoInstall_get_target,$(1)) : SCP2JARTEMPLATE := $(4)
$(call gb_AutoInstall_get_target,$(1)) : SCP2PKGTEMPLATE := $(5)
$(call gb_AutoInstall_get_target,$(1)) : SCP2COMPONENTCONDITION := $(6)

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
