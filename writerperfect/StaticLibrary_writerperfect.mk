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

$(eval $(call gb_StaticLibrary_set_include,writerperfect,\
	$$(INCLUDE) \
    -I$(SRCDIR)/writerperfect/source \
))

$(eval $(call gb_StaticLibrary_use_externals,writerperfect,\
	boost_headers \
	wpd \
	wpg \
))

$(eval $(call gb_StaticLibrary_use_api,writerperfect,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,writerperfect,\
	writerperfect/source/common/DocumentElement \
	writerperfect/source/common/DocumentHandler \
	writerperfect/source/common/FilterInternal \
	writerperfect/source/common/FontStyle \
	writerperfect/source/common/InternalHandler \
	writerperfect/source/common/ListStyle \
	writerperfect/source/common/OdgGenerator \
	writerperfect/source/common/OdtGenerator \
	writerperfect/source/common/PageSpan \
	writerperfect/source/common/SectionStyle \
	writerperfect/source/common/TableStyle \
	writerperfect/source/common/TextRunStyle \
	writerperfect/source/common/WPXSvStream \
))

# vim: set noet sw=4 ts=4:
