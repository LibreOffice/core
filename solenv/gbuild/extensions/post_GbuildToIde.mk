#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(filter gbuildtoide,$(MAKECMDGOALS)),)

gb_FULLDEPS:=

gbuildtoide:
	true

.PHONY : foo
foo:
	true

define gb_LinkTarget__command
mkdir -p $(WORKDIR)/GbuildToIde/$(dir $(2))
mkdir -p $(WORKDIR)/LinkTarget/$(dir $(2))
printf '{"LINKTARGET": "%s"' '$(2)' > $(WORKDIR)/GbuildToIde/$(2)
printf ', "ILIBTARGET": "%s"' '$(ILIBTARGET)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "COBJECTS": "%s"' '$(COBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "CXXOBJECTS": "%s"' '$(CXXOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "YACCOBJECTS": "%s"' '$(YACCOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "OBJCOBJECTS": "%s"' '$(OBJCOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "OBJCXXOBJECTS": "%s"' '$(OBJCXXOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "ASMOBJECTS": "%s"' '$(ASMOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "GENCOBJECTS": "%s"' '$(GENCOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "GENCXXOBJECTS": "%s"' '$(GENCXXOBJECTS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "CFLAGS": "%s"' '$(T_CFLAGS) $(T_CFLAGS_APPEND)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "CXXFLAGS": "%s"' '$(T_CXXFLAGS) $(T_CXXFLAGS_APPEND)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "OBJCFLAGS": "%s"' '$(T_OBJCFLAGS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "OBJCXXFLAGS": "%s"' '$(T_OBJCXXFLAGS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "DEFS": "%s"' '$(DEFS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "INCLUDE": "%s"' '$(INCLUDE)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "LINKED_LIBS": "%s"' '$(LINKED_LIBS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf ', "LINKED_STATIC_LIBS": "%s"' '$(LINKED_STATIC_LIBS)' >> $(WORKDIR)/GbuildToIde/$(2)
printf '}\n' >> $(WORKDIR)/GbuildToIde/$(2)
endef

define gb_Postprocess_register_target
gbuildtoide : $(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3)))

$(call gb_LinkTarget_get_target,$(call gb_$(2)_get_linktarget,$(3))): $(gb_Helper_MISCDUMMY) foo
endef

gb_LinkTarget_use_static_libraries =
gb_UnoApiHeadersTarget_get_target = foo
gb_UnpackedTarball_get_final_target = foo
gb_WinResTarget_get_target = foo
gb_LinkTarget__get_headers_check =
gb_LinkTarget_add_cobject = $(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
gb_LinkTarget_add_cxxobject = $(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)
gb_LinkTarget_use_package =
gb_LinkTarget_add_sdi_headers =
gb_LinkTarget_use_external_project =
gb_LinkTarget_add_scanners =
gb_LinkTarget_add_grammars =
gb_LinkTarget__check_srcdir_paths =

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
