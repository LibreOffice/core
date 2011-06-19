/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
