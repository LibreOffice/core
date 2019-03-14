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

#include <premac.h>
#include <UIKit/UIKit.h>
#include <postmac.h>

#include "ios/iosinst.hxx"
#include "headless/svpdummies.hxx"
#include "unx/gendata.hxx"
#include "quartz/utils.h"
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>

// Totally wrong of course but doesn't seem to harm much in the iOS app.
static int viewWidth = 1, viewHeight = 1;

class IosSalData : public GenericUnixSalData
{
public:
    explicit IosSalData(SalInstance *pInstance)
        : GenericUnixSalData(SAL_DATA_IOS, pInstance)
    {
    }
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

void IosSalInstance::GetWorkArea( tools::Rectangle& rRect )
{
    rRect = tools::Rectangle( Point( 0, 0 ),
                       Size( viewWidth, viewHeight ) );
}

IosSalInstance *IosSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    IosSalData *pData = static_cast<IosSalData *>(ImplGetSVData()->mpSalData);
    if (!pData)
        return NULL;
    return static_cast<IosSalInstance *>(pData->m_pInstance);
}

IosSalInstance::IosSalInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SvpSalInstance( std::move(pMutex) )
{
}

IosSalInstance::~IosSalInstance()
{
}

class IosSalSystem : public SvpSalSystem {
public:
    IosSalSystem() : SvpSalSystem() {}
    virtual ~IosSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::vector< OUString >& rButtons );
};

SalSystem *IosSalInstance::CreateSalSystem()
{
    return new IosSalSystem();
}

class IosSalFrame : public SvpSalFrame
{
public:
    IosSalFrame( IosSalInstance *pInstance,
                     SalFrame           *pParent,
                     SalFrameStyleFlags  nSalFrameStyle)
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle )
    {
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( tools::Rectangle& rRect ) override
    {
        IosSalInstance::getInstance()->GetWorkArea( rRect );
    }

    virtual void ShowFullScreen( bool, sal_Int32 ) override
    {
        SetPosSize( 0, 0, viewWidth, viewHeight,
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }

    virtual void UpdateSettings( AllSettings &rSettings ) override
    {
        // Clobber the UI fonts
        vcl::Font aFont( OUString::fromUtf8( [[[UIFont systemFontOfSize:10] familyName] UTF8String] ), Size( 0, 10 ) );

        StyleSettings aStyleSet = rSettings.GetStyleSettings();
        aStyleSet.SetAppFont( aFont );
        aStyleSet.SetHelpFont( aFont );
        aStyleSet.SetMenuFont( aFont );
        aStyleSet.SetToolFont( aFont );
        aStyleSet.SetLabelFont( aFont );
        aStyleSet.SetRadioCheckFont( aFont );
        aStyleSet.SetPushButtonFont( aFont );
        aStyleSet.SetFieldFont( aFont );
        aStyleSet.SetIconFont( aFont );
        aStyleSet.SetTabFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }
};

SalFrame *IosSalInstance::CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle )
{
    pParent = NULL;
    return new IosSalFrame( this, NULL, nStyle );
}

SalFrame *IosSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new IosSalFrame( this, pParent, nStyle );
}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    (void) bDumpCore;

    NSLog(@"SalAbort: %s", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aEnv( "iOS" );
    return aEnv;
}

SalData::SalData() :
    m_pInstance( 0 ),
    mpFontList( 0 ),
    mxRGBSpace( CGColorSpaceCreateDeviceRGB() ),
    mxGraySpace( CGColorSpaceCreateDeviceGray() )
{
}

SalData::~SalData()
{
}

// This is our main entry point:
SalInstance *CreateSalInstance()
{
    IosSalInstance* pInstance = new IosSalInstance( std::make_unique<SvpSalYieldMutex>() );
    new IosSalData( pInstance );
    pInstance->AcquireYieldMutex();
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutexAll();
    delete pInst;
}

int IosSalSystem::ShowNativeDialog( const OUString& rTitle,
                                    const OUString& rMessage,
                                    const std::vector< OUString >& rButtons )
{
    (void)rButtons;

    NSLog(@"%@: %@", CreateNSString(rTitle), CreateNSString(rMessage));

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
