#**************************************************************
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
#**************************************************************



gb_Helper_NULLFILE := /dev/null

gb_Helper_MISC := $(WORKDIR)/Misc

ifeq ($(JAVACOMPILER),)
JAVACOMPILER := javac
endif

ifeq ($(JAVAINTERPRETER),)
JAVAINTERPRETER := java
endif

# general propose phony target
gb_Helper_PHONY := $(gb_Helper_MISC)/PHONY

# general propose empty dummy target
gb_Helper_MISCDUMMY := $(gb_Helper_MISC)/DUMMY

gb_Helper_REPOSITORYNAMES :=

.PHONY : $(WORKDIR)/Misc/PHONY
$(gb_Helper_MISCDUMMY) :
	@mkdir -p $(dir $@) && touch $@

define gb_Helper_abbreviate_dirs
R=$(REPODIR) && \
$(subst $(REPODIR)/,$$R/,S=$(SRCDIR) && \
$(subst $(SRCDIR)/,$$S/,O=$(OUTDIR)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(OUTDIR)/,$$O/,W=$(WORKDIR) && $(subst $(WORKDIR)/,$$W/,$(1)))))
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

define gb_Helper_register_repository
gb_Helper_CURRENTREPOSITORY := $(1)
gb_Helper_REPOSITORYNAMES += $(1)

endef

# RepositoryExternal.mk is optional
define gb_Helper_add_repository
gb_Helper_CURRENTREPOSITORY :=
include $(1)/Repository.mk
ifeq ($$(gb_Helper_CURRENTREPOSITORY),)
$$(eval $$(call gb_Output_error,No call to gb_Helper_register_repository in Repository.mk for repository $(1)))
endif
$$(gb_Helper_CURRENTREPOSITORY) := $(1)
-include $(1)/RepositoryExternal.mk

endef

define gb_Helper_add_repositories
$(foreach repo,$(1),$(call gb_Helper_add_repository,$(repo)))

endef

define gb_Helper_init_registries
gb_Executable_VALIDGROUPS := UREBIN SDK OOO BRAND NONE
gb_Library_VALIDGROUPS := OOOLIBS PLAINLIBS_NONE PLAINLIBS_URE PLAINLIBS_OOO RTLIBS RTVERLIBS STLLIBS UNOLIBS_URE UNOLIBS_OOO UNOVERLIBS
gb_StaticLibrary_VALIDGROUPS := PLAINLIBS

$$(foreach group,$$(gb_Executable_VALIDGROUPS),$$(eval gb_Executable_$$(group) :=))
$$(foreach group,$$(gb_Library_VALIDGROUPS),$$(eval gb_Library_$$(group) :=))
$$(foreach group,$$(gb_StaticLibrary_VALIDGROUPS),$$(eval gb_StaticLibrary_$$(group) :=))

endef

define gb_Helper_collect_libtargets
gb_Library_TARGETS := $$(foreach group,$$(gb_Library_VALIDGROUPS),$$(gb_Library_$$(group)))
gb_StaticLibrary_TARGETS := $$(foreach group,$$(gb_StaticLibrary_VALIDGROUPS),$$(gb_StaticLibrary_$$(group)))

endef

define gb_Helper_collect_knownlibs
gb_Library_KNOWNLIBS := $$(foreach group,$$(gb_Library_VALIDGROUPS),$$(gb_Library_$$(group)))
gb_StaticLibrary_KNOWNLIBS := $$(foreach group,$$(gb_StaticLibrary_VALIDGROUPS),$$(gb_StaticLibrary_$$(group)))

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

# vim: set noet sw=4 ts=4:
