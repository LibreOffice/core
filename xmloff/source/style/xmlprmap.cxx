/*************************************************************************
 *
 *  $RCSfile: xmlprmap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:06 $
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

///////////////////////////////////////////////////////////////////////////
//
// Ctor
//
XMLPropertySetMapper::XMLPropertySetMapper(
        const XMLPropertyMapEntry* pEntries,
        const UniReference< XMLPropertyHandlerFactory >& rFactory )
: mpEntries    ( pEntries ),
  mnEntryCount ( 0 ),
  maHdlFactory ( rFactory )
{
    if( mpEntries )
    {
        const XMLPropertyMapEntry* pIter = mpEntries;

        // count entries
        while( pIter->msApiName )   // the last element of the array contains { NULL, ??, ?? }
        {
            mnEntryCount++;
            pIter++;
        }

        // fill Sequence of XML-names and PropertyHandler
        maXMLNames.realloc( mnEntryCount );
        maApiNames.realloc( mnEntryCount );
        pIter = mpEntries;
        for( sal_Int32 i=0; i < mnEntryCount; i++ )
        {
            // create XML-name
            maXMLNames[ i ] = OUString::createFromAscii( pIter->msXMLName );
            maApiNames[ i ] = OUString::createFromAscii( pIter->msApiName );

            // create PropertyHandler for property
            const XMLPropertyHandler* pHdl =
                maHdlFactory->GetPropertyHandler(
                        pIter->mnType & MID_FLAG_MASK );

            DBG_ASSERT( pHdl, "Unknown XML property type handler!" );
            maPropertyHandler.push_back( pHdl );

            pIter++;
        }
    }
}

XMLPropertySetMapper::~XMLPropertySetMapper()
{
}

///////////////////////////////////////////////////////////////////////////
//
// Take all properties of the XPropertySet which are also found in the
// XMLPropertyMapEntry-array and which are not set to their default-value,
// if a state is available.
//
// After that I call the method 'ContextFilter'.
//
vector< XMLPropertyState > XMLPropertySetMapper::Filter(
        const Reference< XPropertySet > xPropSet ) const
{
    vector< XMLPropertyState > aPropStateArray;

    // Retrieve XPropertySetInfo and XPropertyState
    Reference< XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
    Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );

    if( mpEntries && xInfo.is() )
    {
        const XMLPropertyMapEntry* pIter = mpEntries;
        sal_Int32            nIndex=0;

        sal_Int32 nCount = 0;
        while( (pIter++)->msApiName )
            nCount++;
        pIter = mpEntries;


        if( xPropState.is() )
        {
            Sequence<OUString> aApiNames( nCount );
            Sequence<sal_uInt32> aIndexes( nCount );
            OUString *pNames = aApiNames.getArray();
            sal_uInt32 *pIndexes = aIndexes.getArray();
            nCount = 0;
            while( pIter->msApiName )   // the last element of the array contains { NULL, ??, ?? }
            {
                // Does the PropertySet contain name of mpEntries-array ?
                if( xInfo->hasPropertyByName( maApiNames[ nIndex ] ) )
                {
                    pNames[nCount] = maApiNames[ nIndex ];
                    pIndexes[nCount] = nIndex;
                    nCount++;
                }

                nIndex++;
                pIter++;
            }

            aApiNames.realloc( nCount );
            aIndexes.realloc( nCount );

            Sequence < PropertyState > aStates = xPropState->getPropertyStates( aApiNames );

            const PropertyState *pStates = aStates.getArray();
            for( sal_Int32 i = 0; i < nCount; i++ )
            {
                if(  pStates[i] == PropertyState_DIRECT_VALUE )
                {
                    // The value is stored in the PropertySet itself, add to list.
                    XMLPropertyState aNewProperty( pIndexes[i],
                            xPropSet->getPropertyValue(
                                    aApiNames[i] ) );
                    aPropStateArray.push_back( aNewProperty );
                }
            }
        }
        else
        {
            while( pIter->msApiName )   // the last element of the array contains { NULL, ??, ?? }
            {
                // Does the PropertySet contain name of mpEntries-array ?
                if( xInfo->hasPropertyByName( maApiNames[ nIndex ] ) )
                {
                    // If there isn't a XPropertyState we can't filter by its state
                    if( !xPropState.is() || xPropState->getPropertyState( maApiNames[ nIndex ] ) == PropertyState_DIRECT_VALUE )
                    {
                        // The value is stored in the PropertySet itself, add to list.
                        XMLPropertyState aNewProperty( nIndex,
                            xPropSet->getPropertyValue(
                                    maApiNames[ nIndex ] ) );
                        aPropStateArray.push_back( aNewProperty );
                    }
                }

                nIndex++;
                pIter++;
            }
        }

        // Call centext-filter
        ContextFilter( aPropStateArray, xPropSet );
    }

    return aPropStateArray;
}

void XMLPropertySetMapper::ContextFilter(
        vector< XMLPropertyState >& rProperties,
        Reference< XPropertySet > rPropSet ) const
{
    // Derived class could implement this.
}


///////////////////////////////////////////////////////////////////////////
//
// Compares two Sequences of XMLPropertyState:
//  1.Number of elements equal ?
//  2.Index of each element equal ? (So I know whether the propertynames are the same)
//  3.Value of each element equal ?
//
sal_Bool XMLPropertySetMapper::Equals(
        const vector< XMLPropertyState >& aProperties1,
        const vector< XMLPropertyState >& aProperties2 ) const
{
    sal_Bool bRet = sal_True;
    sal_Int32 nCount = aProperties1.size();

    if( nCount == aProperties2.size() )
    {
        sal_Int32 nIndex = 0;
        while( bRet && nIndex < nCount )
        {
            const XMLPropertyState& rProp1 = aProperties1[ nIndex ];
            const XMLPropertyState& rProp2 = aProperties2[ nIndex ];

            // Compare index. If equal, compare value
            if( rProp1.mnIndex == rProp2.mnIndex )
            {
                if( rProp1.mnIndex != -1 )
                {
                    // Now compare values
                    if( ( mpEntries[rProp1.mnIndex].mnType &
                          XML_TYPE_BUILDIN_CMP ) != 0 )
                        // simple type ( binary compare )
                        bRet = ( rProp1.maValue == rProp2.maValue );
                    else
                        // complex type ( ask for compare-function )
                        bRet = maPropertyHandler[ rProp1.mnIndex ]->equals(
                            rProp1.maValue, rProp2.maValue );
                }
            }
            else
                bRet = sal_False;

            nIndex++;
        }
    }
    else
        bRet = sal_False;

    return bRet;
}

///////////////////////////////////////////////////////////////////////////
//
// Retrieves a PropertyHandler for that property wich placed at nIndex in
// the XMLPropertyMapEntry-array
//
const XMLPropertyHandler* XMLPropertySetMapper::GetPropertyHandler(
        sal_Int32 nIndex ) const
{
    nIndex &= MID_FLAG_MASK;

    const XMLPropertyHandler* pRet = NULL;

    if( maPropertyHandler.size() > nIndex )
        pRet = (const XMLPropertyHandler*)maPropertyHandler[ nIndex ];

    return pRet;
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
    sal_Int32 nEntries = maXMLNames.getLength();
    sal_Int32 nIndex= nStartAt == - 1? 0 : nStartAt+1;
    BOOL bFound = FALSE;

    do
    {
        if( mpEntries[nIndex].mnNameSpace == nNamespace &&
            rStrName.equals( maXMLNames[nIndex] ))
            bFound = TRUE;
        else
            nIndex++;

    } while( nIndex<nEntries && !bFound );

    if( !bFound )
        nIndex = -1;

    return nIndex;
}

sal_Bool XMLPropertySetMapper::FillPropertySet(
            const vector< XMLPropertyState >& aProperties,
            const Reference<
                    XPropertySet > rPropSet ) const
{
    sal_Bool bSet = sal_False;
    Reference< XPropertySetInfo > xInfo = rPropSet->getPropertySetInfo();

    sal_Int32 nCount = aProperties.size();
    for( sal_Int32 i=0; i < nCount; i++ )
    {
        const XMLPropertyState& rProp = aProperties[i];
        sal_Int32 nIdx = rProp.mnIndex;
        if( -1 == nIdx )
            continue;
        const OUString& rPropName = maApiNames[nIdx];
        if( xInfo->hasPropertyByName( rPropName ) )
        {
            try
            {
                rPropSet->setPropertyValue( rPropName, rProp.maValue );
                bSet = sal_True;
            }
            catch(...)
            {
            }
        }
    }

    return bSet;
}

/** searches for an entry that matches the given api name, namespace and local name or -1 if nothing found */
sal_Int32 XMLPropertySetMapper::FindEntryIndex( sal_Char* sApiName, sal_uInt16 nNameSpace, sal_Char* sXMLName ) const
{
    const XMLPropertyMapEntry* pEntries = mpEntries;
    sal_Int32 nIndex = 0;

    while( pEntries && pEntries->msApiName )
    {
        if( pEntries->mnNameSpace == nNameSpace &&
            (strcmp( pEntries->msApiName, sApiName ) == 0) &&
            (strcmp( pEntries->msXMLName, sXMLName ) == 0))
            return nIndex;

        nIndex++;
        pEntries++;
    }

    return -1;
}
