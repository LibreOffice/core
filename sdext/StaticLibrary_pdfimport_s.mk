# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., Michael Stahl <mstahl@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

include $(dir $(realpath $(lastword $(MAKEFILE_LIST))))platform.mk

$(eval $(call gb_StaticLibrary_StaticLibrary,pdfimport_s))

$(eval $(call gb_StaticLibrary_use_packages,pdfimport_s,\
    basegfx_inc \
    comphelper_inc \
    cppu_inc \
    cppuhelper_inc \
    o3tl_inc \
    tools_inc \
))

$(eval $(call gb_StaticLibrary_use_api,pdfimport_s,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_use_externals,pdfimport_s,\
    zlib \
))

$(eval $(call gb_StaticLibrary_use_custom_headers,pdfimport_s,sdext/pdfimport))

$(eval $(call gb_StaticLibrary_set_include,pdfimport_s,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,pdfimport_s,\
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
    -DPDFI_IMPL_IDENTIFIER=\"com.sun.star.PDFImport-$(sdext_PLATFORM)\" \
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
