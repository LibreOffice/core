# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,pyuno))

$(eval $(call gb_Library_set_include,pyuno,\
    -I$(SRCDIR)/pyuno/source/loader \
    -I$(SRCDIR)/pyuno/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,pyuno,\
	-DLO_DLLIMPLEMENTATION_PYUNO \
))

$(eval $(call gb_Library_use_api,pyuno,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,pyuno,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
))

$(eval $(call gb_Library_use_externals,pyuno,\
    python \
))

$(eval $(call gb_Library_add_exception_objects,pyuno,\
    pyuno/source/module/pyuno_runtime \
    pyuno/source/module/pyuno \
    pyuno/source/module/pyuno_struct \
    pyuno/source/module/pyuno_callable \
    pyuno/source/module/pyuno_module \
    pyuno/source/module/pyuno_type \
    pyuno/source/module/pyuno_util \
    pyuno/source/module/pyuno_except \
    pyuno/source/module/pyuno_adapter \
    pyuno/source/module/pyuno_gc \
    pyuno/source/module/pyuno_iterator \
))

# vim:set noet sw=4 ts=4:
