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

$(eval $(call gb_Library_Library,vclplug_win))

$(eval $(call gb_Library_set_plugin_for,vclplug_win,vcl))

$(eval $(call gb_Library_set_componentfile,vclplug_win,vcl/vclplug_win,services))

$(eval $(call gb_Library_set_include,vclplug_win,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/win \
))

$(eval $(call gb_Library_add_defs,vclplug_win,\
    -DVCLPLUG_WIN_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_use_custom_headers,vclplug_win,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_win))

$(eval $(call gb_Library_use_common_precompiled_header,vclplugin_win))

$(eval $(call gb_Library_use_libraries,vclplug_win,\
    $(call gb_Helper_optional,BREAKPAD, \
        crashreport) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    sal \
    salhelper \
    tl \
    utl \
))

$(eval $(call gb_Library_use_externals,vclplug_win,\
    boost_headers \
    epoxy \
    glm_headers \
    harfbuzz \
    $(if $(filter SKIA,$(BUILD_TYPE)),skia) \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_win,\
    vcl/source/opengl/win/context \
    vcl/win/app/saldata \
    vcl/win/app/salinfo \
    vcl/win/app/salinst \
    vcl/win/app/salshl \
    vcl/win/app/saltimer \
    vcl/win/dtrans/APNDataObject \
    vcl/win/dtrans/clipboardmanager \
    vcl/win/dtrans/DataFmtTransl \
    vcl/win/dtrans/DOTransferable \
    vcl/win/dtrans/DtObjFactory \
    vcl/win/dtrans/DTransHelper \
    vcl/win/dtrans/Fetc \
    vcl/win/dtrans/FetcList \
    vcl/win/dtrans/FmtFilter \
    vcl/win/dtrans/ftransl \
    vcl/win/dtrans/generic_clipboard \
    vcl/win/dtrans/globals \
    vcl/win/dtrans/idroptarget \
    vcl/win/dtrans/ImplHelper \
    vcl/win/dtrans/MtaOleClipb \
    vcl/win/dtrans/source \
    vcl/win/dtrans/sourcecontext \
    vcl/win/dtrans/target \
    vcl/win/dtrans/targetdragcontext \
    vcl/win/dtrans/targetdropcontext \
    vcl/win/dtrans/TxtCnvtHlp \
    vcl/win/dtrans/WinClipboard \
    vcl/win/dtrans/XNotifyingDataObject \
    vcl/win/dtrans/XTDataObject \
    vcl/win/gdi/gdiimpl \
    vcl/win/gdi/salbmp \
    vcl/win/gdi/salfont \
    vcl/win/gdi/salgdi \
    vcl/win/gdi/salgdi2 \
    vcl/win/gdi/salgdi_gdiplus \
    vcl/win/gdi/salnativewidgets-luna \
    vcl/win/gdi/salprn \
    vcl/win/gdi/salvd \
    vcl/win/gdi/winlayout \
    vcl/win/gdi/DWriteTextRenderer \
    vcl/win/window/keynames \
    vcl/win/window/salframe \
    vcl/win/window/salmenu \
    vcl/win/window/salobj \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        vcl/skia/win/gdiimpl ) \
))

$(eval $(call gb_Library_use_system_win32_libs,vclplug_win,\
    advapi32 \
    d2d1 \
    dwrite \
    gdi32 \
    gdiplus \
    imm32 \
    ole32 \
    oleaut32 \
    shell32 \
    shlwapi \
    uuid \
    version \
    winspool \
))

$(eval $(call gb_Library_add_nativeres,vclplug_win,vcl/salsrc))

# HACK: dependency on icon themes so running unit tests don't
# prevent delivering these by having open file handles on WNT
ifeq ($(gb_Side),host)
$(eval $(call gb_Library_use_package,vclplug_win,postprocess_images))
endif

# vim: set noet sw=4 ts=4:
