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


# JunitTest class

gb_JunitTest_JAVACOMMAND := $(JAVAINTERPRETER) $(JAVAIFLAGS)


.PHONY : $(call gb_JunitTest_get_clean_target,%)
$(call gb_JunitTest_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_JunitTest_get_classsetname,%))
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $@ $@.log)

ifneq (,$(strip $(OOO_JUNIT_JAR)))

# DBGSV_ERROR_OUT => in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
.PHONY : $(call gb_JunitTest_get_target,%)
$(call gb_JunitTest_get_target,%) :
	$(call gb_Output_announce,$*,$(true),JUT,2)
	$(call gb_Helper_abbreviate_dirs,\
        rm -rf $(call gb_JunitTest_get_userdir,$*) && \
		mkdir -p $(call gb_JunitTest_get_userdir,$*) && \
        (DBGSV_ERROR_OUT=shell DISABLE_SAL_DBGBOX=t \
		    $(gb_JunitTest_JAVACOMMAND) \
            -cp "$(T_CP)" \
            $(DEFS) \
            org.junit.runner.JUnitCore \
            $(CLASSES) > $@.log 2>&1 || \
		(cat $@.log \
		&& echo "to rerun just this failed test without all others, run:" \
		&& echo && echo "    make $@" && echo \
		&& echo "cd into the module dir to run the tests faster" \
		&& echo "Or to do interactive debugging, run two shells with (Linux only):" \
		&& echo \
		&& echo "    make debugrun" \
		&& echo "    make gb_JunitTest_DEBUGRUN=T $@" \
		&& echo \
		&& false)) && \
        rm -rf $(call gb_JunitTest_get_userdir,$*))
	$(CLEAN_CMD)

define gb_JunitTest_JunitTest
$(call gb_JunitTest_get_target,$(1)) : T_CP := $(if $(value XCLASSPATH),$$(value XCLASSPATH)$$(gb_CLASSPATHSEP))$(call gb_JavaClassSet_get_classdir,$(call gb_JunitTest_get_classsetname,$(1)))$$(gb_CLASSPATHSEP)$(OOO_JUNIT_JAR)$$(gb_CLASSPATHSEP)$(OUTDIR)/lib
$(call gb_JunitTest_get_target,$(1)) : CLASSES :=
$(eval $(call gb_JunitTest_JunitTest_platform,$(1)))

$(call gb_JavaClassSet_JavaClassSet,$(call gb_JunitTest_get_classsetname,$(1)))
$(call gb_JavaClassSet_use_system_jar,$(call gb_JunitTest_get_classsetname,$(1)),$(OOO_JUNIT_JAR))
$(call gb_JunitTest_get_target,$(1)) : $(call gb_JavaClassSet_get_target,$(call gb_JunitTest_get_classsetname,$(1)))
$(eval $(call gb_Module_register_target,$(call gb_JunitTest_get_target,$(1)),$(call gb_JunitTest_get_clean_target,$(1))))
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
$(call gb_JunitTest_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(2)
$(call gb_JunitTest_get_target,$(1)) : $(2)
$(2) :| $(gb_Helper_PHONY)

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

define gb_JunitTest_add_customtarget_dependency
$$(call gb_Output_error,\
 gb_JunitTest_add_customtarget_dependency: use gb_Jar_use_customtarget instead.)
endef

define gb_JunitTest_use_customtarget
$(call gb_JavaClassSet_use_customtarget,$(call gb_JunitTest_get_classsetname,$(1)),$(2))

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
endef

gb_JunitTest_set_defs :=
gb_JunitTest_add_classes :=
gb_JunitTest_add_class :=
gb_JunitTest_add_sourcefile :=
gb_JunitTest_add_sourcefiles :=
gb_JunitTest_use_jar :=
gb_JunitTest_use_jars :=
gb_JunitTest_use_jar_classset :=
gb_JunitTest_use_customtarget :=
gb_JunitTest_use_customtargets :=

endif # OOO_JUNIT_JAR
# vim: set noet sw=4:
