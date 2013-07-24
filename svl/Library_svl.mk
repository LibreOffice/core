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

$(eval $(call gb_Library_Library,svl))

$(eval $(call gb_Library_use_external,svl,boost_headers))

$(eval $(call gb_Library_set_componentfile,svl,svl/util/svl))

$(eval $(call gb_Library_set_include,svl,\
    -I$(SRCDIR)/svl/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,svl,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,svl))

$(eval $(call gb_Library_add_defs,svl,\
    -DSVL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,svl,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(filter TRUE,$(SOLAR_JAVA)), \
        jvmfwk) \
    sal \
    sot \
    tl \
    ucbhelper \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,svl,\
    svl/source/config/asiancfg \
    svl/source/config/cjkoptions \
    svl/source/config/ctloptions \
    svl/source/config/itemholder2 \
    svl/source/config/languageoptions \
    svl/source/filepicker/pickerhistory \
    svl/source/filerec/filerec \
    svl/source/items/aeitem \
    svl/source/items/cenumitm \
    svl/source/items/cintitem \
    svl/source/items/cntwall \
    svl/source/items/ctypeitm \
    svl/source/items/custritm \
    svl/source/items/dateitem \
    svl/source/items/eitem \
    svl/source/items/flagitem \
    svl/source/items/globalnameitem \
    svl/source/items/grabbagitem \
    svl/source/items/ilstitem \
    svl/source/items/imageitm \
    svl/source/items/intitem \
    svl/source/items/itemiter \
    svl/source/items/itempool \
    svl/source/items/itemprop \
    svl/source/items/itemset \
    svl/source/items/lckbitem \
    svl/source/items/macitem \
    svl/source/items/poolcach \
    svl/source/items/poolio \
    svl/source/items/poolitem \
    svl/source/items/ptitem \
    svl/source/items/rectitem \
    svl/source/items/rngitem \
    svl/source/items/sitem \
    svl/source/items/slstitm \
    svl/source/items/srchitem \
    svl/source/items/stritem \
    svl/source/items/style \
    svl/source/items/stylepool \
    svl/source/items/szitem \
    svl/source/items/visitem \
    svl/source/items/whiter \
    svl/source/misc/PasswordHelper \
    svl/source/misc/adrparse \
    $(if $(filter DESKTOP,$(BUILD_TYPE)),\
        svl/source/misc/documentlockfile) \
    svl/source/misc/filenotation \
    svl/source/misc/fstathelper \
    svl/source/misc/getstringresource \
    svl/source/misc/inethist \
    svl/source/misc/inettype \
    svl/source/misc/lngmisc \
    svl/source/misc/lockfilecommon \
    svl/source/misc/ownlist \
    svl/source/misc/sharecontrolfile \
    svl/source/misc/strmadpt \
    svl/source/misc/urihelper \
    svl/source/notify/brdcst \
    svl/source/notify/broadcast \
    svl/source/notify/hint \
    svl/source/notify/isethint \
    svl/source/notify/listener \
    svl/source/notify/listenerbase \
    svl/source/notify/listeneriter \
    svl/source/notify/lstner \
    svl/source/notify/smplhint \
    svl/source/numbers/numfmuno \
    svl/source/numbers/numhead \
    svl/source/numbers/numuno \
    svl/source/numbers/supservs \
    svl/source/numbers/zforfind \
    svl/source/numbers/zforlist \
    svl/source/numbers/zformat \
    svl/source/numbers/zforscan \
    svl/source/svsql/converter \
    svl/source/undo/undo \
    svl/source/uno/pathservice \
    svl/source/uno/registerservices \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,svl,\
    svl/source/svdde/ddecli \
    svl/source/svdde/ddedata \
    svl/source/svdde/ddeinf \
    svl/source/svdde/ddestrg \
    svl/source/svdde/ddesvr \
))

else
$(eval $(call gb_Library_add_exception_objects,svl,\
    svl/unx/source/svdde/ddedummy \
))
endif
# vim: set noet sw=4 ts=4:
