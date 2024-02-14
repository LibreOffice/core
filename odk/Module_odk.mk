# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/odk/build-examples_common.mk

$(eval $(call gb_Module_Module,odk))

$(eval $(call gb_Module_add_targets,odk,\
	$(if $(DOXYGEN),\
		CustomTarget_doxygen \
		GeneratedPackage_odk_doxygen \
	) \
	CustomTarget_html \
	CustomTarget_settings \
	Executable_unoapploader \
	Package_config \
	Package_docs \
	Package_html \
	Package_examples \
	Package_odk_headers \
	Package_odk_headers_generated \
	Package_settings \
	Package_settings_generated \
	Package_share_readme \
	Package_share_readme_generated \
	Package_scripts \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,odk,\
	$(if $(filter-out AARCH64_TRUE,$(CPUNAME)_$(CROSS_COMPILING)),Package_cli) \
	CustomTarget_config_win \
	Package_config_win \
))
endif

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_classes \
	CustomTarget_javadoc \
	GeneratedPackage_odk_javadoc \
	GeneratedPackage_uno_loader_classes \
))
endif

$(eval $(call gb_Module_add_check_targets,odk,\
	CppunitTest_odk_checkapi \
	CustomTarget_allheaders \
	CustomTarget_check \
))

ifneq ($(filter $(OS),LINUX MACOSX),)
$(eval $(call gb_Module_add_subsequentcheck_targets,odk, \
    CustomTarget_build-examples \
    $(if $(ENABLE_JAVA),\
        CustomTarget_build-examples_java \
    ) \
))
endif

# vim: set noet sw=4 ts=4:
