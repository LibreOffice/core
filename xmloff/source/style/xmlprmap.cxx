/*************************************************************************
 *
 *  $RCSfile: xmlprmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-01 13:15:04 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include "xmlprhdl.hxx"
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#include "xmlbahdl.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
    const XMLPropertyMapEntry& rMapEntry,
    const UniReference< XMLPropertyHandlerFactory >& rFactory ) :
    sXMLAttributeName( OUString::createFromAscii(rMapEntry.msXMLName) ),
    sAPIPropertyName( OUString::createFromAscii(rMapEntry.msApiName) ),
    nXMLNameSpace( rMapEntry.mnNameSpace ),
    nType( rMapEntry.mnType ),
    nContextId( rMapEntry.mnContextId ),
    pHdl( rFactory->GetPropertyHandler( rMapEntry.mnType & MID_FLAG_MASK ) )
{
}

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
        const XMLPropertySetMapperEntry_Impl& rEntry ) :
    sXMLAttributeName( rEntry.sXMLAttributeName),
    sAPIPropertyName( rEntry.sAPIPropertyName),
    nXMLNameSpace( rEntry.nXMLNameSpace),
    nType( rEntry.nType),
    nContextId( rEntry.nContextId),
    pHdl( rEntry.pHdl)
{
    DBG_ASSERT( pHdl, "Unknown XML property type handler!" );
}

///////////////////////////////////////////////////////////////////////////
//
// Ctor
//
XMLPropertySetMapper::XMLPropertySetMapper(
        const XMLPropertyMapEntry* pEntries,
        const UniReference< XMLPropertyHandlerFactory >& rFactory )
{
    aHdlFactories.push_back( rFactory );
    if( pEntries )
    {
        const XMLPropertyMapEntry* pIter = pEntries;

        // count entries
        while( pIter->msApiName )
        {
            XMLPropertySetMapperEntry_Impl aEntry( *pIter, rFactory );
            aMapEntries.push_back( aEntry );
            pIter++;
        }
    }
}

XMLPropertySetMapper::~XMLPropertySetMapper()
{
}

void XMLPropertySetMapper::AddMapperEntry(
    const UniReference < XMLPropertySetMapper >& rMapper )
{
    for( vector < UniReference < XMLPropertyHandlerFactory > >::iterator
            aFIter = rMapper->aHdlFactories.begin();
         aFIter != rMapper->aHdlFactories.end();
         aFIter++ )
    {
        aHdlFactories.push_back( *aFIter );
    }

    for( vector < XMLPropertySetMapperEntry_Impl >::iterator
            aEIter = rMapper->aMapEntries.begin();
         aEIter != rMapper->aMapEntries.end();
         aEIter++ )
    {
        aMapEntries.push_back( *aEIter );
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Export a Property
//
sal_Bool XMLPropertySetMapper::exportXML(
        OUString& rStrExpValue,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    const XMLPropertyHandler* pHdl = GetPropertyHandler( rProperty.mnIndex );

    DBG_ASSERT( pHdl, "Unknown XML Type!" );
    if( pHdl )
        bRet = pHdl->exportXML( rStrExpValue, rProperty.maValue,
                                rUnitConverter );

    return bRet;
}

///////////////////////////////////////////////////////////////////////////
//
// Import a Property
//
sal_Bool XMLPropertySetMapper::importXML(
        const OUString& rStrImpValue,
        XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    const XMLPropertyHandler* pHdl = GetPropertyHandler( rProperty.mnIndex );

    if( pHdl )
        bRet = pHdl->importXML( rStrImpValue, rProperty.maValue,
                                rUnitConverter );

    return bRet;
}

///////////////////////////////////////////////////////////////////////////
//
// Search for the given name and the namespace in the list and return
// the index of the entry
// If there is no matching entry the method returns -1
//
const sal_Int32 XMLPropertySetMapper::GetEntryIndex(
        sal_uInt16 nNamespace,
        const OUString& rStrName,
        sal_Int32 nStartAt /* = -1 */ ) const
{
    sal_Int32 nEntries = GetEntryCount();
    sal_Int32 nIndex= nStartAt == - 1? 0 : nStartAt+1;

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nXMLNameSpace == nNamespace &&
            rStrName == rEntry.sXMLAttributeName )
            return nIndex;
        else
            nIndex++;

    } while( nIndex<nEntries );

    return -1;
}


/** searches for an entry that matches the given api name, namespace and local name or -1 if nothing found */
sal_Int32 XMLPropertySetMapper::FindEntryIndex(
        const sal_Char* sApiName,
        sal_uInt16 nNameSpace,
        const sal_Char* sXMLName ) const
{
    sal_Int32 nIndex = 0;
    sal_Int32 nEntries = GetEntryCount();

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nXMLNameSpace == nNameSpace &&
            0 == rEntry.sXMLAttributeName.compareToAscii( sXMLName ) &&
            0 == rEntry.sAPIPropertyName.compareToAscii( sApiName ) )
            return nIndex;
        else
            nIndex++;

    } while( nIndex < nEntries );

    return -1;
}

sal_Int32 XMLPropertySetMapper::FindEntryIndex( const sal_Int16 nContextId ) const
{
    sal_Int32 nIndex = 0;
    sal_Int32 nEntries = GetEntryCount();

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nContextId == nContextId )
            return nIndex;
        else
            nIndex++;

    } while( nIndex < nEntries );

    return -1;
}
