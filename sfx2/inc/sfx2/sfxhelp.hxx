/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SFX_HELP_HXX
#define _SFX_HELP_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <vcl/help.hxx>
#include <tools/string.hxx>

class SfxHelp_Impl;
class SfxFrame;
class SFX2_DLLPUBLIC SfxHelp : public Help
{
    String          aTicket;        // for Plugins
    String          aUser;
    String          aLanguageStr;
    String          aCountryStr;
    sal_Bool        bIsDebug;
    SfxHelp_Impl*   pImp;

private:
    SAL_DLLPRIVATE sal_Bool Start_Impl( const String& rURL, const Window* pWindow, const String& rKeyword );
    SAL_DLLPRIVATE virtual sal_Bool SearchKeyword( const XubString& rKeyWord );
    SAL_DLLPRIVATE virtual sal_Bool Start( const String& rURL, const Window* pWindow );
    SAL_DLLPRIVATE virtual void OpenHelpAgent( const rtl::OString& sHelpId );
    SAL_DLLPRIVATE String GetHelpModuleName_Impl();
    SAL_DLLPRIVATE String CreateHelpURL_Impl( const String& aCommandURL, const String& rModuleName );

public:
    SfxHelp();
    ~SfxHelp();

    inline void             SetTicket( const String& rTicket )  { aTicket = rTicket; }
    inline void             SetUser( const String& rUser )      { aUser = rUser; }

    virtual XubString       GetHelpText( const String&, const Window* pWindow );

    static String           CreateHelpURL( const String& aCommandURL, const String& rModuleName );
    using Help::OpenHelpAgent;
    static void             OpenHelpAgent( SfxFrame* pFrame, const rtl::OString& sHelpId );
    static String           GetDefaultHelpModule();
    static ::rtl::OUString  GetCurrentModuleIdentifier();
};

#endif // #ifndef _SFX_HELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
