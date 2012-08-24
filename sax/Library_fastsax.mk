# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,fastsax))

$(eval $(call gb_Library_set_componentfile,fastsax,sax/source/fastparser/fastsax))

$(eval $(call gb_Library_set_include,fastsax,\
    -I$(SRCDIR)/sax/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,fastsax))

$(eval $(call gb_Library_use_externals,fastsax,\
	expat_utf8 \
	zlib \
))

$(eval $(call gb_Library_use_libraries,fastsax,\
    cppu \
    cppuhelper \
    sal \
	sax \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_static_libraries,fastsax,\
	sax_shared \
))

$(eval $(call gb_Library_add_exception_objects,fastsax,\
	sax/source/fastparser/facreg \
	sax/source/fastparser/fastparser \
))

# vim: set noet sw=4 ts=4:
