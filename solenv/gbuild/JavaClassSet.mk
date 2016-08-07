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



gb_JavaClassSet_REPOSITORYNAMES := $(gb_Helper_REPOSITORYNAMES)
gb_JavaClassSet_JAVACCOMMAND := $(JAVACOMPILER)

define gb_JavaClassSet__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_JavaClassSet_JAVACCOMMAND) -cp "$(CLASSPATH)" -d $(call gb_JavaClassSet_get_classdir,$(2)) $(if $(filter-out $(JARDEPS),$(3)),\
			$(filter-out $(JARDEPS),$(3)),\
			$(filter-out $(JARDEPS),$(4))) &&\
	touch $(1))

endef

$(call gb_JavaClassSet_get_target,%) :
	$(call gb_Output_announce,$*,$(true),JCS,3)
	$(call gb_JavaClassSet__command,$@,$*,$?,$^)

$(call gb_JavaClassSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),JCS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_JavaClassSet_get_target,$*)))

# no initialization of scoped variable CLASSPATH as it is "inherited" from controlling instance (e.g. JUnitTest, Jar)
# UGLY: cannot use target local variable for REPO because it's needed in prereq
define gb_JavaClassSet_JavaClassSet
$(if $(filter $(2),$(gb_JavaClassSet_REPOSITORYNAMES)),,\
  $(error JavaClassSet: no or invalid repository given; known repositories: \
  $(gb_JavaClassSet_REPOSITORYNAMES)))
gb_JavaClassSet_REPO_$(1) := $(2)
$(call gb_JavaClassSet_get_target,$(1)) : JARDEPS :=
endef

define gb_JavaClassSet__get_sourcefile
$($(1))/$(2).java
endef

define gb_JavaClassSet_add_sourcefile
$(eval $(call gb_JavaClassSet_get_target,$(1)) : \
	$(call gb_JavaClassSet__get_sourcefile,$(gb_JavaClassSet_REPO_$(1)),$(2)))
endef

define gb_JavaClassSet_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_sourcefile,$(1),$(sourcefile)))
endef

define gb_JavaClassSet_set_classpath
$(eval $(call gb_JavaClassSet_get_target,$(1)) : CLASSPATH := $(2))
endef

# problem: currently we can't get these dependencies to work
# build order dependency is a hack to get these prerequisites out of the way in the build command
define gb_JavaClassSet_add_jar
$(eval $(call gb_JavaClassSet_get_target,$(1)) : $(2))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : CLASSPATH := $$(CLASSPATH)$$(gb_CLASSPATHSEP)$(strip $(2)))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(2))
endef

# this does not generate dependency on the jar
define gb_JavaClassSet_add_system_jar
$(eval $(call gb_JavaClassSet_get_target,$(1)) : CLASSPATH := $$(CLASSPATH)$$(gb_CLASSPATHSEP)$(strip $(2)))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(2))
endef

define gb_JavaClassSet_add_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_add_jar,$(1),$(jar)))
endef

define gb_JavaClassSet_add_system_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_add_system_jar,$(1),$(jar)))
endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(call gb_LinkTarget_use_external,library,external)
define gb_JavaClassSet_use_external
$(eval $(if $(value gb_JavaClassSet__use_$(2)),\
  $(call gb_JavaClassSet__use_$(2),$(1)),\
  $(error gb_JavaClassSet_use_external: unknown external: $(2))))
endef

define gb_JavaClassSet_use_externals
$(foreach external,$(2),$(call gb_JavaClassSet_use_external,$(1),$(external)))
endef

# vim: set noet sw=4 ts=4:
