# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,pdfimport))

$(eval $(call gb_Library_set_componentfile,pdfimport,sdext/source/pdfimport/pdfimport,services))

$(eval $(call gb_Library_use_sdk_api,pdfimport))

$(eval $(call gb_Library_use_custom_headers,pdfimport,sdext/pdfimport))

$(eval $(call gb_Library_set_include,pdfimport,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,pdfimport,\
    basegfx \
    comphelper \
    vcl \
    cppu \
    cppuhelper \
    sal \
    tl \
    xo \
))

$(eval $(call gb_Library_use_externals,pdfimport,\
	boost_headers \
    zlib \
    $(if $(filter-out WNT MACOSX,$(OS)),fontconfig) \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_defs,pdfimport, \
    -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING \
))
endif

$(eval $(call gb_Library_add_defs,pdfimport, \
    -DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_Library_add_exception_objects,pdfimport,\
    sdext/source/pdfimport/filterdet \
    sdext/source/pdfimport/misc/pdfihelper \
    sdext/source/pdfimport/misc/pwdinteract \
    sdext/source/pdfimport/odf/odfemitter \
    sdext/source/pdfimport/pdfiadaptor \
    sdext/source/pdfimport/pdfparse/pdfentries \
    sdext/source/pdfimport/pdfparse/pdfparse \
    sdext/source/pdfimport/sax/emitcontext \
    sdext/source/pdfimport/sax/saxattrlist \
    sdext/source/pdfimport/tree/drawtreevisiting \
    sdext/source/pdfimport/tree/genericelements \
    sdext/source/pdfimport/tree/imagecontainer \
    sdext/source/pdfimport/tree/pdfiprocessor \
    sdext/source/pdfimport/tree/style \
    sdext/source/pdfimport/tree/treevisitorfactory \
    sdext/source/pdfimport/tree/writertreevisiting \
    sdext/source/pdfimport/wrapper/wrapper \
))

# vim:set noet sw=4 ts=4:
