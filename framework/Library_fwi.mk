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

$(eval $(call gb_Library_Library,fwi))

$(eval $(call gb_Library_use_package,fwi,framework_inc))

$(eval $(call gb_Library_add_defs,fwi,\
    -DFWI_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,fwi))

$(eval $(call gb_Library_set_include,fwi,\
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,fwi,boost_headers))

$(eval $(call gb_Library_use_libraries,fwi,\
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,fwi,\
    framework/source/fwi/classes/converter \
    framework/source/fwi/classes/propertysethelper \
    framework/source/fwi/classes/protocolhandlercache \
    framework/source/fwi/helper/mischelper \
    framework/source/fwi/helper/networkdomain \
    framework/source/fwi/helper/shareablemutex \
    framework/source/fwi/jobs/configaccess \
    framework/source/fwi/jobs/jobconst \
    framework/source/fwi/threadhelp/lockhelper \
    framework/source/fwi/threadhelp/transactionmanager \
    framework/source/fwi/uielement/constitemcontainer \
    framework/source/fwi/uielement/itemcontainer \
    framework/source/fwi/uielement/rootitemcontainer \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,fwi,\
    advapi32 \
))
endif

# vim: set noet sw=4 ts=4:
