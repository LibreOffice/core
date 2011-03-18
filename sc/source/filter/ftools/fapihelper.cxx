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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "fapihelper.hxx"

#include <algorithm>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include "miscuno.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::TypeClass_BOOLEAN;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::lang::XServiceName;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER;

using namespace ::com::sun::star;

// Static helper functions ====================================================

OUString ScfApiHelper::GetServiceName( Reference< XInterface > xInt )
{
    OUString aService;
    Reference< XServiceName > xServiceName( xInt, UNO_QUERY );
    if( xServiceName.is() )
        aService = xServiceName->getServiceName();
    return aService;
}

Reference< XMultiServiceFactory > ScfApiHelper::GetServiceFactory( SfxObjectShell* pShell )
{
    Reference< XMultiServiceFactory > xFactory;
    if( pShell )
        xFactory.set( pShell->GetModel(), UNO_QUERY );
    return xFactory;
}

Reference< XInterface > ScfApiHelper::CreateInstance(
        Reference< XMultiServiceFactory > xFactory, const OUString& rServiceName )
{
    Reference< XInterface > xInt;
    if( xFactory.is() )
    {
        try
        {
            xInt = xFactory->createInstance( rServiceName );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "ScfApiHelper::CreateInstance - cannot create instance" );
        }
    }
    return xInt;
}

Reference< XInterface > ScfApiHelper::CreateInstance( SfxObjectShell* pShell, const OUString& rServiceName )
{
    return CreateInstance( GetServiceFactory( pShell ), rServiceName );
}

Reference< XInterface > ScfApiHelper::CreateInstance( const OUString& rServiceName )
{
    return CreateInstance( ::comphelper::getProcessServiceFactory(), rServiceName );
}

Reference< XInterface > ScfApiHelper::CreateInstanceWithArgs(
        Reference< XMultiServiceFactory > xFactory, const OUString& rServiceName, const Sequence< Any >& rArgs )
{
    Reference< XInterface > xInt;
    if( xFactory.is() )
    {
        try
        {
            xInt = xFactory->createInstanceWithArguments( rServiceName, rArgs );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "ScfApiHelper::CreateInstanceWithArgs - cannot create instance" );
        }
    }
    return xInt;
}

Reference< XInterface > ScfApiHelper::CreateInstanceWithArgs(
        const OUString& rServiceName, const Sequence< Any >& rArgs )
{
    return CreateInstanceWithArgs( ::comphelper::getProcessServiceFactory(), rServiceName, rArgs );
}

uno::Sequence< beans::NamedValue > ScfApiHelper::QueryEncryptionDataForMedium( SfxMedium& rMedium,
        ::comphelper::IDocPasswordVerifier& rVerifier, const ::std::vector< OUString >* pDefaultPasswords )
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false);
    if ( pEncryptionDataItem )
        pEncryptionDataItem->GetValue() >>= aEncryptionData;

    ::rtl::OUString aPassword;
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, false);
    if ( pPasswordItem )
        aPassword = pPasswordItem->GetValue();

    OUString aDocName = INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET );

    bool bIsDefaultPassword = false;
    aEncryptionData = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
        rVerifier, aEncryptionData, aPassword, rMedium.GetInteractionHandler(), aDocName,
        ::comphelper::DocPasswordRequestType_MS, pDefaultPasswords, &bIsDefaultPassword );

    rMedium.GetItemSet()->ClearItem( SID_PASSWORD );
    rMedium.GetItemSet()->ClearItem( SID_ENCRYPTIONDATA );

    if( !bIsDefaultPassword && (aEncryptionData.getLength() > 0) )
        rMedium.GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );

    return aEncryptionData;
}

// Property sets ==============================================================

void ScfPropertySet::Set( Reference< XPropertySet > xPropSet )
{
    mxPropSet = xPropSet;
    mxMultiPropSet.set( mxPropSet, UNO_QUERY );
}

OUString ScfPropertySet::GetServiceName() const
{
    return ScfApiHelper::GetServiceName( mxPropSet );
}

// Get properties -------------------------------------------------------------

bool ScfPropertySet::HasProperty( const OUString& rPropName ) const
{
    bool bHasProp = false;
    try
    {
        Reference< XPropertyState > xPropState( mxPropSet, UNO_QUERY_THROW );
        bHasProp = xPropState->getPropertyState( rPropName ) == ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }
    catch( Exception& )
    {
    }
    return bHasProp;
}

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

bool ScfPropertySet::GetBoolProperty( const ::rtl::OUString& rPropName ) const
{
    Any aAny;
    return GetAnyProperty( aAny, rPropName ) && ScUnoHelpFunctions::GetBoolFromAny( aAny );
}

bool ScfPropertySet::GetStringProperty( String& rValue, const OUString& rPropName ) const
{
    OUString aOUString;
    bool bRet = GetProperty( aOUString, rPropName );
    rValue = aOUString;
    return bRet;
}

bool ScfPropertySet::GetColorProperty( Color& rColor, const ::rtl::OUString& rPropName ) const
{
    sal_Int32 nApiColor = 0;
    bool bRet = GetProperty( nApiColor, rPropName );
    rColor = ScfApiHelper::ConvertFromApiColor( nApiColor );
    return bRet;
}

void ScfPropertySet::GetProperties( Sequence< Any >& rValues, const Sequence< OUString >& rPropNames ) const
{
    try
    {
        DBG_ASSERT( mxMultiPropSet.is(), "ScfPropertySet::GetProperties - multi property set not available" );
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
        DBG_ERRORFILE(
            ByteString( "ScfPropertySet::SetAnyProperty - cannot set property \"" ).
                Append( ByteString( String( rPropName ), RTL_TEXTENCODING_ASCII_US ) ).
                Append( '"' ).
                GetBuffer() );
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
            DBG_ERRORFILE( "ScfPropertySet::SetProperties - multi property set not available" );
            const OUString* pPropName = rPropNames.getConstArray();
            const OUString* pPropNameEnd = pPropName + rPropNames.getLength();
            const Any* pValue = rValues.getConstArray();
            for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
                mxPropSet->setPropertyValue( *pPropName, *pValue );
        }
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "ScfPropertySet::SetAnyProperty - cannot set multiple properties" );
    }
}

// ============================================================================

ScfPropSetHelper::ScfPropSetHelper( const sal_Char* const* ppcPropNames ) :
    mnNextIdx( 0 )
{
    DBG_ASSERT( ppcPropNames, "ScfPropSetHelper::ScfPropSetHelper - no strings found" );

    // create OUStrings from ASCII property names
    typedef ::std::pair< OUString, size_t >     IndexedOUString;
    typedef ::std::vector< IndexedOUString >    IndexedOUStringVec;
    IndexedOUStringVec aPropNameVec;
    for( size_t nVecIdx = 0; *ppcPropNames; ++ppcPropNames, ++nVecIdx )
    {
        OUString aPropName = OUString::createFromAscii( *ppcPropNames );
        aPropNameVec.push_back( IndexedOUString( aPropName, nVecIdx ) );
    }

    // sorts the pairs, which will be sorted by first component (the property name)
    ::std::sort( aPropNameVec.begin(), aPropNameVec.end() );

    // resize member sequences
    size_t nSize = aPropNameVec.size();
    maNameSeq.realloc( static_cast< sal_Int32 >( nSize ) );
    maValueSeq.realloc( static_cast< sal_Int32 >( nSize ) );
    maNameOrder.resize( nSize );

    // fill the property name sequence and store original sort order
    sal_Int32 nSeqIdx = 0;
    for( IndexedOUStringVec::const_iterator aIt = aPropNameVec.begin(),
            aEnd = aPropNameVec.end(); aIt != aEnd; ++aIt, ++nSeqIdx )
    {
        maNameSeq[ nSeqIdx ] = aIt->first;
        maNameOrder[ aIt->second ] = nSeqIdx;
    }
}

// read properties ------------------------------------------------------------

void ScfPropSetHelper::ReadFromPropertySet( const ScfPropertySet& rPropSet )
{
    rPropSet.GetProperties( maValueSeq, maNameSeq );
    mnNextIdx = 0;
}

bool ScfPropSetHelper::ReadValue( UnoAny& rAny )
{
    UnoAny* pAny = GetNextAny();
    if( pAny )
        rAny = *pAny;
    return pAny != 0;
}

bool ScfPropSetHelper::ReadValue( String& rString )
{
    OUString aOUString;
    bool bRet = ReadValue( aOUString );
    rString = aOUString;
    return bRet;
}

bool ScfPropSetHelper::ReadValue( Color& rColor )
{
    sal_Int32 nApiColor(0);
    bool bRet = ReadValue( nApiColor );
    rColor = ScfApiHelper::ConvertFromApiColor( nApiColor );
    return bRet;
}

bool ScfPropSetHelper::ReadValue( bool& rbValue )
{
    Any aAny;
    bool bRet = ReadValue( aAny );
    rbValue = ScUnoHelpFunctions::GetBoolFromAny( aAny );
    return bRet;
}

// write properties -----------------------------------------------------------

void ScfPropSetHelper::InitializeWrite( bool bClearAllAnys )
{
    mnNextIdx = 0;
    if( bClearAllAnys )
        for( sal_Int32 nIdx = 0, nLen = maValueSeq.getLength(); nIdx < nLen; ++nIdx )
            maValueSeq[ nIdx ].clear();
}

void ScfPropSetHelper::WriteValue( const Any& rAny )
{
    if( UnoAny* pAny = GetNextAny() )
        *pAny = rAny;
}

void ScfPropSetHelper::WriteValue( const bool& rbValue )
{
    if( Any* pAny = GetNextAny() )
        ScUnoHelpFunctions::SetBoolInAny( *pAny, rbValue );
}

void ScfPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetProperties( maNameSeq, maValueSeq );
}

// private --------------------------------------------------------------------

Any* ScfPropSetHelper::GetNextAny()
{
    DBG_ASSERT( mnNextIdx < maNameOrder.size(), "ScfPropSetHelper::GetNextAny - sequence overflow" );
    Any* pAny = 0;
    if( mnNextIdx < maNameOrder.size() )
        pAny = &maValueSeq[ maNameOrder[ mnNextIdx++ ] ];
    return pAny;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
