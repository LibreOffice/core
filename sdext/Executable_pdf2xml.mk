# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,pdf2xml))

$(eval $(call gb_Executable_use_sdk_api,pdf2xml))

$(eval $(call gb_Executable_use_externals,pdf2xml,\
    boost_headers \
    cppunit \
    zlib \
))

$(eval $(call gb_Executable_set_include,pdf2xml,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,pdf2xml,\
    basegfx \
    vcl \
    comphelper \
    cppu \
    unotest \
    cppuhelper \
    sal \
    xo \
))

$(eval $(call gb_Executable_use_library_objects,pdf2xml,pdfimport))

$(eval $(call gb_Executable_add_exception_objects,pdf2xml,\
    sdext/source/pdfimport/test/pdf2xml \
))

# vim:set noet sw=4 ts=4:
