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

# in non-product builds, ensure that tools-based assertions do not pop up as message box, but are routed to the shell
DBGSV_ERROR_OUT := shell
export DBGSV_ERROR_OUT

.PHONY : $(call gb_JunitTest_get_clean_target,%)
$(call gb_JunitTest_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_JunitTest_get_classsetname,%))
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $@ $@.log)

.PHONY : $(call gb_JunitTest_get_target,$(1))
$(call gb_JunitTest_get_target,%) :
    $(call gb_Output_announce,$*,$(true),JUT,2)
    $(call gb_Helper_abbreviate_dirs_native,\
        rm -rf $(call gb_JunitTest_get_userdir,$*) && \
        mkdir -p $(call gb_JunitTest_get_userdir,$*) && \
        $(gb_JunitTest_JAVACOMMAND) -cp "$(CLASSPATH)" $(DEFS) org.junit.runner.JUnitCore $(CLASSES) 2>&1 > $@.log || (cat $@.log && false) && \
        rm -rf $(call gb_JunitTest_get_userdir,$*))
    $(CLEAN_CMD)

define gb_JunitTest_JunitTest
$(call gb_JunitTest_get_target,$(1)) : CLASSPATH := $(value XCLASSPATH)$(gb_CLASSPATHSEP)$(call gb_JavaClassSet_get_classdir,$(call gb_JunitTest_get_classsetname,$(1)))$(gb_CLASSPATHSEP)$(OOO_JUNIT_JAR)$(gb_CLASSPATHSEP)$(OUTDIR)/lib
$(call gb_JunitTest_get_target,$(1)) : CLASSES :=
$(call gb_JunitTest_JunitTest_platform,$(1))

$(call gb_JavaClassSet_JavaClassSet,$(call gb_JunitTest_get_classsetname,$(1)))
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

define gb_JunitTest_set_classpath
$(call gb_JunitTest_get_target,$(1)) : CLASSPATH := $(2)

endef

define gb_JunitTest_add_jar
$(call gb_JunitTest_get_target,$(1)) : CLASSPATH := $$(CLASSPATH)$(gb_CLASSPATHSEP)$(2)
$(call gb_JunitTest_get_target,$(1)) : $(2)

endef

define gb_JunitTest_add_jars
$(foreach jar,$(2),$(call gb_JunitTest_add_jar,$(1),$(jar)))

endef
# vim: set noet sw=4 ts=4:
