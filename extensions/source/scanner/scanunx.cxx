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

#include "scanner.hxx"
#include "sanedlg.hxx"
#include <o3tl/safeint.hxx>
#include <osl/thread.hxx>
#include <sal/log.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <memory>

#include <com/sun/star/scanner/ScannerException.hpp>

BitmapTransporter::BitmapTransporter()
{
    SAL_INFO("extensions.scanner", "BitmapTransporter");
}


BitmapTransporter::~BitmapTransporter()
{
    SAL_INFO("extensions.scanner", "~BitmapTransporter");
}


css::awt::Size BitmapTransporter::getSize()
{
    osl::MutexGuard aGuard( m_aProtector );
    css::awt::Size   aRet;

    // ensure that there is at least a header
    int nLen = m_aStream.TellEnd();
    if( nLen > 15 )
    {
        int nPreviousPos = m_aStream.Tell();
        m_aStream.Seek( 4 );
        m_aStream.ReadInt32( aRet.Width ).ReadInt32( aRet.Height );
        m_aStream.Seek( nPreviousPos );
    }
    else
        aRet.Width = aRet.Height = 0;


    return aRet;
}


Sequence< sal_Int8 > BitmapTransporter::getDIB()
{
    osl::MutexGuard aGuard( m_aProtector );
    int         nPreviousPos = m_aStream.Tell();

    // create return value
    int nBytes = m_aStream.TellEnd();
    m_aStream.Seek( 0 );

    Sequence< sal_Int8 > aValue( nBytes );
    m_aStream.ReadBytes( aValue.getArray(), nBytes );
    m_aStream.Seek( nPreviousPos );

    return aValue;
}

namespace {

struct SaneHolder
{
    Sane                m_aSane;
    Reference< css::awt::XBitmap > m_xBitmap;
    osl::Mutex          m_aProtector;
    ScanError           m_nError;
    bool                m_bBusy;

    SaneHolder() : m_nError(ScanError_ScanErrorNone), m_bBusy(false) {}
};

    typedef std::vector< std::shared_ptr<SaneHolder> > sanevec;
    class allSanes
    {
    private:
        int mnRefCount;
    public:
        sanevec m_aSanes;
        allSanes() : mnRefCount(0) {}
        void acquire();
        void release();
    };

    void allSanes::acquire()
    {
        ++mnRefCount;
    }

    void allSanes::release()
    {
        // was unused, now because of i99835: "Scanning interface not SANE API
        // compliant" destroy all SaneHolder to get Sane Dtor called
        --mnRefCount;
        if (!mnRefCount)
            m_aSanes.clear();
    }

    struct theSaneProtector : public rtl::Static<osl::Mutex, theSaneProtector> {};
    struct theSanes : public rtl::Static<allSanes, theSanes> {};

class ScannerThread : public osl::Thread
{
    std::shared_ptr<SaneHolder>               m_pHolder;
    Reference< css::lang::XEventListener >    m_xListener;
    ScannerManager*                           m_pManager; // just for the disposing call

public:
    virtual void SAL_CALL run() override;
    virtual void SAL_CALL onTerminated() override { delete this; }
public:
    ScannerThread( std::shared_ptr<SaneHolder> pHolder,
                   const Reference< css::lang::XEventListener >& listener,
                   ScannerManager* pManager );
    virtual ~ScannerThread() override;
};

}

ScannerThread::ScannerThread(std::shared_ptr<SaneHolder> pHolder,
                             const Reference< css::lang::XEventListener >& listener,
                             ScannerManager* pManager)
        : m_pHolder(std::move( pHolder )), m_xListener( listener ), m_pManager( pManager )
{
    SAL_INFO("extensions.scanner", "ScannerThread");
}


ScannerThread::~ScannerThread()
{
    SAL_INFO("extensions.scanner", "~ScannerThread");
}


void ScannerThread::run()
{
    osl_setThreadName("ScannerThread");

    osl::MutexGuard         aGuard( m_pHolder->m_aProtector );
    rtl::Reference<BitmapTransporter> pTransporter = new BitmapTransporter;

    m_pHolder->m_xBitmap = pTransporter;

    m_pHolder->m_bBusy = true;
    if( m_pHolder->m_aSane.IsOpen() )
    {
        int nOption = m_pHolder->m_aSane.GetOptionByName( "preview" );
        if( nOption != -1 )
            m_pHolder->m_aSane.SetOptionValue( nOption, false );

        m_pHolder->m_nError =
            m_pHolder->m_aSane.Start( *pTransporter ) ?
            ScanError_ScanErrorNone : ScanError_ScanCanceled;
    }
    else
        m_pHolder->m_nError = ScanError_ScannerNotAvailable;


    Reference< XInterface > xXInterface( static_cast< OWeakObject* >( m_pManager ) );
    m_xListener->disposing( css::lang::EventObject(xXInterface) );
    m_pHolder->m_bBusy = false;
}


void ScannerManager::AcquireData()
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    theSanes::get().acquire();
}


void ScannerManager::ReleaseData()
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    theSanes::get().release();
}


css::awt::Size ScannerManager::getSize()
{
    css::awt::Size aRet;
    aRet.Width = aRet.Height = 0;
    return aRet;
}


Sequence< sal_Int8 > ScannerManager::getDIB()
{
    return Sequence< sal_Int8 >();
}


Sequence< ScannerContext > ScannerManager::getAvailableScanners()
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( rSanes.empty() )
    {
        auto pSaneHolder = std::make_shared<SaneHolder>();
        if( Sane::IsSane() )
            rSanes.push_back( pSaneHolder );
    }

    if( Sane::IsSane() )
    {
        Sequence< ScannerContext > aRet{ { /* ScannerName */ u"SANE"_ustr, /* InternalData */ 0 } };
        return aRet;
    }

    return Sequence< ScannerContext >();
}


sal_Bool ScannerManager::configureScannerAndScan( ScannerContext& scanner_context,
                                                  const Reference< css::lang::XEventListener >& listener )
{
    bool bRet;
    bool bScan;
    {
        osl::MutexGuard aGuard( theSaneProtector::get() );
        sanevec &rSanes = theSanes::get().m_aSanes;

        SAL_INFO("extensions.scanner", "ScannerManager::configureScanner");

        if( scanner_context.InternalData < 0 || o3tl::make_unsigned(scanner_context.InternalData) >= rSanes.size() )
            throw ScannerException(
                u"Scanner does not exist"_ustr,
                Reference< XScannerManager >( this ),
                ScanError_InvalidContext
            );

        std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
        if( pHolder->m_bBusy )
            throw ScannerException(
                u"Scanner is busy"_ustr,
                Reference< XScannerManager >( this ),
                ScanError_ScanInProgress
            );

        pHolder->m_bBusy = true;
        SaneDlg aDlg(Application::GetFrameWeld(mxDialogParent), pHolder->m_aSane, listener.is());
        bRet = aDlg.run();
        bScan = aDlg.getDoScan();
        pHolder->m_bBusy = false;
    }
    if ( bScan )
        startScan( scanner_context, listener );

    return bRet;
}


void ScannerManager::startScan( const ScannerContext& scanner_context,
                                const Reference< css::lang::XEventListener >& listener )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    SAL_INFO("extensions.scanner", "ScannerManager::startScan");

    if( scanner_context.InternalData < 0 || o3tl::make_unsigned(scanner_context.InternalData) >= rSanes.size() )
        throw ScannerException(
            u"Scanner does not exist"_ustr,
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );
    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
    if( pHolder->m_bBusy )
        throw ScannerException(
            u"Scanner is busy"_ustr,
            Reference< XScannerManager >( this ),
            ScanError_ScanInProgress
            );
    pHolder->m_bBusy = true;

    ScannerThread* pThread = new ScannerThread( pHolder, listener, this );
    pThread->create();
}


ScanError ScannerManager::getError( const ScannerContext& scanner_context )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( scanner_context.InternalData < 0 || o3tl::make_unsigned(scanner_context.InternalData) >= rSanes.size() )
        throw ScannerException(
            u"Scanner does not exist"_ustr,
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );

    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

    return pHolder->m_nError;
}


Reference< css::awt::XBitmap > ScannerManager::getBitmap( const ScannerContext& scanner_context )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( scanner_context.InternalData < 0 || o3tl::make_unsigned(scanner_context.InternalData) >= rSanes.size() )
        throw ScannerException(
            u"Scanner does not exist"_ustr,
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );
    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

    osl::MutexGuard aProtGuard( pHolder->m_aProtector );

    Reference< css::awt::XBitmap > xRet( pHolder->m_xBitmap );
    pHolder->m_xBitmap.clear();

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
