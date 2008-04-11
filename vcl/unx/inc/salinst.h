/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salinst.h,v $
 * $Revision: 1.17 $
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

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include <vcl/sv.h>
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#include <vcl/dllapi.h>
#include <vcl/salinst.hxx>

class VCL_DLLPUBLIC SalYieldMutex : public NAMESPACE_VOS(OMutex)
{
protected:
    ULONG                                       mnCount;
    NAMESPACE_VOS(OThread)::TThreadIdentifier   mnThreadId;

public:
                                                SalYieldMutex();

    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    ULONG                                       GetAcquireCount() const { return mnCount; }
    NAMESPACE_VOS(OThread)::TThreadIdentifier   GetThreadId() const { return mnThreadId; }
};

// -=-= SalInstanceData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VCL_DLLPUBLIC X11SalInstance : public SalInstance
{
protected:
    SalYieldMutex*                  mpSalYieldMutex;
    bool                            mbPrinterInit;

public:
    X11SalInstance( SalYieldMutex* pMutex )
            : mpSalYieldMutex( pMutex ),
              mbPrinterInit( false )
    {}
    virtual ~X11SalInstance();

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual void                DestroyFrame( SalFrame* pFrame );

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE );
    virtual void                DestroyObject( SalObject* pObject );

    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     USHORT nBitCount, const SystemGraphicsData *pData = NULL );
    virtual void                DestroyVirtualDevice( SalVirtualDevice* pDevice );

    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData );
    virtual void                DestroyInfoPrinter( SalInfoPrinter* pPrinter );
    virtual SalPrinter*     CreatePrinter( SalInfoPrinter* pInfoPrinter );
    virtual void                DestroyPrinter( SalPrinter* pPrinter );

    virtual void                GetPrinterQueueInfo( ImplPrnQueueList* pList );
    virtual void                GetPrinterQueueState( SalPrinterQueueInfo* pInfo );
    virtual void                DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo );
    virtual String             GetDefaultPrinter();

    virtual SalTimer*           CreateSalTimer();
    virtual SalOpenGL*          CreateSalOpenGL( SalGraphics* pGraphics );
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual SalSession*         CreateSalSession();

    virtual vos::IMutex*        GetYieldMutex();
    virtual ULONG               ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( ULONG nCount );

    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( USHORT nType );
    virtual SalMenu*        CreateMenu( BOOL bMenuBar );
    virtual void            DestroyMenu( SalMenu* pMenu);
    virtual SalMenuItem*    CreateMenuItem( const SalItemParams* pItemData );
    virtual void            DestroyMenuItem( SalMenuItem* pItem );

    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );


    bool isPrinterInit() const
    {
        return mbPrinterInit;
    }
};

#endif // _SV_SALINST_H

