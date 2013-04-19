# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,acceptor))

$(eval $(call gb_Library_use_external,acceptor,boost_headers))

$(eval $(call gb_Library_use_udk_api,acceptor))

$(eval $(call gb_Library_use_libraries,acceptor,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,acceptor,io/source/acceptor/acceptor))

$(eval $(call gb_Library_add_exception_objects,acceptor,\
    io/source/acceptor/acc_pipe \
    io/source/acceptor/acc_socket \
    io/source/acceptor/acceptor \
))

# vim:set noet sw=4 ts=4:
