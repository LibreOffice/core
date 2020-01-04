# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cppunit))

$(eval $(call gb_UnpackedTarball_set_tarball,cppunit,$(CPPUNIT_TARBALL),,cppunit))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,cppunit))

# gcc9.patch.0 addressed upstream with <https://gerrit.libreoffice.org/58690> "Avoid GCC 9
#  -Wdeprecated-copy":
# 0001-Unconditionally-use-C-11-noreturn.patch.1 upstream at <https://gerrit.libreoffice.org/67247>
#  "Unconditionally use C++11 [[noreturn]]"
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	external/cppunit/windows.patch \
	external/cppunit/unix.patch \
	external/cppunit/CPPUNIT_PLUGIN_EXPORT.patch.0 \
	external/cppunit/enable-win32-debug.patch \
	external/cppunit/rtti.patch.0 \
	external/cppunit/gcc9.patch.0 \
	external/cppunit/0001-Unconditionally-use-C-11-noreturn.patch.1 \
))
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	external/cppunit/disable-dynloading.patch \
))
endif

# vim: set noet sw=4 ts=4:
