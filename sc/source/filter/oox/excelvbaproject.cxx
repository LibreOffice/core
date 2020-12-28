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

#include <excelvbaproject.hxx>

#include <vector>
#include <set>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>

namespace oox::xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

ExcelVbaProject::ExcelVbaProject( const Reference< XComponentContext >& rxContext, const Reference< XSpreadsheetDocument >& rxDocument ) :
    ::oox::ole::VbaProject( rxContext, Reference< XModel >( rxDocument, UNO_QUERY ), u"Calc" ),
    mxDocument( rxDocument )
{
}

// protected ------------------------------------------------------------------

namespace {

struct SheetCodeNameInfo
{
    PropertySet         maSheetProps;       /// Property set of the sheet without codename.
    OUString            maPrefix;           /// Prefix for the codename to be generated.

    explicit     SheetCodeNameInfo( const PropertySet& rSheetProps, const OUString& rPrefix ) :
                            maSheetProps( rSheetProps ), maPrefix( rPrefix ) {}
};

} // namespace

void ExcelVbaProject::prepareImport()
{
    /*  Check if the sheets have imported codenames. Generate new unused
        codenames if not. */
    if( !mxDocument.is() )
        return;

    try
    {
        // collect existing codenames (do not use them when creating new codenames)
        ::std::set< OUString > aUsedCodeNames;

        // collect sheets without codenames
        ::std::vector< SheetCodeNameInfo >  aCodeNameInfos;

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
                aCodeNameInfos.emplace_back( aSheetProp, "Sheet" );
            }
        }
        catch( Exception& )
        {
        }

        // create new codenames if sheets do not have one
        for (auto & codeName : aCodeNameInfos)
        {
            // search for an unused codename
            sal_Int32 nCounter = 1;
            OUString aCodeName;
            do
            {
                aCodeName = codeName.maPrefix + OUString::number( nCounter++ );
            }
            while( aUsedCodeNames.count( aCodeName ) > 0 );
            aUsedCodeNames.insert( aCodeName );

            // set codename at sheet
            codeName.maSheetProps.setProperty( PROP_CodeName, aCodeName );

            // tell base class to create a dummy module
            addDummyModule( aCodeName, ModuleType::DOCUMENT );
        }
    }
    catch( Exception& )
    {
    }
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
