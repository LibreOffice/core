/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salinst.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SV_SALINST_HXX
#define _SV_SALINST_HXX

#ifdef __cplusplus

#include <tools/string.hxx>
#endif // __cplusplus
#include <vcl/sv.h>
#include <vcl/dllapi.h>

#ifdef __cplusplus

struct SystemParentData;
struct SalPrinterQueueInfo;
struct SalStatus;
struct ImplJobSetup;
class SalGraphics;
class SalFrame;
class SalObject;
class SalOpenGL;
class SalMenu;
class SalMenuItem;
class SalVirtualDevice;
class SalInfoPrinter;
class SalPrinter;
class SalTimer;
class ImplPrnQueueList;
class SalI18NImeStatus;
class SalSystem;
class SalBitmap;
struct SalItemParams;
class SalSession;
struct SystemGraphicsData;
struct SystemWindowData;

namespace vos { class IMutex; }

// ---------------
// - SalInstance -
// ---------------

class VCL_DLLPUBLIC SalInstance
{
public:
    typedef bool(*Callback)(void*,void*,int);
private:
    void*                       m_pEventInst;
    void*                       m_pErrorInst;
    Callback                    m_pEventCallback;
    Callback                    m_pErrorCallback;

public:
    SalInstance() :
            m_pEventInst( NULL ),
            m_pErrorInst( NULL ),
            m_pEventCallback( NULL ),
            m_pErrorCallback( NULL )
    {}
    virtual ~SalInstance();

    // Frame
    // DisplayName for Unix ???
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle ) = 0;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle ) = 0;
    virtual void                DestroyFrame( SalFrame* pFrame ) = 0;

    // Object (System Child Window)
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE ) = 0;
    virtual void                DestroyObject( SalObject* pObject ) = 0;

    // VirtualDevice
    // nDX and nDY in Pixel
    // nBitCount: 0 == Default(=as window) / 1 == Mono
    // pData allows for using a system dependent graphics or device context
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     USHORT nBitCount, const SystemGraphicsData *pData = NULL ) = 0;
    virtual void                DestroyVirtualDevice( SalVirtualDevice* pDevice ) = 0;

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updatet with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) = 0;
    virtual void                DestroyInfoPrinter( SalInfoPrinter* pPrinter ) = 0;
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter ) = 0;
    virtual void                DestroyPrinter( SalPrinter* pPrinter ) = 0;

    virtual void                GetPrinterQueueInfo( ImplPrnQueueList* pList ) = 0;
    virtual void                GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) = 0;
    virtual void                DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) = 0;
    virtual String             GetDefaultPrinter() = 0;

    // SalTimer
    virtual SalTimer*           CreateSalTimer() = 0;
    // SalOpenGL
    virtual SalOpenGL*          CreateSalOpenGL( SalGraphics* pGraphics ) = 0;
    // SalI18NImeStatus
    virtual SalI18NImeStatus*   CreateI18NImeStatus() = 0;
    // SalSystem
    virtual SalSystem*          CreateSalSystem() = 0;
    // SalBitmap
    virtual SalBitmap*          CreateSalBitmap() = 0;

    // YieldMutex
    virtual vos::IMutex*        GetYieldMutex() = 0;
    virtual ULONG               ReleaseYieldMutex() = 0;
    virtual void                AcquireYieldMutex( ULONG nCount ) = 0;

    // wait next event and dispatch
    // must returned by UserEvent (SalFrame::PostEvent)
    // and timer
    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents ) = 0;
    virtual bool                AnyInput( USHORT nType ) = 0;

                            // Menues
    virtual SalMenu*        CreateMenu( BOOL bMenuBar ) = 0;
    virtual void            DestroyMenu( SalMenu* pMenu) = 0;
    virtual SalMenuItem*    CreateMenuItem( const SalItemParams* pItemData ) = 0;
    virtual void            DestroyMenuItem( SalMenuItem* pItem ) = 0;

    // may return NULL to disable session management
    virtual SalSession*     CreateSalSession() = 0;

    // methods for XDisplayConnection

    // the parameters for the callbacks are:
    //    void* pInst:          pInstance form the SetCallback call
    //    void* pEvent:         address of the system specific event structure
    //    int   nBytes:         length of the system specific event structure
    void                SetEventCallback( void* pInstance, Callback pCallback )
    { m_pEventInst = pInstance; m_pEventCallback = pCallback; }
    Callback GetEventCallback() const
    { return m_pEventCallback; }
    bool                CallEventCallback( void* pEvent, int nBytes )
    { return m_pEventCallback ? m_pEventCallback( m_pEventInst, pEvent, nBytes ) : false; }
    void                SetErrorEventCallback( void* pInstance, Callback pCallback )
    { m_pErrorInst = pInstance; m_pErrorCallback = pCallback; }
    Callback            GetErrorEventCallback() const
    { return m_pErrorCallback; }
    bool                CallErrorCallback( void* pEvent, int nBytes )
    { return m_pErrorCallback ? m_pErrorCallback( m_pErrorInst, pEvent, nBytes ) : false; }

    enum ConnectionIdentifierType { AsciiCString, Blob };
    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) = 0;
};

// called from SVMain
SalInstance* CreateSalInstance();
void DestroySalInstance( SalInstance* pInst );

// -------------------------
// - SalInstance-Functions -
// -------------------------

void SalAbort( const XubString& rErrorText );

VCL_DLLPUBLIC const ::rtl::OUString& SalGetDesktopEnvironment();

#endif // __cplusplus

// -----------
// - SalData -
// -----------

void InitSalData();                         // called from Application-Ctor
void DeInitSalData();                       // called from Application-Dtor

void InitSalMain();
void DeInitSalMain();

// ----------
// - SVMain -
// ----------

// Callbacks (indepen in \sv\source\app\svmain.cxx)
VCL_DLLPUBLIC BOOL SVMain();

#endif // _SV_SALINST_HXX
