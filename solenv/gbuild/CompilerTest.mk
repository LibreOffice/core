# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

define gb_CompilerTest_CompilerTest
$(call gb_CompilerTest__CompilerTest_impl,$(1),$(call gb_CompilerTest_get_linktarget,$(1)))

endef

# call gb_CompilerTest__CompilerTest_impl,compilertest,linktarget
define gb_CompilerTest__CompilerTest_impl
$(call gb_LinkTarget_LinkTarget,$(2),CompilerTest_$(1),NONE)
$(call gb_LinkTarget_set_targettype,$(2),CompilerTest)
$(call gb_LinkTarget_get_target,$(2)): COMPILER_TEST := $(true)
$(call gb_LinkTarget_get_target,$(2)): ENABLE_WERROR := $(true)
$(call gb_CompilerTest_get_target,$(1)): $(call gb_LinkTarget_get_target,$(2))
$(call gb_CompilerTest_get_clean_target,$(1)): $(call gb_LinkTarget_get_clean_target,$(2))
$$(eval $$(call gb_Module_register_target,CompilerTest_$(1),$(call gb_CompilerTest_get_target,$(1)),$(call gb_CompilerTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CompilerTest)

endef

# forward the call to the gb_LinkTarget implementation
# (note: because the function name is in $(1), the other args are shifted by 1)
define gb_CompilerTest__forward_to_Linktarget
$(call gb_LinkTarget_$(1),$(call gb_CompilerTest_get_linktarget,$(2)),$(3),$(4),CompilerTest_$(2))

endef

# copy pasta for forwarding: this could be (and was) done more elegantly, but
# these here can be found by both git grep and ctags
gb_CompilerTest_add_cobject = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_cobjects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_cxxobject = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_cxxobjects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_exception_objects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_objcobject = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_objcobjects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_objcxxobject = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_objcxxobjects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_cxxclrobject = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_add_cxxclrobjects = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_use_externals = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))
gb_CompilerTest_use_udk_api = $(call gb_CompilerTest__forward_to_Linktarget,$(subst gb_CompilerTest_,,$(0)),$(1),$(2),$(3))

# vim: set noet sw=4 ts=4:
