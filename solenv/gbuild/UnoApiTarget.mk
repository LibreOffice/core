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



define gb_UnoApiTarget_autopackage_inc
$$(eval $$(call gb_Package_Package,$(1)_inc,$$(patsubst %/empty,%,$$(call gb_UnoApiTarget_get_header_target,empty))))
$(foreach onefile,$(gb_UnoApiTarget_HPPFILES_$(1)), \
	$$(eval $$(call gb_Package_add_file,$(1)_inc,$(patsubst $(1)/%,inc/$(1)/%,$(onefile)),$(onefile))))

$(call gb_Package_get_target,$(1)_inc) : $(call gb_UnoApiTarget_get_target,$(1))

endef

# TODO: make multi repo safe
define gb_UnoApiTarget_autopackage_idl
$$(eval $$(call gb_Package_Package,$(1)_idl,$(SRCDIR)))
$(foreach onefile,$(gb_UnoApiTarget_IDLFILES_$(1)), \
	$$(eval $$(call gb_Package_add_file,$(1)_idl,$(patsubst $(1)/%,idl/%,$(onefile)),$(onefile))))

endef

$(call gb_UnoApiTarget_get_header_target,$(1))/% : $(call gb_UnoApiTarget_get_target,$(1))
	mkdir -p $$(dir $$@)

define gb_UnoApiTarget_UnoApiTarget
$$(eval $$(call gb_Module_register_target,$(call gb_UnoApiOutTarget_get_target,$(1)),$(call gb_UnoApiOutTarget_get_clean_target,$(1))))
$(call gb_UnoApiOutTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiOutTarget_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))
gb_UnoApiTarget_HPPFILES_$(1) :=
gb_UnoApiTarget_IDLFILES_$(1) :=

.DELETE_ON_ERROR : \
    $(call gb_UnoApiTarget_get_clean_target,$(1)) \
    $(call gb_UnoApiTarget_get_target,$(1))

endef

define gb_UnoApiTarget_add_idlfiles
$(foreach idl,$(2),$(call gb_UnoApiTarget_add_idlfile,$(1),$(idl)))

endef

define gb_UnoApiTarget_add_idlfile
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiPartTarget_get_target,$(2).urd)
$(call gb_UnoApiPartTarget_get_target,$(2).urd) : $(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(2).idl))
gb_UnoApiTarget_HPPFILES_$(1) += $(2).hdl
gb_UnoApiTarget_HPPFILES_$(1) += $(2).hpp
gb_UnoApiTarget_IDLFILES_$(1) += $(2).idl

$(call gb_UnoApiTarget_get_header_target,)$(2).hpp :| $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiTarget_get_header_target,)$(2).hdl :| $(call gb_UnoApiTarget_get_target,$(1))

endef

define gb_UnoApiTarget_add_idlfiles_noheader
$(foreach idl,$(2),$(call gb_UnoApiTarget_add_idlfile_noheader,$(1),$(idl)))

endef

define gb_UnoApiTarget_add_idlfile_noheader
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiPartTarget_get_target,$(2).urd)
$(call gb_UnoApiPartTarget_get_target,$(2).urd) : $(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(2).idl))
gb_UnoApiTarget_IDLFILES_$(1) += $(2).idl

endef

define gb_UnoApiTarget_add_idlfiles_nohdl
$(foreach idl,$(2),$(call gb_UnoApiTarget_add_idlfile_nohdl,$(1),$(idl)))

endef

define gb_UnoApiTarget_add_idlfile_nohdl
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiPartTarget_get_target,$(2).urd)
$(call gb_UnoApiPartTarget_get_target,$(2).urd) : $(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(2).idl))
gb_UnoApiTarget_HPPFILES_$(1) += $(2).hpp
gb_UnoApiTarget_IDLFILES_$(1) += $(2).idl

$(call gb_UnoApiTarget_get_header_target,)$(2).hpp :| $(call gb_UnoApiTarget_get_target,$(1))

endef

define gb_UnoApiTarget_add_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiTarget_add_rdbfile,$(1),$(rdb)))

endef

define gb_UnoApiTarget_add_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_DEPS += -X$(call gb_UnoApiOutTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiOutTarget_get_target,$(2)) \
	$(call gb_Package_get_target,$(2)_idl)

endef

define gb_UnoApiTarget_add_merge_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiTarget_add_merge_rdbfile,$(1),$(rdb)))

endef

define gb_UnoApiTarget_add_merge_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE += $(call gb_UnoApiOutTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiOutTarget_get_target,$(2))

endef

#UNOAPI_REFERENCE_$(1) := $(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(strip $(2)).rdb))
define gb_UnoApiTarget_add_reference_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE := $(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(strip $(2)).rdb))

endef

define gb_UnoApiTarget_set_include
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

.PHONY : $(call gb_UnoApiOutTarget_get_clean_target,%)
$(call gb_UnoApiOutTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNOAPI,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiOutTarget_get_target,$*))

.PHONY : $(call gb_UnoApiTarget_get_clean_target,%)
$(call gb_UnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNOAPI,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiTarget_get_target,$*))
	-rm -rf $(call gb_UnoApiTarget_get_header_target,$*)\
			$(call gb_UnoApiPartTarget_get_target,$*)

# idlc doesn't return error codes != 0 in case of an error, so
# check self
define gb_UnoApiPartTarget__command
mkdir -p $(dir $(1)) && \
	mkdir -p  $(gb_Helper_MISC) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "$(call gb_Helper_convert_native,$(5) $(6) -O $(call gb_UnoApiPartTarget_get_target,$(2)) -verbose -C $(4))" > $${RESPONSEFILE} && \
	$(gb_UnoApiTarget_IDLCCOMMAND) @$${RESPONSEFILE} > /dev/null && \
	rm -f $${RESPONSEFILE} && \
	test -f $(@)

endef

$(call gb_UnoApiPartTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),IDL,2)
	$(call gb_UnoApiPartTarget__command,$@,$(dir $*),$<,$?,$(INCLUDE),$(DEFS))
	

define gb_UnoApiTarget__command
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(call gb_Helper_convert_native,$(1) $(4) $(5))) && \
	$(gb_UnoApiTarget_REGMERGECOMMAND) @$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE}

endef

define gb_UnoApiHeaderTarget__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	mkdir -p  $(gb_Helper_MISC) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo " -Gc -L -BUCR -O$(call gb_UnoApiTarget_get_header_target,$*) $(7) \
		$(1) \
		" > $${RESPONSEFILE} && \
	$(gb_UnoApiTarget_CPPUMAKERCOMMAND) @$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE})

endef

# TODO:
# - get idlc switch "-P" (generate .urd into package dir)
# - generate dependencies for included idls
# - empty $? in headertarget?

$(call gb_UnoApiTarget_get_target,%):
	$(call gb_Output_announce,$*,$(true),RDB,3)
	$(if $(gb_UnoApiTarget_IDLFILES_$*),$(call gb_UnoApiTarget__command,$@,$*,$<,UCR,$(addprefix $(call gb_UnoApiPartTarget_get_target,),$(patsubst %.idl,%.urd,$(gb_UnoApiTarget_IDLFILES_$*)))))
	$(if $(UNOAPI_MERGE),$(call gb_UnoApiTarget__command,$@,$*,$<,/,$(UNOAPI_MERGE)))
	$(if $(UNOAPI_REFERENCE), \
		$(call gb_Output_announce,$*,$(true),RDBCHEK,4) \
	    $(gb_UnoApiTarget_REGCOMPARECOMMAND) -f -t -r1 $(call gb_Helper_convert_native,$(UNOAPI_REFERENCE)) -r2 $(call gb_Helper_convert_native,$@))
	$(if $(gb_UnoApiTarget_IDLFILES_$*), \
		$(call gb_Output_announce,$*,$(true),HPP,4) \
		$(call gb_UnoApiHeaderTarget__command,$@,$*,$<,$?,$(INCLUDE),$(DEFS),$(UNOAPI_DEPS)))

# vim: set noet sw=4 ts=4:
