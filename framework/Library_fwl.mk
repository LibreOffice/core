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

$(eval $(call gb_Library_Library,fwl))

$(eval $(call gb_Library_set_componentfile,fwl,framework/util/fwl))

$(eval $(call gb_Library_set_include,fwl,\
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,fwl))

$(eval $(call gb_Library_use_libraries,fwl,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nisolang1 \
    sal \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,fwl,\
    framework/source/classes/fwlresid \
    framework/source/dispatch/mailtodispatcher \
    framework/source/dispatch/oxt_handler \
    framework/source/dispatch/popupmenudispatcher \
    framework/source/dispatch/servicehandler \
    framework/source/recording/dispatchrecorder \
    framework/source/recording/dispatchrecordersupplier \
    framework/source/register/registertemp \
    framework/source/dispatch/dispatchdisabler \
    framework/source/services/dispatchhelper \
    framework/source/services/license \
    framework/source/services/mediatypedetectionhelper \
    framework/source/services/uriabbreviation \
    framework/source/uielement/fontmenucontroller \
    framework/source/uielement/fontsizemenucontroller \
    framework/source/uielement/footermenucontroller \
    framework/source/uielement/headermenucontroller \
    framework/source/uielement/langselectionmenucontroller \
    framework/source/uielement/logoimagestatusbarcontroller \
    framework/source/uielement/logotextstatusbarcontroller \
    framework/source/uielement/macrosmenucontroller \
    framework/source/uielement/newmenucontroller \
    framework/source/uielement/popupmenucontroller \
    framework/source/uielement/simpletextstatusbarcontroller \
    framework/source/uielement/toolbarsmenucontroller \
))

# vim: set noet sw=4 ts=4:
