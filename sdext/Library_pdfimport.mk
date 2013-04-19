# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,pdfimport))

$(eval $(call gb_Library_set_componentfile,pdfimport,sdext/source/pdfimport/pdfimport))

$(eval $(call gb_Library_use_sdk_api,pdfimport))

$(eval $(call gb_Library_set_include,pdfimport,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,pdfimport,\
    basegfx \
    vcl \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_static_libraries,pdfimport,\
    pdfimport_s \
))

$(eval $(call gb_Library_use_externals,pdfimport,\
	boost_headers \
    zlib \
    poppler \
))

$(eval $(call gb_Library_add_exception_objects,pdfimport,\
    sdext/source/pdfimport/services \
))

# vim:set noet sw=4 ts=4:
