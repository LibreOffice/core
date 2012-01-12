/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <vclpluginapi.h>
#include <salinst.hxx>

class VCLPLUG_GEN_PUBLIC SalYieldMutex : public vos::OMutex
{
protected:
    sal_uIntPtr                                     mnCount;
    vos::OThread::TThreadIdentifier mnThreadId;

public:
                                                SalYieldMutex();

    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    sal_uIntPtr                                     GetAcquireCount() const { return mnCount; }
    vos::OThread::TThreadIdentifier GetThreadId() const { return mnThreadId; }
};

// -=-= SalInstanceData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VCLPLUG_GEN_PUBLIC X11SalInstance : public SalInstance
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

    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, sal_uIntPtr nStyle );
    virtual SalFrame*       CreateFrame( SalFrame* pParent, sal_uIntPtr nStyle );
    virtual void                DestroyFrame( SalFrame* pFrame );

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual void                DestroyObject( SalObject* pObject );

    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL );
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
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual SalSession*         CreateSalSession();

    virtual vos::IMutex*        GetYieldMutex();
    virtual sal_uIntPtr             ReleaseYieldMutex();
    virtual void                AcquireYieldMutex( sal_uIntPtr nCount );
    virtual bool                CheckYieldMutex();

    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( sal_uInt16 nType );

    virtual void*           GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    void                    FillFontPathList( std::list< rtl::OString >& o_rFontPaths );

    // dtrans implementation
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateClipboard( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& i_rArguments );
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDragSource();
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDropTarget();
    virtual void            AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);


    bool isPrinterInit() const
    {
        return mbPrinterInit;
    }
};

#endif // _SV_SALINST_H

