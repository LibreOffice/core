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

$(eval $(call gb_UnpackedTarball_set_patchlevel,java_websocket,1))

$(eval $(call gb_UnpackedTarball_add_patches,java_websocket,\
	external/java_websocket/patches/ant-build.patch \
	external/java_websocket/patches/no-slf4j.patch \
))

# vim: set noet sw=4 ts=4:
