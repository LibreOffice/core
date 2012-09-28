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

$(eval $(call gb_Library_Library,msfilter))

$(eval $(call gb_Library_use_package,msfilter,filter_inc))

$(eval $(call gb_Library_set_componentfile,msfilter,filter/source/msfilter/msfilter))

$(eval $(call gb_Library_use_sdk_api,msfilter))

$(eval $(call gb_Library_set_include,msfilter,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,msfilter,\
	-DMSFILTER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,msfilter,\
	svxcore \
	editeng \
	sfx \
	xo \
	sb \
	svt \
	tk \
	vcl \
	svl \
	sot \
	xmlscript \
	utl \
	tl \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,msfilter,\
	filter/source/msfilter/countryid \
	filter/source/msfilter/escherex \
	filter/source/msfilter/eschesdo \
	filter/source/msfilter/mscodec \
	filter/source/msfilter/msdffimp \
	filter/source/msfilter/msocximex \
	filter/source/msfilter/msoleexp \
	filter/source/msfilter/mstoolbar \
	filter/source/msfilter/msvbahelper \
	filter/source/msfilter/services \
	filter/source/msfilter/svdfppt \
	filter/source/msfilter/svxmsbas2 \
	filter/source/msfilter/rtfutil \
	filter/source/msfilter/util \
))

# vim: set noet sw=4 ts=4:
