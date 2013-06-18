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

ifeq ($(gb_ENABLE_PCH),$(true))

# gb_PrecompiledHeader_get_enableflags defined by platform
ifneq ($(strip $(gb_DEBUGLEVEL)$(gb_SYMBOLS)),0)
gb_PrecompiledHeader_DEBUGDIR := debug
gb_NoexPrecompiledHeader_DEBUGDIR := debug
else
gb_PrecompiledHeader_DEBUGDIR := nodebug
gb_NoexPrecompiledHeader_DEBUGDIR := nodebug
endif

$(call gb_PrecompiledHeader_get_dep_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		echo "$(call gb_PrecompiledHeader_get_target,$*) : $(gb_Helper_PHONY)" > $@)

$(call gb_NoexPrecompiledHeader_get_dep_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		echo "$(call gb_NoexPrecompiledHeader_get_target,$*) : $(gb_Helper_PHONY)" > $@)

$(call gb_PrecompiledHeader_get_target,%) :
	rm -f $@
	$(call gb_PrecompiledHeader__command,$@,$*,$<,$(PCH_DEFS),$(PCH_CXXFLAGS) $(gb_PrecompiledHeader_EXCEPTIONFLAGS),$(INCLUDE))

$(call gb_NoexPrecompiledHeader_get_target,%) :
	rm -f $@
	$(call gb_NoexPrecompiledHeader__command,$@,$*,$<,$(PCH_DEFS),$(PCH_CXXFLAGS) $(gb_NoexPrecompiledHeader_NOEXCEPTIONFLAGS),$(INCLUDE))

.PHONY : $(call gb_PrecompiledHeader_get_clean_target,%) $(call gb_NoExPrecompiledHeader_get_clean_target,%)
$(call gb_PrecompiledHeader_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PCH,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_PrecompiledHeader_get_target,$*) \
			$(call gb_PrecompiledHeader_get_target,$*).obj \
			$(call gb_PrecompiledHeader_get_target,$*).pdb \
			$(call gb_PrecompiledHeader_get_timestamp,$*) \
			$(call gb_PrecompiledHeader_get_dep_target,$*))


$(call gb_NoexPrecompiledHeader_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PCH,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_NoexPrecompiledHeader_get_target,$*) \
			$(call gb_NoexPrecompiledHeader_get_target,$*).obj \
			$(call gb_NoexPrecompiledHeader_get_target,$*).pdb \
			$(call gb_NoexPrecompiledHeader_get_timestamp,$*) \
			$(call gb_NoexPrecompiledHeader_get_dep_target,$*))
endif

$(call gb_PrecompiledHeader_get_timestamp,%) :
	mkdir -p $(dir $@) && touch $@

$(call gb_NoexPrecompiledHeader_get_timestamp,%) :
	mkdir -p $(dir $@) && touch $@

# vim: set noet sw=4:
