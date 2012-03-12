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
$(call gb_UnoApiPartTarget_get_target,%.urd) :
	mkdir -p $(dir $@) && touch $@

# TODO:
# - get idlc switch "-P" (generate .urd into package dir)
# - empty $? in headertarget?

define gb_UnoApiPartTarget__command
	$(call gb_Output_announce,$(2),$(true),IDL,2)
	mkdir -p $(call gb_UnoApiPartTarget_get_target,$(dir $(2))) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,\
		$(call gb_Helper_convert_native,$(INCLUDE) $(DEFS) \
		-M $(basename $(call gb_UnoApiPartTarget_get_dep_target,$(dir $(2)))) \
		-O $(call gb_UnoApiPartTarget_get_target,$(dir $(2))) -verbose -C \
		$(sort $(patsubst $(call gb_UnoApiPartTarget_get_target,%.urd),$(SRCDIR)/%.idl,$(3))))) && \
	$(gb_UnoApiPartTarget_IDLCCOMMAND) @$${RESPONSEFILE} > /dev/null && \
	rm -f $${RESPONSEFILE} && \
	touch $(1)

endef

$(call gb_UnoApiPartTarget_get_target,%.done) :
	$(call gb_UnoApiPartTarget__command,$@,$*,$?)

ifeq ($(gb_FULLDEPS),$(true))

$(call gb_UnoApiPartTarget_get_dep_target,%) :
	$(if $(realpath $@),touch $@,\
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
RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(call gb_Helper_convert_native,$(2))) && \
$(1) @$${RESPONSEFILE} && \
rm -f $${RESPONSEFILE}
endef

define gb_UnoApiTarget__regmerge_command_impl
$(call gb_UnoApiTarget__command_impl,$(gb_UnoApiTarget_REGMERGECOMMAND),$(1) $(2) $(3))
endef

# TODO: -bUCR changes content of the RDB files; the old build system
# uses -b/ (by default)
define gb_UnoApiTarget__rdbmaker_command_impl
$(call gb_UnoApiTarget__command_impl,$(gb_UnoApiTarget_RDBMAKERCOMMAND),-O$(1) -B$(2) -b$(2) $(3) $(4))
endef

define gb_UnoApiTarget__command
$(call gb_Output_announce,$*,$(true),RDB,3)
mkdir -p $(dir $(1)) && \
$(if $(UNOAPI_FILES),\
	$(call gb_UnoApiTarget__regmerge_command_impl,$(1),UCR,$(UNOAPI_FILES)),\
	$(if $(UNOAPI_MERGE),\
		$(call gb_UnoApiTarget__regmerge_command_impl,$(1),/,$(UNOAPI_MERGE)),\
		$(call gb_UnoApiTarget__rdbmaker_command_impl,$(1),UCR,\
			$(call gb_UnoApiTarget__get_types,$(UNOAPI_XML)),$(gb_UnoApiTarget_XMLRDB)))) \
$(if $(UNOAPI_REFERENCE), \
	$(call gb_Output_announce,$*,$(true),DBc,3) \
	&& $(gb_UnoApiTarget_REGCOMPARECOMMAND) \
		-f -t \
		-r1 $(call gb_Helper_convert_native,$(UNOAPI_REFERENCE)) \
		-r2 $(call gb_Helper_convert_native,$(1)))
endef

define gb_UnoApiTarget__check_mode
$(if $(or $(and $(1),$(2),$(3)),$(and $(1),$(2)),$(and $(2),$(3)),$(and $(1),$(3))),\
	$(error More than one mode of function of UnoApiTarget used: this is not supported),\
	$(if $(or $(1),$(2),$(3)),,\
		$(error Neither IDL files nor merged RDBs nor XML desc. were used: nothing will be produced)))
endef

$(call gb_UnoApiTarget_get_target,%):
	$(call gb_UnoApiTarget__check_mode,$(UNOAPI_FILES),$(UNOAPI_MERGE),$(UNOAPI_XML))
	$(call gb_UnoApiTarget__command,$@,$*,$<,$?)

.PHONY : $(call gb_UnoApiTarget_get_clean_target,%)
$(call gb_UnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RDB,3)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiTarget_get_target,$*))
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

$(call gb_UnoApiTarget_get_dep_target,%) : $(call gb_UnoApiTarget_get_target,%)
	$(call gb_UnoApiTarget__command_dep,$@,$*,$(UNOAPI_IDLFILES))

endif

define gb_UnoApiTarget_UnoApiTarget
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_FILES :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_XML :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE :=

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

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_UnoApiTarget_get_dep_target,$(1)) : UNOAPI_IDLFILES += $(2)/$(3).idl
$(call gb_UnoApiTarget_get_dep_target,$(1)) : \
       $(call gb_UnoApiPartTarget_get_dep_target,$(2)/$(3))
$(call gb_UnoApiPartTarget_get_dep_target,$(2)/$(3)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done)
endif

endef

define gb_UnoApiTarget__add_idlfiles
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) : \
	$(foreach idl,$(3),$(SRCDIR)/$(2)/$(idl).idl)

endef

define gb_UnoApiTarget_add_idlfiles
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(idl)))
$(call gb_UnoApiTarget__add_idlfiles,$(1),$(2),$(3))

endef

define gb_UnoApiTarget_add_idlfile
$(call gb_UnoApiTarget__add_idlfile,$(1),$(2),$(3))

endef

define gb_UnoApiTarget_merge_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiTarget_merge_rdbfile,$(1),$(rdb)))

endef

define gb_UnoApiTarget_merge_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE += $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(2))

endef

# Set XML component dependencies description.
define gb_UnoApiTarget_set_xmlfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_XML := $(SRCDIR)/$(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(SRCDIR)/$(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_XMLRDB)

endef

define gb_UnoApiTarget_add_reference_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE := $(SRCDIR)/$(strip $(2)).rdb

endef

define gb_UnoApiTarget_set_include
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

# UnoApiHeadersTarget

gb_UnoApiHeadersTarget_CPPUMAKERTARGET := $(call gb_Executable_get_target_for_build,cppumaker)
gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiHeadersTarget_CPPUMAKERTARGET)

define gb_UnoApiHeadersTarget__command
RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
	$(call gb_Helper_convert_native,-Gc $(4) -BUCR \
	-O$(3) $(call gb_UnoApiTarget_get_target,$(2)) $(UNOAPI_DEPS))) && \
$(gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND) @$${RESPONSEFILE} && \
rm -f $${RESPONSEFILE} && \
touch $(1)

endef

$(call gb_UnoApiHeadersTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),HPP,3)
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_dir,$*))

$(call gb_UnoApiHeadersTarget_get_comprehensive_target,%) :
	$(call gb_Output_announce,$*,$(true),HPC,3)
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$*),-C)

$(call gb_UnoApiHeadersTarget_get_lightweight_target,%) :
	$(call gb_Output_announce,$*,$(true),HPL,3)
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_lightweight_dir,$*),-L)

.PHONY : $(call gb_UnoApiHeadersTarget_get_clean_target,%)
$(call gb_UnoApiHeadersTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HPP,3)
	$(call gb_Helper_abbreviate_dirs_native,\
		rm -rf \
			$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_lightweight_dir,$*) \
		   	$(call gb_UnoApiHeadersTarget_get_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$*) \
			$(call gb_UnoApiHeadersTarget_get_lightweight_target,$*) \
		   	$(call gb_UnoApiHeadersTarget_get_target,$*))

# TODO: add second parameter: root of rdb file (UCR vs. /)
define gb_UnoApiHeadersTarget_UnoApiHeadersTarget
$(call gb_UnoApiHeadersTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_lightweight_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))

$(call gb_UnoApiHeadersTarget_get_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_lightweight_target,$(1)) : UNOAPI_DEPS :=

endef

define gb_UnoApiHeadersTarget_add_headerfile

endef

define gb_UnoApiHeadersTarget__add_rdbfile
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : UNOAPI_DEPS += -X$(call gb_UnoApiTarget_get_target,$(2))

endef

define gb_UnoApiHeadersTarget_add_rdbfile
$(call gb_UnoApiHeadersTarget__add_rdbfile,$(1),$(2),target)
$(call gb_UnoApiHeadersTarget__add_rdbfile,$(1),$(2),comprehensive_target)
$(call gb_UnoApiHeadersTarget__add_rdbfile,$(1),$(2),lightweight_target)

endef

define gb_UnoApiHeadersTarget_add_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiHeadersTarget_add_rdbfile,$(1),$(rdb)))

endef

# vim: set noet sw=4 ts=4:
