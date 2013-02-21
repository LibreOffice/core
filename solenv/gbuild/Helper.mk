# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

gb_Helper_MISC := $(WORKDIR)/Misc

# general purpose phony target
gb_Helper_PHONY := $(gb_Helper_MISC)/PHONY

# general purpose empty dummy target
gb_Helper_MISCDUMMY := $(gb_Helper_MISC)/DUMMY

.PHONY : $(WORKDIR)/Misc/PHONY
$(gb_Helper_MISCDUMMY) :
	@mkdir -p $(dir $@) && touch $@

ifeq ($(SRCDIR),$(BUILDDIR))
define gb_Helper_abbreviate_dirs
S=$(SRCDIR) && \
$(subst $(SRCDIR)/,$$S/,O=$(OUTDIR)) && \
$(subst $(SRCDIR)/,$$S/,W=$(WORKDIR)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(OUTDIR)/,$$O/,$(subst $(WORKDIR)/,$$W/,$(1))))
endef
else
define gb_Helper_abbreviate_dirs
S=$(SRCDIR) && \
$(subst $(SRCDIR)/,$$S/,B=$(BUILDDIR)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,O=$(OUTDIR))) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,W=$(WORKDIR))) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(BUILDDIR)/,$$B/,$(subst $(OUTDIR)/,$$O/,$(subst $(WORKDIR)/,$$W/,$(1)))))
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

define gb_Helper_make_clean_target
gb_$(1)_get_clean_target = $(WORKDIR)/Clean/$(1)/$$(1)

endef

define gb_Helper_make_outdir_clean_target
gb_$(1)_get_clean_target = $$(subst $(OUTDIR)/,$(WORKDIR)/Clean/OutDir/,$$(call gb_$(1)_get_target,$$(1)))

endef

define gb_Helper_make_dep_target
gb_$(1)_get_dep_target = $(WORKDIR)/Dep/$(1)/$$(1).d

endef

define gb_Helper_make_clean_targets
$(foreach targettype,$(1),\
	$(call gb_Helper_make_clean_target,$(targettype)))

endef

define gb_Helper_make_outdir_clean_targets
$(foreach targettype,$(1),\
	$(call gb_Helper_make_outdir_clean_target,$(targettype)))

endef

define gb_Helper_make_dep_targets
$(foreach targettype,$(1),\
	$(call gb_Helper_make_dep_target,$(targettype)))

endef

define gb_Helper_get_outdir_clean_target
$$(subst $(OUTDIR)/,$(WORKDIR)/Clean/OutDir/,$(1))
endef

# e.g. 'make CppunitTest_sw_macros_test'
define gb_Helper_make_userfriendly_targets
.PHONY: $(2)_$(1) $(2)_$(1).clean
$(2)_$(1) : $(call gb_$(2)_get_target,$(1))
$(2)_$(1).clean :  $(call gb_$(2)_get_clean_target,$(1))
endef

define gb_Helper_init_registries
gb_Executable_VALIDGROUPS := UREBIN SDK OOO NONE
gb_Library_VALIDGROUPS := OOOLIBS PLAINLIBS_NONE PLAINLIBS_URE PLAINLIBS_OOO RTLIBS RTVERLIBS UNOLIBS_URE UNOLIBS_OOO UNOVERLIBS EXTENSIONLIBS
gb_Library_VALIDINSTALLMODULES := OOO
gb_StaticLibrary_VALIDGROUPS := PLAINLIBS
gb_Jar_VALIDGROUPS := URE OOO OXT

$$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(eval gb_Executable_$$(group) :=))
$$(foreach group,$$(gb_Library_VALIDGROUPS),$$(eval gb_Library_$$(group) :=))
$$(foreach group,$$(gb_StaticLibrary_VALIDGROUPS),$$(eval gb_StaticLibrary_$$(group) :=))
$$(foreach group,$$(gb_Jar_VALIDGROUPS),$$(eval gb_Jar_$$(group) :=))

endef

define gb_Helper_collect_knownlibs
gb_Library_KNOWNLIBS := $$(foreach group,$$(gb_Library_VALIDGROUPS),$$(gb_Library_$$(group)))
gb_StaticLibrary_KNOWNLIBS := $$(foreach group,$$(gb_StaticLibrary_VALIDGROUPS),$$(gb_StaticLibrary_$$(group)))
gb_Executable_KNOWN := $$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(gb_Executable_$$(group)))
gb_Jar_KNOWN := $$(foreach group,$$(gb_Jar_VALIDGROUPS),$$(gb_Jar_$$(group)))

endef

define gb_Helper_process_executable_registrations
$(foreach group,$(gb_Executable_VALIDGROUPS),\
	$(foreach executable,$(gb_Executable_$(group)),\
		$(if $(filter-out undefined,$(origin gb_Executable__register_$(executable))),\
			$(call gb_Executable__register_$(executable)))))

endef

define gb_Helper_register_executables
ifeq ($$(filter $(1),$$(gb_Executable_VALIDGROUPS)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for executables. Valid groups are: $$(gb_Executable_VALIDGROUPS)))
endif
$(foreach group,$(gb_Executable_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Executable_$(group))),\
   $(call gb_Output_error,gb_Helper_register_executables: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_executables: contains duplicates: $(2)))

gb_Executable_$(1) += $(2)

endef

define gb_Helper_register_libraries
ifeq ($$(filter $(1),$$(gb_Library_VALIDGROUPS)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for libraries. Valid groups are: $$(gb_Library_VALIDGROUPS)))
endif
$(foreach group,$(gb_Library_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Library_$(group))),\
   $(call gb_Output_error,gb_Helper_register_libraries: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_libraries: contains duplicates: $(2)))

gb_Library_$(1) += $(2)

endef

# the first argument is the group, which sets rpaths etc.
# the second argument is the install module, which describes in which distro package/msi a lib should show up
define gb_Helper_register_libraries_for_install
ifeq ($$(filter $(2),$$(gb_Library_VALIDINSTALLMODULES)),)
$$(eval $$(call gb_Output_error,$(2) is not a valid install module for libraries. Valid groups are: $$(gb_Library_VALIDINSTALLMODULES)))
endif
$(call gb_Helper_register_libraries,$(1),$(3))

gb_Library_MODULE_$(2) += $(3)

endef

define gb_Helper_register_static_libraries
ifeq ($$(filter $(1),$$(gb_StaticLibrary_VALIDGROUPS)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for static libraries. Valid groups are: $$(gb_StaticLibrary_VALIDGROUPS)))
endif
$(foreach group,$(gb_StaticLibrary_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_StaticLibrary_$(group))),\
   $(call gb_Output_error,gb_Helper_register_static_libraries: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_static_libraries: contains duplicates: $(2)))

gb_StaticLibrary_$(1) += $(2)

endef

define gb_Helper_register_jars
ifeq ($$(filter $(1),$$(gb_Jar_VALIDGROUPS)),)
$$(eval $$(call gb_Output_error,$(1) is not a valid group for jars. Valid groups are: $$(gb_Jar_VALIDGROUPS)))
endif
$(foreach group,$(gb_Jar_VALIDGROUPS),\
 $(foreach target,$(2),\
  $(if $(filter $(target),$(gb_Jar_$(group))),\
   $(call gb_Output_error,gb_Helper_register_jars: already registered: $(target)))))
$(if $(filter-out $(words $(2)),$(words $(sort $(2)))),\
 $(call gb_Output_error,gb_Helper_register_jars: contains duplicates: $(2)))

gb_Jar_$(1) += $(2)

endef

define gb_Helper_define_if_set
$(foreach def,$(1),$(if $(filter TRUE YES,$($(def))),-D$(def)))
endef

define gb_Helper_execute
$(call gb_Executable_get_command,$(firstword $(1))) $(wordlist 2,$(words $(1)),$(1))
endef

# gb_Helper_OUTDIRLIBDIR is set by the platform to the path the dynamic linker need to use
# for libraries from the OUTDIR

define gb_Helper_optional
$(if $(filter $(1),$(BUILD_TYPE)),$(2))
endef

define gb_Helper_optional_for_host
$(if $(filter $(1),$(BUILD_TYPE_FOR_HOST)),$(2))
endef

define gb_Helper_print_on_error
$(if $(gb_QUIET_EXTERNAL),( $(1) ) > $(2) 2>&1 || ( cat $(2) && false ),$(1) )
endef

# vim: set noet sw=4 ts=4:
