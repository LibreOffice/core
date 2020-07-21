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

$(eval $(call gb_Library_Library,dnd))

$(eval $(call gb_Library_set_componentfile,dnd,dtrans/util/dnd))

$(eval $(call gb_Library_use_externals,dnd, \
    boost_headers \
))

$(eval $(call gb_Library_use_sdk_api,dnd))

$(eval $(call gb_Library_use_libraries,dnd,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,dnd,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shell32 \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,dnd,\
	dtobj \
))

$(eval $(call gb_Library_add_exception_objects,dnd,\
	dtrans/source/win32/dnd/globals \
	dtrans/source/win32/dnd/idroptarget \
	dtrans/source/win32/dnd/sourcecontext \
	dtrans/source/win32/dnd/source \
	dtrans/source/win32/dnd/target \
	dtrans/source/win32/dnd/targetdragcontext \
	dtrans/source/win32/dnd/targetdropcontext \
))

# vim: set noet sw=4 ts=4:
