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

$(eval $(call gb_Library_Library,ftransl))

$(eval $(call gb_Library_set_componentfile,ftransl,dtrans/util/ftransl))

$(eval $(call gb_Library_use_sdk_api,ftransl))

$(eval $(call gb_Library_use_libraries,ftransl,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,ftransl,\
	advapi32 \
	gdi32 \
	ole32 \
))

$(eval $(call gb_Library_use_static_libraries,ftransl,\
	dtobj \
))

$(eval $(call gb_Library_add_exception_objects,ftransl,\
	dtrans/source/win32/ftransl/ftransl \
))

$(eval $(call gb_Library_set_include,ftransl,\
        -I$(SRCDIR)/dtrans/source/inc \
        $$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
