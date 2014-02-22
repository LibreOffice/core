/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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



using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;




ExcelVbaProject::ExcelVbaProject( const Reference< XComponentContext >& rxContext, const Reference< XSpreadsheetDocument >& rxDocument ) :
    ::oox::ole::VbaProject( rxContext, Reference< XModel >( rxDocument, UNO_QUERY ), "Calc" ),
    mxDocument( rxDocument )
{
}



namespace {

struct SheetCodeNameInfo
{
    PropertySet         maSheetProps;       
    OUString            maPrefix;           

    inline explicit     SheetCodeNameInfo( PropertySet& rSheetProps, const OUString& rPrefix ) :
                            maSheetProps( rSheetProps ), maPrefix( rPrefix ) {}
};

typedef ::std::set< OUString >              CodeNameSet;
typedef ::std::list< SheetCodeNameInfo >    SheetCodeNameInfoList;

} 

void ExcelVbaProject::prepareImport()
{
    /*  Check if the sheets have imported codenames. Generate new unused
        codenames if not. */
    if( mxDocument.is() ) try
    {
        
        CodeNameSet aUsedCodeNames;

        
        SheetCodeNameInfoList aCodeNameInfos;

        
        Reference< XEnumerationAccess > xSheetsEA( mxDocument->getSheets(), UNO_QUERY_THROW );
        Reference< XEnumeration > xSheetsEnum( xSheetsEA->createEnumeration(), UNO_SET_THROW );
        
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
                
                aCodeNameInfos.push_back( SheetCodeNameInfo( aSheetProp, "Sheet" ) );
            }
        }
        catch( Exception& )
        {
        }

        
        for( SheetCodeNameInfoList::iterator aIt = aCodeNameInfos.begin(), aEnd = aCodeNameInfos.end(); aIt != aEnd; ++aIt )
        {
            
            sal_Int32 nCounter = 1;
            OUString aCodeName;
            do
            {
                aCodeName = OUStringBuffer( aIt->maPrefix ).append( nCounter++ ).makeStringAndClear();
            }
            while( aUsedCodeNames.count( aCodeName ) > 0 );
            aUsedCodeNames.insert( aCodeName );

            
            aIt->maSheetProps.setProperty( PROP_CodeName, aCodeName );

            
            addDummyModule( aCodeName, ModuleType::DOCUMENT );
        }
    }
    catch( Exception& )
    {
    }
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
