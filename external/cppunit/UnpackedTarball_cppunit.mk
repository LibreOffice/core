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

# * external/cppunit/order.patch.0 upstreamed at <https://gerrit.libreoffice.org/c/cppunit/+/123963>
#   "Run tests in deterministic order";
# * external/cppunit/propagate-exceptions.patch.0 upstreamed at
#   <https://gerrit.libreoffice.org/c/cppunit/+/144074> "Introduce CPPUNIT_PROPAGATE_EXCEPTIONS
#   environment variable";
# * external/cppunit/sprintf.patch.0 upstreamed at
#   <https://gerrit.libreoffice.org/c/cppunit/+/147383> "Use snprintf instead of sprintf";
# * external/cppunit/clang.patch.0 upstreamed at
#   <https://gerrit.libreoffice.org/c/cppunit/+/147384> "Don't mis-apply GCC < 4.6 workaround for
#   Clang":
# * external/cppunit/0001-cid-1546460-COPY_INSTEAD_OF_MOVE.patch.1 upstreamed at
#   <https://gerrit.libreoffice.org/c/cppunit/+/161455> "cid#1546460 COPY_INSTEAD_OF_MOVE";
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	external/cppunit/windows.patch \
	external/cppunit/unix.patch \
	external/cppunit/CPPUNIT_PLUGIN_EXPORT.patch.0 \
	external/cppunit/enable-win32-debug.patch \
	external/cppunit/rtti.patch.0 \
	external/cppunit/order.patch.0 \
	external/cppunit/windows-arm64.patch.1 \
	external/cppunit/propagate-exceptions.patch.0 \
	external/cppunit/sprintf.patch.0 \
	external/cppunit/clang.patch.0 \
	external/cppunit/0001-cid-1546460-COPY_INSTEAD_OF_MOVE.patch.1 \
	external/cppunit/help-msvc-analyzer.patch \
))
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	external/cppunit/disable-dynloading.patch \
))
endif

# vim: set noet sw=4 ts=4:
