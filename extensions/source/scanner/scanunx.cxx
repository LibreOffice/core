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

#include <scanner.hxx>
#include <sanedlg.hxx>
#include <osl/thread.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <memory>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

BitmapTransporter::BitmapTransporter()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "BitmapTransporter\n" );
#endif
}

BitmapTransporter::~BitmapTransporter()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "~BitmapTransporter\n" );
#endif
}



Any SAL_CALL BitmapTransporter::queryInterface( const Type& rType ) throw( RuntimeException, std::exception )
{
    const Any aRet( cppu::queryInterface( rType, static_cast< css::awt::XBitmap* >( this ) ) );

    return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}



css::awt::Size BitmapTransporter::getSize() throw(std::exception)
{
    osl::MutexGuard aGuard( m_aProtector );
    int         nPreviousPos = m_aStream.Tell();
    css::awt::Size   aRet;

    // ensure that there is at least a header
    m_aStream.Seek( STREAM_SEEK_TO_END );
    int nLen = m_aStream.Tell();
    if( nLen > 15 )
    {
        m_aStream.Seek( 4 );
        m_aStream.ReadInt32( aRet.Width ).ReadInt32( aRet.Height );
    }
    else
        aRet.Width = aRet.Height = 0;

    m_aStream.Seek( nPreviousPos );

    return aRet;
}



Sequence< sal_Int8 > BitmapTransporter::getDIB() throw(std::exception)
{
    osl::MutexGuard aGuard( m_aProtector );
    int         nPreviousPos = m_aStream.Tell();

    // create return value
    m_aStream.Seek( STREAM_SEEK_TO_END );
    int nBytes = m_aStream.Tell();
    m_aStream.Seek( 0 );

    Sequence< sal_Int8 > aValue( nBytes );
    m_aStream.Read( aValue.getArray(), nBytes );
    m_aStream.Seek( nPreviousPos );

    return aValue;
}


// - SaneHolder -


struct SaneHolder
{
    Sane                m_aSane;
    Reference< css::awt::XBitmap > m_xBitmap;
    osl::Mutex          m_aProtector;
    ScanError           m_nError;
    bool                m_bBusy;

    SaneHolder() : m_nError(ScanError_ScanErrorNone), m_bBusy(false) {}
};

namespace
{
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
}


// - ScannerThread -


class ScannerThread : public osl::Thread
{
    std::shared_ptr<SaneHolder>               m_pHolder;
    Reference< com::sun::star::lang::XEventListener > m_xListener;
    ScannerManager*                             m_pManager; // just for the disposing call

public:
    virtual void run() SAL_OVERRIDE;
    virtual void onTerminated() SAL_OVERRIDE { delete this; }
public:
    ScannerThread( std::shared_ptr<SaneHolder> pHolder,
                   const Reference< com::sun::star::lang::XEventListener >& listener,
                   ScannerManager* pManager );
    virtual ~ScannerThread();
};



ScannerThread::ScannerThread(
                             std::shared_ptr<SaneHolder> pHolder,
                             const Reference< com::sun::star::lang::XEventListener >& listener,
                             ScannerManager* pManager )
        : m_pHolder( pHolder ), m_xListener( listener ), m_pManager( pManager )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerThread\n" );
#endif
}

ScannerThread::~ScannerThread()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "~ScannerThread\n" );
#endif
}

void ScannerThread::run()
{
    osl_setThreadName("ScannerThread");

    osl::MutexGuard         aGuard( m_pHolder->m_aProtector );
    BitmapTransporter*  pTransporter = new BitmapTransporter;
    Reference< XInterface >   aIf( static_cast< OWeakObject* >( pTransporter ) );

    m_pHolder->m_xBitmap = Reference< css::awt::XBitmap >( aIf, UNO_QUERY );

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
    m_xListener->disposing( com::sun::star::lang::EventObject(xXInterface) );
    m_pHolder->m_bBusy = false;
}


// - ScannerManager -


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



css::awt::Size ScannerManager::getSize() throw(std::exception)
{
    css::awt::Size aRet;
    aRet.Width = aRet.Height = 0;
    return aRet;
}



Sequence< sal_Int8 > ScannerManager::getDIB() throw(std::exception)
{
    return Sequence< sal_Int8 >();
}



Sequence< ScannerContext > ScannerManager::getAvailableScanners() throw(std::exception)
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( rSanes.empty() )
    {
        std::shared_ptr<SaneHolder> pSaneHolder(new SaneHolder);
        if( Sane::IsSane() )
            rSanes.push_back( pSaneHolder );
    }

    if( Sane::IsSane() )
    {
        Sequence< ScannerContext > aRet(1);
        aRet[0].ScannerName      = "SANE";
        aRet[0].InternalData     = 0;
        return aRet;
    }

    return Sequence< ScannerContext >();
}



sal_Bool ScannerManager::configureScannerAndScan( ScannerContext& scanner_context,
                                                  const Reference< com::sun::star::lang::XEventListener >& listener )
    throw (ScannerException, RuntimeException, std::exception)
{
    bool bRet;
    bool bScan;
    {
        osl::MutexGuard aGuard( theSaneProtector::get() );
        sanevec &rSanes = theSanes::get().m_aSanes;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "ScannerManager::configureScanner\n" );
#endif

        if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
            throw ScannerException(
                "Scanner does not exist",
                Reference< XScannerManager >( this ),
                ScanError_InvalidContext
            );

        std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
        if( pHolder->m_bBusy )
            throw ScannerException(
                "Scanner is busy",
                Reference< XScannerManager >( this ),
                ScanError_ScanInProgress
            );

        pHolder->m_bBusy = true;
        ScopedVclPtrInstance< SaneDlg > aDlg(nullptr, pHolder->m_aSane, listener.is());
        bRet = aDlg->Execute();
        bScan = aDlg->getDoScan();
        pHolder->m_bBusy = false;
    }
    if ( bScan )
        startScan( scanner_context, listener );

    return bRet;
}



void ScannerManager::startScan( const ScannerContext& scanner_context,
                                const Reference< com::sun::star::lang::XEventListener >& listener ) throw( ScannerException, std::exception )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerManager::startScan\n" );
#endif

    if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
        throw ScannerException(
            "Scanner does not exist",
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );
    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
    if( pHolder->m_bBusy )
        throw ScannerException(
            "Scanner is busy",
            Reference< XScannerManager >( this ),
            ScanError_ScanInProgress
            );
    pHolder->m_bBusy = true;

    ScannerThread* pThread = new ScannerThread( pHolder, listener, this );
    pThread->create();
}



ScanError ScannerManager::getError( const ScannerContext& scanner_context ) throw( ScannerException, std::exception )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
        throw ScannerException(
            "Scanner does not exist",
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );

    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

    return pHolder->m_nError;
}



Reference< css::awt::XBitmap > ScannerManager::getBitmap( const ScannerContext& scanner_context ) throw( ScannerException, std::exception )
{
    osl::MutexGuard aGuard( theSaneProtector::get() );
    sanevec &rSanes = theSanes::get().m_aSanes;

    if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
        throw ScannerException(
            "Scanner does not exist",
            Reference< XScannerManager >( this ),
            ScanError_InvalidContext
            );
    std::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

    osl::MutexGuard aProtGuard( pHolder->m_aProtector );

    Reference< css::awt::XBitmap > xRet( pHolder->m_xBitmap );
    pHolder->m_xBitmap = Reference< css::awt::XBitmap >();

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
