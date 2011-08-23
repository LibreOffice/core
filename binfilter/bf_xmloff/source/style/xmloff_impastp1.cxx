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

#include <tools/debug.hxx>
#include "impastpl.hxx"
namespace binfilter {

using namespace rtl;

//#############################################################################
//
// Class XMLFamilyData_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor class XMLFamilyData_Impl
//

XMLFamilyData_Impl::XMLFamilyData_Impl(
        sal_Int32 nFamily,
        const ::rtl::OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > &rMapper,
        const OUString& rStrPrefix,
        sal_Bool bAsFam )
    : mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ), maStrPrefix( rStrPrefix ),
      mnCount( 0 ), mnName( 0 ), bAsFamily( bAsFam ),
      pCache( 0 )
{
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl( 5, 5 );
    mpNameList   = new SvXMLAutoStylePoolNamesP_Impl( 5, 5 );
}

XMLFamilyData_Impl::~XMLFamilyData_Impl()
{
    if( mpParentList ) delete mpParentList;
    if( mpNameList ) delete mpNameList;
    DBG_ASSERT( !pCache || !pCache->Count(),
                "auto style pool cache is not empty!" );
    if( pCache )
    {
        while( pCache->Count() )
            delete pCache->Remove( 0UL );
    }
}

void XMLFamilyData_Impl::ClearEntries()
{
    if( mpParentList )
        delete mpParentList;
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl( 5, 5 );
    DBG_ASSERT( !pCache || !pCache->Count(),
                "auto style pool cache is not empty!" );
    if( pCache )
    {
        while( pCache->Count() )
            delete pCache->Remove( 0UL );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// friendfunction of class XMLFamilyData_Impl for sorting listelements
//

int XMLFamilyDataSort_Impl( const XMLFamilyData_Impl& r1, const XMLFamilyData_Impl& r2 )
{
    int nRet = 0;

    if( r1.mnFamily != r2.mnFamily )
        nRet = ( r1.mnFamily > r2.mnFamily ? 1 : -1 );

    return nRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of sorted list of XMLFamilyData_Impl - elements
//

IMPL_CONTAINER_SORT( XMLFamilyDataList_Impl, XMLFamilyData_Impl, XMLFamilyDataSort_Impl )

//#############################################################################
//
// Sorted list of OUString - elements
//

///////////////////////////////////////////////////////////////////////////////
//
// Sort-function for OUString-list
//

int SvXMLAutoStylePoolNamesPCmp_Impl( const OUString& r1,
                                     const OUString& r2 )
{
    return (int)r1.compareTo( r2 );
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of sorted OUString-list
//

IMPL_CONTAINER_SORT( SvXMLAutoStylePoolNamesP_Impl,
                     OUString,
                     SvXMLAutoStylePoolNamesPCmp_Impl )

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
