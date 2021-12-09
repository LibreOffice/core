# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,postprocess))

# UGLY: postprocess_XCDS variable from CustomTarget_registry
# is reused in Package_registry!

$(eval $(call gb_Module_add_targets,postprocess,\
    $(if $(ENABLE_CUSTOMTARGET_COMPONENTS),CustomTarget_components) \
	CustomTarget_registry \
	Rdb_services \
))
ifeq ($(gb_Side),host)
$(eval $(call gb_Module_add_targets,postprocess,\
	CustomTarget_images \
	Package_images \
))
endif

$(eval $(call gb_Module_add_l10n_targets,postprocess,\
	Package_registry \
))

ifeq ($(WINDOWS_BUILD_SIGNING),TRUE)
$(eval $(call gb_Module_add_targets,postprocess,\
	CustomTarget_signing \
))
endif

# For configurations that use fontconfig (cf. inclusion of
# vcl/unx/generic/fontmanager/fontconfig.cxx in Library_vcl), add
# instdir/share/fonts/truetype/fc_local.conf when it shall
# contain content from at least one of external/more_fonts/fc_local.snippet
# (conditional on MORE_FONTS in BUILD_TYPE) and
# extras/source/truetype/symbol/fc_local.snippet (unconditional):
ifneq ($(USING_X11)$(DISABLE_GUI)$(filter ANDROID,$(OS)),)
$(eval $(call gb_Module_add_targets,postprocess, \
    CustomTarget_fontconfig \
    Package_fontconfig \
))
endif

ifeq ($(OS),LINUX)
ifneq ($(PKGFORMAT),)
$(eval $(call gb_Module_add_check_targets,postprocess,\
	CustomTarget_check_dynamic_objects \
))
endif
endif

# Implementation com.sun.star.xml.security.SEInitializer_Gpg
# does not provide a constructor or factory in case of MPLv2 subset
ifneq ($(MPL_SUBSET),TRUE)
$(eval $(call gb_Module_add_check_targets,postprocess,\
	CppunitTest_services \
))
endif

# vim: set noet sw=4 ts=4:
