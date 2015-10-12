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

#ifndef INCLUDED_VCL_INC_UNX_KDE_KDEDATA_HXX
#define INCLUDED_VCL_INC_UNX_KDE_KDEDATA_HXX

#include <unx/saldisp.hxx>
#include <unx/saldata.hxx>
#include <unx/salframe.h>

class KDEData : public X11SalData
{
public:
    KDEData( SalInstance *pInstance ) : X11SalData( SAL_DATA_KDE3, pInstance ) {}
    virtual ~KDEData();

    virtual void Init() override;
    virtual void initNWF() override;
    virtual void deInitNWF() override;
};

class SalKDEDisplay : public SalX11Display
{
public:
    SalKDEDisplay( Display* pDisp );
    virtual ~SalKDEDisplay();
};

class KDESalFrame : public X11SalFrame
{
    static const int nMaxGraphics = 2;

    struct GraphicsHolder
    {
        X11SalGraphics*     pGraphics;
        bool                bInUse;
        GraphicsHolder()
                : pGraphics( NULL ),
                  bInUse( false )
        {}
        ~GraphicsHolder();
    };
    GraphicsHolder m_aGraphics[ nMaxGraphics ];

public:
    KDESalFrame( SalFrame* pParent, SalFrameStyleFlags );
    virtual ~KDESalFrame();

    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics( SalGraphics *pGraphics ) override;
    virtual void updateGraphics( bool bClear ) override;
    virtual void UpdateSettings( AllSettings& rSettings ) override;
    virtual void Show( bool bVisible, bool bNoActivate ) override;
};

class KDESalInstance : public X11SalInstance
{
public:
    KDESalInstance( SalYieldMutex* pMutex )
            : X11SalInstance( pMutex ) {}
    virtual ~KDESalInstance() {}
    virtual SalFrame* CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;

    virtual bool hasNativeFileSelection() const override { return true; }

    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& ) override;
};

class KDEXLib : public SalXLib
{
    bool            m_bStartupDone;
    void*           m_pApplication;
    char**          m_pFreeCmdLineArgs;
    char**          m_pAppCmdLineArgs;
    int             m_nFakeCmdLineArgs;
public:
    KDEXLib() : SalXLib(),
        m_bStartupDone( false ),
        m_pApplication( NULL ),
        m_pFreeCmdLineArgs( NULL ),
        m_pAppCmdLineArgs( NULL ),
        m_nFakeCmdLineArgs( 0 )
        {}
    virtual ~KDEXLib();
    virtual void Init() override;

    void doStartup();
};

#endif // INCLUDED_VCL_INC_UNX_KDE_KDEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
