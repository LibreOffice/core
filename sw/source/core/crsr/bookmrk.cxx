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

#include <bookmrk.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swserv.hxx>
#include <sfx2/linkmgr.hxx>
#include <swtypes.hxx>
#include <UndoBookmark.hxx>
#include <unobookmark.hxx>
#include <rtl/random.h>
#include <xmloff/odffields.hxx>

SV_IMPL_REF( SwServerObject )

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    static void lcl_FixPosition(SwPosition& rPos)
    {
        // make sure the position has 1) the proper node, and 2) a proper index
        SwTxtNode* pTxtNode = rPos.nNode.GetNode().GetTxtNode();
        if(pTxtNode == NULL && rPos.nContent.GetIndex() > 0)
        {
            OSL_TRACE(
                "bookmrk.cxx::lcl_FixPosition"
                " - illegal position: %d without proper TxtNode", rPos.nContent.GetIndex());
            rPos.nContent.Assign(NULL, 0);
        }
        else if(pTxtNode != NULL && rPos.nContent.GetIndex() > pTxtNode->Len())
        {
            OSL_TRACE(
                "bookmrk.cxx::lcl_FixPosition"
                " - illegal position: %d is beyond %d", rPos.nContent.GetIndex(), pTxtNode->Len());
            rPos.nContent.Assign(pTxtNode, pTxtNode->Len());
        }
    }

    static void lcl_AssureFieldMarksSet(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        SwPosition& rStart = pField->GetMarkStart();
        SwPosition& rEnd = pField->GetMarkEnd();
        SwTxtNode const*const pStartTxtNode =
            rStart.nNode.GetNode().GetTxtNode();
        SwTxtNode const*const pEndTxtNode = rEnd.nNode.GetNode().GetTxtNode();
        const sal_Unicode ch_start =
            pStartTxtNode->GetTxt()[rStart.nContent.GetIndex()];
        xub_StrLen nEndPos = ( rEnd == rStart ||  rEnd.nContent.GetIndex() == 0 ) ?
            rEnd.nContent.GetIndex() : rEnd.nContent.GetIndex() - 1;
        const sal_Unicode ch_end = pEndTxtNode->GetTxt()[nEndPos];
        SwPaM aStartPaM(rStart);
        SwPaM aEndPaM(rEnd);
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, NULL);
        if( ( ch_start != aStartMark ) && ( aEndMark != CH_TXT_ATR_FORMELEMENT ) )
        {
            io_pDoc->InsertString(aStartPaM, OUString(aStartMark));
            rStart.nContent--;
        }
        if ( aEndMark && ( ch_end != aEndMark ) )
        {
            io_pDoc->InsertString(aEndPaM, OUString(aEndMark));
            rEnd.nContent++;
        }
        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, NULL);
    };

    static void lcl_RemoveFieldMarks(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        SwPosition& rStart = pField->GetMarkStart();
        SwPosition& rEnd = pField->GetMarkEnd();
        SwTxtNode const*const pStartTxtNode =
            rStart.nNode.GetNode().GetTxtNode();
        SwTxtNode const*const pEndTxtNode = rEnd.nNode.GetNode().GetTxtNode();
        const sal_Unicode ch_start =
            pStartTxtNode->GetTxt()[rStart.nContent.GetIndex()];
        xub_StrLen nEndPos = ( rEnd == rStart ||  rEnd.nContent.GetIndex() == 0 ) ?
            rEnd.nContent.GetIndex() : rEnd.nContent.GetIndex() - 1;
        const sal_Unicode ch_end = pEndTxtNode->GetTxt()[nEndPos];
        SwPaM aStartPaM(rStart);
        SwPaM aEndPaM(rEnd);
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, NULL);
        if( ch_start == aStartMark )
        {
            SwPaM aStart(rStart, rStart);
            aStart.End()->nContent++;
            io_pDoc->DeleteRange(aStart);
        }
        if ( ch_end == aEndMark )
        {
            SwPaM aEnd(rEnd, rEnd);
            aEnd.Start()->nContent--;
            io_pDoc->DeleteRange(aEnd);
        }
        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, NULL);
    };
}

namespace sw { namespace mark
{
    MarkBase::MarkBase(const SwPaM& aPaM,
        const OUString& rName)
        : SwModify(0)
        , m_pPos1(new SwPosition(*(aPaM.GetPoint())))
        , m_aName(rName)
    {
        lcl_FixPosition(*m_pPos1);
        if (aPaM.HasMark() && (*aPaM.GetMark() != *aPaM.GetPoint()))
        {
            MarkBase::SetOtherMarkPos(*(aPaM.GetMark()));
            lcl_FixPosition(*m_pPos2);
        }
    }

    // For fieldmarks, the CH_TXT_ATR_FIELDSTART and CH_TXT_ATR_FIELDEND
    // themselves are part of the covered range. This is guaranteed by
    // TextFieldmark::InitDoc/lcl_AssureFieldMarksSet.
    bool MarkBase::IsCoveringPosition(const SwPosition& rPos) const
    {
        return GetMarkStart() <= rPos && rPos < GetMarkEnd();
    }

    void MarkBase::SetMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos1);
    }

    void MarkBase::SetOtherMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos2);
    }

    OUString MarkBase::ToString( ) const
    {
        return "Mark: ( Name, [ Node1, Index1 ] ): ( " + m_aName + ", [ "
            + OUString::number( GetMarkPos().nNode.GetIndex( ) )  + ", "
            + OUString::number( GetMarkPos().nContent.GetIndex( ) ) + " ] )";
    }

    MarkBase::~MarkBase()
    { }

    OUString MarkBase::GenerateNewName(const OUString& rPrefix)
    {
        static rtlRandomPool aPool = rtl_random_createPool();
        static OUString sUniquePostfix;
        static sal_Int32 nCount = SAL_MAX_INT32;
        OUStringBuffer aResult(rPrefix);
        if(nCount == SAL_MAX_INT32)
        {
            sal_Int32 nRandom;
            rtl_random_getBytes(aPool, &nRandom, sizeof(nRandom));
            sUniquePostfix = OUStringBuffer(13).append('_').append(static_cast<sal_Int32>(abs(nRandom))).makeStringAndClear();
            nCount = 0;
        }
        // putting the counter in front of the random parts will speed up string comparisons
        return aResult.append(nCount++).append(sUniquePostfix).makeStringAndClear();
    }

    void MarkBase::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
    {
        NotifyClients(pOld, pNew);
        if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
        {   // invalidate cached uno object
            SetXBookmark(uno::Reference<text::XTextContent>(0));
        }
    }

    // TODO: everything else uses MarkBase::GenerateNewName ?
    NavigatorReminder::NavigatorReminder(const SwPaM& rPaM)
        : MarkBase(rPaM, OUString("__NavigatorReminder__"))
    { }

    UnoMark::UnoMark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(OUString("__UnoMark__")))
    { }

    DdeBookmark::DdeBookmark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(OUString("__DdeLink__")))
        , m_aRefObj(NULL)
    { }

    void DdeBookmark::SetRefObject(SwServerObject* pObj)
    {
        m_aRefObj = pObj;
    }

    void DdeBookmark::DeregisterFromDoc(SwDoc* const pDoc)
    {
        if(m_aRefObj.Is())
            pDoc->GetLinkManager().RemoveServer(m_aRefObj);
    }

    DdeBookmark::~DdeBookmark()
    {
        if( m_aRefObj.Is() )
        {
            if(m_aRefObj->HasDataLinks())
            {
                ::sfx2::SvLinkSource* p = &m_aRefObj;
                p->SendDataChanged();
            }
            m_aRefObj->SetNoServer();
        }
    }

    Bookmark::Bookmark(const SwPaM& aPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : DdeBookmark(aPaM)
        , ::sfx2::Metadatable()
        , m_aCode(rCode)
        , m_sShortName(rShortName)
    {
        m_aName = rName;
    }

    void Bookmark::InitDoc(SwDoc* const io_pDoc)
    {
        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo(
                    new SwUndoInsBookmark(*this));
        }
        io_pDoc->SetModified();
    }

    ::sfx2::IXmlIdRegistry& Bookmark::GetRegistry()
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::MakeUnoObject: no doc?");
        return pDoc->GetXmlIdRegistry();
    }

    bool Bookmark::IsInClipboard() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::IsInClipboard: no doc?");
        return pDoc->IsClipBoard();
    }

    bool Bookmark::IsInUndo() const
    {
        return false;
    }

    bool Bookmark::IsInContent() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::IsInContent: no doc?");
        return !pDoc->IsInHeaderFooter( SwNodeIndex(GetMarkPos().nNode) );
    }

    uno::Reference< rdf::XMetadatable > Bookmark::MakeUnoObject()
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::MakeUnoObject: no doc?");
        const uno::Reference< rdf::XMetadatable> xMeta(
                SwXBookmark::CreateXBookmark(*pDoc, *this), uno::UNO_QUERY);
        return xMeta;
    }

    Fieldmark::Fieldmark(const SwPaM& rPaM)
        : MarkBase(rPaM, MarkBase::GenerateNewName(OUString("__Fieldmark__")))
    {
        if(!IsExpanded())
            SetOtherMarkPos(GetMarkPos());
    }

    OUString Fieldmark::ToString( ) const
    {
        return "Fieldmark: ( Name, Type, [ Nd1, Id1 ], [ Nd2, Id2 ] ): ( " + m_aName + ", "
            + m_aFieldname + ", [ " + OUString::number( GetMarkPos().nNode.GetIndex( ) )
            + ", " + OUString::number( GetMarkPos( ).nContent.GetIndex( ) ) + " ], ["
            + OUString::number( GetOtherMarkPos().nNode.GetIndex( ) ) + ", "
            + OUString::number( GetOtherMarkPos( ).nContent.GetIndex( ) ) + " ] ) ";
    }

    void Fieldmark::Invalidate( )
    {
        // TODO: Does exist a better solution to trigger a format of the
        //       fieldmark portion? If yes, please use it.
        SwPaM aPaM( this->GetMarkPos(), this->GetOtherMarkPos() );
        aPaM.InvalidatePaM();
    }

    TextFieldmark::TextFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void TextFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    void TextFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    CheckboxFieldmark::CheckboxFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void CheckboxFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);

        // For some reason the end mark is moved from 1 by the Insert: we don't
        // want this for checkboxes
        this->GetMarkEnd( ).nContent--;
    }

    void CheckboxFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc,
                CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);
    }

    void CheckboxFieldmark::SetChecked(bool checked)
    {
        if ( IsChecked() != checked )
        {
            (*GetParameters())[OUString(ODF_FORMCHECKBOX_RESULT)] = makeAny(checked);
            // mark document as modified
            SwDoc *const pDoc( GetMarkPos().GetDoc() );
            if ( pDoc )
                pDoc->SetModified();
        }
    }

    bool CheckboxFieldmark::IsChecked() const
    {
        bool bResult = false;
        parameter_map_t::const_iterator pResult = GetParameters()->find(OUString(ODF_FORMCHECKBOX_RESULT));
        if(pResult != GetParameters()->end())
            pResult->second >>= bResult;
        return bResult;
    }

    OUString CheckboxFieldmark::toString( ) const
    {

        return "CheckboxFieldmark: ( Name, Type, [ Nd1, Id1 ], [ Nd2, Id2 ] ): ( "
            + m_aName + ", " + GetFieldname() + ", [ "
            + OUString::number(GetMarkPos().nNode.GetIndex( ) ) + ", "
            + OUString::number( GetMarkPos( ).nContent.GetIndex( ) ) + " ], ["
            + OUString::number( GetOtherMarkPos().nNode.GetIndex( ) ) + ", "
            + OUString::number( GetOtherMarkPos( ).nContent.GetIndex( ) ) + " ] ) ";
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
