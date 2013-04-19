# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,textoutstream))

$(eval $(call gb_Library_use_api,textoutstream,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,textoutstream,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,textoutstream,io/source/TextOutputStream/textoutstream))

$(eval $(call gb_Library_add_exception_objects,textoutstream,\
    io/source/TextOutputStream/TextOutputStream \
))

# vim:set noet sw=4 ts=4:
