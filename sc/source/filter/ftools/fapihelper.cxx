/*************************************************************************
 *
 *  $RCSfile: fapihelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 13:41:52 $
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

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
// no include guard in this header!!!
//#ifndef ...
#include <svtools/docpasswdrequest.hxx>
//#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::TypeClass_BOOLEAN;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionRequest;

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

// Static helper functions ====================================================

String ScfApiHelper::QueryPasswordForMedium( SfxMedium& rMedium )
{
    String aPassw;
    const SfxItemSet* pSet = rMedium.GetItemSet();
    const SfxPoolItem *pPasswordItem;

    if( pSet && (SFX_ITEM_SET == pSet->GetItemState( SID_PASSWORD, TRUE, &pPasswordItem )) )
    {
        aPassw = static_cast< const SfxStringItem* >( pPasswordItem )->GetValue();
    }
    else
    {
        try
        {
            Reference< XInteractionHandler > xHandler( rMedium.GetInteractionHandler() );
            if( xHandler.is() )
            {
                RequestDocumentPassword* pRequest = new RequestDocumentPassword(
                    ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER,
                    INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );
                Reference< XInteractionRequest > xRequest( pRequest );

               xHandler->handle( xRequest );

               if( pRequest->isPassword() )
                   aPassw = pRequest->getPassword();
            }
        }
        catch( Exception& )
        {
        }
    }

    return aPassw;
}

// Property sets ==============================================================

void ScfPropertySet::Set( Reference< XPropertySet > xPropSet )
{
    mxPropSet = xPropSet;
    mxMultiPropSet.set( mxPropSet, UNO_QUERY );
}

// Get properties -------------------------------------------------------------

bool ScfPropertySet::GetAnyProperty( Any& rValue, const OUString& rPropName ) const
{
    bool bHasValue = false;
    try
    {
        if( mxPropSet.is() )
        {
            rValue = mxPropSet->getPropertyValue( rPropName );
            bHasValue = true;
        }
    }
    catch( Exception& )
    {
    }
    return bHasValue;
}

bool ScfPropertySet::GetBoolProperty( bool& rbValue, const ::rtl::OUString& rPropName ) const
{
    Any aAny;
    bool bRet = GetAnyProperty( aAny, rPropName ) && (aAny.getValueTypeClass() == TypeClass_BOOLEAN);
    if( bRet )
        rbValue = *static_cast< const sal_Bool* >( aAny.getValue() );
    return bRet;
}

bool ScfPropertySet::GetStringProperty( String& rValue, const OUString& rPropName ) const
{
    OUString aOUString;
    bool bRet = GetProperty( aOUString, rPropName );
    if( bRet )
        rValue = aOUString;
    return bRet;
}

bool ScfPropertySet::GetColorProperty( Color& rColor, const ::rtl::OUString& rPropName ) const
{
    sal_Int32 nApiColor;
    bool bRet = GetProperty( nApiColor, rPropName );
    if( bRet )
        rColor.SetColor( static_cast< ColorData >( nApiColor ) );
    return bRet;
}

void ScfPropertySet::GetProperties( Sequence< Any >& rValues, const Sequence< OUString >& rPropNames ) const
{
    try
    {
        if( mxMultiPropSet.is() )   // first try the XMultiPropertySet
        {
            rValues = mxMultiPropSet->getPropertyValues( rPropNames );
        }
        else if( mxPropSet.is() )
        {
            sal_Int32 nLen = rPropNames.getLength();
            const OUString* pPropName = rPropNames.getConstArray();
            const OUString* pPropNameEnd = pPropName + nLen;
            rValues.realloc( nLen );
            Any* pValue = rValues.getArray();
            for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
                *pValue = mxPropSet->getPropertyValue( *pPropName );
        }
    }
    catch( Exception& )
    {
    }
}

// Set properties -------------------------------------------------------------

void ScfPropertySet::SetAnyProperty( const OUString& rPropName, const Any& rValue )
{
    try
    {
        if( mxPropSet.is() )
            mxPropSet->setPropertyValue( rPropName, rValue );
    }
    catch( Exception& )
    {
    }
}

void ScfPropertySet::SetProperties( const Sequence< OUString >& rPropNames, const Sequence< Any >& rValues )
{
    DBG_ASSERT( rPropNames.getLength() == rValues.getLength(), "ScfPropertySet::SetProperties - length of sequences different" );
    try
    {
        if( mxMultiPropSet.is() )   // first try the XMultiPropertySet
        {
            mxMultiPropSet->setPropertyValues( rPropNames, rValues );
        }
        else if( mxPropSet.is() )
        {
            const OUString* pPropName = rPropNames.getConstArray();
            const OUString* pPropNameEnd = pPropName + rPropNames.getLength();
            const Any* pValue = rValues.getConstArray();
            for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
                mxPropSet->setPropertyValue( *pPropName, *pValue );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

ScfPropSetHelper::ScfPropSetHelper( const sal_Char** ppcPropNames, sal_Int32 nPropCount ) :
    maNameSeq( nPropCount ),
    maValueSeq( nPropCount )
{
    DBG_ASSERT( nPropCount > 0, "ScfPropSetHelper::ScfPropSetHelper - invalid sequence size" );
    DBG_ASSERT( ppcPropNames && *ppcPropNames, "ScfPropSetHelper::ScfPropSetHelper - no strings found" );

    const sal_Char** ppCurrName = ppcPropNames;
    for( sal_Int32 nIndex = 0; nIndex < nPropCount; ++nIndex, ++ppCurrName )
    {
        maNameSeq[ nIndex ] = OUString::createFromAscii( *ppCurrName );
        DBG_ASSERT( !nIndex || (maNameSeq[ nIndex - 1 ] < maNameSeq[ nIndex ]),
            "ScfPropSetHelper::ScfPropSetHelper - names MUST be ordered alphabetically" );
    }
}

void ScfPropSetHelper::ReadFromPropertySet( const ScfPropertySet& rPropSet )
{
    rPropSet.GetProperties( maValueSeq, maNameSeq );
}

void ScfPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetProperties( maNameSeq, maValueSeq );
}

// ============================================================================

