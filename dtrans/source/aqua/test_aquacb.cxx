/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: test_aquacb.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#include "aqua_clipboard.hxx"
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

Reference< XTransferable > rXTransfRead;

class TestTransferable : public WeakImplHelper2< XClipboardOwner, XTransferable >
{
public:
    TestTransferable();
    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors() throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);
    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);

private:
    Sequence< DataFlavor > m_seqDFlv;
    OUString               m_Data;
};

TestTransferable::TestTransferable() :
    m_seqDFlv( 1 ),
    m_Data( RTL_CONSTASCII_USTRINGPARAM( "This is a test string" ) )
{
    DataFlavor df;

    df.MimeType = OUString::createFromAscii( "text/html" );
    df.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );

    m_seqDFlv[0] = df;
}

Any SAL_CALL TestTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    if ( aFlavor.MimeType == m_seqDFlv[0].MimeType )
    {
        OString aStr( m_Data.getStr(), m_Data.getLength(), 1252 );
        Sequence< sal_Int8 > sOfChars( aStr.getLength() );
        sal_Int32 lenStr = aStr.getLength();

        for ( sal_Int32 i = 0; i < lenStr; ++i )
            sOfChars[i] = aStr[i];

        anyData = makeAny( sOfChars );
    }

    return anyData;
}

Sequence< DataFlavor > SAL_CALL TestTransferable::getTransferDataFlavors()
    throw(RuntimeException)
{
    return m_seqDFlv;
}

sal_Bool SAL_CALL TestTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    sal_Int32 nLength = m_seqDFlv.getLength();
    sal_Bool bRet     = sal_False;

    for ( sal_Int32 i = 0; i < nLength; ++i )
    {
        if ( m_seqDFlv[i].MimeType == aFlavor.MimeType )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

void SAL_CALL TestTransferable::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
}

int SAL_CALL main( int argc, char** argv )
{
    if(argc != 2)
    {
        fprintf( stderr, "usage: %s <my rdb file>\n", argv[0] );
        return 1;
    }

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------
    OUString rdbName = OUString::createFromAscii( argv[1] );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occured.
    if ( !g_xFactory.is() )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    Reference< XTransferable > rXTransf( static_cast< XTransferable* >( new TestTransferable ) );

    Reference< XClipboard > xClipboard( g_xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( AQUA_CLIPBOARD_SERVICE_NAME ) ) ), UNO_QUERY );
    if ( !xClipboard.is() )
    {
        OSL_ENSURE( sal_False, "Error creating FolderPicker Service" );
        return(-1);
    }

    Reference< XTypeProvider > rXTypProv( xClipboard, UNO_QUERY );

    if ( rXTypProv.is() )
    {
        Sequence< Type >     seqType = rXTypProv->getTypes();
        sal_Int32 nLen = seqType.getLength();
        for ( sal_Int32 i = 0; i < nLen; i++ )
        {
            Type nxtType = seqType[i];
        }

        Sequence< sal_Int8 > seqInt8 = rXTypProv->getImplementationId();
    }

    xClipboard->setContents( rXTransf, Reference< XClipboardOwner >( rXTransf, UNO_QUERY )  );

    rXTransfRead = xClipboard->getContents();

    // destroy the transferable explicitly
    rXTransfRead = Reference< XTransferable>();

    // destroy the clipboard
    xClipboard = Reference< XClipboard >();

    //--------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    if ( !xComponent.is() )
        OSL_ENSURE(sal_False, "Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    return 0;
}
