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

# Library_xmlsecurity links against Library_xsec_gpg (on certain OS), which
# links against gpgmepp dynamic library from external project gpgmepp, which
# (for non-SYSTEM_GPGMEPP) is delivered to instdir/program in
# ExternalPackage_gpgme, and at least the Linux linker wants to see all
# (recursively) linked libraries when linking an executable:
ifneq ($(filter-out WNT MACOSX ANDROID IOS,$(OS)),)
ifneq ($(SYSTEM_GPGMEPP),TRUE)
$(call gb_Executable_get_target,pdfverify): \
    $(call gb_ExternalPackage_get_target,gpgme)
endif
endif

# vim:set noet sw=4 ts=4:
