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

# DBGSV_ERROR_OUT => in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
# DISABLE_SAL_DBGBOX is the same, for osl/diagnose.h on Windows only
.PHONY : $(call gb_JunitTest_get_target,%)
$(call gb_JunitTest_get_target,%) :
	$(call gb_Output_announce,$*,$(true),JUT,2)
	$(call gb_Helper_abbreviate_dirs,\
        rm -rf $(call gb_JunitTest_get_userdir,$*) && \
		mkdir -p $(call gb_JunitTest_get_userdir,$*) && \
        (DBGSV_ERROR_OUT=shell DISABLE_SAL_DBGBOX=t \
		    $(gb_JunitTest_JAVACOMMAND) \
            -classpath "$(T_CP)" \
            $(DEFS) \
            org.junit.runner.JUnitCore \
            $(CLASSES) > $@.log 2>&1 || \
		(cat $@.log \
		&& echo "to rerun just this failed test without all others, run:" \
		&& echo && echo "    make JunitTest_$*" && echo \
		&& echo "cd into the module dir to run the tests faster" \
		&& echo "Or to do interactive debugging (non-Windows), run two shells with:" \
		&& echo \
		&& echo "    make debugrun" \
		&& echo "    make gb_JunitTest_DEBUGRUN=T JunitTest_$*" \
		&& echo \
		&& false)))
	$(CLEAN_CMD)

define gb_JunitTest_JunitTest
$(call gb_JunitTest_get_target,$(1)) : T_CP := $(call gb_JavaClassSet_get_classdir,$(call gb_JunitTest_get_classsetname,$(1)))$$(gb_CLASSPATHSEP)$(OOO_JUNIT_JAR)$$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_URE_LIB_FOLDER)
$(call gb_JunitTest_get_target,$(1)) : CLASSES :=
$(eval $(call gb_JunitTest_JunitTest_platform,$(1)))

$(call gb_JavaClassSet_JavaClassSet,$(call gb_JunitTest_get_classsetname,$(1)))
$(call gb_JavaClassSet_use_system_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(OOO_JUNIT_JAR))
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

define gb_JunitTest_add_jar
$$(call gb_Output_error,\
 gb_JunitTest_add_jar: use gb_JunitTest_use_jar instead.)
endef

define gb_JunitTest_use_jar
$(call gb_JavaClassSet_use_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(call gb_Jar_get_outdir_target,$(2))
$(call gb_JunitTest_get_target,$(1)) : $(call gb_Jar_get_outdir_target,$(2))

endef

define gb_JunitTest_add_jars
$$(call gb_Output_error,\
 gb_JunitTest_add_jars: use gb_JunitTest_use_jars instead.)
endef

define gb_JunitTest_use_jars
$(foreach jar,$(2),$(call gb_JunitTest_use_jar,$(1),$(jar)))

endef

define gb_JunitTest_add_jar_classset
$$(call gb_Output_error,\
 gb_JunitTest_add_jar_classset: use gb_JunitTest_use_jar_classset instead.)
endef

# see gb_JavaClassSet_use_jar_classset
define gb_JunitTest_use_jar_classset
$(call gb_JavaClassSet_use_jar_classset,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(2)))

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

define gb_JunitTest_add_customtarget_dependency
$$(call gb_Output_error,\
 gb_JunitTest_add_customtarget_dependency: use gb_Jar_use_customtarget instead.)
endef

define gb_JunitTest_use_customtarget
$(call gb_JavaClassSet_use_customtarget,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(call gb_CustomTarget_get_workdir,$(2))

endef

define gb_JunitTest_add_customtarget_dependencies
$$(call gb_Output_error,\
 gb_JunitTest_add_customtarget_dependencies: use gb_Jar_use_customtargets instead.)
endef

define gb_JunitTest_use_customtargets
$(foreach dependency,$(2),$(call gb_JunitTest_use_customtarget,$(1),$(dependency)))

endef

else # OOO_JUNIT_JAR

.PHONY : $(call gb_JunitTest_get_target,$(1))
$(call gb_JunitTest_get_target,%) :
	$(call gb_Output_announce,$* (skipped - no Junit),$(true),JUT,2)
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
