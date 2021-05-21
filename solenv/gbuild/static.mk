# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# "spezialgelagerter Sonderfall" :-(
#
# *******************************************
# Use gb_DEBUG_STATIC=t to dump variuos debug info for the static build preparation!
# Use gb_DEBUG_STATIC=a to abort / error after the dump
# *******************************************
#
# "Generic" comment from the author:
# My thought was: the dependency info is already there. Can't be too hard to use it for
# correct static linkage... well it took more then two weeks to hopefully identify all
# problems / edge cases. Now I can appreciate the usage bin/lo-all-static-libs even more.
#
# This code moved a few times to the various gbuild / make phases: Makefile / module
# setup (in use_(libraries|externals)), post parsing (like now), make "dependency" tree
# processing. It currently looks like this is the only working option.
#
# For static linking, we must collect all depending libraries, externals and statics to
# be available at link time. And Libraries and externals can use each other. This could
# be done "in-line", while make processes the dependency tree and would have the correct
# order, so no separate tree-walking would be needed.
# It can't be done while reading / processing the modules, as this happens in no order, so
# depending modules will be missing.
#
# Then there is the (gbuild new) concept of plugin libraries. These depend on some loader
# libraries, like vcl and vclplug_* or gie. For a shared library build, these plugins are
# just dlopen'ed, but for a static build, any linked binary must also link the plugins, which
# turns plugins + loaders into a dependency cycle. The implemented solution is to just add
# plugins to executables, if these depend on a loader library. This results in the additional
# rule: nothing non-plugin is allowed to depend on a plugins (see gb_Library_set_plugin_for).
#
# And we can't add these dependencies while make is already processing the Executables,
# because these contain additional eval'ed targets, which we can't create in make recipes.
# This is especially true for externals (see gb_LinkTarget__use_* in RepositoryExternal.mk).
# We also can't add all plugins to all executables, as we have multiple helper
# binaries, which are needed early and don't depend on plugins.
#
# So the only option left seems to be to walk the dependency tree ourself and expand all
# the libraries and externals and then add the plugins to the executables. Statics are
# handled after that, since these won't need extra dependencies not already known.
# It's a bit fragile in theory, as you can add "gbuild-undetectable" dependencies to any
# target in the modules, which would need some manual adjustment, but currently that
# doesn't seem to happen in any breaking way and it works to link multiple Executable
# with large and small expanded dependency lists.
#
# Then there is the special static "components" library, which simply depends on all build
# components. In theory these could be limited per-module (Writer, Calc, etc.), but currently
# this is not implemented and instead solenv/bin/native-code.py is used, so actually
# everything is build and "cleaned up" at link time, which is especially expensive for WASM.
# That library is currently just used for Emscripten, but could be used generally for
# static builds.
#
# There is already a lot of $(info ...) protected by the already mentioned $(gb_DEBUG_STATIC).

ifeq ($(true),$(gb_FULLDEPS))

ifeq (,$(gb_PARTIAL_BUILD))

$(foreach lib,$(gb_Library_KNOWNLIBS),$(if $(call gb_Library__get_component,$(lib)), \
    $(eval $(call gb_Library_use_libraries,components,$(lib)))))

define gb_LinkTarget__add_x_template

# call gb_LinkTarget__add_$(1),linktarget,objects
define gb_LinkTarget__add_$(1)
$$(foreach item,$$(2),$$(if $$(filter $$(item),GBUILD_TOUCHED $$(call gb_LinkTarget__get_all_$(1),$$(1))),,
    $$(if $(gb_DEBUG_STATIC),$$(info $$(call gb_LinkTarget__get_all_$(1)_var,$$(call gb_LinkTarget__get_workdir_linktargetname,$$(1))) += $$(item)))
    $$(eval $$(call gb_LinkTarget__get_all_$(1)_var,$$(call gb_LinkTarget__get_workdir_linktargetname,$$(1))) += $$(item))
))
endef

endef # gb_LinkTarget__add_x_template

ifneq (,$(gb_DEBUG_STATIC))
$(info $(call gb_LinkTarget__add_x_template,libraries))
$(info $(call gb_LinkTarget__add_x_template,externals))
$(info $(call gb_LinkTarget__add_x_template,statics))
endif
$(eval $(call gb_LinkTarget__add_x_template,libraries))
$(eval $(call gb_LinkTarget__add_x_template,externals))
$(eval $(call gb_LinkTarget__add_x_template,statics))

# call gb_LinkTarget__add_linktargets,linktarget,class,func,objects
define gb_LinkTarget__add_linktargets
$(call gb_LinkTarget__add_$(3),$(1),$(4))
$(foreach item,$(foreach mapped,$(4),$(call gb_$(2)__get_workdir_linktargetname,$(mapped))),
    $(call gb_LinkTarget__add_libraries,$(1),$(call gb_LinkTarget__get_all_libraries,$(item)))
    $(call gb_LinkTarget__add_externals,$(1),$(call gb_LinkTarget__get_all_externals,$(item)))
    $(call gb_LinkTarget__add_statics,$(1),$(call gb_LinkTarget__get_all_statics,$(item)))
)
endef

# contains the list of all touched workdir_linktargetname(s)
gb_LinkTarget__ALL_TOUCHED =

define gb_LinkTarget__add_touch
$(eval $(call gb_LinkTarget__get_all_libraries_var,$(call gb_LinkTarget__get_workdir_linktargetname,$(1))) += GBUILD_TOUCHED)
$(eval $(call gb_LinkTarget__get_all_externals_var,$(call gb_LinkTarget__get_workdir_linktargetname,$(1))) += GBUILD_TOUCHED)
$(eval $(call gb_LinkTarget__get_all_statics_var,$(call gb_LinkTarget__get_workdir_linktargetname,$(1))) += GBUILD_TOUCHED)
gb_LinkTarget__ALL_TOUCHED += $(1)

endef

define gb_LinkTarget__remove_touch
$(call gb_LinkTarget__get_all_libraries_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_libraries,$(1)))
$(call gb_LinkTarget__get_all_externals_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_externals,$(1)))
$(call gb_LinkTarget__get_all_statics_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_statics,$(1)))

endef

# call gb_LinkTarget__fill_all_deps.linktargetname
define gb_LinkTarget__fill_all_deps
$(if $(filter GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_lo_libraries,$(1))),,

    # LO has quite a few dependency loops, so touch first to break them
    $(call gb_LinkTarget__add_touch,$(1))

    # Add lo libraries
    $(foreach item,$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_lo_libraries,$(1))),
        $(call gb_LinkTarget__fill_all_deps,$(call gb_Library_get_linktarget,$(item)))
        $(call gb_LinkTarget__add_libraries,$(1),$(call gb_Library__get_all_libraries,$(item)))
        $(call gb_LinkTarget__add_externals,$(1),$(call gb_Library__get_all_externals,$(item)))
        $(call gb_LinkTarget__add_statics,$(1),$(call gb_Library__get_all_statics,$(item)))
    )

    # Add (win32) system libraries
    $(call gb_LinkTarget__add_libraries,$(1),$(call gb_LinkTarget__get_all_sys_libraries,$(1)))

    # Add externals
    $(foreach item,$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_externals,$(1))),
        $(call gb_LinkTarget__fill_all_deps,$(call gb_ExternalProject__get_workdir_linktargetname,$(item)))
        $(call gb_LinkTarget__add_libraries,$(1),$(call gb_ExternalProject__get_all_libraries,$(item)))
        $(call gb_LinkTarget__add_externals,$(1),$(call gb_ExternalProject__get_all_externals,$(item)))
        $(call gb_LinkTarget__add_statics,$(1),$(call gb_ExternalProject__get_all_statics,$(item)))
    )

    # Add statics
    $(foreach item,$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_statics,$(1))),
        $(call gb_LinkTarget__fill_all_deps,$(call gb_StaticLibrary_get_linktarget,$(item)))
        $(call gb_LinkTarget__add_libraries,$(1),$(call gb_StaticLibrary__get_all_libraries,$(item)))
        $(call gb_LinkTarget__add_externals,$(1),$(call gb_StaticLibrary__get_all_externals,$(item)))
        $(call gb_LinkTarget__add_statics,$(1),$(call gb_StaticLibrary__get_all_statics,$(item)))
    )

    $(if $(gb_DEBUG_STATIC),
        $(info gb_LinkTarget__fill_all_deps libraries for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)) out: $(call gb_LinkTarget__get_all_libraries,$(1)))
        $(info gb_LinkTarget__fill_all_deps externals for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)) out: $(call gb_LinkTarget__get_all_externals,$(1)))
        $(info gb_LinkTarget__fill_all_deps statics   for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)) out: $(call gb_LinkTarget__get_all_statics,$(1)))
    )
)

endef


# call gb_LinkTarget__expand_executable,linktarget
define gb_LinkTarget__expand_executable
$(call gb_LinkTarget__fill_all_deps,$(1))

# 1. Check if cppuhelper loader for components is requested and add the needed plugin dependences
#    This is a *HACK*, so we don't have to recursively check loader libraries
# 2. Find any other loader libraries and add the needed plugin dependences
$(if $(filter cppuhelper,$(filter $(gb_Library_KNOWNLOADERS),$(call gb_LinkTarget__get_all_libraries,$(1)))),
    $(call gb_LinkTarget__add_linktargets,$(1),Library,libraries,$(call gb_Library__get_plugins,cppuhelper)))
$(foreach loader,$(filter $(filter-out cppuhelper,$(gb_Library_KNOWNLOADERS)),$(call gb_LinkTarget__get_all_libraries,$(1))),
    $(call gb_LinkTarget__add_linktargets,$(1),Library,libraries,$(call gb_Library__get_plugins,$(loader))))

$(if $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_libraries,$(1))),
    $(eval $(call gb_LinkTarget_use_libraries,$(1),$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_libraries,$(1))))))

$(if $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_externals,$(1))),
    $(eval $(call gb_LinkTarget_use_externals,$(1),$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_externals,$(1))))))

$(if $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_statics,$(1))),
    $(eval $(call gb_LinkTarget_use_static_libraries,$(1),$(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_statics,$(1))))))

# Some fixes for the _use_external_project(s) mess
$(if $(filter icui28n icuuc,$(call gb_LinkTarget__get_all_externals,$(1))),
    $(call gb_LinkTarget_use_externals,$(1),icudata))
$(if $(filter orcus-parser,$(call gb_LinkTarget__get_all_externals,$(1))),
    $(call gb_LinkTarget_use_static_libraries,$(1),boost_filesystem))

$(if $(gb_DEBUG_STATIC),
    $(info gb_LinkTarget__expand_executable libraries for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)): $(call gb_LinkTarget__get_all_libraries,$(1)))
    $(info gb_LinkTarget__expand_executable externals for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)): $(call gb_LinkTarget__get_all_externals,$(1)))
    $(info gb_LinkTarget__expand_executable statics   for $(call gb_LinkTarget__get_workdir_linktargetname,$(1)): $(call gb_LinkTarget__get_all_statics,$(1)))
)

endef

$(foreach lib,$(gb_Library_KNOWNLIBS), \
    $(eval $(call gb_LinkTarget__fill_all_deps,$(call gb_Library_get_linktarget,$(lib)))))
$(foreach exec,$(gb_Executable_KNOWN), \
    $(eval $(call gb_LinkTarget__expand_executable,$(call gb_Executable_get_linktarget,$(exec)))))
$(foreach workdir_linktargetname,$(gb_LinkTarget__ALL_TOUCHED), \
    $(eval $(call gb_LinkTarget__remove_touch,$(workdir_linktargetname))))

else # gb_PARTIAL_BUILD

# call gb_LinkTarget__expand_executable_template,class
define gb_LinkTarget__expand_executable_template

gb_$(1)__get_dep_libraries_target = $$(call gb_LinkTarget_get_dep_libraries_target,$$(call gb_$(1)__get_workdir_linktargetname,$$(1)))
gb_$(1)__get_dep_externals_target = $$(call gb_LinkTarget_get_dep_externals_target,$$(call gb_$(1)__get_workdir_linktargetname,$$(1)))
gb_$(1)__get_dep_statics_target = $$(call gb_LinkTarget_get_dep_statics_target,$$(call gb_$(1)__get_workdir_linktargetname,$$(1)))

# call gb_$(1)__has_any_dependencies,item
define gb_$(1)__has_any_dependencies
$$(if $$(strip $$(filter-out GBUILD_TOUCHED,
    $$(call gb_$(1)__get_all_libraries,$$(1))
    $$(call gb_$(1)__get_all_externals,$$(1))
    $$(call gb_$(1)__get_all_statics,$$(1)))),$$(1))

endef

# call gb_$(1)__expand_deps,item
define gb_$(1)__expand_deps
$$(if $$(call gb_$(1)__has_any_dependencies,$$(1)),
    $$(if $$(shell cat $$(call gb_$(1)__get_dep_libraries_target,$$(1)) 2>/dev/null),
        $$(eval $$(call gb_$(1)_use_libraries,$$(1),$$(shell cat $$(call gb_$(1)__get_dep_libraries_target,$$(1))))))
    $$(if $$(shell cat $$(call gb_$(1)__get_dep_externals_target,$$(1)) 2>/dev/null),
        $$(eval $$(call gb_$(1)_use_externals,$$(1),$$(shell cat $$(call gb_$(1)__get_dep_externals_target,$$(1))))))
    $$(if $$(shell cat $$(call gb_$(1)__get_dep_statics_target,$$(1)) 2>/dev/null), \
        $$(eval $$(call gb_$(1)_use_static_libraries,$$(1),$$(shell cat $$(call gb_$(1)__get_dep_statics_target,$$(1))))))
)

endef

endef # gb_LinkTarget__expand_executable_template

ifneq (,$(gb_DEBUG_STATIC))
$(info $(call gb_LinkTarget__expand_executable_template,Executable))
$(info $(call gb_LinkTarget__expand_executable_template,CppunitTest))
endif
$(eval $(call gb_LinkTarget__expand_executable_template,Executable))
$(eval $(call gb_LinkTarget__expand_executable_template,CppunitTest))

$(foreach exec,$(gb_Executable_KNOWN),$(eval $(call gb_Executable__expand_deps,$(exec))))

endif # gb_PARTIAL_BUILD
endif # gb_FULLDEPS

# vim: set noet sw=4 ts=4:
