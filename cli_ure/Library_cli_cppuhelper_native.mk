# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_Library_Assembly,cli_cppuhelper))

# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
$(eval $(call gb_Library_add_cxxflags,cli_cppuhelper,\
	-AI $(gb_Helper_OUTDIRLIBDIR) \
	-clr \
	-wd4339 \
))

$(eval $(call gb_Library_add_ldflags,cli_cppuhelper,\
	-ignore:4248 \
	-keyfile:$(call gb_Helper_windows_path,$(SRCDIR)/cli_ure/source/cliuno.snk) \
))

$(eval $(call gb_Library_add_ldflags,cli_cppuhelper,\
	-delayload:$(call gb_Library_get_filename,cppuhelper) \
	-delayload:$(call gb_Library_get_filename,cppu) \
	-delayload:$(call gb_Library_get_filename,sal) \
))

$(eval $(call gb_Library_use_internal_bootstrap_api,cli_cppuhelper,\
	udkapi \
))

$(eval $(call gb_Library_use_packages,cli_cppuhelper,\
	cli_ure_cliuno \
))

$(eval $(call gb_Library_use_libraries,cli_cppuhelper,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,cli_cppuhelper,\
	advapi32 \
	delayimp \
	mscoree \
	msvcmrt \
))

$(eval $(call gb_Library_add_exception_objects,cli_cppuhelper,\
	cli_ure/source/native/native_bootstrap \
	cli_ure/source/native/path \
))

$(eval $(call gb_Library_add_generated_exception_objects,cli_cppuhelper,\
	CustomTarget/cli_ure/source/native/assembly \
))

# vim: set noet sw=4 ts=4:
