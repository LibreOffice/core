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
# *******************************************
#
# "Generic" comment form the author:
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
# For WASM, this also serialize the linking, because the wasm-opt process is multi-threaded,
# running on all available cores, using GB of memory. Extra parallelism is counterproductive.
#
# Best way to "debug" is to replace "eval" with "info" to see the expanded template, which
# doesn't look that horrible (both do the same expansion internally).
# There is already a lot of $(info ...) protected by the already mentioned $(gb_DEBUG_STATIC).
#
# P.S. remeber to keep the $(info ...) and $(eval ...) blocks in sync (maybe add a function?)
#
ifeq ($(true),$(gb_FULLDEPS))

# strip doesn't remove newlines, so this must be a single line (no define!), otherwise empty tests fail!
gb_Executable__has_any_dependencies = \
$(if $(strip $(filter-out GBUILD_TOUCHED, \
	$(call gb_Executable__get_all_libraries,$(1)) \
	$(call gb_Executable__get_all_externals,$(1)) \
	$(call gb_Executable__get_all_statics,$(1)))),$(1))

gb_CppunitTest__has_any_dependencies = \
$(if $(strip $(filter-out GBUILD_TOUCHED, \
	$(call gb_CppunitTest__get_all_libraries,$(1)) \
	$(call gb_CppunitTest__get_all_externals,$(1)) \
	$(call gb_CppunitTest__get_all_statics,$(1)))),$(1))

ifeq (,$(gb_PARTIAL_BUILD))

#ifeq ($(OS),EMSCRIPTEN)
$(foreach lib,$(gb_Library_KNOWNLIBS),$(if $(call gb_Library__get_component,$(lib)),$(eval $(call gb_Library_use_libraries,components,$(lib)))))
#endif

define gb_LinkTarget__add_x_template

define gb_LinkTarget__add_$(1)
$$(if $(gb_DEBUG_STATIC),$$(info $$(call gb_LinkTarget__get_all_$(1)_var,$$(call gb_LinkTarget__get_workdir_linktargetname,$$(1))) += $$(2)))
$$(call gb_LinkTarget__get_all_$(1)_var,$$(call gb_LinkTarget__get_workdir_linktargetname,$$(1))) += $$(2)

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


define gb_Class__add_x_template

define gb_$(1)__add_$(3)
$$(foreach item,$$(2),
	$$(foreach dep,$$(call gb_$(2)__get_all_$(3),$$(item)),
		$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(1)__get_all_$(3),$$(1))),,
			$$(eval $$(call gb_LinkTarget__add_$(3),$$(call gb_$(1)__get_workdir_linktargetname,$$(1)),$$(dep)))))
	$$(foreach dep,$$(call gb_$(2)__get_all_$(4),$$(item)),
		$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(1)__get_all_$(4),$$(1))),,
			$$(eval $$(call gb_LinkTarget__add_$(4),$$(call gb_$(1)__get_workdir_linktargetname,$$(1)),$$(dep))))))

endef

endef # gb_Class__add_x_template

ifneq (,$(gb_DEBUG_STATIC))
$(info $(call gb_Class__add_x_template,Executable,ExternalProject,externals,libraries))
$(info $(call gb_Class__add_x_template,Executable,Library,libraries,externals))
$(info $(call gb_Class__add_x_template,CppunitTest,ExternalProject,externals,libraries))
$(info $(call gb_Class__add_x_template,CppunitTest,Library,libraries,externals))
endif
$(eval $(call gb_Class__add_x_template,Executable,ExternalProject,externals,libraries))
$(eval $(call gb_Class__add_x_template,Executable,Library,libraries,externals))
$(eval $(call gb_Class__add_x_template,CppunitTest,ExternalProject,externals,libraries))
$(eval $(call gb_Class__add_x_template,CppunitTest,Library,libraries,externals))


# contains the list of all touched workdir_linktargetname(s)
gb_LinkTarget__ALL_TOUCHED =

define gb_LinkTarget__add_touch
$(eval $(call gb_LinkTarget__add_libraries,$(1),GBUILD_TOUCHED))
$(eval $(call gb_LinkTarget__add_externals,$(1),GBUILD_TOUCHED))
gb_LinkTarget__ALL_TOUCHED += $(1)

endef

define gb_LinkTarget__remove_touch
$(call gb_LinkTarget__get_all_libraries_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_libraries,$(1)))
$(call gb_LinkTarget__get_all_externals_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_externals,$(1)))
$(call gb_LinkTarget__get_all_statics_var,$(1)) := $(filter-out GBUILD_TOUCHED,$(call gb_LinkTarget__get_all_statics,$(1)))

endef

# The following code is a template, because libraries and externals can use each others.
# It recursively walks the dependency tree for all libraries and externals, storing
# the information in variables per workdir_linktargetname, filtered for uniqueness. In the end
# all of them have two variables, which you can get with gb_LinkTarget__get_all_(libraries|externals).
# Expanded lists have a GBUILD_TOUCHED entry to prevent processing them multiple times
#
# eval call gb_LinkTarget__fill_all_x_template,type,typeclassname,othertype,other typeclassname,override,override typeclassname
define gb_LinkTarget__fill_all_x_template

define gb_LinkTarget__fill_all_$(if $(5),$(5),$(1))
$$(if $$(filter GBUILD_TOUCHED,$$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1))),,
	$(if $(gb_DEBUG_STATIC),$$(info gb_LinkTarget__fill_all_$(if $(5),$(5),$(1)) $(1) for $$(1) in: $$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1))))
	$(if $(gb_DEBUG_STATIC),$$(info gb_LinkTarget__fill_all_$(if $(5),$(5),$(1)) $(3) for $$(1) in: $$(call gb_$(if $(6),$(6),$(2))__get_all_$(3),$$(1))))
	$$(eval $$(call gb_LinkTarget__add_touch,$$(call gb_$(if $(6),$(6),$(2))__get_workdir_linktargetname,$$(1))))
	$$(foreach item,$$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1)),
		$$(call gb_LinkTarget__fill_all_$(1),$$(item))
		$$(foreach dep,$$(call gb_$(2)__get_all_$(1),$$(item)),
			$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1))),,
				$$(eval $$(call gb_LinkTarget__add_$(1),$$(call gb_$(if $(6),$(6),$(2))__get_workdir_linktargetname,$$(1)),$$(dep)))))
		$$(foreach dep,$$(call gb_$(2)__get_all_$(3),$$(item)),
			$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(if $(6),$(6),$(2))__get_all_$(3),$$(1))),,
				$$(eval $$(call gb_LinkTarget__add_$(3),$$(call gb_$(if $(6),$(6),$(2))__get_workdir_linktargetname,$$(1)),$$(dep))))))
	$$(foreach item,$$(call gb_$(if $(6),$(6),$(2))__get_all_$(3),$$(1)),
		$$(call gb_LinkTarget__fill_all_$(3),$$(item))
		$$(foreach dep,$$(call gb_$(4)__get_all_$(1),$$(item)),
			$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1))),,
				$$(eval $$(call gb_LinkTarget__add_$(1),$$(call gb_$(if $(6),$(6),$(2))__get_workdir_linktargetname,$$(1)),$$(dep)))))
		$$(foreach dep,$$(call gb_$(4)__get_all_$(3),$$(item)),
			$$(if $$(filter $$(dep),GBUILD_TOUCHED $$(call gb_$(if $(6),$(6),$(2))__get_all_$(3),$$(1))),,
				$$(eval $$(call gb_LinkTarget__add_$(3),$$(call gb_$(if $(6),$(6),$(2))__get_workdir_linktargetname,$$(1)),$$(dep))))))
	$(if $(gb_DEBUG_STATIC),$$(info gb_LinkTarget__fill_all_$(if $(5),$(5),$(1)) $(1) for $$(1) out: $$(call gb_$(if $(6),$(6),$(2))__get_all_$(1),$$(1))))
	$(if $(gb_DEBUG_STATIC),$$(info gb_LinkTarget__fill_all_$(if $(5),$(5),$(1)) $(3) for $$(1) out: $$(call gb_$(if $(6),$(6),$(2))__get_all_$(3),$$(1)))))

endef

endef # gb_LinkTarget__fill_all_x_template

ifneq (,$(gb_DEBUG_STATIC))
$(info $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject))
$(info $(call gb_LinkTarget__fill_all_x_template,externals,ExternalProject,libraries,Library))
$(info $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject,executable,Executable))
$(info $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject,cppunit,CppunitTest))
endif
$(eval $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject))
$(eval $(call gb_LinkTarget__fill_all_x_template,externals,ExternalProject,libraries,Library))
$(eval $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject,executable,Executable))
$(eval $(call gb_LinkTarget__fill_all_x_template,libraries,Library,externals,ExternalProject,cppunit,CppunitTest))


# contains the last known executable workdir targetname
gb_Executable__LAST_KNOWN =
gb_CppunitTest__LAST_KNOWN =

#$(eval $(call gb_Library_use_libraries,cppuhelper,components))

# The comment exists To help decipering / verifying the following block. Most later items depends on previous one(s).
#
# * Expand all libraries. It's not strictly needed, as we only need the info for the executables,
#   but this way we can implement updating single gbuild-module dependencies as needed.
# * For all executables (incl. CppunitTest(s)):
#   * Expand all normal dependencies
#   * Check if cppuhelper loader for components is requested and add the needed plugin dependences
#     This is a *HACK*, so we don't have to recursively check loader libraries - at least currently
#   * Find any other loader libraries and add the needed plugin dependences
#   * Add all statics to the executables
#   * Add icudata as needed (it should be a plugin somehow declared in RepositoryExternal.mk, but that didn't work)
#   * Serialize the linking of executables for EMSCRIPTEN, because wasm-opt is multi-threaded using all cores.
# * Remove "touch" mark from all touched targets
$(foreach lib,$(gb_Library_KNOWNLIBS),$(eval $(call gb_LinkTarget__fill_all_libraries,$(lib))))

define gb_LinkTarget__expand_executable
$$(eval $$(call gb_LinkTarget__fill_all_$(1),$(3))) \
$$(if $$(filter cppuhelper,$$(filter $$(gb_Library_KNOWNLOADERS),$$(call gb_$(2)__get_all_libraries,$(3)))), \
	$$(eval $$(call gb_$(2)_use_libraries,$(3),$$(call gb_Library__get_plugins,cppuhelper))) \
	$$(eval $$(call gb_$(2)__add_libraries,$(3),$$(call gb_Library__get_plugins,cppuhelper)))) \
$$(foreach loader,$$(filter $$(filter-out cppuhelper,$$(gb_Library_KNOWNLOADERS)),$$(call gb_$(2)__get_all_libraries,$(3))), \
	$$(eval $$(call gb_$(2)_use_libraries,$(3),$$(call gb_Library__get_plugins,$$(loader)))) \
	$$(eval $$(call gb_$(2)__add_libraries,$(3),$$(call gb_Library__get_plugins,$$(loader))))) \
$$(if $$(filter-out GBUILD_TOUCHED,$$(call gb_$(2)__get_all_libraries,$(3))), \
	$$(foreach lib,$$(filter-out GBUILD_TOUCHED,$$(call gb_$(2)__get_all_libraries,$(3))), \
		$$(if $$(call gb_Library__get_all_statics,$$(lib)), \
			$$(eval $$(call gb_$(2)_use_static_libraries,$(3),$$(call gb_Library__get_all_statics,$$(lib)))))) \
	$$(eval $$(call gb_$(2)_use_libraries,$(3),$$(filter-out GBUILD_TOUCHED,$$(call gb_$(2)__get_all_libraries,$(3)))))) \
$$(if $$(filter-out GBUILD_TOUCHED,$$(call gb_$(2)__get_all_externals,$(3))), \
	$$(eval $$(call gb_$(2)_use_externals,$(3),$$(filter-out GBUILD_TOUCHED,$$(call gb_$(2)__get_all_externals,$(3)))))) \
$$(if $$(filter icui28n icuuc,$$(call gb_$(2)__get_all_externals,$(3))), \
	$$(eval $$(call gb_$(2)_use_externals,$(3),icudata))) \
$$(if $$(and $$(filter EMSCRIPTEN,$$(OS)),$$(call gb_$(2)__has_any_dependencies,$(3)),), \
	$$(if $$(gb_$(2)__LAST_KNOWN), \
		$$(if $$(gb_DEBUG_STATIC),$$(info $$(call gb_$(2)_get_linktargetfile,$(3)) => $$(call gb_$(2)_get_linktargetfile,$$(gb_$(2)__LAST_KNOWN)))) \
		$$(eval $$(call gb_$(2)_get_linktargetfile,$(3)) : $$(call gb_$(2)_get_linktargetfile,$$(gb_$(2)__LAST_KNOWN)))) \
	$$(eval gb_$(2)__LAST_KNOWN = $(3)))

endef

$(foreach exec,$(gb_Executable_KNOWN),$(eval $(call gb_LinkTarget__expand_executable,executable,Executable,$(exec))))
$(foreach cppunit,$(gb_CppunitTest_KNOWN),$(eval $(call gb_LinkTarget__expand_executable,cppunit,CppunitTest,$(cppunit))))
$(foreach workdir_linktargetname,$(gb_LinkTarget__ALL_TOUCHED),$(eval $(call gb_LinkTarget__remove_touch,$(workdir_linktargetname))))

else # $(gb_PARTIAL_BUILD)

gb_Executable__get_dep_libraries_target = $(call gb_LinkTarget_get_dep_libraries_target,$(call gb_Executable__get_workdir_linktargetname,$(1)))
gb_Executable__get_dep_externals_target = $(call gb_LinkTarget_get_dep_externals_target,$(call gb_Executable__get_workdir_linktargetname,$(1)))
gb_Executable__get_dep_statics_target = $(call gb_LinkTarget_get_dep_statics_target,$(call gb_Executable__get_workdir_linktargetname,$(1)))
gb_CppunitTest__get_dep_libraries_target = $(call gb_LinkTarget_get_dep_libraries_target,$(call gb_CppunitTest__get_workdir_linktargetname,$(1)))
gb_CppunitTest__get_dep_externals_target = $(call gb_LinkTarget_get_dep_externals_target,$(call gb_CppunitTest__get_workdir_linktargetname,$(1)))
gb_CppunitTest__get_dep_statics_target = $(call gb_LinkTarget_get_dep_statics_target,$(call gb_CppunitTest__get_workdir_linktargetname,$(1)))

define gb_LinkTarget__expand_executable
$$(if $$(call gb_$(1)__has_any_dependencies,$(2)), \
	$$(if $$(shell cat $$(call gb_$(1)__get_dep_libraries_target,$(2)) 2>/dev/null), \
		$$(eval $$(call gb_$(1)_use_libraries,$(2),$$(shell cat $$(call gb_$(1)__get_dep_libraries_target,$(2)))))) \
	$$(if $$(shell cat $$(call gb_$(1)__get_dep_externals_target,$(2)) 2>/dev/null), \
		$$(eval $$(call gb_$(1)_use_externals,$(2),$$(shell cat $$(call gb_$(1)__get_dep_externals_target,$(2)))))) \
	$$(if $$(shell cat $$(call gb_$(1)__get_dep_statics_target,$(2)) 2>/dev/null), \
		$$(eval $$(call gb_$(1)_use_static_libraries,$(2),$$(shell cat $$(call gb_$(1)__get_dep_statics_target,$(2)))))) \
	$$(if $$(filter EMSCRIPTEN,$$(OS)), \
		$$(if $$(gb_$(1)__LAST_KNOWN), \
			$$(if $$(gb_DEBUG_STATIC),$$(info $$(call gb_$(1)_get_linktargetfile,$(2)) => $$(call gb_$(1)_get_linktargetfile,$$(gb_$(1)__LAST_KNOWN)))) \
			$$(eval $$(call gb_$(1)_get_linktargetfile,$(2)) : $$(call gb_$(1)_get_linktargetfile,$$(gb_$(1)__LAST_KNOWN)))) \
		$$(eval gb_$(1)__LAST_KNOWN = $(2))))

endef # gb_LinkTarget__expand_executable

$(foreach exec,$(gb_Executable_KNOWN),$(eval $(call gb_LinkTarget__expand_executable,Executable,$(exec))))
$(foreach cppunit,$(gb_CppunitTest_KNOWN),$(eval $(call gb_LinkTarget__expand_executable,CppunitTest,$(cppunit))))

endif # $(gb_PARTIAL_BUILD)
endif # $(gb_FULLDEPS)

# vim: set noet sw=4 ts=4:
