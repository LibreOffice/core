# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,expwrap))

$(eval $(call gb_Library_set_componentfile,expwrap,sax/source/expatwrap/expwrap))

$(eval $(call gb_Library_set_include,expwrap,\
    -I$(SRCDIR)/sax/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,expwrap))

$(eval $(call gb_Library_use_externals,expwrap,\
    expat_utf16 \
))

$(eval $(call gb_Library_use_libraries,expwrap,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_static_libraries,expwrap,\
	sax_shared \
))

$(eval $(call gb_Library_add_exception_objects,expwrap,\
	sax/source/expatwrap/attrlistimpl \
	sax/source/expatwrap/sax_expat \
	sax/source/expatwrap/saxwriter \
))

# vim: set noet sw=4 ts=4:
