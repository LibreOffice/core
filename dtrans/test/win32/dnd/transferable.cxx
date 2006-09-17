/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transferable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:05:08 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"
#include "transferable.hxx"

//----------------------------------------------------------------
//  ctor
//----------------------------------------------------------------



CTransferable::CTransferable( wchar_t* dataString ) :
    m_seqDFlv( 1 ),
    m_Data( dataString )
{
    DataFlavor df;

    /*
    df.MimeType = L"text/plain; charset=unicode";
    df.DataType = getCppuType( ( OUString* )0 );

    m_seqDFlv[0] = df;
    */

    //df.MimeType = L"text/plain; charset=windows1252";
    df.MimeType = L"text/plain";
    df.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );


    m_seqDFlv[0] = df;
}

//----------------------------------------------------------------
//  getTransferData
//----------------------------------------------------------------

Any SAL_CALL CTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    /*if ( aFlavor == m_seqDFlv[0] )
    {
        anyData = makeAny( m_Data );
    }
    else*/ if ( aFlavor == m_seqDFlv[0] )
    {
        OString aStr( m_Data.getStr( ), m_Data.getLength( ), 1252 );
        Sequence< sal_Int8 > sOfChars( aStr.getLength( ) );
        sal_Int32 lenStr = aStr.getLength( );

        for ( sal_Int32 i = 0; i < lenStr; ++i )
            sOfChars[i] = aStr[i];

        anyData = makeAny( sOfChars );
    }

    return anyData;
}

//----------------------------------------------------------------
//  getTransferDataFlavors
//----------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL CTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_seqDFlv;
}

//----------------------------------------------------------------
//  isDataFlavorSupported
//----------------------------------------------------------------

sal_Bool SAL_CALL CTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    sal_Int32 nLength = m_seqDFlv.getLength( );
    sal_Bool bRet     = sal_False;

    for ( sal_Int32 i = 0; i < nLength; ++i )
    {
        if ( m_seqDFlv[i] == aFlavor )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

//----------------------------------------------------------------
//  lostOwnership
//----------------------------------------------------------------

void SAL_CALL CTransferable::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
}
