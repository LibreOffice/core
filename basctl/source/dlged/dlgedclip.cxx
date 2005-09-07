/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedclip.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:13:37 $
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

#ifndef _BASCTL_DLGEDCLIP_HXX
#include "dlgedclip.hxx"
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif


using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;


//============================================================================
// DlgEdTransferableImpl
//============================================================================

//----------------------------------------------------------------------------

DlgEdTransferableImpl::DlgEdTransferableImpl()
    :m_SeqFlavors(0)
    ,m_SeqData(0)
{
}

//----------------------------------------------------------------------------

DlgEdTransferableImpl::DlgEdTransferableImpl( const Sequence< DataFlavor >& aSeqFlavors, const Sequence< Any >& aSeqData )
{
    m_SeqFlavors = aSeqFlavors;
    m_SeqData = aSeqData;
}

//----------------------------------------------------------------------------

DlgEdTransferableImpl::~DlgEdTransferableImpl()
{
}

//----------------------------------------------------------------------------

sal_Bool DlgEdTransferableImpl::compareDataFlavors( const DataFlavor& lFlavor, const DataFlavor& rFlavor )
{
    sal_Bool bRet = sal_False;

    // compare mime content types
    Reference< lang::XMultiServiceFactory >  xMSF = getProcessServiceFactory();
    Reference< datatransfer::XMimeContentTypeFactory >
        xMCntTypeFactory( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.datatransfer.MimeContentTypeFactory" ) ) ), UNO_QUERY );

    if ( xMCntTypeFactory.is( ) )
    {
        // compare full media types
        Reference< datatransfer::XMimeContentType > xLType = xMCntTypeFactory->createMimeContentType( lFlavor.MimeType );
        Reference< datatransfer::XMimeContentType > xRType = xMCntTypeFactory->createMimeContentType( rFlavor.MimeType );

        ::rtl::OUString aLFullMediaType = xLType->getFullMediaType();
        ::rtl::OUString aRFullMediaType = xRType->getFullMediaType();

        bRet = aLFullMediaType.equalsIgnoreAsciiCase( aRFullMediaType );
    }

    return bRet;
}

// XTransferable
//----------------------------------------------------------------------------

Any SAL_CALL DlgEdTransferableImpl::getTransferData( const DataFlavor& rFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( !isDataFlavorSupported( rFlavor ) )
        throw UnsupportedFlavorException();

    Any aData;

    for ( sal_Int32 i = 0; i < m_SeqFlavors.getLength(); i++ )
    {
        if ( compareDataFlavors( m_SeqFlavors[i] , rFlavor ) )
        {
            aData = m_SeqData[i];
            break;
        }
    }

    return aData;
}

//----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL DlgEdTransferableImpl::getTransferDataFlavors(  ) throw(RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return m_SeqFlavors;
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL DlgEdTransferableImpl::isDataFlavorSupported( const DataFlavor& rFlavor ) throw(RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Bool bRet = sal_False;

    for ( sal_Int32 i = 0; i < m_SeqFlavors.getLength(); i++ )
    {
        if ( compareDataFlavors( m_SeqFlavors[i] , rFlavor ) )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

// XClipboardOwner
//----------------------------------------------------------------------------

void SAL_CALL DlgEdTransferableImpl::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    m_SeqFlavors = Sequence< DataFlavor >();
    m_SeqData = Sequence< Any >();
}

