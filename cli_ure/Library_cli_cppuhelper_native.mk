# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_Library_Assembly,cli_cppuhelper_native))

# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
$(eval $(call gb_Library_add_cxxflags,cli_cppuhelper_native,\
	-AI $(gb_Helper_OUTDIRLIBDIR) \
	-clr \
	-wd4339 \
))

$(eval $(call gb_Library_add_ldflags,cli_cppuhelper_native,\
	-ignore:4248 \
	-keyfile:$(call gb_Helper_windows_path,$(SRCDIR)/cli_ure/source/cliuno.snk) \
))

# TODO do not encode filenames here
$(eval $(call gb_Library_add_ldflags,cli_cppuhelper_native,\
	-delayload:cppuhelper3MSC.dll \
	-delayload:cppu3.dll \
	-delayload:sal3.dll \
))

$(eval $(call gb_Library_use_internal_bootstrap_api,cli_cppuhelper_native,\
	udkapi \
))

$(eval $(call gb_Library_use_packages,cli_cppuhelper_native,\
	cli_ure_cliuno \
))

$(eval $(call gb_Library_use_libraries,cli_cppuhelper_native,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,cli_cppuhelper_native,\
	advapi32 \
	delayimp \
	mscoree \
	$(if $(USE_DEBUG_RUNTIME)\
		,msvcmrtd \
		,msvcmrt \
	) \
))

$(eval $(call gb_Library_add_exception_objects,cli_cppuhelper_native,\
	cli_ure/source/native/native_bootstrap \
	cli_ure/source/native/path \
))

$(eval $(call gb_Library_add_generated_exception_objects,cli_cppuhelper_native,\
	CustomTarget/cli_ure/source/native/assembly \
))

# vim: set noet sw=4 ts=4:
