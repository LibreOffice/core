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

#include <com/sun/star/table/TableSortField.hpp>
#include <comphelper/string.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/endian.h>
#include <rtl/ustrbuf.hxx>
#include <unotools/collatorwrapper.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <IMark.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <istyleaccess.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <swundo.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <ftnidx.hxx>
#include <sfx2/linkmgr.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <swerror.h>
#include <swtblfmt.hxx>
#include <fmtruby.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <edimp.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unomap.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <unometa.hxx>
#include <fmtanchr.hxx>
#include <editeng/flstitem.hxx>
#include <svtools/ctrltool.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/syslocale.hxx>
#include <fmtflcnt.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/unolingu.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>
#include <sortopt.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <memory>
#include <unoparaframeenum.hxx>
#include <unoparagraph.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;

// Helper classes
SwUnoInternalPaM::SwUnoInternalPaM(SwDoc& rDoc) :
    SwPaM(rDoc.GetNodes())
{
}

SwUnoInternalPaM::~SwUnoInternalPaM()
{
    while( GetNext() != this)
    {
        delete GetNext();
    }
}

SwUnoInternalPaM&   SwUnoInternalPaM::operator=(const SwPaM& rPaM)
{
    const SwPaM* pTmp = &rPaM;
    *GetPoint() = *rPaM.GetPoint();
    if(rPaM.HasMark())
    {
        SetMark();
        *GetMark() = *rPaM.GetMark();
    }
    else
        DeleteMark();
    while(&rPaM != (pTmp = static_cast<const SwPaM*>(pTmp->GetNext())))
    {
        if(pTmp->HasMark())
            new SwPaM(*pTmp->GetMark(), *pTmp->GetPoint(), this);
        else
            new SwPaM(*pTmp->GetPoint(), this);
    }
    return *this;
}

void SwUnoCursorHelper::SelectPam(SwPaM & rPam, const bool bExpand)
{
    if (bExpand)
    {
        if (!rPam.HasMark())
        {
            rPam.SetMark();
        }
    }
    else if (rPam.HasMark())
    {
        rPam.DeleteMark();
    }
}

void SwUnoCursorHelper::GetTextFromPam(SwPaM & rPam, OUString & rBuffer)
{
    if (!rPam.HasMark())
    {
        return;
    }
    SvMemoryStream aStream;
#ifdef OSL_BIGENDIAN
    aStream.SetEndian( SvStreamEndian::BIG );
#else
    aStream.SetEndian( SvStreamEndian::LITTLE );
#endif
    WriterRef xWrt;
    // TODO/MBA: looks like a BaseURL doesn't make sense here
    SwReaderWriter::GetWriter( FILTER_TEXT_DLG, OUString(), xWrt );
    if( xWrt.Is() )
    {
        SwWriter aWriter( aStream, rPam );
        xWrt->bASCII_NoLastLineEnd = true;
        xWrt->bExportPargraphNumbering = false;
        SwAsciiOptions aOpt = xWrt->GetAsciiOptions();
        aOpt.SetCharSet( RTL_TEXTENCODING_UNICODE );
        xWrt->SetAsciiOptions( aOpt );
        xWrt->bUCS2_WithStartChar = false;
        // #i68522#
        const bool bOldShowProgress = xWrt->bShowProgress;
        xWrt->bShowProgress = false;

        if( !IsError( aWriter.Write( xWrt ) ) )
        {
            const sal_uInt64 lUniLen = aStream.GetSize()/sizeof( sal_Unicode );
            if (lUniLen < static_cast<sal_uInt64>(SAL_MAX_INT32-1))
            {
                aStream.WriteUInt16( '\0' );

                aStream.Seek( 0 );
                aStream.ResetError();

                rtl_uString *pStr = rtl_uString_alloc(lUniLen);
                aStream.Read(pStr->buffer, lUniLen * sizeof(sal_Unicode));
                rBuffer = OUString(pStr, SAL_NO_ACQUIRE);
            }
        }
        xWrt->bShowProgress = bOldShowProgress;
    }
}

static void
lcl_setCharStyle(SwDoc *const pDoc, const uno::Any & rValue, SfxItemSet & rSet)
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SwDocShell *const pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uStyle;
        if (!(rValue >>= uStyle))
        {
            throw lang::IllegalArgumentException();
        }
        OUString sStyle;
        SwStyleNameMapper::FillUIName(uStyle, sStyle,
                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
        SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR));
        if (!pStyle)
        {
            throw lang::IllegalArgumentException();
        }
        const SwFormatCharFormat aFormat(pStyle->GetCharFormat());
        rSet.Put(aFormat);
    }
};

static void
lcl_setAutoStyle(IStyleAccess & rStyleAccess, const uno::Any & rValue,
        SfxItemSet & rSet, const bool bPara)
throw (lang::IllegalArgumentException)
{
    OUString uStyle;
    if (!(rValue >>= uStyle))
    {
         throw lang::IllegalArgumentException();
    }
    StylePool::SfxItemSet_Pointer_t pStyle = bPara ?
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_PARA ):
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_CHAR );
    if(pStyle.get())
    {
        SwFormatAutoFormat aFormat( (bPara)
            ? sal::static_int_cast< sal_uInt16 >(RES_AUTO_STYLE)
            : sal::static_int_cast< sal_uInt16 >(RES_TXTATR_AUTOFMT) );
        aFormat.SetStyleHandle( pStyle );
        rSet.Put(aFormat);
    }
    else
    {
         throw lang::IllegalArgumentException();
    }
};

void
SwUnoCursorHelper::SetTextFormatColl(const uno::Any & rAny, SwPaM & rPaM)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SwDoc *const pDoc = rPaM.GetDoc();
    SwDocShell *const pDocSh = pDoc->GetDocShell();
    if(!pDocSh)
        return;
    OUString uStyle;
    rAny >>= uStyle;
    OUString sStyle;
    SwStyleNameMapper::FillUIName(uStyle, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
    SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_PARA));
    if (!pStyle)
    {
        throw lang::IllegalArgumentException();
    }

    SwTextFormatColl *const pLocal = pStyle->GetCollection();
    UnoActionContext aAction(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
    SwPaM *pTmpCursor = &rPaM;
    do {
        pDoc->SetTextFormatColl(*pTmpCursor, pLocal);
        pTmpCursor = static_cast<SwPaM*>(pTmpCursor->GetNext());
    } while ( pTmpCursor != &rPaM );
    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
}

bool
SwUnoCursorHelper::SetPageDesc(
        const uno::Any& rValue, SwDoc & rDoc, SfxItemSet & rSet)
{
    OUString uDescName;
    if (!(rValue >>= uDescName))
    {
        return false;
    }
    ::std::unique_ptr<SwFormatPageDesc> pNewDesc;
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
    {
        pNewDesc.reset(new SwFormatPageDesc(
                    *static_cast<const SwFormatPageDesc*>(pItem)));
    }
    if (!pNewDesc.get())
    {
        pNewDesc.reset(new SwFormatPageDesc());
    }
    OUString sDescName;
    SwStyleNameMapper::FillUIName(uDescName, sDescName,
            nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true);
    if (!pNewDesc->GetPageDesc() ||
        (pNewDesc->GetPageDesc()->GetName() != sDescName))
    {
        bool bPut = false;
        if (!sDescName.isEmpty())
        {
            SwPageDesc *const pPageDesc = SwPageDesc::GetByName(rDoc, sDescName);
            if (!pPageDesc)
            {
                throw lang::IllegalArgumentException();
            }
            pNewDesc.get()->RegisterToPageDesc( *pPageDesc );
            bPut = true;
        }
        if(!bPut)
        {
            rSet.ClearItem(RES_BREAK);
            rSet.Put(SwFormatPageDesc());
        }
        else
        {
            rSet.Put(*pNewDesc);
        }
    }
    return true;
}

static void
lcl_SetNodeNumStart(SwPaM & rCursor, uno::Any const& rValue)
{
    sal_Int16 nTmp = 1;
    rValue >>= nTmp;
    sal_uInt16 nStt = (nTmp < 0 ? USHRT_MAX : (sal_uInt16)nTmp);
    SwDoc* pDoc = rCursor.GetDoc();
    UnoActionContext aAction(pDoc);

    if( rCursor.GetNext() != &rCursor )         // MultiSelection?
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( rCursor );
        SwPaM aPam( *rCursor.GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
        {
          pDoc->SetNumRuleStart(*aRangeArr.SetPam( n, aPam ).GetPoint());
          pDoc->SetNodeNumStart(*aRangeArr.SetPam( n, aPam ).GetPoint(),
                    nStt );
        }
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    else
    {
        pDoc->SetNumRuleStart( *rCursor.GetPoint());
        pDoc->SetNodeNumStart( *rCursor.GetPoint(), nStt );
    }
}

static bool
lcl_setCharFormatSequence(SwPaM & rPam, uno::Any const& rValue)
{
    uno::Sequence<OUString> aCharStyles;
    if (!(rValue >>= aCharStyles))
    {
        return false;
    }

    for (sal_Int32 nStyle = 0; nStyle < aCharStyles.getLength(); nStyle++)
    {
        uno::Any aStyle;
        rPam.GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_START, nullptr);
        aStyle <<= aCharStyles.getConstArray()[nStyle];
        // create a local set and apply each format directly
        SfxItemSet aSet(rPam.GetDoc()->GetAttrPool(),
                RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT);
        lcl_setCharStyle(rPam.GetDoc(), aStyle, aSet);
        // the first style should replace the current attributes,
        // all other have to be added
        SwUnoCursorHelper::SetCursorAttr(rPam, aSet, (nStyle)
                ? SetAttrMode::DONTREPLACE
                : SetAttrMode::DEFAULT);
        rPam.GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_START, nullptr);
    }
    return true;
}

static void
lcl_setDropcapCharStyle(SwPaM & rPam, SfxItemSet & rItemSet,
        uno::Any const& rValue)
{
    OUString uStyle;
    if (!(rValue >>= uStyle))
    {
        throw lang::IllegalArgumentException();
    }
    OUString sStyle;
    SwStyleNameMapper::FillUIName(uStyle, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
    SwDoc *const pDoc = rPam.GetDoc();
    //default character style must not be set as default format
    SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDoc->GetDocShell()
            ->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR));
    if (!pStyle ||
        (static_cast<SwDocStyleSheet*>(pStyle)->GetCharFormat() ==
             pDoc->GetDfltCharFormat()))
    {
        throw lang::IllegalArgumentException();
    }
    ::std::unique_ptr<SwFormatDrop> pDrop;
    SfxPoolItem const* pItem(nullptr);
    if (SfxItemState::SET ==
            rItemSet.GetItemState(RES_PARATR_DROP, true, &pItem))
    {
        pDrop.reset(new SwFormatDrop(*static_cast<const SwFormatDrop*>(pItem)));
    }
    if (!pDrop.get())
    {
        pDrop.reset(new SwFormatDrop);
    }
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*pStyle));
    pDrop->SetCharFormat(xStyle->GetCharFormat());
    rItemSet.Put(*pDrop);
}

static void
lcl_setRubyCharstyle(SfxItemSet & rItemSet, uno::Any const& rValue)
{
    OUString sTmp;
    if (!(rValue >>= sTmp))
    {
        throw lang::IllegalArgumentException();
    }

    ::std::unique_ptr<SwFormatRuby> pRuby;
    const SfxPoolItem* pItem;
    if (SfxItemState::SET ==
            rItemSet.GetItemState(RES_TXTATR_CJK_RUBY, true, &pItem))
    {
        pRuby.reset(new SwFormatRuby(*static_cast<const SwFormatRuby*>(pItem)));
    }
    if (!pRuby.get())
    {
        pRuby.reset(new SwFormatRuby(OUString()));
    }
    OUString sStyle;
    SwStyleNameMapper::FillUIName(sTmp, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
    pRuby->SetCharFormatName(sStyle);
    pRuby->SetCharFormatId(0);
    if (!sStyle.isEmpty())
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
                sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
        pRuby->SetCharFormatId(nId);
    }
    rItemSet.Put(*pRuby);
}

bool
SwUnoCursorHelper::SetCursorPropertyValue(
        SfxItemPropertySimpleEntry const& rEntry, const uno::Any& rValue,
        SwPaM & rPam, SfxItemSet & rItemSet)
throw (lang::IllegalArgumentException, uno::RuntimeException,
       uno::DeploymentException, std::exception)
{
    if (!(rEntry.nFlags & beans::PropertyAttribute::MAYBEVOID) &&
        (rValue.getValueType() == cppu::UnoType<void>::get()))
    {
        return false;
    }
    bool bRet = true;
    switch (rEntry.nWID)
    {
        case RES_TXTATR_CHARFMT:
            lcl_setCharStyle(rPam.GetDoc(), rValue, rItemSet);
        break;
        case RES_TXTATR_AUTOFMT:
            lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(),
                    rValue, rItemSet, false);
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
            lcl_setCharFormatSequence(rPam, rValue);
        break;
        case FN_UNO_PARA_STYLE :
            SwUnoCursorHelper::SetTextFormatColl(rValue, rPam);
        break;
        case RES_AUTO_STYLE:
            lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(),
                    rValue, rItemSet, true);
        break;
        case FN_UNO_PAGE_STYLE:
            //FIXME nothing here?
        break;
        case FN_UNO_NUM_START_VALUE:
            lcl_SetNodeNumStart( rPam, rValue );
        break;
        case FN_UNO_NUM_LEVEL:
        // #i91601#
        case FN_UNO_LIST_ID:
        case FN_UNO_IS_NUMBER:
        {
            // multi selection is not considered
            SwTextNode *const pTextNd = rPam.GetNode().GetTextNode();
            if (!pTextNd)
            {
                throw lang::IllegalArgumentException();
            }
            if (FN_UNO_NUM_LEVEL == rEntry.nWID)
            {
                sal_Int16 nLevel = 0;
                if (rValue >>= nLevel)
                {
                    if (nLevel < 0 || MAXLEVEL <= nLevel)
                    {
                        throw lang::IllegalArgumentException(
                            "invalid NumberingLevel", nullptr, 0);
                    }
                    pTextNd->SetAttrListLevel(nLevel);
                }
            }
            // #i91601#
            else if (FN_UNO_LIST_ID == rEntry.nWID)
            {
                OUString sListId;
                if (rValue >>= sListId)
                {
                    pTextNd->SetListId( sListId );
                }
            }
            else if (FN_UNO_IS_NUMBER == rEntry.nWID)
            {
                bool bIsNumber(false);
                if (rValue >>= bIsNumber)
                {
                    if (!bIsNumber)
                    {
                        pTextNd->SetCountedInList( false );
                    }
                }
            }
            //PROPERTY_MAYBEVOID!
        }
        break;
        case FN_NUMBER_NEWSTART:
        {
            bool bVal = false;
            if (!(rValue >>= bVal))
            {
                throw lang::IllegalArgumentException();
            }
            rPam.GetDoc()->SetNumRuleStart(*rPam.GetPoint(), bVal);
        }
        break;
        case FN_UNO_NUM_RULES:
            SwUnoCursorHelper::setNumberingProperty(rValue, rPam);
        break;
        case RES_PARATR_DROP:
        {
            if (MID_DROPCAP_CHAR_STYLE_NAME == rEntry.nMemberId)
            {
                lcl_setDropcapCharStyle(rPam, rItemSet, rValue);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        case RES_TXTATR_CJK_RUBY:
        {
            if (MID_RUBY_CHARSTYLE == rEntry.nMemberId)
            {
                lcl_setRubyCharstyle(rItemSet, rValue);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        case RES_PAGEDESC:
        {
            if (MID_PAGEDESC_PAGEDESCNAME == rEntry.nMemberId)
            {
                SwUnoCursorHelper::SetPageDesc(
                        rValue, *rPam.GetDoc(), rItemSet);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

SwFormatColl *
SwUnoCursorHelper::GetCurTextFormatColl(SwPaM & rPaM, const bool bConditional)
{
    static const sal_uLong nMaxLookup = 1000;
    SwFormatColl *pFormat = nullptr;
    bool bError = false;
    SwPaM *pTmpCursor = &rPaM;
    do
    {
        const sal_uLong nSttNd = pTmpCursor->Start()->nNode.GetIndex();
        const sal_uLong nEndNd = pTmpCursor->End()->nNode.GetIndex();

        if( nEndNd - nSttNd >= nMaxLookup )
        {
            pFormat = nullptr;
            break;
        }

        const SwNodes& rNds = rPaM.GetDoc()->GetNodes();
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            SwTextNode const*const pNd = rNds[ n ]->GetTextNode();
            if( pNd )
            {
                SwFormatColl *const pNdFormat = (bConditional)
                    ? pNd->GetFormatColl() : &pNd->GetAnyFormatColl();
                if( !pFormat )
                {
                    pFormat = pNdFormat;
                }
                else if( pFormat != pNdFormat )
                {
                    bError = true;
                    break;
                }
            }
        }

        pTmpCursor = static_cast<SwPaM*>(pTmpCursor->GetNext());
    } while ( pTmpCursor != &rPaM );
    return (bError) ? nullptr : pFormat;
}

class SwXTextCursor::Impl
{

public:
    const SfxItemPropertySet &  m_rPropSet;
    const enum CursorType       m_eType;
    const uno::Reference< text::XText > m_xParentText;
    sw::UnoCursorPointer m_pUnoCursor;

    Impl(   SwDoc & rDoc,
            const enum CursorType eType,
            uno::Reference<text::XText> xParent,
            SwPosition const& rPoint, SwPosition const*const pMark)
        : m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eType(eType)
        , m_xParentText(xParent)
        , m_pUnoCursor(rDoc.CreateUnoCursor(rPoint), true)
    {
        if (pMark)
        {
            m_pUnoCursor->SetMark();
            *m_pUnoCursor->GetMark() = *pMark;
        }
    }

    SwUnoCursor& GetCursorOrThrow() {
        if(!m_pUnoCursor)
            throw uno::RuntimeException("SwXTextCursor: disposed or invalid", nullptr);
        return *m_pUnoCursor;
    }
};

SwUnoCursor& SwXTextCursor::GetCursor()
    { return *m_pImpl->m_pUnoCursor; }

SwPaM const* SwXTextCursor::GetPaM() const
    { return &(*m_pImpl->m_pUnoCursor); }

SwPaM* SwXTextCursor::GetPaM()
    { return &(*m_pImpl->m_pUnoCursor); }

SwDoc const* SwXTextCursor::GetDoc() const
    { return m_pImpl->m_pUnoCursor ? m_pImpl->m_pUnoCursor->GetDoc() : nullptr; }

SwDoc* SwXTextCursor::GetDoc()
    { return m_pImpl->m_pUnoCursor ? m_pImpl->m_pUnoCursor->GetDoc() : nullptr; }

SwXTextCursor::SwXTextCursor(
        SwDoc & rDoc,
        uno::Reference< text::XText > const& xParent,
        const enum CursorType eType,
        const SwPosition& rPos,
        SwPosition const*const pMark)
    : m_pImpl( new Impl(rDoc, eType, xParent, rPos, pMark) )
{
}

SwXTextCursor::SwXTextCursor(uno::Reference< text::XText > const& xParent,
        SwPaM const& rSourceCursor, const enum CursorType eType)
    : m_pImpl( new Impl(*rSourceCursor.GetDoc(), eType,
                xParent, *rSourceCursor.GetPoint(),
                rSourceCursor.HasMark() ? rSourceCursor.GetMark() : nullptr) )
{
}

SwXTextCursor::~SwXTextCursor()
{
}

void SwXTextCursor::DeleteAndInsert(const OUString& rText,
        const bool bForceExpandHints)
{
    auto pUnoCursor = static_cast<SwCursor*>(&(*m_pImpl->m_pUnoCursor));
    if(pUnoCursor)
    {
        // Start/EndAction
        SwDoc* pDoc = pUnoCursor->GetDoc();
        UnoActionContext aAction(pDoc);
        const sal_Int32 nTextLen = rText.getLength();
        pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);
        auto pCurrent = static_cast<SwCursor*>(pUnoCursor);
        do
        {
            if (pCurrent->HasMark())
            {
                pDoc->getIDocumentContentOperations().DeleteAndJoin(*pCurrent);
            }
            if(nTextLen)
            {
                const bool bSuccess(
                    SwUnoCursorHelper::DocInsertStringSplitCR(
                        *pDoc, *pCurrent, rText, bForceExpandHints ) );
                OSL_ENSURE( bSuccess, "Doc->Insert(Str) failed." );
                (void) bSuccess;

                SwUnoCursorHelper::SelectPam(*pUnoCursor, true);
                pCurrent->Left(rText.getLength(),
                        CRSR_SKIP_CHARS, false, false);
            }
            pCurrent = static_cast<SwCursor*>(pCurrent->GetNext());
        } while (pCurrent != pUnoCursor);
        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
    }
}

enum ForceIntoMetaMode { META_CHECK_BOTH, META_INIT_START, META_INIT_END };

static bool
lcl_ForceIntoMeta(SwPaM & rCursor,
        uno::Reference<text::XText> const & xParentText,
        const enum ForceIntoMetaMode eMode)
{
    bool bRet( true ); // means not forced in META_CHECK_BOTH
    SwXMeta const * const pXMeta( dynamic_cast<SwXMeta*>(xParentText.get()) );
    OSL_ENSURE(pXMeta, "no parent?");
    if (!pXMeta)
        throw uno::RuntimeException();
    SwTextNode * pTextNode;
    sal_Int32 nStart;
    sal_Int32 nEnd;
    const bool bSuccess( pXMeta->SetContentRange(pTextNode, nStart, nEnd) );
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
        throw uno::RuntimeException();
    // force the cursor back into the meta if it has moved outside
    SwPosition start(*pTextNode, nStart);
    SwPosition end(*pTextNode, nEnd);
    switch (eMode)
    {
        case META_INIT_START:
            *rCursor.GetPoint() = start;
            break;
        case META_INIT_END:
            *rCursor.GetPoint() = end;
            break;
        case META_CHECK_BOTH:
            if (*rCursor.Start() < start)
            {
                *rCursor.Start() = start;
                bRet = false;
            }
            if (*rCursor.End() > end)
            {
                *rCursor.End() = end;
                bRet = false;
            }
            break;
    }
    return bRet;
}

bool SwXTextCursor::IsAtEndOfMeta() const
{
    if (CURSOR_META == m_pImpl->m_eType)
    {
        auto pCursor( m_pImpl->m_pUnoCursor );
        SwXMeta const*const pXMeta(
                dynamic_cast<SwXMeta*>(m_pImpl->m_xParentText.get()) );
        OSL_ENSURE(pXMeta, "no meta?");
        if (pCursor && pXMeta)
        {
            SwTextNode * pTextNode;
            sal_Int32 nStart;
            sal_Int32 nEnd;
            const bool bSuccess(
                    pXMeta->SetContentRange(pTextNode, nStart, nEnd) );
            OSL_ENSURE(bSuccess, "no pam?");
            if (bSuccess)
            {
                const SwPosition end(*pTextNode, nEnd);
                if (   (*pCursor->GetPoint() == end)
                    || (*pCursor->GetMark()  == end))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

OUString SwXTextCursor::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXTextCursor");
}

static char const*const g_ServicesTextCursor[] =
{
    "com.sun.star.text.TextCursor",
    "com.sun.star.style.CharacterProperties",
    "com.sun.star.style.CharacterPropertiesAsian",
    "com.sun.star.style.CharacterPropertiesComplex",
    "com.sun.star.style.ParagraphProperties",
    "com.sun.star.style.ParagraphPropertiesAsian",
    "com.sun.star.style.ParagraphPropertiesComplex",
    "com.sun.star.text.TextSortable",
};

static const size_t g_nServicesTextCursor(
    sizeof(g_ServicesTextCursor)/sizeof(g_ServicesTextCursor[0]));

sal_Bool SAL_CALL SwXTextCursor::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextCursor::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextCursor, g_ServicesTextCursor);
}

namespace
{
    class theSwXTextCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextCursorUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextCursor::getUnoTunnelId()
{
    return theSwXTextCursorUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXTextCursor::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException, std::exception)
{
    const sal_Int64 nRet( ::sw::UnoTunnelImpl<SwXTextCursor>(rId, this) );
    return (nRet) ? nRet : OTextCursorHelper::getSomething(rId);
}

void SAL_CALL SwXTextCursor::collapseToStart() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        if (*rUnoCursor.GetPoint() > *rUnoCursor.GetMark())
        {
            rUnoCursor.Exchange();
        }
        rUnoCursor.DeleteMark();
    }
}

void SAL_CALL SwXTextCursor::collapseToEnd() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        if (*rUnoCursor.GetPoint() < *rUnoCursor.GetMark())
        {
            rUnoCursor.Exchange();
        }
        rUnoCursor.DeleteMark();
    }
}

sal_Bool SAL_CALL SwXTextCursor::isCollapsed() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bRet = true;
    auto pUnoCursor(m_pImpl->m_pUnoCursor);
    if(pUnoCursor && pUnoCursor->GetMark())
    {
        bRet = (*pUnoCursor->GetPoint() == *pUnoCursor->GetMark());
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::goLeft(sal_Int16 nCount, sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = rUnoCursor.Left( nCount, CRSR_SKIP_CHARS, false, false);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::goRight(sal_Int16 nCount, sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = rUnoCursor.Right(nCount, CRSR_SKIP_CHARS, false, false);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

void SAL_CALL
SwXTextCursor::gotoStart(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (CURSOR_BODY == m_pImpl->m_eType)
    {
        rUnoCursor.Move( fnMoveBackward, fnGoDoc );
        //check, that the cursor is not in a table
        SwTableNode * pTableNode = rUnoCursor.GetNode().FindTableNode();
        SwContentNode * pCNode = nullptr;
        while (pTableNode)
        {
            rUnoCursor.GetPoint()->nNode = *pTableNode->EndOfSectionNode();
            pCNode = GetDoc()->GetNodes().GoNext(&rUnoCursor.GetPoint()->nNode);
            pTableNode = (pCNode) ? pCNode->FindTableNode() : nullptr;
        }
        if (pCNode)
        {
            rUnoCursor.GetPoint()->nContent.Assign(pCNode, 0);
        }
        SwStartNode const*const pTmp =
            rUnoCursor.GetNode().StartOfSectionNode();
        if (pTmp->IsSectionNode())
        {
            SwSectionNode const*const pSectionStartNode =
                static_cast<SwSectionNode const*>(pTmp);
            if (pSectionStartNode->GetSection().IsHiddenFlag())
            {
                pCNode = GetDoc()->GetNodes().GoNextSection(
                        &rUnoCursor.GetPoint()->nNode, true, false);
                if (pCNode)
                {
                    rUnoCursor.GetPoint()->nContent.Assign(pCNode, 0);
                }
            }
        }
    }
    else if (   (CURSOR_FRAME   == m_pImpl->m_eType)
            ||  (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ||  (CURSOR_HEADER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTNOTE== m_pImpl->m_eType)
            ||  (CURSOR_REDLINE == m_pImpl->m_eType))
    {
        rUnoCursor.MoveSection(fnSectionCurr, fnSectionStart);
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText, META_INIT_START);
    }
}

void SAL_CALL
SwXTextCursor::gotoEnd(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (CURSOR_BODY == m_pImpl->m_eType)
    {
        rUnoCursor.Move( fnMoveForward, fnGoDoc );
    }
    else if (   (CURSOR_FRAME   == m_pImpl->m_eType)
            ||  (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ||  (CURSOR_HEADER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTNOTE== m_pImpl->m_eType)
            ||  (CURSOR_REDLINE == m_pImpl->m_eType))
    {
        rUnoCursor.MoveSection( fnSectionCurr, fnSectionEnd);
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText, META_INIT_END);
    }
}

void SAL_CALL
SwXTextCursor::gotoRange(
    const uno::Reference< text::XTextRange > & xRange, sal_Bool bExpand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!xRange.is())
    {
        throw uno::RuntimeException();
    }

    SwUnoCursor & rOwnCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    if (!pRange && !pCursor)
    {
        throw uno::RuntimeException();
    }

    SwPaM aPam(GetDoc()->GetNodes());
    const SwPaM * pPam(nullptr);
    if (pCursor)
    {
        pPam = pCursor->GetPaM();
    }
    else if (pRange)
    {
        if (pRange->GetPositions(aPam))
        {
            pPam = & aPam;
        }
    }

    if (!pPam)
    {
        throw uno::RuntimeException();
    }

    {
        SwStartNodeType eSearchNodeType = SwNormalStartNode;
        switch (m_pImpl->m_eType)
        {
        case CURSOR_FRAME:      eSearchNodeType = SwFlyStartNode;       break;
        case CURSOR_TBLTEXT:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CURSOR_FOOTNOTE:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CURSOR_HEADER:     eSearchNodeType = SwHeaderStartNode;    break;
        case CURSOR_FOOTER:     eSearchNodeType = SwFooterStartNode;    break;
            //case CURSOR_INVALID:
            //case CURSOR_BODY:
        default:
            ;
        }

        const SwStartNode* pOwnStartNode = rOwnCursor.GetNode().FindSttNodeByType(eSearchNodeType);
        while ( pOwnStartNode != nullptr
                && pOwnStartNode->IsSectionNode())
        {
            pOwnStartNode = pOwnStartNode->StartOfSectionNode();
        }

        const SwStartNode* pTmp =
            pPam->GetNode().FindSttNodeByType(eSearchNodeType);
        while ( pTmp != nullptr
                && pTmp->IsSectionNode() )
        {
            pTmp = pTmp->StartOfSectionNode();
        }

        if ( eSearchNodeType == SwTableBoxStartNode )
        {
            if (!pOwnStartNode || !pTmp)
            {
                throw uno::RuntimeException();
            }

            if ( pOwnStartNode->FindTableNode() != pTmp->FindTableNode() )
            {
                throw uno::RuntimeException();
            }
        }
        else
        {
            if ( pOwnStartNode != pTmp )
            {
                throw uno::RuntimeException();
            }
        }
    }

    if (CURSOR_META == m_pImpl->m_eType)
    {
        SwPaM CopyPam(*pPam->GetMark(), *pPam->GetPoint());
        const bool bNotForced( lcl_ForceIntoMeta(
                    CopyPam, m_pImpl->m_xParentText, META_CHECK_BOTH) );
        if (!bNotForced)
        {
            throw uno::RuntimeException(
                "gotoRange: parameter range not contained in nesting"
                    " text content for which this cursor was created",
                static_cast<text::XWordCursor*>(this));
        }
    }

    // selection has to be expanded here
    if(bExpand)
    {
        // cursor should include its previous range plus the given range
        const SwPosition aOwnLeft(*rOwnCursor.Start());
        const SwPosition aOwnRight(*rOwnCursor.End());
        SwPosition const& rParamLeft  = *pPam->Start();
        SwPosition const& rParamRight = *pPam->End();

        // now there are four SwPositions,
        // two of them are going to be used, but which ones?
        *rOwnCursor.GetPoint() = (aOwnRight > rParamRight)
            ? aOwnRight : *rOwnCursor.GetPoint() = rParamRight;
        rOwnCursor.SetMark();
        *rOwnCursor.GetMark() = (aOwnLeft < rParamLeft)
            ? aOwnLeft : *rOwnCursor.GetMark() = rParamLeft;
    }
    else
    {
        // cursor should be the given range
        *rOwnCursor.GetPoint() = *pPam->GetPoint();
        if (pPam->HasMark())
        {
            rOwnCursor.SetMark();
            *rOwnCursor.GetMark() = *pPam->GetMark();
        }
        else
        {
            rOwnCursor.DeleteMark();
        }
    }
}

sal_Bool SAL_CALL SwXTextCursor::isStartOfWord() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bRet =
        rUnoCursor.IsStartWordWT( i18n::WordType::DICTIONARY_WORD );
    return bRet;
}

sal_Bool SAL_CALL SwXTextCursor::isEndOfWord() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bRet =
        rUnoCursor.IsEndWordWT( i18n::WordType::DICTIONARY_WORD );
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoNextWord(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // problems arise when a paragraph starts with something other than a word
    bool bRet = false;
    // remember old position to check if cursor has moved
    // since the called functions are sometimes a bit unreliable
    // in specific cases...
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      *const pOldNode   = &pPoint->nNode.GetNode();
    sal_Int32 const nOldIndex  = pPoint->nContent.GetIndex();

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // end of paragraph
    if (rUnoCursor.GetContentNode() &&
            (pPoint->nContent == rUnoCursor.GetContentNode()->Len()))
    {
        rUnoCursor.Right(1, CRSR_SKIP_CHARS, false, false);
    }
    else
    {
        const bool bTmp =
            rUnoCursor.GoNextWordWT( i18n::WordType::DICTIONARY_WORD );
        // if there is no next word within the current paragraph
        // try to go to the start of the next paragraph
        if (!bTmp)
        {
            rUnoCursor.MovePara(fnParaNext, fnParaStart);
        }
    }

    // return true if cursor has moved
    bRet =  (&pPoint->nNode.GetNode() != pOldNode)  ||
            (pPoint->nContent.GetIndex() != nOldIndex);
    if (bRet && (CURSOR_META == m_pImpl->m_eType))
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousWord(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // white spaces create problems on the paragraph start
    bool bRet = false;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      *const pOldNode   = &pPoint->nNode.GetNode();
    sal_Int32 const nOldIndex  = pPoint->nContent.GetIndex();

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // start of paragraph?
    if (pPoint->nContent == 0)
    {
        rUnoCursor.Left(1, CRSR_SKIP_CHARS, false, false);
    }
    else
    {
        rUnoCursor.GoPrevWordWT( i18n::WordType::DICTIONARY_WORD );
        if (pPoint->nContent == 0)
        {
            rUnoCursor.Left(1, CRSR_SKIP_CHARS, false, false);
        }
    }

    // return true if cursor has moved
    bRet =  (&pPoint->nNode.GetNode() != pOldNode)  ||
            (pPoint->nContent.GetIndex() != nOldIndex);
    if (bRet && (CURSOR_META == m_pImpl->m_eType))
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfWord(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    bool bRet = false;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      &      rOldNode   = pPoint->nNode.GetNode();
    sal_Int32 const nOldIndex  = pPoint->nContent.GetIndex();

    const sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (!rUnoCursor.IsEndWordWT( nWordType ))
    {
        rUnoCursor.GoEndWordWT( nWordType );
    }

    // restore old cursor if we are not at the end of a word by now
    // otherwise use current one
    bRet = rUnoCursor.IsEndWordWT( nWordType );
    if (!bRet)
    {
        pPoint->nNode       = rOldNode;
        pPoint->nContent    = nOldIndex;
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfWord(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    bool bRet = false;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      &      rOldNode   = pPoint->nNode.GetNode();
    sal_Int32 const nOldIndex  = pPoint->nContent.GetIndex();

    const sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (!rUnoCursor.IsStartWordWT( nWordType ))
    {
        rUnoCursor.GoStartWordWT( nWordType );
    }

    // restore old cursor if we are not at the start of a word by now
    // otherwise use current one
    bRet = rUnoCursor.IsStartWordWT( nWordType );
    if (!bRet)
    {
        pPoint->nNode       = rOldNode;
        pPoint->nContent    = nOldIndex;
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::isStartOfSentence() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // start of paragraph?
    bool bRet = rUnoCursor.GetPoint()->nContent == 0;
    // with mark ->no sentence start
    // (check if cursor is no selection, i.e. it does not have
    // a mark or else point and mark are identical)
    if (!bRet && (!rUnoCursor.HasMark() ||
                    *rUnoCursor.GetPoint() == *rUnoCursor.GetMark()))
    {
        SwCursor aCursor(*rUnoCursor.GetPoint(),nullptr,false);
        SwPosition aOrigPos = *aCursor.GetPoint();
        aCursor.GoSentence(SwCursor::START_SENT );
        bRet = aOrigPos == *aCursor.GetPoint();
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::isEndOfSentence() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // end of paragraph?
    bool bRet = rUnoCursor.GetContentNode() &&
        (rUnoCursor.GetPoint()->nContent == rUnoCursor.GetContentNode()->Len());
    // with mark->no sentence end
    // (check if cursor is no selection, i.e. it does not have
    // a mark or else point and mark are identical)
    if (!bRet && (!rUnoCursor.HasMark() ||
                    *rUnoCursor.GetPoint() == *rUnoCursor.GetMark()))
    {
        SwCursor aCursor(*rUnoCursor.GetPoint(), nullptr, false);
        SwPosition aOrigPos = *aCursor.GetPoint();
        aCursor.GoSentence(SwCursor::END_SENT);
        bRet = aOrigPos == *aCursor.GetPoint();
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoNextSentence(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bWasEOS = isEndOfSentence();
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = rUnoCursor.GoSentence(SwCursor::NEXT_SENT);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaNext, fnParaStart);
    }

    // if at the end of the sentence (i.e. at the space after the '.')
    // advance to next word in order for GoSentence to work properly
    // next time and have isStartOfSentence return true after this call
    if (!rUnoCursor.IsStartWord())
    {
        const bool bNextWord = rUnoCursor.GoNextWord();
        if (bWasEOS && !bNextWord)
        {
            bRet = false;
        }
    }
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousSentence(sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = rUnoCursor.GoSentence(SwCursor::PREV_SENT);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaPrev, fnParaStart);
        if (bRet)
        {
            rUnoCursor.MovePara(fnParaCurr, fnParaEnd);
            // at the end of a paragraph move to the sentence end again
            rUnoCursor.GoSentence(SwCursor::PREV_SENT);
        }
    }
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfSentence(sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // if we're at the para start then we won't move
    // but bRet is also true if GoSentence failed but
    // the start of the sentence is reached
    bool bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor)
        || rUnoCursor.GoSentence(SwCursor::START_SENT)
        || SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfSentence(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // bRet is true if GoSentence() succeeded or if the
    // MovePara() succeeded while the end of the para is
    // not reached already
    bool bAlreadyParaEnd = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
    bool bRet = !bAlreadyParaEnd
            &&  (rUnoCursor.GoSentence(SwCursor::END_SENT)
                 || rUnoCursor.MovePara(fnParaCurr, fnParaEnd));
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::isStartOfParagraph() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::isEndOfParagraph() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bRet = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfParagraph(sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaCurr, fnParaStart);
    }

    // since MovePara(fnParaCurr, fnParaStart) only returns false
    // if we were already at the start of the paragraph this function
    // should always complete successfully.
    OSL_ENSURE( bRet, "gotoStartOfParagraph failed" );
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfParagraph(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    bool bRet = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaCurr, fnParaEnd);
    }

    // since MovePara(fnParaCurr, fnParaEnd) only returns false
    // if we were already at the end of the paragraph this function
    // should always complete successfully.
    OSL_ENSURE( bRet, "gotoEndOfParagraph failed" );
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoNextParagraph(sal_Bool Expand) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    const bool bRet = rUnoCursor.MovePara(fnParaNext, fnParaStart);
    return bRet;
}

sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousParagraph(sal_Bool Expand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    const bool bRet = rUnoCursor.MovePara(fnParaPrev, fnParaStart);
    return bRet;
}

uno::Reference< text::XText > SAL_CALL
SwXTextCursor::getText() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextCursor::getStart() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference< text::XTextRange > xRet;
    SwPaM aPam(*rUnoCursor.Start());
    const uno::Reference< text::XText >  xParent = getText();
    if (CURSOR_META == m_pImpl->m_eType)
    {
        // return cursor to prevent modifying SwXTextRange for META
        SwXTextCursor * const pXCursor(
            new SwXTextCursor(*rUnoCursor.GetDoc(), xParent, CURSOR_META,
                *rUnoCursor.GetPoint()) );
        pXCursor->gotoStart(sal_False);
        xRet = static_cast<text::XWordCursor*>(pXCursor);
    }
    else
    {
        xRet = new SwXTextRange(aPam, xParent);
    }
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextCursor::getEnd() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference< text::XTextRange >  xRet;
    SwPaM aPam(*rUnoCursor.End());
    const uno::Reference< text::XText >  xParent = getText();
    if (CURSOR_META == m_pImpl->m_eType)
    {
        // return cursor to prevent modifying SwXTextRange for META
        SwXTextCursor * const pXCursor(
            new SwXTextCursor(*rUnoCursor.GetDoc(), xParent, CURSOR_META,
                *rUnoCursor.GetPoint()) );
        pXCursor->gotoEnd(sal_False);
        xRet = static_cast<text::XWordCursor*>(pXCursor);
    }
    else
    {
        xRet = new SwXTextRange(aPam, xParent);
    }
    return xRet;
}

OUString SAL_CALL SwXTextCursor::getString() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    OUString aText;
    SwUnoCursorHelper::GetTextFromPam(rUnoCursor, aText);
    return aText;
}

void SAL_CALL
SwXTextCursor::setString(const OUString& aString) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );
    (void) rUnoCursor; // just to check if valid

    const bool bForceExpandHints( (CURSOR_META == m_pImpl->m_eType)
        && dynamic_cast<SwXMeta*>(m_pImpl->m_xParentText.get())
                ->CheckForOwnMemberMeta(*GetPaM(), true) );
    DeleteAndInsert(aString, bForceExpandHints);
}

uno::Any SwUnoCursorHelper::GetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    uno::Any aAny;
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(nullptr));
    }

    beans::PropertyState eTemp;
    const bool bDone = SwUnoCursorHelper::getCursorPropertyValue(
            *pEntry, rPaM, &aAny, eTemp );

    if (!bDone)
    {
        SfxItemSet aSet(rPaM.GetDoc()->GetAttrPool(),
            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
            RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
            0L);
        SwUnoCursorHelper::GetCursorAttr(rPaM, aSet);

        rPropSet.getPropertyValue(*pEntry, aSet, aAny);
    }

    return aAny;
}

void SwUnoCursorHelper::SetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName,
    const uno::Any& rValue,
    const SetAttrMode nAttrMode, const bool bTableMode)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    uno::Sequence< beans::PropertyValue > aValues(1);
    aValues[0].Name = rPropertyName;
    aValues[0].Value = rValue;
    SetPropertyValues(rPaM, rPropSet, aValues, nAttrMode, bTableMode);
}

// FN_UNO_PARA_STYLE is known to set attributes for nodes, inside
// SwUnoCursorHelper::SetTextFormatColl, instead of extending item set.
// We need to get them from nodes in next call to GetCursorAttr.
// The rest could cause similar problems in theory, so we just list them here.
inline bool propertyCausesSideEffectsInNodes(sal_uInt16 nWID)
{
    return nWID == FN_UNO_PARA_STYLE ||
           nWID == FN_UNO_CHARFMT_SEQUENCE ||
           nWID == FN_UNO_NUM_START_VALUE ||
           nWID == FN_UNO_NUM_RULES;
}

void SwUnoCursorHelper::SetPropertyValues(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const uno::Sequence< beans::PropertyValue > &rPropertyValues,
    const SetAttrMode nAttrMode, const bool bTableMode)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    if (!rPropertyValues.getLength())
        return;

    SwDoc *const pDoc = rPaM.GetDoc();
    OUString aUnknownExMsg, aPropertyVetoExMsg;

    // Build set of attributes we want to fetch
    std::vector<sal_uInt16> aWhichPairs;
    std::vector<SfxItemPropertySimpleEntry const*> aEntries;
    aEntries.reserve(rPropertyValues.getLength());
    for (sal_Int32 i = 0; i < rPropertyValues.getLength(); ++i)
    {
        const OUString &rPropertyName = rPropertyValues[i].Name;

        SfxItemPropertySimpleEntry const* pEntry =
            rPropSet.getPropertyMap().getByName(rPropertyName);

        // Queue up any exceptions until the end ...
        if (!pEntry)
        {
            aUnknownExMsg += "Unknown property: '" + rPropertyName + "' ";
            break;
        }
        else if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            aPropertyVetoExMsg += "Property is read-only: '" + rPropertyName + "' ";
            break;
        } else {
// FIXME: we should have some nice way of merging ranges surely ?
            aWhichPairs.push_back(pEntry->nWID);
            aWhichPairs.push_back(pEntry->nWID);
        }
        aEntries.push_back(pEntry);
    }

    if (!aWhichPairs.empty())
    {
        aWhichPairs.push_back(0); // terminate
        SfxItemSet aItemSet(pDoc->GetAttrPool(), &aWhichPairs[0]);

        // Fetch, overwrite, and re-set the attributes from the core

        bool bPreviousPropertyCausesSideEffectsInNodes = false;
        for (size_t i = 0; i < aEntries.size(); ++i)
        {
            SfxItemPropertySimpleEntry const*const pEntry = aEntries[i];
            bool bPropertyCausesSideEffectsInNodes =
                propertyCausesSideEffectsInNodes(pEntry->nWID);

            // we need to get up-to-date item set from nodes
            if (i == 0 || bPreviousPropertyCausesSideEffectsInNodes)
                SwUnoCursorHelper::GetCursorAttr(rPaM, aItemSet);

            const uno::Any &rValue = rPropertyValues[i].Value;
            // this can set some attributes in nodes' mpAttrSet
            if (!SwUnoCursorHelper::SetCursorPropertyValue(*pEntry, rValue, rPaM, aItemSet))
                rPropSet.setPropertyValue(*pEntry, rValue, aItemSet);

            if (i + 1 == aEntries.size() || bPropertyCausesSideEffectsInNodes)
                SwUnoCursorHelper::SetCursorAttr(rPaM, aItemSet, nAttrMode, bTableMode);

            bPreviousPropertyCausesSideEffectsInNodes = bPropertyCausesSideEffectsInNodes;
        }
    }

    if (!aUnknownExMsg.isEmpty())
        throw beans::UnknownPropertyException(aUnknownExMsg, static_cast<cppu::OWeakObject *>(nullptr));
    if (!aPropertyVetoExMsg.isEmpty())
        throw beans::PropertyVetoException(aPropertyVetoExMsg, static_cast<cppu::OWeakObject *>(nullptr));
}

uno::Sequence< beans::PropertyState >
SwUnoCursorHelper::GetPropertyStates(
            SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
            const uno::Sequence< OUString >& rPropertyNames,
            const SwGetPropertyStatesCaller eCaller)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const SfxItemPropertyMap &rMap = rPropSet.getPropertyMap();
    ::std::unique_ptr<SfxItemSet> pSet;
    ::std::unique_ptr<SfxItemSet> pSetParent;

    for (sal_Int32 i = 0, nEnd = rPropertyNames.getLength(); i < nEnd; i++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
                rMap.getByName( pNames[i] );
        if(!pEntry)
        {
            if (pNames[i] == UNO_NAME_IS_SKIP_HIDDEN_TEXT ||
                pNames[i] == UNO_NAME_IS_SKIP_PROTECTED_TEXT)
            {
                pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                continue;
            }
            else if (SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT ==
                        eCaller)
            {
                //this values marks the element as unknown property
                pStates[i] = beans::PropertyState_MAKE_FIXED_SIZE;
                continue;
            }
            else
            {
                throw beans::UnknownPropertyException(
                    "Unknown property: " + pNames[i],
                    static_cast<cppu::OWeakObject *>(nullptr));
            }
        }
        if (((SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION == eCaller)  ||
             (SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT == eCaller)) &&
            pEntry->nWID < FN_UNO_RANGE_BEGIN &&
            pEntry->nWID > FN_UNO_RANGE_END  &&
            pEntry->nWID < RES_CHRATR_BEGIN &&
            pEntry->nWID > RES_TXTATR_END )
        {
            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
        }
        else
        {
            if ( pEntry->nWID >= FN_UNO_RANGE_BEGIN &&
                 pEntry->nWID <= FN_UNO_RANGE_END )
            {
                (void)SwUnoCursorHelper::getCursorPropertyValue(
                    *pEntry, rPaM, nullptr, pStates[i] );
            }
            else
            {
                if (!pSet.get())
                {
                    switch ( eCaller )
                    {
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT:
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION:
                            pSet.reset(
                                new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN,   RES_TXTATR_END ));
                        break;
                        case SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY:
                            pSet.reset(
                                new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    pEntry->nWID, pEntry->nWID ));
                        break;
                        default:
                            pSet.reset( new SfxItemSet(
                                rPaM.GetDoc()->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                                RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                0L ));
                    }
                    // #i63870#
                    SwUnoCursorHelper::GetCursorAttr( rPaM, *pSet );
                }

                pStates[i] = ( pSet->Count() )
                    ? rPropSet.getPropertyState( *pEntry, *pSet )
                    : beans::PropertyState_DEFAULT_VALUE;

                //try again to find out if a value has been inherited
                if( beans::PropertyState_DIRECT_VALUE == pStates[i] )
                {
                    if (!pSetParent.get())
                    {
                        pSetParent.reset( pSet->Clone( false ) );
                        // #i63870#
                        SwUnoCursorHelper::GetCursorAttr(
                                rPaM, *pSetParent, true, false );
                    }

                    pStates[i] = ( (pSetParent)->Count() )
                        ? rPropSet.getPropertyState( *pEntry, *pSetParent )
                        : beans::PropertyState_DEFAULT_VALUE;
                }
            }
        }
    }
    return aRet;
}

beans::PropertyState SwUnoCursorHelper::GetPropertyState(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Sequence< OUString > aStrings { rPropertyName };
    uno::Sequence< beans::PropertyState > aSeq =
        GetPropertyStates(rPaM, rPropSet, aStrings,
                SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY );
    return aSeq[0];
}

static void
lcl_SelectParaAndReset( SwPaM &rPaM, SwDoc & rDoc,
                        std::set<sal_uInt16> const &rWhichIds )
{
    // if we are reseting paragraph attributes, we need to select the full paragraph first
    SwPosition aStart = *rPaM.Start();
    SwPosition aEnd = *rPaM.End();
    auto pTemp ( rDoc.CreateUnoCursor(aStart) );
    if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
    {
        pTemp->MovePara(fnParaCurr, fnParaStart);
    }
    pTemp->SetMark();
    *pTemp->GetPoint() = aEnd;
    SwUnoCursorHelper::SelectPam(*pTemp, true);
    if(!SwUnoCursorHelper::IsEndOfPara(*pTemp))
    {
        pTemp->MovePara(fnParaCurr, fnParaEnd);
    }
    rDoc.ResetAttrs(*pTemp, true, rWhichIds);
}

void SwUnoCursorHelper::SetPropertyToDefault(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SwDoc & rDoc = *rPaM.GetDoc();
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName,
            static_cast<cppu::OWeakObject *>(nullptr));
    }

    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(
                "setPropertyToDefault: property is read-only: "
                + rPropertyName, nullptr);
    }

    if (pEntry->nWID < RES_FRMATR_END)
    {
        std::set<sal_uInt16> aWhichIds;
        aWhichIds.insert( pEntry->nWID );
        if (pEntry->nWID < RES_PARATR_BEGIN)
        {
            rDoc.ResetAttrs(rPaM, true, aWhichIds);
        }
        else
        {
            lcl_SelectParaAndReset ( rPaM, rDoc, aWhichIds );
        }
    }
    else
    {
        SwUnoCursorHelper::resetCursorPropertyValue(*pEntry, rPaM);
    }
}

uno::Any SwUnoCursorHelper::GetPropertyDefault(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            "Unknown property: " + rPropertyName, static_cast<cppu::OWeakObject *>(nullptr));
    }

    uno::Any aRet;
    if (pEntry->nWID < RES_FRMATR_END)
    {
        SwDoc & rDoc = *rPaM.GetDoc();
        const SfxPoolItem& rDefItem =
            rDoc.GetAttrPool().GetDefaultItem(pEntry->nWID);
        rDefItem.QueryValue(aRet, pEntry->nMemberId);
    }
    return aRet;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextCursor::getPropertySetInfo() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo >  xRef;
    if(!xRef.is())
    {
        static SfxItemPropertyMapEntry const aCursorExtMap_Impl[] =
        {
            { OUString(UNO_NAME_IS_SKIP_HIDDEN_TEXT), FN_SKIP_HIDDEN_TEXT, cppu::UnoType<bool>::get(), PROPERTY_NONE,     0},
            { OUString(UNO_NAME_IS_SKIP_PROTECTED_TEXT), FN_SKIP_PROTECTED_TEXT, cppu::UnoType<bool>::get(), PROPERTY_NONE,     0},
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        const uno::Reference< beans::XPropertySetInfo >  xInfo =
            m_pImpl->m_rPropSet.getPropertySetInfo();
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xRef = new SfxExtItemPropertySetInfo(
            aCursorExtMap_Impl,
            aPropSeq );
    }
    return xRef;
}

void SAL_CALL
SwXTextCursor::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rPropertyName == UNO_NAME_IS_SKIP_HIDDEN_TEXT)
    {
        bool bSet(false);
        if (!(rValue >>= bSet))
        {
            throw lang::IllegalArgumentException();
        }
        rUnoCursor.SetSkipOverHiddenSections(bSet);
    }
    else if (rPropertyName == UNO_NAME_IS_SKIP_PROTECTED_TEXT)
    {
        bool bSet(false);
        if (!(rValue >>= bSet))
        {
            throw lang::IllegalArgumentException();
        }
        rUnoCursor.SetSkipOverProtectSections(bSet);
    }
    else
    {
        SwUnoCursorHelper::SetPropertyValue(rUnoCursor,
                m_pImpl->m_rPropSet, rPropertyName, rValue);
    }
}

uno::Any SAL_CALL
SwXTextCursor::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Any aAny;
    if (rPropertyName == UNO_NAME_IS_SKIP_HIDDEN_TEXT)
    {
        const bool bSet = rUnoCursor.IsSkipOverHiddenSections();
        aAny <<= bSet;
    }
    else if (rPropertyName == UNO_NAME_IS_SKIP_PROTECTED_TEXT)
    {
        const bool bSet = rUnoCursor.IsSkipOverProtectSections();
        aAny <<= bSet;
    }
    else
    {
        aAny = SwUnoCursorHelper::GetPropertyValue(rUnoCursor,
                m_pImpl->m_rPropSet, rPropertyName);
    }
    return aAny;
}

void SAL_CALL
SwXTextCursor::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextCursor::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextCursor::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextCursor::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextCursor::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const beans::PropertyState eRet = SwUnoCursorHelper::GetPropertyState(
            rUnoCursor, m_pImpl->m_rPropSet, rPropertyName);
    return eRet;
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextCursor::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    return SwUnoCursorHelper::GetPropertyStates(
            rUnoCursor, m_pImpl->m_rPropSet, rPropertyNames);
}

void SAL_CALL
SwXTextCursor::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    // forward: need no solar mutex here
    uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    setPropertiesToDefault ( aSequence );
}

uno::Any SAL_CALL
SwXTextCursor::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    // forward: need no solar mutex here
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

void SAL_CALL SwXTextCursor::setPropertyValues(
    const uno::Sequence< OUString >& aPropertyNames,
    const uno::Sequence< uno::Any >& aValues )
    throw (
        css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    if( aValues.getLength() != aPropertyNames.getLength() )
    {
        OSL_FAIL( "mis-matched property value sequences" );
        throw lang::IllegalArgumentException();
    }

    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // a little lame to have to copy into this.
    uno::Sequence< beans::PropertyValue > aPropertyValues( aValues.getLength() );
    for ( sal_Int32 i = 0; i < aPropertyNames.getLength(); i++ )
    {
        if ( aPropertyNames[ i ] == UNO_NAME_IS_SKIP_HIDDEN_TEXT ||
             aPropertyNames[ i ] == UNO_NAME_IS_SKIP_PROTECTED_TEXT )
        {
            // the behaviour of these is hard to model in a group
            OSL_FAIL("invalid property name for batch setting");
            throw lang::IllegalArgumentException();
        }
        aPropertyValues[ i ].Name = aPropertyNames[ i ];
        aPropertyValues[ i ].Value = aValues[ i ];
    }
    try
    {
        SwUnoCursorHelper::SetPropertyValues( rUnoCursor, m_pImpl->m_rPropSet, aPropertyValues );
    }
    catch (const css::beans::UnknownPropertyException& e)
    {
        uno::Any a(cppu::getCaughtException());
        throw lang::WrappedTargetException(
            "wrapped Exception " + e.Message,
            uno::Reference<uno::XInterface>(), a);
    }
}

uno::Sequence< uno::Any > SAL_CALL
SwXTextCursor::getPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
    throw (css::uno::RuntimeException, std::exception)
{
    // a banal implementation for now
    uno::Sequence< uno::Any > aValues( aPropertyNames.getLength() );
    for (sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        aValues[i] = getPropertyValue( aPropertyNames[ i ] );
    return aValues;
}

void SAL_CALL SwXTextCursor::addPropertiesChangeListener(
        const uno::Sequence< OUString >& /* aPropertyNames */,
        const uno::Reference< css::beans::XPropertiesChangeListener >& /* xListener */ )
    throw (css::uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::addPropertiesChangeListener(): not implemented");
}
void SAL_CALL SwXTextCursor::removePropertiesChangeListener(
        const uno::Reference< css::beans::XPropertiesChangeListener >& /* xListener */ )
    throw (css::uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::removePropertiesChangeListener(): not implemented");
}

void SAL_CALL SwXTextCursor::firePropertiesChangeEvent(
        const uno::Sequence< OUString >& /* aPropertyNames */,
        const uno::Reference< css::beans::XPropertiesChangeListener >& /* xListener */ )
    throw (css::uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextCursor::firePropertiesChangeEvent(): not implemented");
}

// para specific attribute ranges
static sal_uInt16 g_ParaResetableSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// selection specific attribute ranges
static sal_uInt16 g_ResetableSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
    RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
    0
};

static void
lcl_EnumerateIds(sal_uInt16 const* pIdRange, std::set<sal_uInt16> &rWhichIds)
{
    while (*pIdRange)
    {
        const sal_uInt16 nStart = *pIdRange++;
        const sal_uInt16 nEnd   = *pIdRange++;
        for (sal_uInt16 nId = nStart + 1;  nId <= nEnd;  ++nId)
        {
            rWhichIds.insert( rWhichIds.end(), nId );
        }
    }
}

void SAL_CALL
SwXTextCursor::setAllPropertiesToDefault()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    std::set<sal_uInt16> aParaWhichIds;
    std::set<sal_uInt16> aWhichIds;
    lcl_EnumerateIds(g_ParaResetableSetRange, aParaWhichIds);
    lcl_EnumerateIds(g_ResetableSetRange, aWhichIds);
    if (!aParaWhichIds.empty())
    {
        lcl_SelectParaAndReset(rUnoCursor, *rUnoCursor.GetDoc(),
            aParaWhichIds);
    }
    if (!aWhichIds.empty())
    {
        rUnoCursor.GetDoc()->ResetAttrs(rUnoCursor, true, aWhichIds);
    }
}

void SAL_CALL
SwXTextCursor::setPropertiesToDefault(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Int32 nCount = rPropertyNames.getLength();
    if ( nCount )
    {
        SwDoc & rDoc = *rUnoCursor.GetDoc();
        const OUString * pNames = rPropertyNames.getConstArray();
        std::set<sal_uInt16> aWhichIds;
        std::set<sal_uInt16> aParaWhichIds;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            SfxItemPropertySimpleEntry const*const  pEntry =
                m_pImpl->m_rPropSet.getPropertyMap().getByName( pNames[i] );
            if (!pEntry)
            {
                if (pNames[i] == UNO_NAME_IS_SKIP_HIDDEN_TEXT ||
                    pNames[i] == UNO_NAME_IS_SKIP_PROTECTED_TEXT)
                {
                    continue;
                }
                throw beans::UnknownPropertyException(
                    "Unknown property: " + pNames[i],
                    static_cast<cppu::OWeakObject *>(this));
            }
            if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
            {
                throw uno::RuntimeException(
                    "setPropertiesToDefault: property is read-only: " + pNames[i],
                    static_cast<cppu::OWeakObject *>(this));
            }

            if (pEntry->nWID < RES_FRMATR_END)
            {
                if (pEntry->nWID < RES_PARATR_BEGIN)
                {
                    aWhichIds.insert( pEntry->nWID );
                }
                else
                {
                    aParaWhichIds.insert( pEntry->nWID );
                }
            }
            else if (pEntry->nWID == FN_UNO_NUM_START_VALUE)
            {
                SwUnoCursorHelper::resetCursorPropertyValue(*pEntry, rUnoCursor);
            }
        }

        if (!aParaWhichIds.empty())
        {
            lcl_SelectParaAndReset(rUnoCursor, rDoc, aParaWhichIds);
        }
        if (!aWhichIds.empty())
        {
            rDoc.ResetAttrs(rUnoCursor, true, aWhichIds);
        }
    }
}

uno::Sequence< uno::Any > SAL_CALL
SwXTextCursor::getPropertyDefaults(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Int32 nCount = rPropertyNames.getLength();
    uno::Sequence< uno::Any > aRet(nCount);
    if ( nCount )
    {
        SwDoc & rDoc = *rUnoCursor.GetDoc();
        const OUString *pNames = rPropertyNames.getConstArray();
        uno::Any *pAny = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            SfxItemPropertySimpleEntry const*const pEntry =
                m_pImpl->m_rPropSet.getPropertyMap().getByName( pNames[i] );
            if (!pEntry)
            {
                if (pNames[i] == UNO_NAME_IS_SKIP_HIDDEN_TEXT ||
                    pNames[i] == UNO_NAME_IS_SKIP_PROTECTED_TEXT)
                {
                    continue;
                }
                throw beans::UnknownPropertyException(
                    "Unknown property: " + pNames[i],
                    static_cast<cppu::OWeakObject *>(nullptr));
            }
            if (pEntry->nWID < RES_FRMATR_END)
            {
                const SfxPoolItem& rDefItem =
                    rDoc.GetAttrPool().GetDefaultItem(pEntry->nWID);
                rDefItem.QueryValue(pAny[i], pEntry->nMemberId);
            }
        }
    }
    return aRet;
}

void SAL_CALL SwXTextCursor::invalidateMarkings(::sal_Int32 nType)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwNode& node = rUnoCursor.GetNode();

    SwTextNode* txtNode = node.GetTextNode();

    if (txtNode == nullptr) return;

    if ( text::TextMarkupType::SPELLCHECK == nType )
    {
        txtNode->SetWrongDirty(SwTextNode::WrongState::TODO);
        txtNode->SetWrong(nullptr);
    }
    else if( text::TextMarkupType::PROOFREADING == nType )
    {
        txtNode->SetGrammarCheckDirty(true);
        txtNode->SetGrammarCheck(nullptr);
    }
    else if ( text::TextMarkupType::SMARTTAG == nType )
    {
        txtNode->SetSmartTagDirty(true);
        txtNode->SetSmartTags(nullptr);
    }
    else return;

    SwFormatColl* fmtColl=txtNode->GetFormatColl();

    if (fmtColl == nullptr) return;

    SwFormatChg aNew( fmtColl );
    txtNode->NotifyClients( nullptr, &aNew );
}

void SAL_CALL
SwXTextCursor::makeRedline(
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::makeRedline(rUnoCursor, rRedlineType, rRedlineProperties);
}

void SAL_CALL SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
throw (lang::IllegalArgumentException, io::IOException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::InsertFile(&rUnoCursor, rURL, rOptions);
}

uno::Sequence< beans::PropertyValue >
SwUnoCursorHelper::CreateSortDescriptor(const bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(5);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    aVal <<= bFromTable;
    pArray[0] = beans::PropertyValue("IsSortInTable", -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= sal_Unicode(' ');
    pArray[1] = beans::PropertyValue("Delimiter", -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= false;
    pArray[2] = beans::PropertyValue("IsSortColumns", -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32) 3;
    pArray[3] = beans::PropertyValue("MaxSortFieldsCount", -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    uno::Sequence< table::TableSortField > aFields(3);
    table::TableSortField* pFields = aFields.getArray();

    lang::Locale aLang( SvtSysLocale().GetLanguageTag().getLocale());
    // get collator algorithm to be used for the locale
    uno::Sequence< OUString > aSeq(
            GetAppCollator().listCollatorAlgorithms( aLang ) );
    const sal_Int32 nLen = aSeq.getLength();
    OSL_ENSURE( nLen > 0, "list of collator algorithms is empty!");
    OUString aCollAlg;
    if (nLen > 0)
    {
        aCollAlg = aSeq.getConstArray()[0];
    }

#if OSL_DEBUG_LEVEL > 1
    const OUString *pText = aSeq.getConstArray();
    (void)pText;
#endif

    pFields[0].Field = 1;
    pFields[0].IsAscending = sal_True;
    pFields[0].IsCaseSensitive = sal_False;
    pFields[0].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[0].CollatorLocale = aLang;
    pFields[0].CollatorAlgorithm = aCollAlg;

    pFields[1].Field = 1;
    pFields[1].IsAscending = sal_True;
    pFields[1].IsCaseSensitive = sal_False;
    pFields[1].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[1].CollatorLocale = aLang;
    pFields[1].CollatorAlgorithm = aCollAlg;

    pFields[2].Field = 1;
    pFields[2].IsAscending = sal_True;
    pFields[2].IsCaseSensitive = sal_False;
    pFields[2].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[2].CollatorLocale = aLang;
    pFields[2].CollatorAlgorithm = aCollAlg;

    aVal <<= aFields;
    pArray[4] = beans::PropertyValue("SortFields", -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    return aRet;
}

uno::Sequence< beans::PropertyValue > SAL_CALL
SwXTextCursor::createSortDescriptor() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return SwUnoCursorHelper::CreateSortDescriptor(false);
}

bool SwUnoCursorHelper::ConvertSortProperties(
    const uno::Sequence< beans::PropertyValue >& rDescriptor,
    SwSortOptions& rSortOpt)
{
    bool bRet = true;
    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();

    rSortOpt.bTable = false;
    rSortOpt.cDeli = ' ';
    rSortOpt.eDirection = SRT_COLUMNS;  //!! UI text may be contrary though !!

    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->bIsNumeric = true;
    pKey1->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->bIsNumeric = true;
    pKey2->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->bIsNumeric = true;
    pKey3->eSortOrder = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    bool bOldSortdescriptor(false);
    bool bNewSortdescriptor(false);

    for (sal_Int32 n = 0; n < rDescriptor.getLength(); ++n)
    {
        uno::Any aValue( pProperties[n].Value );
        const OUString& rPropName = pProperties[n].Name;

        // old and new sortdescriptor
        if ( rPropName == "IsSortInTable" )
        {
            if (aValue.getValueType() == cppu::UnoType<bool>::get())
            {
                rSortOpt.bTable = *static_cast<sal_Bool const *>(aValue.getValue());
            }
            else
            {
                bRet = false;
            }
        }
        else if ( rPropName == "Delimiter" )
        {
            sal_Unicode uChar;
            if (aValue >>= uChar)
            {
                rSortOpt.cDeli = uChar;
            }
            else
            {
                bRet = false;
            }
        }
        // old sortdescriptor
        else if ( rPropName == "SortColumns" )
        {
            bOldSortdescriptor = true;
            bool bTemp(false);
            if (aValue >>= bTemp)
            {
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = false;
            }
        }
        else if ( rPropName == "IsCaseSensitive" )
        {
            bOldSortdescriptor = true;
            bool bTemp(false);
            if (aValue >>= bTemp)
            {
                rSortOpt.bIgnoreCase = !bTemp;
            }
            else
            {
                bRet = false;
            }
        }
        else if ( rPropName == "CollatorLocale" )
        {
            bOldSortdescriptor = true;
            lang::Locale aLocale;
            if (aValue >>= aLocale)
            {
                rSortOpt.nLanguage = LanguageTag::convertToLanguageType( aLocale);
            }
            else
            {
                bRet = false;
            }
        }
        else if (rPropName.startsWith("CollatorAlgorithm") &&
            rPropName.getLength() == 18 &&
            (rPropName[17] >= '0' && rPropName[17] <= '9'))
        {
            bOldSortdescriptor = true;
            sal_uInt16 nIndex = rPropName[17];
            nIndex -= '0';
            OUString aText;
            if ((aValue >>= aText) && nIndex < 3)
            {
                aKeys[nIndex]->sSortType = aText;
            }
            else
            {
                bRet = false;
            }
        }
        else if (rPropName.startsWith("SortRowOrColumnNo") &&
            rPropName.getLength() == 18 &&
            (rPropName[17] >= '0' && rPropName[17] <= '9'))
        {
            bOldSortdescriptor = true;
            sal_uInt16 nIndex = rPropName[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if (aValue.getValueType() == ::cppu::UnoType<sal_Int16>::get()
                && nIndex < 3)
            {
                aValue >>= nCol;
            }
            if (nCol >= 0)
            {
                aKeys[nIndex]->nColumnId = nCol;
            }
            else
            {
                bRet = false;
            }
        }
        else if (rPropName.startsWith("IsSortNumeric") &&
            rPropName.getLength() == 14 &&
            (rPropName[13] >= '0' && rPropName[13] <= '9'))
        {
            bOldSortdescriptor = true;
            sal_uInt16 nIndex = rPropName[13];
            nIndex = nIndex - '0';
            if (aValue.getValueType() == cppu::UnoType<bool>::get() && nIndex < 3)
            {
                bool bTemp = *static_cast<sal_Bool const *>(aValue.getValue());
                aKeys[nIndex]->bIsNumeric = bTemp;
            }
            else
            {
                bRet = false;
            }
        }
        else if (rPropName.startsWith("IsSortAscending") &&
            rPropName.getLength() == 16 &&
            (rPropName[15] >= '0' && rPropName[15] <= '9'))
        {
            bOldSortdescriptor = true;
            sal_uInt16 nIndex = rPropName[15];
            nIndex -= '0';
            if (aValue.getValueType() == cppu::UnoType<bool>::get() && nIndex < 3)
            {
                bool bTemp = *static_cast<sal_Bool const *>(aValue.getValue());
                aKeys[nIndex]->eSortOrder = (bTemp)
                    ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
            {
                bRet = false;
            }
        }
        // new sortdescriptor
        else if ( rPropName == "IsSortColumns" )
        {
            bNewSortdescriptor = true;
            if (aValue.getValueType() == cppu::UnoType<bool>::get())
            {
                bool bTemp = *static_cast<sal_Bool const *>(aValue.getValue());
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = false;
            }
        }
        else if ( rPropName == "SortFields" )
        {
            bNewSortdescriptor = true;
            uno::Sequence < table::TableSortField > aFields;
            if (aValue >>= aFields)
            {
                sal_Int32 nCount(aFields.getLength());
                if (nCount <= 3)
                {
                    table::TableSortField* pFields = aFields.getArray();
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        rSortOpt.bIgnoreCase = !pFields[i].IsCaseSensitive;
                        rSortOpt.nLanguage =
                            LanguageTag::convertToLanguageType( pFields[i].CollatorLocale );
                        aKeys[i]->sSortType = pFields[i].CollatorAlgorithm;
                        aKeys[i]->nColumnId =
                            static_cast<sal_uInt16>(pFields[i].Field);
                        aKeys[i]->bIsNumeric = (pFields[i].FieldType ==
                                table::TableSortFieldType_NUMERIC);
                        aKeys[i]->eSortOrder = (pFields[i].IsAscending)
                            ? SRT_ASCENDING : SRT_DESCENDING;
                    }
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                bRet = false;
            }
        }
    }

    if (bNewSortdescriptor && bOldSortdescriptor)
    {
        OSL_FAIL("someone tried to set the old deprecated and "
            "the new sortdescriptor");
        bRet = false;
    }

    if (pKey1->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.push_back(pKey1);
    }
    if (pKey2->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.push_back(pKey2);
    }
    if (pKey3->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.push_back(pKey3);
    }

    return bRet && !rSortOpt.aKeys.empty();
}

void SAL_CALL
SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        SwSortOptions aSortOpt;
        if (!SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
        {
            throw uno::RuntimeException();
        }
        UnoActionContext aContext( rUnoCursor.GetDoc() );

        SwPosition & rStart = *rUnoCursor.Start();
        SwPosition & rEnd   = *rUnoCursor.End();

        SwNodeIndex aPrevIdx( rStart.nNode, -1 );
        const sal_uLong nOffset = rEnd.nNode.GetIndex() - rStart.nNode.GetIndex();
        const sal_Int32 nCntStt  = rStart.nContent.GetIndex();

        rUnoCursor.GetDoc()->SortText(rUnoCursor, aSortOpt);

        // update selection
        rUnoCursor.DeleteMark();
        rUnoCursor.GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
        SwContentNode *const pCNd = rUnoCursor.GetContentNode();
        sal_Int32 nLen = pCNd->Len();
        if (nLen > nCntStt)
        {
            nLen = nCntStt;
        }
        rUnoCursor.GetPoint()->nContent.Assign(pCNd, nLen );
        rUnoCursor.SetMark();

        rUnoCursor.GetPoint()->nNode += nOffset;
        SwContentNode *const pCNd2 = rUnoCursor.GetContentNode();
        rUnoCursor.GetPoint()->nContent.Assign( pCNd2, pCNd2->Len() );
    }
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createContentEnumeration(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    if (rServiceName != "com.sun.star.text.TextContent")
        throw uno::RuntimeException();
    SwUnoCursor& rUnoCursor( m_pImpl->GetCursorOrThrow() );
    return SwXParaFrameEnumeration::Create(rUnoCursor, PARAFRAME_PORTION_TEXTRANGE);
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createEnumeration() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    SwUnoCursor & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const uno::Reference<lang::XUnoTunnel> xTunnel(
            m_pImpl->m_xParentText, uno::UNO_QUERY);
    SwXText* pParentText = nullptr;
    if (xTunnel.is())
    {
        pParentText = ::sw::UnoTunnelGetImplementation<SwXText>(xTunnel);
    }
    OSL_ENSURE(pParentText, "parent is not a SwXText");
    if (!pParentText)
    {
        throw uno::RuntimeException();
    }

    auto pNewCursor(rUnoCursor.GetDoc()->CreateUnoCursor(*rUnoCursor.GetPoint()) );
    if (rUnoCursor.HasMark())
    {
        pNewCursor->SetMark();
        *pNewCursor->GetMark() = *rUnoCursor.GetMark();
    }
    const CursorType eSetType = (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    SwTableNode const*const pStartNode( (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ? rUnoCursor.GetPoint()->nNode.GetNode().FindTableNode()
            : nullptr);
    SwTable const*const pTable(
            (pStartNode) ? & pStartNode->GetTable() : nullptr );
    return SwXParagraphEnumeration::Create(pParentText, pNewCursor, eSetType, pStartNode, pTable);
}

uno::Type SAL_CALL
SwXTextCursor::getElementType() throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXTextCursor::hasElements() throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

uno::Sequence< OUString > SAL_CALL
SwXTextCursor::getAvailableServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.TextContent" };
    return aRet;
}

IMPLEMENT_FORWARD_REFCOUNT( SwXTextCursor,SwXTextCursor_Base )

uno::Any SAL_CALL
SwXTextCursor::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException, std::exception)
{
    return (rType == cppu::UnoType<lang::XUnoTunnel>::get())
        ? OTextCursorHelper::queryInterface(rType)
        : SwXTextCursor_Base::queryInterface(rType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
