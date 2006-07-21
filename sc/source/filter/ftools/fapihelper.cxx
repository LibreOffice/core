/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fapihelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:24:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif

#include <algorithm>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_DOCPASSWDREQUEST_HXX
#include <svtools/docpasswdrequest.hxx>
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::TypeClass_BOOLEAN;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionRequest;

// Static helper functions ====================================================

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
        SfxObjectShell* pShell, const OUString& rServiceName, const Sequence< Any >& rArgs )
{
    return CreateInstanceWithArgs( GetServiceFactory( pShell ), rServiceName, rArgs );
}

Reference< XInterface > ScfApiHelper::CreateInstanceWithArgs(
        const OUString& rServiceName, const Sequence< Any >& rArgs )
{
    return CreateInstanceWithArgs( ::comphelper::getProcessServiceFactory(), rServiceName, rArgs );
}

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
//        DBG_ASSERT( mxMultiPropSet.is(), "ScfPropertySet::SetProperties - multi property set not available" );
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
//        DBG_ERRORFILE( "ScfPropertySet::SetAnyProperty - cannot set multiple properties" );
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
        ::comphelper::setBOOL( *pAny, rbValue );
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

