###############################################################
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
###############################################################



# Ant class

gb_Ant_ANTCOMMAND := $(ANT)

# clean target
$(call gb_Ant_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),ANT,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Ant_get_target,$*) $(call gb_Jar_get_outdir_target,$*) && \
		$(gb_Ant_ANTCOMMAND) -f $(ANTBUILDFILE) clean)

# the outdir target depends on the workdir target and is built by delivering the latter

# rule for creating the jar file
# creates the target folder of the jar file if it doesn't exist
# creates the jar file
$(call gb_Ant_get_target,%) : FORCE
	$(call gb_Output_announce,$*,$(true),ANT,3)
	$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $@) && \
	$(gb_Ant_ANTCOMMAND) -f $(ANTBUILDFILE) )

FORCE:


define gb_Ant_add_dependency
$(1) : $(2)

endef

# registers target and clean target
# adds jar files to DeliverLogTarget
# adds dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
define gb_Ant_Ant
ifeq ($(OS),WNT)
$(call gb_Ant_get_target,$(1)) : ANTBUILDFILE := `cygpath -m $(2)`
$(call gb_Ant_get_clean_target,$(1)) : ANTBUILDFILE := `cygpath -m $(2)`
else
$(call gb_Ant_get_target,$(1)) : ANTBUILDFILE := $(2)
$(call gb_Ant_get_clean_target,$(1)) : ANTBUILDFILE := $(2)
endif
$(eval $(call gb_Module_register_target,$(call gb_Jar_get_outdir_target,$(1)),$(call gb_Ant_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Jar_get_outdir_target,$(1)),$(call gb_Ant_get_target,$(1)),$(1))
$(call gb_Jar_get_outdir_target,$(1)) : $(call gb_Ant_get_target,$(1))

$(call gb_Ant_add_dependencies,$(1),$(2))

endef


# possible directories for jar files containing UNO services 
gb_Ant_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/classes/ \
    URE:vnd.sun.star.expand:\dURE_INTERNAL_JAVA_DIR/ \
    INTERN:vnd.sun.star.expand:\dOOO_INBUILD_JAVA_DIR/

# get component prefix from layer name ("OOO", "URE", "INTERN")
gb_Ant__get_componentprefix = \
    $(patsubst $(1):%,%,$(or \
        $(filter $(1):%,$(gb_Ant_COMPONENTPREFIXES)), \
        $(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

# layer must be specified explicitly in this macro (different to libraries)
define gb_Ant_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Ant__get_componentprefix,$(3)),$(notdir $(call gb_Ant_get_target,$(1))))
$(call gb_Ant_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Ant_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# vim: set noet sw=4 ts=4:
