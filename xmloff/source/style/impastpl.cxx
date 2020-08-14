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

#include <memory>
#include <algorithm>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/solar.h>
#include <PageMasterStyleMap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmltoken.hxx>

#include "impastpl.hxx"

using namespace ::std;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// Class XMLAutoStyleFamily
// ctor/dtor class XMLAutoStyleFamily

XMLAutoStyleFamily::XMLAutoStyleFamily(
        XmlStyleFamily nFamily,
        const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > &rMapper,
        const OUString& rStrPrefix,
        bool bAsFamily ) :
    mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ),
    mnCount( 0 ), mnName( 0 ), maStrPrefix( rStrPrefix ), mbAsFamily( bAsFamily )
{}

XMLAutoStyleFamily::XMLAutoStyleFamily( XmlStyleFamily nFamily ) :
    mnFamily(nFamily), mnCount(0), mnName(0), mbAsFamily(false) {}

void XMLAutoStyleFamily::ClearEntries()
{
    m_ParentSet.clear();
}

static OUString
data2string(void *data,
            const typelib_TypeDescriptionReference *type);

static OUString
struct2string(void *data,
              const typelib_TypeDescription *type)
{
    assert(type->eTypeClass == typelib_TypeClass_STRUCT);

    OUStringBuffer result;

    result.append("{");

    const typelib_CompoundTypeDescription *compoundType =
        &reinterpret_cast<const typelib_StructTypeDescription*>(type)->aBase;

    for (int i = 0; i < compoundType->nMembers; i++)
    {
        if (i > 0)
            result.append(":");
        result.append(compoundType->ppMemberNames[i]);
        result.append("=");
        result.append(data2string(static_cast<char *>(data)+compoundType->pMemberOffsets[i],
                                  compoundType->ppTypeRefs[i]));
    }

    result.append("}");

    return result.makeStringAndClear();
}

static OUString
data2string(void *data,
            const typelib_TypeDescriptionReference *type)
{
    switch (type->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        return "";
    case typelib_TypeClass_BOOLEAN:
        return *static_cast<const sal_Bool*>(data) ? OUString("true") : OUString("false");
    case typelib_TypeClass_BYTE:
        return OUString::number((*static_cast<const sal_Int8*>(data)));
    case typelib_TypeClass_SHORT:
        return OUString::number((*static_cast<const sal_Int16*>(data)));
    case typelib_TypeClass_LONG:
        return OUString::number((*static_cast<const sal_Int32*>(data)));
    case typelib_TypeClass_HYPER:
        return OUString::number((*static_cast<const sal_Int64*>(data)));
    case typelib_TypeClass_UNSIGNED_SHORT:
        return OUString::number((*static_cast<const sal_uInt16*>(data)));
    case typelib_TypeClass_UNSIGNED_LONG:
        return OUString::number((*static_cast<const sal_uInt32*>(data)), 16);
    case typelib_TypeClass_UNSIGNED_HYPER:
        return OUString::number((*static_cast<const sal_uInt64*>(data)), 16);
    case typelib_TypeClass_FLOAT:
        return OUString::number(*static_cast<const float*>(data));
    case typelib_TypeClass_DOUBLE:
        return OUString::number(*static_cast<const double*>(data));
    case typelib_TypeClass_CHAR:
        return ("U+" + OUString::number((*static_cast<const sal_uInt16*>(data))));
    case typelib_TypeClass_STRING:
        return *static_cast<OUString*>(data);
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_INTERFACE:
        return "wtf";
    case typelib_TypeClass_STRUCT:
        return struct2string(data, type->pType);
    case typelib_TypeClass_ENUM:
        return OUString::number((*static_cast<const sal_Int32*>(data)));
    default:
        assert(false); // this cannot happen I hope
        break;
    }
    return "";
}

static OUString any2string(const uno::Any& any)
{
    return data2string(const_cast<void*>(any.getValue()), any.pType);
}

// Class SvXMLAutoStylePoolProperties_Impl
// ctor class SvXMLAutoStylePoolProperties_Impl

XMLAutoStylePoolProperties::XMLAutoStylePoolProperties( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties, OUString const & rParentName )
: maProperties( rProperties ),
  mnPos       ( rFamilyData.mnCount )
{
    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

    if (bHack)
    {
        OUStringBuffer aStemBuffer(32);
        aStemBuffer.append( rFamilyData.maStrPrefix );

        if (!rParentName.isEmpty())
            {
                aStemBuffer.append("-");
                aStemBuffer.append(rParentName);
            }

        // Create a name based on the properties used
        for(XMLPropertyState const & rState : maProperties)
            {
                if (rState.mnIndex == -1)
                    continue;
                OUString sXMLName(rFamilyData.mxMapper->getPropertySetMapper()->GetEntryXMLName(rState.mnIndex));
                if (sXMLName.isEmpty())
                    continue;
                aStemBuffer.append("-");
                aStemBuffer.append(OUString::number(rFamilyData.mxMapper->getPropertySetMapper()->GetEntryNameSpace(rState.mnIndex)));
                aStemBuffer.append(":");
                aStemBuffer.append(sXMLName);
                aStemBuffer.append("=");
                aStemBuffer.append(any2string(rState.maValue));
            }

#if 0
        // Finally append an incremental counter in an attempt to make identical
        // styles always come out in the same order. Will see if this works.
        aStemBuffer.append("-z");
        static sal_Int32 nCounter = 0;
        aStemBuffer.append(OUString::number(nCounter++));
#endif

        // create a name that hasn't been used before. The created name has not
        // to be added to the array, because it will never tried again
        OUStringBuffer aTry( aStemBuffer );

        msName = aTry.makeStringAndClear();
        bool bWarned = false;
        while (rFamilyData.maNameSet.find(msName) !=
               rFamilyData.maNameSet.end())
        {
            if (!bWarned)
                SAL_WARN("xmloff", "Overlapping style name for " << msName);
            bWarned = true;
            rFamilyData.mnName++;
            aTry.append( aStemBuffer );
            aTry.append( "-" );
            aTry.append( OUString::number( rFamilyData.mnName ) );
            msName = aTry.makeStringAndClear();
        }
        rFamilyData.maNameSet.insert(msName);
    }
    else
    {
        // create a name that hasn't been used before. The created name has not
        // to be added to the array, because it will never tried again
        OUStringBuffer sBuffer( 7 );
        do
        {
            rFamilyData.mnName++;
            sBuffer.append( rFamilyData.maStrPrefix );
            sBuffer.append( OUString::number( rFamilyData.mnName ) );
            msName = sBuffer.makeStringAndClear();
        }
        while (rFamilyData.maNameSet.find(msName) != rFamilyData.maNameSet.end() || rFamilyData.maReservedNameSet.find(msName) != rFamilyData.maReservedNameSet.end());
    }

#if OSL_DEBUG_LEVEL > 0
    std::set<sal_Int32> DebugProperties;
    for (XMLPropertyState const & rPropState : maProperties)
    {
        sal_Int32 const property(rPropState.mnIndex);
        // serious bug: will cause duplicate attributes to be exported
        assert(DebugProperties.find(property) == DebugProperties.end());
        if (-1 != property)
        {
            DebugProperties.insert(property);
        }
    }
#endif
}

bool operator<( const XMLAutoStyleFamily& r1, const XMLAutoStyleFamily& r2)
{
    return r1.mnFamily < r2.mnFamily;
}


XMLAutoStylePoolParent::~XMLAutoStylePoolParent()
{
}

namespace {

struct ComparePartial
{
    const XMLAutoStyleFamily& rFamilyData;

    bool operator()(const vector< XMLPropertyState >& lhs,
                    const std::unique_ptr<XMLAutoStylePoolProperties>& rhs) const
    {
        return rFamilyData.mxMapper->LessPartial(lhs, rhs->GetProperties());
    }
    bool operator()(const std::unique_ptr<XMLAutoStylePoolProperties>& lhs,
                    const vector< XMLPropertyState >& rhs ) const
    {
        return rFamilyData.mxMapper->LessPartial(lhs->GetProperties(), rhs);
    }
};

}

// Adds an array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.

bool XMLAutoStylePoolParent::Add( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek )
{
    XMLAutoStylePoolProperties *pProperties = nullptr;
    auto [itBegin, itEnd] = std::equal_range(m_PropertiesList.begin(), m_PropertiesList.end(), rProperties, ComparePartial{rFamilyData});
    if (!bDontSeek)
        for (auto it = itBegin; it != itEnd; ++it)
            if (rFamilyData.mxMapper->Equals((*it)->GetProperties(), rProperties))
                pProperties = it->get();

    bool bAdded = false;
    if( bDontSeek || !pProperties )
    {
        pProperties = new XMLAutoStylePoolProperties( rFamilyData, rProperties, msParent );
        m_PropertiesList.insert(itBegin, std::unique_ptr<XMLAutoStylePoolProperties>(pProperties));
        bAdded = true;
    }

    rName = pProperties->GetName();

    return bAdded;
}


// Adds an array of XMLPropertyState ( vector< XMLPropertyState > ) with a given name.
// If the name exists already, nothing is done. If a style with a different name and
// the same properties exists, a new one is added (like with bDontSeek).


bool XMLAutoStylePoolParent::AddNamed( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties, const OUString& rName )
{
    if (rFamilyData.maNameSet.find(rName) != rFamilyData.maNameSet.end())
        return false;
    
    auto it = std::lower_bound(m_PropertiesList.begin(), m_PropertiesList.end(), rProperties, ComparePartial{rFamilyData});

    std::unique_ptr<XMLAutoStylePoolProperties> pProperties(
        new XMLAutoStylePoolProperties(rFamilyData, rProperties, msParent));
    // ignore the generated name
    pProperties->SetName( rName );
    m_PropertiesList.insert(it, std::move(pProperties));
    return true;
}


// Search for an array of XMLPropertyState ( vector< XMLPropertyState > ) in list


OUString XMLAutoStylePoolParent::Find( const XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;
    auto [itBegin,itEnd] = std::equal_range(m_PropertiesList.begin(), m_PropertiesList.end(), rProperties, ComparePartial{rFamilyData});
    for (auto it = itBegin; it != itEnd; ++it)
        if (rFamilyData.mxMapper->Equals((*it)->GetProperties(), rProperties))
            sName = (*it)->GetName();

    return sName;
}

bool XMLAutoStylePoolParent::operator< (const XMLAutoStylePoolParent& rOther) const
{
    return msParent < rOther.msParent;
}

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
        XmlStyleFamily nFamily,
        const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper,
           const OUString& rStrPrefix,
        bool bAsFamily )
{
    // store family in a list if not already stored
    SvXMLExportFlags nExportFlags = GetExport().getExportFlags();
    bool bStylesOnly = (nExportFlags & SvXMLExportFlags::STYLES) && !(nExportFlags & SvXMLExportFlags::CONTENT);

    OUString aPrefix( rStrPrefix );
    if( bStylesOnly )
    {
        aPrefix = "M" + rStrPrefix;
    }

#if OSL_DEBUG_LEVEL > 0
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    if (iter != m_FamilySet.end())
    {
        // FIXME: do we really intend to replace the previous nFamily
        // entry in this case ?
        SAL_WARN_IF( (*iter)->mxMapper != rMapper, "xmloff",
                     "Adding duplicate family " << rStrName <<
                     " with mismatching mapper ! " <<
                     typeid((*iter)->mxMapper.get()).name() << " " <<
                     typeid(*rMapper).name() );
    }
#endif

    m_FamilySet.insert(std::make_unique<XMLAutoStyleFamily>(nFamily, rStrName, rMapper, aPrefix, bAsFamily));
}

void SvXMLAutoStylePoolP_Impl::SetFamilyPropSetMapper(
        XmlStyleFamily nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    if (iter != m_FamilySet.end())
        (*iter)->mxMapper = rMapper;
}

// Adds a name to list
void SvXMLAutoStylePoolP_Impl::RegisterName( XmlStyleFamily nFamily, const OUString& rName )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known
    // SAL_DEBUG("SvXMLAutoStylePoolP_Impl::RegisterName: " << nFamily << ", '" << rName << "'");
    (*iter)->maNameSet.insert(rName);
}

// Adds a name to list
void SvXMLAutoStylePoolP_Impl::RegisterDefinedName( XmlStyleFamily nFamily, const OUString& rName )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known
    (*iter)->maReservedNameSet.insert(rName);
}


// Retrieve the list of registered names


void SvXMLAutoStylePoolP_Impl::GetRegisteredNames(
    uno::Sequence<sal_Int32>& rFamilies,
    uno::Sequence<OUString>& rNames )
{
    // collect registered names + families
    vector<sal_Int32> aFamilies;
    vector<OUString> aNames;

    // iterate over families
    for (auto const& aJ : m_FamilySet)
    {
        XMLAutoStyleFamily &rFamily = *aJ;

        // iterate over names
        for (const auto& rName : rFamily.maNameSet)
        {
            aFamilies.push_back( static_cast<sal_Int32>(rFamily.mnFamily) );
            aNames.push_back( rName );
        }
    }

    // copy the families + names into the sequence types
    assert(aFamilies.size() == aNames.size());

    rFamilies.realloc( aFamilies.size() );
    std::copy( aFamilies.begin(), aFamilies.end(), rFamilies.getArray() );

    rNames.realloc( aNames.size() );
    std::copy( aNames.begin(), aNames.end(), rNames.getArray() );
}

// Adds an array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.

bool SvXMLAutoStylePoolP_Impl::Add(
    OUString& rName, XmlStyleFamily nFamily, const OUString& rParentName,
    const ::std::vector< XMLPropertyState >& rProperties, bool bDontSeek )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known

    XMLAutoStyleFamily &rFamily = **iter;

    auto itPair = rFamily.m_ParentSet.insert(std::make_unique<XMLAutoStylePoolParent>(
                        rParentName));
    XMLAutoStylePoolParent& rParent = **itPair.first;

    bool bRet = false;
    if (rParent.Add(rFamily, rProperties, rName, bDontSeek))
    {
        rFamily.mnCount++;
        bRet = true;
    }

    return bRet;
}

bool SvXMLAutoStylePoolP_Impl::AddNamed(
    const OUString& rName, XmlStyleFamily nFamily, const OUString& rParentName,
    const ::std::vector< XMLPropertyState >& rProperties )
{
    // get family and parent the same way as in Add()

    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end());  // family must be known

    XMLAutoStyleFamily &rFamily = **iter;

    auto itPair = rFamily.m_ParentSet.insert(std::make_unique<XMLAutoStylePoolParent>(
                        rParentName));
    XMLAutoStylePoolParent& rParent = **itPair.first;

    bool bRet = false;
    if (rParent.AddNamed(rFamily, rProperties, rName))
    {
        rFamily.mnCount++;
        bRet = true;
    }

    return bRet;
}


// Search for an array of XMLPropertyState ( vector< XMLPropertyState > ) in list


OUString SvXMLAutoStylePoolP_Impl::Find( XmlStyleFamily nFamily,
                                         const OUString& rParent,
                                         const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;

    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known

    XMLAutoStyleFamily const& rFamily = **iter;
    std::unique_ptr<XMLAutoStylePoolParent> pTmp(new XMLAutoStylePoolParent(rParent));
    auto const it2 = rFamily.m_ParentSet.find(pTmp);
    if (it2 != rFamily.m_ParentSet.end())
    {
        sName = (*it2)->Find(rFamily, rProperties);
    }

    return sName;
}

std::vector<xmloff::AutoStyleEntry> SvXMLAutoStylePoolP_Impl::GetAutoStyleEntries() const
{
    std::vector<xmloff::AutoStyleEntry> rReturnVector;

    for (std::unique_ptr<XMLAutoStyleFamily> const & rFamily : m_FamilySet)
    {
        rtl::Reference<XMLPropertySetMapper> aPropertyMapper = rFamily->mxMapper->getPropertySetMapper();
        for (auto const & rParent : rFamily->m_ParentSet)
        {
            for (auto const & rProperty : rParent->GetPropertiesList())
            {
                rReturnVector.emplace_back();
                xmloff::AutoStyleEntry & rEntry = rReturnVector.back();
                for (XMLPropertyState const & rPropertyState : rProperty->GetProperties())
                {
                    if (rPropertyState.mnIndex >= 0)
                    {
                        OUString sXmlName = aPropertyMapper->GetEntryXMLName(rPropertyState.mnIndex);
                        rEntry.m_aXmlProperties.emplace_back(sXmlName, rPropertyState.maValue);
                    }
                }
            }
        }
    }
    return rReturnVector;
}

namespace {

struct AutoStylePoolExport
{
    const OUString* mpParent;
    XMLAutoStylePoolProperties* mpProperties;

    AutoStylePoolExport() : mpParent(nullptr), mpProperties(nullptr) {}
};

struct StyleComparator
{
    bool operator() (const AutoStylePoolExport& a, const AutoStylePoolExport& b)
    {
        return (a.mpProperties->GetName() < b.mpProperties->GetName() ||
                (a.mpProperties->GetName() == b.mpProperties->GetName() && *a.mpParent < *b.mpParent));
    }
};

}

void SvXMLAutoStylePoolP_Impl::exportXML(
        XmlStyleFamily nFamily,
        const SvXMLAutoStylePoolP *pAntiImpl) const
{
    // Get list of parents for current family (nFamily)
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known

    const XMLAutoStyleFamily &rFamily = **iter;
    sal_uInt32 nCount = rFamily.mnCount;

    if (!nCount)
        return;

    // create, initialize and fill helper-structure (SvXMLAutoStylePoolProperties_Impl)
    // which contains a parent-name and a SvXMLAutoStylePoolProperties_Impl
    std::vector<AutoStylePoolExport> aExpStyles(nCount);

    for (auto const& it : rFamily.m_ParentSet)
    {
        XMLAutoStylePoolParent& rParent = *it;
        size_t nProperties = rParent.GetPropertiesList().size();
        for( size_t j = 0; j < nProperties; j++ )
        {
            XMLAutoStylePoolProperties *const pProperties =
                rParent.GetPropertiesList()[j].get();
            sal_uInt32 nPos = pProperties->GetPos();
            assert(nPos < nCount);
            assert(!aExpStyles[nPos].mpProperties);
            aExpStyles[nPos].mpProperties = pProperties;
            aExpStyles[nPos].mpParent = &rParent.GetParent();
        }
    }

    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

    if (bHack)
    {

        std::sort(aExpStyles.begin(), aExpStyles.end(), StyleComparator());

        for (size_t i = 0; i < nCount; i++)
        {
            OUString oldName = aExpStyles[i].mpProperties->GetName();
            sal_Int32 dashIx = oldName.indexOf('-');
            OUString newName = (dashIx > 0 ? oldName.copy(0, dashIx) : oldName) + OUString::number(i);
            // SAL_DEBUG("renaming '" << oldName << "' -> '" << newName << "'");
            aExpStyles[i].mpProperties->SetName(newName);
        }
    }


    // create string to export for each XML-style. That means for each property-list

    OUString aStrFamilyName = rFamily.maStrFamilyName;

    for( size_t i = 0; i < nCount; i++ )
    {
        assert(aExpStyles[i].mpProperties);

        if( aExpStyles[i].mpProperties )
        {
            GetExport().AddAttribute(
                XML_NAMESPACE_STYLE, XML_NAME,
                aExpStyles[i].mpProperties->GetName() );

            bool bExtensionNamespace = false;
            if( rFamily.mbAsFamily )
            {
                GetExport().AddAttribute(
                    XML_NAMESPACE_STYLE, XML_FAMILY, aStrFamilyName );
                if(aStrFamilyName != "graphic" &&
                        aStrFamilyName != "drawing-page" &&
                        aStrFamilyName != "presentation" &&
                        aStrFamilyName != "chart" )
                    bExtensionNamespace = true;
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

            pAntiImpl->exportStyleAttributes(GetExport().GetAttrList(), nFamily,
                                             aExpStyles[i].mpProperties->GetProperties(),
                                             *rFamily.mxMapper, GetExport().GetMM100UnitConverter(),
                                             GetExport().GetNamespaceMap());

            SvXMLElementExport aElem( GetExport(),
                                      XML_NAMESPACE_STYLE, sName,
                                      true, true );

            sal_Int32 nStart(-1);
            sal_Int32 nEnd(-1);
            if (nFamily == XmlStyleFamily::PAGE_MASTER)
            {
                nStart = 0;
                sal_Int32 nIndex = 0;
                rtl::Reference< XMLPropertySetMapper > aPropMapper =
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
                nStart, nEnd, SvXmlExportFlags::IGN_WS, bExtensionNamespace );

            pAntiImpl->exportStyleContent(GetExport().GetDocHandler(), nFamily,
                                          aExpStyles[i].mpProperties->GetProperties(),
                                          *rFamily.mxMapper, GetExport().GetMM100UnitConverter(),
                                          GetExport().GetNamespaceMap());
        }
    }
}

void SvXMLAutoStylePoolP_Impl::ClearEntries()
{
    for (auto & aI : m_FamilySet)
        aI->ClearEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
