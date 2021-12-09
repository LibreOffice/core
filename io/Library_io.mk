# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,io))

$(eval $(call gb_Library_use_external,io,boost_headers))

$(eval $(call gb_Library_use_udk_api,io))

$(eval $(call gb_Library_use_libraries,io,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
))

$(eval $(call gb_Library_set_componentfile,io,io/source/io,ure/services))

$(eval $(call gb_Library_set_include,io,\
	-I$(SRCDIR)/io/source \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_exception_objects,io,\
    io/source/acceptor/acc_pipe \
    io/source/acceptor/acc_socket \
    io/source/acceptor/acceptor \
    io/source/connector/connector \
    io/source/connector/ctr_pipe \
    io/source/connector/ctr_socket \
    io/source/stm/odata \
    io/source/stm/omark \
    io/source/stm/opipe \
    io/source/stm/opump \
    io/source/stm/streamhelper \
    io/source/TextInputStream/TextInputStream \
    io/source/TextOutputStream/TextOutputStream \
))

# vim:set noet sw=4 ts=4:
