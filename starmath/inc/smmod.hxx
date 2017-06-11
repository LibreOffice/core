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

#ifndef INCLUDED_STARMATH_INC_SMMOD_HXX
#define INCLUDED_STARMATH_INC_SMMOD_HXX

#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>

#include <sfx2/module.hxx>

#include <unotools/options.hxx>
#include <memory>

class SfxObjectFactory;
class SmSymbolManager;
class SmMathConfig;

/*************************************************************************
|*
|* This subclass of <SfxModule> (which is a subclass of <SfxShell>) is
|* linked to the DLL. One instance of this class exists while the DLL is
|* loaded.
|*
|* SdModule is like to be compared with the <SfxApplication>-subclass.
|*
|* Remember: Don`t export this class! It uses DLL-internal symbols.
|*
\************************************************************************/

class SvtSysLocale;
class VirtualDevice;


OUString SmResId(const char* pId);

class SmLocalizedSymbolData
{
public:
    SmLocalizedSymbolData();
    ~SmLocalizedSymbolData();

    static const OUString GetUiSymbolName( const OUString &rExportName );
    static const OUString GetExportSymbolName( const OUString &rUiName );

    static const OUString GetUiSymbolSetName( const OUString &rExportName );
    static const OUString GetExportSymbolSetName( const OUString &rUiName );
};

class SmModule : public SfxModule, public utl::ConfigurationListener
{
    std::unique_ptr<svtools::ColorConfig> mpColorConfig;
    std::unique_ptr<SmMathConfig> mpConfig;
    std::unique_ptr<SmLocalizedSymbolData> mpLocSymbolData;
    std::unique_ptr<SvtSysLocale> mpSysLocale;
    VclPtr<VirtualDevice>    mpVirtualDev;

    static void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

public:
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START + SfxInterfaceId(0))

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    explicit SmModule(SfxObjectFactory* pObjFact);
    virtual ~SmModule() override;

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

    svtools::ColorConfig &  GetColorConfig();

    SmMathConfig *          GetConfig();
    SmSymbolManager &       GetSymbolManager();

    SmLocalizedSymbolData &   GetLocSymbolData();

    static void GetState(SfxItemSet&);

    const SvtSysLocale& GetSysLocale();

    VirtualDevice &     GetDefaultVirtualDev();

    //virtual methods for options dialog
    virtual std::unique_ptr<SfxItemSet> CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual VclPtr<SfxTabPage> CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet ) override;
};

#define SM_MOD() ( static_cast<SmModule*>(SfxApplication::GetModule(SfxToolsModule::Math)) )

#endif // INCLUDED_STARMATH_INC_SMMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
