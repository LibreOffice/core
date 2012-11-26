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
	$(gb_JavaClassSet_JAVACCOMMAND) -cp "$(CLASSPATH)" -d $(call gb_JavaClassSet_get_classdir,$(2)) $(3) && \
	touch $(1))

endef

define gb_JavaClassSet__rules
$$(call gb_JavaClassSet_get_repo_target,$(1),%) :
	$$(call gb_JavaClassSet__command,$$@,$$*,$$?)

$$(call gb_JavaClassSet_get_target,%) : $$(call gb_JavaClassSet_get_repo_target,$(1),%)
	$$(call gb_Output_announce,$$*,$$(true),JCS,3)
	$$(call gb_Helper_abbreviate_dirs,\
		touch $$@)

endef

$(call gb_JavaClassSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),JCS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_JavaClassSet_get_target,$*)))


$(foreach reponame,$(gb_JavaClassSet_REPOSITORYNAMES),$(eval $(call gb_JavaClassSet__rules,$(reponame))))

define gb_JavaClassSet_JavaClassSet
endef

define gb_JavaClassSet__get_sourcefile
$(1)/$(2).java
endef

define gb_JavaClassSet_add_sourcefile
$(foreach reponame,$(gb_JavaClassSet_REPOSITORYNAMES),\
	$(eval $(call gb_JavaClassSet_get_repo_target,$(reponame),$(1)) : $(call gb_JavaClassSet__get_sourcefile,$($(reponame)),$(2))))

endef

define gb_JavaClassSet_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_JavaClassSet_set_classpath
$(call gb_JavaClassSet_get_target,$(1)) : CLASSPATH := $(2)

endef

# vim: set noet sw=4 ts=4:
