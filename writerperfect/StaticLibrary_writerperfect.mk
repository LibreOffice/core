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

$(eval $(call gb_StaticLibrary_StaticLibrary,writerperfect))

$(eval $(call gb_StaticLibrary_use_packages,writerperfect,\
	comphelper_inc \
	cppu_inc \
	cppuhelper_inc \
	sot_inc \
	tools_inc \
	unotools_inc \
	xmloff_inc \
))

$(eval $(call gb_StaticLibrary_set_include,writerperfect,\
	$$(INCLUDE) \
    -I$(SRCDIR)/writerperfect/source \
))

$(eval $(call gb_StaticLibrary_use_externals,writerperfect,\
	wpd \
	wpg \
))

$(eval $(call gb_StaticLibrary_use_api,writerperfect,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,writerperfect,\
	writerperfect/source/filter/DocumentElement \
	writerperfect/source/filter/DocumentHandler \
	writerperfect/source/filter/FilterInternal \
	writerperfect/source/filter/FontStyle \
	writerperfect/source/filter/InternalHandler \
	writerperfect/source/filter/ListStyle \
	writerperfect/source/filter/OdgGenerator \
	writerperfect/source/filter/OdtGenerator \
	writerperfect/source/filter/PageSpan \
	writerperfect/source/filter/SectionStyle \
	writerperfect/source/filter/TableStyle \
	writerperfect/source/filter/TextRunStyle \
	writerperfect/source/stream/WPXSvStream \
))

# vim: set noet sw=4 ts=4:
