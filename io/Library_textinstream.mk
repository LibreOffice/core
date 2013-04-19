# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,textinstream))

$(eval $(call gb_Library_use_api,textinstream,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,textinstream,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,textinstream,io/source/TextInputStream/textinstream))

$(eval $(call gb_Library_add_exception_objects,textinstream,\
    io/source/TextInputStream/TextInputStream \
))

# vim:set noet sw=4 ts=4:
