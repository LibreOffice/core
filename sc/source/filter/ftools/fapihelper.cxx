/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "fapihelper.hxx"

#include <algorithm>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySetOption.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include "miscuno.hxx"

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

OUString ScfApiHelper::GetServiceName( const Reference< XInterface >& xInt )
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
        const Reference< XMultiServiceFactory >& xFactory, const OUString& rServiceName )
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
            OSL_FAIL( "ScfApiHelper::CreateInstance - cannot create instance" );
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

uno::Sequence< beans::NamedValue > ScfApiHelper::QueryEncryptionDataForMedium( SfxMedium& rMedium,
        ::comphelper::IDocPasswordVerifier& rVerifier, const ::std::vector< OUString >* pDefaultPasswords )
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA);
    if ( pEncryptionDataItem )
        pEncryptionDataItem->GetValue() >>= aEncryptionData;

    OUString aPassword;
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD);
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

ScfPropertySet::~ScfPropertySet()
{
    Reference<beans::XPropertySetOption> xPropSetOpt(mxPropSet, UNO_QUERY);
    if (xPropSetOpt.is())
        // Turn the property value change notification back on when finished.
        xPropSetOpt->enableChangeListenerNotification(true);
}

void ScfPropertySet::Set( Reference< XPropertySet > xPropSet )
{
    mxPropSet = xPropSet;
    mxMultiPropSet.set( mxPropSet, UNO_QUERY );
    Reference<beans::XPropertySetOption> xPropSetOpt(mxPropSet, UNO_QUERY);
    if (xPropSetOpt.is())
        // We don't want to broadcast property value changes during import to
        // improve performance.
        xPropSetOpt->enableChangeListenerNotification(false);
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

bool ScfPropertySet::GetBoolProperty( const OUString& rPropName ) const
{
    Any aAny;
    return GetAnyProperty( aAny, rPropName ) && ScUnoHelpFunctions::GetBoolFromAny( aAny );
}

OUString ScfPropertySet::GetStringProperty( const OUString& rPropName ) const
{
    OUString aOUString;
    GetProperty( aOUString, rPropName );
    return aOUString;
}

bool ScfPropertySet::GetColorProperty( Color& rColor, const OUString& rPropName ) const
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
        OSL_ENSURE( mxMultiPropSet.is(), "ScfPropertySet::GetProperties - multi property set not available" );
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
    catch (const Exception&)
    {
        SAL_WARN("sc", "ScfPropertySet::SetAnyProperty - cannot set property \"" + rPropName + "\"");
    }
}

void ScfPropertySet::SetProperties( const Sequence< OUString >& rPropNames, const Sequence< Any >& rValues )
{
    OSL_ENSURE( rPropNames.getLength() == rValues.getLength(), "ScfPropertySet::SetProperties - length of sequences different" );
    try
    {
        if( mxMultiPropSet.is() )   // first try the XMultiPropertySet
        {
            mxMultiPropSet->setPropertyValues( rPropNames, rValues );
        }
        else if( mxPropSet.is() )
        {
            OSL_FAIL( "ScfPropertySet::SetProperties - multi property set not available" );
            const OUString* pPropName = rPropNames.getConstArray();
            const OUString* pPropNameEnd = pPropName + rPropNames.getLength();
            const Any* pValue = rValues.getConstArray();
            for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
                mxPropSet->setPropertyValue( *pPropName, *pValue );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "ScfPropertySet::SetAnyProperty - cannot set multiple properties" );
    }
}

ScfPropSetHelper::ScfPropSetHelper( const sal_Char* const* ppcPropNames ) :
    mnNextIdx( 0 )
{
    OSL_ENSURE( ppcPropNames, "ScfPropSetHelper::ScfPropSetHelper - no strings found" );

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

bool ScfPropSetHelper::ReadValue( Any& rAny )
{
    Any* pAny = GetNextAny();
    if( pAny )
        rAny = *pAny;
    return pAny != 0;
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
    if( Any* pAny = GetNextAny() )
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
    OSL_ENSURE( mnNextIdx < maNameOrder.size(), "ScfPropSetHelper::GetNextAny - sequence overflow" );
    Any* pAny = 0;
    if( mnNextIdx < maNameOrder.size() )
        pAny = &maValueSeq[ maNameOrder[ mnNextIdx++ ] ];
    return pAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
