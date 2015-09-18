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

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest_GDBTRACE := gdb -nx -ex "add-auto-load-safe-path $(INSTDIR)" --batch --command=$(SRCDIR)/solenv/bin/gdbtrycatchtrace-stdout -return-child-result --args
else ifneq ($(strip $(CPPUNITTRACE)),)
gb_CppunitTest_GDBTRACE := $(CPPUNITTRACE)
gb_CppunitTest__interactive := $(true)
endif

ifneq ($(strip $(VALGRIND)),)
gb_CppunitTest_VALGRINDTOOL := valgrind --tool=$(VALGRIND) --num-callers=50 --error-exitcode=1 --trace-children=yes --trace-children-skip='*/java,*/gij'
ifeq ($(strip $(VALGRIND)),memcheck)
G_SLICE := always-malloc
GLIBCXX_FORCE_NEW := 1
endif
endif

ifneq (,$(filter perfcheck,$(MAKECMDGOALS)))
$(if $(ENABLE_VALGRIND),,$(call gb_Output_error,Running performance tests with empty $$(ENABLE_VALGRIND) does not make sense))
gb_CppunitTest_VALGRINDTOOL := valgrind --tool=callgrind --dump-instr=yes --instr-atstart=no --simulate-cache=yes --dump-instr=yes --collect-bus=yes --branch-sim=yes
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

define gb_CppunitTest__make_args
$(HEADLESS) \
"-env:BRAND_BASE_DIR=$(call gb_Helper_make_url,$(INSTROOT))" \
"-env:BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)" \
"-env:UserInstallation=$(call gb_Helper_make_url,$(call gb_CppunitTest_get_target,$*).user)" \
$(if $(URE),\
    $(if $(strip $(CONFIGURATION_LAYERS)),\
	    "-env:CONFIGURATION_LAYERS=$(strip $(CONFIGURATION_LAYERS))") \
    $(if $(strip $(UNO_TYPES)),\
	    "-env:UNO_TYPES=$(foreach item,$(UNO_TYPES),$(call gb_Helper_make_url,$(item)))") \
    $(if $(strip $(UNO_SERVICES)),\
	"-env:UNO_SERVICES=$(foreach item,$(UNO_SERVICES),$(call gb_Helper_make_url,$(item)))") \
	$(if $(strip $(JAVA_URE)),\
		-env:URE_MORE_JAVA_TYPES=$(call gb_Helper_make_url,$(call gb_Jar_get_target,unoil))) \
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
	$(call gb_Output_announce,$*,$(true),CUT,2)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		rm -fr $@.user && mkdir $@.user && \
		$(if $(gb_CppunitTest__use_confpreinit), \
		    $(INSTDIR)/program/lokconf_init $(call gb_CppunitTest__make_args) &&) \
		$(if $(gb_CppunitTest__interactive),, \
			$(if $(value gb_CppunitTest_postprocess), \
				rm -fr $@.core && mkdir $@.core && cd $@.core &&)) \
		($(gb_CppunitTest_CPPTESTPRECOMMAND) \
		$(if $(G_SLICE),G_SLICE=$(G_SLICE)) \
		$(if $(GLIBCXX_FORCE_NEW),GLIBCXX_FORCE_NEW=$(GLIBCXX_FORCE_NEW)) \
		$(if $(HEADLESS),,VCL_HIDE_WINDOWS=1) \
		$(gb_CppunitTest_malloc_check) \
		$(if $(strip $(PYTHON_URE)),\
			PYTHONDONTWRITEBYTECODE=1) \
		$(ICECREAM_RUN) $(gb_CppunitTest_GDBTRACE) $(gb_CppunitTest_VALGRINDTOOL) $(gb_CppunitTest_CPPTESTCOMMAND) \
		$(call gb_LinkTarget_get_target,$(call gb_CppunitTest_get_linktarget,$*)) \
		$(call gb_CppunitTest__make_args) "-env:CPPUNITTESTTARGET=$@" \
		$(if $(gb_CppunitTest__interactive),, \
			> $@.log 2>&1 \
			|| ($(if $(value gb_CppunitTest_postprocess), \
					RET=$$?; \
					$(call gb_CppunitTest_postprocess,$(gb_CppunitTest_CPPTESTCOMMAND),$@.core,$$RET) >> $@.log 2>&1;) \
				cat $@.log; $(SRCDIR)/solenv/bin/unittest-failed.sh Cppunit $* $(OS)))))

define gb_CppunitTest_CppunitTest
$(call gb_CppunitTest__CppunitTest_impl,$(1),$(call gb_CppunitTest_get_linktarget,$(1)))

endef

# call gb_CppunitTest__CppunitTest_impl,cppunittest,linktarget
define gb_CppunitTest__CppunitTest_impl
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
$(call gb_CppunitTest_get_target,$(1)) : JAVA_URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : PYTHON_URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : URE := $(false)
$(call gb_CppunitTest_get_target,$(1)) : VCL := $(false)
$(call gb_CppunitTest_get_target,$(1)) : UNO_SERVICES :=
$(call gb_CppunitTest_get_target,$(1)) : UNO_TYPES :=
$(call gb_CppunitTest_get_target,$(1)) : HEADLESS := --headless
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
ifeq ($(USING_X),TRUE)
$(call gb_CppunitTest_get_target,$(1)) : $(call gb_Library_get_target,desktop_detector)
$(call gb_CppunitTest_get_target,$(1)) : $(if $(filter $(2),$(true)), \
    $(call gb_Library_get_target,vclplug_gen) \
        $(if $(ENABLE_GTK),$(call gb_Library_get_target,vclplug_gtk)) \
        $(if $(ENABLE_GTK3),$(call gb_Library_get_target,vclplug_gtk3)) \
        $(if $(ENABLE_KDE4),$(call gb_Library_get_target,vclplug_kde4)) \
        $(if $(ENABLE_TDE),$(call gb_Library_get_target,vclplug_tde)))
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
$(call gb_CppunitTest_get_target,$(1)) : HEADLESS :=
$(call gb_CppunitTest__use_vcl,$(1),$(false))

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

define gb_CppunitTest_use_java_ure
$(call gb_CppunitTest_get_target,$(1)) : JAVA_URE := $(true)
$(call gb_CppunitTest_get_target,$(1)) : \
    $(foreach jar,java_uno juh jurt unoil unoloader,$(call gb_Jar_get_target,$(jar))) \
    $(call gb_Library_get_target,affine_uno_uno) \
    $(call gb_Library_get_target,java_uno) \
    $(call gb_Library_get_target,jpipe) \
    $(call gb_Library_get_target,juhx) \
    $(call gb_Library_get_target,juhx) \
    $(call gb_Library_get_target,jvmaccess) \
    $(call gb_Library_get_target,jvmfwk) \
    $(call gb_Package_get_target,jvmfwk_javavendors) \
    $(call gb_Package_get_target,jvmfwk_jreproperties)

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
	$(if $(filter-out WNT,$(OS)),\
		$(call gb_Library_get_target,pyuno_wrapper) \
	) \
	$(if $(SYSTEM_PYTHON),, \
		$(if $(filter MACOSX,$(OS)),\
			$(call gb_GeneratedPackage_get_target,python3),\
			$(call gb_Package_get_target,python3) \
		) \
	) \
	$(call gb_Package_get_target,pyuno_python_scripts)

endef

define gb_CppunitTest__forward_to_Linktarget
gb_CppunitTest_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_CppunitTest_get_linktarget,$$(1)),$$(2),$$(3),CppunitTest_$$(1))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_exception_objects \
	use_executable_objects \
	use_library_objects \
	use_libraries \
	use_static_libraries \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_asmobject \
	add_asmobjects \
	use_package \
	use_packages \
	set_precompiled_header \
	add_sdi_headers \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_yaccflags \
	add_objcflags \
	add_objcxxflags \
	add_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	add_libs \
	disable_standard_system_libs \
	use_system_darwin_frameworks \
	use_system_win32_libs \
	use_internal_api \
	use_internal_bootstrap_api \
	use_internal_comprehensive_api \
	set_library_path_flags \
	use_external \
	use_externals \
	use_custom_headers \
	set_visibility_default \
	set_warnings_not_errors \
	set_external_code \
	set_generated_cxx_suffix \
,\
	$(call gb_CppunitTest__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
