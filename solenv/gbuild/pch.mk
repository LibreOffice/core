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

$(call gb_PrecompiledHeader_get_target,%) : $(SRCDIR)/%.cxx
    $(call gb_PrecompiledHeader__command,$@,$*,$<,$(DEFS),$(CXXFLAGS),$(INCLUDE_STL) $(INCLUDE))

.PHONY : $(call gb_PrecompiledHeader_get_clean_target,%)
$(call gb_PrecompiledHeader_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up pch $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_PrecompiledHeader_get_target,$*))

define gb_PrecompiledHeader_PrecompiledHeader
$(call gb_PrecompiledHeader_get_dep_target,$(1)) \
$(call gb_PrecompiledHeader_get_target,$(1)) : CXXFLAGS := $$(gb_PrecompiledHeader_CXXFLAGS)
$(call gb_PrecompiledHeader_get_dep_target,$(1)) \
$(call gb_PrecompiledHeader_get_target,$(1)) : DEFS := $$(gb_PrecompiledHeader_DEFAULTDEFS)
$(call gb_PrecompiledHeader_get_dep_target,$(1)) \
$(call gb_PrecompiledHeader_get_target,$(1)) : INCLUDE := $$(gb_PrecompiledHeader_INCLUDE)
$(call gb_PrecompiledHeader_get_dep_target,$(1)) \
$(call gb_PrecompiledHeader_get_target,$(1)) : INCLUDE_STL := $$(gb_PrecompiledHeader_INCLUDE_STL)

endef

define gb_PrecompiledHeader_set_defs
$(call gb_PrecompiledHeader_get_target,$(1)) : DEFS := $(2)
$(call gb_PrecompiledHeader_get_dep_target,$(1)) : DEFS := $(2)
endef

define gb_PrecompiledHeader_set_cxxflags
$(call gb_PrecompiledHeader_get_target,$(1)) \
$(call gb_PrecompiledHeader_get_dep_target,$(1)) : CXXFLAGS := $(2)
endef

define gb_PrecompiledHeader_set_include
$(call gb_PrecompiledHeader_get_target,$(1)) \
$(call gb_PrecompiledHeader_get_dep_target,$(1)) : INCLUDE := $(2)
endef

define gb_PrecompiledHeader_set_include_stl
$(call gb_PrecompiledHeader_get_target,$(1)) \
$(call gb_PrecompiledHeader_get_dep_target,$(1)) : INCLUDE_STL := $(2)
endef

# vim: set noet sw=4 ts=4:
