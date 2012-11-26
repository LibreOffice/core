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



# SdiTarget is evil, in that it does not support multiple repositories for now (hardcoded to SRCDIR)
# Also there is no way to cleanly deliver the generated header to OUTDIR.
# (This can be workarounded by using gb_Package, but really should not.)

# SdiTarget class

gb_SdiTarget_SVIDLTARGET := $(call gb_Executable_get_target,svidl)
gb_SdiTarget_SVIDLCOMMAND := $(gb_SdiTarget_SVIDLPRECOMMAND) $(gb_SdiTarget_SVIDLTARGET)

$(call gb_SdiTarget_get_target,%) : $(SRCDIR)/%.sdi | $(gb_SdiTarget_SVIDLTARGET)
	$(call gb_Output_announce,$*,$(true),SDI,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@))
	$(call gb_Helper_abbreviate_dirs_native,\
		cd $(dir $<) && \
		$(gb_SdiTarget_SVIDLCOMMAND) -quiet \
			$(INCLUDE) \
			-fs$@.hxx \
			-fd$@.ilb \
			-fl$@.lst \
			-fz$@.sid \
			-fx$(EXPORTS) \
			-fm$@ \
			$<)

.PHONY : $(call gb_SdiTarget_get_clean_target,%)
$(call gb_SdiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SDI,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(foreach ext,.hxx .ilb .lst .sid,\
			$(call gb_SdiTarget_get_target,$*)$(ext)) \
			$(call gb_SdiTarget_get_target,$*))

define gb_SdiTarget_SdiTarget
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $$(subst -I. ,-I$$(dir $(SRCDIR)/$(1)) ,$$(SOLARINC))
$(call gb_SdiTarget_get_target,$(1)) : EXPORTS := $(SRCDIR)/$(2).sdi
endef

define gb_SdiTarget_set_include
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

# vim: set noet sw=4 ts=4:
