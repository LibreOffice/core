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


# PrecompiledHeader class

# Use different PCH file depending on whether we use debugging symbols.
gb_PrecompiledHeader__get_debugdir = $(if $(call gb_LinkTarget__symbols_enabled,$(1)),debug,nodebug)

# $(call gb_PrecompiledHeader_generate_timestamp_rule,linktargetmakefilename)
define gb_PrecompiledHeader_generate_timestamp_rule
$(call gb_LinkTarget_get_pch_timestamp,$(1)) :
	mkdir -p $$(dir $$@) && touch $$@

endef

ifneq ($(gb_ENABLE_PCH),)

# IMPORTANT: Since these defines get expanded, every $ needs to be doubled to $$, except
# for $(1)'s and things that are constant.
# The defines are needed to get the right version of gb_PrecompiledHeader__get_debugdir.

# $(call gb_PrecompiledHeader_generate_rules,pchtarget,linktarget,linktargetmakefilename,pchcxxfile,compiler)
define gb_PrecompiledHeader_generate_rules

$(call gb_PrecompiledHeader_get_dep_target,$(1),$(3)) :
	$$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $$(dir $$@) && \
		echo "$$(call gb_PrecompiledHeader_get_target,$(1),$(3)) : $$(gb_Helper_PHONY)" > $$@)

# despite this being only one .d file, need to run concat-deps on it to
# re-write external headers from UnpackedTarball
$(call gb_PrecompiledHeader_get_target,$(1),$(3)) :
	test "$$(PCH_LINKTARGETMAKEFILENAME)" = "$(3)" \
		 || ( echo "Error, PCH $(1) built by $$(PCH_LINKTARGETMAKEFILENAME) instead of $(3)" >&2; exit 1)
	rm -f $$@
	$$(call gb_PrecompiledHeader__command,$$@,$(1),$$<,$$(PCH_DEFS),$$(PCH_CXXFLAGS) $$(gb_PrecompiledHeader_EXCEPTIONFLAGS),$$(INCLUDE),$(3),$(5))
	$$(call gb_PrecompiledHeader__sum_command,$$@,$(1),$$<,$$(PCH_DEFS),$$(PCH_CXXFLAGS) $$(gb_PrecompiledHeader_EXCEPTIONFLAGS),$$(INCLUDE),$(3))
	echo $$(sort $$(PCH_DEFS) $$(PCH_CXXFLAGS) $$(gb_PrecompiledHeader_EXCEPTIONFLAGS)) > $$(call gb_PrecompiledHeader_get_target,$(1),$(3)).flags
ifeq ($(gb_FULLDEPS),$(true))
	$$(call gb_Helper_abbreviate_dirs,\
		RESPONSEFILE=$$(call var2file,$$(shell $$(gb_MKTEMP)),200,$$(call gb_PrecompiledHeader_get_dep_target_tmp,$(1),$(3))) && \
		$$(call gb_Executable_get_command,concat-deps) $$$${RESPONSEFILE} \
			> $$(call gb_PrecompiledHeader_get_dep_target,$(1),$(3)) && \
		rm -f $$$${RESPONSEFILE} $$(call gb_PrecompiledHeader_get_dep_target_tmp,$(1),$(3)))
endif

$(call gb_PrecompiledHeader_get_for_reuse_target,$(1),$(3)) : $(call gb_LinkTarget_get_target,$(2))
	$$(call gb_PrecompiledHeader__create_reuse_files,$(2),$(1),$(3))
	mkdir -p $$(dir $$@) && touch $$@

.PHONY : $(call gb_PrecompiledHeader_get_clean_target,$(1))
$(call gb_PrecompiledHeader_get_clean_target,$(1)) :
	$$(call gb_Output_announce,$(1),$(false),PCH,1)
	-$$(call gb_Helper_abbreviate_dirs,\
		rm -f $$(call gb_PrecompiledHeader_get_target,$(1),$(3)) \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(3)).obj \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(3)).pdb \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(3)).sum \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(3)).flags \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(3)).reuse \
			$$(call gb_PrecompiledHeader_get_dep_target,$(1),$(3)))

endef

# $(call gb_PrecompiledHeader_check_flags,linktargetmakefilename,pchcxxfile,pchfile,flags)
# When creating a PCH, the PCH's CXXFLAGS are saved to a matching .flags file. When reusing the PCH
# from another linktarget, use the file to check that the linktarget uses the same CXXFLAGS as the PCH.
# This complements the check in gb_CxxObject__set_pchflags.
define gb_PrecompiledHeader_check_flags
$$(call gb_Helper_abbreviate_dirs,\
	$$(if $$(strip $$(call gb_PrecompiledHeader_check_flags_internal,$$(shell cat $(3).flags),$(4),$(2))),false,true) || ( \
		echo Error reusing $(2) by $(1). >&2 && \
		echo -n " precompiled header flags : ">&2 && \
		cat $(3).flags >&2 && \
		echo    "            object flags  : "$$(sort $(4)) >&2 && \
		echo    " reason : $$(call gb_PrecompiledHeader_check_flags_internal,$$(shell cat $(3).flags),$(4),$(2))" >&2 && \
		echo Incorrect precompiled header setup or internal gbuild error. >&2 ; \
		exit 1) \
)

endef

# When trying to reuse one PCH between multiple linktargets, there is a problem that we have
# various defines that cause mismatch in the check above, but these defines actually should not affect the PCH.
# Specifically, there are 3 kinds:
# - -DXXX_DLLIMPLEMENTATION - they are used only in our headers, should not affect system headers.
# - -DSYSTEM_XXX - they are used only by our code (if at all), should not affect system headers
# - various LO configuration defines - they again should only be used by our code and not system headers
# Technically, different compilers handle additional defines like this:
# - GCC
#   * It is explicitly allowed to have different macros, as long as they do not affect the PCH.
#   * With -Winvalid-pch GCC will even warn if there is a change in a macro affecting the PCH.
#   * https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html
# - Clang
#   * I could not find an official statement on what happens if definitions are different.
#   * In practice a conflict does not seem to be detected, but the PCH and all the code in it
#     acts according to the settings it was built with. Using a PCH and adding more defines
#     seems to be functionally equivalent to creating the definitions only after the PCH inclusion.
#   * As a side-effect, macros defined on the command line not present in the PCH suddenly
#     trigger the -Wunused-macros warning. See bottom of pch/inc/clangfix.hxx .
# - MSVC
#   * MSVC explicitly states that the definitions must be the same, but they are not checked,
#     and "unpredictable results can occur" if files depend on them.
#   * In practice the situation seems to be the same as with Clang, the PCH and the code from it
#     act according to the settings it was built with.
#   * https://docs.microsoft.com/en-us/cpp/build/creating-precompiled-header-files
# So while this is officially tricky, in practice it seems to work to allow PCH reuse if the linktarget
# has more defines than the PCH was built with, as long as the defines do not affect the PCH.
gb_PrecompiledHeader_ignore_flags_system := \
-DFASTSAX_DLLIMPLEMENTATION \
-DSAX_DLLIMPLEMENTATION \
-DSCQAHELPER_DLLIMPLEMENTATION \
-DVCLPLUG_WIN_IMPLEMENTATION \
-DVCLPLUG_GEN_IMPLEMENTATION \
-DSYSTEM_EXPAT \
-DSYSTEM_LIBXML \
-DSYSTEM_ZLIB \
-DHAVE_VALGRIND_HEADERS \
-DUSE_RANDR \
-DUSE_XINERAMA_XORG \
-DDISABLE_CVE_TESTS \
-DCPPUNIT_PLUGIN_EXPORT='extern "C" SAL_DLLPUBLIC_EXPORT' \
-DOOO_DLLIMPLEMENTATION_TEST \
-DSK_USER_CONFIG_HEADER=% \
-DSKIA_DLL \
-DGLM_FORCE_CTOR_INIT \
-DVCL_INTERNALS \
-include $(SRCDIR)/pch/inc/clangfix.hxx \

# Probably also update pch/inc/clangfix.hxx if you extend the list.

# $(call gb_PrecompiledHeader_check_flags_internal,pchfileflags,flags,pchcxxfile)
# Check if two sets of flags are compatible, allowing reuse of the PCH. Flags are compatible if
# - they are the same
# - the PCH is precompiled_system and the linktarget has additional defines listed above
define gb_PrecompiledHeader_check_flags_internal
$(if $(filter-out $(2),$(1)),$(filter-out $(2),$(1)), \
	$(if $(filter-out $(1),$(2)),\
		$(if $(filter-out precompiled_system,$(notdir $(3))),$(filter-out $(1),$(2)), \
			$(foreach flag,$(filter-out $(1),$(2)),$(filter-out $(gb_PrecompiledHeader_ignore_flags_system),$(flag))) \
		) \
	,) \
)
endef

endif

# vim: set noet sw=4:
