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
mkdir -p $(WORKDIR)/GbuildToIde/$(dir $(2)) $(WORKDIR)/Headers/$(dir $(2))
$(if $(filter WNT,$(OS)),mkdir -p $(dir $(call gb_WinResTarget_get_target,dummy)))
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

endif
# vim: set noet ts=4 sw=4:
