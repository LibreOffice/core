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

$(eval $(call gb_Library_Library,flash))

$(eval $(call gb_Library_set_componentfile,flash,filter/source/flash/flash))

$(eval $(call gb_Library_use_sdk_api,flash))

$(eval $(call gb_Library_set_include,flash,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,flash,\
	svt \
	vcl \
	utl \
	tl \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,flash))

$(eval $(call gb_Library_use_externals,flash,zlib))

$(eval $(call gb_Library_add_exception_objects,flash,\
	filter/source/flash/impswfdialog \
	filter/source/flash/swfdialog \
	filter/source/flash/swfexporter \
	filter/source/flash/swffilter \
	filter/source/flash/swfuno \
	filter/source/flash/swfwriter \
	filter/source/flash/swfwriter1 \
	filter/source/flash/swfwriter2 \
))

# vim: set noet sw=4 ts=4:
