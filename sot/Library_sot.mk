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

$(eval $(call gb_Library_Library,sot))

$(eval $(call gb_Library_set_componentfile,sot,sot/util/sot,services))

$(eval $(call gb_Library_use_external,sot,boost_headers))

$(eval $(call gb_Library_use_sdk_api,sot))

$(eval $(call gb_Library_add_defs,sot,\
    -DSOT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,sot,sot/inc/pch/precompiled_sot))

$(eval $(call gb_Library_set_include,sot,\
    -I$(SRCDIR)/sot/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,sot,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	i18nlangtag \
    tl \
    ucbhelper \
    utl \
))

$(eval $(call gb_Library_add_exception_objects,sot,\
    sot/source/unoolestorage/xolesimplestorage \
    sot/source/base/formats \
    sot/source/base/object \
    sot/source/base/exchange \
    sot/source/base/filelist \
    sot/source/sdstor/stg \
    sot/source/sdstor/stgavl \
    sot/source/sdstor/stgcache \
    sot/source/sdstor/stgdir \
    sot/source/sdstor/stgelem \
    sot/source/sdstor/stgio \
    sot/source/sdstor/stgole \
    sot/source/sdstor/stgstrms \
    sot/source/sdstor/storage \
    sot/source/sdstor/storinfo \
    sot/source/sdstor/ucbstorage \
))

# vim: set noet sw=4 ts=4:
