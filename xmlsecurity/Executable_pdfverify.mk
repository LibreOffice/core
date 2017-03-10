# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,pdfverify))

$(eval $(call gb_Executable_use_sdk_api,pdfverify))

$(eval $(call gb_Executable_use_externals,pdfverify,\
	pdfium \
))

$(eval $(call gb_Executable_set_include,pdfverify,\
    $$(INCLUDE) \
    -I$(SRCDIR)/xmlsecurity/inc \
))

$(eval $(call gb_Executable_use_libraries,pdfverify,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    vcl \
    xmlsecurity \
))

$(eval $(call gb_Executable_add_exception_objects,pdfverify,\
    xmlsecurity/workben/pdfverify \
))

# vim:set noet sw=4 ts=4:
