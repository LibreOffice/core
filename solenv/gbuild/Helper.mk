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

gb_Helper_MISC := $(WORKDIR)/Misc

# general purpose phony target
gb_Helper_PHONY := $(gb_Helper_MISC)/PHONY

# general purpose empty dummy target
gb_Helper_MISCDUMMY := $(gb_Helper_MISC)/DUMMY

# target for reacting to changes in the list of configured languages
gb_Helper_LANGSTARGET := $(BUILDDIR)/config_$(gb_Side)_lang.mk.stamp

.PHONY : $(WORKDIR)/Misc/PHONY
$(gb_Helper_MISCDUMMY) :
	@mkdir -p $(dir $@) && touch $@

ifeq ($(SRCDIR),$(BUILDDIR))
define gb_Helper_abbreviate_dirs
S=$(SRCDIR) && \
$(subst $(SRCDIR)/,$$S/,I=$(INSTDIR)) && \
$(subst $(SRCDIR)/,$$S/,W=$(WORKDIR)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(INSTDIR)/,$$I/,$(subst $(WORKDIR)/,$$W/,$(1))))
endef
else
define gb_Helper_abbreviate_dirs
S=$(SRCDIR) && \
$(subst $(SRCDIR)/,$$S/,B=$(BUILDDIR)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,I=$(INSTDIR))) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,W=$(WORKDIR))) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,$(subst $(INSTDIR)/,$$I/,$(subst $(WORKDIR)/,$$W/,$(1)))))
endef
endif

define gb_Helper_abbreviate_dirs_native
$(call gb_Output_error,gb_Helper_abbreviate_dirs_native: use gb_Helper_abbreviate_dirs instead.)
endef

define gb_Helper_native_path
$(call gb_Output_error,gb_Helper_native_path: Do not use. Should not be necessary.)
endef

# cygwin seems to eat one backslash when executing command, thus replace with '\\'
define gb_Helper_windows_path
$(subst /,\\,$(1))
endef


ifeq ($(OS),WNT)
# path-replacement optimizations, instead of calling cygpath/wslpath all the time, just do it once
# and then do all other path-replacements without the need to spawn processes/shells for that
SRCDIR_CYG := $(shell cygpath -u $(SRCDIR))
BUILDDIR_CYG := $(shell cygpath -u $(BUILDDIR))
INSTDIR_CYG := $(shell cygpath -u $(INSTDIR))
WORKDIR_CYG := $(shell cygpath -u $(WORKDIR))
TARFILE_LOCATION_CYG := $(shell cygpath -u $(TARFILE_LOCATION))

define gb_Helper_cyg_path
$(subst $(TARFILE_LOCATION)/,$(TARFILE_LOCATION_CYG)/,$(subst $(SRCDIR)/,$(SRCDIR_CYG)/,$(subst $(BUILDDIR)/,$(BUILDDIR_CYG)/,$(subst $(INSTDIR)/,$(INSTDIR_CYG)/,$(subst $(WORKDIR)/,$(WORKDIR_CYG)/,$(1))))))
endef

ifneq ($(MSYSTEM),)
# paths to reach into windows realm from within wsl
SRCDIR_WSL := $(shell $(WSL) wslpath -u $(SRCDIR))
BUILDDIR_WSL := $(shell $(WSL) wslpath -u $(BUILDDIR))
INSTDIR_WSL := $(shell $(WSL) wslpath -u $(INSTDIR))
WORKDIR_WSL := $(shell $(WSL) wslpath -u $(WORKDIR))
TARFILE_LOCATION_WSL := $(shell $(WSL) wslpath -u $(TARFILE_LOCATION))

define gb_Helper_wsl_path
$(subst $(TARFILE_LOCATION)/,$(TARFILE_LOCATION_WSL)/,$(subst $(SRCDIR)/,$(SRCDIR_WSL)/,$(subst $(BUILDDIR)/,$(BUILDDIR_WSL)/,$(subst $(INSTDIR)/,$(INSTDIR_WSL)/,$(subst $(WORKDIR)/,$(WORKDIR_WSL)/,$(1))))))
endef
else
# not needed for cygwin
gb_Helper_wsl_path=$(1)
endif

else
# noop for non-Windows
gb_Helper_wsl_path=$(1)
gb_Helper_cyg_path=$(1)
endif

define gb_Helper_make_clean_target
gb_$(1)_get_clean_target = $(WORKDIR)/Clean/$(1)/$$(1)

endef

define gb_Helper_make_dep_target
gb_$(1)_get_dep_target = $(WORKDIR)/Dep/$(1)/$$(1).d

endef

define gb_Helper_make_clean_targets
$(foreach targettype,$(1),\
	$(call gb_Helper_make_clean_target,$(targettype)))

endef

define gb_Helper_make_dep_targets
$(foreach targettype,$(1),\
	$(call gb_Helper_make_dep_target,$(targettype)))

endef

# e.g. 'make CppunitTest_sw_macros_test'
#
# gb_Helper_make_userfriendly_targets target class build-target? clean-target?
define gb_Helper_make_userfriendly_targets
.PHONY: $(2) $(2)_$(1) $(2)_$(1).clean
$(2): $(2)_$(1)
$(2)_$(1) : $(if $(3),$(3),$(call gb_$(2)_get_target,$(1)))
$(2)_$(1).clean : $(if $(4),$(4),$(call gb_$(2)_get_clean_target,$(1)))

endef

define gb_Helper_init_registries
gb_Executable_VALIDGROUPS_INSTALLED := UREBIN SDK OOO
gb_Executable_VALIDGROUPS_NOTINSTALLED := NONE
gb_Executable_VALIDGROUPS := UREBIN SDK OOO NONE
gb_Library_VALIDGROUPS_INSTALLED := OOOLIBS PLAINLIBS_URE PLAINLIBS_OOO PRIVATELIBS_URE RTVERLIBS UNOVERLIBS PLAINLIBS_SHLXTHDL
gb_Library_VALIDGROUPS_NOTINSTALLED := PLAINLIBS_NONE PLAINLIBS_OXT EXTENSIONLIBS
gb_Library_VALIDGROUPS := OOOLIBS PLAINLIBS_URE PLAINLIBS_OOO PRIVATELIBS_URE RTVERLIBS UNOVERLIBS PLAINLIBS_SHLXTHDL PLAINLIBS_NONE PLAINLIBS_OXT EXTENSIONLIBS
gb_Jar_VALIDGROUPS_INSTALLED := URE OOO
gb_Jar_VALIDGROUPS_NOTINSTALLED := OXT NONE
gb_Jar_VALIDGROUPS := URE OOO OXT NONE

$$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(eval gb_Executable_$$(group) :=))
$$(foreach group,$$(gb_Library_VALIDGROUPS),$$(eval gb_Library_$$(group) :=))
$$(foreach group,$$(gb_Jar_VALIDGROUPS),$$(eval gb_Jar_$$(group) :=))

endef

define gb_Helper_collect_knownlibs
gb_Library_KNOWNLIBS := $$(foreach group,$$(gb_Library_VALIDGROUPS),$$(gb_Library_$$(group)))
gb_Executable_KNOWN := $$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(gb_Executable_$$(group)))
gb_Jar_KNOWN := $$(foreach group,$$(gb_Jar_VALIDGROUPS),$$(gb_Jar_$$(group)))
gb_Fuzzers_KNOWN := $$(filter %fuzzer,$$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(gb_Executable_$$(group))))

endef

define gb_Helper_process_executable_registrations
$(foreach group,$(gb_Executable_VALIDGROUPS),\
	$(foreach executable,$(gb_Executable_$(group)),\
		$(if $(filter-out undefined,$(origin gb_Executable__register_$(executable))),\
			$(call gb_Executable__register_$(executable)))))

endef

define gb_Helper__register_executables
$(foreach group,$(gb_Executable_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Executable_$(group))),\
   $(call gb_Output_error,gb_Helper_register_executables: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_executables: contains duplicates: $(2)))

gb_Executable_$(1) += $(2)

endef

# $(call gb_Helper_register_executables,layer,exes)
define gb_Helper_register_executables
ifeq ($$(filter $(1),$$(gb_Executable_VALIDGROUPS_NOTINSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for executables that are not installed. Valid groups are: $$(gb_Executable_VALIDGROUPS_NOTINSTALLED). Use gb_Helper_register_executables_for_install for installed executables.))
endif
$(call gb_Helper__register_executables,$(1),$(2))

endef

# $(call gb_Helper_register_executables_for_install,layer,installmodule,exes)
define gb_Helper_register_executables_for_install
$(if $(3),,$(call gb_Output_error,gb_Helper_register_executables_for_install: no executables - need 3 parameters))
ifeq ($$(filter $(1),$$(gb_Executable_VALIDGROUPS_INSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for installed executables. Valid groups are: $$(gb_Executable_VALIDGROUPS_INSTALLED). Use gb_Helper_register_executables for executables that are not installed.))
endif
$(call gb_Helper__register_executables,$(1),$(3))

gb_Executable_MODULE_$(2) += $(3)

endef

define gb_Helper__register_libraries
$(foreach group,$(gb_Library_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Library_$(group))),\
   $(call gb_Output_error,gb_Helper_register_libraries: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_libraries: contains duplicates: $(2)))

gb_Library_$(1) += $(2)

endef

# $(call gb_Helper_register_libraries,layer,libs)
define gb_Helper_register_libraries
ifeq ($$(filter $(1),$$(gb_Library_VALIDGROUPS_NOTINSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for libraries that are not installed. Valid groups are: $$(gb_Library_VALIDGROUPS_NOTINSTALLED). Use gb_Helper_register_libraries_for_install for installed libraries.))
endif
$(call gb_Helper__register_libraries,$(1),$(2))

endef

# the first argument is the group, which sets rpaths etc.
# the second argument is the install module, which describes in which distro package/msi a lib should show up
# UGLY: for versioned libraries "sdk" module is hard-coded for now
# $(call gb_Helper_register_libraries_for_install,layer,installmodule,libs)
define gb_Helper_register_libraries_for_install
$(if $(3),,$(call gb_Output_error,gb_Helper_register_libraries_for_install: no libraries - need 3 parameters))
ifeq ($$(filter $(1),$$(gb_Library_VALIDGROUPS_INSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for installed libraries. Valid groups are: $$(gb_Library_VALIDGROUPS_INSTALLED). Use gb_Helper_register_libraries for libraries that are not installed.))
endif
$(call gb_Helper__register_libraries,$(1),$(3))

gb_Library_MODULE_$(2) += $(filter-out $(gb_MERGEDLIBS),$(3))

$(if $(filter UNOVERLIBS RTVERLIBS,$(1)),\
	gb_SdkLinkLibrary_MODULE_sdk += $(3))

endef

# a plugin is a library, why can't be dynamically linked and must be dlopen'd, but must be linked static
define gb_Helper_register_plugins_for_install
$(call gb_Helper_register_libraries_for_install,$(1),$(2),$(3))
gb_Library_KNOWNPLUGINS += $(3)

endef

define gb_Helper__register_jars
$(foreach group,$(gb_Jar_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Jar_$(group))),\
   $(call gb_Output_error,gb_Helper_register_jars: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_jars: contains duplicates: $(2)))

gb_Jar_$(1) += $(2)

endef

# $(call gb_Helper_register_jars,layer,jars)
define gb_Helper_register_jars
ifeq ($$(filter $(1),$$(gb_Jar_VALIDGROUPS_NOTINSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for jars that are not installed. Valid groups are: $$(gb_Jar_VALIDGROUPS_NOTINSTALLED). Use gb_Helper_register_jars_for_install for installed jars.))
endif
$(call gb_Helper__register_jars,$(1),$(2))

endef

# $(call gb_Helper_register_jars_for_install,layer,installmodule,jars)
define gb_Helper_register_jars_for_install
$(if $(3),,$(call gb_Output_error,gb_Helper_register_jars_for_install: no jars - need 3 parameters))
ifeq ($$(filter $(1),$$(gb_Jar_VALIDGROUPS_INSTALLED)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for installed jars. Valid groups are: $$(gb_Jar_VALIDGROUPS_INSTALLED). Use gb_Helper_register_jars for jars that are not installed.))
endif
$(call gb_Helper__register_jars,$(1),$(3))

gb_Jar_MODULE_$(2) += $(3)

endef

define gb_Helper__register_packages
$(foreach target,$(1),\
 $(if $(filter $(target),$(gb_Package_REGISTERED)),\
  $(call gb_Output_error,gb_Helper_register_packages: already registered: $(target))))
$(if $(filter-out $(words $(1)),$(words $(sort $(1)))),\
 $(call gb_Output_error,gb_Helper_register_packages: contains duplicates: $(1)))

gb_Package_REGISTERED += $(1)

endef

# $(call gb_Helper_register_packages,packages)
define gb_Helper_register_packages
$(call gb_Helper__register_packages,$(1))

endef

# $(call gb_Helper_register_packages_for_install,installmodule,packages)
define gb_Helper_register_packages_for_install
$(if $(2),,$(call gb_Output_error,gb_Helper_register_packages_for_install: no packages - need 2 parameters))
$(call gb_Helper__register_packages,$(2))

gb_Package_MODULE_$(1) += $(2)

endef

define gb_Helper_register_mos
gb_AllLangMoTarget_REGISTERED += $(1)

endef

# TODO: this should be extended to handle auto-installation.
define gb_Helper_register_uiconfigs
gb_UIConfig_REGISTERED += $(1)

endef

define gb_Helper_get_imagelists
$(foreach ui,$(gb_UIConfig_REGISTERED),$(call gb_UIConfig_get_imagelist_target,$(ui)))
endef

# call gb_Helper_replace_if_different_and_touch,source,target,optional-touch-reference-file
define gb_Helper_replace_if_different_and_touch
if cmp -s $(1) $(2); then rm $(1); \
else mv $(1) $(2) $(if $(3),&& touch -r $(3) $(2)); \
fi
endef

# call gb_Helper_copy_if_different_and_touch,source,target,optional-touch-reference-file
define gb_Helper_copy_if_different_and_touch
if ! cmp -s $(1) $(2); then \
    cp $(1) $(2) $(if $(3),&& touch -r $(3) $(2)); \
fi
endef

define gb_Helper_define_if_set
$(foreach def,$(1),$(if $(filter TRUE YES,$($(def))),-D$(def)))
endef

define gb_Helper_execute
$(call gb_Executable_get_command,$(firstword $(1))) $(wordlist 2,$(words $(1)),$(1))
endef

# define gb_Helper_install registered-target target-to-install target-from-workdir
define gb_Helper_install
$(1) :| $(2)
$(2) : $(3) | $(dir $(2)).dir
$(call gb_Deliver_add_deliverable,$(2),$(3),$(2))
endef

# use if the installed target is the final target
define gb_Helper_install_final
$(1) : $(2) | $(dir $(1)).dir
$(call gb_Deliver_add_deliverable,$(1),$(2),$(1))
endef

# call gb_Helper_optional,build_type,if-true,if-false
define gb_Helper_optional
$(if $(filter $(1),$(BUILD_TYPE)),$(2),$(3))
endef

# call gb_Helper_optionals_or,build_types,if-true,if-false
define gb_Helper_optionals_or
$(call gb_Helper_optional,$(1),$(2),$(3))
endef

# call gb_Helper_optionals_and,build_types,if-true,if-false
define gb_Helper_optionals_and
$(if $(strip $(filter-out $(filter $(1),$(BUILD_TYPE)),$(1))),$(3),$(2))
endef

ifeq ($(WITH_LOCALES),)
define gb_Helper_optional_locale
$(2)
endef
else
define gb_Helper_optional_locale
$(if $(filter $(1) $(1)_%,$(WITH_LOCALES)),$(2))
endef
endif

define gb_Helper_print_on_error
$(if $(gb_QUIET_EXTERNAL), \
    $(if $(2), \
        ( ( $(1) ) > $(2) 2>&1 || ( cat $(2) && false ) ), \
        ( TEMPFILE=$(shell $(gb_MKTEMP)) && ( $(1) ) > $$TEMPFILE 2>&1 \
            && rm $$TEMPFILE \
            || ( cat $$TEMPFILE && rm $$TEMPFILE && false ) )), \
    ( $(1) ))
endef

# vim: set noet sw=4 ts=4:
