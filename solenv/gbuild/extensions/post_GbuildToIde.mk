#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(filter gbuildtoide,$(MAKECMDGOALS)),)

# possibly recurse to ensure gbuildtojson was build before running the modded make
gb_GbuildToIde_prep := $(shell $(MAKE) -f $(SRCDIR)/solenv/Makefile Executable_gbuildtojson)
gb_FULLDEPS:=

gbuildtoide:
	@true

.PHONY : foo
foo:
	@true

define gb_LinkTarget__command
mkdir -p $(WORKDIR)/GbuildToIde/$(dir $(2))
mkdir -p $(WORKDIR)/LinkTarget/$(dir $(2))
$(call gb_Executable_get_command,gbuildtojson) \
--linktarget=$(call var2file,$(shell $(gb_MKTEMP)),100,$(2)) \
--ilibtarget=$(call var2file,$(shell $(gb_MKTEMP)),100,$(ILIBTARGET)) \
--cxxobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(CXXOBJECTS)) \
--yaccobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(YACCOBJECTS)) \
--objcobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(OBJCOBJECTS)) \
--objcxxobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(OBJCXXOBJECTS)) \
--asmobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(ASMOBJECTS)) \
--gencobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(GENCOBJECTS)) \
--gencxxobjects=$(call var2file,$(shell $(gb_MKTEMP)),100,$(GENCXXOBJECTS)) \
--cflags=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_CFLAGS)) \
--cflagsappend=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_CFLAGS_APPEND)) \
--cxxflags=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXFLAGS)) \
--cxxflagsappend=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_CXXFLAGS_APPEND)) \
--objcflags=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCFLAGS)) \
--objcflagsappend=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCFLAGS_APPEND)) \
--objcxxflags=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCXXFLAGS)) \
--objcxxflagsappend=$(call var2file,$(shell $(gb_MKTEMP)),100,$(T_OBJCXXFLAGS_APPEND)) \
--defs=$(call var2file,$(shell $(gb_MKTEMP)),100,$(DEFS)) \
--include=$(call var2file,$(shell $(gb_MKTEMP)),100,$(INCLUDE)) \
--linked_libs=$(call var2file,$(shell $(gb_MKTEMP)),100,$(LINKED_LIBS)) \
--linked_static_libs=$(call var2file,$(shell $(gb_MKTEMP)),100,$(LINKED_STATIC_LIBS)) \
> $(WORKDIR)/GbuildToIde/$(2)
endef

define gb_Postprocess_register_target
gbuildtoide : $(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3)))

$(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3))): $(gb_Helper_MISCDUMMY) foo
endef

gb_LinkTarget_use_static_libraries =
gb_UnoApiHeadersTarget_get_target = foo
gb_UnpackedTarball_get_final_target = foo
gb_LinkTarget__get_headers_check =
gb_LinkTarget_add_cobject = $(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
gb_LinkTarget_add_cxxobject = $(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)
gb_LinkTarget_use_package =
gb_LinkTarget_add_sdi_headers =
gb_LinkTarget_use_external_project =
gb_LinkTarget_add_scanners =
gb_LinkTarget_add_grammars =
gb_LinkTarget__check_srcdir_paths =
gb_LinkTarget__command_objectlist = @true
gb_WinResTarget_WinResTarget_init =
gb_WinResTarget_set_rcfile =
gb_WinResTarget_add_defs =
gb_LinkTarget_set_nativeres =
gb_LinkTarget_add_nativeres =
gb_Library_set_componentfile =
gb_LinkTarget_use_restarget =


#$(call gb_Library_get_exports_target,%):
$(WORKDIR)/LinkTarget/Library/%.exports:
	@true

define gb_LinkTarget__use_custom_headers
$(call gb_LinkTarget__add_include,$(1),$(call gb_CustomTarget_get_workdir,$(2)))

endef

define gb_Module_add_target
$(if $(filter Library_% Executable_%,$(2)),$(call gb_Module__read_targetfile,$(1),$(2),target))

$(if $(filter Library_% Executable_%,$(2)),$(call gb_Module_get_nonl10n_target,$(1)) : $$(gb_Module_CURRENTTARGET))

endef

gb_Module_add_l10n_target =

endif

# vim: set noet ts=4 sw=4:
