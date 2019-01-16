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

#include <vector>

#include <tools/solar.h>
#include <o3tl/sorted_vector.hxx>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/xmlexp.hxx>


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
    sal_uInt32 const  nPos;
    bool    bIsNamed;


public:

    XMLTextListAutoStylePoolEntry_Impl(
            sal_uInt32 nPos,
            const Reference < XIndexReplace > & rNumRules,
            XMLTextListAutoStylePoolNames_Impl& rNames,
            const OUString& rPrefix,
            sal_uInt32& rName );

    explicit XMLTextListAutoStylePoolEntry_Impl(
            const Reference < XIndexReplace > & rNumRules ) :
        xNumRules( rNumRules ),
        nPos( 0 ),
        bIsNamed( false )
    {
        Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
        if( xNamed.is() )
        {
            sInternalName = xNamed->getName();
            bIsNamed = true;
        }
    }

    explicit XMLTextListAutoStylePoolEntry_Impl(
            const OUString& rInternalName ) :
        sInternalName( rInternalName ),
        nPos( 0 ),
        bIsNamed( true )
    {
    }

    const OUString& GetName() const { return sName; }
    const OUString& GetInternalName() const { return sInternalName; }
    const Reference < XIndexReplace > & GetNumRules() const { return xNumRules; }
    sal_uInt32 GetPos() const { return nPos; }
    bool IsNamed() const { return bIsNamed; }
};

XMLTextListAutoStylePoolEntry_Impl::XMLTextListAutoStylePoolEntry_Impl(
        sal_uInt32 nP,
        const Reference < XIndexReplace > & rNumRules,
        XMLTextListAutoStylePoolNames_Impl& rNames,
        const OUString& rPrefix,
        sal_uInt32& rName ) :
    xNumRules( rNumRules ),
    nPos( nP ),
    bIsNamed( false )
{
    Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
    if( xNamed.is() )
    {
        sInternalName = xNamed->getName();
        bIsNamed = true;
    }

    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        rName++;
        sBuffer.append( rPrefix );
        sBuffer.append( static_cast<sal_Int32>(rName) );
        sName = sBuffer.makeStringAndClear();
    }
    while (rNames.find(sName) != rNames.end());
}

struct XMLTextListAutoStylePoolEntryCmp_Impl
{
    bool operator()(
            std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl> const& r1,
            std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl> const& r2 ) const
    {
        if( r1->IsNamed() )
        {
            if( r2->IsNamed() )
                return r1->GetInternalName().compareTo( r2->GetInternalName() ) < 0;
            else
                return true;
        }
        else
        {
            if( r2->IsNamed() )
                return false;
            else
                return r1->GetNumRules().get() < r2->GetNumRules().get();
        }
    }
};
class XMLTextListAutoStylePool_Impl : public o3tl::sorted_vector<std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl>, XMLTextListAutoStylePoolEntryCmp_Impl> {};

XMLTextListAutoStylePool::XMLTextListAutoStylePool( SvXMLExport& rExp ) :
    rExport( rExp ),
    sPrefix( "L" ),
    pPool( new XMLTextListAutoStylePool_Impl ),
    nName( 0 )
{
    Reference<ucb::XAnyCompareFactory> xCompareFac( rExp.GetModel(), uno::UNO_QUERY );
    if( xCompareFac.is() )
        mxNumRuleCompare = xCompareFac->createAnyCompareByName( "NumberingRules" );
    SvXMLExportFlags nExportFlags = rExport.getExportFlags();
    bool bStylesOnly = (nExportFlags & SvXMLExportFlags::STYLES) && !(nExportFlags & SvXMLExportFlags::CONTENT);
    if( bStylesOnly )
        sPrefix = "ML";

}

XMLTextListAutoStylePool::~XMLTextListAutoStylePool()
{
}

void XMLTextListAutoStylePool::RegisterName( const OUString& rName )
{
    m_aNames.insert(rName);
}

sal_uInt32 XMLTextListAutoStylePool::Find( const XMLTextListAutoStylePoolEntry_Impl* pEntry ) const
{
    if( !pEntry->IsNamed() && mxNumRuleCompare.is() )
    {
        const sal_uInt32 nCount = pPool->size();

        uno::Any aAny1, aAny2;
        aAny1 <<= pEntry->GetNumRules();

        for( sal_uLong nPos = 0; nPos < nCount; nPos++ )
        {
            aAny2 <<= (*pPool)[nPos]->GetNumRules();

            if( mxNumRuleCompare->compare( aAny1, aAny2 ) == 0 )
                return nPos;
        }
    }
    else
    {
        XMLTextListAutoStylePool_Impl::const_iterator it = pPool->find( pEntry );
        if( it != pPool->end() )
            return it - pPool->begin();
    }

    return sal_uInt32(-1);
}

OUString XMLTextListAutoStylePool::Add(
            const Reference < XIndexReplace > & rNumRules )
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );

    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != sal_uInt32(-1) )
    {
        sName = (*pPool)[ nPos ]->GetName();
    }
    else
    {
        std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl> pEntry(
            new XMLTextListAutoStylePoolEntry_Impl( pPool->size(),
                                               rNumRules, m_aNames, sPrefix,
                                               nName ));
        sName = pEntry->GetName();
        pPool->insert( std::move(pEntry) );
    }

    return sName;
}

OUString XMLTextListAutoStylePool::Find(
            const Reference < XIndexReplace > & rNumRules ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );

    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != sal_uInt32(-1) )
        sName = (*pPool)[ nPos ]->GetName();

    return sName;
}

OUString XMLTextListAutoStylePool::Find(
            const OUString& rInternalName ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rInternalName );
    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != sal_uInt32(-1) )
        sName = (*pPool)[ nPos ]->GetName();

    return sName;
}

void XMLTextListAutoStylePool::exportXML() const
{
    sal_uInt32 nCount = pPool->size();
    if( !nCount )
        return;

    std::vector<XMLTextListAutoStylePoolEntry_Impl*> aExpEntries(nCount);

    sal_uInt32 i;
    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = (*pPool)[i].get();
        SAL_WARN_IF( pEntry->GetPos() >= nCount, "xmloff", "Illegal pos" );
        aExpEntries[pEntry->GetPos()] = pEntry;
    }

    SvxXMLNumRuleExport aNumRuleExp( rExport );

    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = aExpEntries[i];
        aNumRuleExp.exportNumberingRule( pEntry->GetName(), false,
                                         pEntry->GetNumRules() );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
