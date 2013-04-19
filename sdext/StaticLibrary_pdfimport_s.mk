# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,pdfimport_s))

$(eval $(call gb_StaticLibrary_use_packages,pdfimport_s,\
    basegfx_inc \
    comphelper_inc \
    cppu_odk_headers \
    cppuhelper_odk_headers \
	i18nlangtag_inc \
    o3tl_inc \
	rsc_inc \
	sal_generated \
	sal_odk_headers \
    tools_inc \
	unotools_inc \
	vcl_inc \
))

$(eval $(call gb_StaticLibrary_use_api,pdfimport_s,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_use_externals,pdfimport_s,\
    boost_headers \
    zlib \
))

$(eval $(call gb_StaticLibrary_use_custom_headers,pdfimport_s,sdext/pdfimport))

$(eval $(call gb_StaticLibrary_set_include,pdfimport_s,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,pdfimport_s,\
	-DBASEGFX_STATIC_LIBRARY \
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,pdfimport_s,\
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
