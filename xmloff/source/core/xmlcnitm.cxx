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
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <xmloff/xmlcnimp.hxx>
#include "xmloff/unoatrcn.hxx"

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;

typedef ::rtl::OUString *OUStringPtr;
SV_DECL_PTRARR_DEL( SvXMLAttrContainerData_Impl, OUStringPtr, 5, 5 )
SV_IMPL_PTRARR( SvXMLAttrContainerData_Impl, OUStringPtr )


SvXMLAttrContainerData::SvXMLAttrContainerData(
                            const SvXMLAttrContainerData& rImpl ) :
    aNamespaceMap( rImpl.aNamespaceMap ),
    pLNames( new SvXMLAttrContainerData_Impl ),
    pValues( new SvXMLAttrContainerData_Impl )
{
    sal_uInt16 nCount = rImpl.pLNames->Count();
    for( sal_uInt16 i=0; i<nCount; i++ )
    {
        aPrefixPoss.Insert( rImpl.aPrefixPoss[i], i );
        pLNames->Insert( new OUString( *(*rImpl.pLNames)[i] ), i );
        pValues->Insert( new OUString( *(*rImpl.pValues)[i] ), i );
    }
}

SvXMLAttrContainerData::SvXMLAttrContainerData() :
    pLNames( new SvXMLAttrContainerData_Impl ),
    pValues( new SvXMLAttrContainerData_Impl )
{
}

SvXMLAttrContainerData::~SvXMLAttrContainerData()
{
    delete pLNames;
    delete pValues;
}

int SvXMLAttrContainerData::operator ==(
                        const SvXMLAttrContainerData& rCmp ) const
{
    sal_Bool bRet = pLNames->Count() == rCmp.pLNames->Count() &&
                aNamespaceMap == rCmp.aNamespaceMap;
    if( bRet )
    {
        sal_uInt16 nCount = pLNames->Count();
        sal_uInt16 i;
        for( i=0; bRet && i < nCount; i++ )
            bRet = aPrefixPoss[i] == rCmp.aPrefixPoss[i];

        if( bRet )
        {
            for( i=0; bRet && i < nCount; i++ )
                bRet = *(*pLNames)[i] == *(*rCmp.pLNames)[i] &&
                       *(*pValues)[i] == *(*rCmp.pValues)[i];
        }
    }

    return (int)bRet;
}

sal_Bool SvXMLAttrContainerData::AddAttr( const OUString& rLName,
                                              const OUString& rValue )
{
    aPrefixPoss.Insert( USHRT_MAX, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return sal_True;
}

sal_Bool SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                              const OUString& rNamespace,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return sal_True;
}

sal_Bool SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return sal_False;

    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return sal_True;
}

sal_Bool SvXMLAttrContainerData::SetAt( sal_uInt16 i,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = USHRT_MAX;

    return sal_True;
}

sal_Bool SvXMLAttrContainerData::SetAt( sal_uInt16 i,
            const rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;

    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    if( USHRT_MAX == nPos )
        return sal_False;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = nPos;

    return sal_True;
}

sal_Bool SvXMLAttrContainerData::SetAt( sal_uInt16 i,
            const rtl::OUString& rPrefix,
            const rtl::OUString& rLName,
            const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;

    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return sal_False;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = nPos;

    return sal_True;
}

void SvXMLAttrContainerData::Remove( sal_uInt16 i )
{
    if( i < GetAttrCount() )
    {
        delete (*pLNames)[i];
        pLNames->Remove( i );
        delete (*pValues)[i];
        pValues->Remove( i );
        aPrefixPoss.Remove( i );
    }
    else
    {
        DBG_ERROR( "illegal index" );
    }
}

sal_uInt16 SvXMLAttrContainerData::GetAttrCount() const
{
    return pLNames->Count();
}

const ::rtl::OUString& SvXMLAttrContainerData::GetAttrLName(sal_uInt16 i) const
{
    OSL_ENSURE( i < pLNames->Count(), "SvXMLAttrContainerData::GetLName: illegal index" );
    return *(*pLNames)[i];
}

const ::rtl::OUString& SvXMLAttrContainerData::GetAttrValue(sal_uInt16 i) const
{
    OSL_ENSURE( i < pValues->Count(), "SvXMLAttrContainerData::GetValue: illegal index" );
    return *(*pValues)[i];
}

