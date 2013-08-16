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

#include <tools/debug.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/attrlist.hxx>
#include "impastpl.hxx"
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/PageMasterStyleMap.hxx>

using namespace ::std;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// Class SvXMLAutoStylePool_Impl
// ctor/dtor class SvXMLAutoStylePool_Impl

SvXMLAutoStylePoolP_Impl::SvXMLAutoStylePoolP_Impl( SvXMLExport& rExp)
    :   rExport( rExp )
{
}

SvXMLAutoStylePoolP_Impl::~SvXMLAutoStylePoolP_Impl()
{
}

// Adds stylefamily-information to sorted list

void SvXMLAutoStylePoolP_Impl::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
           const OUString& rStrPrefix,
        sal_Bool bAsFamily )
{
    // store family in a list if not already stored
    sal_uInt16 nExportFlags = GetExport().getExportFlags();
    sal_Bool bStylesOnly = (nExportFlags & EXPORT_STYLES) != 0 && (nExportFlags & EXPORT_CONTENT) == 0;

    OUString aPrefix( rStrPrefix );
    if( bStylesOnly )
    {
        aPrefix = OUString( 'M' );
        aPrefix += rStrPrefix;
    }

#if OSL_DEBUG_LEVEL > 0
    XMLFamilyData_Impl aTemporary( nFamily );
    FamilyListType::iterator aFind = maFamilyList.find(aTemporary);
    if( aFind != maFamilyList.end() )
    {
        // FIXME: do we really intend to replace the previous nFamily
        // entry in this case ?
        SAL_WARN_IF( aFind->mxMapper != rMapper, "xmloff",
                     "Adding duplicate family " << rStrName <<
                     " with mismatching mapper ! " <<
                     typeid(*aFind->mxMapper.get()).name() << " " <<
                     typeid(*rMapper.get()).name() );
    }
#endif

    XMLFamilyData_Impl *pFamily = new XMLFamilyData_Impl( nFamily, rStrName, rMapper, aPrefix, bAsFamily );
    maFamilyList.insert(pFamily);
}

void SvXMLAutoStylePoolP_Impl::SetFamilyPropSetMapper(
        sal_Int32 nFamily,
        const UniReference < SvXMLExportPropertyMapper > & rMapper )
{

    XMLFamilyData_Impl aTemporary( nFamily );
    FamilyListType::iterator aFind = maFamilyList.find(aTemporary);
    if (aFind != maFamilyList.end())
        aFind->mxMapper = rMapper;
}

// Adds a name to list
void SvXMLAutoStylePoolP_Impl::RegisterName( sal_Int32 nFamily, const OUString& rName )
{
    XMLFamilyData_Impl aTmp( nFamily );
    FamilyListType::iterator aFind = maFamilyList.find(aTmp);
    DBG_ASSERT( aFind != maFamilyList.end(),
                "SvXMLAutoStylePool_Impl::RegisterName: unknown family" );
    if (aFind != maFamilyList.end())
        aFind->mpNameList->insert(rName);
}

//
// Retrieve the list of registered names
//

void SvXMLAutoStylePoolP_Impl::GetRegisteredNames(
    uno::Sequence<sal_Int32>& rFamilies,
    uno::Sequence<OUString>& rNames )
{
    // collect registered names + families
    vector<sal_Int32> aFamilies;
    vector<OUString> aNames;

    // iterate over families
    for (FamilyListType::iterator aJ = maFamilyList.begin(); aJ != maFamilyList.end(); ++aJ)
    {
        XMLFamilyData_Impl &rFamily = *aJ;

        // iterate over names
        SvXMLAutoStylePoolNamesP_Impl* pNames = rFamily.mpNameList;
        if (!pNames)
            continue;
        for (SvXMLAutoStylePoolNamesP_Impl::const_iterator aI = pNames->begin(); aI != pNames->end(); ++aI)
        {
            aFamilies.push_back( rFamily.mnFamily );
            aNames.push_back( *aI );
        }
    }

    // copy the families + names into the sequence types
    DBG_ASSERT( aFamilies.size() == aNames.size(), "families != names" );

    rFamilies.realloc( aFamilies.size() );
    std::copy( aFamilies.begin(), aFamilies.end(), rFamilies.getArray() );

    rNames.realloc( aNames.size() );
    std::copy( aNames.begin(), aNames.end(), rNames.getArray() );
}

// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.

bool SvXMLAutoStylePoolP_Impl::Add(
    OUString& rName, sal_Int32 nFamily, const OUString& rParentName,
    const ::std::vector< XMLPropertyState >& rProperties, bool bDontSeek )
{
    XMLFamilyData_Impl aTemporary( nFamily );
    FamilyListType::iterator aFind = maFamilyList.find(aTemporary);
    DBG_ASSERT(aFind != maFamilyList.end(), "SvXMLAutoStylePool_Impl::Add: unknown family");

    if (aFind == maFamilyList.end())
        return false;

    XMLFamilyData_Impl &rFamily = *aFind;

    SvXMLAutoStylePoolParentP_Impl aTmp(rParentName);
    XMLFamilyData_Impl::ParentsType::iterator it2 = rFamily.maParents.find(aTmp);
    if (it2 == rFamily.maParents.end())
    {
        std::pair<XMLFamilyData_Impl::ParentsType::iterator,bool> r =
            rFamily.maParents.insert(new SvXMLAutoStylePoolParentP_Impl(rParentName));
        it2 = r.first;
    }

    SvXMLAutoStylePoolParentP_Impl& rParent = *it2;

    bool bRet = false;
    if (rParent.Add(rFamily, rProperties, rName, bDontSeek))
    {
        rFamily.mnCount++;
        bRet = true;
    }

    return bRet;
}

bool SvXMLAutoStylePoolP_Impl::AddNamed(
    const OUString& rName, sal_Int32 nFamily, const OUString& rParentName,
    const ::std::vector< XMLPropertyState >& rProperties )
{
    // get family and parent the same way as in Add()

    XMLFamilyData_Impl aTemporary( nFamily );
    FamilyListType::iterator aFind = maFamilyList.find(aTemporary);
    DBG_ASSERT(aFind != maFamilyList.end(), "SvXMLAutoStylePool_Impl::Add: unknown family");

    if (aFind == maFamilyList.end())
        return false;

    XMLFamilyData_Impl &rFamily = *aFind;

    SvXMLAutoStylePoolParentP_Impl aTmp(rParentName);
    XMLFamilyData_Impl::ParentsType::iterator it2 = rFamily.maParents.find(aTmp);
    if (it2 == rFamily.maParents.end())
    {
        std::pair<XMLFamilyData_Impl::ParentsType::iterator,bool> r =
            rFamily.maParents.insert(new SvXMLAutoStylePoolParentP_Impl(rParentName));
        it2 = r.first;
    }

    SvXMLAutoStylePoolParentP_Impl& rParent = *it2;

    bool bRet = false;
    if (rParent.AddNamed(rFamily, rProperties, rName))
    {
        rFamily.mnCount++;
        bRet = true;
    }

    return bRet;
}

//
// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list
//

OUString SvXMLAutoStylePoolP_Impl::Find( sal_Int32 nFamily,
                                         const OUString& rParent,
                                         const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;

    XMLFamilyData_Impl aTemporary( nFamily );
    FamilyListType::const_iterator const iter = maFamilyList.find(aTemporary);
    OSL_ENSURE(iter != maFamilyList.end(), "SvXMLAutoStylePool_Impl::Find: unknown family");

    if (iter != maFamilyList.end())
    {
        XMLFamilyData_Impl const& rFamily = *iter;
        SvXMLAutoStylePoolParentP_Impl aTmp( rParent );
        XMLFamilyData_Impl::ParentsType::const_iterator it2 = rFamily.maParents.find(aTmp);
        if (it2 != rFamily.maParents.end())
        {
            sName = it2->Find(rFamily, rProperties);
        }
    }

    return sName;
}

//
// export
//

void SvXMLAutoStylePoolP_Impl::exportXML(
           sal_Int32 nFamily,
        const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > &,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&,
        const SvXMLAutoStylePoolP *pAntiImpl) const
{
    // Get list of parents for current family (nFamily)
    XMLFamilyData_Impl aTmp( nFamily );
    FamilyListType::const_iterator aFind = maFamilyList.find(aTmp);
    DBG_ASSERT( aFind != maFamilyList.end(),
                "SvXMLAutoStylePool_Impl::exportXML: unknown family" );
    if (aFind == maFamilyList.end())
        return;

    const XMLFamilyData_Impl &rFamily = *aFind;
    sal_uInt32 nCount = rFamily.mnCount;

    if (!nCount)
        return;

    // create, initialize and fill helper-structure (SvXMLAutoStylePoolProperties_Impl)
    // which contains a parent-name and a SvXMLAutoStylePoolProperties_Impl
    //
    SvXMLAutoStylePoolPExport_Impl* aExpStyles =
        new SvXMLAutoStylePoolPExport_Impl[nCount];

    sal_uInt32 i;
    for( i=0; i < nCount; i++ )
    {
        aExpStyles[i].mpParent = 0;
        aExpStyles[i].mpProperties = 0;
    }

    XMLFamilyData_Impl::ParentsType::const_iterator it = rFamily.maParents.begin(), itEnd = rFamily.maParents.end();
    for (; it != itEnd; ++it)
    {
        const SvXMLAutoStylePoolParentP_Impl& rParent = *it;
        size_t nProperties = rParent.GetPropertiesList().size();
        for( size_t j = 0; j < nProperties; j++ )
        {
            const SvXMLAutoStylePoolPropertiesP_Impl* pProperties =
                rParent.GetPropertiesList()[ j ];
            sal_uLong nPos = pProperties->GetPos();
            DBG_ASSERT( nPos < nCount,
                    "SvXMLAutoStylePool_Impl::exportXML: wrong position" );
            if( nPos < nCount )
            {
                DBG_ASSERT( !aExpStyles[nPos].mpProperties,
                    "SvXMLAutoStylePool_Impl::exportXML: double position" );
                aExpStyles[nPos].mpProperties = pProperties;
                aExpStyles[nPos].mpParent = &rParent.GetParent();
            }
        }
    }

    //
    // create string to export for each XML-style. That means for each property-list
    //
    OUString aStrFamilyName = rFamily.maStrFamilyName;

    for( i=0; i<nCount; i++ )
    {
        DBG_ASSERT( aExpStyles[i].mpProperties,
                    "SvXMLAutoStylePool_Impl::exportXML: empty position" );

        if( aExpStyles[i].mpProperties )
        {
            GetExport().AddAttribute(
                XML_NAMESPACE_STYLE, XML_NAME,
                aExpStyles[i].mpProperties->GetName() );

            if( rFamily.mbAsFamily )
            {
                GetExport().AddAttribute(
                    XML_NAMESPACE_STYLE, XML_FAMILY, aStrFamilyName );
            }

            if( !aExpStyles[i].mpParent->isEmpty() )
            {
                GetExport().AddAttribute(
                    XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME,
                    GetExport().EncodeStyleName(
                        *aExpStyles[i].mpParent ) );
            }

            OUString sName;
            if( rFamily.mbAsFamily )
                sName = GetXMLToken(XML_STYLE);
            else
                sName = rFamily.maStrFamilyName;

            pAntiImpl->exportStyleAttributes(
                GetExport().GetAttrList(),
                nFamily,
                aExpStyles[i].mpProperties->GetProperties(),
                *rFamily.mxMapper.get()
                    , GetExport().GetMM100UnitConverter(),
                    GetExport().GetNamespaceMap()
                );

            SvXMLElementExport aElem( GetExport(),
                                      XML_NAMESPACE_STYLE, sName,
                                      sal_True, sal_True );

            sal_Int32 nStart(-1);
            sal_Int32 nEnd(-1);
            if (nFamily == XML_STYLE_FAMILY_PAGE_MASTER)
            {
                nStart = 0;
                sal_Int32 nIndex = 0;
                UniReference< XMLPropertySetMapper > aPropMapper =
                    rFamily.mxMapper->getPropertySetMapper();
                sal_Int16 nContextID;
                while(nIndex < aPropMapper->GetEntryCount() && nEnd == -1)
                {
                    nContextID = aPropMapper->GetEntryContextId( nIndex );
                    if (nContextID && ((nContextID & CTF_PM_FLAGMASK) != XML_PM_CTF_START))
                        nEnd = nIndex;
                    nIndex++;
                }
                if (nEnd == -1)
                    nEnd = nIndex;
            }

            rFamily.mxMapper->exportXML(
                GetExport(),
                aExpStyles[i].mpProperties->GetProperties(),
                nStart, nEnd, XML_EXPORT_FLAG_IGN_WS );

            pAntiImpl->exportStyleContent(
                GetExport().GetDocHandler(),
                nFamily,
                aExpStyles[i].mpProperties->GetProperties(),
                *rFamily.mxMapper.get(),
                GetExport().GetMM100UnitConverter(),
                GetExport().GetNamespaceMap()
                );
        }
    }

    delete[] aExpStyles;
}

void SvXMLAutoStylePoolP_Impl::ClearEntries()
{
    for (FamilyListType::iterator aI = maFamilyList.begin(); aI != maFamilyList.end(); ++aI)
        aI->ClearEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
