# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,net_uno))

$(eval $(call gb_Library_use_udk_api,net_uno))

$(eval $(call gb_Library_set_include,net_uno,\
	-I$(SRCDIR)/bridges/source/net_uno \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,net_uno,\
	cppu \
	sal \
	salhelper \
))

$(eval $(call gb_Library_use_externals,net_uno,\
    frozen \
))


$(eval $(call gb_Library_add_exception_objects,net_uno,\
	bridges/source/net_uno/net_base \
	bridges/source/net_uno/net_bridge \
	bridges/source/net_uno/net_data \
	bridges/source/net_uno/net_func \
	bridges/source/net_uno/net_proxy \
))

# vim: set noet sw=4 ts=4:
