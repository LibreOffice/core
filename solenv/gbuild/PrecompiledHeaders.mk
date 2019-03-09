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

ifneq ($(gb_ENABLE_PCH),)

# Use different PCH file depending on whether we use debugging symbols.
gb_PrecompiledHeader__get_debugdir = $(if $(call gb_LinkTarget__symbols_enabled,$(1)),debug,nodebug)

# IMPORTANT: Since these defines get expanded, every $ needs to be doubled to $$, except
# for $(1)'s and things that are constant.
# The defines are needed to get the right version of gb_PrecompiledHeader__get_debugdir.

# $(call gb_PrecompiledHeader_generate_rules,pchtarget,linktargetmakefilename)
define gb_PrecompiledHeader_generate_rules

$(call gb_PrecompiledHeader_get_dep_target,$(1),$(2)) :
	$$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $$(dir $$@) && \
		echo "$$(call gb_PrecompiledHeader_get_target,$(1),$(2)) : $$(gb_Helper_PHONY)" > $$@)

# despite this being only one .d file, need to run concat-deps on it to
# re-write external headers from UnpackedTarball
$(call gb_PrecompiledHeader_get_target,$(1),$(2)) :
	rm -f $$@
	$$(call gb_PrecompiledHeader__command,$$@,$(1),$$<,$$(PCH_DEFS),$$(PCH_CXXFLAGS) $$(gb_PrecompiledHeader_EXCEPTIONFLAGS),$$(INCLUDE),$(2))
	$(SHA256SUM) $$@ >$$@.sum
ifeq ($(gb_FULLDEPS),$(true))
	$$(call gb_Helper_abbreviate_dirs,\
		RESPONSEFILE=$$(call var2file,$$(shell $$(gb_MKTEMP)),200,$$(call gb_PrecompiledHeader_get_dep_target_tmp,$(1),$(2))) && \
		$$(call gb_Executable_get_command,concat-deps) $$$${RESPONSEFILE} \
			> $$(call gb_PrecompiledHeader_get_dep_target,$(1),$(2)) && \
		rm -f $$$${RESPONSEFILE} $$(call gb_PrecompiledHeader_get_dep_target_tmp,$(1),$(2)))
endif

.PHONY : $(call gb_PrecompiledHeader_get_clean_target,$(1))
$(call gb_PrecompiledHeader_get_clean_target,$(1)) :
	$$(call gb_Output_announce,$(1),$(false),PCH,1)
	-$$(call gb_Helper_abbreviate_dirs,\
		rm -f $$(call gb_PrecompiledHeader_get_target,$(1),$(2)) \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(2)).obj \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(2)).pdb \
			$$(call gb_PrecompiledHeader_get_target,$(1),$(2)).sum \
			$$(call gb_PrecompiledHeader_get_timestamp,$(2)) \
			$$(call gb_PrecompiledHeader_get_dep_target,$(1),$(2)))

endef

endif

# $(call gb_PrecompiledHeader_generate_timestamp_rule,linktargetmakefilename)
define gb_PrecompiledHeader_generate_timestamp_rule
$(call gb_PrecompiledHeader_get_timestamp,$(1)) :
	mkdir -p $$(dir $$@) && touch $$@
endef

# vim: set noet sw=4:
