/*************************************************************************
 *
 *  $RCSfile: toxhlp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-06-29 13:12:39 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XINDEXENTRYSUPPLIER_HPP_
#include <com/sun/star/i18n/XIndexEntrySupplier.hpp>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOXHLP_HXX
#include <toxhlp.hxx>
#endif


IndexEntrySupplierWrapper::IndexEntrySupplierWrapper(
        const STAR_NMSPC::lang::Locale& rLcl,
        STAR_REFERENCE( lang::XMultiServiceFactory )& rxMSF )
    : aLcl( rLcl )
{
    try {
        STAR_REFERENCE( uno::XInterface ) xI =
            rxMSF->createInstance( ::rtl::OUString::createFromAscii(
                                "com.sun.star.i18n.IndexEntrySupplier" ) );
        if( xI.is() )
        {
            UNO_NMSPC::Any x = xI->queryInterface( ::getCppuType(
                    (const STAR_REFERENCE( i18n::XIndexEntrySupplier )*)0) );
            x >>= xIES;
        }
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "IndexEntrySupplierWrapper: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}

IndexEntrySupplierWrapper::~IndexEntrySupplierWrapper()
{
}

String IndexEntrySupplierWrapper::GetIndexChar( const String& rTxt,
                                           const String& rSortAlgorithm ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexCharacter( rTxt, aLcl, rSortAlgorithm );
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getIndexCharacter: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

String IndexEntrySupplierWrapper::GetFollowingText( BOOL bMorePages ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexFollowPageWord( bMorePages, aLcl );
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getIndexCharacter: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}


