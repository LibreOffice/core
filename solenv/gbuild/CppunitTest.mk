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

# CppunitTest class

gb_CppunitTest_UNITTESTFAILED ?= $(GBUILDDIR)/platform/unittest-failed-default.sh
gb_CppunitTest_PYTHONDEPS ?= $(call gb_Library_get_target,pyuno_wrapper) $(if $(SYSTEM_PYTHON),,$(call gb_Package_get_target,python3))

ifneq ($(strip $(CPPUNITTRACE)),)
ifneq ($(filter gdb,$(CPPUNITTRACE)),)
# sneak (a) setting the LD_LIBRARY_PATH, and (b) setting malloc debug flags, into the "gdb --args" command line
gb_CppunitTest_GDBTRACE := $(subst gdb,\
	gdb -return-child-result -ex "add-auto-load-safe-path $(INSTDIR)" -ex "set environment $(subst =, ,$(gb_CppunitTest_CPPTESTPRECOMMAND))" $(gb_CppunitTest_malloc_check) $(gb_CppunitTest_DEBUGCPPUNIT),\
	$(CPPUNITTRACE))
else ifneq ($(filter lldb,$(CPPUNITTRACE)),)
gb_CppunitTest_PREGDBTRACE := lo_dyldpathfile=$(call var2file,$(shell $(gb_MKTEMP)),500,settings set target.env-vars $(gb_CppunitTest_CPPTESTPRECOMMAND))
gb_CppunitTest_GDBTRACE := $(subst lldb,\
	lldb -s $$lo_dyldpathfile $(gb_CppunitTest_malloc_check),\
	$(CPPUNITTRACE))
gb_CppunitTest_POSTGDBTRACE := rm $$lo_dyldpathfile
else
gb_CppunitTest_GDBTRACE := $(CPPUNITTRACE)
endif
ifneq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest__interactive := $(true)
endif
endif

ifneq ($(strip $(VALGRIND)),)
gb_CppunitTest_VALGRINDTOOL := valgrind --tool=$(VALGRIND) --num-callers=50 --error-exitcode=1 --trace-children=yes --trace-children-skip='*/java,*/gij'
ifneq ($(strip $(VALGRIND_GDB)),)
gb_CppunitTest_VALGRINDTOOL += --vgdb=yes --vgdb-error=0
endif
ifeq ($(strip $(VALGRIND)),memcheck)
G_SLICE := always-malloc
GLIBCXX_FORCE_NEW := 1
endif
endif

ifneq (,$(filter perfcheck,$(MAKECMDGOALS)))
$(if $(ENABLE_VALGRIND),,$(call gb_Output_error,Running performance tests with empty $$(ENABLE_VALGRIND) does not make sense. Please install valgrind-dev and re-run autogen.))
gb_CppunitTest_VALGRINDTOOL := valgrind --tool=callgrind --dump-instr=yes --instr-atstart=no --simulate-cache=yes --dump-instr=yes --collect-bus=yes --branch-sim=yes
ifneq ($(strip $(VALGRIND_GDB)),)
gb_CppunitTest_VALGRINDTOOL += --vgdb=yes --vgdb-error=0
endif
endif

ifneq ($(strip $(RR)),)
gb_CppunitTest_RR := rr record
endif

# defined by platform
#  gb_CppunitTest_get_filename
gb_CppunitTest_RUNTIMEDEPS := $(call gb_Executable_get_runtime_dependencies,cppunittester)
gb_CppunitTest_CPPTESTCOMMAND := $(call gb_Executable_get_target_for_build,cppunittester)

# i18npool dlopens localedata_* libraries.
gb_CppunitTest_RUNTIMEDEPS += \
	$(call gb_Library_get_target,localedata_en) \
	$(call gb_Library_get_target,localedata_es) \
	$(call gb_Library_get_target,localedata_euro) \
	$(call gb_Library_get_target,localedata_others) \
	$(call gb_Package_get_target,test_unittest) \

define gb_CppunitTest__make_args
$(HEADLESS) \
"-env:BRAND_BASE_DIR=$(call gb_Helper_make_url,$(INSTROOT))" \
"-env:BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)" \
"-env:BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)" \
"-env:UserInstallation=$(call gb_Helper_make_url,$(call gb_CppunitTest_get_target,$*).user)" \
$(if $(URE),\
    $(if $(strip $(CONFIGURATION_LAYERS)),\
	    "-env:CONFIGURATION_LAYERS=$(strip $(CONFIGURATION_LAYERS))") \
    $(if $(strip $(UNO_TYPES)),\
	    "-env:UNO_TYPES=$(foreach item,$(UNO_TYPES),$(call gb_Helper_make_url,$(item)))") \
    $(if $(strip $(UNO_SERVICES)),\
	"-env:UNO_SERVICES=$(foreach item,$(UNO_SERVICES),$(call gb_Helper_make_url,$(item)))") \
	-env:URE_INTERNAL_LIB_DIR=$(call gb_Helper_make_url,$(INSTROOT)/$(LIBO_URE_LIB_FOLDER)) \
	-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(INSTROOT)/$(LIBO_LIB_FOLDER)) \
	-env:LO_JAVA_DIR=$(call gb_Helper_make_url,$(INSTROOT)/$(LIBO_SHARE_JAVA_FOLDER)) \
	--protector $(call gb_Library_get_target,unoexceptionprotector) unoexceptionprotector \
	--protector $(call gb_Library_get_target,unobootstrapprotector) unobootstrapprotector \
 ) \
$(if $(VCL),\
	--protector $(call gb_Library_get_target,vclbootstrapprotector) vclbootstrapprotector \
 ) \
$(ARGS)
endef

.PHONY : $(call gb_CppunitTest_get_clean_target,%)
$(call gb_CppunitTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CppunitTest_get_target,$*) $(call gb_CppunitTest_get_target,$*).log)

.PHONY : $(call gb_CppunitTest_get_target,%)
$(call gb_CppunitTest_get_target,%) :| $(gb_CppunitTest_RUNTIMEDEPS)
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$*,$(true),CUT,2)
	$(call gb_Trace_StartRange,$*,CUT)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		rm -fr $@.user && cp -r $(WORKDIR)/unittest $@.user && \
		$(if $(gb_CppunitTest__use_confpreinit), \
		    $(INSTDIR)/program/lokconf_init $(call gb_CppunitTest__make_args) &&) \
		$(if $(gb_CppunitTest__interactive),, \
			$(if $(value gb_CppunitTest_postprocess), \
				rm -fr $@.core && mkdir $@.core && cd $@.core &&)) \
		{ \
		$(if $(gb_CppunitTest_localized),for l in $(WITH_LANG_LIST) ; do \
			printf 'LO_TEST_LOCALE=%s\n' "$$l" && LO_TEST_LOCALE="$$l" ) \
		$(if $(gb_CppunitTest_PREGDBTRACE),$(gb_CppunitTest_PREGDBTRACE) &&) \
		$(if $(gb_CppunitTest__vcl_no_svp), \
			$(filter-out SAL_USE_VCLPLUGIN=svp,$(gb_TEST_ENV_VARS)),$(gb_TEST_ENV_VARS)) \
		$(EXTRA_ENV_VARS) \
		$(if $(filter gdb,$(gb_CppunitTest_GDBTRACE)),,$(gb_CppunitTest_CPPTESTPRECOMMAND)) \
		$(if $(G_SLICE),G_SLICE=$(G_SLICE)) \
		$(if $(GLIBCXX_FORCE_NEW),GLIBCXX_FORCE_NEW=$(GLIBCXX_FORCE_NEW)) \
		$(if $(strip $(PYTHON_URE)),\
			PYTHONDONTWRITEBYTECODE=1) \
		$(ICECREAM_RUN) $(gb_CppunitTest_GDBTRACE) $(gb_CppunitTest_VALGRINDTOOL) $(gb_CppunitTest_RR) \
			$(gb_CppunitTest_CPPTESTCOMMAND) \
		$(call gb_LinkTarget_get_target,$(call gb_CppunitTest_get_linktarget,$*)) \
		$(call gb_CppunitTest__make_args) "-env:CPPUNITTESTTARGET=$@" \
		$(if $(gb_CppunitTest_POSTGDBTRACE), \
			; RET=$$? && $(gb_CppunitTest_POSTGDBTRACE) && (exit $$RET)) \
		$(if $(gb_CppunitTest_localized),|| exit $$?; done) \
		; } \
		$(if $(gb_CppunitTest__interactive),, \
			> $@.log 2>&1 \
			|| ($(if $(value gb_CppunitTest_postprocess), \
					RET=$$?; \
					$(call gb_CppunitTest_postprocess,$(gb_CppunitTest_CPPTESTCOMMAND),$@.core,$$RET) >> $@.log 2>&1;) \
				cat $@.log; $(gb_CppunitTest_UNITTESTFAILED) Cppunit $*)))
	$(call gb_Trace_EndRange,$*,CUT)
endif

define gb_CppunitTest_CppunitTest
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest_get_linktarget,$(1)))

endef

define gb_CppunitTest_CppunitScreenShot
$(call gb_CppunitTest_get_target,$(1)) : gb_CppunitTest_localized := $(true)
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest_get_linktarget,$(1)))

endef

define gb_CppunitTest_register_target
endef

# call gb_CppunitTest__CppunitTest_impl,cppunittest,linktarget
define gb_CppunitTest__CppunitTest_impl
$(call gb_CppunitTest_register_target, $(1), $(2), "test")
$(call gb_LinkTarget_LinkTarget,$(2),CppunitTest_$(1),NONE)
$(call gb_LinkTarget_set_targettype,$(2),CppunitTest)
$(call gb_LinkTarget_add_libs,$(2),$(gb_STDLIBS))
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
$(call gb_CppunitTest_CppunitTest_platform,$(1),$(2),$(gb_CppunitTest_DLLDIR)/$(call gb_CppunitTest_get_ilibfilename,$(1)))
$(call gb_CppunitTest_get_target,$(1)) : ARGS :=
$(call gb_CppunitTest_get_target,$(1)) : CONFIGURATION_LAYERS :=
$(call gb_CppunitTest_get_target,$(1)) : PYTHON_URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : VCL := $(false)
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES :=
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES :=
$(call gb_CppunitTest_get_target,$(1)) : HEADLESS := --headless
$(call gb_CppunitTest_get_target,$(1)) : EXTRA_ENV_VARS :=
$$(eval $$(call gb_Module_register_target,$(call gb_CppunitTest_get_target,$(1)),$(call gb_CppunitTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CppunitTest)

endef

# Add additional command line arguments for the test.
#
# You should practically never need to use this, as there are special
# functions for adding many commonly used arguments.
define gb_CppunitTest_add_arguments
$(call gb_CppunitTest_get_target,$(1)) : ARGS += $(2)

endef

define gb_CppunitTest_use_ure
$(call gb_CppunitTest_use_rdb,$(1),ure/services)
$(call gb_CppunitTest_get_target,$(1)) : URE := $(true)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,$(gb_CPPU_ENV)_uno)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,affine_uno)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,unobootstrapprotector)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,unoexceptionprotector)

endef

# $(2) == $(true) if headless:
define gb_CppunitTest__use_vcl
$(call gb_CppunitTest_get_target,$(1)) : VCL := $(true)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,vclbootstrapprotector)
ifeq ($(USING_X11),TRUE)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,desktop_detector)
$(call gb_CppunitTest_get_target,$(1)) : $(if $(filter $(2),$(true)),, \
    $(call gb_Library_get_target,vclplug_gen) \
        $(if $(ENABLE_GTK3),$(call gb_Library_get_target,vclplug_gtk3)) \
        $(if $(ENABLE_QT5),$(call gb_Library_get_target,vclplug_qt5)) \
	 )
else ifeq ($(OS),MACOSX)
$(call gb_CppunitTest_get_target,$(1)): $(call gb_Library_get_target,vclplug_osx)
else ifeq ($(OS),WNT)
$(call gb_CppunitTest_get_target,$(1)): $(call gb_Library_get_target,vclplug_win)
endif

endef

define gb_CppunitTest_use_confpreinit
$(call gb_CppunitTest_use_executable,$(1),lokconf_init)
$(call gb_CppunitTest_get_target,$(1)) : gb_CppunitTest__use_confpreinit := TRUE

endef

define gb_CppunitTest_use_vcl
$(call gb_CppunitTest__use_vcl,$(1),$(true))

endef

define gb_CppunitTest_use_vcl_non_headless
$(call gb_CppunitTest_get_target,$(1)) : gb_CppunitTest__vcl_no_svp := $(true)
$(call gb_CppunitTest__use_vcl,$(1),$(false))

endef

define gb_CppunitTest_use_vcl_non_headless_with_windows
$(call gb_CppunitTest_get_target,$(1)) : HEADLESS :=
$(call gb_CppunitTest_get_target,$(1)) : gb_CppunitTest__vcl_no_svp := $(true)
$(call gb_CppunitTest__use_vcl,$(1),$(false))

endef

define gb_CppunitTest_localized_run
$(call gb_CppunitTest_get_target,$(1)) : gb_CppunitTest_localized := $(true)

endef

define gb_CppunitTest__use_api
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_UnoApi_get_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES += $(call gb_UnoApi_get_target,$(2))

endef

define gb_CppunitTest_use_api
$(call gb_LinkTarget_use_api,$(call gb_CppunitTest_get_linktarget,$(1)),$(2))
$(foreach rdb,$(2),$(call gb_CppunitTest__use_api,$(1),$(rdb)))

endef

define gb_CppunitTest_use_udk_api
$(call gb_CppunitTest_use_api,$(1),udkapi)

endef

define gb_CppunitTest_use_sdk_api
$(call gb_CppunitTest_use_api,$(1),udkapi offapi)

endef

define gb_CppunitTest_use_rdb
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Rdb_get_target_for_build,$(2))
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES += $(call gb_Rdb_get_target_for_build,$(2))

endef

define gb_CppunitTest_use_rdbs
$(foreach rdb,$(2),$(call gb_CppunitTest_use_rdb,$(1),$(rdb)))

endef

define gb_CppunitTest_use_component
$(call gb_CppunitTest_get_target,$(1)) : \
    $(call gb_ComponentTarget_get_target,$(2))
$(call gb_CppunitTest_get_target,$(1)) : \
    UNO_SERVICES += $(call gb_ComponentTarget_get_target,$(2))

endef

define gb_CppunitTest_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,CppunitTest/$(2),\
    $(call gb_Helper_make_url,$(gb_CppunitTest_DLLDIR))/,\
    $(call gb_CppunitTest_get_filename,$(1)))
$(call gb_CppunitTest_get_target,$(1)) : \
    $(call gb_ComponentTarget_get_target,CppunitTest/$(2))
$(call gb_CppunitTest_get_clean_target,$(1)) : \
    $(call gb_ComponentTarget_get_clean_target,CppunitTest/$(2))
$(call gb_CppunitTest_use_component,$(1),CppunitTest/$(2))

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

define gb_CppunitTest_use_components
$(foreach component,$(call gb_CppunitTest__filter_not_built_components,$(2)),$(call gb_CppunitTest_use_component,$(1),$(component)))

endef

define gb_CppunitTest__use_configuration
$(call gb_CppunitTest_get_target,$(1)) : CONFIGURATION_LAYERS += $(2):$(call gb_Helper_make_url,$(3))

endef

# Use instdir configuration
define gb_CppunitTest_use_instdir_configuration
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Package_get_target,postprocess_registry)
$(call gb_CppunitTest__use_configuration,$(1),xcsxcu,$(INSTROOT)/$(LIBO_SHARE_FOLDER)/registry)

endef

# Use configuration in $(WORKDIR)/unittest/registry.
define gb_CppunitTest_use_unittest_configuration
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Package_get_target,test_unittest)
$(call gb_CppunitTest__use_configuration,$(1),xcsxcu,$(WORKDIR)/unittest/registry)

endef

# Use standard configuration: instdir config + unittest config (in this order!)
define gb_CppunitTest_use_configuration
$(call gb_CppunitTest_use_instdir_configuration,$(1))
$(call gb_CppunitTest_use_unittest_configuration,$(1))

endef

define gb_CppunitTest_use_executable
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Executable_get_target,$(2))

endef

define gb_CppunitTest_use_more_fonts
ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(call gb_CppunitTest_get_target,$(1)) : \
    $(foreach font,$(gb_Package_MODULE_ooo_fonts),$(call gb_Package_get_target,$(font)))
endif

endef

define gb_CppunitTest__use_java_ure
$(call gb_CppunitTest_get_target,$(1)) : \
    $(foreach jar,java_uno libreoffice unoloader,$(call gb_Jar_get_target,$(jar))) \
    $(call gb_Library_get_target,affine_uno_uno) \
    $(call gb_Library_get_target,java_uno) \
    $(call gb_Library_get_target,jpipe) \
    $(call gb_Library_get_target,juhx) \
    $(call gb_Library_get_target,juhx) \
    $(call gb_Library_get_target,jvmaccess) \
    $(call gb_Library_get_target,jvmfwk) \
    $(call gb_Package_get_target,jvmfwk_javavendors) \
    $(call gb_Package_get_target,jvmfwk_jreproperties) \
    $(call gb_Package_get_target,jvmfwk_jvmfwk3_ini) \

endef

define gb_CppunitTest_use_jar
$(call gb_CppunitTest__use_java_ure,$(1))
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Jar_get_target,$(2))

endef

define gb_CppunitTest_use_jars
$(foreach jar,$(2),$(call gb_CppunitTest_use_jar,$(1),$(jar)))

endef

define gb_CppunitTest_use_python_ure
$(call gb_CppunitTest_get_target,$(1)) : PYTHON_URE := $(true)
$(call gb_CppunitTest_get_target,$(1)) :\
	$(call gb_Library_get_target,pythonloader) \
	$(call gb_Library_get_target,pyuno) \
	$(gb_CppunitTest_PYTHONDEPS) \
	$(call gb_Package_get_target,pyuno_python_scripts)

endef

define gb_CppunitTest_use_uiconfig
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_UIConfig_get_target,$(2))

endef

define gb_CppunitTest_use_uiconfigs
$(foreach uiconfig,$(2),$(call gb_CppunitTest_use_uiconfig,$(1),$(uiconfig)))

endef

# forward the call to the gb_LinkTarget implementation
# (note: because the function name is in $(1), the other args are shifted by 1)
define gb_CppunitTest__forward_to_Linktarget
$(call gb_LinkTarget_$(subst gb_CppunitTest_,,$(1)),$(call gb_CppunitTest_get_linktarget,$(2)),$(3),$(4),CppunitTest_$(2))

endef

# copy pasta for forwarding: this could be (and was) done more elegantly, but
# these here can be found by both git grep and ctags
gb_CppunitTest_add_cobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_exception_objects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_executable_objects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_library_objects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_libraries = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_static_libraries = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcxxobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcxxobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxclrobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxclrobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_asmobject = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_asmobjects = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_package = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_packages = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_precompiled_header = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_reuse_precompiled_header = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_common_precompiled_header = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_sdi_headers = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_cflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_yaccflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_objcxxflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_cxxclrflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_defs = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_include = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_ldflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_ldflags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_add_libs = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_disable_standard_system_libs = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_system_darwin_frameworks = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_system_win32_libs = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_internal_api = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_internal_bootstrap_api = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_internal_comprehensive_api = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_library_path_flags = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_external = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_externals = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_custom_headers = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_visibility_default = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_warnings_not_errors = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_warnings_disabled = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_external_code = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_generated_cxx_suffix = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_use_clang = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_CppunitTest_set_clang_precompiled_header = $(call gb_CppunitTest__forward_to_Linktarget,$(0),$(1),$(2),$(3))

# vim: set noet sw=4:
