/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impastp1.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:42:25 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif

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

