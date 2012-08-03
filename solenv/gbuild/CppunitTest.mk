# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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


UNIT_FAILED_MSG := echo; echo "Error: a unit test failed, please do one of:"; echo; echo "export DEBUGCPPUNIT=TRUE            \# for exception catching"; echo "export GDBCPPUNITTRACE=\"gdb --args\" \# for interactive debugging"; echo "export VALGRIND=memcheck            \# for memory checking" ; echo "and retry."

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest_GDBTRACE := gdb -nx --command=$(SOLARENV)/bin/gdbtrycatchtrace-stdout -return-child-result --args
else ifneq ($(strip $(GDBCPPUNITTRACE)),)
gb_CppunitTest_GDBTRACE := $(GDBCPPUNITTRACE)
gb_CppunitTest__interactive := $(true)
endif

ifneq ($(strip $(VALGRIND)),)
gb_CppunitTest_VALGRINDTOOL := valgrind --tool=$(VALGRIND) --num-callers=50
ifeq ($(strip $(VALGRIND)),memcheck)
G_SLICE := always-malloc
GLIBCXX_FORCE_NEW := 1
endif
endif

# defined by platform
#  gb_CppunitTest_TARGETTYPE
#  gb_CppunitTest_get_filename
# DBGSV_ERROR_OUT => in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
ifneq ($(OS),IOS)
gb_CppunitTest_CPPTESTTARGET := $(call gb_Executable_get_target,cppunit/cppunittester)
endif

gb_CppunitTest__get_linktargetname = CppunitTest/$(call gb_CppunitTest_get_filename,$(1))

define gb_CppunitTest__make_args
--headless \
$(if $(URE),\
    $(if $(strip $(CONFIGURATION_LAYERS)),\
	    "-env:CONFIGURATION_LAYERS=$(strip $(CONFIGURATION_LAYERS))") \
    $(if $(strip $(UNO_TYPES)),\
	    "-env:UNO_TYPES=$(foreach item,$(UNO_TYPES),$(call gb_Helper_make_url,$(item)))") \
    $(if $(strip $(UNO_SERVICES)),\
	"-env:UNO_SERVICES=$(foreach item,$(UNO_SERVICES),$(call gb_Helper_make_url,$(item)))") \
    $(foreach dir,URE_INTERNAL_LIB_DIR LO_LIB_DIR,\
	    -env:$(dir)=$(call gb_Helper_make_url,$(gb_CppunitTest_LIBDIR))) \
    --protector unoexceptionprotector$(gb_Library_DLLEXT) unoexceptionprotector \
    --protector unobootstrapprotector$(gb_Library_DLLEXT) unobootstrapprotector \
 ) $(ARGS)
endef

.PHONY : $(call gb_CppunitTest_get_clean_target,%)
$(call gb_CppunitTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CppunitTest_get_target,$*) $(call gb_CppunitTest_get_target,$*).log)

.PHONY : $(call gb_CppunitTest_get_target,%)
$(call gb_CppunitTest_get_target,%) :| $(gb_CppunitTest_CPPTESTTARGET)
	$(call gb_Output_announce,$*,$(true),CUT,2)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		($(gb_CppunitTest_CPPTESTPRECOMMAND) \
		$(if $(G_SLICE),G_SLICE=$(G_SLICE)) \
		$(if $(GLIBCXX_FORCE_NEW),GLIBCXX_FORCE_NEW=$(GLIBCXX_FORCE_NEW)) \
		$(if $(DBGSV_ERROR_OUT),DBGSV_ERROR_OUT=$(DBGSV_ERROR_OUT)) \
		DISABLE_SAL_DBGBOX=t \
		$(if $(SAL_DIAGNOSE_ABORT),SAL_DIAGNOSE_ABORT=$(SAL_DIAGNOSE_ABORT)) \
		STAR_RESOURCEPATH=$(dir $(call gb_ResTarget_get_outdir_target,example)) \
		$(gb_CppunitTest_GDBTRACE) $(gb_CppunitTest_VALGRINDTOOL) $(gb_CppunitTest_CPPTESTTARGET) \
		$(call gb_LinkTarget_get_target,CppunitTest/$(call gb_CppunitTest_get_libfilename,$*)) \
		$(call gb_CppunitTest__make_args) \
		$(if $(gb_CppunitTest__interactive),,> $@.log 2>&1 || (cat $@.log && $(UNIT_FAILED_MSG) && false))))

define gb_CppunitTest_CppunitTest
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest__get_linktargetname,$(1)))

endef

define gb_CppunitTest__CppunitTest_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),CppunitTest)
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_CppunitTest_DEFS) \
)
$(call gb_LinkTarget_use_external,$(2),cppunit)
$(call gb_LinkTarget_set_include,$(2),\
    $$(INCLUDE) \
    $(filter -I%,$(CPPUNIT_CFLAGS)) \
)
$(call gb_LinkTarget_add_defs,$(2), \
    $(filter-out -I%,$(CPPUNIT_CFLAGS)) \
	-DCPPUNIT_PLUGIN_EXPORT='extern "C" SAL_DLLPUBLIC_EXPORT' \
)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_CppunitTest_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_CppunitTest_CppunitTest_platform,$(1),$(2),$(gb_CppunitTest_DLLDIR)/$(call gb_CppunitTest_get_libfilename,$(1)))
$(call gb_CppunitTest_get_target,$(1)) : ARGS :=
$(call gb_CppunitTest_get_target,$(1)) : CONFIGURATION_LAYERS :=
$(call gb_CppunitTest_get_target,$(1)) : URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES :=
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES :=
$(call gb_CppunitTest_get_target,$(1)) : DBGSV_ERROR_OUT := shell
$(call gb_CppunitTest_get_target,$(1)) : SAL_DIAGNOSE_ABORT :=
$$(eval $$(call gb_Module_register_target,$(call gb_CppunitTest_get_target,$(1)),$(call gb_CppunitTest_get_clean_target,$(1))))

endef

define gb_CppunitTest_abort_on_assertion
$(call gb_CppunitTest_get_target,$(1)) : DBGSV_ERROR_OUT := abort
$(call gb_CppunitTest_get_target,$(1)) : SAL_DIAGNOSE_ABORT := TRUE

endef

define gb_CppunitTest_set_args
$$(call gb_Output_error,gb_CppunitTest_set_args: use gb_CppunitTest_add_arguments instead.))
endef

# Add additional command line arguments for the test.
#
# You should practically never need to use this, as there are special
# functions for adding many commonly used arguments.
define gb_CppunitTest_add_arguments
$(call gb_CppunitTest_get_target,$(1)) : ARGS += $(2)

endef

define gb_CppunitTest_uses_ure
$$(call gb_Output_error,gb_CppunitTest_uses_ure: use gb_CppunitTest_use_ure instead.))
endef

define gb_CppunitTest_use_ure
$(call gb_CppunitTest_use_rdb,$(1),ure/services)
$(call gb_CppunitTest_get_target,$(1)) : URE := $(true)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,unobootstrapprotector)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,unoexceptionprotector)

endef

define gb_CppunitTest_add_type_rdb
$$(call gb_Output_error,\
 gb_CppunitTest_add_type_rdb: use gb_CppunitTest_use_api instead.)
endef

define gb_CppunitTest_use_type_rdb
$$(call gb_Output_error,\
 gb_CppunitTest_use_type_rdb: use gb_CppunitTest_use_api instead.)
endef

define gb_CppunitTest__use_api
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_UnoApi_get_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES += $(call gb_UnoApi_get_target,$(2))

endef

define gb_CppunitTest_add_type_rdbs
$$(call gb_Output_error,\
 gb_CppunitTest_add_type_rdbs: use gb_CppunitTest_use_api instead.)
endef

define gb_CppunitTest_use_type_rdbs
$$(call gb_Output_error,\
 gb_CppunitTest_use_type_rdbs: use gb_CppunitTest_use_api instead.)
endef

define gb_CppunitTest_use_api
$(call gb_LinkTarget_use_api,$(call gb_CppunitTest__get_linktargetname,$(1)),$(2))
$(foreach rdb,$(2),$(call gb_CppunitTest__use_api,$(1),$(rdb)))

endef

define gb_CppunitTest_add_service_rdb
$$(call gb_Output_error,\
 gb_CppunitTest_add_service_rdb: use gb_CppunitTest_use_rdb instead.)
endef

define gb_CppunitTest_use_service_rdb
$$(call gb_Output_error,gb_CppunitTest_use_service_rdb: use gb_CppunitTest_use_rdb instead.))
endef

define gb_CppunitTest_use_rdb
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Rdb_get_outdir_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES += $(call gb_Rdb_get_outdir_target,$(2))

endef

define gb_CppunitTest_add_service_rdbs
$$(call gb_Output_error,\
 gb_CppunitTest_add_service_rdbs: use gb_CppunitTest_use_rdbs instead.)
endef

define gb_CppunitTest_use_service_rdbs
$$(call gb_Output_error,gb_CppunitTest_use_service_rdbs: use gb_CppunitTest_use_rdbs instead.))
endef

define gb_CppunitTest_use_rdbs
$(foreach rdb,$(2),$(call gb_CppunitTest_use_rdb,$(1),$(rdb)))

endef

define gb_CppunitTest_add_component
$$(call gb_Output_error,\
 gb_CppunitTest_add_component: use gb_CppunitTest_use_component instead.)
endef

define gb_CppunitTest_use_component
$(call gb_CppunitTest_get_target,$(1)) : \
    $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : \
    UNO_SERVICES += $(call gb_ComponentTarget_get_outdir_target,$(2))

endef

# Given a list of component files, filter out those corresponding
# to libraries not built in this configuration.
define gb_CppunitTest__filter_not_built_components
$(filter-out \
	$(if $(filter SCRIPTING,$(BUILD_TYPE)),, \
		basic/util/sb \
	    sw/util/vbaswobj \
	    scripting/source/basprov/basprov \
	    scripting/util/scriptframe) \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),, \
	    dbaccess/util/dba \
		forms/util/frm),$(1))
endef

define gb_CppunitTest_add_components
$$(call gb_Output_error,\
 gb_CppunitTest_add_components: use gb_CppunitTest_use_components instead.)
endef

define gb_CppunitTest_use_components
$(foreach component,$(call gb_CppunitTest__filter_not_built_components,$(2)),$(call gb_CppunitTest_use_component,$(1),$(component)))

endef

define gb_CppunitTest_add_old_component
$$(call gb_Output_error,\
 gb_CppunitTest_add_old_component: use gb_CppunitTest_use_old_component instead.)
endef

define gb_CppunitTest_use_old_component
$(call gb_CppunitTest_get_target,$(1)) : \
    $(call gb_CppunitTest__get_old_component_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : \
    UNO_SERVICES += $(call gb_ComponentTarget__get_old_component_target,$(2))

endef

define gb_CppunitTest_add_old_components
$$(call gb_Output_error,\
 gb_CppunitTest_add_old_components: use gb_CppunitTest_use_old_components instead.)
endef

define gb_CppunitTest_use_old_components
$(foreach component,$(2),$(call gb_CppunitTest_use_old_component,$(1),$(component)))

endef

gb_ComponentTarget__get_old_component_target = $(OUTDIR)/xml/$(1).component

define gb_CppunitTest__use_configuration
$(call gb_CppunitTest_get_target,$(1)) : CONFIGURATION_LAYERS += $(2):$(call gb_Helper_make_url,$(3))

endef

# Use standard configuration.
define gb_CppunitTest_use_configuration
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Configuration_get_target,officecfg)
$(call gb_CppunitTest__use_configuration,$(1),xcsxcu,$(gb_Configuration_registry))

endef

# Use configuration for filters.
#
# Okay, this is not exactly true, because there may be configuration
# for more things than just filters in spool, but it is good enough.
define gb_CppunitTest_use_filter_configuration
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Configuration_get_target,fcfg_langpack)
$(call gb_CppunitTest__use_configuration,$(1),module,$(gb_Configuration_registry)/spool)

endef

# Use extra configuration dir(s).
define gb_CppunitTest_use_extra_configuration
$(foreach extra,$(2),$(call gb_CppunitTest__use_configuration,$(1),xcsxcu,$(extra)))

endef

define gb_CppunitTest_use_executable
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Executable_get_target,$(2))
endef

define gb_CppunitTest__forward_to_Linktarget
gb_CppunitTest_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_CppunitTest__get_linktargetname,$$(1)),$$(2),$$(3),CppunitTest_$$(1))

endef

$(eval $(foreach method,\
	add_api \
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_exception_objects \
	add_executable_objects \
	use_executable_objects \
	add_library_objects \
	use_library_objects \
	add_linked_libs \
	use_libraries \
	add_linked_static_libs \
	use_static_libraries \
	add_noexception_objects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_asmobject \
	add_asmobjects \
	add_package_headers \
	use_package \
	use_packages \
	add_sdi_headers \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_yaccflags \
	add_objcflags \
	add_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	add_libs \
	use_sdk_api \
	use_udk_api \
	use_internal_api \
	use_internal_bootstrap_api \
	use_internal_comprehensive_api \
	set_library_path_flags \
	set_objcflags \
	set_objcxxflags \
	use_external \
	use_externals \
,\
	$(call gb_CppunitTest__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
