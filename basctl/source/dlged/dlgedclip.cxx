/*************************************************************************
 *
 *  $RCSfile: dlgedclip.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-04-10 15:17:07 $
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

        bRet = aLFullMediaType.equalsIgnoreCase( aRFullMediaType );
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

