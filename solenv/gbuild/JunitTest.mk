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

$(call gb_JavaClassSet_JavaClassSet,$(call gb_JunitTest_get_classsetname,$(1)),$(2))
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
