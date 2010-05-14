/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bookmrk.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <bookmrk.hxx>
#include <IDocumentMarkAccess.hxx>
#include <doc.hxx>
#include <errhdl.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swserv.hxx>
#include <svx/linkmgr.hxx>
#include <swtypes.hxx>
#include <undobj.hxx>
#include <rtl/random.h>

SV_IMPL_REF( SwServerObject )

using namespace ::sw::mark;

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
    };

    static void lcl_AssureFieldMarksSet(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        const SwPosition& rStart = pField->GetMarkStart();
        const SwPosition& rEnd = pField->GetMarkEnd();
        SwTxtNode const * const pStartTxtNode = io_pDoc->GetNodes()[rStart.nNode]->GetTxtNode();
        SwTxtNode const * const pEndTxtNode = io_pDoc->GetNodes()[rEnd.nNode]->GetTxtNode();
        const sal_Unicode ch_start=pStartTxtNode->GetTxt().GetChar(rStart.nContent.GetIndex());
        const sal_Unicode ch_end=pEndTxtNode->GetTxt().GetChar(rEnd.nContent.GetIndex()-1);
        const SwPaM aStartPaM(rStart);
        const SwPaM aEndPaM(rEnd);
        io_pDoc->StartUndo(UNDO_UI_REPLACE, NULL);
        if(ch_start != aStartMark)
            io_pDoc->Insert(aStartPaM, aStartMark);
        if(aEndMark && ch_end != aEndMark)
            io_pDoc->Insert(aEndPaM, aEndMark);
        io_pDoc->EndUndo(UNDO_UI_REPLACE, NULL);
    };
}

namespace sw { namespace mark
{
    MarkBase::MarkBase(const SwPaM& aPaM,
        const ::rtl::OUString& rName)
        : SwModify(0)
        , m_pPos1(new SwPosition(*(aPaM.GetPoint())))
        , m_aName(rName)
    {
        lcl_FixPosition(*m_pPos1);
        if(aPaM.HasMark())
        {
            MarkBase::SetOtherMarkPos(*(aPaM.GetMark()));
            lcl_FixPosition(*m_pPos2);
        }
    }

    void MarkBase::SetMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos1);
        //lcl_FixPosition(*m_pPos1);
    }

    void MarkBase::SetOtherMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos2);
        //lcl_FixPosition(*m_pPos2);
    }

    MarkBase::~MarkBase()
    { }

    ::rtl::OUString MarkBase::GenerateNewName(const ::rtl::OUString& rPrefix)
    {
        static rtlRandomPool aPool = rtl_random_createPool();
        static ::rtl::OUString sUniquePostfix;
        static sal_Int32 nCount = SAL_MAX_INT32;
        ::rtl::OUStringBuffer aResult(rPrefix);
        if(nCount == SAL_MAX_INT32)
        {
            sal_Int32 nRandom;
            ::rtl::OUStringBuffer sUniquePostfixBuffer;
            rtl_random_getBytes(aPool, &nRandom, sizeof(nRandom));
            sUniquePostfix = ::rtl::OUStringBuffer(13).appendAscii("_").append(static_cast<sal_Int32>(abs(nRandom))).makeStringAndClear();
            nCount = 0;
        }
        // putting the counter in front of the random parts will speed up string comparisons
        return aResult.append(nCount++).append(sUniquePostfix).makeStringAndClear();
    }


    NavigatorReminder::NavigatorReminder(const SwPaM& rPaM)
        : MarkBase(rPaM, our_sNamePrefix)
    { }

    const ::rtl::OUString NavigatorReminder::our_sNamePrefix = ::rtl::OUString::createFromAscii("__NavigatorReminder__");

    UnoMark::UnoMark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    { }

    const ::rtl::OUString UnoMark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__UnoMark__");

    DdeBookmark::DdeBookmark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
        , m_aRefObj(NULL)
    { }

    void DdeBookmark::SetRefObject(SwServerObject* pObj)
    {
        m_aRefObj = pObj;
    }

    const ::rtl::OUString DdeBookmark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__DdeLink__");

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
        const ::rtl::OUString& rName,
        const ::rtl::OUString& rShortName)
        : DdeBookmark(aPaM)
        , m_aCode(rCode)
        , m_sShortName(rShortName)
    {
        m_aName = rName;
    }

    void Bookmark::InitDoc(SwDoc* const io_pDoc)
    {
        if(io_pDoc->DoesUndo())
        {
            io_pDoc->ClearRedo();
            io_pDoc->AppendUndo(new SwUndoInsBookmark(*this));
        }
        io_pDoc->SetModified();
    }

    Fieldmark::Fieldmark(const SwPaM& rPaM)
        : MarkBase(rPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    {
        if(!IsExpanded())
            SetOtherMarkPos(GetMarkPos());
    }

    const ::rtl::OUString Fieldmark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__Fieldmark__");

    TextFieldmark::TextFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void TextFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    CheckboxFieldmark::CheckboxFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void CheckboxFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    void CheckboxFieldmark::SetChecked(bool checked)
        { m_isChecked = checked; }

    bool CheckboxFieldmark::IsChecked() const
        { return m_isChecked; }
}}
