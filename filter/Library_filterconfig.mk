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

$(eval $(call gb_Library_Library,filterconfig))

$(eval $(call gb_Library_set_componentfile,filterconfig,filter/source/config/cache/filterconfig1))

$(eval $(call gb_Library_use_external,filterconfig,boost_headers))

$(eval $(call gb_Library_use_custom_headers,filterconfig,officecfg/registry))

$(eval $(call gb_Library_use_sdk_api,filterconfig))

$(eval $(call gb_Library_set_include,filterconfig,\
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Library_use_libraries,filterconfig,\
	fwe \
	utl \
	tl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	i18nlangtag \
	$(gb_UWINAPI) \
))


$(eval $(call gb_Library_add_exception_objects,filterconfig,\
	filter/source/config/cache/basecontainer \
	filter/source/config/cache/cacheitem \
	filter/source/config/cache/cacheupdatelistener \
	filter/source/config/cache/configflush \
	filter/source/config/cache/contenthandlerfactory \
	filter/source/config/cache/filtercache \
	filter/source/config/cache/filterfactory \
	filter/source/config/cache/frameloaderfactory \
	filter/source/config/cache/lateinitlistener \
	filter/source/config/cache/lateinitthread \
	filter/source/config/cache/querytokenizer \
	filter/source/config/cache/registration \
	filter/source/config/cache/typedetection \
))

# vim: set noet sw=4 ts=4:
