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

#include <utility>
#include <vector>

#include <o3tl/sorted_vector.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
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
    sal_uInt32  nPos;
    bool    bIsNamed;


public:

    XMLTextListAutoStylePoolEntry_Impl(
            sal_uInt32 nPos,
            const Reference < XIndexReplace > & rNumRules,
            XMLTextListAutoStylePoolNames_Impl& rNames,
            std::u16string_view rPrefix,
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
            OUString aInternalName ) :
        sInternalName(std::move( aInternalName )),
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
        std::u16string_view rPrefix,
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
    do
    {
        rName++;
        sName = rPrefix + OUString::number( static_cast<sal_Int32>(rName) );
    }
    while (rNames.find(sName) != rNames.end());
}

namespace {

struct XMLTextListAutoStylePoolEntryCmp_Impl
{
    template <typename T1, typename T2>
        requires o3tl::is_reference_to<T1, XMLTextListAutoStylePoolEntry_Impl>
                 && o3tl::is_reference_to<T2, XMLTextListAutoStylePoolEntry_Impl>
    bool operator()(T1 const& r1, T2 const& r2) const
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

}

class XMLTextListAutoStylePool_Impl : public o3tl::sorted_vector<std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl>, XMLTextListAutoStylePoolEntryCmp_Impl> {};

XMLTextListAutoStylePool::XMLTextListAutoStylePool( SvXMLExport& rExp ) :
    m_rExport( rExp ),
    m_sPrefix( "L" ),
    m_pPool( new XMLTextListAutoStylePool_Impl ),
    m_nName( 0 )
{
    Reference<ucb::XAnyCompareFactory> xCompareFac( rExp.GetModel(), uno::UNO_QUERY );
    if( xCompareFac.is() )
        mxNumRuleCompare = xCompareFac->createAnyCompareByName( "NumberingRules" );
    SvXMLExportFlags nExportFlags = m_rExport.getExportFlags();
    bool bStylesOnly = (nExportFlags & SvXMLExportFlags::STYLES) && !(nExportFlags & SvXMLExportFlags::CONTENT);
    if( bStylesOnly )
        m_sPrefix = "ML";

    Reference<XStyleFamiliesSupplier> xFamiliesSupp(m_rExport.GetModel(), UNO_QUERY);
    SAL_WARN_IF(!xFamiliesSupp.is(), "xmloff", "getStyleFamilies() from XModel failed for export!");
    Reference< XNameAccess > xFamilies;
    if (xFamiliesSupp.is())
        xFamilies = xFamiliesSupp->getStyleFamilies();

    Reference<XIndexAccess> xStyles;
    static constexpr OUString aNumberStyleName(u"NumberingStyles"_ustr);
    if (xFamilies.is() && xFamilies->hasByName(aNumberStyleName))
        xFamilies->getByName(aNumberStyleName) >>= xStyles;

    const sal_Int32 nStyles = xStyles.is() ? xStyles->getCount() : 0;
    for (sal_Int32 i = 0; i < nStyles; i++)
    {
        Reference<XStyle> xStyle;
        xStyles->getByIndex(i) >>= xStyle;
        RegisterName(xStyle->getName());
    }
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
        const sal_uInt32 nCount = m_pPool->size();

        uno::Any aAny1, aAny2;
        aAny1 <<= pEntry->GetNumRules();

        for( sal_uInt32 nPos = 0; nPos < nCount; nPos++ )
        {
            aAny2 <<= (*m_pPool)[nPos]->GetNumRules();

            if( mxNumRuleCompare->compare( aAny1, aAny2 ) == 0 )
                return nPos;
        }
    }
    else
    {
        XMLTextListAutoStylePool_Impl::const_iterator it = m_pPool->find( pEntry );
        if( it != m_pPool->end() )
            return it - m_pPool->begin();
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
        sName = (*m_pPool)[ nPos ]->GetName();
    }
    else
    {
        std::unique_ptr<XMLTextListAutoStylePoolEntry_Impl> pEntry(
            new XMLTextListAutoStylePoolEntry_Impl( m_pPool->size(),
                                               rNumRules, m_aNames, m_sPrefix,
                                               m_nName ));
        sName = pEntry->GetName();
        m_pPool->insert( std::move(pEntry) );
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
        sName = (*m_pPool)[ nPos ]->GetName();

    return sName;
}

OUString XMLTextListAutoStylePool::Find(
            const OUString& rInternalName ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rInternalName );
    sal_uInt32 nPos = Find( &aTmp );
    if( nPos != sal_uInt32(-1) )
        sName = (*m_pPool)[ nPos ]->GetName();

    return sName;
}

void XMLTextListAutoStylePool::exportXML() const
{
    sal_uInt32 nCount = m_pPool->size();
    if( !nCount )
        return;

    std::vector<XMLTextListAutoStylePoolEntry_Impl*> aExpEntries(nCount);

    sal_uInt32 i;
    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = (*m_pPool)[i].get();
        SAL_WARN_IF( pEntry->GetPos() >= nCount, "xmloff", "Illegal pos" );
        aExpEntries[pEntry->GetPos()] = pEntry;
    }

    SvxXMLNumRuleExport aNumRuleExp( m_rExport );

    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = aExpEntries[i];
        aNumRuleExp.exportNumberingRule( pEntry->GetName(), false,
                                         pEntry->GetNumRules() );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
