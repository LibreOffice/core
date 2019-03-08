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
#include <unoidx.hxx>
#include <unoidxcoll.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/memberids.h>
#include <hints.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <shellres.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <fmtcntnt.hxx>
#include <unomap.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unosection.hxx>
#include <doctxm.hxx>
#include <txttxmrk.hxx>
#include <unocrsr.hxx>
#include <unostyle.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <chpfld.hxx>
#include <editsh.hxx>
#include <SwStyleNameMapper.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <svl/itemprop.hxx>

using namespace ::com::sun::star;

/// @throws lang::IllegalArgumentException
static OUString
lcl_AnyToString(uno::Any const& rVal)
{
    OUString sRet;
    if(!(rVal >>= sRet))
    {
        throw lang::IllegalArgumentException();
    }
    return sRet;
}

/// @throws lang::IllegalArgumentException
static sal_Int16
lcl_AnyToInt16(uno::Any const& rVal)
{
    sal_Int16 nRet = 0;
    if(!(rVal >>= nRet))
    {
        throw lang::IllegalArgumentException();
    }
    return nRet;
}

/// @throws lang::IllegalArgumentException
static bool
lcl_AnyToBool(uno::Any const& rVal)
{
    bool bRet = false;
    if(!(rVal >>= bRet))
    {
        throw lang::IllegalArgumentException();
    }
    return bRet;
}

/// @throws lang::IllegalArgumentException
template<typename T>
static void lcl_AnyToBitMask(uno::Any const& rValue,
        T & rBitMask, const T nBit)
{
    rBitMask = lcl_AnyToBool(rValue)
        ? (rBitMask |  nBit)
        : (rBitMask & ~nBit);
}

template<typename T>
static void lcl_BitMaskToAny(uno::Any & o_rValue,
        const T nBitMask, const T nBit)
{
    const bool bRet(nBitMask & nBit);
    o_rValue <<= bRet;
}

static void
lcl_ReAssignTOXType(SwDoc* pDoc, SwTOXBase& rTOXBase, const OUString& rNewName)
{
    const sal_uInt16 nUserCount = pDoc->GetTOXTypeCount( TOX_USER );
    const SwTOXType* pNewType = nullptr;
    for(sal_uInt16 nUser = 0; nUser < nUserCount; nUser++)
    {
        const SwTOXType* pType = pDoc->GetTOXType( TOX_USER, nUser );
        if (pType->GetTypeName()==rNewName)
        {
            pNewType = pType;
            break;
        }
    }
    if(!pNewType)
    {
        SwTOXType aNewType(TOX_USER, rNewName);
        pNewType = pDoc->InsertTOXType( aNewType );
    }

    rTOXBase.RegisterToTOXType( *const_cast<SwTOXType*>(pNewType) );
}

static const char cUserDefined[] = "User-Defined";
static const char cUserSuffix[] = " (user)";
#define USER_LEN 12
#define USER_AND_SUFFIXLEN 19

static void lcl_ConvertTOUNameToProgrammaticName(OUString& rTmp)
{
    ShellResource* pShellRes = SwViewShell::GetShellRes();

    if(rTmp==pShellRes->aTOXUserName)
    {
        rTmp = cUserDefined;
    }
    // if the version is not English but the alternative index's name is
    // "User-Defined" a " (user)" is appended
    else if(rTmp == cUserDefined)
    {
        rTmp += cUserSuffix;
    }
}

static void
lcl_ConvertTOUNameToUserName(OUString& rTmp)
{
    ShellResource* pShellRes = SwViewShell::GetShellRes();
    if (rTmp == cUserDefined)
    {
        rTmp = pShellRes->aTOXUserName;
    }
    else if (pShellRes->aTOXUserName != cUserDefined &&
        USER_AND_SUFFIXLEN == rTmp.getLength())
    {
        //make sure that in non-English versions the " (user)" suffix is removed
        if (rTmp.matchAsciiL(cUserDefined, sizeof(cUserDefined)) &&
            rTmp.matchAsciiL(cUserSuffix, sizeof(cUserSuffix), USER_LEN))
        {
            rTmp = cUserDefined;
        }
    }
}

typedef ::cppu::WeakImplHelper
<   lang::XServiceInfo
,   container::XIndexReplace
> SwXDocumentIndexStyleAccess_Base;

class SwXDocumentIndex::StyleAccess_Impl
    : public SwXDocumentIndexStyleAccess_Base
{

private:
    /// can be destroyed threadsafely, so no UnoImplPtr here
    ::rtl::Reference<SwXDocumentIndex> m_xParent;

    virtual ~StyleAccess_Impl() override;

public:
    explicit StyleAccess_Impl(SwXDocumentIndex& rParentIdx);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL
        supportsService(const OUString& rServiceName) override;
    virtual uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XIndexReplace
    virtual void SAL_CALL
        replaceByIndex(sal_Int32 Index, const uno::Any& rElement) override;

};

typedef ::cppu::WeakImplHelper
<   lang::XServiceInfo
,   container::XIndexReplace
> SwXDocumentIndexTokenAccess_Base;

class SwXDocumentIndex::TokenAccess_Impl
    : public SwXDocumentIndexTokenAccess_Base
{

private:
    /// can be destroyed threadsafely, so no UnoImplPtr here
    ::rtl::Reference<SwXDocumentIndex> m_xParent;

    virtual ~TokenAccess_Impl() override;

public:

    explicit TokenAccess_Impl(SwXDocumentIndex& rParentIdx);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL
        supportsService(const OUString& rServiceName) override;
    virtual uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XIndexReplace
    virtual void SAL_CALL
        replaceByIndex(sal_Int32 Index, const uno::Any& rElement) override;

};

class SwDocIndexDescriptorProperties_Impl
{
private:
    std::unique_ptr<SwTOXBase> m_pTOXBase;
    OUString m_sUserTOXTypeName;

public:
    explicit SwDocIndexDescriptorProperties_Impl(SwTOXType const*const pType);

    SwTOXBase &     GetTOXBase() { return *m_pTOXBase; }
    const OUString& GetTypeName() const { return m_sUserTOXTypeName; }
    void  SetTypeName(const OUString& rSet) { m_sUserTOXTypeName = rSet; }
};

SwDocIndexDescriptorProperties_Impl::SwDocIndexDescriptorProperties_Impl(
        SwTOXType const*const pType)
{
    SwForm aForm(pType->GetType());
    m_pTOXBase.reset(new SwTOXBase(pType, aForm,
                             SwTOXElement::Mark, pType->GetTypeName()));
    if(pType->GetType() == TOX_CONTENT || pType->GetType() == TOX_USER)
    {
        m_pTOXBase->SetLevel(MAXLEVEL);
    }
    m_sUserTOXTypeName = pType->GetTypeName();
}

static sal_uInt16
lcl_TypeToPropertyMap_Index(const TOXTypes eType)
{
    switch (eType)
    {
        case TOX_INDEX:         return PROPERTY_MAP_INDEX_IDX;
        case TOX_CONTENT:       return PROPERTY_MAP_INDEX_CNTNT;
        case TOX_TABLES:        return PROPERTY_MAP_INDEX_TABLES;
        case TOX_ILLUSTRATIONS: return PROPERTY_MAP_INDEX_ILLUSTRATIONS;
        case TOX_OBJECTS:       return PROPERTY_MAP_INDEX_OBJECTS;
        case TOX_AUTHORITIES:   return PROPERTY_MAP_BIBLIOGRAPHY;
        //case TOX_USER:
        default:
            return PROPERTY_MAP_INDEX_USER;
    }
}

class SwXDocumentIndex::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OMultiTypeInterfaceContainerHelper m_Listeners;
    SfxItemPropertySet const&   m_rPropSet;
    const TOXTypes              m_eTOXType;
    bool                        m_bIsDescriptor;
    SwDoc *                     m_pDoc;
    std::unique_ptr<SwDocIndexDescriptorProperties_Impl> m_pProps;
    uno::WeakReference<container::XIndexReplace> m_wStyleAccess;
    uno::WeakReference<container::XIndexReplace> m_wTokenAccess;

    Impl(   SwDoc & rDoc,
            const TOXTypes eType,
            SwTOXBaseSection *const pBaseSection)
        : SwClient(pBaseSection ? pBaseSection->GetFormat() : nullptr)
        , m_Listeners(m_Mutex)
        , m_rPropSet(
            *aSwMapProvider.GetPropertySet(lcl_TypeToPropertyMap_Index(eType)))
        , m_eTOXType(eType)
        , m_bIsDescriptor(nullptr == pBaseSection)
        , m_pDoc(&rDoc)
        , m_pProps(m_bIsDescriptor
            ? new SwDocIndexDescriptorProperties_Impl(rDoc.GetTOXType(eType, 0))
            : nullptr)
    {
    }

    SwSectionFormat * GetSectionFormat() const {
        return static_cast<SwSectionFormat *>(
                const_cast<SwModify *>(GetRegisteredIn()));
    }

    SwTOXBase & GetTOXSectionOrThrow() const
    {
        SwSectionFormat *const pSectionFormat(GetSectionFormat());
        SwTOXBase *const pTOXSection( m_bIsDescriptor
            ?  &m_pProps->GetTOXBase()
            : (pSectionFormat
                ? static_cast<SwTOXBaseSection*>(pSectionFormat->GetSection())
                : nullptr));
        if (!pTOXSection)
        {
            throw uno::RuntimeException(
                    "SwXDocumentIndex: disposed or invalid", nullptr);
        }
        return *pTOXSection;
    }

    sal_Int32 GetFormMax() const
    {
        SwTOXBase & rSection( GetTOXSectionOrThrow() );
        return m_bIsDescriptor
            ? SwForm::GetFormMaxLevel(m_eTOXType)
            : rSection.GetTOXForm().GetFormMax();
    }
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;

};

void SwXDocumentIndex::Impl::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (GetRegisteredIn())
    {
        return; // core object still alive
    }

    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_Listeners.disposeAndClear(ev);
}

SwXDocumentIndex::SwXDocumentIndex(
        SwTOXBaseSection & rBaseSection, SwDoc & rDoc)
    : m_pImpl( new SwXDocumentIndex::Impl(
                rDoc, rBaseSection.SwTOXBase::GetType(), & rBaseSection) )
{
}

SwXDocumentIndex::SwXDocumentIndex(const TOXTypes eType, SwDoc& rDoc)
    : m_pImpl( new SwXDocumentIndex::Impl(rDoc, eType, nullptr) )
{
}

SwXDocumentIndex::~SwXDocumentIndex()
{
}

uno::Reference<text::XDocumentIndex>
SwXDocumentIndex::CreateXDocumentIndex(
        SwDoc & rDoc, SwTOXBaseSection * pSection, TOXTypes const eTypes)
{
    // re-use existing SwXDocumentIndex
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference<text::XDocumentIndex> xIndex;
    if (pSection)
    {
        SwSectionFormat const *const pFormat = pSection->GetFormat();
        xIndex.set(pFormat->GetXObject(), uno::UNO_QUERY);
    }
    if (!xIndex.is())
    {
        SwXDocumentIndex *const pIndex(pSection
                ? new SwXDocumentIndex(*pSection, rDoc)
                : new SwXDocumentIndex(eTypes, rDoc));
        xIndex.set(pIndex);
        if (pSection)
        {
            pSection->GetFormat()->SetXObject(xIndex);
        }
        // need a permanent Reference to initialize m_wThis
        pIndex->m_pImpl->m_wThis = xIndex;
    }
    return xIndex;
}

namespace
{
    class theSwXDocumentIndexUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXDocumentIndexUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXDocumentIndex::getUnoTunnelId()
{
    return theSwXDocumentIndexUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXDocumentIndex::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return ::sw::UnoTunnelImpl<SwXDocumentIndex>(rId, this);
}

OUString SAL_CALL
SwXDocumentIndex::getImplementationName()
{
    return OUString("SwXDocumentIndex");
}

sal_Bool SAL_CALL
SwXDocumentIndex::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndex::getSupportedServiceNames()
{
    SolarMutexGuard g;

    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.BaseIndex";
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_INDEX:
            pArray[1] = "com.sun.star.text.DocumentIndex";
        break;
        case TOX_CONTENT:
            pArray[1] = "com.sun.star.text.ContentIndex";
        break;
        case TOX_TABLES:
            pArray[1] = "com.sun.star.text.TableIndex";
        break;
        case TOX_ILLUSTRATIONS:
            pArray[1] = "com.sun.star.text.IllustrationsIndex";
        break;
        case TOX_OBJECTS:
            pArray[1] = "com.sun.star.text.ObjectIndex";
        break;
        case TOX_AUTHORITIES:
            pArray[1] = "com.sun.star.text.Bibliography";
        break;
        //case TOX_USER:
        default:
            pArray[1] = "com.sun.star.text.UserDefinedIndex";
    }
    return aRet;
}

OUString SAL_CALL SwXDocumentIndex::getServiceName()
{
    SolarMutexGuard g;

    SwServiceType nObjectType = SwServiceType::TypeIndex;
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_USER:          nObjectType = SwServiceType::UserIndex;
        break;
        case TOX_CONTENT:       nObjectType = SwServiceType::ContentIndex;
        break;
        case TOX_ILLUSTRATIONS: nObjectType = SwServiceType::IndexIllustrations;
        break;
        case TOX_OBJECTS:       nObjectType = SwServiceType::IndexObjects;
        break;
        case TOX_TABLES:        nObjectType = SwServiceType::IndexTables;
        break;
        case TOX_AUTHORITIES:   nObjectType = SwServiceType::IndexBibliography;
        break;
        default:
        break;
    }
    return SwXServiceProvider::GetProviderName(nObjectType);
}

void SAL_CALL SwXDocumentIndex::update()
{
    return refresh(); // update is from deprecated XDocumentIndex
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXDocumentIndex::getPropertySetInfo()
{
    SolarMutexGuard g;

    const uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXDocumentIndex::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw beans::PropertyVetoException(
            "Property is read-only: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    SwSectionFormat *const pSectionFormat(m_pImpl->GetSectionFormat());
    SwTOXBase & rTOXBase( m_pImpl->GetTOXSectionOrThrow() );

    SwTOXElement nCreate = rTOXBase.GetCreateType();
    SwTOOElements nOLEOptions = rTOXBase.GetOLEOptions();
    const TOXTypes eTxBaseType = rTOXBase.GetTOXType()->GetType();
    SwTOIOptions nTOIOptions = (eTxBaseType == TOX_INDEX)
        ? rTOXBase.GetOptions() : SwTOIOptions::NONE;
    SwForm  aForm(rTOXBase.GetTOXForm());
    bool bForm = false;
    switch (pEntry->nWID)
    {
        case WID_IDX_TITLE:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetTitle(sNewName);
        }
        break;
        case WID_IDX_NAME:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetTOXName(sNewName);
        }
        break;
        case WID_USER_IDX_NAME:
        {
            OUString sNewName;
            if (!(rValue >>= sNewName))
            {
                throw lang::IllegalArgumentException();
            }
            lcl_ConvertTOUNameToUserName(sNewName);
            OSL_ENSURE(TOX_USER == eTxBaseType,
                    "tox type name can only be changed for user indexes");
            if (pSectionFormat)
            {
                if (rTOXBase.GetTOXType()->GetTypeName() != sNewName)
                {
                    lcl_ReAssignTOXType(pSectionFormat->GetDoc(),
                            rTOXBase, sNewName);
                }
            }
            else
            {
                m_pImpl->m_pProps->SetTypeName(sNewName);
            }
        }
        break;
        case WID_IDX_LOCALE:
        {
            lang::Locale aLocale;
            if (!(rValue>>= aLocale))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetLanguage( LanguageTag::convertToLanguageType(aLocale));
        }
        break;
        case WID_IDX_SORT_ALGORITHM:
        {
            OUString sTmp;
            if (!(rValue >>= sTmp))
            {
                throw lang::IllegalArgumentException();
            }
            rTOXBase.SetSortAlgorithm(sTmp);
        }
        break;
        case WID_LEVEL:
        {
            rTOXBase.SetLevel(lcl_AnyToInt16(rValue));
        }
        break;
        case WID_TOC_BOOKMARK:
        {
           rTOXBase.SetBookmarkName(lcl_AnyToString(rValue));
           nCreate = SwTOXElement::Bookmark;
           rTOXBase.SetCreate(nCreate);
        }
        break;
        case WID_INDEX_ENTRY_TYPE:
        {
            rTOXBase.SetEntryTypeName(lcl_AnyToString(rValue));
            nCreate = SwTOXElement::IndexEntryType;
            rTOXBase.SetCreate(nCreate);
        }
        break;
        case WID_CREATE_FROM_MARKS:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Mark);
        break;
        case WID_CREATE_FROM_OUTLINE:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::OutlineLevel);
        break;
        case WID_TOC_PARAGRAPH_OUTLINE_LEVEL:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::ParagraphOutlineLevel);
        break;
        case WID_TAB_IN_TOC:
             lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::TableInToc);
        break;
        case WID_TOC_NEWLINE:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Newline);
        break;
//          case WID_PARAGRAPH_STYLE_NAMES             :OSL_FAIL("not implemented")
//          break;
        case WID_HIDE_TABLEADER_PAGENUMBERS:
              lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::TableLeader);
        break ;
        case WID_CREATE_FROM_CHAPTER:
            rTOXBase.SetFromChapter(lcl_AnyToBool(rValue));
        break;
        case WID_CREATE_FROM_LABELS:
            rTOXBase.SetFromObjectNames(! lcl_AnyToBool(rValue));
        break;
        case WID_PROTECTED:
        {
            bool bSet = lcl_AnyToBool(rValue);
            rTOXBase.SetProtected(bSet);
            if (pSectionFormat)
            {
                static_cast<SwTOXBaseSection &>(rTOXBase).SetProtect(bSet);
            }
        }
        break;
        case WID_USE_ALPHABETICAL_SEPARATORS:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    SwTOIOptions::AlphaDelimiter);
        break;
        case WID_USE_KEY_AS_ENTRY:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    SwTOIOptions::KeyAsEntry);
        break;
        case WID_USE_COMBINED_ENTRIES:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    SwTOIOptions::SameEntry);
        break;
        case WID_IS_CASE_SENSITIVE:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    SwTOIOptions::CaseSensitive);
        break;
        case WID_USE_P_P:
            lcl_AnyToBitMask(rValue, nTOIOptions, SwTOIOptions::FF);
        break;
        case WID_USE_DASH:
            lcl_AnyToBitMask(rValue, nTOIOptions, SwTOIOptions::Dash);
        break;
        case WID_USE_UPPER_CASE:
            lcl_AnyToBitMask(rValue, nTOIOptions,
                    SwTOIOptions::InitialCaps);
        break;
        case WID_IS_COMMA_SEPARATED:
            bForm = true;
            aForm.SetCommaSeparated(lcl_AnyToBool(rValue));
        break;
        case WID_LABEL_CATEGORY:
        {
            // convert file-format/API/external programmatic english name
            // to internal UI name before usage
            rTOXBase.SetSequenceName( SwStyleNameMapper::GetSpecialExtraUIName(
                                lcl_AnyToString(rValue) ) );
        }
        break;
        case WID_LABEL_DISPLAY_TYPE:
        {
            const sal_Int16 nVal = lcl_AnyToInt16(rValue);
            sal_uInt16 nSet = CAPTION_COMPLETE;
            switch (nVal)
            {
                case text::ReferenceFieldPart::TEXT:
                    nSet = CAPTION_COMPLETE;
                break;
                case text::ReferenceFieldPart::CATEGORY_AND_NUMBER:
                    nSet = CAPTION_NUMBER;
                break;
                case text::ReferenceFieldPart::ONLY_CAPTION:
                    nSet = CAPTION_TEXT;
                break;
                default:
                    throw lang::IllegalArgumentException();
            }
            rTOXBase.SetCaptionDisplay(static_cast<SwCaptionDisplay>(nSet));
        }
        break;
        case WID_USE_LEVEL_FROM_SOURCE:
            rTOXBase.SetLevelFromChapter(lcl_AnyToBool(rValue));
        break;
        case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME:
        {
            OUString aString;
            SwStyleNameMapper::FillUIName(lcl_AnyToString(rValue),
                aString, SwGetPoolIdFromName::ChrFmt);
            rTOXBase.SetMainEntryCharStyle( aString );
        }
        break;
        case WID_CREATE_FROM_TABLES:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Table);
        break;
        case WID_CREATE_FROM_TEXT_FRAMES:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Frame);
        break;
        case WID_CREATE_FROM_GRAPHIC_OBJECTS:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Graphic);
        break;
        case WID_CREATE_FROM_EMBEDDED_OBJECTS:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Ole);
        break;
        case WID_CREATE_FROM_STAR_MATH:
            lcl_AnyToBitMask(rValue, nOLEOptions, SwTOOElements::Math);
        break;
        case WID_CREATE_FROM_STAR_CHART:
            lcl_AnyToBitMask(rValue, nOLEOptions, SwTOOElements::Chart);
        break;
        case WID_CREATE_FROM_STAR_CALC:
            lcl_AnyToBitMask(rValue, nOLEOptions, SwTOOElements::Calc);
        break;
        case WID_CREATE_FROM_STAR_DRAW:
            lcl_AnyToBitMask(rValue, nOLEOptions,
                    SwTOOElements::DrawImpress);
        break;
        case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
            lcl_AnyToBitMask(rValue, nOLEOptions, SwTOOElements::Other);
        break;
        case WID_PARA_HEAD:
        {
            OUString aString;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, SwGetPoolIdFromName::TxtColl);
            bForm = true;
            // Header is on Pos 0
            aForm.SetTemplate( 0, aString );
        }
        break;
        case WID_IS_RELATIVE_TABSTOPS:
            bForm = true;
            aForm.SetRelTabPos(lcl_AnyToBool(rValue));
        break;
        case WID_PARA_SEP:
        {
            OUString aString;
            bForm = true;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, SwGetPoolIdFromName::TxtColl);
            aForm.SetTemplate( 1, aString );
        }
        break;
        case WID_CREATE_FROM_PARAGRAPH_STYLES:
            lcl_AnyToBitMask(rValue, nCreate, SwTOXElement::Template);
        break;

        case WID_PARA_LEV1:
        case WID_PARA_LEV2:
        case WID_PARA_LEV3:
        case WID_PARA_LEV4:
        case WID_PARA_LEV5:
        case WID_PARA_LEV6:
        case WID_PARA_LEV7:
        case WID_PARA_LEV8:
        case WID_PARA_LEV9:
        case WID_PARA_LEV10:
        {
            bForm = true;
            // in sdbcx::Index Label 1 begins at Pos 2 otherwise at Pos 1
            const sal_uInt16 nLPos = rTOXBase.GetType() == TOX_INDEX ? 2 : 1;
            OUString aString;
            SwStyleNameMapper::FillUIName( lcl_AnyToString(rValue),
                aString, SwGetPoolIdFromName::TxtColl);
            aForm.SetTemplate(nLPos + pEntry->nWID - WID_PARA_LEV1, aString );
        }
        break;
        default:
            //this is for items only
            if (WID_PRIMARY_KEY > pEntry->nWID)
            {
                const SwAttrSet& rSet =
                    SwDoc::GetTOXBaseAttrSet(rTOXBase);
                SfxItemSet aAttrSet(rSet);
                m_pImpl->m_rPropSet.setPropertyValue(
                        rPropertyName, rValue, aAttrSet);

                const SwSectionFormats& rSects = m_pImpl->m_pDoc->GetSections();
                for (size_t i = 0; i < rSects.size(); ++i)
                {
                    const SwSectionFormat* pTmpFormat = rSects[ i ];
                    if (pTmpFormat == pSectionFormat)
                    {
                        SwSectionData tmpData(
                            static_cast<SwTOXBaseSection&>(rTOXBase));
                        m_pImpl->m_pDoc->UpdateSection(i, tmpData, & aAttrSet);
                        break;
                    }
                }
            }
    }
    rTOXBase.SetCreate(nCreate);
    rTOXBase.SetOLEOptions(nOLEOptions);
    if (rTOXBase.GetTOXType()->GetType() == TOX_INDEX)
    {
        rTOXBase.SetOptions(nTOIOptions);
    }
    if (bForm)
    {
        rTOXBase.SetTOXForm(aForm);
    }
}

uno::Any SAL_CALL
SwXDocumentIndex::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }
    // TODO: is this the best approach to tell API clients about the change?
    if (pEntry->nWID == RES_BACKGROUND && pEntry->nMemberId == MID_GRAPHIC_URL)
    {
        throw uno::RuntimeException("Getting GraphicURL property is not supported");
    }

    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    SwTOXBase* pTOXBase = nullptr;
    if (pSectionFormat)
    {
        pTOXBase = static_cast<SwTOXBaseSection*>(pSectionFormat->GetSection());
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        pTOXBase = &m_pImpl->m_pProps->GetTOXBase();
    }
    if(pTOXBase)
    {
        const SwTOXElement nCreate = pTOXBase->GetCreateType();
        const SwTOOElements nOLEOptions = pTOXBase->GetOLEOptions();
        const SwTOIOptions nTOIOptions =
            (pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            ? pTOXBase->GetOptions()
            : SwTOIOptions::NONE;
        const SwForm& rForm = pTOXBase->GetTOXForm();
        switch(pEntry->nWID)
        {
            case WID_IDX_CONTENT_SECTION:
            case WID_IDX_HEADER_SECTION :
                if(WID_IDX_CONTENT_SECTION == pEntry->nWID)
                {
                    const uno::Reference <text::XTextSection> xContentSect =
                        SwXTextSection::CreateXTextSection( pSectionFormat );
                    aRet <<= xContentSect;
                }
                else if (pSectionFormat)
                {
                    SwSections aSectArr;
                    pSectionFormat->GetChildSections(aSectArr,
                            SectionSort::Not, false);
                    for(SwSection* pSect : aSectArr)
                    {
                        if(pSect->GetType() == TOX_HEADER_SECTION)
                        {
                            const uno::Reference <text::XTextSection> xHeader =
                                SwXTextSection::CreateXTextSection(
                                    pSect->GetFormat() );
                            aRet <<= xHeader;
                            break;
                        }
                    }
                }
            break;
            case WID_IDX_TITLE  :
            {
                aRet <<= pTOXBase->GetTitle();
                break;
            }
            case WID_IDX_NAME:
                aRet <<= pTOXBase->GetTOXName();
            break;
            case WID_USER_IDX_NAME:
            {
                OUString sTmp((!m_pImpl->m_bIsDescriptor)
                    ? pTOXBase->GetTOXType()->GetTypeName()
                    : m_pImpl->m_pProps->GetTypeName());
                //I18N
                lcl_ConvertTOUNameToProgrammaticName(sTmp);
                aRet <<= sTmp;
            }
            break;
            case WID_IDX_LOCALE:
                aRet <<= LanguageTag(pTOXBase->GetLanguage()).getLocale();
            break;
            case WID_IDX_SORT_ALGORITHM:
                aRet <<= pTOXBase->GetSortAlgorithm();
            break;
            case WID_LEVEL      :
                aRet <<= static_cast<sal_Int16>(pTOXBase->GetLevel());
            break;
            case WID_TOC_BOOKMARK  :
               aRet <<= pTOXBase->GetBookmarkName();
            break;
            case WID_INDEX_ENTRY_TYPE  :
               aRet <<= pTOXBase->GetEntryTypeName();
            break;
            case WID_CREATE_FROM_MARKS:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Mark);
            break;
            case WID_CREATE_FROM_OUTLINE:
                lcl_BitMaskToAny(aRet, nCreate,
                        SwTOXElement::OutlineLevel);
            break;
            case WID_CREATE_FROM_CHAPTER:
            {
                const bool bRet = pTOXBase->IsFromChapter();
                aRet <<= bRet;
            }
            break;
            case WID_CREATE_FROM_LABELS:
            {
                const bool bRet = ! pTOXBase->IsFromObjectNames();
                aRet <<= bRet;
            }
            break;
            case WID_PROTECTED:
            {
                const bool bRet = pTOXBase->IsProtected();
                aRet <<= bRet;
            }
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        SwTOIOptions::AlphaDelimiter);
            break;
            case WID_USE_KEY_AS_ENTRY:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        SwTOIOptions::KeyAsEntry);
            break;
            case WID_USE_COMBINED_ENTRIES:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        SwTOIOptions::SameEntry);
            break;
            case WID_IS_CASE_SENSITIVE:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        SwTOIOptions::CaseSensitive);
            break;
            case WID_USE_P_P:
                lcl_BitMaskToAny(aRet, nTOIOptions, SwTOIOptions::FF);
            break;
            case WID_USE_DASH:
                lcl_BitMaskToAny(aRet, nTOIOptions, SwTOIOptions::Dash);
            break;
            case WID_USE_UPPER_CASE:
                lcl_BitMaskToAny(aRet, nTOIOptions,
                        SwTOIOptions::InitialCaps);
            break;
            case WID_IS_COMMA_SEPARATED:
            {
                const bool bRet = rForm.IsCommaSeparated();
                aRet <<= bRet;
            }
            break;
            case WID_LABEL_CATEGORY:
            {
                // convert internal UI name to
                // file-format/API/external programmatic english name
                // before usage
                aRet <<= SwStyleNameMapper::GetSpecialExtraProgName(
                                    pTOXBase->GetSequenceName() );
            }
            break;
            case WID_LABEL_DISPLAY_TYPE:
            {
                sal_Int16 nSet = text::ReferenceFieldPart::TEXT;
                switch (pTOXBase->GetCaptionDisplay())
                {
                    case CAPTION_COMPLETE:
                        nSet = text::ReferenceFieldPart::TEXT;
                    break;
                    case CAPTION_NUMBER:
                        nSet = text::ReferenceFieldPart::CATEGORY_AND_NUMBER;
                    break;
                    case CAPTION_TEXT:
                        nSet = text::ReferenceFieldPart::ONLY_CAPTION;
                    break;
                }
                aRet <<= nSet;
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE:
            {
                const bool bRet = pTOXBase->IsLevelFromChapter();
                aRet <<= bRet;
            }
            break;
            case WID_LEVEL_FORMAT:
            {
                uno::Reference< container::XIndexReplace > xTokenAccess(
                    m_pImpl->m_wTokenAccess);
                if (!xTokenAccess.is())
                {
                    xTokenAccess = new TokenAccess_Impl(*this);
                    m_pImpl->m_wTokenAccess = xTokenAccess;
                }
                aRet <<= xTokenAccess;
            }
            break;
            case WID_LEVEL_PARAGRAPH_STYLES:
            {
                uno::Reference< container::XIndexReplace > xStyleAccess(
                    m_pImpl->m_wStyleAccess);
                if (!xStyleAccess.is())
                {
                    xStyleAccess = new StyleAccess_Impl(*this);
                    m_pImpl->m_wStyleAccess = xStyleAccess;
                }
                aRet <<= xStyleAccess;
            }
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME:
            {
                OUString aString;
                SwStyleNameMapper::FillProgName(
                        pTOXBase->GetMainEntryCharStyle(),
                        aString,
                        SwGetPoolIdFromName::ChrFmt);
                aRet <<= aString;
            }
            break;
            case WID_CREATE_FROM_TABLES:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Table);
            break;
            case WID_CREATE_FROM_TEXT_FRAMES:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Frame);
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Graphic);
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Ole);
            break;
            case WID_CREATE_FROM_STAR_MATH:
                lcl_BitMaskToAny(aRet, nOLEOptions, SwTOOElements::Math);
            break;
            case WID_CREATE_FROM_STAR_CHART:
                lcl_BitMaskToAny(aRet, nOLEOptions, SwTOOElements::Chart);
            break;
            case WID_CREATE_FROM_STAR_CALC:
                lcl_BitMaskToAny(aRet, nOLEOptions, SwTOOElements::Calc);
            break;
            case WID_CREATE_FROM_STAR_DRAW:
                lcl_BitMaskToAny(aRet, nOLEOptions,
                        SwTOOElements::DrawImpress);
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                lcl_BitMaskToAny(aRet, nOLEOptions, SwTOOElements::Other);
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                lcl_BitMaskToAny(aRet, nCreate, SwTOXElement::Template);
            break;
            case WID_PARA_HEAD:
            {
                //Header steht an Pos 0
                OUString aString;
                SwStyleNameMapper::FillProgName(rForm.GetTemplate( 0 ), aString,
                        SwGetPoolIdFromName::TxtColl );
                aRet <<= aString;
            }
            break;
            case WID_PARA_SEP:
            {
                OUString aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate( 1 ),
                        aString,
                        SwGetPoolIdFromName::TxtColl);
                aRet <<= aString;
            }
            break;
            case WID_PARA_LEV1:
            case WID_PARA_LEV2:
            case WID_PARA_LEV3:
            case WID_PARA_LEV4:
            case WID_PARA_LEV5:
            case WID_PARA_LEV6:
            case WID_PARA_LEV7:
            case WID_PARA_LEV8:
            case WID_PARA_LEV9:
            case WID_PARA_LEV10:
            {
                // in sdbcx::Index Label 1 begins at Pos 2 otherwise at Pos 1
                const sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                OUString aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate(nLPos + pEntry->nWID - WID_PARA_LEV1),
                        aString,
                        SwGetPoolIdFromName::TxtColl);
                aRet <<= aString;
            }
            break;
            case WID_IS_RELATIVE_TABSTOPS:
            {
                const bool bRet = rForm.IsRelTabPos();
                aRet <<= bRet;
            }
            break;
            case WID_INDEX_MARKS:
            {
                SwTOXMarks aMarks;
                const SwTOXType* pType = pTOXBase->GetTOXType();
                SwTOXMark::InsertTOXMarks( aMarks, *pType );
                uno::Sequence< uno::Reference<text::XDocumentIndexMark> > aXMarks(aMarks.size());
                uno::Reference<text::XDocumentIndexMark>* pxMarks = aXMarks.getArray();
                for(size_t i = 0; i < aMarks.size(); ++i)
                {
                    SwTOXMark* pMark = aMarks[i];
                    pxMarks[i] = SwXDocumentIndexMark::CreateXDocumentIndexMark(
                        *m_pImpl->m_pDoc, pMark);
                }
                aRet <<= aXMarks;
            }
            break;
            default:
                //this is for items only
                if(WID_PRIMARY_KEY > pEntry->nWID)
                {
                    const SwAttrSet& rSet =
                        SwDoc::GetTOXBaseAttrSet(*pTOXBase);
                    aRet = m_pImpl->m_rPropSet.getPropertyValue(
                            rPropertyName, rSet);
                }
        }
    }
    return aRet;
}

void SAL_CALL
SwXDocumentIndex::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndex::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndex::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndex::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndex::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndex::removeVetoableChangeListener(): not implemented");
}

static void lcl_CalcLayout(SwDoc *pDoc)
{
    SwViewShell *pViewShell = nullptr;
    SwEditShell* pEditShell = nullptr;
    if( pDoc )
    {
        pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
        pEditShell = pDoc->GetEditShell();
    }

    if (pEditShell)
    {
        pEditShell->CalcLayout();
    }
    else if (pViewShell)
    {
        pViewShell->CalcLayout();
    }
}

// XRefreshable
void SAL_CALL SwXDocumentIndex::refresh()
{
    {
        SolarMutexGuard g;

        SwSectionFormat *const pFormat = m_pImpl->GetSectionFormat();
        SwTOXBaseSection *const pTOXBase = pFormat ?
            static_cast<SwTOXBaseSection*>(pFormat->GetSection()) : nullptr;
        if (!pTOXBase)
        {
            throw uno::RuntimeException(
                    "SwXDocumentIndex::refresh: must be in attached state",
                     static_cast< ::cppu::OWeakObject*>(this));
        }
        pTOXBase->Update(nullptr, m_pImpl->m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout());

        // the insertion of TOC will affect the document layout
        lcl_CalcLayout(m_pImpl->m_pDoc);

        // page numbers
        pTOXBase->UpdatePageNum();
    }

    ::cppu::OInterfaceContainerHelper *const pContainer(
        m_pImpl->m_Listeners.getContainer(
            cppu::UnoType<util::XRefreshListener>::get()));
    if (pContainer)
    {
        lang::EventObject const event(static_cast< ::cppu::OWeakObject*>(this));
        pContainer->notifyEach(& util::XRefreshListener::refreshed, event);
    }
}

void SAL_CALL SwXDocumentIndex::addRefreshListener(
        const uno::Reference<util::XRefreshListener>& xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            cppu::UnoType<util::XRefreshListener>::get(), xListener);
}

void SAL_CALL SwXDocumentIndex::removeRefreshListener(
        const uno::Reference<util::XRefreshListener>& xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            cppu::UnoType<util::XRefreshListener>::get(), xListener);
}

void SAL_CALL
SwXDocumentIndex::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }
    const uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange *const pRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);

    SwDoc *const pDoc =
        pRange ? &pRange->GetDoc() : (pCursor ? pCursor->GetDoc() : nullptr);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);

    const SwTOXBase* pOld = SwDoc::GetCurTOX( *aPam.Start() );
    if (pOld)
    {
        throw lang::IllegalArgumentException();
    }

    UnoActionContext aAction(pDoc);

    SwTOXBase & rTOXBase = m_pImpl->m_pProps->GetTOXBase();
    SwTOXType const*const pTOXType = rTOXBase.GetTOXType();
    if ((TOX_USER == pTOXType->GetType()) &&
        m_pImpl->m_pProps->GetTypeName() != pTOXType->GetTypeName())
    {
        lcl_ReAssignTOXType(pDoc, rTOXBase, m_pImpl->m_pProps->GetTypeName());
    }
    //TODO: apply Section attributes (columns and background)
    SwTOXBaseSection *const pTOX =
        pDoc->InsertTableOf( aPam, rTOXBase, nullptr, false,
                m_pImpl->m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout());

    pDoc->SetTOXBaseName(*pTOX, m_pImpl->m_pProps->GetTOXBase().GetTOXName());

    // update page numbers
    pTOX->GetFormat()->Add(m_pImpl.get());
    pTOX->GetFormat()->SetXObject(static_cast< ::cppu::OWeakObject*>(this));
    pTOX->UpdatePageNum();

    m_pImpl->m_pProps.reset();
    m_pImpl->m_pDoc = pDoc;
    m_pImpl->m_bIsDescriptor = false;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXDocumentIndex::getAnchor()
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (!pSectionFormat)
    {
        throw uno::RuntimeException();
    }

    uno::Reference< text::XTextRange > xRet;
    SwNodeIndex const*const pIdx( pSectionFormat->GetContent().GetContentIdx() );
    if (pIdx && pIdx->GetNode().GetNodes().IsDocNodes())
    {
        SwPaM aPaM(*pIdx);
        aPaM.Move( fnMoveForward, GoInContent );
        aPaM.SetMark();
        aPaM.GetPoint()->nNode = *pIdx->GetNode().EndOfSectionNode();
        aPaM.Move( fnMoveBackward, GoInContent );
        xRet = SwXTextRange::CreateXTextRange(*pSectionFormat->GetDoc(),
            *aPaM.GetMark(), aPaM.GetPoint());
    }
    return xRet;
}

void SAL_CALL SwXDocumentIndex::dispose()
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (pSectionFormat)
    {
        pSectionFormat->GetDoc()->DeleteTOX(
            *static_cast<SwTOXBaseSection*>(pSectionFormat->GetSection()),
            true);
    }
}

void SAL_CALL
SwXDocumentIndex::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            cppu::UnoType<lang::XEventListener>::get(), xListener);
}

void SAL_CALL
SwXDocumentIndex::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            cppu::UnoType<lang::XEventListener>::get(), xListener);
}

OUString SAL_CALL SwXDocumentIndex::getName()
{
    SolarMutexGuard g;

    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (m_pImpl->m_bIsDescriptor)
    {
        return m_pImpl->m_pProps->GetTOXBase().GetTOXName();
    }

    if(!pSectionFormat)
    {
        throw uno::RuntimeException();
    }

    return pSectionFormat->GetSection()->GetSectionName();
}

void SAL_CALL
SwXDocumentIndex::setName(const OUString& rName)
{
    SolarMutexGuard g;

    if (rName.isEmpty())
    {
        throw uno::RuntimeException();
    }

    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pProps->GetTOXBase().SetTOXName(rName);
    }
    else if (pSectionFormat)
    {
        const bool bSuccess = pSectionFormat->GetDoc()->SetTOXBaseName(
            *static_cast<SwTOXBaseSection*>(pSectionFormat->GetSection()), rName);
        if (!bSuccess)
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}

// MetadatableMixin
::sfx2::Metadatable* SwXDocumentIndex::GetCoreObject()
{
    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    return pSectionFormat;
}

uno::Reference<frame::XModel> SwXDocumentIndex::GetModel()
{
    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (pSectionFormat)
    {
        SwDocShell const*const pShell( pSectionFormat->GetDoc()->GetDocShell() );
        return pShell ? pShell->GetModel() : nullptr;
    }
    return nullptr;
}

static sal_uInt16
lcl_TypeToPropertyMap_Mark(const TOXTypes eType)
{
    switch (eType)
    {
        case TOX_INDEX:         return PROPERTY_MAP_INDEX_MARK;
        case TOX_CONTENT:       return PROPERTY_MAP_CNTIDX_MARK;
        case TOX_CITATION   :   return PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY;
        //case TOX_USER:
        default:
            return PROPERTY_MAP_USER_MARK;
    }
}

class SwXDocumentIndexMark::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2
    SwXDocumentIndexMark & m_rThis;
    bool m_bInReplaceMark;

public:

    uno::WeakReference<uno::XInterface> m_wThis;
    SfxItemPropertySet const& m_rPropSet;
    const TOXTypes m_eTOXType;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    bool m_bIsDescriptor;
    sw::WriterMultiListener m_aListener;
    const SwTOXType* m_pTOXType;
    const SwTOXMark* m_pTOXMark;
    SwDoc* m_pDoc;

    bool m_bMainEntry;
    sal_uInt16 m_nLevel;
    OUString m_aBookmarkName;
    OUString m_aEntryTypeName;
    OUString m_sAltText;
    OUString m_sPrimaryKey;
    OUString m_sSecondaryKey;
    OUString m_sTextReading;
    OUString m_sPrimaryKeyReading;
    OUString m_sSecondaryKeyReading;
    OUString m_sUserIndexName;

    Impl(SwXDocumentIndexMark& rThis,
            SwDoc* const pDoc,
            const enum TOXTypes eType,
            SwTOXType* const pType, SwTOXMark const*const pMark)
        : SwClient()
        , m_rThis(rThis)
        , m_bInReplaceMark(false)
        , m_rPropSet(
            *aSwMapProvider.GetPropertySet(lcl_TypeToPropertyMap_Mark(eType)))
        , m_eTOXType(eType)
        , m_EventListeners(m_Mutex)
        , m_bIsDescriptor(nullptr == pMark)
        , m_aListener(*this)
        , m_pTOXType(pType)
        , m_pTOXMark(pMark)
        , m_pDoc(pDoc)
        , m_bMainEntry(false)
        , m_nLevel(0)
    {
        m_aListener.StartListening(const_cast<SwTOXMark*>(pMark));
        m_aListener.StartListening(pType);
    }

    SwTOXType* GetTOXType() const {
        return const_cast<SwTOXType*>(m_pTOXType);
    }

    void DeleteTOXMark()
    {
        m_pDoc->DeleteTOXMark(m_pTOXMark); // calls Invalidate() via Notify
    }

    void InsertTOXMark(SwTOXType & rTOXType, SwTOXMark & rMark, SwPaM & rPam,
            SwXTextCursor const*const pTextCursor);

    void ReplaceTOXMark(SwTOXType & rTOXType, SwTOXMark & rMark, SwPaM & rPam)
    {
        m_bInReplaceMark = true;
        DeleteTOXMark();
        m_bInReplaceMark = false;
        try {
            InsertTOXMark(rTOXType, rMark, rPam, nullptr);
        } catch (...) {
            OSL_FAIL("ReplaceTOXMark() failed!");
            lang::EventObject const ev(
                    static_cast< ::cppu::OWeakObject&>(m_rThis));
            m_EventListeners.disposeAndClear(ev);
            throw;
        }
    }

    void    Invalidate();
protected:
    // SwClient
    virtual void SwClientNotify(const SwModify&, const SfxHint& ) override;
};

void SwXDocumentIndexMark::Impl::Invalidate()
{
    if (!m_bInReplaceMark) // #i109983# only dispose on delete, not on replace!
    {
        uno::Reference<uno::XInterface> const xThis(m_wThis);
        // fdo#72695: if UNO object is already dead, don't revive it with event
        if (xThis.is())
        {
            lang::EventObject const ev(xThis);
            m_EventListeners.disposeAndClear(ev);
        }
    }
    m_aListener.EndListeningAll();
    m_pDoc = nullptr;
    m_pTOXMark = nullptr;
    m_pTOXType = nullptr;
}

void SwXDocumentIndexMark::Impl::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    assert(!GetRegisteredIn()); // we should only listen with the WriterMultiListener from now on
    if(auto pModifyChangedHint = dynamic_cast<const sw::ModifyChangedHint*>(&rHint))
    {
        if(pModifyChangedHint->m_pNew == nullptr || &rModify == m_pTOXMark)
            Invalidate();
        else if(&rModify == m_pTOXType)
            m_pTOXType = dynamic_cast<const SwTOXType*>(pModifyChangedHint->m_pNew);
    }
}

SwXDocumentIndexMark::SwXDocumentIndexMark(const TOXTypes eToxType)
    : m_pImpl( new SwXDocumentIndexMark::Impl(*this, nullptr, eToxType, nullptr, nullptr) )
{
}

SwXDocumentIndexMark::SwXDocumentIndexMark(SwDoc & rDoc,
                SwTOXType & rType, SwTOXMark & rMark)
    : m_pImpl( new SwXDocumentIndexMark::Impl(*this, &rDoc, rType.GetType(),
                    &rType, &rMark) )
{
}

SwXDocumentIndexMark::~SwXDocumentIndexMark()
{
}

uno::Reference<text::XDocumentIndexMark>
SwXDocumentIndexMark::CreateXDocumentIndexMark(
        SwDoc & rDoc, SwTOXMark *const pMark, TOXTypes const eType)
{
    // re-use existing SwXDocumentIndexMark
    // NB: xmloff depends on this caching to generate ID from the address!
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference<text::XDocumentIndexMark> xTOXMark;
    if (pMark)
    {
        xTOXMark = pMark->GetXTOXMark();
    }
    if (!xTOXMark.is())
    {
        SwXDocumentIndexMark *const pNew(pMark
            ? new SwXDocumentIndexMark(rDoc,
                    *const_cast<SwTOXType*>(pMark->GetTOXType()), *pMark)
            : new SwXDocumentIndexMark(eType));
        xTOXMark.set(pNew);
        if (pMark)
        {
            pMark->SetXTOXMark(xTOXMark);
        }
        // need a permanent Reference to initialize m_wThis
        pNew->m_pImpl->m_wThis = xTOXMark;
    }
    return xTOXMark;
}

namespace
{
    class theSwXDocumentIndexMarkUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXDocumentIndexMarkUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXDocumentIndexMark::getUnoTunnelId()
{
    return theSwXDocumentIndexMarkUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXDocumentIndexMark::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return ::sw::UnoTunnelImpl<SwXDocumentIndexMark>(rId, this);
}

OUString SAL_CALL
SwXDocumentIndexMark::getImplementationName()
{
    return OUString("SwXDocumentIndexMark");
}

sal_Bool SAL_CALL SwXDocumentIndexMark::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndexMark::getSupportedServiceNames()
{
    SolarMutexGuard g;

    const sal_Int32 nCnt = (m_pImpl->m_eTOXType == TOX_INDEX) ? 4 : 3;
    uno::Sequence< OUString > aRet(nCnt);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.BaseIndexMark";
    pArray[1] = "com.sun.star.text.TextContent";
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_USER:
            pArray[2] = "com.sun.star.text.UserIndexMark";
        break;
        case TOX_CONTENT:
            pArray[2] = "com.sun.star.text.ContentIndexMark";
        break;
        case TOX_INDEX:
            pArray[2] = "com.sun.star.text.DocumentIndexMark";
            pArray[3] = "com.sun.star.text.DocumentIndexMarkAsian";
        break;

        default:
            ;
    }
    return aRet;
}

OUString SAL_CALL
SwXDocumentIndexMark::getMarkEntry()
{
    SolarMutexGuard aGuard;

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (pType && m_pImpl->m_pTOXMark)
    {
        return m_pImpl->m_pTOXMark->GetAlternativeText();
    }

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    return m_pImpl->m_sAltText;
}

void SAL_CALL
SwXDocumentIndexMark::setMarkEntry(const OUString& rIndexEntry)
{
    SolarMutexGuard aGuard;

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (pType && m_pImpl->m_pTOXMark)
    {
        SwTOXMark aMark(*m_pImpl->m_pTOXMark);
        aMark.SetAlternativeText(rIndexEntry);
        SwTextTOXMark const*const pTextMark =
            m_pImpl->m_pTOXMark->GetTextTOXMark();
        SwPaM aPam(pTextMark->GetTextNode(), pTextMark->GetStart());
        aPam.SetMark();
        if(pTextMark->End())
        {
            aPam.GetPoint()->nContent = *pTextMark->End();
        }
        else
            ++aPam.GetPoint()->nContent;

        m_pImpl->ReplaceTOXMark(*pType, aMark, aPam);
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_sAltText = rIndexEntry;
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL
SwXDocumentIndexMark::attach(
        const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    const uno::Reference<XUnoTunnel> xRangeTunnel(xTextRange, uno::UNO_QUERY);
    SwXTextRange *const pRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    SwDoc *const pDoc =
        pRange ? &pRange->GetDoc() : (pCursor ? pCursor->GetDoc() : nullptr);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    const SwTOXType* pTOXType = nullptr;
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_INDEX:
        case TOX_CONTENT:
        case TOX_CITATION:
            pTOXType = pDoc->GetTOXType( m_pImpl->m_eTOXType, 0 );
        break;
        case TOX_USER:
        {
            if (m_pImpl->m_sUserIndexName.isEmpty())
            {
                pTOXType = pDoc->GetTOXType( m_pImpl->m_eTOXType, 0 );
            }
            else
            {
                const sal_uInt16 nCount =
                    pDoc->GetTOXTypeCount(m_pImpl->m_eTOXType);
                for (sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwTOXType const*const pTemp =
                        pDoc->GetTOXType( m_pImpl->m_eTOXType, i );
                    if (m_pImpl->m_sUserIndexName == pTemp->GetTypeName())
                    {
                        pTOXType = pTemp;
                        break;
                    }
                }
                if (!pTOXType)
                {
                    SwTOXType aUserType(TOX_USER, m_pImpl->m_sUserIndexName);
                    pTOXType = pDoc->InsertTOXType(aUserType);
                }
            }
        }
        break;

        default:
        break;
    }
    if (!pTOXType)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    SwTOXMark aMark (pTOXType);
    if (!m_pImpl->m_sAltText.isEmpty())
    {
        aMark.SetAlternativeText(m_pImpl->m_sAltText);
    }
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_INDEX:
            if (!m_pImpl->m_sPrimaryKey.isEmpty())
            {
                aMark.SetPrimaryKey(m_pImpl->m_sPrimaryKey);
            }
            if (!m_pImpl->m_sSecondaryKey.isEmpty())
            {
                aMark.SetSecondaryKey(m_pImpl->m_sSecondaryKey);
            }
            if (!m_pImpl->m_sTextReading.isEmpty())
            {
                aMark.SetTextReading(m_pImpl->m_sTextReading);
            }
            if (!m_pImpl->m_sPrimaryKeyReading.isEmpty())
            {
                aMark.SetPrimaryKeyReading(m_pImpl->m_sPrimaryKeyReading);
            }
            if (!m_pImpl->m_sSecondaryKeyReading.isEmpty())
            {
                aMark.SetSecondaryKeyReading(m_pImpl->m_sSecondaryKeyReading);
            }
            aMark.SetMainEntry(m_pImpl->m_bMainEntry);
        break;
        case TOX_CITATION:
            aMark.SetMainEntry(m_pImpl->m_bMainEntry);
        break;
        case TOX_USER:
        case TOX_CONTENT:
            if (USHRT_MAX != m_pImpl->m_nLevel)
            {
                aMark.SetLevel(m_pImpl->m_nLevel+1);
            }
        break;

        default:
        break;
    }

    m_pImpl->InsertTOXMark(*const_cast<SwTOXType *>(pTOXType), aMark, aPam,
            dynamic_cast<SwXTextCursor const*>(pCursor));

    m_pImpl->m_bIsDescriptor = false;
}

template<typename T> struct NotContainedIn
{
    std::vector<T> const& m_rVector;
    explicit NotContainedIn(std::vector<T> const& rVector)
        : m_rVector(rVector) { }
    bool operator() (T const& rT) {
        return std::find(m_rVector.begin(), m_rVector.end(), rT)
                    == m_rVector.end();
    }
};

void SwXDocumentIndexMark::Impl::InsertTOXMark(
        SwTOXType & rTOXType, SwTOXMark & rMark, SwPaM & rPam,
        SwXTextCursor const*const pTextCursor)
{
    SwDoc *const pDoc( rPam.GetDoc() );
    UnoActionContext aAction(pDoc);
    bool bMark = *rPam.GetPoint() != *rPam.GetMark();
    // n.b.: toxmarks must have either alternative text or an extent
    if (bMark && !rMark.GetAlternativeText().isEmpty())
    {
        rPam.Normalize();
        rPam.DeleteMark();
        bMark = false;
    }
    // Marks without alternative text and without selected text cannot be inserted,
    // thus use a space - is this really the ideal solution?
    if (!bMark && rMark.GetAlternativeText().isEmpty())
    {
        rMark.SetAlternativeText( " " );
    }

    const bool bForceExpandHints( !bMark && pTextCursor && pTextCursor->IsAtEndOfMeta() );
    const SetAttrMode nInsertFlags = bForceExpandHints
        ?   ( SetAttrMode::FORCEHINTEXPAND
            | SetAttrMode::DONTEXPAND)
        : SetAttrMode::DONTEXPAND;

    std::vector<SwTextAttr *> oldMarks;
    if (bMark)
    {
        oldMarks = rPam.GetNode().GetTextNode()->GetTextAttrsAt(
            rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_TOXMARK);
    }

    pDoc->getIDocumentContentOperations().InsertPoolItem(rPam, rMark, nInsertFlags);
    if (bMark && *rPam.GetPoint() > *rPam.GetMark())
    {
        rPam.Exchange();
    }

    // rMark was copied into the document pool; now retrieve real format...
    SwTextAttr * pTextAttr(nullptr);
    if (bMark)
    {
        // #i107672#
        // ensure that we do not retrieve a different mark at the same position
        std::vector<SwTextAttr *> const newMarks(
            rPam.GetNode().GetTextNode()->GetTextAttrsAt(
                rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_TOXMARK));
        std::vector<SwTextAttr *>::const_iterator const iter(
            std::find_if(newMarks.begin(), newMarks.end(),
                NotContainedIn<SwTextAttr *>(oldMarks)));
        assert(newMarks.end() != iter);
        if (newMarks.end() != iter)
        {
            pTextAttr = *iter;
        }
    }
    else
    {
        pTextAttr = rPam.GetNode().GetTextNode()->GetTextAttrForCharAt(
            rPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_TOXMARK );
    }

    if (!pTextAttr)
    {
        throw uno::RuntimeException(
            "SwXDocumentIndexMark::InsertTOXMark(): cannot insert attribute",
            nullptr);
    }

    m_pDoc = pDoc;
    m_pTOXMark = &pTextAttr->GetTOXMark();
    m_pTOXType = &rTOXType;
    m_aListener.EndListeningAll();
    m_aListener.StartListening(const_cast<SwTOXMark*>(m_pTOXMark));
    m_aListener.StartListening(const_cast<SwTOXType*>(m_pTOXType));
}

uno::Reference< text::XTextRange > SAL_CALL
SwXDocumentIndexMark::getAnchor()
{
    SolarMutexGuard aGuard;

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (!pType || !m_pImpl->m_pTOXMark)
    {
        throw uno::RuntimeException();
    }
    if (!m_pImpl->m_pTOXMark->GetTextTOXMark())
    {
        throw uno::RuntimeException();
    }
    const SwTextTOXMark* pTextMark = m_pImpl->m_pTOXMark->GetTextTOXMark();
    SwPaM aPam(pTextMark->GetTextNode(), pTextMark->GetStart());
    aPam.SetMark();
    if(pTextMark->End())
    {
        aPam.GetPoint()->nContent = *pTextMark->End();
    }
    else
    {
        ++aPam.GetPoint()->nContent;
    }
    const uno::Reference< frame::XModel > xModel =
        m_pImpl->m_pDoc->GetDocShell()->GetBaseModel();
    const uno::Reference< text::XTextDocument > xTDoc(xModel, uno::UNO_QUERY);
    const uno::Reference< text::XTextRange >  xRet =
        new SwXTextRange(aPam, xTDoc->getText());

    return xRet;
}

void SAL_CALL
SwXDocumentIndexMark::dispose()
{
    SolarMutexGuard aGuard;

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (pType && m_pImpl->m_pTOXMark)
    {
        m_pImpl->DeleteTOXMark(); // call Invalidate() via modify!
    }
}

void SAL_CALL
SwXDocumentIndexMark::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL
SwXDocumentIndexMark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXDocumentIndexMark::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo >  xInfos[3];
    int nPos = 0;
    switch (m_pImpl->m_eTOXType)
    {
        case TOX_INDEX:     nPos = 0; break;
        case TOX_CONTENT:   nPos = 1; break;
        case TOX_USER:      nPos = 2; break;
        default:
            ;
    }
    if(!xInfos[nPos].is())
    {
        const uno::Reference< beans::XPropertySetInfo > xInfo =
            m_pImpl->m_rPropSet.getPropertySetInfo();
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xInfos[nPos] = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMapEntries(
                PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return xInfos[nPos];
}

void SAL_CALL
SwXDocumentIndexMark::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw beans::PropertyVetoException(
            "Property is read-only: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (pType && m_pImpl->m_pTOXMark)
    {
        SwTOXMark aMark(*m_pImpl->m_pTOXMark);
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aMark.SetAlternativeText(lcl_AnyToString(rValue));
            break;
            case WID_LEVEL:
                aMark.SetLevel(std::min( static_cast<sal_Int8>( MAXLEVEL ),
                            static_cast<sal_Int8>(lcl_AnyToInt16(rValue)+1)));
            break;
            case WID_TOC_BOOKMARK :
                aMark.SetBookmarkName(lcl_AnyToString(rValue));
            break;
            case WID_INDEX_ENTRY_TYPE :
                aMark.SetEntryTypeName(lcl_AnyToString(rValue));
            break;
            case WID_PRIMARY_KEY  :
                aMark.SetPrimaryKey(lcl_AnyToString(rValue));
            break;
            case WID_SECONDARY_KEY:
                aMark.SetSecondaryKey(lcl_AnyToString(rValue));
            break;
            case WID_MAIN_ENTRY:
                aMark.SetMainEntry(lcl_AnyToBool(rValue));
            break;
            case WID_TEXT_READING:
                aMark.SetTextReading(lcl_AnyToString(rValue));
            break;
            case WID_PRIMARY_KEY_READING:
                aMark.SetPrimaryKeyReading(lcl_AnyToString(rValue));
            break;
            case WID_SECONDARY_KEY_READING:
                aMark.SetSecondaryKeyReading(lcl_AnyToString(rValue));
            break;
        }
        SwTextTOXMark const*const pTextMark =
            m_pImpl->m_pTOXMark->GetTextTOXMark();
        SwPaM aPam(pTextMark->GetTextNode(), pTextMark->GetStart());
        aPam.SetMark();
        if(pTextMark->End())
        {
            aPam.GetPoint()->nContent = *pTextMark->End();
        }
        else
        {
            ++aPam.GetPoint()->nContent;
        }

        m_pImpl->ReplaceTOXMark(*pType, aMark, aPam);
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                m_pImpl->m_sAltText = lcl_AnyToString(rValue);
            break;
            case WID_LEVEL:
            {
                const sal_Int16 nVal = lcl_AnyToInt16(rValue);
                if(nVal < 0 || nVal >= MAXLEVEL)
                {
                    throw lang::IllegalArgumentException();
                }
                m_pImpl->m_nLevel = nVal;
            }
            break;
            case WID_TOC_BOOKMARK :
            {
                m_pImpl->m_aBookmarkName = lcl_AnyToString(rValue);
            }
            break;
            case WID_INDEX_ENTRY_TYPE :
            {
                m_pImpl->m_aEntryTypeName = lcl_AnyToString(rValue);
            }
            break;
            case WID_PRIMARY_KEY:
                m_pImpl->m_sPrimaryKey = lcl_AnyToString(rValue);
            break;
            case WID_SECONDARY_KEY:
                m_pImpl->m_sSecondaryKey = lcl_AnyToString(rValue);
            break;
            case WID_TEXT_READING:
                m_pImpl->m_sTextReading = lcl_AnyToString(rValue);
            break;
            case WID_PRIMARY_KEY_READING:
                m_pImpl->m_sPrimaryKeyReading = lcl_AnyToString(rValue);
            break;
            case WID_SECONDARY_KEY_READING:
                m_pImpl->m_sSecondaryKeyReading = lcl_AnyToString(rValue);
            break;
            case WID_USER_IDX_NAME:
            {
                OUString sTmp(lcl_AnyToString(rValue));
                lcl_ConvertTOUNameToUserName(sTmp);
                m_pImpl->m_sUserIndexName = sTmp;
            }
            break;
            case WID_MAIN_ENTRY:
                m_pImpl->m_bMainEntry = lcl_AnyToBool(rValue);
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
                // unsupported
            break;
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}

uno::Any SAL_CALL
SwXDocumentIndexMark::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }
    if (::sw::GetDefaultTextContentValue(aRet, rPropertyName, pEntry->nWID))
    {
        return aRet;
    }

    SwTOXType *const pType = m_pImpl->GetTOXType();
    if (pType && m_pImpl->m_pTOXMark)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= m_pImpl->m_pTOXMark->GetAlternativeText();
            break;
            case WID_LEVEL:
                aRet <<= static_cast<sal_Int16>(
                            m_pImpl->m_pTOXMark->GetLevel() - 1);
            break;
            case WID_TOC_BOOKMARK :
                aRet <<= m_pImpl->m_pTOXMark->GetBookmarkName();
            break;
            case WID_INDEX_ENTRY_TYPE :
                aRet <<= m_pImpl->m_pTOXMark->GetEntryTypeName();
            break;
            case WID_PRIMARY_KEY  :
                aRet <<= m_pImpl->m_pTOXMark->GetPrimaryKey();
            break;
            case WID_SECONDARY_KEY:
                aRet <<= m_pImpl->m_pTOXMark->GetSecondaryKey();
            break;
            case WID_TEXT_READING:
                aRet <<= m_pImpl->m_pTOXMark->GetTextReading();
            break;
            case WID_PRIMARY_KEY_READING:
                aRet <<= m_pImpl->m_pTOXMark->GetPrimaryKeyReading();
            break;
            case WID_SECONDARY_KEY_READING:
                aRet <<= m_pImpl->m_pTOXMark->GetSecondaryKeyReading();
            break;
            case WID_USER_IDX_NAME :
            {
                OUString sTmp(pType->GetTypeName());
                lcl_ConvertTOUNameToProgrammaticName(sTmp);
                aRet <<= sTmp;
            }
            break;
            case WID_MAIN_ENTRY:
            {
                const bool bTemp = m_pImpl->m_pTOXMark->IsMainEntry();
                aRet <<= bTemp;
            }
            break;
        }
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        switch(pEntry->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= m_pImpl->m_sAltText;
            break;
            case WID_LEVEL:
                aRet <<= static_cast<sal_Int16>(m_pImpl->m_nLevel);
            break;
            case WID_TOC_BOOKMARK :
                aRet <<= m_pImpl->m_aBookmarkName;
            break;
            case WID_INDEX_ENTRY_TYPE :
                aRet <<= m_pImpl->m_aEntryTypeName;
            break;
            case WID_PRIMARY_KEY:
                aRet <<= m_pImpl->m_sPrimaryKey;
            break;
            case WID_SECONDARY_KEY:
                aRet <<= m_pImpl->m_sSecondaryKey;
            break;
            case WID_TEXT_READING:
                aRet <<= m_pImpl->m_sTextReading;
            break;
            case WID_PRIMARY_KEY_READING:
                aRet <<= m_pImpl->m_sPrimaryKeyReading;
            break;
            case WID_SECONDARY_KEY_READING:
                aRet <<= m_pImpl->m_sSecondaryKeyReading;
            break;
            case WID_USER_IDX_NAME :
                aRet <<= m_pImpl->m_sUserIndexName;
            break;
            case WID_MAIN_ENTRY:
                aRet <<= m_pImpl->m_bMainEntry;
            break;
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
    return aRet;
}

void SAL_CALL
SwXDocumentIndexMark::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndexMark::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndexMark::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndexMark::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndexMark::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndexMark::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXDocumentIndexMark::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXDocumentIndexMark::removeVetoableChangeListener(): not implemented");
}

SwXDocumentIndexes::SwXDocumentIndexes(SwDoc *const _pDoc)
    : SwUnoCollection(_pDoc)
{
}

SwXDocumentIndexes::~SwXDocumentIndexes()
{
}

OUString SAL_CALL
SwXDocumentIndexes::getImplementationName()
{
    return OUString("SwXDocumentIndexes");
}

static char const*const g_ServicesDocumentIndexes[] =
{
    "com.sun.star.text.DocumentIndexes",
};

sal_Bool SAL_CALL SwXDocumentIndexes::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndexes::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
        SAL_N_ELEMENTS(g_ServicesDocumentIndexes), g_ServicesDocumentIndexes);
}

sal_Int32 SAL_CALL
SwXDocumentIndexes::getCount()
{
    SolarMutexGuard aGuard;

    if(!IsValid())
        throw uno::RuntimeException();

    sal_uInt32 nRet = 0;
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        const SwSection* pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() )
        {
            ++nRet;
        }
    }
    return nRet;
}

uno::Any SAL_CALL
SwXDocumentIndexes::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
        throw uno::RuntimeException();

    sal_Int32 nIdx = 0;

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        SwSection* pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() &&
            nIdx++ == nIndex )
        {
           const uno::Reference< text::XDocumentIndex > xTmp =
               SwXDocumentIndex::CreateXDocumentIndex(
                   *GetDoc(), static_cast<SwTOXBaseSection *>(pSect));
           uno::Any aRet;
           aRet <<= xTmp;
           return aRet;
        }
    }

    throw lang::IndexOutOfBoundsException();
}

uno::Any SAL_CALL
SwXDocumentIndexes::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
        throw uno::RuntimeException();

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        SwSection* pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() &&
            (static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()
                == rName))
        {
           const uno::Reference< text::XDocumentIndex > xTmp =
               SwXDocumentIndex::CreateXDocumentIndex(
                   *GetDoc(), static_cast<SwTOXBaseSection *>(pSect));
           uno::Any aRet;
           aRet <<= xTmp;
           return aRet;
        }
    }
    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndexes::getElementNames()
{
    SolarMutexGuard aGuard;

    if(!IsValid())
        throw uno::RuntimeException();

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    sal_Int32 nCount = 0;
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        SwSection const*const pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() )
        {
            ++nCount;
        }
    }

    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    sal_Int32 nCnt = 0;
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        SwSection const*const pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode())
        {
            pArray[nCnt++] = static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName();
        }
    }
    return aRet;
}

sal_Bool SAL_CALL
SwXDocumentIndexes::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
        throw uno::RuntimeException();

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    for( size_t n = 0; n < rFormats.size(); ++n )
    {
        SwSection const*const pSect = rFormats[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode())
        {
            if (static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()
                    == rName)
            {
                return true;
            }
        }
    }
    return false;
}

uno::Type SAL_CALL
SwXDocumentIndexes::getElementType()
{
    return cppu::UnoType<text::XDocumentIndex>::get();
}

sal_Bool SAL_CALL
SwXDocumentIndexes::hasElements()
{
    return 0 != getCount();
}

SwXDocumentIndex::StyleAccess_Impl::StyleAccess_Impl(
        SwXDocumentIndex& rParentIdx)
    : m_xParent(&rParentIdx)
{
}

SwXDocumentIndex::StyleAccess_Impl::~StyleAccess_Impl()
{
}

OUString SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::getImplementationName()
{
    return OUString("SwXDocumentIndex::StyleAccess_Impl");
}

static char const*const g_ServicesIndexStyleAccess[] =
{
    "com.sun.star.text.DocumentIndexParagraphStyles",
};

sal_Bool SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
        SAL_N_ELEMENTS(g_ServicesIndexStyleAccess),
        g_ServicesIndexStyleAccess);
}

void SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::replaceByIndex(
        sal_Int32 nIndex, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;

    if(nIndex < 0 || nIndex >= MAXLEVEL)
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwTOXBase & rTOXBase( m_xParent->m_pImpl->GetTOXSectionOrThrow() );

    uno::Sequence<OUString> aSeq;
    if(!(rElement >>= aSeq))
    {
        throw lang::IllegalArgumentException();
    }

    const sal_Int32 nStyles = aSeq.getLength();
    const OUString* pStyles = aSeq.getConstArray();
    OUStringBuffer sSetStyles;
    OUString aString;
    for(sal_Int32 i = 0; i < nStyles; i++)
    {
        if(i)
        {
            sSetStyles.append(TOX_STYLE_DELIMITER);
        }
        SwStyleNameMapper::FillUIName(pStyles[i], aString,
                SwGetPoolIdFromName::TxtColl);
        sSetStyles.append(aString);
    }
    rTOXBase.SetStyleNames(sSetStyles.makeStringAndClear(), static_cast<sal_uInt16>(nIndex));
}

sal_Int32 SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::getCount()
{
    return MAXLEVEL;
}

uno::Any SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if(nIndex < 0 || nIndex >= MAXLEVEL)
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwTOXBase & rTOXBase( m_xParent->m_pImpl->GetTOXSectionOrThrow() );

    const OUString& rStyles =
        rTOXBase.GetStyleNames(static_cast<sal_uInt16>(nIndex));
    const sal_Int32 nStyles = comphelper::string::getTokenCount(rStyles, TOX_STYLE_DELIMITER);
    uno::Sequence<OUString> aStyles(nStyles);
    OUString* pStyles = aStyles.getArray();
    OUString aString;
    sal_Int32 nPos = 0;
    for(sal_Int32 i = 0; i < nStyles; ++i)
    {
        SwStyleNameMapper::FillProgName(
            rStyles.getToken(0, TOX_STYLE_DELIMITER, nPos),
            aString,
            SwGetPoolIdFromName::TxtColl);
        pStyles[i] = aString;
    }
    uno::Any aRet(&aStyles, cppu::UnoType<uno::Sequence<OUString>>::get());
    return aRet;
}

uno::Type SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::getElementType()
{
    return cppu::UnoType<uno::Sequence<OUString>>::get();
}

sal_Bool SAL_CALL
SwXDocumentIndex::StyleAccess_Impl::hasElements()
{
    return true;
}

SwXDocumentIndex::TokenAccess_Impl::TokenAccess_Impl(
        SwXDocumentIndex& rParentIdx)
    : m_xParent(&rParentIdx)
{
}

SwXDocumentIndex::TokenAccess_Impl::~TokenAccess_Impl()
{
}

OUString SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::getImplementationName()
{
    return OUString("SwXDocumentIndex::TokenAccess_Impl");
}

static char const*const g_ServicesIndexTokenAccess[] =
{
    "com.sun.star.text.DocumentIndexLevelFormat",
};

sal_Bool SAL_CALL SwXDocumentIndex::TokenAccess_Impl::supportsService(
        const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
            SAL_N_ELEMENTS(g_ServicesIndexTokenAccess),
            g_ServicesIndexTokenAccess);
}

struct TokenType_ {
    const char *pName;
    enum FormTokenType const eTokenType;
};

static const struct TokenType_ g_TokenTypes[] =
{
    { "TokenEntryNumber",           TOKEN_ENTRY_NO  },
    { "TokenEntryText",             TOKEN_ENTRY_TEXT },
    { "TokenTabStop",               TOKEN_TAB_STOP },
    { "TokenText",                  TOKEN_TEXT },
    { "TokenPageNumber",            TOKEN_PAGE_NUMS },
    { "TokenChapterInfo",           TOKEN_CHAPTER_INFO },
    { "TokenHyperlinkStart",        TOKEN_LINK_START },
    { "TokenHyperlinkEnd",          TOKEN_LINK_END },
    { "TokenBibliographyDataField", TOKEN_AUTHORITY },
    { nullptr, static_cast<enum FormTokenType>(0) }
};

void SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::replaceByIndex(
        sal_Int32 nIndex, const uno::Any& rElement)
{
    SolarMutexGuard aGuard;

    SwTOXBase & rTOXBase( m_xParent->m_pImpl->GetTOXSectionOrThrow() );

    if ((nIndex < 0) || (nIndex > rTOXBase.GetTOXForm().GetFormMax()))
    {
        throw lang::IndexOutOfBoundsException();
    }

    uno::Sequence<beans::PropertyValues> aSeq;
    if(!(rElement >>= aSeq))
    {
        throw lang::IllegalArgumentException();
    }

    OUStringBuffer sPattern;
    const sal_Int32 nTokens = aSeq.getLength();
    const beans::PropertyValues* pTokens = aSeq.getConstArray();
    for(sal_Int32 i = 0; i < nTokens; i++)
    {
        const beans::PropertyValue* pProperties = pTokens[i].getConstArray();
        const sal_Int32 nProperties = pTokens[i].getLength();
        //create an invalid token
        SwFormToken aToken(TOKEN_END);
        for(sal_Int32 j = 0; j < nProperties; j++)
        {
            if ( pProperties[j].Name == "TokenType" )
            {
                const OUString sTokenType =
                        lcl_AnyToString(pProperties[j].Value);
                for (TokenType_ const* pTokenType = g_TokenTypes;
                        pTokenType->pName; ++pTokenType)
                {
                    if (sTokenType.equalsAscii(pTokenType->pName))
                    {
                        aToken.eTokenType = pTokenType->eTokenType;
                        break;
                    }
                }
            }
            else if ( pProperties[j].Name == "CharacterStyleName" )
            {
                OUString sCharStyleName;
                SwStyleNameMapper::FillUIName(
                        lcl_AnyToString(pProperties[j].Value),
                        sCharStyleName,
                        SwGetPoolIdFromName::ChrFmt);
                aToken.sCharStyleName = sCharStyleName;
                aToken.nPoolId = SwStyleNameMapper::GetPoolIdFromUIName (
                    sCharStyleName, SwGetPoolIdFromName::ChrFmt );
            }
            else if ( pProperties[j].Name == "TabStopRightAligned" )
            {
                const bool bRight = lcl_AnyToBool(pProperties[j].Value);
                aToken.eTabAlign = bRight ?
                                    SvxTabAdjust::End : SvxTabAdjust::Left;
            }
            else if ( pProperties[j].Name == "TabStopPosition" )
            {
                sal_Int32 nPosition = 0;
                if (!(pProperties[j].Value >>= nPosition))
                {
                    throw lang::IllegalArgumentException();
                }
                nPosition = convertMm100ToTwip(nPosition);
                if(nPosition < 0)
                {
                    throw lang::IllegalArgumentException();
                }
                aToken.nTabStopPosition = nPosition;
            }
            else if ( pProperties[j].Name == "TabStopFillCharacter" )
            {
                const OUString sFillChar =
                    lcl_AnyToString(pProperties[j].Value);
                if (sFillChar.getLength() > 1)
                {
                    throw lang::IllegalArgumentException();
                }
                aToken.cTabFillChar =
                    sFillChar.isEmpty() ? ' ' : sFillChar[0];
            }
            else if ( pProperties[j].Name == "Text" )
            {
                aToken.sText = lcl_AnyToString(pProperties[j].Value);
            }
            else if ( pProperties[j].Name == "ChapterFormat" )
            {
                sal_Int16 nFormat = lcl_AnyToInt16(pProperties[j].Value);
                switch(nFormat)
                {
                    case text::ChapterFormat::NUMBER:
                        nFormat = CF_NUMBER;
                    break;
                    case text::ChapterFormat::NAME:
                        nFormat = CF_TITLE;
                    break;
                    case text::ChapterFormat::NAME_NUMBER:
                        nFormat = CF_NUM_TITLE;
                    break;
                    case text::ChapterFormat::NO_PREFIX_SUFFIX:
                        nFormat = CF_NUMBER_NOPREPST;
                    break;
                    case text::ChapterFormat::DIGIT:
                        nFormat = CF_NUM_NOPREPST_TITLE;
                    break;
                    default:
                        throw lang::IllegalArgumentException();
                }
                aToken.nChapterFormat = nFormat;
            }
// #i53420#
            else if ( pProperties[j].Name == "ChapterLevel" )
            {
                const sal_Int16 nLevel = lcl_AnyToInt16(pProperties[j].Value);
                if( nLevel < 1 || nLevel > MAXLEVEL )
                {
                    throw lang::IllegalArgumentException();
                }
                aToken.nOutlineLevel = nLevel;
            }
            else if ( pProperties[j].Name == "BibliographyDataField" )
            {
                sal_Int16 nType = 0;
                pProperties[j].Value >>= nType;
                if(nType < 0 || nType > text::BibliographyDataField::ISBN)
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = "BibliographyDataField - wrong value";
                    aExcept.ArgumentPosition = static_cast< sal_Int16 >(j);
                    throw aExcept;
                }
                aToken.nAuthorityField = nType;
            }
            // #i21237#
            else if ( pProperties[j].Name == "WithTab" )
            {
                aToken.bWithTab = lcl_AnyToBool(pProperties[j].Value);
            }

        }
        //exception if wrong TokenType
        if(TOKEN_END <= aToken.eTokenType )
        {
            throw lang::IllegalArgumentException();
        }
        // set TokenType from TOKEN_ENTRY_TEXT to TOKEN_ENTRY if it is
        // not a content index
        if(TOKEN_ENTRY_TEXT == aToken.eTokenType &&
                                (TOX_CONTENT != rTOXBase.GetType()))
        {
            aToken.eTokenType = TOKEN_ENTRY;
        }
// #i53420#
// check for chapter format allowed values if it was TOKEN_ENTRY_NO type
// only allowed value are CF_NUMBER and CF_NUM_NOPREPST_TITLE
// reading from file
        if( TOKEN_ENTRY_NO == aToken.eTokenType )
        {
            switch(aToken.nChapterFormat)
            {
            case CF_NUMBER:
            case CF_NUM_NOPREPST_TITLE:
                break;
            default:
                throw lang::IllegalArgumentException();
            }
        }
        sPattern.append(aToken.GetString());
    }
    SwForm aForm(rTOXBase.GetTOXForm());
    aForm.SetPattern(static_cast<sal_uInt16>(nIndex), sPattern.makeStringAndClear());
    rTOXBase.SetTOXForm(aForm);
}

sal_Int32 SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::getCount()
{
    SolarMutexGuard aGuard;

    const sal_Int32 nRet = m_xParent->m_pImpl->GetFormMax();
    return nRet;
}

uno::Any SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    SwTOXBase & rTOXBase( m_xParent->m_pImpl->GetTOXSectionOrThrow() );

    if ((nIndex < 0) || (nIndex > rTOXBase.GetTOXForm().GetFormMax()))
    {
        throw lang::IndexOutOfBoundsException();
    }

    // #i21237#
    SwFormTokens aPattern = rTOXBase.GetTOXForm().
        GetPattern(static_cast<sal_uInt16>(nIndex));

    sal_Int32 nTokenCount = 0;
    uno::Sequence< beans::PropertyValues > aRetSeq;
    OUString aProgCharStyle;
    for(const SwFormToken& aToken : aPattern) // #i21237#
    {
        nTokenCount++;
        aRetSeq.realloc(nTokenCount);
        beans::PropertyValues* pTokenProps = aRetSeq.getArray();

        uno::Sequence< beans::PropertyValue >& rCurTokenSeq =
            pTokenProps[nTokenCount-1];
        SwStyleNameMapper::FillProgName(
                        aToken.sCharStyleName,
                        aProgCharStyle,
                        SwGetPoolIdFromName::ChrFmt);
        switch(aToken.eTokenType)
        {
            case TOKEN_ENTRY_NO:
            {
// #i53420#
// writing to file (from doc to properties)
                sal_Int32 nElements = 2;
                sal_Int32 nCurrentElement = 0;

                // check for default value
                if (aToken.nChapterFormat != CF_NUMBER)
                {
                    nElements++;//we need the element
                }
                if( aToken.nOutlineLevel != MAXLEVEL )
                {
                    nElements++;
                }

                rCurTokenSeq.realloc( nElements );

                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[nCurrentElement].Name = "TokenType";
                pArr[nCurrentElement++].Value <<=
                    OUString("TokenEntryNumber");

                pArr[nCurrentElement].Name = "CharacterStyleName";
                pArr[nCurrentElement++].Value <<= aProgCharStyle;
                if( aToken.nChapterFormat != CF_NUMBER )
                {
                    pArr[nCurrentElement].Name = "ChapterFormat";
                    sal_Int16 nVal;
// the allowed values for chapter format, when used as entry number,
// are CF_NUMBER and CF_NUM_NOPREPST_TITLE only, all else forced to
//CF_NUMBER
                    switch(aToken.nChapterFormat)
                    {
                    default:
                    case CF_NUMBER:
                        nVal = text::ChapterFormat::NUMBER;
                    break;
                    case CF_NUM_NOPREPST_TITLE:
                        nVal = text::ChapterFormat::DIGIT;
                    break;
                    }
                    pArr[nCurrentElement++].Value <<= nVal;
                }

                // only  a ChapterLevel != MAXLEVEL is registered
                if (aToken.nOutlineLevel != MAXLEVEL)
                {
                    pArr[nCurrentElement].Name = "ChapterLevel";
                    pArr[nCurrentElement].Value <<= aToken.nOutlineLevel;
                }
            }
            break;
            case TOKEN_ENTRY:   // no difference between Entry and Entry Text
            case TOKEN_ENTRY_TEXT:
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<= OUString("TokenEntryText");

                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_TAB_STOP:
            {
                rCurTokenSeq.realloc(5); // #i21237#
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<= OUString("TokenTabStop");

                if(SvxTabAdjust::End == aToken.eTabAlign)
                {
                    pArr[1].Name = "TabStopRightAligned";
                    pArr[1].Value <<= true;
                }
                else
                {
                    pArr[1].Name = "TabStopPosition";
                    sal_Int32 nPos = convertTwipToMm100(aToken.nTabStopPosition);
                    if(nPos < 0)
                        nPos = 0;
                    pArr[1].Value <<= nPos;
                }
                pArr[2].Name = "TabStopFillCharacter";
                pArr[2].Value <<= OUString(aToken.cTabFillChar);
                pArr[3].Name = "CharacterStyleName";
                pArr[3].Value <<= aProgCharStyle;
                // #i21237#
                pArr[4].Name = "WithTab";
                pArr[4].Value <<= aToken.bWithTab;
            }
            break;
            case TOKEN_TEXT:
            {
                rCurTokenSeq.realloc( 3 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<= OUString("TokenText");

                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = "Text";
                pArr[2].Value <<= aToken.sText;
            }
            break;
            case TOKEN_PAGE_NUMS:
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<= OUString("TokenPageNumber");

                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_CHAPTER_INFO:
            {
                rCurTokenSeq.realloc( 4 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<= OUString("TokenChapterInfo");

                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = "ChapterFormat";
                sal_Int16 nVal = text::ChapterFormat::NUMBER;
                switch(aToken.nChapterFormat)
                {
                    case CF_NUMBER:
                        nVal = text::ChapterFormat::NUMBER;
                    break;
                    case CF_TITLE:
                        nVal = text::ChapterFormat::NAME;
                    break;
                    case CF_NUM_TITLE:
                        nVal = text::ChapterFormat::NAME_NUMBER;
                    break;
                    case CF_NUMBER_NOPREPST:
                        nVal = text::ChapterFormat::NO_PREFIX_SUFFIX;
                    break;
                    case CF_NUM_NOPREPST_TITLE:
                        nVal = text::ChapterFormat::DIGIT;
                    break;
                }
                pArr[2].Value <<= nVal;
// #i53420#
                pArr[3].Name = "ChapterLevel";
                pArr[3].Value <<= aToken.nOutlineLevel;
            }
            break;
            case TOKEN_LINK_START:
            {
                rCurTokenSeq.realloc( 2 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<=
                    OUString("TokenHyperlinkStart");
                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_LINK_END:
            {
                rCurTokenSeq.realloc( 1 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<=
                    OUString("TokenHyperlinkEnd");
            }
            break;
            case TOKEN_AUTHORITY:
            {
                rCurTokenSeq.realloc( 3 );
                beans::PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = "TokenType";
                pArr[0].Value <<=
                    OUString("TokenBibliographyDataField");

                pArr[1].Name = "CharacterStyleName";
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = "BibliographyDataField";
                pArr[2].Value <<= sal_Int16(aToken.nAuthorityField);
            }
            break;

            default:
                ;
        }
    }

    uno::Any aRet;
    aRet <<= aRetSeq;
    return aRet;
}

uno::Type SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::getElementType()
{
    return cppu::UnoType<uno::Sequence< beans::PropertyValues >>::get();
}

sal_Bool SAL_CALL
SwXDocumentIndex::TokenAccess_Impl::hasElements()
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
