/*************************************************************************
 *
 *  $RCSfile: xmlcnitm.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:11 $
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
#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#include "xmlcnimp.hxx"
#include "unoatrcn.hxx"

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
    USHORT nCount = rImpl.pLNames->Count();
    for( USHORT i=0; i<nCount; i++ )
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
    BOOL bRet = pLNames->Count() == rCmp.pLNames->Count() &&
                aNamespaceMap == rCmp.aNamespaceMap;
    if( bRet )
    {
        USHORT nCount = pLNames->Count();
        USHORT i;
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

BOOL SvXMLAttrContainerData::AddAttr( const OUString& rLName,
                                              const OUString& rValue )
{
    aPrefixPoss.Insert( USHRT_MAX, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                              const OUString& rNamespace,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    USHORT nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                           const OUString& rLName,
                                            const OUString& rValue )
{
    USHORT nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return FALSE;

    aPrefixPoss.Insert( nPos, aPrefixPoss.Count() );
    pLNames->Insert( new OUString(rLName), pLNames->Count() );
    pValues->Insert( new OUString(rValue), pValues->Count() );

    return TRUE;
}

BOOL SvXMLAttrContainerData::SetAt( USHORT i,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = USHRT_MAX;

    return TRUE;
}

BOOL SvXMLAttrContainerData::SetAt( USHORT i,
            const rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
            const rtl::OUString& rLName, const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    USHORT nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    if( USHRT_MAX == nPos )
        return FALSE;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = nPos;

    return TRUE;
}

BOOL SvXMLAttrContainerData::SetAt( USHORT i,
            const rtl::OUString& rPrefix,
            const rtl::OUString& rLName,
            const rtl::OUString& rValue )
{
    if( i >= GetAttrCount() )
        return FALSE;

    USHORT nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return FALSE;

    *(*pLNames)[i] = rLName;
    *(*pValues)[i] = rValue;
    aPrefixPoss[i] = nPos;

    return TRUE;
}

void SvXMLAttrContainerData::Remove( USHORT i )
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
    OSL_ENSURE( i >= 0 && i < pLNames->Count(),
                "SvXMLAttrContainerData::GetLName: illegal index" );
    return *(*pLNames)[i];
}

const ::rtl::OUString& SvXMLAttrContainerData::GetAttrValue(sal_uInt16 i) const
{
    OSL_ENSURE( i >= 0 && i < pValues->Count(),
                "SvXMLAttrContainerData::GetValue: illegal index" );
    return *(*pValues)[i];
}

