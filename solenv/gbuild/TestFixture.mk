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


# TestFixture class

# defined globally in TargetLocations.mk
#  gb_TestFixture_get_linktargetname
# defined by platform
#  gb_TestFixture_TARGETTYPEFLAGS
#  gb_TestFixture_get_filename
gb_TestFixture_CPPTESTTARGET := $(call gb_Executable_get_target,cppunittester)
gb_TestFixture_CPPTESTCOMMAND := $(gb_TestFixture_CPPTESTPRECOMMAND) $(gb_TestFixture_CPPTESTTARGET)

.PHONY : $(call gb_TestFixture_get_clean_target,%)
$(call gb_TestFixture_get_clean_target,%) : $(call gb_LinkTarget_get_clean_target,$(call gb_Library_get_linktargetname,%))
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $@)

.PHONY : $(call gb_TestFixture_get_target,%)
$(call gb_TestFixture_get_target,%) : $(gb_TestFixture_CPPTESTTARGET)
    $(call gb_Output_announce,$*,$(true),CUT,2)
    $(call gb_Helper_abbreviate_dirs_native,\
        $(gb_TestFixture_CPPTESTCOMMAND) $(call gb_LinkTarget_get_target,$(call gb_TestFixture_get_linktargetname,$(call gb_TestFixture_get_filename,$*))))

#		mkdir -p $(dir $@) && \
#		$(gb_TestFixture_CPPTESTCOMMAND) $(call gb_LinkTarget_get_target,$(call gb_TestFixture_get_linktargetname,$(call gb_TestFixture_get_filename,$*))) > $@)

define gb_TestFixture_TestFixture
$(call gb_TestFixture__TestFixture_impl,$(1),$(call gb_TestFixture_get_linktargetname,$(call gb_TestFixture_get_filename,$(1))))

endef

define gb_TestFixture__TestFixture_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype_flags,$(2),$(gb_TestFixture_TARGETTYPEFLAGS))
$(call gb_LinkTarget_add_linked_libs,$(2),cppunit)
$(call gb_TestFixture_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_TestFixture_get_target,$(1)),$(call gb_TestFixture_get_clean_target,$(1))))

endef

define gb_TestFixture__forward_to_Linktarget
gb_TestFixture_$(1) = $$(call gb_LinkTarget_$(1),$(call gb_TestFixture_get_linktargetname,$$(call gb_TestFixture_get_filename,$$(1))),$$(2),$$(3))

endef

$(eval $(foreach method,\
    add_cobject \
    add_cobjects \
    add_cxxobject \
    add_cxxobjects \
    add_objcxxobject \
    add_objcxxobjects \
    add_exception_objects \
    add_noexception_objects \
    set_cflags \
    set_cxxflags \
    set_objcxxflags \
    set_defs \
    set_include \
    set_ldflags \
    set_library_path_flags \
    add_linked_libs \
    add_linked_static_libs \
    add_package_headers \
    add_sdi_headers \
    add_precompiled_header \
,\
    $(call gb_TestFixture__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4 ts=4:
