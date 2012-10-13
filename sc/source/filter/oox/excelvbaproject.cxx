/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "excelvbaproject.hxx"

#include <list>
#include <set>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

ExcelVbaProject::ExcelVbaProject( const Reference< XComponentContext >& rxContext, const Reference< XSpreadsheetDocument >& rxDocument ) :
    ::oox::ole::VbaProject( rxContext, Reference< XModel >( rxDocument, UNO_QUERY ), "Calc" ),
    mxDocument( rxDocument )
{
}

// protected ------------------------------------------------------------------

namespace {

struct SheetCodeNameInfo
{
    PropertySet         maSheetProps;       /// Property set of the sheet without codename.
    OUString            maPrefix;           /// Prefix for the codename to be generated.

    inline explicit     SheetCodeNameInfo( PropertySet& rSheetProps, const OUString& rPrefix ) :
                            maSheetProps( rSheetProps ), maPrefix( rPrefix ) {}
};

typedef ::std::set< OUString >              CodeNameSet;
typedef ::std::list< SheetCodeNameInfo >    SheetCodeNameInfoList;

} // namespace

void ExcelVbaProject::prepareImport()
{
    /*  Check if the sheets have imported codenames. Generate new unused
        codenames if not. */
    if( mxDocument.is() ) try
    {
        // collect existing codenames (do not use them when creating new codenames)
        CodeNameSet aUsedCodeNames;

        // collect sheets without codenames
        SheetCodeNameInfoList aCodeNameInfos;

        // iterate over all imported sheets
        Reference< XEnumerationAccess > xSheetsEA( mxDocument->getSheets(), UNO_QUERY_THROW );
        Reference< XEnumeration > xSheetsEnum( xSheetsEA->createEnumeration(), UNO_SET_THROW );
        // own try/catch for every sheet
        while( xSheetsEnum->hasMoreElements() ) try
        {
            PropertySet aSheetProp( xSheetsEnum->nextElement() );
            OUString aCodeName;
            aSheetProp.getProperty( aCodeName, PROP_CodeName );
            if( !aCodeName.isEmpty() )
            {
                aUsedCodeNames.insert( aCodeName );
            }
            else
            {
                // TODO: once we have chart sheets we need a switch/case on sheet type ('SheetNNN' vs. 'ChartNNN')
                aCodeNameInfos.push_back( SheetCodeNameInfo( aSheetProp, "Sheet" ) );
            }
        }
        catch( Exception& )
        {
        }

        // create new codenames if sheets do not have one
        for( SheetCodeNameInfoList::iterator aIt = aCodeNameInfos.begin(), aEnd = aCodeNameInfos.end(); aIt != aEnd; ++aIt )
        {
            // search for an unused codename
            sal_Int32 nCounter = 1;
            OUString aCodeName;
            do
            {
                aCodeName = OUStringBuffer( aIt->maPrefix ).append( nCounter++ ).makeStringAndClear();
            }
            while( aUsedCodeNames.count( aCodeName ) > 0 );
            aUsedCodeNames.insert( aCodeName );

            // set codename at sheet
            aIt->maSheetProps.setProperty( PROP_CodeName, aCodeName );

            // tell base class to create a dummy module
            addDummyModule( aCodeName, ModuleType::DOCUMENT );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
