# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,headless))

$(eval $(call gb_StaticLibrary_use_externals,headless,\
	boost_headers \
	freetype_headers \
))

$(eval $(call gb_StaticLibrary_use_api,headless,\
	offapi \
	udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,headless,\
	vcl/headless/svpbmp \
	vcl/headless/svpdummies \
	vcl/headless/svpelement \
	vcl/headless/svpframe \
	vcl/headless/svpprn \
	vcl/headless/svptext \
	vcl/headless/svpvd \
))

$(eval $(call gb_StaticLibrary_set_include,headless,\
	$$(INCLUDE) \
	-I$(SRCDIR)/vcl/inc \
))

# vim: set noet sw=4 ts=4:
