/*************************************************************************
 *
 *  $RCSfile: fapihelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:50 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;


// Set properties =============================================================

void setPropAny( const Reference< XPropertySet >& xProp, const OUString& rName, const Any& rAny )
{
    DBG_ASSERT( xProp.is(), "setPropAny - invalid PropertySet" );
    try
    {
        Reference< XPropertySetInfo > xInfo = xProp->getPropertySetInfo();
        if( xInfo.is() && xInfo->hasPropertyByName( rName ) )
            xProp->setPropertyValue( rName, rAny );
    }
    catch( Exception& )
    {
    }
}


// Get properties =============================================================

bool getPropAny( Any& rAny, const Reference< XPropertySet >& rxProp, const OUString& rName )
{
    DBG_ASSERT( rxProp.is(), "getPropAny - invalid XPropertySet" );
    bool bSuccess = false;
    try
    {
        Reference< XPropertySetInfo > xInfo = rxProp->getPropertySetInfo();
        if( xInfo.is() && xInfo->hasPropertyByName( rName ) )
        {
            rAny = rxProp->getPropertyValue( rName );
            bSuccess = true;
        }
    }
    catch( Exception& )
    {
    }
    return bSuccess;
}


// MultiPropertySets ==========================================================

ScfMultiPSHelper::ScfMultiPSHelper( const sal_Char** ppPropNames, sal_Int32 nPropCount ) :
    maNameSeq( nPropCount ),
    maValueSeq( nPropCount )
{
    DBG_ASSERT( nPropCount > 0, "ScfMultiPSHelper::ScfMultiPSHelper - invalid sequence size" );
    DBG_ASSERT( ppPropNames && *ppPropNames, "ScfMultiPSHelper::ScfMultiPSHelper - no strings found" );

    const sal_Char** ppCurrName = ppPropNames;
    for( sal_Int32 nIndex = 0; nIndex < nPropCount; ++nIndex, ++ppCurrName )
    {
        maNameSeq[ nIndex ] = OUString::createFromAscii( *ppCurrName );
        DBG_ASSERT( !nIndex || maNameSeq[ nIndex - 1 ] < maNameSeq[ nIndex ],
            "ScfMultiPSHelper::ScfMultiPSHelper - names MUST be ordered alphabetically" );
    }
}

bool ScfMultiPSHelper::getPropertyValues(
        const XMultiPropertySetRef& xMultiPS,
        const XPropertySetRef& xPropSet )
{
    bool bSuccess = false;
    try
    {
        if( xMultiPS.is() )         // first try the XMultiPropertySet
        {
            maValueSeq = xMultiPS->getPropertyValues( maNameSeq );
            bSuccess = true;
        }
        else if( xPropSet.is() )    // then try the XPropertySet
        {
            sal_Int32 nLen = maNameSeq.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nLen; ++nIndex )
                maValueSeq[ nIndex ] = xPropSet->getPropertyValue( maNameSeq[ nIndex ] );
            bSuccess = true;
        }
    }
    catch( Exception& )
    {
    }
    return bSuccess;
}

bool ScfMultiPSHelper::setPropertyValues(
        const XMultiPropertySetRef& xMultiPS,
        const XPropertySetRef& xPropSet )
{
    bool bSuccess = false;
    try
    {
        if( xMultiPS.is() )         // first try the XMultiPropertySet
        {
            xMultiPS->setPropertyValues( maNameSeq, maValueSeq );
            bSuccess = true;
        }
        else if( xPropSet.is() )    // then try the XPropertySet
        {
            sal_Int32 nLen = maNameSeq.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nLen; ++nIndex )
                xPropSet->setPropertyValue( maNameSeq[ nIndex ], maValueSeq[ nIndex ] );
            bSuccess = true;
        }
    }
    catch( Exception& )
    {
    }
    return bSuccess;
}


//___________________________________________________________________

