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
#include <unosection.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>

#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <cmdid.h>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <svl/listener.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <fmtclds.hxx>
#include <unotextrange.hxx>
#include <TextCursorHelper.hxx>
#include <unoport.hxx>
#include <redline.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <section.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docsh.hxx>
#include <sfx2/docfile.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <hints.hxx>
#include <tox.hxx>
#include <unoidx.hxx>
#include <doctxm.hxx>
#include <fmtftntx.hxx>
#include <fmtclbl.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtcntnt.hxx>
#include <editeng/lrspitem.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;

struct SwTextSectionProperties_Impl
{
    uno::Sequence<sal_Int8> m_Password;
    OUString  m_sCondition;
    OUString  m_sLinkFileName;
    OUString  m_sSectionFilter;
    OUString  m_sSectionRegion;

    std::unique_ptr<SwFormatCol>                 m_pColItem;
    std::unique_ptr<SvxBrushItem>             m_pBrushItem;
    std::unique_ptr<SwFormatFootnoteAtTextEnd>         m_pFootnoteItem;
    std::unique_ptr<SwFormatEndAtTextEnd>         m_pEndItem;
    std::unique_ptr<SvXMLAttrContainerItem> m_pXMLAttr;
    std::unique_ptr<SwFormatNoBalancedColumns> m_pNoBalanceItem;
    std::unique_ptr<SvxFrameDirectionItem>    m_pFrameDirItem;
    std::unique_ptr<SvxLRSpaceItem>           m_pLRSpaceItem;

    bool m_bDDE;
    bool m_bHidden;
    bool m_bCondHidden;
    bool m_bProtect;
    bool m_bEditInReadonly;
    bool m_bUpdateType;

    SwTextSectionProperties_Impl()
        : m_bDDE(false)
        , m_bHidden(false)
        , m_bCondHidden(false)
        , m_bProtect(false)
        , m_bEditInReadonly(false)
        , m_bUpdateType(true)
    {
    }

};

class SwXTextSection::Impl
    : public SvtListener
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    SwXTextSection &            m_rThis;
    uno::WeakReference<uno::XInterface> m_wThis;
    const SfxItemPropertySet &  m_rPropSet;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    const bool                  m_bIndexHeader;
    bool                        m_bIsDescriptor;
    OUString             m_sName;
    std::unique_ptr<SwTextSectionProperties_Impl> m_pProps;
    SwSectionFormat* m_pFormat;

    Impl(   SwXTextSection& rThis,
            SwSectionFormat* const pFormat, const bool bIndexHeader)
        : SvtListener()
        , m_rThis(rThis)
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_SECTION))
        , m_EventListeners(m_Mutex)
        , m_bIndexHeader(bIndexHeader)
        , m_bIsDescriptor(nullptr == pFormat)
        , m_pProps(pFormat ? nullptr : new SwTextSectionProperties_Impl())
        , m_pFormat(pFormat)
    {
        if(m_pFormat)
            StartListening(m_pFormat->GetNotifier());
    }

    void Attach(SwSectionFormat* pFormat)
    {
        EndListeningAll();
        StartListening(pFormat->GetNotifier());
        m_pFormat = pFormat;
    }

    SwSectionFormat* GetSectionFormat() const
        { return m_pFormat; }

    SwSectionFormat & GetSectionFormatOrThrow() const {
        SwSectionFormat *const pFormat( GetSectionFormat() );
        if (!pFormat) {
            throw uno::RuntimeException("SwXTextSection: disposed or invalid", nullptr);
        }
        return *pFormat;
    }

    /// @throws beans::UnknownPropertyException
    /// @throws beans::PropertyVetoException,
    /// @throws lang::IllegalArgumentException
    /// @throws lang::WrappedTargetException,
    /// @throws uno::RuntimeException
    void SetPropertyValues_Impl(
            const uno::Sequence< OUString >& rPropertyNames,
            const uno::Sequence< uno::Any >& aValues);
    /// @throws beans::UnknownPropertyException
    /// @throws lang::WrappedTargetException,
    /// @throws uno::RuntimeException
    uno::Sequence< uno::Any >
        GetPropertyValues_Impl(
            const uno::Sequence< OUString >& rPropertyNames);
    virtual void Notify(const SfxHint& rHint) override;
};

void SwXTextSection::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pFormat = nullptr;
        uno::Reference<uno::XInterface> const xThis(m_wThis);
        if (!xThis.is())
        {   // fdo#72695: if UNO object is already dead, don't revive it with event
            return;
        }
        lang::EventObject const ev(xThis);
        m_EventListeners.disposeAndClear(ev);
    }
}

SwSectionFormat * SwXTextSection::GetFormat() const
{
    return m_pImpl->GetSectionFormat();
}

uno::Reference< text::XTextSection >
SwXTextSection::CreateXTextSection(
        SwSectionFormat *const pFormat, const bool bIndexHeader)
{
    // re-use existing SwXTextSection
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference< text::XTextSection > xSection;
    if (pFormat)
    {
        xSection.set(pFormat->GetXTextSection());
    }
    if ( !xSection.is() )
    {
        SwXTextSection *const pNew = new SwXTextSection(pFormat, bIndexHeader);
        xSection.set(pNew);
        if (pFormat)
        {
            pFormat->SetXTextSection(xSection);
        }
        // need a permanent Reference to initialize m_wThis
        pNew->m_pImpl->m_wThis = xSection;
    }
    return xSection;
}

SwXTextSection::SwXTextSection(
        SwSectionFormat *const pFormat, const bool bIndexHeader)
    : m_pImpl( new SwXTextSection::Impl(*this, pFormat, bIndexHeader) )
{
}

SwXTextSection::~SwXTextSection()
{
}

namespace
{
    class theSwXTextSectionUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextSectionUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextSection::getUnoTunnelId()
{
    return theSwXTextSectionUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXTextSection::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return ::sw::UnoTunnelImpl<SwXTextSection>(rId, this);
}

uno::Reference< text::XTextSection > SAL_CALL
SwXTextSection::getParentSection()
{
    SolarMutexGuard aGuard;

    SwSectionFormat & rSectionFormat( m_pImpl->GetSectionFormatOrThrow() );

    SwSectionFormat *const pParentFormat = rSectionFormat.GetParent();
    const uno::Reference< text::XTextSection > xRet =
        pParentFormat ? CreateXTextSection(pParentFormat) : nullptr;
    return xRet;
}

uno::Sequence< uno::Reference< text::XTextSection > > SAL_CALL
SwXTextSection::getChildSections()
{
    SolarMutexGuard aGuard;

    SwSectionFormat & rSectionFormat( m_pImpl->GetSectionFormatOrThrow() );

    SwSections aChildren;
    rSectionFormat.GetChildSections(aChildren, SectionSort::Not, false);
    uno::Sequence<uno::Reference<text::XTextSection> > aSeq(aChildren.size());
    uno::Reference< text::XTextSection > * pArray = aSeq.getArray();
    for (size_t i = 0; i < aChildren.size(); ++i)
    {
        SwSectionFormat *const pChild = aChildren[i]->GetFormat();
        pArray[i] = CreateXTextSection(pChild);
    }
    return aSeq;
}

void SAL_CALL
SwXTextSection::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard g;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    SwDoc *const pDoc =
        pRange ? &pRange->GetDoc() : (pCursor ? pCursor->GetDoc() : nullptr);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDoc);
    // this has to return true now
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aCont(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSSECTION, nullptr );

    if (m_pImpl->m_sName.isEmpty())
    {
        m_pImpl->m_sName = "TextSection";
    }
    SectionType eType(FILE_LINK_SECTION);
    if( m_pImpl->m_pProps->m_bDDE )
        eType = DDE_LINK_SECTION;
    else if( m_pImpl->m_pProps->m_sLinkFileName.isEmpty() && m_pImpl->m_pProps->m_sSectionRegion.isEmpty() )
        eType = CONTENT_SECTION;
    // index header section?
    if (m_pImpl->m_bIndexHeader)
    {
        // caller wants an index header section, but will only
        // give him one if a) we are inside an index, and b) said
        // index doesn't yet have a header section.
        const SwTOXBase* pBase = SwDoc::GetCurTOX(*aPam.Start());

        // are we inside an index?
        if (pBase)
        {
            // get all child sections
            SwSections aSectionsArr;
            static_cast<const SwTOXBaseSection*>(pBase)->GetFormat()->
                GetChildSections(aSectionsArr);

            // and search for current header section
            const size_t nCount = aSectionsArr.size();
            bool bHeaderPresent = false;
            for(size_t i = 0; i < nCount; ++i)
            {
                if (aSectionsArr[i]->GetType() == TOX_HEADER_SECTION)
                    bHeaderPresent = true;
            }
            if (! bHeaderPresent)
            {
                eType = TOX_HEADER_SECTION;
            }
        }
    }

    SwSectionData aSect(eType, pDoc->GetUniqueSectionName(&m_pImpl->m_sName));
    aSect.SetCondition(m_pImpl->m_pProps->m_sCondition);
    aSect.SetLinkFileName(m_pImpl->m_pProps->m_sLinkFileName +
        OUStringLiteral1(sfx2::cTokenSeparator) +
        m_pImpl->m_pProps->m_sSectionFilter +
        OUStringLiteral1(sfx2::cTokenSeparator) +
        m_pImpl->m_pProps->m_sSectionRegion);

    aSect.SetHidden(m_pImpl->m_pProps->m_bHidden);
    aSect.SetProtectFlag(m_pImpl->m_pProps->m_bProtect);
    aSect.SetEditInReadonlyFlag(m_pImpl->m_pProps->m_bEditInReadonly);

    SfxItemSet aSet(
        pDoc->GetAttrPool(),
        svl::Items<
            RES_LR_SPACE, RES_LR_SPACE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            RES_FTN_AT_TXTEND, RES_FRAMEDIR,
            RES_UNKNOWNATR_CONTAINER,RES_UNKNOWNATR_CONTAINER>{});
    if (m_pImpl->m_pProps->m_pBrushItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pBrushItem);
    }
    if (m_pImpl->m_pProps->m_pColItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pColItem);
    }
    if (m_pImpl->m_pProps->m_pFootnoteItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pFootnoteItem);
    }
    if (m_pImpl->m_pProps->m_pEndItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pEndItem);
    }
    if (m_pImpl->m_pProps->m_pXMLAttr)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pXMLAttr);
    }
    if (m_pImpl->m_pProps->m_pNoBalanceItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pNoBalanceItem);
    }
    if (m_pImpl->m_pProps->m_pFrameDirItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pFrameDirItem);
    }
    if (m_pImpl->m_pProps->m_pLRSpaceItem)
    {
        aSet.Put(*m_pImpl->m_pProps->m_pLRSpaceItem);
    }
    // section password
    if (m_pImpl->m_pProps->m_Password.getLength() > 0)
    {
        aSect.SetPassword(m_pImpl->m_pProps->m_Password);
    }

    SwSection *const pRet =
        pDoc->InsertSwSection( aPam, aSect, nullptr, aSet.Count() ? &aSet : nullptr );
    if (!pRet) // fdo#42450 text range could partially overlap existing section
    {
        // shouldn't have created an undo object yet
        pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSSECTION, nullptr );
        throw lang::IllegalArgumentException(
                "SwXTextSection::attach(): invalid TextRange",
                static_cast< ::cppu::OWeakObject*>(this), 0);
    }
    m_pImpl->Attach(pRet->GetFormat());
    pRet->GetFormat()->SetXObject(static_cast< ::cppu::OWeakObject*>(this));

    // XML import must hide sections depending on their old
    //         condition status
    if (!m_pImpl->m_pProps->m_sCondition.isEmpty())
    {
        pRet->SetCondHidden(m_pImpl->m_pProps->m_bCondHidden);
    }

    // set update type if DDE link (and connect, if necessary)
    if (m_pImpl->m_pProps->m_bDDE)
    {
        if (! pRet->IsConnected())
        {
            pRet->CreateLink(CREATE_CONNECT);
        }
        pRet->SetUpdateType( m_pImpl->m_pProps->m_bUpdateType ?
                SfxLinkUpdateMode::ALWAYS : SfxLinkUpdateMode::ONCALL );
    }

    // end the Undo bracketing here
    pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSSECTION, nullptr );
    m_pImpl->m_pProps.reset();
    m_pImpl->m_bIsDescriptor = false;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextSection::getAnchor()
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XTextRange >  xRet;
    SwSectionFormat *const pSectFormat = m_pImpl->GetSectionFormat();
    if(pSectFormat)
    {
        const SwNodeIndex* pIdx;
        if( nullptr != ( pSectFormat->GetSection() ) &&
            nullptr != ( pIdx = pSectFormat->GetContent().GetContentIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, GoInContent );

            const SwEndNode* pEndNode = pIdx->GetNode().EndOfSectionNode();
            SwPaM aEnd(*pEndNode);
            aEnd.Move( fnMoveBackward, GoInContent );
            xRet = SwXTextRange::CreateXTextRange(*pSectFormat->GetDoc(),
                *aPaM.Start(), aEnd.Start());
        }
    }
    return xRet;
}

void SAL_CALL SwXTextSection::dispose()
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pSectFormat = m_pImpl->GetSectionFormat();
    if (pSectFormat)
    {
        pSectFormat->GetDoc()->DelSectionFormat( pSectFormat );
    }
}

void SAL_CALL SwXTextSection::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXTextSection::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextSection::getPropertySetInfo()
{
    SolarMutexGuard g;
    return m_pImpl->m_rPropSet.getPropertySetInfo();
}

static void
lcl_UpdateLinkType(SwSection & rSection, bool const bLinkUpdateAlways)
{
    if (rSection.GetType() == DDE_LINK_SECTION)
    {
        // set update type; needs an established link
        if (!rSection.IsConnected())
        {
            rSection.CreateLink(CREATE_CONNECT);
        }
        rSection.SetUpdateType( bLinkUpdateAlways
            ? SfxLinkUpdateMode::ALWAYS : SfxLinkUpdateMode::ONCALL );
    }
}

static void
lcl_UpdateSection(SwSectionFormat *const pFormat,
    std::unique_ptr<SwSectionData> const& pSectionData,
    std::unique_ptr<SfxItemSet> const& pItemSet,
    bool const bLinkModeChanged, bool const bLinkUpdateAlways = true)
{
    if (pFormat)
    {
        SwSection & rSection = *pFormat->GetSection();
        SwDoc *const pDoc = pFormat->GetDoc();
        SwSectionFormats const& rFormats = pDoc->GetSections();
        UnoActionContext aContext(pDoc);
        for (size_t i = 0; i < rFormats.size(); ++i)
        {
            if (rFormats[i]->GetSection()->GetSectionName()
                    == rSection.GetSectionName())
            {
                pDoc->UpdateSection(i, *pSectionData, pItemSet.get(),
                        pDoc->IsInReading());
                {
                    // temporarily remove actions to allow cursor update
                    // TODO: why? no table cursor here!
                    UnoActionRemoveContext aRemoveContext( pDoc );
                }

                if (bLinkModeChanged)
                {
                    lcl_UpdateLinkType(rSection, bLinkUpdateAlways);
                }
                // section found and processed: break from loop
                break;
            }
        }
    }
}

void SwXTextSection::Impl::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues)
{
    if(rPropertyNames.getLength() != rValues.getLength())
    {
        throw lang::IllegalArgumentException();
    }
    SwSectionFormat *const pFormat = GetSectionFormat();
    if (!pFormat && !m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    std::unique_ptr<SwSectionData> const pSectionData(
        pFormat ? new SwSectionData(*pFormat->GetSection()) : nullptr);

    OUString const*const pPropertyNames = rPropertyNames.getConstArray();
    uno::Any const*const pValues = rValues.getConstArray();
    std::unique_ptr<SfxItemSet> pItemSet;
    bool bLinkModeChanged = false;
    bool bLinkMode = false;

    for (sal_Int32 nProperty = 0; nProperty < rPropertyNames.getLength();
         nProperty++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            m_rPropSet.getPropertyMap().getByName(pPropertyNames[nProperty]);
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + pPropertyNames[nProperty],
                static_cast<cppu::OWeakObject *>(& m_rThis));
        }
        if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            throw beans::PropertyVetoException(
                "Property is read-only: " + pPropertyNames[nProperty],
                static_cast<cppu::OWeakObject *>(& m_rThis));
        }
        switch (pEntry->nWID)
        {
            case WID_SECT_CONDITION:
            {
                OUString uTmp;
                pValues[nProperty] >>= uTmp;
                if (m_bIsDescriptor)
                {
                    m_pProps->m_sCondition = uTmp;
                }
                else
                {
                    pSectionData->SetCondition(uTmp);
                }
            }
            break;
            case WID_SECT_DDE_TYPE:
            case WID_SECT_DDE_FILE:
            case WID_SECT_DDE_ELEMENT:
            {
                OUString sTmp;
                pValues[nProperty] >>= sTmp;
                if (m_bIsDescriptor)
                {
                    if (!m_pProps->m_bDDE)
                    {
                        m_pProps->m_sLinkFileName =
                            OUStringLiteral1(sfx2::cTokenSeparator) + OUStringLiteral1(sfx2::cTokenSeparator);
                        m_pProps->m_bDDE = true;
                    }
                    m_pProps->m_sLinkFileName = comphelper::string::setToken(
                        m_pProps->m_sLinkFileName,
                        pEntry->nWID - WID_SECT_DDE_TYPE, sfx2::cTokenSeparator, sTmp);
                }
                else
                {
                    OUString sLinkFileName(pSectionData->GetLinkFileName());
                    if (pSectionData->GetType() != DDE_LINK_SECTION)
                    {
                        sLinkFileName = OUStringLiteral1(sfx2::cTokenSeparator) + OUStringLiteral1(sfx2::cTokenSeparator);
                        pSectionData->SetType(DDE_LINK_SECTION);
                    }
                    sLinkFileName = comphelper::string::setToken(sLinkFileName,
                        pEntry->nWID - WID_SECT_DDE_TYPE,
                            sfx2::cTokenSeparator, sTmp);
                    pSectionData->SetLinkFileName(sLinkFileName);
                }
            }
            break;
            case WID_SECT_DDE_AUTOUPDATE:
            {
                bool bVal(false);
                if (!(pValues[nProperty] >>= bVal))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bUpdateType = bVal;
                }
                else
                {
                    bLinkModeChanged = true;
                    bLinkMode = bVal;
                }
            }
            break;
            case WID_SECT_LINK:
            {
                text::SectionFileLink aLink;
                if (!(pValues[nProperty] >>= aLink))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bDDE = false;
                    m_pProps->m_sLinkFileName = aLink.FileURL;
                    m_pProps->m_sSectionFilter = aLink.FilterName;
                }
                else
                {
                    if (pSectionData->GetType() != FILE_LINK_SECTION &&
                        !aLink.FileURL.isEmpty())
                    {
                        pSectionData->SetType(FILE_LINK_SECTION);
                    }
                    const OUString sTmp(!aLink.FileURL.isEmpty()
                        ? URIHelper::SmartRel2Abs(
                            pFormat->GetDoc()->GetDocShell()->GetMedium()->GetURLObject(),
                            aLink.FileURL, URIHelper::GetMaybeFileHdl())
                        : OUString());
                    const OUString sFileName(
                        sTmp + OUStringLiteral1(sfx2::cTokenSeparator) +
                        aLink.FilterName + OUStringLiteral1(sfx2::cTokenSeparator) +
                        pSectionData->GetLinkFileName().getToken(2, sfx2::cTokenSeparator));
                    pSectionData->SetLinkFileName(sFileName);
                    if (sFileName.getLength() < 3)
                    {
                        pSectionData->SetType(CONTENT_SECTION);
                    }
                }
            }
            break;
            case WID_SECT_REGION:
            {
                OUString sLink;
                pValues[nProperty] >>= sLink;
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bDDE = false;
                    m_pProps->m_sSectionRegion = sLink;
                }
                else
                {
                    if (pSectionData->GetType() != FILE_LINK_SECTION &&
                        !sLink.isEmpty())
                    {
                        pSectionData->SetType(FILE_LINK_SECTION);
                    }
                    OUString sSectLink(pSectionData->GetLinkFileName());
                    for (sal_Int32 i = comphelper::string::getTokenCount(sSectLink, sfx2::cTokenSeparator);
                         i < 3; ++i)
                    {
                        sSectLink += OUStringLiteral1(sfx2::cTokenSeparator);
                    }
                    sSectLink = comphelper::string::setToken(sSectLink, 2, sfx2::cTokenSeparator, sLink);
                    pSectionData->SetLinkFileName(sSectLink);
                    if (sSectLink.getLength() < 3)
                    {
                        pSectionData->SetType(CONTENT_SECTION);
                    }
                }
            }
            break;
            case WID_SECT_VISIBLE:
            {
                bool bVal(false);
                if (!(pValues[nProperty] >>= bVal))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bHidden = !bVal;
                }
                else
                {
                    pSectionData->SetHidden(!bVal);
                }
            }
            break;
            case WID_SECT_CURRENTLY_VISIBLE:
            {
                bool bVal(false);
                if (!(pValues[nProperty] >>= bVal))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bCondHidden = !bVal;
                }
                else
                {
                    if (!pSectionData->GetCondition().isEmpty())
                    {
                        pSectionData->SetCondHidden(!bVal);
                    }
                }
            }
            break;
            case WID_SECT_PROTECTED:
            {
                bool bVal(false);
                if (!(pValues[nProperty] >>= bVal))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bProtect = bVal;
                }
                else
                {
                    pSectionData->SetProtectFlag(bVal);
                }
            }
            break;
            case WID_SECT_EDIT_IN_READONLY:
            {
                bool bVal(false);
                if (!(pValues[nProperty] >>= bVal))
                {
                    throw lang::IllegalArgumentException();
                }
                if (m_bIsDescriptor)
                {
                    m_pProps->m_bEditInReadonly = bVal;
                }
                else
                {
                    pSectionData->SetEditInReadonlyFlag(bVal);
                }
            }
            break;
            case WID_SECT_PASSWORD:
            {
                uno::Sequence<sal_Int8> aSeq;
                pValues[nProperty] >>= aSeq;
                if (m_bIsDescriptor)
                {
                    m_pProps->m_Password = aSeq;
                }
                else
                {
                    pSectionData->SetPassword(aSeq);
                }
            }
            break;
            default:
            {
                if (pFormat)
                {
                    const SfxItemSet& rOldAttrSet = pFormat->GetAttrSet();
                    pItemSet.reset( new SfxItemSet(*rOldAttrSet.GetPool(), {{pEntry->nWID, pEntry->nWID}}));
                    pItemSet->Put(rOldAttrSet);
                    m_rPropSet.setPropertyValue(*pEntry,
                            pValues[nProperty], *pItemSet);
                }
                else
                {
                    SfxPoolItem* pPutItem = nullptr;
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pProps->m_pColItem)
                        {
                            m_pProps->m_pColItem.reset(new SwFormatCol);
                        }
                        pPutItem = m_pProps->m_pColItem.get();
                    }
                    else if (RES_BACKGROUND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pBrushItem)
                        {
                            m_pProps->m_pBrushItem.reset(
                                new SvxBrushItem(RES_BACKGROUND));
                        }
                        pPutItem = m_pProps->m_pBrushItem.get();
                    }
                    else if (RES_FTN_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFootnoteItem)
                        {
                            m_pProps->m_pFootnoteItem.reset(new SwFormatFootnoteAtTextEnd);
                        }
                        pPutItem = m_pProps->m_pFootnoteItem.get();
                    }
                    else if (RES_END_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pEndItem)
                        {
                            m_pProps->m_pEndItem.reset(new SwFormatEndAtTextEnd);
                        }
                        pPutItem = m_pProps->m_pEndItem.get();
                    }
                    else if (RES_UNKNOWNATR_CONTAINER== pEntry->nWID)
                    {
                        if (!m_pProps->m_pXMLAttr)
                        {
                            m_pProps->m_pXMLAttr.reset(
                                new SvXMLAttrContainerItem(
                                    RES_UNKNOWNATR_CONTAINER));
                        }
                        pPutItem = m_pProps->m_pXMLAttr.get();
                    }
                    else if (RES_COLUMNBALANCE== pEntry->nWID)
                    {
                        if (!m_pProps->m_pNoBalanceItem)
                        {
                            m_pProps->m_pNoBalanceItem.reset(
                                new SwFormatNoBalancedColumns(true));
                        }
                        pPutItem = m_pProps->m_pNoBalanceItem.get();
                    }
                    else if (RES_FRAMEDIR == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFrameDirItem)
                        {
                            m_pProps->m_pFrameDirItem.reset(
                                new SvxFrameDirectionItem(
                                SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR));
                        }
                        pPutItem = m_pProps->m_pFrameDirItem.get();
                    }
                    else if (RES_LR_SPACE == pEntry->nWID)
                    {
                        if (!m_pProps->m_pLRSpaceItem)
                        {
                            m_pProps->m_pLRSpaceItem.reset(
                                new SvxLRSpaceItem( RES_LR_SPACE ));
                        }
                        pPutItem = m_pProps->m_pLRSpaceItem.get();
                    }
                    if (pPutItem)
                    {
                        pPutItem->PutValue(pValues[nProperty],
                                pEntry->nMemberId);
                    }
                }
            }
        }
    }

    lcl_UpdateSection(pFormat, pSectionData, pItemSet, bLinkModeChanged,
        bLinkMode);
}

void SAL_CALL
SwXTextSection::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        m_pImpl->SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

void SwXTextSection::setPropertyValue(
    const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aPropertyNames { rPropertyName };
    uno::Sequence< uno::Any > aValues(1);
    aValues.getArray()[0] = rValue;
    m_pImpl->SetPropertyValues_Impl( aPropertyNames, aValues );
}

uno::Sequence< uno::Any >
SwXTextSection::Impl::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
{
    SwSectionFormat *const pFormat = GetSectionFormat();
    if (!pFormat && !m_bIsDescriptor)
    {
        throw uno::RuntimeException( "non-descriptor section without format");
    }

    uno::Sequence< uno::Any > aRet(rPropertyNames.getLength());
    uno::Any* pRet = aRet.getArray();
    SwSection *const pSect = pFormat ? pFormat->GetSection() : nullptr;
    const OUString* pPropertyNames = rPropertyNames.getConstArray();

    for (sal_Int32 nProperty = 0; nProperty < rPropertyNames.getLength();
        nProperty++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            m_rPropSet.getPropertyMap().getByName(pPropertyNames[nProperty]);
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + pPropertyNames[nProperty],
                static_cast<cppu::OWeakObject *>(& m_rThis));
        }
        switch(pEntry->nWID)
        {
            case WID_SECT_CONDITION:
            {
                const OUString uTmp( m_bIsDescriptor
                    ? m_pProps->m_sCondition
                    : pSect->GetCondition());
                pRet[nProperty] <<= uTmp;
            }
            break;
            case WID_SECT_DDE_TYPE:
            case WID_SECT_DDE_FILE:
            case WID_SECT_DDE_ELEMENT:
            {
                OUString sRet;
                if (m_bIsDescriptor)
                {
                    if (m_pProps->m_bDDE)
                    {
                        sRet = m_pProps->m_sLinkFileName;
                    }
                }
                else if (DDE_LINK_SECTION == pSect->GetType())
                {
                    sRet = pSect->GetLinkFileName();
                }
                pRet[nProperty] <<= sRet.getToken(pEntry->nWID - WID_SECT_DDE_TYPE,
                    sfx2::cTokenSeparator);
            }
            break;
            case WID_SECT_DDE_AUTOUPDATE:
            {
                // GetUpdateType() returns .._ALWAYS or .._ONCALL
                if (pSect && pSect->IsLinkType() && pSect->IsConnected())  // #i73247#
                {
                    const bool bTemp =
                        (pSect->GetUpdateType() == SfxLinkUpdateMode::ALWAYS);
                    pRet[nProperty] <<= bTemp;
                }
            }
            break;
            case WID_SECT_LINK     :
            {
                text::SectionFileLink aLink;
                if (m_bIsDescriptor)
                {
                    if (!m_pProps->m_bDDE)
                    {
                        aLink.FileURL = m_pProps->m_sLinkFileName;
                        aLink.FilterName = m_pProps->m_sSectionFilter;
                    }
                }
                else if (FILE_LINK_SECTION == pSect->GetType())
                {
                    const OUString& sRet( pSect->GetLinkFileName() );
                    sal_Int32 nIndex(0);
                    aLink.FileURL =
                        sRet.getToken(0, sfx2::cTokenSeparator, nIndex);
                    aLink.FilterName =
                        sRet.getToken(0, sfx2::cTokenSeparator, nIndex);
                }
                pRet[nProperty] <<= aLink;
            }
            break;
            case WID_SECT_REGION :
            {
                OUString sRet;
                if (m_bIsDescriptor)
                {
                    sRet = m_pProps->m_sSectionRegion;
                }
                else if (FILE_LINK_SECTION == pSect->GetType())
                {
                    sRet = pSect->GetLinkFileName().getToken(2,
                            sfx2::cTokenSeparator);
                }
                pRet[nProperty] <<= sRet;
            }
            break;
            case WID_SECT_VISIBLE   :
            {
                const bool bTemp = m_bIsDescriptor
                    ? !m_pProps->m_bHidden : !pSect->IsHidden();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_CURRENTLY_VISIBLE:
            {
                const bool bTemp = m_bIsDescriptor
                    ? !m_pProps->m_bCondHidden : !pSect->IsCondHidden();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_PROTECTED:
            {
                const bool bTemp = m_bIsDescriptor
                    ? m_pProps->m_bProtect : pSect->IsProtect();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_EDIT_IN_READONLY:
            {
                const bool bTemp = m_bIsDescriptor
                    ? m_pProps->m_bEditInReadonly : pSect->IsEditInReadonly();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case  FN_PARAM_LINK_DISPLAY_NAME:
            {
                if (pFormat)
                {
                    pRet[nProperty] <<= pFormat->GetSection()->GetSectionName();
                }
            }
            break;
            case WID_SECT_DOCUMENT_INDEX:
            {
                // search enclosing index
                SwSection* pEnclosingSection = pSect;
                while ((pEnclosingSection != nullptr) &&
                       (TOX_CONTENT_SECTION != pEnclosingSection->GetType()))
                {
                    pEnclosingSection = pEnclosingSection->GetParent();
                }
                SwTOXBaseSection* const pTOXBaseSect = pEnclosingSection ?
                    dynamic_cast<SwTOXBaseSection*>( pEnclosingSection ) : nullptr;
                if (pTOXBaseSect)
                {
                    // convert section to TOXBase and get SwXDocumentIndex
                    const uno::Reference<text::XDocumentIndex> xIndex =
                        SwXDocumentIndex::CreateXDocumentIndex(
                            *pTOXBaseSect->GetFormat()->GetDoc(), pTOXBaseSect);
                    pRet[nProperty] <<= xIndex;
                }
                // else: no enclosing index found -> empty return value
            }
            break;
            case WID_SECT_IS_GLOBAL_DOC_SECTION:
            {
                const bool bRet = pFormat && (nullptr != pFormat->GetGlobalDocSection());
                pRet[nProperty] <<= bRet;
            }
            break;
            case  FN_UNO_ANCHOR_TYPES:
            case  FN_UNO_TEXT_WRAP:
            case  FN_UNO_ANCHOR_TYPE:
                ::sw::GetDefaultTextContentValue(
                        pRet[nProperty], OUString(), pEntry->nWID);
            break;
            case FN_UNO_REDLINE_NODE_START:
            case FN_UNO_REDLINE_NODE_END:
            {
                if (!pFormat)
                    break;      // #i73247#
                SwNode* pSectNode = pFormat->GetSectionNode();
                if (FN_UNO_REDLINE_NODE_END == pEntry->nWID)
                {
                    pSectNode = pSectNode->EndOfSectionNode();
                }
                const SwRedlineTable& rRedTable =
                    pFormat->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
                for (SwRangeRedline* pRedline : rRedTable)
                {
                    const SwNode& rRedPointNode = pRedline->GetNode();
                    const SwNode& rRedMarkNode = pRedline->GetNode(false);
                    if ((&rRedPointNode == pSectNode) ||
                        (&rRedMarkNode == pSectNode))
                    {
                        const SwNode& rStartOfRedline =
                            (SwNodeIndex(rRedPointNode) <=
                             SwNodeIndex(rRedMarkNode))
                                 ? rRedPointNode : rRedMarkNode;
                        const bool bIsStart = (&rStartOfRedline == pSectNode);
                        pRet[nProperty] <<=
                            SwXRedlinePortion::CreateRedlineProperties(
                                    *pRedline, bIsStart);
                        break;
                    }
                }
            }
            break;
            case WID_SECT_PASSWORD:
            {
                pRet[nProperty] <<= m_bIsDescriptor
                    ? m_pProps->m_Password : pSect->GetPassword();
            }
            break;
            default:
            {
                if (pFormat)
                {
                    m_rPropSet.getPropertyValue(*pEntry,
                            pFormat->GetAttrSet(), pRet[nProperty]);
                }
                else
                {
                    const SfxPoolItem* pQueryItem = nullptr;
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pProps->m_pColItem)
                        {
                            m_pProps->m_pColItem.reset(new SwFormatCol);
                        }
                        pQueryItem = m_pProps->m_pColItem.get();
                    }
                    else if (RES_BACKGROUND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pBrushItem)
                        {
                            m_pProps->m_pBrushItem.reset(
                                new SvxBrushItem(RES_BACKGROUND));
                        }
                        pQueryItem = m_pProps->m_pBrushItem.get();
                    }
                    else if (RES_FTN_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFootnoteItem)
                        {
                            m_pProps->m_pFootnoteItem.reset(new SwFormatFootnoteAtTextEnd);
                        }
                        pQueryItem = m_pProps->m_pFootnoteItem.get();
                    }
                    else if (RES_END_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pEndItem)
                        {
                            m_pProps->m_pEndItem.reset(new SwFormatEndAtTextEnd);
                        }
                        pQueryItem = m_pProps->m_pEndItem.get();
                    }
                    else if (RES_UNKNOWNATR_CONTAINER== pEntry->nWID)
                    {
                        if (!m_pProps->m_pXMLAttr)
                        {
                            m_pProps->m_pXMLAttr.reset(
                                new SvXMLAttrContainerItem);
                        }
                        pQueryItem = m_pProps->m_pXMLAttr.get();
                    }
                    else if (RES_COLUMNBALANCE== pEntry->nWID)
                    {
                        if (!m_pProps->m_pNoBalanceItem)
                        {
                            m_pProps->m_pNoBalanceItem.reset(
                                new SwFormatNoBalancedColumns);
                        }
                        pQueryItem = m_pProps->m_pNoBalanceItem.get();
                    }
                    else if (RES_FRAMEDIR == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFrameDirItem)
                        {
                            m_pProps->m_pFrameDirItem.reset(
                                new SvxFrameDirectionItem(
                                    SvxFrameDirection::Environment, RES_FRAMEDIR));
                        }
                        pQueryItem = m_pProps->m_pFrameDirItem.get();
                    }
                    else if (RES_LR_SPACE == pEntry->nWID)
                    {
                        if (!m_pProps->m_pLRSpaceItem)
                        {
                            m_pProps->m_pLRSpaceItem.reset(
                                new SvxLRSpaceItem( RES_LR_SPACE ));
                        }
                        pQueryItem = m_pProps->m_pLRSpaceItem.get();
                    }
                    if (pQueryItem)
                    {
                        pQueryItem->QueryValue(pRet[nProperty],
                                pEntry->nMemberId);
                    }
                }
            }
        }
    }
    return aRet;
}

uno::Sequence< uno::Any > SAL_CALL
SwXTextSection::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = m_pImpl->GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("Unknown property exception caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }
    catch (lang::WrappedTargetException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException("WrappedTargetException caught",
                static_cast < cppu::OWeakObject * > ( this ), anyEx );
    }

    return aValues;
}

uno::Any SAL_CALL
SwXTextSection::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aPropertyNames { rPropertyName };
    return m_pImpl->GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}

void SAL_CALL SwXTextSection::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXTextSection::addPropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXTextSection::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXTextSection::removePropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXTextSection::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
{
    OSL_FAIL("SwXTextSection::firePropertiesChangeEvent(): not implemented");
}

void SAL_CALL
SwXTextSection::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextSection::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextSection::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextSection::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextSection::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextSection::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames { rPropertyName };
    return getPropertyStates(aNames).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextSection::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pFormat = m_pImpl->GetSectionFormat();
    if (!pFormat && !m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    uno::Sequence< beans::PropertyState > aStates(rPropertyNames.getLength());
    beans::PropertyState *const pStates = aStates.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();
    for (sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
    {
        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
        SfxItemPropertySimpleEntry const*const pEntry =
            m_pImpl->m_rPropSet.getPropertyMap().getByName( pNames[i]);
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                "Unknown property: " + pNames[i],
                static_cast< cppu::OWeakObject* >(this));
        }
        switch (pEntry->nWID)
        {
            case WID_SECT_CONDITION:
            case WID_SECT_DDE_TYPE:
            case WID_SECT_DDE_FILE:
            case WID_SECT_DDE_ELEMENT:
            case WID_SECT_DDE_AUTOUPDATE:
            case WID_SECT_LINK:
            case WID_SECT_REGION :
            case WID_SECT_VISIBLE:
            case WID_SECT_PROTECTED:
            case WID_SECT_EDIT_IN_READONLY:
            case  FN_PARAM_LINK_DISPLAY_NAME:
            case  FN_UNO_ANCHOR_TYPES:
            case  FN_UNO_TEXT_WRAP:
            case  FN_UNO_ANCHOR_TYPE:
                pStates[i] = beans::PropertyState_DIRECT_VALUE;
            break;
            default:
            {
                if (pFormat)
                {
                    pStates[i] = m_pImpl->m_rPropSet.getPropertyState(
                                    pNames[i], pFormat->GetAttrSet());
                }
                else
                {
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pImpl->m_pProps->m_pColItem)
                        {
                            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                        }
                        else
                        {
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                        }
                    }
                    else
                    {
                        if (!m_pImpl->m_pProps->m_pBrushItem)
                        {
                            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                        }
                        else
                        {
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                        }
                    }
                }
            }
        }
    }
    return aStates;
}

void SAL_CALL
SwXTextSection::setPropertyToDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pFormat = m_pImpl->GetSectionFormat();
    if (!pFormat && !m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast< cppu::OWeakObject* >(this));
    }
    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(
            "Property is read-only: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    std::unique_ptr<SwSectionData> const pSectionData(
        pFormat ? new SwSectionData(*pFormat->GetSection()) : nullptr);

    std::unique_ptr<SfxItemSet> pNewAttrSet;
    bool bLinkModeChanged = false;

    switch (pEntry->nWID)
    {
        case WID_SECT_CONDITION:
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_sCondition.clear();
            }
            else
            {
                pSectionData->SetCondition(OUString());
            }
        }
        break;
        case WID_SECT_DDE_TYPE      :
        case WID_SECT_DDE_FILE      :
        case WID_SECT_DDE_ELEMENT   :
        case WID_SECT_LINK     :
        case WID_SECT_REGION :
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_bDDE = false;
                m_pImpl->m_pProps->m_sLinkFileName.clear();
                m_pImpl->m_pProps->m_sSectionRegion.clear();
                m_pImpl->m_pProps->m_sSectionFilter.clear();
            }
            else
            {
                pSectionData->SetType(CONTENT_SECTION);
            }
        break;
        case WID_SECT_DDE_AUTOUPDATE:
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_bUpdateType = true;
            }
            else
            {
                bLinkModeChanged = true;
            }
        break;
        case WID_SECT_VISIBLE   :
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_bHidden = false;
            }
            else
            {
                pSectionData->SetHidden(false);
            }
        }
        break;
        case WID_SECT_PROTECTED:
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_bProtect = false;
            }
            else
            {
                pSectionData->SetProtectFlag(false);
            }
        }
        break;
        case WID_SECT_EDIT_IN_READONLY:
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_bEditInReadonly = false;
            }
            else
            {
                pSectionData->SetEditInReadonlyFlag(false);
            }
        }
        break;

        case  FN_UNO_ANCHOR_TYPES:
        case  FN_UNO_TEXT_WRAP:
        case  FN_UNO_ANCHOR_TYPE:
        break;
        default:
        {
            if (SfxItemPool::IsWhich(pEntry->nWID))
            {
                if (pFormat)
                {
                    const SfxItemSet& rOldAttrSet = pFormat->GetAttrSet();
                    pNewAttrSet.reset( new SfxItemSet(*rOldAttrSet.GetPool(), {{pEntry->nWID, pEntry->nWID}}));
                    pNewAttrSet->ClearItem(pEntry->nWID);
                }
                else
                {
                    if (RES_COL == pEntry->nWID)
                    {
                        m_pImpl->m_pProps->m_pColItem.reset();
                    }
                    else if (RES_BACKGROUND == pEntry->nWID)
                    {
                        m_pImpl->m_pProps->m_pBrushItem.reset();
                    }
                }
            }
        }
    }

    lcl_UpdateSection(pFormat, pSectionData, pNewAttrSet, bLinkModeChanged);
}

uno::Any SAL_CALL
SwXTextSection::getPropertyDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    SwSectionFormat *const pFormat = m_pImpl->GetSectionFormat();
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    switch(pEntry->nWID)
    {
        case WID_SECT_CONDITION:
        case WID_SECT_DDE_TYPE      :
        case WID_SECT_DDE_FILE      :
        case WID_SECT_DDE_ELEMENT   :
        case WID_SECT_REGION :
        case FN_PARAM_LINK_DISPLAY_NAME:
            aRet <<= OUString();
        break;
        case WID_SECT_LINK     :
            aRet <<= text::SectionFileLink();
        break;
        case WID_SECT_DDE_AUTOUPDATE:
        case WID_SECT_VISIBLE   :
            aRet <<= true;
        break;
        case WID_SECT_PROTECTED:
        case WID_SECT_EDIT_IN_READONLY:
            aRet <<= false;
        break;
        case  FN_UNO_ANCHOR_TYPES:
        case  FN_UNO_TEXT_WRAP:
        case  FN_UNO_ANCHOR_TYPE:
            ::sw::GetDefaultTextContentValue(aRet, OUString(), pEntry->nWID);
        break;
        default:
        if(pFormat && SfxItemPool::IsWhich(pEntry->nWID))
        {
            SwDoc *const pDoc = pFormat->GetDoc();
            const SfxPoolItem& rDefItem =
                pDoc->GetAttrPool().GetDefaultItem(pEntry->nWID);
            rDefItem.QueryValue(aRet, pEntry->nMemberId);
        }
    }
    return aRet;
}

OUString SAL_CALL SwXTextSection::getName()
{
    SolarMutexGuard aGuard;

    OUString sRet;
    SwSectionFormat const*const pFormat = m_pImpl->GetSectionFormat();
    if(pFormat)
    {
        sRet = pFormat->GetSection()->GetSectionName();
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        sRet = m_pImpl->m_sName;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return sRet;
}

void SAL_CALL SwXTextSection::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;

    SwSectionFormat *const pFormat = m_pImpl->GetSectionFormat();
    if(pFormat)
    {
        SwSection *const pSect = pFormat->GetSection();
        SwSectionData aSection(*pSect);
        aSection.SetSectionName(rName);

        const SwSectionFormats& rFormats = pFormat->GetDoc()->GetSections();
        size_t nApplyPos = SIZE_MAX;
        for( size_t i = 0; i < rFormats.size(); ++i )
        {
            if(rFormats[i]->GetSection() == pSect)
            {
                nApplyPos = i;
            }
            else if (rName == rFormats[i]->GetSection()->GetSectionName())
            {
                throw uno::RuntimeException();
            }
        }
        if (nApplyPos != SIZE_MAX)
        {
            {
                UnoActionContext aContext(pFormat->GetDoc());
                pFormat->GetDoc()->UpdateSection(nApplyPos, aSection);
            }
            {
                // temporarily remove actions to allow cursor update
                // TODO: why? no table cursor here!
                UnoActionRemoveContext aRemoveContext( pFormat->GetDoc() );
            }
        }
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_sName = rName;
    }
    else
    {
        throw uno::RuntimeException();
    }
}

OUString SAL_CALL
SwXTextSection::getImplementationName()
{
    return OUString("SwXTextSection");
}

static char const*const g_ServicesTextSection[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.TextSection",
    "com.sun.star.document.LinkTarget",
};

sal_Bool SAL_CALL SwXTextSection::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextSection::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
                SAL_N_ELEMENTS(g_ServicesTextSection),
                g_ServicesTextSection);
}

// MetadatableMixin
::sfx2::Metadatable* SwXTextSection::GetCoreObject()
{
    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    return pSectionFormat;
}

uno::Reference<frame::XModel> SwXTextSection::GetModel()
{
    SwSectionFormat *const pSectionFormat( m_pImpl->GetSectionFormat() );
    if (pSectionFormat)
    {
        SwDocShell const*const pShell( pSectionFormat->GetDoc()->GetDocShell() );
        return pShell ? pShell->GetModel() : nullptr;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
