/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdedata.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:03:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_KDEDATA_HXX
#define _VCL_KDEDATA_HXX

#include <saldisp.hxx>
#include <saldata.hxx>
#include <salframe.h>

class KDEData : public X11SalData
{
public:
    KDEData() {}
    virtual ~KDEData();

    virtual void Init();
    virtual void initNWF();
    virtual void deInitNWF();
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
    KDESalFrame( SalFrame* pParent, ULONG nStyle );
    virtual ~KDESalFrame();

    virtual SalGraphics* GetGraphics();
    virtual void ReleaseGraphics( SalGraphics *pGraphics );
    virtual void updateGraphics();
    virtual void UpdateSettings( AllSettings& rSettings );
    virtual void Show( BOOL bVisible, BOOL bNoActivate );
};

class KDESalInstance : public X11SalInstance
{
public:
    KDESalInstance( SalYieldMutex* pMutex )
            : X11SalInstance( pMutex ) {}
    virtual ~KDESalInstance() {}
    virtual SalFrame* CreateFrame( SalFrame* pParent, ULONG nStyle );
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
    virtual void Init();

    void doStartup();
};

#endif // _VCL_KDEDATA_HXX
