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


# PrecompiledHeader class

ifeq ($(gb_ENABLE_PCH),$(true))

# gb_PrecompiledHeader_get_enableflags defined by platform
ifeq ($(gb_DEBUGLEVEL),2)
gb_PrecompiledHeader_DEBUGDIR := debug
gb_NoexPrecompiledHeader_DEBUGDIR := debug
else
gb_PrecompiledHeader_DEBUGDIR := nodebug
gb_NoexPrecompiledHeader_DEBUGDIR := nodebug
endif

$(call gb_PrecompiledHeader_get_dep_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		echo '$(call gb_PrecompiledHeader_get_target,$*) : $$(gb_Helper_PHONY)' > $@)

$(call gb_NoexPrecompiledHeader_get_dep_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		echo '$(call gb_NoexPrecompiledHeader_get_target,$*) : $$(gb_Helper_PHONY)' > $@)

$(call gb_PrecompiledHeader_get_target,%) :
	$(call gb_PrecompiledHeader__command,$@,$*,$<,$(PCH_DEFS),$(PCH_CXXFLAGS) $(gb_PrecompiledHeader_EXCEPTIONFLAGS),$(INCLUDE_STL) $(INCLUDE))

$(call gb_NoexPrecompiledHeader_get_target,%) :
	$(call gb_NoexPrecompiledHeader__command,$@,$*,$<,$(PCH_DEFS),$(PCH_CXXFLAGS) $(gb_NoexPrecompiledHeader_NOEXCEPTIONFLAGS),$(INCLUDE_STL) $(INCLUDE))

.PHONY : $(call gb_PrecompiledHeader_get_clean_target,%) $(call gb_NoExPrecompiledHeader_get_clean_target,%)
$(call gb_PrecompiledHeader_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PCH,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_PrecompiledHeader_get_target,$*) \
			$(call gb_PrecompiledHeader_get_target,$*).obj \
			$(call gb_PrecompiledHeader_get_target,$*).pdb \
			$(call gb_PrecompiledHeader_get_dep_target,$*))


$(call gb_NoexPrecompiledHeader_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PCH,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_NoexPrecompiledHeader_get_target,$*) \
			$(call gb_NoexPrecompiledHeader_get_target,$*).obj \
			$(call gb_NoexPrecompiledHeader_get_target,$*).pdb \
			$(call gb_NoexPrecompiledHeader_get_dep_target,$*))
endif

# vim: set noet sw=4:
