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
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <xmloff/xmlcnimp.hxx>
#include "xmloff/unoatrcn.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;

using ::rtl::OUString;

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
        OSL_FAIL( "illegal index" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
