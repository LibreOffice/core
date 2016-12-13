# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,pdfunzip))

$(eval $(call gb_Executable_use_sdk_api,pdfunzip))

$(eval $(call gb_Executable_use_externals,pdfunzip,\
    boost_headers \
    zlib \
))

$(eval $(call gb_Executable_set_include,pdfunzip,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,pdfunzip,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    vcl \
))

$(eval $(call gb_Executable_use_library_objects,pdfunzip,pdfimport))

$(eval $(call gb_Executable_add_exception_objects,pdfunzip,\
    sdext/source/pdfimport/test/pdfunzip \
))

# vim:set noet sw=4 ts=4:
