/*************************************************************************
 *
 *  $RCSfile: MultiPropertySetHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2001-09-05 08:30:32 $
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

#ifndef _XMLOFF_MULTIPROPERTYSETHELPER_HXX
#include "MultiPropertySetHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

// STL includes
#include <algorithm>


using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::comphelper::UStringLess;
using ::rtl::OUString;
using ::std::sort;


MultiPropertySetHelper::MultiPropertySetHelper(
    const sal_Char** pNames ) :
        pPropertyNames( NULL ),
        nLength( 0 ),
        aPropertySequence(),
        pSequenceIndex( NULL ),
        aValues(),
        pValues( NULL )
{
    // first count the elements
    for( const sal_Char** pPtr = pNames; *pPtr != NULL; pPtr++ )
        nLength++;

    // allocate array and create strings
    pPropertyNames = new OUString[nLength];
    for( sal_Int16 i = 0; i < nLength; i++ )
        pPropertyNames[i] = OUString::createFromAscii( pNames[i] );
}

MultiPropertySetHelper::MultiPropertySetHelper(
    const OUString* pNames ) :
        pPropertyNames( NULL ),
        nLength( 0 ),
        aPropertySequence(),
        pSequenceIndex( NULL ),
        aValues(),
        pValues( NULL )
{
    // count elements
    for( const OUString* pPtr = pNames; pPtr != NULL; pPtr++ )
        nLength++;

    // allocate array and assign strings
    pPropertyNames = new OUString[nLength];
    for( sal_Int16 i = 0; i < nLength; i++ )
        pPropertyNames[i] = pNames[i];
}


MultiPropertySetHelper::~MultiPropertySetHelper()
{
    pValues = NULL; // memory 'owned' by aValues

    delete[] pSequenceIndex;
    delete[] pPropertyNames;
}



void MultiPropertySetHelper::hasProperties(
    const Reference<XPropertySetInfo> & rInfo )
{
    DBG_ASSERT( rInfo.is(), "I'd really like an XPropertySetInfo here." );

    // allocate sequence index
    if ( NULL == pSequenceIndex )
        pSequenceIndex = new sal_Int16[nLength] ;

    // construct pSequenceIndex
    sal_Int16 nNumberOfProperties = 0;
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        // ask for property
        sal_Bool bHasProperty =
            rInfo->hasPropertyByName( pPropertyNames[i] );

        // set index and increment (if appropriate)
        pSequenceIndex[i]= bHasProperty ? nNumberOfProperties : -1;
        if ( bHasProperty )
            nNumberOfProperties++;
    }

    // construct property sequence from index array
    if ( aPropertySequence.getLength() != nNumberOfProperties )
        aPropertySequence.realloc( nNumberOfProperties );
    OUString* pPropertySequence = aPropertySequence.getArray();
    for( i = 0; i < nLength; i ++ )
    {
        sal_Int16 nIndex = pSequenceIndex[i];
        if ( nIndex != -1 )
            pPropertySequence[nIndex] = pPropertyNames[i];
    }
}

sal_Bool MultiPropertySetHelper::checkedProperties()
{
    return (NULL != pSequenceIndex);
}



void MultiPropertySetHelper::getValues(
    const Reference<XMultiPropertySet> & rMultiPropertySet )
{
    DBG_ASSERT( rMultiPropertySet.is(), "We need an XMultiPropertySet." );

    aValues = rMultiPropertySet->getPropertyValues( aPropertySequence );
    pValues = aValues.getConstArray();
}

void MultiPropertySetHelper::getValues(
    const Reference<XPropertySet> & rPropertySet )
{
    DBG_ASSERT( rPropertySet.is(), "We need an XPropertySet." );

    // re-alloc aValues (if necessary) and fill with values from XPropertySet
    sal_Int16 nSupportedPropertiesCount =
        (sal_Int16)aPropertySequence.getLength();
    if ( aValues.getLength() != nSupportedPropertiesCount )
        aValues.realloc( nSupportedPropertiesCount );
    Any* pMutableArray = aValues.getArray();
    for( sal_Int16 i = 0; i < nSupportedPropertiesCount; i++ )
    {
        pMutableArray[i] = rPropertySet->getPropertyValue(
            pPropertyNames[ pSequenceIndex[ i ] ] );
    }

    // re-establish pValues pointer
    pValues = aValues.getConstArray();
}


const Any& MultiPropertySetHelper::getValue( sal_Int16 nIndex,
                     const Reference< XPropertySet> & rPropSet,
                     sal_Bool bTryMulti )
{
    if( !pValues )
    {
        if( bTryMulti )
        {
            Reference < XMultiPropertySet > xMultiPropSet( rPropSet,
                                                           UNO_QUERY );
            if( xMultiPropSet.is() )
                getValues( xMultiPropSet );
            else
                getValues( rPropSet );
        }
        else
        {
            getValues( rPropSet );
        }
    }

    return getValue( nIndex );
}

const Any& MultiPropertySetHelper::getValue( sal_Int16 nIndex,
                     const Reference< XMultiPropertySet> & rMultiPropSet )
{
    if( !pValues )
        getValues( rMultiPropSet );

    return getValue( nIndex );
}

// inline methods defined in header:
// inline Any& MultiPropertySetHelper::getValue( sal_Int16 nIndex )
// inline sal_Bool MultiPropertySetHelper::hasProperty( sal_Int16 nValueNo )
