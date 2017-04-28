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
$$(eval $$(call gb_Module_register_target,$(call gb_CompilerTest_get_target,$(1)),$(call gb_CompilerTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CompilerTest)

endef

define gb_CompilerTest__forward_to_Linktarget
gb_CompilerTest_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_CompilerTest_get_linktarget,$$(1)),$$(2),$$(3),CompilerTest_$$(1))

endef

$(eval $(foreach method, \
    add_cobject \
    add_cobjects \
    add_cxxobject \
    add_cxxobjects \
    add_exception_objects \
    add_objcobject \
    add_objcobjects \
    add_objcxxobject \
    add_objcxxobjects \
    add_cxxclrobject \
    add_cxxclrobjects \
    use_externals \
    use_udk_api \
, \
    $(call gb_CompilerTest__forward_to_Linktarget,$(method)) \
))

# vim: set noet sw=4 ts=4:
