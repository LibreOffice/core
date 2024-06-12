# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# AutoInstall class

$(gb_AutoInstall_targetdir)/% : $(GBUILDDIR)/AutoInstall.mk \
        $(SRCDIR)/Repository.mk $(SRCDIR)/RepositoryExternal.mk \
        $(BUILDDIR)/config_host.mk \
        $(GBUILDDIR)/gen-autoinstall.py \
        $(call gb_ExternalExecutable_get_dependencies,python) \
        | $(gb_AutoInstall_targetdir)/.dir
	$(call gb_Output_announce,$*,$(true),AIN,3)
	$(call gb_Trace_StartRange,$*,AIN)
	SDKLIBFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),\
	   $(foreach lib,$(gb_SdkLinkLibrary_MODULE_$*),\
			$(lib) \
			$(notdir $(call gb_Library_get_sdk_link_lib,$(lib))) \
			../../program/$(call gb_Library_get_runtime_filename,$(lib)))) \
	&& LIBFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),\
		$(foreach lib,$(gb_Library_MODULE_$*),\
			$(lib) \
			$(call gb_Library_get_runtime_filename,$(lib)))) \
	&& EXEFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),\
		$(foreach exe,$(gb_Executable_MODULE_$*),\
			$(exe) \
			$(call gb_Executable_get_filename,$(exe)))) \
	&& JARFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_Jar_MODULE_$*)) \
	&& PKGFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_Package_MODULE_$*)) \
	&& $(call gb_ExternalExecutable_get_command,python) \
			$(GBUILDDIR)/gen-autoinstall.py \
			'$*' '$(SCP2COMPONENTCONDITION)' \
			'$(SCP2LIBTEMPLATE)' '$(SCP2EXETEMPLATE)' '$(SCP2JARTEMPLATE)' \
			'$(SCP2PKGTEMPLATE)' \
			$${SDKLIBFILE} $${LIBFILE} $${EXEFILE} $${JARFILE} $${PKGFILE} \
			> $@ \
	&& rm -f $${SDKLIBFILE} $${LIBFILE} $${EXEFILE} $${JARFILE} $${PKGFILE}
	$(call gb_Trace_EndRange,$*,AIN)


$(call gb_AutoInstall_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),AIL,3)
	rm -f $(gb_AutoInstall_targetdir)/$*

define gb_AutoInstall_AutoInstall
$$(eval $$(call gb_Module_register_target,$(gb_AutoInstall_targetdir)/all,$(call gb_AutoInstall_get_clean_target,all)))
$(call gb_Helper_make_userfriendly_targets,all,AutoInstall,$(gb_AutoInstall_targetdir)/all)

endef

# gb_AutoInstall_add_module module lib_template exe_template jar_template package_template componentcondition
define gb_AutoInstall_add_module
$(gb_AutoInstall_targetdir)/all : $(gb_AutoInstall_targetdir)/$(1)
$(call gb_AutoInstall_get_clean_target,all) : $(call gb_AutoInstall_get_clean_target,$(1))
$(call gb_Helper_make_userfriendly_targets,$(1),AutoInstall,$(gb_AutoInstall_targetdir)/$(1))

$(gb_AutoInstall_targetdir)/$(1) : $(gb_Module_CURRENTMAKEFILE)
$(gb_AutoInstall_targetdir)/$(1) : SCP2LIBTEMPLATE := $(2)
$(gb_AutoInstall_targetdir)/$(1) : SCP2EXETEMPLATE := $(3)
$(gb_AutoInstall_targetdir)/$(1) : SCP2JARTEMPLATE := $(4)
$(gb_AutoInstall_targetdir)/$(1) : SCP2PKGTEMPLATE := $(5)
$(gb_AutoInstall_targetdir)/$(1) : SCP2COMPONENTCONDITION := $(6)

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
