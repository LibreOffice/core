# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,connector))

$(eval $(call gb_Library_use_external,connector,boost_headers))

$(eval $(call gb_Library_use_udk_api,connector))

$(eval $(call gb_Library_use_libraries,connector,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,connector,io/source/connector/connector))

$(eval $(call gb_Library_add_exception_objects,connector,\
    io/source/connector/connector \
    io/source/connector/ctr_pipe \
    io/source/connector/ctr_socket \
))

# vim:set noet sw=4 ts=4:
