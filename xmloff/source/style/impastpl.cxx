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
#include <xmloff/PageMasterStyleMap.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/families.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmltoken.hxx>

#include "impastpl.hxx"
#include <o3tl/make_unique.hxx>
using namespace ::std;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// Class XMLAutoStyleFamily
// ctor/dtor class XMLAutoStyleFamily

XMLAutoStyleFamily::XMLAutoStyleFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > &rMapper,
        const OUString& rStrPrefix,
        bool bAsFamily ) :
    mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ),
    mnCount( 0 ), mnName( 0 ), maStrPrefix( rStrPrefix ), mbAsFamily( bAsFamily )
{}

XMLAutoStyleFamily::XMLAutoStyleFamily( sal_Int32 nFamily ) :
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
    OUStringBuffer result;

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        break;
    case typelib_TypeClass_BOOLEAN:
        result.append(*static_cast<const sal_Bool*>(data) ? OUString("true") : OUString("false"));
        break;
    case typelib_TypeClass_BYTE:
        result.append(OUString::number((*static_cast<const sal_Int8*>(data))));
        break;
    case typelib_TypeClass_SHORT:
        result.append(OUString::number((*static_cast<const sal_Int16*>(data))));
        break;
    case typelib_TypeClass_LONG:
        result.append(OUString::number((*static_cast<const sal_Int32*>(data))));
        break;
    case typelib_TypeClass_HYPER:
        result.append(OUString::number((*static_cast<const sal_Int64*>(data))));
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        result.append(OUString::number((*static_cast<const sal_uInt16*>(data))));
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        result.append(OUString::number((*static_cast<const sal_uInt32*>(data)), 16));
        break;
    case typelib_TypeClass_UNSIGNED_HYPER:
        result.append(OUString::number((*static_cast<const sal_uInt64*>(data)), 16));
        break;
    case typelib_TypeClass_FLOAT:
        result.append(OUString::number(*static_cast<const float*>(data)));
        break;
    case typelib_TypeClass_DOUBLE:
        result.append(OUString::number(*static_cast<const double*>(data)));
        break;
    case typelib_TypeClass_CHAR:
        result.append("U+");
        result.append(OUString::number((*static_cast<const sal_uInt16*>(data))));
        break;
    case typelib_TypeClass_STRING:
        result.append(*static_cast<OUString*>(data));
        break;
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_INTERFACE:
        result.append("wtf");
        break;
    case typelib_TypeClass_STRUCT:
        result.append(struct2string(data, type->pType));
        break;
    case typelib_TypeClass_ENUM:
        result.append(OUString::number((*static_cast<const sal_Int32*>(data))));
        break;
    default:
        assert(false); // this cannot happen I hope
        break;
    }

    return result.makeStringAndClear();
}

static OUString
any2string(uno::Any any)
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
        for(XMLPropertyState & rState : maProperties)
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
    for (XMLPropertyState & rPropState : maProperties)
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

// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.

bool XMLAutoStylePoolParent::Add( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek )
{
    bool bAdded = false;
    XMLAutoStylePoolProperties *pProperties = nullptr;
    sal_Int32 nProperties = rProperties.size();
    size_t i = 0;
    for (size_t n = m_PropertiesList.size(); i < n; ++i)
    {
        XMLAutoStylePoolProperties *const pIS = m_PropertiesList[i].get();
        if( nProperties > static_cast<sal_Int32>(pIS->GetProperties().size()) )
        {
            continue;
        }
        else if( nProperties < static_cast<sal_Int32>(pIS->GetProperties().size()) )
        {
            break;
        }
        else if( !bDontSeek && rFamilyData.mxMapper->Equals( pIS->GetProperties(), rProperties ) )
        {
            pProperties = pIS;
            break;
        }
    }

    if( !pProperties )
    {
        pProperties = new XMLAutoStylePoolProperties( rFamilyData, rProperties, msParent );
        PropertiesListType::iterator it = m_PropertiesList.begin();
        ::std::advance( it, i );
        m_PropertiesList.insert(it, std::unique_ptr<XMLAutoStylePoolProperties>(pProperties));
        bAdded = true;
    }

    rName = pProperties->GetName();

    return bAdded;
}


// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) with a given name.
// If the name exists already, nothing is done. If a style with a different name and
// the same properties exists, a new one is added (like with bDontSeek).


bool XMLAutoStylePoolParent::AddNamed( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties, const OUString& rName )
{
    bool bAdded = false;
    sal_Int32 nProperties = rProperties.size();
    size_t i = 0;
    for (size_t n = m_PropertiesList.size(); i < n; ++i)
    {
        XMLAutoStylePoolProperties *const pIS = m_PropertiesList[i].get();
        if( nProperties > static_cast<sal_Int32>(pIS->GetProperties().size()) )
        {
            continue;
        }
        else if( nProperties < static_cast<sal_Int32>(pIS->GetProperties().size()) )
        {
            break;
        }
    }

    if (rFamilyData.maNameSet.find(rName) == rFamilyData.maNameSet.end())
    {
        std::unique_ptr<XMLAutoStylePoolProperties> pProperties(
            new XMLAutoStylePoolProperties(rFamilyData, rProperties, msParent));
        // ignore the generated name
        pProperties->SetName( rName );
        PropertiesListType::iterator it = m_PropertiesList.begin();
        ::std::advance( it, i );
        m_PropertiesList.insert(it, std::move(pProperties));
        bAdded = true;
    }

    return bAdded;
}


// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list


OUString XMLAutoStylePoolParent::Find( const XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;
    vector< XMLPropertyState>::size_type nItems = rProperties.size();
    for (const auto & i : m_PropertiesList)
    {
        const XMLAutoStylePoolProperties *const pIS = i.get();
        if( nItems > pIS->GetProperties().size() )
        {
            continue;
        }
        else if( nItems < pIS->GetProperties().size() )
        {
            break;
        }
        else if( rFamilyData.mxMapper->Equals( pIS->GetProperties(), rProperties ) )
        {
            sName = pIS->GetName();
            break;
        }
    }

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
        sal_Int32 nFamily,
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
                     typeid(*rMapper.get()).name() );
    }
#endif

    m_FamilySet.insert(o3tl::make_unique<XMLAutoStyleFamily>(nFamily, rStrName, rMapper, aPrefix, bAsFamily));
}

void SvXMLAutoStylePoolP_Impl::SetFamilyPropSetMapper(
        sal_Int32 nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    if (iter != m_FamilySet.end())
        (*iter)->mxMapper = rMapper;
}

// Adds a name to list
void SvXMLAutoStylePoolP_Impl::RegisterName( sal_Int32 nFamily, const OUString& rName )
{
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known
    // SAL_DEBUG("SvXMLAutoStylePoolP_Impl::RegisterName: " << nFamily << ", '" << rName << "'");
    (*iter)->maNameSet.insert(rName);
}

// Adds a name to list
void SvXMLAutoStylePoolP_Impl::RegisterDefinedName( sal_Int32 nFamily, const OUString& rName )
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
        for (std::set<OUString>::const_iterator aI = rFamily.maNameSet.begin(); aI != rFamily.maNameSet.end(); ++aI)
        {
            aFamilies.push_back( rFamily.mnFamily );
            aNames.push_back( *aI );
        }
    }

    // copy the families + names into the sequence types
    assert(aFamilies.size() == aNames.size());

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
    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end()); // family must be known

    XMLAutoStyleFamily &rFamily = **iter;

    std::unique_ptr<XMLAutoStylePoolParent> pTmp(new XMLAutoStylePoolParent(rParentName));
    auto it2 = rFamily.m_ParentSet.find(pTmp);
    if (it2 == rFamily.m_ParentSet.end())
    {
        std::pair<XMLAutoStyleFamily::ParentSetType::iterator,bool> r =
            rFamily.m_ParentSet.insert(o3tl::make_unique<XMLAutoStylePoolParent>(
                        rParentName));
        it2 = r.first;
    }

    XMLAutoStylePoolParent& rParent = **it2;

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

    std::unique_ptr<XMLAutoStyleFamily> pTemp(new XMLAutoStyleFamily(nFamily));
    auto const iter = m_FamilySet.find(pTemp);
    assert(iter != m_FamilySet.end());  // family must be known

    XMLAutoStyleFamily &rFamily = **iter;

    std::unique_ptr<XMLAutoStylePoolParent> pTmp(new XMLAutoStylePoolParent(rParentName));
    auto it2 = rFamily.m_ParentSet.find(pTmp);
    if (it2 == rFamily.m_ParentSet.end())
    {
        std::pair<XMLAutoStyleFamily::ParentSetType::iterator,bool> r =
            rFamily.m_ParentSet.insert(o3tl::make_unique<XMLAutoStylePoolParent>(
                        rParentName));
        it2 = r.first;
    }

    XMLAutoStylePoolParent& rParent = **it2;

    bool bRet = false;
    if (rParent.AddNamed(rFamily, rProperties, rName))
    {
        rFamily.mnCount++;
        bRet = true;
    }

    return bRet;
}


// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list


OUString SvXMLAutoStylePoolP_Impl::Find( sal_Int32 nFamily,
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
                rEntry.m_aParentName = rParent->GetParent();
                rEntry.m_aName = rProperty->GetName();
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
        sal_Int32 nFamily,
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
            sal_uLong nPos = pProperties->GetPos();
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
                                      true, true );

            sal_Int32 nStart(-1);
            sal_Int32 nEnd(-1);
            if (nFamily == XML_STYLE_FAMILY_PAGE_MASTER)
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
}

void SvXMLAutoStylePoolP_Impl::ClearEntries()
{
    for (auto & aI : m_FamilySet)
        aI->ClearEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
