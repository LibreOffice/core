# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

# UnoApiPartTarget

gb_UnoApiPartTarget_IDLCTARGET := $(call gb_Executable_get_target_for_build,idlc)
gb_UnoApiPartTarget_IDLCCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiPartTarget_IDLCTARGET)
ifeq (,$(SYSTEM_UCPP))
gb_UnoApiPartTarget_UCPPTARGET := $(call gb_Executable_get_target_for_build,ucpp)
endif

# The .urd files are actually created by the gb_UnoApiPartTarget__command,
# invoked for the per-directory .done files.
# The reason why .urd files are tracked is so new files that are added are
# picked up and cause a rebuild, even if older than the .done file (also, as a
# convenience for users who delete them from the workdir by hand; this dummy
# rule plus the dependency from the .done target to the .urd file plus the
# sort/patsubst call in gb_UnoApiPartTarget__command cause command to be
# invoked with the .idl file corresponding to the .urd in that case.
# Touch the .urd file, so it is newer than the .done file, causing that to
# be rebuilt and overwriting the .urd file again.
# the .dir is for make 3.81, which ignores trailing /
$(dir $(call gb_UnoApiPartTarget_get_target,))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UnoApiPartTarget_get_target,%.urd) :
	touch $@

# TODO:
# - get idlc switch "-P" (generate .urd into package dir)
# - empty $? in headertarget?

define gb_UnoApiPartTarget__command
	$(call gb_Output_announce,$(2),$(true),IDL,2)
	mkdir -p $(call gb_UnoApiPartTarget_get_target,$(dir $(2))) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,\
		$(sort $(patsubst $(call gb_UnoApiPartTarget_get_target,%.urd),$(SRCDIR)/%.idl,$(3)))) && \
	$(gb_UnoApiPartTarget_IDLCCOMMAND) \
		$(INCLUDE) \
		-M $(basename $(call gb_UnoApiPartTarget_get_dep_target,$(dir $(2)))) \
		-O $(call gb_UnoApiPartTarget_get_target,$(dir $(2))) -verbose \
		@$${RESPONSEFILE} > /dev/null && \
	rm -f $${RESPONSEFILE} && \
	touch $(1)

endef

$(call gb_UnoApiPartTarget_get_target,%.done) : \
		$(gb_UnoApiPartTarget_IDLCTARGET) \
		| $(gb_UnoApiPartTarget_UCPPTARGET)
	$(call gb_UnoApiPartTarget__command,$@,$*,$(filter-out $(gb_UnoApiPartTarget_IDLCTARGET),$?))

ifeq ($(gb_FULLDEPS),$(true))

$(call gb_UnoApiPartTarget_get_dep_target,%) :
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_UnoApiPartTarget_get_target,$*.urd)))

endif

# UnoApiTarget

gb_UnoApiTarget_RDBMAKERTARGET := $(call gb_Executable_get_target_for_build,rdbmaker)
gb_UnoApiTarget_RDBMAKERCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_RDBMAKERTARGET)
gb_UnoApiTarget_REGCOMPARETARGET := $(call gb_Executable_get_target_for_build,regcompare)
gb_UnoApiTarget_REGCOMPARECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGCOMPARETARGET)
gb_UnoApiTarget_REGMERGETARGET := $(call gb_Executable_get_target_for_build,regmerge)
gb_UnoApiTarget_REGMERGECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGMERGETARGET)
gb_UnoApiTarget_XML2CMPTARGET := $(call gb_Executable_get_target_for_build,xml2cmp)
gb_UnoApiTarget_XML2CMPCOMMAND := $(gb_Helper_set_ld_path) $(gb_UnoApiTarget_XML2CMPTARGET)

gb_UnoApiTarget_XMLRDB := $(call gb_UnoApiTarget_get_target,types)

define gb_UnoApiTarget__get_types
$(if $(1),$(foreach type,$(shell $(gb_UnoApiTarget_XML2CMPCOMMAND) -types stdout $(1)),$(addprefix -T,$(type))))
endef

define gb_UnoApiTarget__command_impl
RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(2)) && \
$(1) @$${RESPONSEFILE} && \
rm -f $${RESPONSEFILE}
endef

define gb_UnoApiTarget__regmerge_command_impl
$(call gb_UnoApiTarget__command_impl,$(gb_UnoApiTarget_REGMERGECOMMAND),$(1) $(2) $(3))
endef

define gb_UnoApiTarget__rdbmaker_command_impl
$(call gb_UnoApiTarget__command_impl,$(gb_UnoApiTarget_RDBMAKERCOMMAND),-O$(1) -B$(2) -b$(3) $(4) $(5))
endef

define gb_UnoApiTarget__command
$(call gb_Output_announce,$*,$(true),UNO,4)
mkdir -p $(dir $(1)) && \
$(if $(UNOAPI_FILES),\
	$(call gb_UnoApiTarget__regmerge_command_impl,$(1),$(UNOAPI_ROOT),$(UNOAPI_FILES)),\
	$(if $(UNOAPI_MERGE),\
		$(call gb_UnoApiTarget__regmerge_command_impl,$(1),$(UNOAPI_ROOT),$(UNOAPI_MERGE)),\
		$(call gb_UnoApiTarget__rdbmaker_command_impl,$(1),UCR,$(UNOAPI_ROOT),\
			$(call gb_UnoApiTarget__get_types,$(UNOAPI_XML)),$(gb_UnoApiTarget_XMLRDB)))) \
$(if $(UNOAPI_REFERENCE), \
	$(call gb_Output_announce,$*,$(true),DBc,3) \
	&& $(gb_UnoApiTarget_REGCOMPARECOMMAND) \
		-f -t \
		-r1 $(UNOAPI_REFERENCE) \
		-r2 $(1))
endef

define gb_UnoApiTarget__check_mode
$(if $(or $(and $(1),$(2),$(3)),$(and $(1),$(2)),$(and $(2),$(3)),$(and $(1),$(3))),\
	$(error More than one mode of function of UnoApiTarget used: this is not supported),\
	$(if $(or $(1),$(2),$(3)),,\
		$(error Neither IDL files nor merged RDBs nor XML desc. were used: nothing will be produced)))
$(if $(4),,$(error No root has been set for the rdb file))
endef

$(call gb_UnoApiTarget_get_external_headers_target,%) :
	mkdir -p $(dir $@) && touch $@

$(call gb_UnoApiTarget_get_headers_target,%) : $(call gb_UnoApiTarget_get_external_headers_target,%)
	mkdir -p $(dir $@) && touch $@

$(call gb_UnoApiTarget_get_target,%) :
	$(call gb_UnoApiTarget__check_mode,$(UNOAPI_FILES),$(UNOAPI_MERGE),$(UNOAPI_XML),$(UNOAPI_ROOT))
	$(call gb_UnoApiTarget__command,$@,$*)

.PHONY : $(call gb_UnoApiTarget_get_clean_target,%)
$(call gb_UnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNO,4)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiTarget_get_target,$*) \
			$(call gb_UnoApiTarget_get_external_headers_target,$*) \
			$(call gb_UnoApiTarget_get_headers_target,$*))
		-rm -rf $(call gb_UnoApiTarget_get_dep_target,$*) \
			$(basename $(call gb_UnoApiPartTarget_get_dep_target,$*)) \
			$(call gb_UnoApiPartTarget_get_target,$*)

# cat the deps of all IDLs in one file, then we need only open that one file
define gb_UnoApiTarget__command_dep
$(call gb_Output_announce,IDL:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
		$(foreach idl,$(patsubst %.idl,%,$(3)),$(call gb_UnoApiPartTarget_get_dep_target,$(idl)))) && \
	$(SOLARENV)/bin/concat-deps $${RESPONSEFILE} > $(1)) && \
	rm -f $${RESPONSEFILE}

endef

ifeq ($(gb_FULLDEPS),$(true))

$(call gb_UnoApiTarget_get_dep_target,%) :
	$(call gb_UnoApiTarget__command_dep,$@,$*,$(UNOAPI_IDLFILES))

endif

define gb_UnoApiTarget_UnoApiTarget
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_FILES :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_XML :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_ROOT :=

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_UnoApiTarget_get_dep_target,$(1)) : UNOAPI_IDLFILES :=
-include $(call gb_UnoApiTarget_get_dep_target,$(1))
endif

endef

define gb_UnoApiTarget__add_urdfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_FILES += $(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(2)

endef

define gb_UnoApiTarget__add_idlfile
$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd)
$(call gb_UnoApiTarget__add_urdfile,$(1),$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd))
$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd) :| \
	$(call gb_UnoApiPartTarget_get_target,$(2)/.dir)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_UnoApiTarget_get_dep_target,$(1)) : UNOAPI_IDLFILES += $(2)/$(3).idl
$(call gb_UnoApiTarget_get_dep_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_dep_target,$(2)/$(3))
endif

endef

define gb_UnoApiTarget__add_idlfiles
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) : \
	$(foreach idl,$(3),$(SRCDIR)/$(2)/$(idl).idl)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) :| $(call gb_UnoApiTarget_get_external_headers_target,$(1))

endef

define gb_UnoApiTarget_add_idlfiles
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(idl)))
$(call gb_UnoApiTarget__add_idlfiles,$(1),$(2),$(3))
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_REGMERGETARGET)

endef

define gb_UnoApiTarget_add_idlfile
$(call gb_UnoApiTarget__add_idlfile,$(1),$(2),$(3))

endef

define gb_UnoApiTarget_merge_rdbfiles
$$(call gb_Output_error,gb_UnoApiTarget_merge_rdbfiles: use gb_UnoApiTarget_merge_api instead.)
endef

define gb_UnoApiTarget_merge_api
$(foreach rdb,$(2),$(call gb_UnoApiTarget__merge_api,$(1),$(rdb)))
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_REGMERGETARGET)

endef

define gb_UnoApiTarget_merge_rdbfile
$$(call gb_Output_error,gb_UnoApiTarget_merge_rdbfile: use gb_UnoApiTarget_merge_api instead.)
endef

define gb_UnoApiTarget__merge_api
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE += $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(2))

endef

# Set XML component dependencies description.
define gb_UnoApiTarget_set_xmlfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_XML := $(SRCDIR)/$(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(SRCDIR)/$(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_XMLRDB)
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_XML2CMPTARGET)
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_RDBMAKERTARGET)

endef

define gb_UnoApiTarget_add_reference_rdbfile
$$(call gb_Output_error,gb_UnoApiTarget_add_reference_rdbfile: use gb_UnoApiTarget_set_reference_rdbfile instead.)
endef

define gb_UnoApiTarget_set_reference_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE := $(SRCDIR)/$(strip $(2)).rdb
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_REGCOMPARETARGET)

endef

define gb_UnoApiTarget_set_include
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

define gb_UnoApiTarget_set_root
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_ROOT := $(2)

endef

# UnoApiHeadersTarget

gb_UnoApiHeadersTarget_CPPUMAKERTARGET := $(call gb_Executable_get_target_for_build,cppumaker)
gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiHeadersTarget_CPPUMAKERTARGET)

define gb_UnoApiHeadersTarget__command
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
		$(UNOAPI_DEPS)) && \
	$(gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND) \
		-Gc $(4) -BUCR \
		-O$(3) $(call gb_UnoApiTarget_get_target,$(2)) \
		@$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE} && \
	touch $(1)

endef

# On iOS we use static linking because dynamic loading of own code
# isn't allowed by the iOS App Store rules, and we want our code to be
# eventually distributable there as part of apps.

# To avoid problems that this causes together with the lovely
# intentional breaking of the One Definition Rule, for iOS we always
# generate comprehensive headers for udkapi. (The ODR breakage doesn't
# harm, by accident or careful design, on platforms where shared
# libraries are used.)

$(call gb_UnoApiHeadersTarget_get_bootstrap_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERTARGET)
	$(if $(filter TRUEudkapi,$(DISABLE_DYNLOADING)$*), \
		$(call gb_Output_announce,$*,$(true),HPB,3) \
		$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$*),-C), \
	\
		$(call gb_Output_announce,$*,$(true),HPB,3) \
		$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$*)) \
	)

$(call gb_UnoApiHeadersTarget_get_comprehensive_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERTARGET)
	$(call gb_Output_announce,$*,$(true),HPC,3)
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$*),-C)

$(call gb_UnoApiHeadersTarget_get_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERTARGET)
	$(if $(filter TRUEudkapi,$(DISABLE_DYNLOADING)$*), \
		$(call gb_Output_announce,$*,$(true),HPP,3) \
		$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_dir,$*),-C), \
	\
		$(call gb_Output_announce,$*,$(true),HPP,3) \
		$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_dir,$*),-L) \
	)

.PHONY : $(call gb_UnoApiHeadersTarget_get_clean_target,%)
$(call gb_UnoApiHeadersTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HPP,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_UnoApiHeadersTarget_get_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_target,$*) \
			$(call gb_UnoApiHeadersTarget_get_bootstrap_target,$*)) \
			$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$*)

define gb_UnoApiHeadersTarget_UnoApiHeadersTarget
$(call gb_UnoApiHeadersTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_bootstrap_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))

$(call gb_UnoApiHeadersTarget_get_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_bootstrap_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : UNOAPI_DEPS :=

# need dummy recipes so that header files are delivered in Package_inc;
# otherwise make will consider the header to be up-to-date because it was
# actually built by the recipe for gb_UnoApiHeadersTarget_get_target
$(call gb_UnoApiHeadersTarget_get_dir,$(1),/%.hdl) :
	touch $$@

$(call gb_UnoApiHeadersTarget_get_dir,$(1),/%.hpp) :
	touch $$@

$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$(1),/%.hdl) :
	touch $$@

$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$(1),/%.hpp) :
	touch $$@

$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$(1),/%.hdl) :
	touch $$@

$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$(1),/%.hpp) :
	touch $$@

endef

# ensure that new urd triggers the dummy rule to rebuild the headers
define gb_UnoApiHeadersTarget_add_headerfile
$(call gb_UnoApiHeadersTarget_get_dir,$(1),/$(3)) : \
	$(call gb_UnoApiPartTarget_get_target,$(basename $(2)).urd)

endef

define gb_UnoApiHeadersTarget__use_api_for_target
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : UNOAPI_DEPS += -X$(call gb_UnoApiTarget_get_target,$(2))

endef

define gb_UnoApiHeadersTarget_add_rdbfile
$$(call gb_Output_error,gb_UnoApiHeadersTarget_add_rdbfile: use gb_UnoApiHeadersTarget_use_api instead.)
endef

define gb_UnoApiHeadersTarget__use_api
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),target)
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),bootstrap_target)
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),comprehensive_target)

endef

define gb_UnoApiHeadersTarget_add_rdbfiles
$$(call gb_Output_error,gb_UnoApiHeadersTarget_add_rdbfiles: use gb_UnoApiHeadersTarget_use_api instead.)
endef

define gb_UnoApiHeadersTarget_use_api
$(foreach rdb,$(2),$(call gb_UnoApiHeadersTarget__use_api,$(1),$(rdb)))

endef

# vim: set noet sw=4 ts=4:
