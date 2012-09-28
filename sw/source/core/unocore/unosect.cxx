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


#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>

#include <cmdid.h>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtclds.hxx>
#include <unotextrange.hxx>
#include <unosection.hxx>
#include <TextCursorHelper.hxx>
#include <unoredline.hxx>
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
using ::rtl::OUString;

struct SwTextSectionProperties_Impl
{
    uno::Sequence<sal_Int8> m_Password;
    ::rtl::OUString  m_sCondition;
    ::rtl::OUString  m_sLinkFileName;
    ::rtl::OUString  m_sSectionFilter;
    ::rtl::OUString  m_sSectionRegion;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwFmtCol>               m_pColItem;
    ::std::auto_ptr<SvxBrushItem>           m_pBrushItem;
    ::std::auto_ptr<SwFmtFtnAtTxtEnd>       m_pFtnItem;
    ::std::auto_ptr<SwFmtEndAtTxtEnd>       m_pEndItem;
    ::std::auto_ptr<SvXMLAttrContainerItem> m_pXMLAttr;
    ::std::auto_ptr<SwFmtNoBalancedColumns> m_pNoBalanceItem;
    ::std::auto_ptr<SvxFrameDirectionItem>  m_pFrameDirItem;
    ::std::auto_ptr<SvxLRSpaceItem>         m_pLRSpaceItem;
    SAL_WNODEPRECATED_DECLARATIONS_POP

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
    : public SwClient
{

public:

    SwXTextSection &            m_rThis;
    const SfxItemPropertySet &  m_rPropSet;
    SwEventListenerContainer    m_ListenerContainer;
    const bool                  m_bIndexHeader;
    bool                        m_bIsDescriptor;
    ::rtl::OUString             m_sName;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwTextSectionProperties_Impl> m_pProps;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    Impl(   SwXTextSection & rThis,
            SwSectionFmt *const pFmt, const bool bIndexHeader)
        : SwClient(pFmt)
        , m_rThis(rThis)
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_SECTION))
        , m_ListenerContainer(static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_bIndexHeader(bIndexHeader)
        // #i111177# unxsols4 (Sun C++ 5.9 SunOS_sparc) may generate wrong code
        , m_bIsDescriptor((0 == pFmt) ? true : false)
        , m_pProps((pFmt) ? 0 : new SwTextSectionProperties_Impl())
    {
    }

    SwSectionFmt * GetSectionFmt() const
    {
        return static_cast<SwSectionFmt*>(const_cast<SwModify*>(
                    GetRegisteredIn()));
    }

    SwSectionFmt & GetSectionFmtOrThrow() const {
        SwSectionFmt *const pFmt( GetSectionFmt() );
        if (!pFmt) {
            throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "SwXTextSection: disposed or invalid")), 0);
        }
        return *pFmt;
    }

    void SAL_CALL SetPropertyValues_Impl(
            const uno::Sequence< ::rtl::OUString >& rPropertyNames,
            const uno::Sequence< uno::Any >& aValues)
        throw (beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException);
    uno::Sequence< uno::Any > SAL_CALL
        GetPropertyValues_Impl(
            const uno::Sequence< ::rtl::OUString >& rPropertyNames)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                uno::RuntimeException);
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXTextSection::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (!GetRegisteredIn())
    {
        m_ListenerContainer.Disposing();
    }
}

SwSectionFmt * SwXTextSection::GetFmt() const
{
    return m_pImpl->GetSectionFmt();
}

uno::Reference< text::XTextSection >
SwXTextSection::CreateXTextSection(
        SwSectionFmt *const pFmt, const bool bIndexHeader)
{
    // re-use existing SwXTextSection
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference< text::XTextSection > xSection;
    if (pFmt)
    {
        xSection.set(pFmt->GetXTextSection());
    }
    if ( !xSection.is() )
    {
        SwXTextSection *const pNew = new SwXTextSection(pFmt, bIndexHeader);
        xSection.set(pNew);
        if (pFmt)
        {
            pFmt->SetXTextSection(xSection);
        }
    }
    return xSection;
}

SwXTextSection::SwXTextSection(
        SwSectionFmt *const pFmt, const bool bIndexHeader)
    : m_pImpl( new SwXTextSection::Impl(*this, pFmt, bIndexHeader) )
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
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextSection>(rId, this);
}

uno::Reference< text::XTextSection > SAL_CALL
SwXTextSection::getParentSection() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt & rSectionFmt( m_pImpl->GetSectionFmtOrThrow() );

    SwSectionFmt *const pParentFmt = rSectionFmt.GetParent();
    const uno::Reference< text::XTextSection > xRet =
        (pParentFmt) ? CreateXTextSection(pParentFmt) : 0;
    return xRet;
}

uno::Sequence< uno::Reference< text::XTextSection > > SAL_CALL
SwXTextSection::getChildSections() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt & rSectionFmt( m_pImpl->GetSectionFmtOrThrow() );

    SwSections aChildren;
    rSectionFmt.GetChildSections(aChildren, SORTSECT_NOT, sal_False);
    uno::Sequence<uno::Reference<text::XTextSection> > aSeq(aChildren.size());
    uno::Reference< text::XTextSection > * pArray = aSeq.getArray();
    for (sal_uInt16 i = 0; i < aChildren.size(); i++)
    {
        SwSectionFmt *const pChild = aChildren[i]->GetFmt();
        pArray[i] = CreateXTextSection(pChild);
    }
    return aSeq;
}

void SAL_CALL
SwXTextSection::attach(const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    SwDoc *const pDoc =
        (pRange) ? pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : 0);
    if (!pDoc)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDoc);
    //das muss jetzt sal_True liefern
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aCont(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_INSSECTION, NULL );

    if (m_pImpl->m_sName.isEmpty())
    {
        m_pImpl->m_sName = C2U("TextSection");
    }
    SectionType eType = (m_pImpl->m_pProps->m_bDDE)
        ? DDE_LINK_SECTION
        : ((!m_pImpl->m_pProps->m_sLinkFileName.isEmpty() ||
            !m_pImpl->m_pProps->m_sSectionRegion.isEmpty())
                ?  FILE_LINK_SECTION : CONTENT_SECTION);
    // index header section?
    if (m_pImpl->m_bIndexHeader)
    {
        // caller wants an index header section, but will only
        // give him one if a) we are inside an index, and b) said
        // index doesn't yet have a header section.
        const SwTOXBase* pBase = aPam.GetDoc()->GetCurTOX(*aPam.Start());

        // are we inside an index?
        if (pBase)
        {
            // get all child sections
            SwSections aSectionsArr;
            static_cast<const SwTOXBaseSection*>(pBase)->GetFmt()->
                GetChildSections(aSectionsArr);

            // and search for current header section
            const sal_uInt16 nCount = aSectionsArr.size();
            sal_Bool bHeaderPresent = sal_False;
            for(sal_uInt16 i = 0; i < nCount; i++)
            {
                bHeaderPresent |=
                    (aSectionsArr[i]->GetType() == TOX_HEADER_SECTION);
            }
            if (! bHeaderPresent)
            {
                eType = TOX_HEADER_SECTION;
            }
        }
    }

    String tmp(m_pImpl->m_sName);
    SwSectionData aSect(eType, pDoc->GetUniqueSectionName(&tmp));
    aSect.SetCondition(m_pImpl->m_pProps->m_sCondition);
    ::rtl::OUStringBuffer sLinkNameBuf(m_pImpl->m_pProps->m_sLinkFileName);
    sLinkNameBuf.append(sfx2::cTokenSeperator);
    sLinkNameBuf.append(m_pImpl->m_pProps->m_sSectionFilter);
    sLinkNameBuf.append(sfx2::cTokenSeperator);
    sLinkNameBuf.append(m_pImpl->m_pProps->m_sSectionRegion);
    aSect.SetLinkFileName(sLinkNameBuf.makeStringAndClear());

    aSect.SetHidden(m_pImpl->m_pProps->m_bHidden);
    aSect.SetProtectFlag(m_pImpl->m_pProps->m_bProtect);
    aSect.SetEditInReadonlyFlag(m_pImpl->m_pProps->m_bEditInReadonly);

    SfxItemSet aSet(pDoc->GetAttrPool(),
                RES_COL, RES_COL,
                RES_BACKGROUND, RES_BACKGROUND,
                RES_FTN_AT_TXTEND, RES_FRAMEDIR,
                RES_LR_SPACE, RES_LR_SPACE,
                RES_UNKNOWNATR_CONTAINER,RES_UNKNOWNATR_CONTAINER,
                0);
    if (m_pImpl->m_pProps->m_pBrushItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pBrushItem);
    }
    if (m_pImpl->m_pProps->m_pColItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pColItem);
    }
    if (m_pImpl->m_pProps->m_pFtnItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pFtnItem);
    }
    if (m_pImpl->m_pProps->m_pEndItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pEndItem);
    }
    if (m_pImpl->m_pProps->m_pXMLAttr.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pXMLAttr);
    }
    if (m_pImpl->m_pProps->m_pNoBalanceItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pNoBalanceItem);
    }
    if (m_pImpl->m_pProps->m_pFrameDirItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pFrameDirItem);
    }
    if (m_pImpl->m_pProps->m_pLRSpaceItem.get())
    {
        aSet.Put(*m_pImpl->m_pProps->m_pLRSpaceItem);
    }
    // section password
    if (m_pImpl->m_pProps->m_Password.getLength() > 0)
    {
        aSect.SetPassword(m_pImpl->m_pProps->m_Password);
    }

    SwSection *const pRet =
        pDoc->InsertSwSection( aPam, aSect, 0, aSet.Count() ? &aSet : 0 );
    if (!pRet) // fdo#42450 text range could parially overlap existing section
    {
        // shouldn't have created an undo object yet
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_INSSECTION, NULL );
        throw lang::IllegalArgumentException(
                "SwXTextSection::attach(): invalid TextRange",
                static_cast< ::cppu::OWeakObject*>(this), 0);
    }
    pRet->GetFmt()->Add(m_pImpl.get());
    pRet->GetFmt()->SetXObject(static_cast< ::cppu::OWeakObject*>(this));

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
        pRet->SetUpdateType( static_cast< sal_uInt16 >(
            (m_pImpl->m_pProps->m_bUpdateType) ?
                sfx2::LINKUPDATE_ALWAYS : sfx2::LINKUPDATE_ONCALL) );
    }

    // Undo-Klammerung hier beenden
    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_INSSECTION, NULL );
    m_pImpl->m_pProps.reset();
    m_pImpl->m_bIsDescriptor = false;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextSection::getAnchor() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XTextRange >  xRet;
    SwSectionFmt *const pSectFmt = m_pImpl->GetSectionFmt();
    if(pSectFmt)
    {
        const SwNodeIndex* pIdx;
        if( 0 != ( pSectFmt->GetSection() ) &&
            0 != ( pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, fnGoCntnt );

            const SwEndNode* pEndNode = pIdx->GetNode().EndOfSectionNode();
            SwPaM aEnd(*pEndNode);
            aEnd.Move( fnMoveBackward, fnGoCntnt );
            xRet = SwXTextRange::CreateXTextRange(*pSectFmt->GetDoc(),
                *aPaM.Start(), aEnd.Start());
        }
    }
    return xRet;
}

void SAL_CALL SwXTextSection::dispose() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt *const pSectFmt = m_pImpl->GetSectionFmt();
    if (pSectFmt)
    {
        pSectFmt->GetDoc()->DelSectionFmt( pSectFmt );
    }
}

void SAL_CALL SwXTextSection::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetSectionFmt())
    {
        throw uno::RuntimeException();
    }
    m_pImpl->m_ListenerContainer.AddListener(xListener);
}

void SAL_CALL SwXTextSection::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetSectionFmt() ||
        !m_pImpl->m_ListenerContainer.RemoveListener(xListener))
    {
        throw uno::RuntimeException();
    }
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextSection::getPropertySetInfo() throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    static const uno::Reference< beans::XPropertySetInfo >  aRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return aRef;
}

static void
lcl_UpdateLinkType(SwSection & rSection, bool const bLinkUpdateAlways = true)
{
    if (rSection.GetType() == DDE_LINK_SECTION)
    {
        // set update type; needs an established link
        if (!rSection.IsConnected())
        {
            rSection.CreateLink(CREATE_CONNECT);
        }
        rSection.SetUpdateType( static_cast< sal_uInt16 >((bLinkUpdateAlways)
            ? sfx2::LINKUPDATE_ALWAYS : sfx2::LINKUPDATE_ONCALL) );
    }
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
static void
lcl_UpdateSection(SwSectionFmt *const pFmt,
    ::std::auto_ptr<SwSectionData> const& pSectionData,
    ::std::auto_ptr<SfxItemSet> const& pItemSet,
    bool const bLinkModeChanged, bool const bLinkUpdateAlways = true)
{
    if (pFmt)
    {
        SwSection & rSection = *pFmt->GetSection();
        SwDoc *const pDoc = pFmt->GetDoc();
        SwSectionFmts const& rFmts = pDoc->GetSections();
        UnoActionContext aContext(pDoc);
        for (sal_uInt16 i = 0; i < rFmts.size(); i++)
        {
            if (rFmts[i]->GetSection()->GetSectionName()
                    == rSection.GetSectionName())
            {
                pDoc->UpdateSection(i, *pSectionData, pItemSet.get(),
                        pDoc->IsInReading());
                {
                    // temporarily remove actions to allow cursor update
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
SAL_WNODEPRECATED_DECLARATIONS_POP

void SwXTextSection::Impl::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    if(rPropertyNames.getLength() != rValues.getLength())
    {
        throw lang::IllegalArgumentException();
    }
    SwSectionFmt *const pFmt = GetSectionFmt();
    if (!pFmt && !m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwSectionData> const pSectionData(
        (pFmt) ? new SwSectionData(*pFmt->GetSection()) : 0);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    OUString const*const pPropertyNames = rPropertyNames.getConstArray();
    uno::Any const*const pValues = rValues.getConstArray();
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxItemSet> pItemSet;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_Bool bLinkModeChanged = sal_False;
    sal_Bool bLinkMode = sal_False;

    for (sal_Int32 nProperty = 0; nProperty < rPropertyNames.getLength();
         nProperty++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            m_rPropSet.getPropertyMap().getByName(pPropertyNames[nProperty]);
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + pPropertyNames[nProperty],
                static_cast<cppu::OWeakObject *>(& m_rThis));
        }
        if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            throw beans::PropertyVetoException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only: "))
                    + pPropertyNames[nProperty],
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
                OUString uTmp;
                pValues[nProperty] >>= uTmp;
                String sTmp(uTmp);
                if (m_bIsDescriptor)
                {
                    if (!m_pProps->m_bDDE)
                    {
                        ::rtl::OUStringBuffer buf;
                        buf.append(sfx2::cTokenSeperator);
                        buf.append(sfx2::cTokenSeperator);
                        m_pProps->m_sLinkFileName = buf.makeStringAndClear();
                        m_pProps->m_bDDE = true;
                    }
                    String sLinkFileName(m_pProps->m_sLinkFileName);
                    sLinkFileName.SetToken(pEntry->nWID - WID_SECT_DDE_TYPE,
                            sfx2::cTokenSeperator, sTmp);
                    m_pProps->m_sLinkFileName = sLinkFileName;
                }
                else
                {
                    String sLinkFileName(pSectionData->GetLinkFileName());
                    if (pSectionData->GetType() != DDE_LINK_SECTION)
                    {
                        sLinkFileName = sfx2::cTokenSeperator;
                        sLinkFileName += sfx2::cTokenSeperator;
                        pSectionData->SetType(DDE_LINK_SECTION);
                    }
                    sLinkFileName.SetToken(pEntry->nWID - WID_SECT_DDE_TYPE,
                            sfx2::cTokenSeperator, sTmp);
                    pSectionData->SetLinkFileName(sLinkFileName);
                }
            }
            break;
            case WID_SECT_DDE_AUTOUPDATE:
            {
                sal_Bool bVal(sal_False);
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
                    bLinkModeChanged = sal_True;
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
                    m_pProps->m_bDDE = sal_False;
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
                    ::rtl::OUStringBuffer sFileNameBuf;
                    if (!aLink.FileURL.isEmpty())
                    {
                        sFileNameBuf.append( URIHelper::SmartRel2Abs(
                            pFmt->GetDoc()->GetDocShell()->GetMedium()
                                ->GetURLObject(),
                            aLink.FileURL, URIHelper::GetMaybeFileHdl()));
                    }
                    sFileNameBuf.append(sfx2::cTokenSeperator);
                    sFileNameBuf.append(aLink.FilterName);
                    sFileNameBuf.append(sfx2::cTokenSeperator);
                    sFileNameBuf.append(
                        pSectionData->GetLinkFileName().GetToken(2,
                            sfx2::cTokenSeperator));
                    const ::rtl::OUString sFileName(
                            sFileNameBuf.makeStringAndClear());
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
                    m_pProps->m_bDDE = sal_False;
                    m_pProps->m_sSectionRegion = sLink;
                }
                else
                {
                    if (pSectionData->GetType() != FILE_LINK_SECTION &&
                        !sLink.isEmpty())
                    {
                        pSectionData->SetType(FILE_LINK_SECTION);
                    }
                    String sSectLink(pSectionData->GetLinkFileName());
                    while (3 < comphelper::string::getTokenCount(sSectLink, sfx2::cTokenSeperator))
                    {
                        sSectLink += sfx2::cTokenSeperator;
                    }
                    sSectLink.SetToken(2, sfx2::cTokenSeperator, sLink);
                    pSectionData->SetLinkFileName(sSectLink);
                    if (sSectLink.Len() < 3)
                    {
                        pSectionData->SetType(CONTENT_SECTION);
                    }
                }
            }
            break;
            case WID_SECT_VISIBLE:
            {
                sal_Bool bVal(sal_False);
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
                sal_Bool bVal(sal_False);
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
                    if (pSectionData->GetCondition().Len() != 0)
                    {
                        pSectionData->SetCondHidden(!bVal);
                    }
                }
            }
            break;
            case WID_SECT_PROTECTED:
            {
                sal_Bool bVal(sal_False);
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
                sal_Bool bVal(sal_False);
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
                if (pFmt)
                {
                    const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                    pItemSet.reset( new SfxItemSet(*rOldAttrSet.GetPool(), pEntry->nWID, pEntry->nWID));
                    pItemSet->Put(rOldAttrSet);
                    m_rPropSet.setPropertyValue(*pEntry,
                            pValues[nProperty], *pItemSet);
                }
                else
                {
                    SfxPoolItem* pPutItem = 0;
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pProps->m_pColItem.get())
                        {
                            m_pProps->m_pColItem.reset(new SwFmtCol);
                        }
                        pPutItem = m_pProps->m_pColItem.get();
                    }
                    else if (RES_BACKGROUND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pBrushItem.get())
                        {
                            m_pProps->m_pBrushItem.reset(
                                new SvxBrushItem(RES_BACKGROUND));
                        }
                        pPutItem = m_pProps->m_pBrushItem.get();
                    }
                    else if (RES_FTN_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFtnItem.get())
                        {
                            m_pProps->m_pFtnItem.reset(new SwFmtFtnAtTxtEnd);
                        }
                        pPutItem = m_pProps->m_pFtnItem.get();
                    }
                    else if (RES_END_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pEndItem.get())
                        {
                            m_pProps->m_pEndItem.reset(new SwFmtEndAtTxtEnd);
                        }
                        pPutItem = m_pProps->m_pEndItem.get();
                    }
                    else if (RES_UNKNOWNATR_CONTAINER== pEntry->nWID)
                    {
                        if (!m_pProps->m_pXMLAttr.get())
                        {
                            m_pProps->m_pXMLAttr.reset(
                                new SvXMLAttrContainerItem(
                                    RES_UNKNOWNATR_CONTAINER));
                        }
                        pPutItem = m_pProps->m_pXMLAttr.get();
                    }
                    else if (RES_COLUMNBALANCE== pEntry->nWID)
                    {
                        if (!m_pProps->m_pNoBalanceItem.get())
                        {
                            m_pProps->m_pNoBalanceItem.reset(
                                new SwFmtNoBalancedColumns(RES_COLUMNBALANCE));
                        }
                        pPutItem = m_pProps->m_pNoBalanceItem.get();
                    }
                    else if (RES_FRAMEDIR == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFrameDirItem.get())
                        {
                            m_pProps->m_pFrameDirItem.reset(
                                new SvxFrameDirectionItem(
                                FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));
                        }
                        pPutItem = m_pProps->m_pFrameDirItem.get();
                    }
                    else if (RES_LR_SPACE == pEntry->nWID)
                    {
                        if (!m_pProps->m_pLRSpaceItem.get())
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

    lcl_UpdateSection(pFmt, pSectionData, pItemSet, bLinkModeChanged,
        bLinkMode);
}

void SAL_CALL
SwXTextSection::setPropertyValues(
    const uno::Sequence< ::rtl::OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues)
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException)
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
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    uno::Sequence< uno::Any > aValues(1);
    aValues.getArray()[0] = rValue;
    m_pImpl->SetPropertyValues_Impl( aPropertyNames, aValues );
}

uno::Sequence< uno::Any >
SwXTextSection::Impl::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SwSectionFmt *const pFmt = GetSectionFmt();
    if (!pFmt && !m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    uno::Sequence< uno::Any > aRet(rPropertyNames.getLength());
    uno::Any* pRet = aRet.getArray();
    SwSection *const pSect = (pFmt) ? pFmt->GetSection() : 0;
    const OUString* pPropertyNames = rPropertyNames.getConstArray();

    for (sal_Int32 nProperty = 0; nProperty < rPropertyNames.getLength();
        nProperty++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
            m_rPropSet.getPropertyMap().getByName(pPropertyNames[nProperty]);
        if (!pEntry)
        {
            throw beans::UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + pPropertyNames[nProperty],
                static_cast<cppu::OWeakObject *>(& m_rThis));
        }
        switch(pEntry->nWID)
        {
            case WID_SECT_CONDITION:
            {
                OUString uTmp( (m_bIsDescriptor)
                    ? m_pProps->m_sCondition
                    : ::rtl::OUString(pSect->GetCondition()));
                pRet[nProperty] <<= uTmp;
            }
            break;
            case WID_SECT_DDE_TYPE:
            case WID_SECT_DDE_FILE:
            case WID_SECT_DDE_ELEMENT:
            {
                ::rtl::OUString sRet;
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
                sal_Int32 nDummy(0);
                sRet = sRet.getToken(pEntry->nWID - WID_SECT_DDE_TYPE,
                            sfx2::cTokenSeperator, nDummy);
                pRet[nProperty] <<= sRet;
            }
            break;
            case WID_SECT_DDE_AUTOUPDATE:
            {
                // GetUpdateType() returns .._ALWAYS or .._ONCALL
                if (pSect && pSect->IsLinkType() && pSect->IsConnected())  // #i73247#
                {
                    const sal_Bool bTemp =
                        (pSect->GetUpdateType() == sfx2::LINKUPDATE_ALWAYS);
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
                    ::rtl::OUString sRet( pSect->GetLinkFileName() );
                    sal_Int32 nIndex(0);
                    aLink.FileURL =
                        sRet.getToken(0, sfx2::cTokenSeperator, nIndex);
                    aLink.FilterName =
                        sRet.getToken(0, sfx2::cTokenSeperator, nIndex);
                }
                pRet[nProperty] <<= aLink;
            }
            break;
            case WID_SECT_REGION :
            {
                ::rtl::OUString sRet;
                if (m_bIsDescriptor)
                {
                    sRet = m_pProps->m_sSectionRegion;
                }
                else if (FILE_LINK_SECTION == pSect->GetType())
                {
                    sRet = pSect->GetLinkFileName().GetToken(2,
                            sfx2::cTokenSeperator);
                }
                pRet[nProperty] <<= sRet;
            }
            break;
            case WID_SECT_VISIBLE   :
            {
                const sal_Bool bTemp = (m_bIsDescriptor)
                    ? !m_pProps->m_bHidden : !pSect->IsHidden();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_CURRENTLY_VISIBLE:
            {
                const sal_Bool bTemp = (m_bIsDescriptor)
                    ? !m_pProps->m_bCondHidden : !pSect->IsCondHidden();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_PROTECTED:
            {
                const sal_Bool bTemp = (m_bIsDescriptor)
                    ? m_pProps->m_bProtect : pSect->IsProtect();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case WID_SECT_EDIT_IN_READONLY:
            {
                const sal_Bool bTemp = (m_bIsDescriptor)
                    ? m_pProps->m_bEditInReadonly : pSect->IsEditInReadonly();
                pRet[nProperty] <<= bTemp;
            }
            break;
            case  FN_PARAM_LINK_DISPLAY_NAME:
            {
                if (pFmt)
                {
                    pRet[nProperty] <<=
                        OUString(pFmt->GetSection()->GetSectionName());
                }
            }
            break;
            case WID_SECT_DOCUMENT_INDEX:
            {
                // search enclosing index
                SwSection* pEnclosingSection = pSect;
                while ((pEnclosingSection != NULL) &&
                       (TOX_CONTENT_SECTION != pEnclosingSection->GetType()))
                {
                    pEnclosingSection = pEnclosingSection->GetParent();
                }
                if (pEnclosingSection)
                {
                    // convert section to TOXBase and get SwXDocumentIndex
                    SwTOXBaseSection *const pTOXBaseSect =
                        PTR_CAST(SwTOXBaseSection, pEnclosingSection);
                    const uno::Reference<text::XDocumentIndex> xIndex =
                        SwXDocumentIndex::CreateXDocumentIndex(
                            *pTOXBaseSect->GetFmt()->GetDoc(), *pTOXBaseSect);
                    pRet[nProperty] <<= xIndex;
                }
                // else: no enclosing index found -> empty return value
            }
            break;
            case WID_SECT_IS_GLOBAL_DOC_SECTION:
            {
                const sal_Bool bRet = (NULL == pFmt) ? sal_False :
                    static_cast<sal_Bool>(NULL != pFmt->GetGlobalDocSection());
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
                if (!pFmt)
                    break;      // #i73247#
                SwNode* pSectNode = pFmt->GetSectionNode();
                if (FN_UNO_REDLINE_NODE_END == pEntry->nWID)
                {
                    pSectNode = pSectNode->EndOfSectionNode();
                }
                const SwRedlineTbl& rRedTbl =
                    pFmt->GetDoc()->GetRedlineTbl();
                for (sal_uInt16 nRed = 0; nRed < rRedTbl.size(); nRed++)
                {
                    const SwRedline* pRedline = rRedTbl[nRed];
                    SwNode const*const pRedPointNode = pRedline->GetNode(sal_True);
                    SwNode const*const pRedMarkNode = pRedline->GetNode(sal_False);
                    if ((pRedPointNode == pSectNode) ||
                        (pRedMarkNode == pSectNode))
                    {
                        SwNode const*const pStartOfRedline =
                            (SwNodeIndex(*pRedPointNode) <=
                             SwNodeIndex(*pRedMarkNode))
                                 ? pRedPointNode : pRedMarkNode;
                        const bool bIsStart = (pStartOfRedline == pSectNode);
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
                pRet[nProperty] <<= (m_bIsDescriptor)
                    ? m_pProps->m_Password : pSect->GetPassword();
            }
            break;
            default:
            {
                if (pFmt)
                {
                    m_rPropSet.getPropertyValue(*pEntry,
                            pFmt->GetAttrSet(), pRet[nProperty]);
                }
                else
                {
                    const SfxPoolItem* pQueryItem = 0;
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pProps->m_pColItem.get())
                        {
                            m_pProps->m_pColItem.reset(new SwFmtCol);
                        }
                        pQueryItem = m_pProps->m_pColItem.get();
                    }
                    else if (RES_BACKGROUND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pBrushItem.get())
                        {
                            m_pProps->m_pBrushItem.reset(
                                new SvxBrushItem(RES_BACKGROUND));
                        }
                        pQueryItem = m_pProps->m_pBrushItem.get();
                    }
                    else if (RES_FTN_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFtnItem.get())
                        {
                            m_pProps->m_pFtnItem.reset(new SwFmtFtnAtTxtEnd);
                        }
                        pQueryItem = m_pProps->m_pFtnItem.get();
                    }
                    else if (RES_END_AT_TXTEND == pEntry->nWID)
                    {
                        if (!m_pProps->m_pEndItem.get())
                        {
                            m_pProps->m_pEndItem.reset(new SwFmtEndAtTxtEnd);
                        }
                        pQueryItem = m_pProps->m_pEndItem.get();
                    }
                    else if (RES_UNKNOWNATR_CONTAINER== pEntry->nWID)
                    {
                        if (!m_pProps->m_pXMLAttr.get())
                        {
                            m_pProps->m_pXMLAttr.reset(
                                new SvXMLAttrContainerItem);
                        }
                        pQueryItem = m_pProps->m_pXMLAttr.get();
                    }
                    else if (RES_COLUMNBALANCE== pEntry->nWID)
                    {
                        if (!m_pProps->m_pNoBalanceItem.get())
                        {
                            m_pProps->m_pNoBalanceItem.reset(
                                new SwFmtNoBalancedColumns);
                        }
                        pQueryItem = m_pProps->m_pNoBalanceItem.get();
                    }
                    else if (RES_FRAMEDIR == pEntry->nWID)
                    {
                        if (!m_pProps->m_pFrameDirItem.get())
                        {
                            m_pProps->m_pFrameDirItem.reset(
                                new SvxFrameDirectionItem(
                                    FRMDIR_ENVIRONMENT, RES_FRAMEDIR));
                        }
                        pQueryItem = m_pProps->m_pFrameDirItem.get();
                    }
                    else if (RES_LR_SPACE == pEntry->nWID)
                    {
                        if (!m_pProps->m_pLRSpaceItem.get())
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
    const uno::Sequence< ::rtl::OUString >& rPropertyNames)
throw (uno::RuntimeException)
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
        throw uno::RuntimeException(OUString(
            RTL_CONSTASCII_USTRINGPARAM("Unknown property exception caught")),
            static_cast<cppu::OWeakObject *>(this));
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException(OUString(
                RTL_CONSTASCII_USTRINGPARAM("WrappedTargetException caught")),
            static_cast<cppu::OWeakObject *>(this));
    }

    return aValues;
}

uno::Any SAL_CALL
SwXTextSection::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    return m_pImpl->GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}

void SAL_CALL SwXTextSection::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
throw (uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::addPropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXTextSection::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
throw (uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::removePropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXTextSection::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::firePropertiesChangeEvent(): not implemented");
}

void SAL_CALL
SwXTextSection::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextSection::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_FAIL("SwXTextSection::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextSection::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames(1);
    aNames.getArray()[0] = rPropertyName;
    return getPropertyStates(aNames).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextSection::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt *const pFmt = m_pImpl->GetSectionFmt();
    if (!pFmt && !m_pImpl->m_bIsDescriptor)
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                    + pNames[i], static_cast< cppu::OWeakObject* >(this));
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
                if (pFmt)
                {
                    pStates[i] = m_pImpl->m_rPropSet.getPropertyState(
                                    pNames[i], pFmt->GetAttrSet());
                }
                else
                {
                    if (RES_COL == pEntry->nWID)
                    {
                        if (!m_pImpl->m_pProps->m_pColItem.get())
                        {
                            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                        }
                        else
                        {
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                        }
                    }
                    else //if(RES_BACKGROUND == pEntry->nWID)
                    {
                        if (!m_pImpl->m_pProps->m_pBrushItem.get())
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
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt *const pFmt = m_pImpl->GetSectionFmt();
    if (!pFmt && !m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName, static_cast< cppu::OWeakObject* >(this));
    }
    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "setPropertyToDefault: property is read-only: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(this));
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwSectionData> const pSectionData(
        (pFmt) ? new SwSectionData(*pFmt->GetSection()) : 0);

    ::std::auto_ptr<SfxItemSet> pNewAttrSet;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    bool bLinkModeChanged = false;

    switch (pEntry->nWID)
    {
        case WID_SECT_CONDITION:
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_pProps->m_sCondition = aEmptyStr;
            }
            else
            {
                pSectionData->SetCondition(aEmptyStr);
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
                m_pImpl->m_pProps->m_sLinkFileName = ::rtl::OUString();
                m_pImpl->m_pProps->m_sSectionRegion = ::rtl::OUString();
                m_pImpl->m_pProps->m_sSectionFilter = ::rtl::OUString();
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
            if (pEntry->nWID <= SFX_WHICH_MAX)
            {
                if (pFmt)
                {
                    const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                    pNewAttrSet.reset( new SfxItemSet(*rOldAttrSet.GetPool(), pEntry->nWID, pEntry->nWID));
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

    lcl_UpdateSection(pFmt, pSectionData, pNewAttrSet, bLinkModeChanged);
}

uno::Any SAL_CALL
SwXTextSection::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    SwSectionFmt *const pFmt = m_pImpl->GetSectionFmt();
    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
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
        {
            sal_Bool bTemp = sal_True;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        case WID_SECT_PROTECTED:
        case WID_SECT_EDIT_IN_READONLY:
        {
            sal_Bool bTemp = sal_False;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        case  FN_UNO_ANCHOR_TYPES:
        case  FN_UNO_TEXT_WRAP:
        case  FN_UNO_ANCHOR_TYPE:
            ::sw::GetDefaultTextContentValue(aRet, OUString(), pEntry->nWID);
        break;
        default:
        if(pFmt && pEntry->nWID <= SFX_WHICH_MAX)
        {
            SwDoc *const pDoc = pFmt->GetDoc();
            const SfxPoolItem& rDefItem =
                pDoc->GetAttrPool().GetDefaultItem(pEntry->nWID);
            rDefItem.QueryValue(aRet, pEntry->nMemberId);
        }
    }
    return aRet;
}

OUString SAL_CALL SwXTextSection::getName() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString sRet;
    SwSectionFmt const*const pFmt = m_pImpl->GetSectionFmt();
    if(pFmt)
    {
        sRet = pFmt->GetSection()->GetSectionName();
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
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwSectionFmt *const pFmt = m_pImpl->GetSectionFmt();
    if(pFmt)
    {
        SwSection *const pSect = pFmt->GetSection();
        SwSectionData aSection(*pSect);
        String sNewName(rName);
        aSection.SetSectionName(sNewName);

        const SwSectionFmts& rFmts = pFmt->GetDoc()->GetSections();
        sal_uInt16 nApplyPos = USHRT_MAX;
        for( sal_uInt16 i = 0; i < rFmts.size(); i++ )
        {
            if(rFmts[i]->GetSection() == pSect)
            {
                nApplyPos = i;
            }
            else if (sNewName == rFmts[i]->GetSection()->GetSectionName())
            {
                throw uno::RuntimeException();
            }
        }
        if(nApplyPos != USHRT_MAX)
        {
            {
                UnoActionContext aContext(pFmt->GetDoc());
                pFmt->GetDoc()->UpdateSection(nApplyPos, aSection);
            }
            {
                // temporarily remove actions to allow cursor update
                UnoActionRemoveContext aRemoveContext( pFmt->GetDoc() );
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
SwXTextSection::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXTextSection");
}

static char const*const g_ServicesTextSection[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.TextSection",
    "com.sun.star.document.LinkTarget",
};

sal_Bool SAL_CALL SwXTextSection::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            SAL_N_ELEMENTS(g_ServicesTextSection),
            g_ServicesTextSection, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextSection::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
                SAL_N_ELEMENTS(g_ServicesTextSection),
                g_ServicesTextSection);
}

// MetadatableMixin
::sfx2::Metadatable* SwXTextSection::GetCoreObject()
{
    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    return pSectionFmt;
}

uno::Reference<frame::XModel> SwXTextSection::GetModel()
{
    SwSectionFmt *const pSectionFmt( m_pImpl->GetSectionFmt() );
    if (pSectionFmt)
    {
        SwDocShell const*const pShell( pSectionFmt->GetDoc()->GetDocShell() );
        return (pShell) ? pShell->GetModel() : 0;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
