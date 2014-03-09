# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,tiledrendering))

$(eval $(call gb_Executable_use_external,tiledrendering,boost_headers))

$(eval $(call gb_Executable_use_api,tiledrendering,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_set_include,tiledrendering,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/uibase/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,tiledrendering,\
    vclmain \
))

$(eval $(call gb_Executable_use_libraries,tiledrendering,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
    tl \
    ucbhelper \
    vcl \
    sfx \
	sw \
))

$(eval $(call gb_Executable_add_exception_objects,tiledrendering,\
    sw/qa/tiledrendering/tiledrendering \
))

# vim: set noet sw=4 ts=4:
