#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

gb_Helper_NULLFILE := /dev/null

gb_Helper_MISC := $(WORKDIR)/Misc

# general propose phony target
gb_Helper_PHONY := $(gb_Helper_MISC)/PHONY

# general propose empty dummy target
gb_Helper_MISCDUMMY := $(gb_Helper_MISC)/DUMMY

# if ($true) then old files will get removed from the target location before
# they are copied there. In multi-user environments, this is needed you need to
# be the owner of the target file to be able to cp -pf 
gb_Helper_CLEARONDELIVER := $(true)

.PHONY : $(WORKDIR)/Misc/PHONY
$(gb_Helper_MISCDUMMY) :
    @mkdir -p $(dir $@) && touch $@

define gb_Helper__format_type
[ $(2) $(word 2,$(1) >==> Oo._) ]
endef

define gb_Helper__format_target
$(1)
endef

ifeq ($(TERM),xterm)
ifeq ($(strip $(gb_NOCOLOR)),)
gb_Helper_ESCAPE := $(shell echo -e '\033')
gb_Helper_COLOR_RESET := $(gb_Helper_ESCAPE)[0m
gb_Helper_COLOR_RESETANDESCAPE := $(gb_Helper_COLOR_RESET)$(gb_Helper_ESCAPE)
gb_Helper_COLOR_OUTBUILD_LEVEL1 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTBUILD_LEVEL2 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTBUILD_LEVEL3 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTBUILD_LEVEL4 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTBUILD_LEVEL5 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;46m
gb_Helper_COLOR_OUTBUILD_LEVEL6 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;44m

gb_Helper_COLOR_INBUILD_LEVEL1 := $(gb_Helper_COLOR_RESETANDESCAPE)[36;40m
gb_Helper_COLOR_INBUILD_LEVEL2 := $(gb_Helper_COLOR_RESETANDESCAPE)[36;1;40m
gb_Helper_COLOR_INBUILD_LEVEL3 := $(gb_Helper_COLOR_RESETANDESCAPE)[32;40m
gb_Helper_COLOR_INBUILD_LEVEL4 := $(gb_Helper_COLOR_RESETANDESCAPE)[32;1;40m
gb_Helper_COLOR_INBUILD_LEVEL5 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;46m
gb_Helper_COLOR_INBUILD_LEVEL6 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;44m

gb_Helper_COLOR_OUTCLEAN_LEVEL1 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTCLEAN_LEVEL2 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTCLEAN_LEVEL3 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTCLEAN_LEVEL4 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;40m
gb_Helper_COLOR_OUTCLEAN_LEVEL5 := $(gb_Helper_COLOR_RESETANDESCAPE)[33;1;41m
gb_Helper_COLOR_OUTCLEAN_LEVEL6 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;41m

gb_Helper_COLOR_INCLEAN_LEVEL1 := $(gb_Helper_COLOR_RESETANDESCAPE)[33;40m
gb_Helper_COLOR_INCLEAN_LEVEL2 := $(gb_Helper_COLOR_RESETANDESCAPE)[33;1;40m
gb_Helper_COLOR_INCLEAN_LEVEL3 := $(gb_Helper_COLOR_RESETANDESCAPE)[31;40m
gb_Helper_COLOR_INCLEAN_LEVEL4 := $(gb_Helper_COLOR_RESETANDESCAPE)[31;1;40m
gb_Helper_COLOR_INCLEAN_LEVEL5 := $(gb_Helper_COLOR_RESETANDESCAPE)[33;1;41m
gb_Helper_COLOR_INCLEAN_LEVEL6 := $(gb_Helper_COLOR_RESETANDESCAPE)[37;1;41m

define gb_Helper__format_type
$(subst :, ,$(word 2,$(1) \
    $(gb_Helper_COLOR_OUTBUILD_LEVEL$(3))[:$(gb_Helper_COLOR_INBUILD_LEVEL$(3))$(subst $(WHITESPACE),:,$(2)):>==>$(gb_Helper_COLOR_OUTBUILD_LEVEL$(3)):] \
    $(gb_Helper_COLOR_OUTCLEAN_LEVEL$(3))[:$(gb_Helper_COLOR_INCLEAN_LEVEL$(3))$(subst $(WHITESPACE),:,$(2)):Oo._$(gb_Helper_COLOR_OUTCLEAN_LEVEL$(3)):]))$(gb_Helper_COLOR_RESET)
endef

endif
endif

define gb_Helper_announce
$(info $(call gb_Helper__format_type,$(2),$(3),$(4)) $(call gb_Helper__format_target,$(1)))
endef

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

gb_Helper__deliverprefix = mkdir -p $(dir $(1)) &&

ifeq ($(gb_Helper_CLEARONDELIVER),$(true))
gb_Helper__deliverprefix += rm -rf $(1) &&
endif

define gb_Helper_deliver
$(call gb_Helper__deliverprefix,$(2)) cp -f $(1) $(2) && touch -r $(1) $(2)
endef

define gb_Helper_register_repository
gb_Helper_CURRENTREPOSITORY := $(1)

endef

define gb_Helper_add_repository
gb_Helper_CURRENTREPOSITORY :=
include $(1)/Repository.mk
ifeq ($$(gb_Helper_CURRENTREPOSITORY),)
$$(error no gb_Helper_register_repository in Repository.mk for repository $(1))
endif
$$(gb_Helper_CURRENTREPOSITORY) := $(1)

endef

define gb_Helper_add_repositories
$(foreach repo,$(1),$(call gb_Helper_add_repository,$(repo)))

endef

define gb_Helper_init_registries
gb_Executable_VALIDLAYERS := UREBIN SDK OOO BRAND NONE
gb_Library_VALIDLAYERS := OOOLIBS PLAINLIBS_URE PLAINLIBS_OOO RTLIBS RTVERLIBS STLLIBS UNOLIBS_URE UNOLIBS_OOO UNOVERLIBS
gb_StaticLibrary_VALIDLAYERS := PLAINLIBS
gb_Library_NAMESCHEMES := OOO PLAIN RT RTVER STL UNO UNOVER
gb_StaticLibrary_NAMESCHEMES := PLAIN

$$(foreach layer,$$(gb_Executable_VALIDLAYERS),$$(eval gb_Executable_$$(layer) :=))
$$(foreach layer,$$(gb_Library_VALIDLAYERS),$$(eval gb_Library_$$(layer) :=))
$$(foreach layer,$$(gb_StaticLibrary_VALIDLAYERS),$$(eval gb_StaticLibrary_$$(layer) :=))

endef

define gb_Helper_collect_libtargets
gb_Library_PLAINLIBS := \
    $$(gb_Library_PLAINLIBS_URE) \
    $$(gb_Library_PLAINLIBS_OOO) \

gb_Library_UNOLIBS := \
    $$(gb_Library_UNOLIBS_URE) \
    $$(gb_Library_UNOLIBS_OOO) \

gb_Library_TARGETS := $$(foreach namescheme,$$(gb_Library_NAMESCHEMES),$$(gb_Library_$$(namescheme)LIBS))
gb_StaticLibrary_TARGETS := $$(foreach namescheme,$$(gb_StaticLibrary_NAMESCHEMES),$$(gb_StaticLibrary_$$(namescheme)LIBS))

endef

define gb_Helper_collect_knownlibs
gb_Library_KNOWNLIBS := $$(foreach namescheme,$$(gb_Library_NAMESCHEMES),$$(gb_Library_$$(namescheme)LIBS))
gb_StaticLibrary_KNOWNLIBS := $$(foreach namescheme,$$(gb_StaticLibrary_NAMESCHEMES),$$(gb_StaticLibrary_$$(namescheme)LIBS))

endef

define gb_Helper_register_executables
ifeq ($$(filter $(1),$$(gb_Executable_VALIDLAYERS)),)
$$(error $(1) is not a valid layer for executables. Valid layers are: $$(gb_Executable_VALIDLAYERS))
endif

gb_Executable_$(1) += $(2)

endef

define gb_Helper_register_libraries
ifeq ($$(filter $(1),$$(gb_Library_VALIDLAYERS)),)
$$(error $(1) is not a valid layer for layer. Valid layers are: $$(gb_Library_VALIDLAYERS))
endif

gb_Library_$(1) += $(2)

endef

define gb_Helper_register_static_libraries
ifeq ($$(filter $(1),$$(gb_StaticLibrary_VALIDLAYERS)),)
$$(error $(1) is not a valid layer for layer. Valid layers are: $$(gb_StaticLibrary_VALIDLAYERS))
endif

gb_StaticLibrary_$(1) += $(2)

endef

# vim: set noet sw=4 ts=4:
