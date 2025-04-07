# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,java_websocket))

$(eval $(call gb_UnpackedTarball_set_tarball,java_websocket,$(JAVA_WEBSOCKET_TARBALL),,java_websocket))

$(eval $(call gb_UnpackedTarball_add_patches,java_websocket,\
	external/java_websocket/patches/ant-build.patch \
	external/java_websocket/patches/no-slf4j.patch \
	external/java_websocket/patches/0001-cid-1546192-SIC-Inner-class-could-be-made-static.patch \
	external/java_websocket/patches/0001-cid-1545614-UR-Uninitialized-read-of-field-in-constr.patch \
	external/java_websocket/patches/0002-cid-1545284-UR-Uninitialized-read-of-field-in-constr.patch \
	external/java_websocket/patches/0001-cid-1546292-DLS-Dead-local-store.patch \
	external/java_websocket/patches/0001-cid-1545227-Dm-Dubious-method-used.patch \
	external/java_websocket/patches/0001-cid-1545515-Dm-Dubious-method-used.patch \
	external/java_websocket/patches/0001-cid-1546264-Dm-Dubious-method-used.patch \
	external/java_websocket/patches/0001-cid-1546341-Resource-leak-on-an-exceptional-path.patch \
	external/java_websocket/patches/0001-cid-1545249-Bad-bit-shift-operation.patch \
	external/java_websocket/patches/0001-cid-1546344-Dm-Dubious-method-used.patch \
))

# vim: set noet sw=4 ts=4:
