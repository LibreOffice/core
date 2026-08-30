# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/sbom))

$(call gb_CustomTarget_get_target,instsetoo_native/sbom) : instsetoo_SBOM

instsetoo_create_SBOM := $(SRCDIR)/solenv/bin/create-sbom.py

ifneq ($(OS),MACOSX)
instsetoo_SBOM_DIR := $(INSTDIR)/sbom/
else
instsetoo_SBOM_DIR := $(INSTDIR)/Resources/sbom/
endif

instsetoo_SBOM : $(instsetoo_create_SBOM) \
		$(gb_CustomTarget_workdir)/readlicense_oo/license/LICENSE.html \
		$(BUILDDIR)/instsetoo_native/util/openoffice.lst \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_brand.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_help.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_lang.txt \
		$(call gb_Helper_optional,ODK,$(SRCDIR)/setup_native/source/packinfo/packinfo_sdkoo.txt) \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_ure.txt \
		$(call gb_InstallScript_get_target,setup_osl) \
		$(call gb_Helper_optional,ODK,$(call gb_InstallScript_get_target,sdkoo)) \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		| $(call gb_Postprocess_get_target,AllModulesButInstsetNative) \
		  $(call gb_CustomTarget_get_target,instsetoo_native/setup) \
		  $(gb_CustomTarget_workdir)/instsetoo_native/sbom/.dir
	$(if $(gb_External_StaticLink),,$(error can only be invoked on top-level))
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PY ,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PY )
# documents of packages that no longer exist would otherwise linger forever
	rm -f $(gb_CustomTarget_workdir)/instsetoo_native/sbom/*.json
	$(foreach v, \
		$(filter MPL_SUBSET PRODUCTNAME_WITHOUT_SPACES LIBO_VERSION% %TARBALL %SHA256SUM, $(.VARIABLES)), \
		$(eval export $(v)=$($v)) \
	)
	EXTERNALSFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_Externals)) \
	EXTERNALSTATICFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_External_StaticLink)) \
	EXTERNALPACKAGESTATICFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_ExternalPackage_StaticLink)) \
	&& $(if $(filter LINUX,$(OS_FOR_BUILD)),$(gb_Python_PRECOMMAND) $(INSTROOT_FOR_BUILD)/program/python.bin,$(call gb_ExternalExecutable_get_command,python)) \
		$(instsetoo_create_SBOM) \
		$(gb_CustomTarget_workdir)/instsetoo_native/sbom \
		$(gb_CustomTarget_workdir)/readlicense_oo/license/LICENSE.html \
		$(BUILDDIR)/instsetoo_native/util/openoffice.lst \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_ure.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_help.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_lang.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_brand.txt \
		$(call gb_InstallScript_get_target,setup_osl) \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_sdkoo.txt \
		$(call gb_InstallScript_get_target,sdkoo) \
		"$(if $(filter en-US,$(gb_WITH_LANG)),,en-US) $(gb_WITH_LANG)" \
		$${EXTERNALSFILE} \
		$${EXTERNALSTATICFILE} \
		$${EXTERNALPACKAGESTATICFILE} \
	&& rm -f $${EXTERNALSFILE} $${EXTERNALSTATICFILE} $${EXTERNALPACKAGESTATICFILE}
	rm -rf $(instsetoo_SBOM_DIR)
	mkdir -p $(instsetoo_SBOM_DIR)
	cp $(gb_CustomTarget_workdir)/instsetoo_native/sbom/*sbom.spdx.json $(instsetoo_SBOM_DIR)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PY )

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
