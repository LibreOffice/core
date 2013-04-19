# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,java_uno_accessbridge))

$(eval $(call gb_Library_use_custom_headers,java_uno_accessbridge,accessibility/bridge/inc))

$(eval $(call gb_Library_use_external,java_uno_accessbridge,boost_headers))

$(eval $(call gb_Library_use_sdk_api,java_uno_accessbridge,))

$(eval $(call gb_Library_use_libraries,java_uno_accessbridge,\
    cppu \
    jvmaccess \
    sal \
    salhelper \
    tl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,java_uno_accessbridge,\
    accessibility/bridge/source/java/WindowsAccessBridgeAdapter \
))

# vim:set noet sw=4 ts=4:
