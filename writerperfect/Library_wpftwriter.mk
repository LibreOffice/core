# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,wpftwriter))

$(eval $(call gb_Library_set_include,wpftwriter,\
	-I$(SRCDIR)/writerperfect/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,wpftwriter,writerperfect/source/writer/wpftwriter))

$(eval $(call gb_Library_use_sdk_api,wpftwriter))

$(eval $(call gb_Library_use_libraries,wpftwriter,\
	comphelper \
	cppu \
	cppuhelper \
	vcl \
	sal \
	sfx \
	sot \
	svx \
	tl \
	ucbhelper \
	utl \
	vcl \
	writerperfect \
	xo \
))

$(eval $(call gb_Library_use_externals,wpftwriter,\
	abw \
	boost_headers \
	ebook \
	epubgen \
	etonyek \
	icu_headers \
	icui18n \
	icuuc \
	libxml2 \
	mwaw \
	odfgen \
	revenge \
	staroffice \
	wpd \
	wpg \
	wps \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,wpftwriter,\
	writerperfect/source/writer/AbiWordImportFilter \
	writerperfect/source/writer/EBookImportFilter \
	writerperfect/source/writer/EPUBExportFilter \
	writerperfect/source/writer/EPUBPackage \
	writerperfect/source/writer/MSWorksImportFilter \
	writerperfect/source/writer/MWAWImportFilter \
	writerperfect/source/writer/PagesImportFilter \
	writerperfect/source/writer/StarOfficeWriterImportFilter \
	writerperfect/source/writer/WordPerfectImportFilter \
	writerperfect/source/writer/exp/XMLBase64ImportContext \
	writerperfect/source/writer/exp/XMLTextFrameContext \
	writerperfect/source/writer/exp/txtparai \
	writerperfect/source/writer/exp/txtstyli \
	writerperfect/source/writer/exp/xmlfmt \
	writerperfect/source/writer/exp/xmlictxt \
	writerperfect/source/writer/exp/xmlimp \
	writerperfect/source/writer/exp/xmlmetai \
	writerperfect/source/writer/exp/xmltbli \
	writerperfect/source/writer/exp/xmltext \
))

# vim: set noet sw=4 ts=4:
