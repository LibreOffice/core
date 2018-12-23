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
#ifndef INCLUDED_SFX2_SFXHELP_HXX
#define INCLUDED_SFX2_SFXHELP_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/help.hxx>
#include <memory>

class SfxFrame;

class SFX2_DLLPUBLIC SfxHelp : public Help
{
    bool            bIsDebug;

private:
    SAL_DLLPRIVATE static bool Start_Impl( const OUString& rURL, const vcl::Window* pWindow, const OUString& rKeyword );
    SAL_DLLPRIVATE static bool Start_Impl(const OUString& rURL, weld::Widget* pWidget, const OUString& rKeyword);
    SAL_DLLPRIVATE virtual void SearchKeyword( const OUString& rKeyWord ) override;
    SAL_DLLPRIVATE virtual bool Start( const OUString& rURL, const vcl::Window* pWindow ) override;
    SAL_DLLPRIVATE virtual bool Start(const OUString& rURL, weld::Widget* pWidget) override;
    SAL_DLLPRIVATE static OUString GetHelpModuleName_Impl(const OUString &rHelpId);
    SAL_DLLPRIVATE static OUString CreateHelpURL_Impl( const OUString& aCommandURL, const OUString& rModuleName );
    SAL_DLLPRIVATE static void incBusy(const vcl::Window* pParent);
    SAL_DLLPRIVATE static void decBusy(const vcl::Window* pParent);

public:
    SfxHelp();
    virtual ~SfxHelp() override;

    virtual OUString        GetHelpText( const OUString&, const vcl::Window* pWindow ) override;

    static OUString         CreateHelpURL( const OUString& aCommandURL, const OUString& rModuleName );
    static OUString         GetDefaultHelpModule();
    static OUString         GetCurrentModuleIdentifier();
    // Check for built-in help
    static bool             IsHelpInstalled();
};

#endif // INCLUDED_SFX2_SFXHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
