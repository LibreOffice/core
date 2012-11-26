#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************




# CppunitTest class

# in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
DBGSV_ERROR_OUT := shell
export DBGSV_ERROR_OUT

# defined by platform
#  gb_CppunitTest_TARGETTYPE
#  gb_CppunitTest_get_filename
gb_CppunitTest_CPPTESTTARGET := $(call gb_Executable_get_target,cppunittester)
gb_CppunitTest_CPPTESTCOMMAND := $(gb_CppunitTest_CPPTESTPRECOMMAND) $(gb_CppunitTest_CPPTESTTARGET)
gb_CppunitTest__get_linktargetname = CppunitTest/$(call gb_CppunitTest_get_filename,$(1))

.PHONY : $(call gb_CppunitTest_get_clean_target,%)
$(call gb_CppunitTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CppunitTest_get_target,$*) $(call gb_CppunitTest_get_target,$*).log)

.PHONY : $(call gb_CppunitTest_get_target,%)
$(call gb_CppunitTest_get_target,%) : $(gb_CppunitTest_CPPTESTTARGET)
	$(call gb_Output_announce,$*,$(true),CUT,2)
	$(call gb_Helper_abbreviate_dirs_native,\
		mkdir -p $(dir $@) && \
		$(gb_CppunitTest_CPPTESTCOMMAND) $(call gb_LinkTarget_get_target,CppunitTest/$(call gb_CppunitTest_get_libfilename,$*)) > $@.log 2>&1 || (cat $@.log && false))

define gb_CppunitTest_CppunitTest
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest__get_linktargetname,$(1)))

endef

define gb_CppunitTest__CppunitTest_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),CppunitTest)
$(call gb_LinkTarget_add_linked_libs,$(2),cppunit)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_CppunitTest_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_CppunitTest_CppunitTest_platform,$(1),$(2),$(gb_CppunitTest_DLLDIR)/$(call gb_CppunitTest_get_libfilename,$(1)))
$$(eval $$(call gb_Module_register_target,$(call gb_CppunitTest_get_target,$(1)),$(call gb_CppunitTest_get_clean_target,$(1))))

endef

define gb_CppunitTest__forward_to_Linktarget
gb_CppunitTest_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_CppunitTest__get_linktargetname,$$(1)),$$(2),$$(3))

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
	$(call gb_CppunitTest__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4 ts=4:
