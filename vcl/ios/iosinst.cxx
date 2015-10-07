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

#include <basebmp/scanlineformats.hxx>

#include "ios/iosinst.hxx"
#include "headless/svpdummies.hxx"
#include "generic/gendata.hxx"
#include "quartz/utils.h"
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

class IosSalData : public SalGenericData
{
public:
    IosSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_IOS, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

void IosSalInstance::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
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

IosSalInstance::IosSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
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
                                  const std::list< OUString >& rButtons,
                                  int nDefButton );
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
                     SalFrameStyleFlags  nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::Format::ThirtyTwoBitTcMaskRGBA,
                       pSysParent )
    {
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE
    {
        IosSalInstance::getInstance()->GetWorkArea( rRect );
    }

    virtual void ShowFullScreen( bool, sal_Int32 ) SAL_OVERRIDE
    {
        SetPosSize( 0, 0, viewWidth, viewHeight,
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }

    virtual void UpdateSettings( AllSettings &rSettings ) SAL_OVERRIDE
    {
        // Clobber the UI fonts
        vcl::Font aFont( OUString( "Helvetica" ), Size( 0, 14 ) );

        StyleSettings aStyleSet = rSettings.GetStyleSettings();
        aStyleSet.SetAppFont( aFont );
        aStyleSet.SetHelpFont( aFont );
        aStyleSet.SetMenuFont( aFont );
        aStyleSet.SetToolFont( aFont );
        aStyleSet.SetLabelFont( aFont );
        aStyleSet.SetInfoFont( aFont );
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
    return new IosSalFrame( this, NULL, nStyle, pParent );
}

SalFrame *IosSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new IosSalFrame( this, pParent, nStyle, NULL );
}

// All the interesting stuff is slaved from the IosSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}

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
    IosSalInstance* pInstance = new IosSalInstance( new SalYieldMutex() );
    new IosSalData( pInstance );
    pInstance->AcquireYieldMutex(1);
    ImplGetSVData()->maWinData.mbNoSaveBackground = true;
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutex();
    delete pInst;
}

int IosSalSystem::ShowNativeDialog( const OUString& rTitle,
                                    const OUString& rMessage,
                                    const std::list< OUString >& rButtons,
                                    int nDefButton )
{
    (void)rButtons;
    (void)nDefButton;

    NSLog(@"%@: %@", CreateNSString(rTitle), CreateNSString(rMessage));

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
