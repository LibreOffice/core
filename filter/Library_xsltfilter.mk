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

$(eval $(call gb_Library_Library,xsltfilter))

$(eval $(call gb_Library_set_componentfile,xsltfilter,filter/source/xsltfilter/xsltfilter))

$(eval $(call gb_Library_use_sdk_api,xsltfilter))

$(eval $(call gb_Library_set_include,xsltfilter,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,xsltfilter,\
	xo \
	tl \
	package2 \
	sax \
	comphelper \
	ucbhelper \
	cppuhelper \
	cppu \
	salhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,xsltfilter))

$(eval $(call gb_Library_use_externals,xsltfilter,\
	libxml2 \
	libxslt \
	libexslt \
))

$(eval $(call gb_Library_add_exception_objects,xsltfilter,\
	filter/source/xsltfilter/LibXSLTTransformer \
	filter/source/xsltfilter/OleHandler \
	filter/source/xsltfilter/XSLTFilter \
))

# vim: set noet sw=4 ts=4:
