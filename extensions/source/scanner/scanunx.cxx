/*************************************************************************
 *
 *  $RCSfile: scanunx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <scanner.hxx>
#include <sanedlg.hxx>
#include <vos/thread.hxx>
#include <tools/list.hxx>

// -----------------------------------------------------------------------------

ANY SAL_CALL BitmapTransporter::queryInterface( const Type& rType ) throw( RuntimeException )
{
    const ANY aRet( cppu::queryInterface( rType, static_cast< AWT::XBitmap* >( this ) ) );

    return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}

// -----------------------------------------------------------------------------

AWT::Size BitmapTransporter::getSize()
{
    vos::OGuard aGuard( m_aProtector );
    int         nPreviousPos = m_aStream.Tell();
    AWT::Size   aRet;

    // ensure that there is at least a header
    m_aStream.Seek( STREAM_SEEK_TO_END );
    int nLen = m_aStream.Tell();
    if( nLen > 15 )
    {
        m_aStream.Seek( 4 );
        m_aStream >> aRet.Width >> aRet.Height;
    }
    else
        aRet.Width = aRet.Height = 0;

    m_aStream.Seek( nPreviousPos );

    return aRet;
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) BitmapTransporter::getDIB()
{
    vos::OGuard aGuard( m_aProtector );
    int         nPreviousPos = m_aStream.Tell();

    // create return value
    m_aStream.Seek( STREAM_SEEK_TO_END );
    int nBytes = m_aStream.Tell();
    m_aStream.Seek( 0 );

    SEQ( sal_Int8 ) aValue( nBytes );
    m_aStream.Read( aValue.getArray(), nBytes );
    m_aStream.Seek( nPreviousPos );

    return aValue;
}

// --------------
// - SaneHolder -
// --------------

struct SaneHolder
{
    Sane                m_aSane;
    REF( AWT::XBitmap ) m_xBitmap;
    vos::OMutex         m_aProtector;
    ScanError           m_nError;
};

DECLARE_LIST( SaneHolderList, SaneHolder* );

static SaneHolderList   allSanes;
static vos::OMutex      aSaneProtector;

// -----------------
// - ScannerThread -
// -----------------

class ScannerThread : public vos::OThread
{
    SaneHolder*                                 m_pHolder;
    REF( com::sun::star::lang::XEventListener ) m_xListener;
    ScannerManager*                             m_pManager; // just for the disposing call

    virtual void run();
    virtual void onTerminated() { delete this; }
public:
    ScannerThread( SaneHolder* pHolder,
                   const REF( com::sun::star::lang::XEventListener )& listener,
                   ScannerManager* pManager )
            : m_pHolder( pHolder ), m_xListener( listener ), m_pManager( pManager ) {}
    virtual ~ScannerThread() {}
};

// -----------------------------------------------------------------------------

void ScannerThread::run()
{
    vos::OGuard         aGuard( m_pHolder->m_aProtector );
    REF( XInterface )   aIf( static_cast< OWeakObject* >( new BitmapTransporter ) );

    m_pHolder->m_xBitmap = REF( AWT::XBitmap )( aIf, UNO_QUERY );

    if( m_pHolder->m_aSane.IsOpen() )
    {
        int nOption = m_pHolder->m_aSane.GetOptionByName( "preview" );
        if( nOption != -1 )
            m_pHolder->m_aSane.SetOptionValue( nOption, (BOOL)FALSE );

        m_pHolder->m_nError =
            m_pHolder->m_aSane.Start( *(BitmapTransporter*)&m_pHolder->m_xBitmap ) ?
            ScanError_ScanErrorNone : ScanError_ScanCanceled;
    }
    else
        m_pHolder->m_nError = ScanError_ScannerNotAvailable;

    m_xListener->disposing( com::sun::star::lang::EventObject( REF( XInterface )( static_cast< OWeakObject* >( m_pManager ) ) ) );
}

// ------------------
// - ScannerManager -
// ------------------

void ScannerManager::DestroyData()
{
    // unused
}

// -----------------------------------------------------------------------------

AWT::Size ScannerManager::getSize()
{
    AWT::Size aRet;
    aRet.Width = aRet.Height = 0;
    return aRet;
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) ScannerManager::getDIB()
{
    return SEQ( sal_Int8 )();
}

// -----------------------------------------------------------------------------

SEQ( ScannerContext ) ScannerManager::getAvailableScanners()
{
    vos::OGuard aGuard( aSaneProtector );

    if( ! allSanes.Count() )
    {
        SaneHolder* pSaneHolder = new SaneHolder;
        pSaneHolder->m_nError = ScanError_ScanErrorNone;
        if( Sane::IsSane() )
            allSanes.Insert( pSaneHolder );
        else
            delete pSaneHolder;
    }

    if( Sane::IsSane() )
    {
        SEQ( ScannerContext ) aRet(1);
        aRet.getArray()[0].ScannerName      = ::rtl::OUString::createFromAscii( "SANE" );
        aRet.getArray()[0].InternalData     = 0;
        return aRet;
    }

    return SEQ( ScannerContext )();
}

// -----------------------------------------------------------------------------

BOOL ScannerManager::configureScanner( ScannerContext& scanner_context ) throw( ScannerException )
{
    vos::OGuard aGuard( aSaneProtector );

    if( scanner_context.InternalData < 0 || scanner_context.InternalData >= allSanes.Count() )
        throw ScannerException(
            ::rtl::OUString::createFromAscii( "Scanner does not exist" ),
            REF( XScannerManager )( this ),
            ScanError_InvalidContext
            );
    SaneDlg aDlg( NULL, allSanes.GetObject( scanner_context.InternalData )->m_aSane );
    return (BOOL)aDlg.Execute();
}

// -----------------------------------------------------------------------------

void ScannerManager::startScan( const ScannerContext& scanner_context,
                                const REF( com::sun::star::lang::XEventListener )& listener ) throw( ScannerException )
{
    vos::OGuard aGuard( aSaneProtector );

    if( scanner_context.InternalData < 0 || scanner_context.InternalData >= allSanes.Count() )
        throw ScannerException(
            ::rtl::OUString::createFromAscii( "Scanner does not exist" ),
            REF( XScannerManager )( this ),
            ScanError_InvalidContext
            );
    SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );

    ScannerThread* pThread = new ScannerThread( pHolder, listener, this );
    pThread->create();
}

// -----------------------------------------------------------------------------

ScanError ScannerManager::getError( const ScannerContext& scanner_context ) throw( ScannerException )
{
    vos::OGuard aGuard( aSaneProtector );

    if( scanner_context.InternalData < 0 || scanner_context.InternalData >= allSanes.Count() )
        throw ScannerException(
            ::rtl::OUString::createFromAscii( "Scanner does not exist" ),
            REF( XScannerManager )( this ),
            ScanError_InvalidContext
            );

    SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );

    return pHolder->m_nError;
}

// -----------------------------------------------------------------------------

REF( AWT::XBitmap ) ScannerManager::getBitmap( const ScannerContext& scanner_context ) throw( ScannerException )
{
    vos::OGuard aGuard( aSaneProtector );

    if( scanner_context.InternalData < 0 || scanner_context.InternalData >= allSanes.Count() )
        throw ScannerException(
            ::rtl::OUString::createFromAscii( "Scanner does not exist" ),
            REF( XScannerManager )( this ),
            ScanError_InvalidContext
            );
    SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );

    vos::OGuard aProtGuard( pHolder->m_aProtector );

    REF( AWT::XBitmap ) xRet( pHolder->m_xBitmap );
    pHolder->m_xBitmap = REF( AWT::XBitmap )();

    return xRet;
}
