#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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


# CppunitTest class

# in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
DBGSV_ERROR_OUT := shell
export DBGSV_ERROR_OUT

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest_GDBTRACE := gdb -nx --command=$(SOLARENV)/bin/gdbtrycatchtrace-stdout -return-child-result --args
else ifneq ($(strip $(GDBCPPUNITTRACE)),)
gb_CppunitTest_GDBTRACE := $(GDBCPPUNITTRACE)
gb_CppunitTest__interactive := $(true)
endif

# defined by platform
#  gb_CppunitTest_TARGETTYPE
#  gb_CppunitTest_get_filename
gb_CppunitTest_CPPTESTTARGET := $(call gb_Executable_get_target,cppunit/cppunittester)
gb_CppunitTest_CPPTESTCOMMAND := $(gb_CppunitTest_CPPTESTPRECOMMAND) STAR_RESOURCEPATH=$(dir $(call gb_ResTarget_get_outdir_target,example)) LANG=en_US.UTF-8 $(gb_CppunitTest_GDBTRACE) $(gb_CppunitTest_CPPTESTTARGET)
gb_CppunitTest__get_linktargetname = CppunitTest/$(call gb_CppunitTest_get_filename,$(1))

# TODO: move this to platform under suitable name
gb_CppunitTarget__make_url = file://$(if $(filter WNT,$(OS)),/)$(1)

gb_CppunitTest__get_uno_type_target = $(OUTDIR)/bin/$(1).rdb
define gb_CppunitTest__make_args
$(ARGS) \
$(if $(strip $(UNO_TYPES)),\
	"-env:UNO_TYPES=$(foreach rdb,udkapi $(UNO_TYPES),\
		$(call gb_CppunitTarget__make_url,$(call gb_CppunitTest__get_uno_type_target,$(rdb))))") \
$(if $(strip $(UNO_SERVICES)),\
	"-env:UNO_SERVICES=$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/ure/services.rdb) \
		$(foreach rdb,$(UNO_SERVICES),\
			$(call gb_CppunitTarget__make_url,$(call gb_RdbTarget_get_target,$(rdb))))") \
$(if $(URE),\
	$(foreach dir,URE_INTERNAL_LIB_DIR OOO_BASE_DIR BRAND_BASE_DIR,\
		-env:$(dir)=file://$(if $(filter WNT,$(OS)),/$(OUTDIR)/bin,$(OUTDIR)/lib)))
endef

.PHONY : $(call gb_CppunitTest_get_clean_target,%)
$(call gb_CppunitTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CppunitTest_get_target,$*) $(call gb_CppunitTest_get_target,$*).log)

.PHONY : $(call gb_CppunitTest_get_target,%)
$(call gb_CppunitTest_get_target,%) :| $(gb_CppunitTest_CPPTESTTARGET)
	$(call gb_Output_announce,$*,$(true),CUT,2)
	$(call gb_Helper_abbreviate_dirs_native,\
		mkdir -p $(dir $@) && \
		$(gb_CppunitTest_CPPTESTCOMMAND) $(call gb_LinkTarget_get_target,CppunitTest/$(call gb_CppunitTest_get_libfilename,$*)) $(call gb_CppunitTest__make_args,$(ARGS),$(UNO_SERVICES),$(UNO_TYPES)) $(if $(gb_CppunitTest__interactive),,> $@.log 2>&1 || (cat $@.log && false)))

define gb_CppunitTest_CppunitTest
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest__get_linktargetname,$(1)))

endef

define gb_CppunitTest__CppunitTest_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),CppunitTest)
$(call gb_LinkTarget_add_linked_libs,$(2),cppunit)
$(call gb_LinkTarget_add_includes,$(2),$(filter -I%,$(CPPUNIT_CFLAGS)))
$(call gb_LinkTarget_set_defs,$(2), \
    $$(DEFS) \
    $(filter-out -I%,$(CPPUNIT_CFLAGS)) \
)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_CppunitTest_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_CppunitTest_CppunitTest_platform,$(1),$(2),$(gb_CppunitTest_DLLDIR)/$(call gb_CppunitTest_get_libfilename,$(1)))
$(call gb_CppunitTest_get_target,$(1)) : ARGS :=
$(call gb_CppunitTest_get_target,$(1)) : URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES :=
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES :=
$$(eval $$(call gb_Module_register_target,$(call gb_CppunitTest_get_target,$(1)),$(call gb_CppunitTest_get_clean_target,$(1))))

endef

define gb_CppunitTest_set_args
$(call gb_CppunitTest_get_target,$(1)) : ARGS := $(2)

endef

# TODO: Once we build the services.rdb with gbuild we should use its *_get_target method
define gb_CppunitTest_uses_ure
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_CppunitTest__get_uno_type_target,udkapi)
$(call gb_CppunitTest_get_target,$(1)) : $(OUTDIR)/xml/ure/services.rdb
$(call gb_CppunitTest_get_target,$(1)) : URE := $(true)

endef

define gb_CppunitTest_add_type_rdb
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_CppunitTest__get_uno_type_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES += $(2)
endef

define gb_CppunitTest_add_type_rdbs
$(foreach rdb,$(2),$(eval $(call gb_CppunitTest_add_type_rdb,$(1),$(rdb))))
endef

define gb_CppunitTest_add_service_rdb
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_RdbTarget_get_target,$(2))
$(call gb_CppunitTest_get_clean_target,$(1)) : $(call gb_RdbTarget_get_clean_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES += $(2)
endef

define gb_CppunitTest_add_service_rdbs
$(foreach rdb,$(2),$(eval $(call gb_CppunitTest_add_service_rdb,$(1),$(rdb))))
endef

define gb_CppunitTest__forward_to_Linktarget
gb_CppunitTest_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_CppunitTest__get_linktargetname,$$(1)),$$(2),$$(3))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_exception_objects \
	add_executable_objects \
	add_library_objects \
	add_linked_libs \
	add_linked_static_libs \
	add_noexception_objects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_package_headers \
	add_precompiled_header \
	add_sdi_headers \
	set_cflags \
	set_cxxflags \
	set_defs \
	set_include \
	set_ldflags \
	set_library_path_flags \
	set_objcflags \
	set_objcxxflags \
,\
	$(call gb_CppunitTest__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
