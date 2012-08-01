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

#include <tools/debug.hxx>
#include <svl/cntnrsrt.hxx>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlnume.hxx>
#include "xmloff/XMLTextListAutoStylePool.hxx"
#include <xmloff/xmlexp.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;


class XMLTextListAutoStylePoolEntry_Impl
{
    OUString    sName;
    OUString    sInternalName;
    Reference < XIndexReplace > xNumRules;
    sal_uInt32  nPos;
    sal_Bool    bIsNamed;


public:

    XMLTextListAutoStylePoolEntry_Impl(
            sal_uInt32 nPos,
            const Reference < XIndexReplace > & rNumRules,
            XMLTextListAutoStylePoolNames_Impl& rNames,
            const OUString& rPrefix,
            sal_uInt32& rName );

    XMLTextListAutoStylePoolEntry_Impl(
            const Reference < XIndexReplace > & rNumRules ) :
        xNumRules( rNumRules ),
        nPos( 0 ),
        bIsNamed( sal_False )
    {
        Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
        if( xNamed.is() )
        {
            sInternalName = xNamed->getName();
            bIsNamed = sal_True;
        }
    }

    XMLTextListAutoStylePoolEntry_Impl(
            const OUString& rInternalName ) :
        sInternalName( rInternalName ),
        nPos( 0 ),
        bIsNamed( sal_True )
    {
    }

    const OUString& GetName() const { return sName; }
    const OUString& GetInternalName() const { return sInternalName; }
    const Reference < XIndexReplace > & GetNumRules() const { return xNumRules; }
    sal_uInt32 GetPos() const { return nPos; }
    sal_Bool IsNamed() const { return bIsNamed; }
};

XMLTextListAutoStylePoolEntry_Impl::XMLTextListAutoStylePoolEntry_Impl(
        sal_uInt32 nP,
        const Reference < XIndexReplace > & rNumRules,
        XMLTextListAutoStylePoolNames_Impl& rNames,
        const OUString& rPrefix,
        sal_uInt32& rName ) :
    xNumRules( rNumRules ),
    nPos( nP ),
    bIsNamed( sal_False )
{
    Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
    if( xNamed.is() )
    {
        sInternalName = xNamed->getName();
        bIsNamed = sal_True;
    }

    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        rName++;
        sBuffer.append( rPrefix );
        sBuffer.append( (sal_Int32)rName );
        sName = sBuffer.makeStringAndClear();
    }
    while (rNames.find(sName) != rNames.end());
}

int XMLTextListAutoStylePoolEntryCmp_Impl(
        const XMLTextListAutoStylePoolEntry_Impl& r1,
        const XMLTextListAutoStylePoolEntry_Impl& r2 )
{
    int nRet;
    if( r1.IsNamed() )
    {
        if( r2.IsNamed() )
             nRet = (int)r1.GetInternalName().compareTo( r2.GetInternalName());
        else
            nRet = -1;
    }
    else
    {
        if( r2.IsNamed() )
            nRet = 1;
        else
             nRet = (int)(r1.GetNumRules().get() - r2.GetNumRules().get());
    }

    return nRet;
}

typedef XMLTextListAutoStylePoolEntry_Impl *XMLTextListAutoStylePoolEntryPtr;
DECLARE_CONTAINER_SORT( XMLTextListAutoStylePool_Impl,
                        XMLTextListAutoStylePoolEntry_Impl )
IMPL_CONTAINER_SORT( XMLTextListAutoStylePool_Impl,
                     XMLTextListAutoStylePoolEntry_Impl,
                     XMLTextListAutoStylePoolEntryCmp_Impl )

XMLTextListAutoStylePool::XMLTextListAutoStylePool( SvXMLExport& rExp ) :
    rExport( rExp ),
    sPrefix( RTL_CONSTASCII_USTRINGPARAM("L") ),
    pPool( new XMLTextListAutoStylePool_Impl( 5, 5 ) ),
    nName( 0 )
{
    Reference<ucb::XAnyCompareFactory> xCompareFac( rExp.GetModel(), uno::UNO_QUERY );
    if( xCompareFac.is() )
        mxNumRuleCompare = xCompareFac->createAnyCompareByName( OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ) );
    sal_uInt16 nExportFlags = rExport.getExportFlags();
    sal_Bool bStylesOnly = (nExportFlags & EXPORT_STYLES) != 0 && (nExportFlags & EXPORT_CONTENT) == 0;
    if( bStylesOnly )
        sPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM("ML") );

}

XMLTextListAutoStylePool::~XMLTextListAutoStylePool()
{
    // The XMLTextListAutoStylePoolEntry_Impl object in the pool need delete explicitly in dtor.
    sal_uLong nCount = pPool->Count();
    while ( nCount-- )
        delete pPool->Remove(nCount);
    delete pPool;
}

void XMLTextListAutoStylePool::RegisterName( const OUString& rName )
{
    m_aNames.insert(rName);
}

sal_uInt32 XMLTextListAutoStylePool::Find( XMLTextListAutoStylePoolEntry_Impl* pEntry ) const
{
    sal_uLong nPos;
    if( !pEntry->IsNamed() && mxNumRuleCompare.is() )
    {
        const sal_uInt32 nCount = pPool->Count();

        uno::Any aAny1, aAny2;
        aAny1 <<= pEntry->GetNumRules();

        for( nPos = 0; nPos < nCount; nPos++ )
        {
            aAny2 <<= pPool->GetObject(nPos)->GetNumRules();

            if( mxNumRuleCompare->compare( aAny1, aAny2 ) == 0 )
                return nPos;
        }
    }
    else if( pPool->Seek_Entry( pEntry, &nPos ) )
    {
        return nPos;
    }

    return (sal_uInt32)-1;
}

OUString XMLTextListAutoStylePool::Add(
            const Reference < XIndexReplace > & rNumRules )
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );

    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != (sal_uInt32)-1 )
    {
        sName = pPool->GetObject( nPos )->GetName();
    }
    else
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry =
            new XMLTextListAutoStylePoolEntry_Impl( pPool->Count(),
                                               rNumRules, m_aNames, sPrefix,
                                               nName );
        pPool->Insert( pEntry );
        sName = pEntry->GetName();
    }

    return sName;
}

::rtl::OUString XMLTextListAutoStylePool::Find(
            const Reference < XIndexReplace > & rNumRules ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );

    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != (sal_uInt32)-1 )
        sName = pPool->GetObject( nPos )->GetName();

    return sName;
}

::rtl::OUString XMLTextListAutoStylePool::Find(
            const OUString& rInternalName ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rInternalName );
    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != (sal_uInt32)-1 )
        sName = pPool->GetObject( nPos )->GetName();

    return sName;
}

void XMLTextListAutoStylePool::exportXML() const
{
    sal_uInt32 nCount = pPool->Count();
    if( !nCount )
        return;

    XMLTextListAutoStylePoolEntry_Impl **aExpEntries =
        new XMLTextListAutoStylePoolEntryPtr[nCount];

    sal_uInt32 i;
    for( i=0; i < nCount; i++ )
    {
        aExpEntries[i] = 0;
    }
    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = pPool->GetObject(i);
        DBG_ASSERT( pEntry->GetPos() < nCount, "Illegal pos" );
        aExpEntries[pEntry->GetPos()] = pEntry;
    }

    SvxXMLNumRuleExport aNumRuleExp( rExport );

    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = aExpEntries[i];
        aNumRuleExp.exportNumberingRule( pEntry->GetName(),
                                         pEntry->GetNumRules() );
    }
    delete [] aExpEntries;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
