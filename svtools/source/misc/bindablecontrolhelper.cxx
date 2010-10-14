/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imageresourceaccess.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_svtools.hxx"
#include "bindablecontrolhelper.hxx"
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

//........................................................................
namespace svt
{
//........................................................................

#ifndef C2U
#define C2U(cChar)  rtl::OUString::createFromAscii(cChar)
#endif

    using namespace ::com::sun::star;

bool lcl_isNamedRange( const rtl::OUString& sAddress, const uno::Reference< frame::XModel >& xModel, table::CellRangeAddress& aAddress )
{
    bool bRes = false;
    const static rtl::OUString sNamedRanges( RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XCellRangeReferrer > xReferrer;
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet( xModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xNamed( xPropSet->getPropertyValue( sNamedRanges ), uno::UNO_QUERY_THROW );
        xReferrer.set ( xNamed->getByName( sAddress ), uno::UNO_QUERY );
    }
    catch( uno::Exception& /*e*/ )
    {
        // do nothing
    }
    if ( xReferrer.is() )
    {
        uno::Reference< sheet::XCellRangeAddressable > xRangeAddressable( xReferrer->getReferredCells(), uno::UNO_QUERY );
        if ( xRangeAddressable.is() )
        {
            aAddress = xRangeAddressable->getRangeAddress();
            bRes = true;
        }
    }
    return bRes;
}


void
BindableControlHelper::ApplyListSourceAndBindableData( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& xModel, const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& rObj, const rtl::OUString& rsCtrlSource, const rtl::OUString& rsRowSource )
{
// XBindable etc.
    uno::Reference< lang::XMultiServiceFactory > xFac;
    if ( xModel.is() )
        xFac.set( xModel, uno::UNO_QUERY );
    uno::Reference< form::binding::XBindableValue > xBindable( rObj, uno::UNO_QUERY );
    if (  xFac.is() && rsCtrlSource.getLength() && xBindable.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellAddressConversion" )), uno::UNO_QUERY );
         table::CellAddress aAddress;
         if ( xConvertor.is() )
         {
             // we need this service to properly convert XL notation also
             // Should be easy to extend
             xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsCtrlSource ) );
             xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("BoundCell");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XValueBinding > xBinding( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellValueBinding" ), aArgs ), uno::UNO_QUERY );
         xBindable->setValueBinding( xBinding );
    }
    else if ( xBindable.is() ) // reset it
        xBindable->setValueBinding( uno::Reference< form::binding::XValueBinding >() );
    uno::Reference< form::binding::XListEntrySink > xListEntrySink( rObj, uno::UNO_QUERY );
    if (  xFac.is() && rsRowSource.getLength() && xListEntrySink.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellRangeAddressConversion" )), uno::UNO_QUERY );
         table::CellRangeAddress aAddress;
         if ( xConvertor.is() )
         {
             if ( !lcl_isNamedRange( rsRowSource, xModel, aAddress ) )
             {
                 // we need this service to properly convert XL notation also
                 // Should be easy to extend
                 xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsRowSource ) );
                 xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
             }
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("CellRange");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XListEntrySource > xSource( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellRangeListSource" ), aArgs ), uno::UNO_QUERY );
         xListEntrySink->setListEntrySource( xSource );
    }
    else if (  xListEntrySink.is() ) // reset
         xListEntrySink->setListEntrySource( uno::Reference< form::binding::XListEntrySource >()  );

}

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
