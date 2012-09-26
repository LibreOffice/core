# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sax))

$(eval $(call gb_Library_use_package,sax,\
    sax_inc \
))

$(eval $(call gb_Library_set_include,sax,\
    -I$(SRCDIR)/sax/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,sax))

$(eval $(call gb_Library_use_libraries,sax,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_defs,sax,\
    -DSAX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_exception_objects,sax,\
    sax/source/tools/converter \
    sax/source/tools/fastattribs \
	sax/source/tools/fastserializer \
    sax/source/tools/fshelper \
))

# vim: set noet sw=4 ts=4:
