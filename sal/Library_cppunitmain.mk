# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cppunitmain))

$(eval $(call gb_Library_set_include,cppunitmain,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_use_libraries,cppunitmain,\
    sal \
    unoexceptionprotector \
    unobootstrapprotector \
    vclbootstrapprotector \
))

$(eval $(call gb_Library_use_externals,cppunitmain,\
    boost_headers \
    cppunit \
))

$(eval $(call gb_Library_add_exception_objects,cppunitmain,\
    sal/cppunittester/cppunittester \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Library_add_ldflags,cppunitmain,\
    /STACK:10000000 \
))

endif

# vim: set noet sw=4 ts=4:
