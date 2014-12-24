# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,AppleRemote))

$(eval $(call gb_Library_use_system_darwin_frameworks,AppleRemote,\
    Cocoa \
    Carbon \
    IOKit \
))

$(eval $(call gb_Library_add_libs,AppleRemote,\
    -lobjc \
))

$(eval $(call gb_Library_add_objcobjects,AppleRemote,\
    apple_remote/source/KeyspanFrontRowControl \
    apple_remote/source/AppleRemote \
    apple_remote/source/RemoteControl \
    apple_remote/source/RemoteControlContainer \
    apple_remote/source/GlobalKeyboardDevice \
    apple_remote/source/HIDRemoteControlDevice \
    apple_remote/source/MultiClickRemoteBehavior \
    apple_remote/source/RemoteMainController \
))

# vim: set noet sw=4 ts=4:
