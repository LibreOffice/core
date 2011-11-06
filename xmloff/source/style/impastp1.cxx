/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include "impastpl.hxx"

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
    : pCache( 0 ), mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ),
      mnCount( 0 ), mnName( 0 ), maStrPrefix( rStrPrefix ), bAsFamily( bAsFam )

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

