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


# JunitTest class

gb_JunitTest_JAVACOMMAND := $(JAVAINTERPRETER) $(JAVAIFLAGS)


.PHONY : $(call gb_JunitTest_get_clean_target,%)
$(call gb_JunitTest_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_JunitTest_get_classsetname,%))
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $@ $@.log)

ifneq (,$(strip $(OOO_JUNIT_JAR)))

.PHONY : $(call gb_JunitTest_get_target,%)
$(call gb_JunitTest_get_target,%) :
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$*,$(true),JUT,2)
	$(call gb_Trace_StartRange,$*,JUT)
	$(call gb_Helper_abbreviate_dirs,\
        rm -rf $(call gb_JunitTest_get_userdir,$*) && \
		mkdir -p $(call gb_JunitTest_get_userdir,$*)/user && \
		cp $(SRCDIR)/qadevOOo/qa/registrymodifications.xcu $(call gb_JunitTest_get_userdir,$*)/user/ && \
        $(call gb_CppunitTest_coredumpctl_setup,$@) \
        ($(gb_TEST_ENV_VARS) $(ICECREAM_RUN) $(gb_CppunitTest_coredumpctl_run) \
            $(gb_JunitTest_JAVACOMMAND) \
            -classpath "$(T_CP)" \
            $(DEFS) \
            org.junit.runner.JUnitCore \
            $(CLASSES) > $@.log 2>&1 || \
		(cat $@.log \
		&& echo "to rerun just this failed test without all others, run:" \
		&& echo && echo "    make JunitTest_$*" && echo \
		&& echo "cd into the module dir to run the tests faster" \
		&& echo "Or to do interactive debugging, run two shells with:" \
		&& echo \
		&& echo "    make debugrun" \
		&& echo "    make gb_JunitTest_DEBUGRUN=T JunitTest_$*" \
		&& echo \
		&& false)))
	$(CLEAN_CMD)
	$(call gb_Trace_EndRange,$*,JUT)
endif

define gb_JunitTest_JunitTest
$(call gb_JunitTest_get_target,$(1)) : T_CP := $(call gb_JavaClassSet_get_classdir,$(call gb_JunitTest_get_classsetname,$(1)))$$(gb_CLASSPATHSEP)$(OOO_JUNIT_JAR)$(if $(HAMCREST_JAR),$$(gb_CLASSPATHSEP)$(HAMCREST_JAR))$$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_URE_LIB_FOLDER)
$(call gb_JunitTest_get_target,$(1)) : CLASSES :=
$(eval $(call gb_JunitTest_JunitTest_platform,$(1)))

$(call gb_JavaClassSet_JavaClassSet,$(call gb_JunitTest_get_classsetname,$(1)))
$(call gb_JavaClassSet_use_system_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(OOO_JUNIT_JAR))
$(if $(HAMCREST_JAR),$(call gb_JavaClassSet_use_system_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(HAMCREST_JAR)))
$(call gb_JunitTest_get_target,$(1)) : $(call gb_JavaClassSet_get_target,$(call gb_JunitTest_get_classsetname,$(1)))
$(eval $(call gb_Module_register_target,$(call gb_JunitTest_get_target,$(1)),$(call gb_JunitTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),JunitTest)

endef

define gb_JunitTest_set_defs
$(call gb_JunitTest_get_target,$(1)) : DEFS := $(2)

endef

define gb_JunitTest_add_classes
$(call gb_JunitTest_get_target,$(1)) : CLASSES += $(2)

endef

define gb_JunitTest_add_class
$(call gb_JunitTest_add_classes,$(1),$(2))

endef


define gb_JunitTest_add_sourcefile
$(call gb_JavaClassSet_add_sourcefile,$(call gb_JunitTest_get_classsetname,$(1)),$(2))

endef

define gb_JunitTest_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JunitTest_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_JunitTest_use_jar
$(call gb_JavaClassSet_use_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(call gb_Jar_get_target,$(2))
$(call gb_JunitTest_get_target,$(1)) : $(call gb_Jar_get_target,$(2))

endef

define gb_JunitTest_use_jars
$(foreach jar,$(2),$(call gb_JunitTest_use_jar,$(1),$(jar)))

endef

# see gb_JavaClassSet_use_jar_classset
define gb_JunitTest_use_jar_classset
$(call gb_JavaClassSet_use_jar_classset,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(2)))

endef

define gb_JunitTest_add_classpath
$(call gb_JavaClassSet_add_classpath,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(2)

endef

define gb_JunitTest_use_system_jar
$(call gb_JavaClassSet_use_system_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(2))

endef

define gb_JunitTest_use_system_jars
$(foreach jar,$(2),$(call gb_JunitTest_use_system_jar,$(1),$(jar)))

endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(eval $(call gb_JunitTest_use_external,jar,external))
define gb_JunitTest_use_external
$(if $(value gb_JunitTest__use_$(2)),\
  $(call gb_JunitTest__use_$(2),$(1)),\
  $(error gb_JunitTest_use_external: unknown external: $(2)))

endef

define gb_JunitTest_use_externals
$(foreach external,$(2),$(call gb_JunitTest_use_external,$(1),$(external)))

endef

define gb_JunitTest_use_customtarget
$(call gb_JavaClassSet_use_customtarget,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(gb_CustomTarget_workdir)/$(2)

endef

define gb_JunitTest_use_customtargets
$(foreach dependency,$(2),$(call gb_JunitTest_use_customtarget,$(1),$(dependency)))

endef

define gb_JunitTest_use_unoapi_jars
$(eval $(call gb_JunitTest_use_jars,$(1),\
    OOoRunner \
    libreoffice \
    test \
))

endef

define gb_JunitTest_use_unoapi_test_class
$(eval $(call gb_JunitTest_add_classes,$(1),\
    org.openoffice.test.UnoApiTest \
))

endef

# To be used by gb_JunitTest_set_unoapi_test_defaults
# <module>_unoapi_1 => <module>_1 => <module> => <module>/qa/unoapi
gb_JunitTest__unoapi_iter = $(subst _unoapi,,$(1))
gb_JunitTest__unoapi_module = $(firstword $(subst _, ,$(gb_JunitTest__unoapi_iter)))
gb_JunitTest__unoapi_dir = $(if $(2),$(2),$(gb_JunitTest__unoapi_module)/qa/unoapi)

# $(1) = test name (prefer <module>_unoapi for defaults, example <module>_unoapi_1)
# $(2) = test directory base (def: <module>_unoapi_1 => <module>_1/qa/unoapi)
# $(3) = SCE file (def: <module>_1.sce)
# $(4) = XCL file (def: knownissues.xcl)
# $(5) = test document directory (def: testdocuments; use . for base)
define gb_JunitTest_set_unoapi_test_defaults
$(eval $(call gb_JunitTest_set_defs,$(1),\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.sce=$(SRCDIR)/$(gb_JunitTest__unoapi_dir)/$(if $(3),$(3),$(gb_JunitTest__unoapi_iter).sce) \
    -Dorg.openoffice.test.arg.xcl=$(SRCDIR)/$(gb_JunitTest__unoapi_dir)/$(if $(4),$(4),knownissues.xcl) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/$(gb_JunitTest__unoapi_dir)/$(if $(5),$(5),testdocuments) \
))

$(eval $(call gb_JunitTest_use_unoapi_jars,$(1)))
$(eval $(call gb_JunitTest_use_unoapi_test_class,$(1)))

endef

else # OOO_JUNIT_JAR

.PHONY : $(call gb_JunitTest_get_target,$(1))
$(call gb_JunitTest_get_target,%) :
ifeq ($(gb_SUPPRESS_TESTS),)
	$(call gb_Output_announce,$* (skipped - no Junit),$(true),JUT,2)
endif
	@true

define gb_JunitTest_JunitTest
$(eval $(call gb_Module_register_target,$(call gb_JunitTest_get_target,$(1)),$(call gb_JunitTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),JunitTest)

endef

gb_JunitTest_set_defs :=
gb_JunitTest_add_classes :=
gb_JunitTest_add_class :=
gb_JunitTest_add_sourcefile :=
gb_JunitTest_add_sourcefiles :=
gb_JunitTest_use_jar :=
gb_JunitTest_use_jars :=
gb_JunitTest_use_jar_classset :=
gb_JunitTest_use_system_jar :=
gb_JunitTest_use_system_jars :=
gb_JunitTest_use_external :=
gb_JunitTest_use_externals :=
gb_JunitTest_use_customtarget :=
gb_JunitTest_use_customtargets :=

endif # OOO_JUNIT_JAR
# vim: set noet sw=4:
