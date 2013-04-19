# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,streams))

$(eval $(call gb_Library_use_external,streams,boost_headers))

$(eval $(call gb_Library_use_udk_api,streams))

$(eval $(call gb_Library_use_libraries,streams,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,streams,io/source/stm/streams))

$(eval $(call gb_Library_add_exception_objects,streams,\
    io/source/stm/factreg \
    io/source/stm/odata \
    io/source/stm/omark \
    io/source/stm/opipe \
    io/source/stm/opump \
    io/source/stm/streamhelper \
))

# vim:set noet sw=4 ts=4:
