# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,odk))

$(eval $(call gb_Module_add_targets,odk,\
	$(if $(filter WNT,$(OS)),Package_cli) \
	$(if $(DOXYGEN),\
		CustomTarget_doxygen \
		GeneratedPackage_cpp_docs \
	) \
	CustomTarget_html \
	CustomTarget_settings \
	CustomTarget_autodoc \
	Executable_unoapploader \
	Package_autodoc \
	Package_bin \
	Package_config \
	Package_docs \
	Package_html \
	Package_examples \
	Package_lib \
	Package_odk_headers \
	Package_odk_headers_generated \
	Package_settings \
	Package_settings_generated \
	PackageSet_autodoc \
	PackageSet_odk_headers \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_config_win \
	Package_config_win \
))
else
$(eval $(call gb_Module_add_targets,odk,\
	Package_config_notwin \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,odk,\
	Package_macosx \
))
endif

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,odk,\
	$(if $(filter YESGCC,$(BUILD_UNOWINREG)$(COM)),CustomTarget_unowinreg) \
	CustomTarget_classes \
	CustomTarget_javadoc \
	$(if $(filter WNT,$(OS)),Library_unowinreg) \
	Package_javadoc \
	Package_uno_loader_classes \
	Package_unowinreg \
))
endif

$(eval $(call gb_Module_add_check_targets,odk,\
	CppunitTest_checkapi \
	CustomTarget_allheaders \
	CustomTarget_check \
))

# vim: set noet sw=4 ts=4:
