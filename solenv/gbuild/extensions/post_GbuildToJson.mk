# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


# Outstanding work:
#
# fill files names in:
#   --ASMOBJECTS
#   --GENCOBJECTS
#   --YACCOBJECTS
#   --LEXOBJECTS
#   --JAVAOBJECTS
#   --PYTHONOBJECTS
#
# Add black listed modules a json files (--DENYLIST)
#
# Reduce number of denylisted modules

ifneq ($(filter gbuildtojson,$(MAKECMDGOALS)),)

ifeq ($(MAKE_VERSION),3.81)
$(error make version 3.81 not supported for gbuildtojson; install newer make via LODE)
endif

# possibly recurse to ensure gbuildtojson was build before running the modded make
gb_GbuildToJson_prep := $(shell $(MAKE) -f $(SRCDIR)/solenv/Makefile Executable_gbuildtojson)
gb_FULLDEPS:=

gbuildtojson:
	@true


gb_GbuildToJson_PHONY := $(WORKDIR)/GBUILDTOJSONPHONY
.PHONY : $(gb_GbuildToJson_PHONY)
$(gb_GbuildToJson_PHONY):
	@true



define gb_LinkTarget__command
mkdir -p $(WORKDIR)/GbuildToJson/$(dir $(2))
mkdir -p $(WORKDIR)/LinkTarget/$(dir $(2))
$(if $(GBUILDTOJSON_LD_LIBRARY_PATH),LD_LIBRARY_PATH=$(GBUILDTOJSON_LD_LIBRARY_PATH)) \
$(call gb_Executable_get_command,gbuildtojson) \
--makefile=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_MAKEFILE)) \
--linktarget=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(2)) \
--ilibtarget=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(ILIBTARGET)) \
--cxxobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(CXXOBJECTS)) \
--yaccobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(YACCOBJECTS)) \
--objcobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(OBJCOBJECTS)) \
--objcxxobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(OBJCXXOBJECTS)) \
--cxxclrobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(CXXCLROBJECTS)) \
--asmobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(ASMOBJECTS)) \
--lexobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(LEXOBJECTS)) \
--gencobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(GENCOBJECTS)) \
--gencxxobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(GENCXXOBJECTS)) \
--gencxxclrobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(GENCXXCLROBJECTS)) \
--cobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(COBJECTS)) \
--javaobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(JAVAOBJECTS)) \
--pythonobjects=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(PYTHONOBJECTS)) \
--cflags=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CFLAGS)) \
--cflagsappend=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CFLAGS_APPEND)) \
--cxxflags=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXFLAGS)) \
--cxxflagsappend=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXFLAGS_APPEND)) \
--objcflags=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCFLAGS)) \
--objcflagsappend=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCFLAGS_APPEND)) \
--objcxxflags=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCXXFLAGS)) \
--objcxxflagsappend=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCXXFLAGS_APPEND)) \
--cxxclrflags=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXCLRFLAGS)) \
--cxxclrflagsappend=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXCLRFLAGS_APPEND)) \
--defs=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(DEFS)) \
--include=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(INCLUDE)) \
--linked_libs=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(LINKED_LIBS)) \
--linked_static_libs=$(call gb_var2file,$(shell $(gb_MKTEMP)),100,$(LINKED_STATIC_LIBS)) \
> $(WORKDIR)/GbuildToJson/$(2)
endef

define gb_Postprocess_register_target
gbuildtojson : $(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3)))

$(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3))): $(gb_Helper_MISCDUMMY) $(gb_GbuildToJson_PHONY)
$(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3))): T_MAKEFILE := $(lastword $(MAKEFILE_LIST))
endef

define gb_CppunitTest_register_target
gbuildtojson : $(call gb_LinkTarget_get_target,$(2))

$(call gb_LinkTarget_get_target,$(2)): $(gb_Helper_MISCDUMMY) $(gb_GbuildToJson_PHONY)
$(call gb_LinkTarget_get_target,$(2)): T_MAKEFILE := $(lastword $(MAKEFILE_LIST))
endef

gb_LinkTarget_use_static_libraries =
gb_UnoApiHeadersTarget_get_target = $(gb_Helper_MISCDUMMY)
gb_UnpackedTarball_get_final_target = $(gb_Helper_MISCDUMMY)
gb_LinkTarget__get_headers_check =
define gb_LinkTarget_add_cobject
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)

endef
define gb_LinkTarget_add_cxxobject_internal
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)

endef
define gb_LinkTarget_add_generated_c_object
$(call gb_LinkTarget_get_target,$(1)) : GENCOBJECTS += $(2)

endef
define gb_LinkTarget_add_generated_cxx_object_internal
$(call gb_LinkTarget_get_target,$(1)) : GENCXXOBJECTS += $(2)

endef
define gb_LinkTarget_add_generated_cxxclrobject
$(call gb_LinkTarget_get_target,$(1)) : GENCXXCLROBJECTS += $(2)

endef
define gb_LinkTarget_add_objcobject
$(call gb_LinkTarget_get_target,$(1)) : OBJCOBJECTS += $(2)

endef
define gb_LinkTarget_add_objcxxobject
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXOBJECTS += $(2)

endef
define gb_LinkTarget_add_cxxclrobject
$(call gb_LinkTarget_get_target,$(1)) : CXXCLROBJECTS += $(2)

endef
define gb_LinkTarget_add_scanners
$(call gb_LinkTarget_get_target,$(1)) : LEXOBJECTS += $(2)

endef
define gb_LinkTarget_add_grammars
$(call gb_LinkTarget_get_target,$(1)) : YACCOBJECTS += $(2)

endef
gb_LinkTarget_use_package =
gb_LinkTarget_use_generated_package =
gb_LinkTarget_add_sdi_headers =
gb_LinkTarget_use_external_project =
gb_LinkTarget__check_srcdir_paths =
gb_LinkTarget__command_objectlist = @true
gb_WinResTarget_WinResTarget_init =
gb_WinResTarget_set_rcfile =
gb_WinResTarget_add_defs =
gb_LinkTarget_set_nativeres =
gb_LinkTarget_add_nativeres =
gb_Library_set_componentfile =
gb_Library_add_componentimpl =

#$(call gb_Library_get_exports_target,%):
$(WORKDIR)/LinkTarget/Library/%.exports:
	@true

define gb_LinkTarget__use_custom_headers
$(call gb_LinkTarget__add_include,$(1),$(call gb_CustomTarget_get_workdir,$(2)))

endef


define gb_Module__add_target_impl
$(call gb_Module__read_targetfile,$(1),$(2),target)

$(call gb_Module_get_nonl10n_target,$(1)) : $(3)

endef

define gb_Module__add_check_target_impl
$(call gb_Module__read_targetfile,$(1),$(2),check target)

$(call gb_Module_get_check_target,$(1)) : $(3)

endef

define gb_Module_add_target
$(if $(filter Library_% Executable_%,$(2)),$(call gb_Module__add_target_impl,$(1),$(2),$$(gb_Module_CURRENTTARGET)))
endef

define gb_Module_add_check_target
$(if $(filter CppunitTest_% Library_%,$(2)),$(call gb_Module__add_check_target_impl,$(1),$(2),$$(gb_Module_CURRENTTARGET)))
endef

gb_Module_add_l10n_target =

gb_GbuildToJson_DENYLISTEDMODULES := cli_ure jurt external

define gb_Module__add_moduledir_impl
include $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))/$(2)/Module_$(notdir $(2)).mk
$(call gb_Module_get_target,$(1)) : $$(firstword $$(gb_Module_TARGETSTACK))
gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))

endef

define gb_Module_add_moduledir
$(if $(filter $(gb_GbuildToJson_DENYLISTEDMODULES),$(2)),,$(call gb_Module__add_moduledir_impl,$(1),$(2)))

endef

endif

# vim: set noet ts=4 sw=4:
