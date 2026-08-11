# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,readlicense_oo/license))

readlicense_oo_DIR := $(gb_CustomTarget_workdir)/readlicense_oo/license
readlicense_oo_LICENSE_xml := $(SRCDIR)/readlicense_oo/license/license.xml
create_SBOM := $(SRCDIR)/solenv/bin/create-sbom.py

ifneq ($(OS),MACOSX)
SBOM_DIR := $(INSTDIR)/sbom/
else
SBOM_DIR := $(INSTDIR)/Resources/sbom/
endif

$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/LICENSE.html
$(call gb_CustomTarget_get_target,readlicense_oo/license) : SBOM

ifeq ($(OS),WNT)
$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/license.txt
else
$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/LICENSE
endif

$(readlicense_oo_DIR)/LICENSE.html : \
		$(SRCDIR)/readlicense_oo/license/license_html.xsl \
		$(readlicense_oo_LICENSE_xml) \
		| $(readlicense_oo_DIR)/.dir \
		  $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),XSL)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --novalid -o $@ \
			--stringparam build_type "$(BUILD_TYPE)" \
			--stringparam os "$(OS)" \
			--stringparam themes "$(WITH_THEMES)" \
			$(if $(MPL_SUBSET),,--stringparam no_mpl_subset no_mpl_subset) \
			$< \
			$(readlicense_oo_LICENSE_xml) \
		$(if $(filter WNT,$(OS)), \
			&& $(gb_AWK) 'sub("$$","\r")' $@ > $@.tmp \
			&& mv $@.tmp $@ \
		) \
	)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),XSL)

$(readlicense_oo_DIR)/LICENSE : \
		$(SRCDIR)/readlicense_oo/license/license_plain_text.xsl \
		$(readlicense_oo_LICENSE_xml) \
		| $(readlicense_oo_DIR)/.dir \
		  $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),XSL)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --novalid -o $@ \
			--stringparam build_type "$(BUILD_TYPE)" \
			--stringparam os "$(OS)" \
			--stringparam themes "$(WITH_THEMES)" \
			$(if $(MPL_SUBSET),,--stringparam no_mpl_subset no_mpl_subset) \
			$< \
			$(readlicense_oo_LICENSE_xml) \
	)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),XSL)

ifeq ($(OS),WNT)
$(readlicense_oo_DIR)/license.txt : \
		$(readlicense_oo_DIR)/LICENSE \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AWK,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),AWK)
	$(call gb_Helper_abbreviate_dirs, \
		$(gb_AWK) 'sub("$$","\r")' $< > $@.tmp && mv $@.tmp $@ \
	)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),AWK)
endif

SBOM : $(readlicense_oo_DIR)/LICENSE.html $(create_SBOM) \
		$(BUILDDIR)/instsetoo_native/util/openoffice.lst \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_brand.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_extensions.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_help.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_lang.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_ure.txt \
		$(call gb_InstallScript_get_target,setup_osl) \
		$(call gb_Helper_optional,ODK,$(call gb_InstallScript_get_target,sdkoo)) \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		| $(call gb_Postprocess_get_target,AllLibraries) \
		  $(call gb_Postprocess_get_target,AllExecutables) \
		  $(call gb_Postprocess_get_target,AllPackages)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PY ,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PY )
	$(foreach v, \
		$(filter PRODUCTNAME_WITHOUT_SPACES LIBO_VERSION% %TARBALL, $(.VARIABLES)), \
		$(eval export $(v)=$($v)) \
	)
	EXTERNALSFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(gb_Externals)) \
	&& $(call gb_ExternalExecutable_get_command,python) $(create_SBOM) \
		$(readlicense_oo_DIR) \
		$(readlicense_oo_DIR)/LICENSE.html \
		$(BUILDDIR)/instsetoo_native/util/openoffice.lst \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_ure.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_help.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_office_lang.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_brand.txt \
		$(SRCDIR)/setup_native/source/packinfo/packinfo_extensions.txt \
		$(call gb_InstallScript_get_target,setup_osl) \
		"$(if $(filter en-US,$(gb_WITH_LANG)),,en-US) $(gb_WITH_LANG)" \
		$${EXTERNALSFILE} \
	&& rm -f $${EXTERNALSFILE}
	mkdir -p $(SBOM_DIR)
	cp $(readlicense_oo_DIR)/*sbom.spdx.json $(SBOM_DIR)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PY )

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
