# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,pythonloader))

$(eval $(call gb_Library_set_componentfile,pythonloader,pyuno/source/loader/pythonloader,pyuno))

$(eval $(call gb_Library_set_include,pythonloader,\
    -I$(SRCDIR)/pyuno/source/loader \
    -I$(SRCDIR)/pyuno/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_api,pythonloader,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,pythonloader,\
    cppu \
    cppuhelper \
    pyuno \
    sal \
))

$(eval $(call gb_Library_use_externals,pythonloader,\
    boost_headers \
    python \
))

$(eval $(call gb_Library_add_exception_objects,pythonloader,\
    pyuno/source/loader/pyuno_loader \
))

# vim:set noet sw=4 ts=4:
