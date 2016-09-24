# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,jvmaccess))

$(eval $(call gb_Library_add_defs,jvmaccess,\
    -DJVMACCESS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_api,jvmaccess,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,jvmaccess,\
    cppu \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,jvmaccess,\
    jvmaccess/source/classpath \
    jvmaccess/source/unovirtualmachine \
    jvmaccess/source/virtualmachine \
))

# vim:set noet sw=4 ts=4:
