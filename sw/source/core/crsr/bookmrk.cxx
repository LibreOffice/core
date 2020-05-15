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
#include <bookmrk.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentState.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swserv.hxx>
#include <sfx2/linkmgr.hxx>
#include <swtypes.hxx>
#include <UndoBookmark.hxx>
#include <unobookmark.hxx>
#include <o3tl/make_unique.hxx>
#include <xmloff/odffields.hxx>
#include <libxml/xmlwriter.h>
#include <comphelper/random.hxx>
#include <comphelper/anytostring.hxx>
#include <sal/log.hxx>
#include <edtwin.hxx>
#include <DateFormFieldButton.hxx>
#include <DropDownFormFieldButton.hxx>
#include <svx/numfmtsh.hxx>
#include <ndtxt.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <comphelper/lok.hxx>
#include <view.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <wrtsh.hxx>
#include <rtl/strbuf.hxx>
#include <strings.hrc>

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    void lcl_FixPosition(SwPosition& rPos)
    {
        // make sure the position has 1) the proper node, and 2) a proper index
        SwTextNode* pTextNode = rPos.nNode.GetNode().GetTextNode();
        if(pTextNode == nullptr && rPos.nContent.GetIndex() > 0)
        {
            SAL_INFO(
                "sw.core",
                "illegal position: " << rPos.nContent.GetIndex()
                    << " without proper TextNode");
            rPos.nContent.Assign(nullptr, 0);
        }
        else if(pTextNode != nullptr && rPos.nContent.GetIndex() > pTextNode->Len())
        {
            SAL_INFO(
                "sw.core",
                "illegal position: " << rPos.nContent.GetIndex()
                    << " is beyond " << pTextNode->Len());
            rPos.nContent.Assign(pTextNode, pTextNode->Len());
        }
    }

    void lcl_AssertFieldMarksSet(Fieldmark const * const pField,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        if (aEndMark != CH_TXT_ATR_FORMELEMENT)
        {
            SwPosition const& rStart(pField->GetMarkStart());
            assert(rStart.nNode.GetNode().GetTextNode()->GetText()[rStart.nContent.GetIndex()] == aStartMark); (void) rStart; (void) aStartMark;
        }
        SwPosition const& rEnd(pField->GetMarkEnd());
        assert(rEnd.nNode.GetNode().GetTextNode()->GetText()[rEnd.nContent.GetIndex() - 1] == aEndMark); (void) rEnd;
    }

    void lcl_SetFieldMarks(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::UI_REPLACE, nullptr);

        SwPosition start = pField->GetMarkStart();
        if (aEndMark != CH_TXT_ATR_FORMELEMENT)
        {
            SwPaM aStartPaM(start);
            io_pDoc->getIDocumentContentOperations().InsertString(aStartPaM, OUString(aStartMark));
            --start.nContent; // restore, it was moved by InsertString
            // do not manipulate via reference directly but call SetMarkStartPos
            // which works even if start and end pos were the same
            pField->SetMarkStartPos( start );
        }

        SwPosition& rEnd = pField->GetMarkEnd();
        if (aEndMark)
        {
            SwPaM aEndPaM(rEnd);
            io_pDoc->getIDocumentContentOperations().InsertString(aEndPaM, OUString(aEndMark));
            ++rEnd.nContent;
        }

        io_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::UI_REPLACE, nullptr);
    };

    void lcl_RemoveFieldMarks(Fieldmark const * const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::UI_REPLACE, nullptr);

        const SwPosition& rStart = pField->GetMarkStart();
        SwTextNode const*const pStartTextNode = rStart.nNode.GetNode().GetTextNode();
        assert(pStartTextNode);
        if (aEndMark != CH_TXT_ATR_FORMELEMENT)
        {
            (void) aStartMark;
            (void) pStartTextNode;
            assert(pStartTextNode->GetText()[rStart.nContent.GetIndex()] == aStartMark);
            SwPaM aStart(rStart, rStart);
            ++aStart.End()->nContent;
            io_pDoc->getIDocumentContentOperations().DeleteRange(aStart);
        }

        const SwPosition& rEnd = pField->GetMarkEnd();
        SwTextNode const*const pEndTextNode = rEnd.nNode.GetNode().GetTextNode();
        assert(pEndTextNode);
        const sal_Int32 nEndPos = (rEnd == rStart)
                                   ? rEnd.nContent.GetIndex()
                                   : rEnd.nContent.GetIndex() - 1;
        assert(pEndTextNode->GetText()[nEndPos] == aEndMark);
        (void) pEndTextNode;
        (void) nEndPos;
        SwPaM aEnd(rEnd, rEnd);
        if (aEnd.Start()->nContent > 0)
            --aEnd.Start()->nContent;
        io_pDoc->getIDocumentContentOperations().DeleteRange(aEnd);

        io_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::UI_REPLACE, nullptr);
    };
}

namespace sw { namespace mark
{
    MarkBase::MarkBase(const SwPaM& aPaM,
        const OUString& rName)
        : SwModify(nullptr)
        , m_pPos1(new SwPosition(*(aPaM.GetPoint())))
        , m_aName(rName)
    {
        m_pPos1->nContent.SetMark(this);
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
        o3tl::make_unique<SwPosition>(rNewPos).swap(m_pPos1);
        m_pPos1->nContent.SetMark(this);
    }

    void MarkBase::SetOtherMarkPos(const SwPosition& rNewPos)
    {
        o3tl::make_unique<SwPosition>(rNewPos).swap(m_pPos2);
        m_pPos2->nContent.SetMark(this);
    }

    OUString MarkBase::ToString( ) const
    {
        return "Mark: ( Name, [ Node1, Index1 ] ): ( " + m_aName + ", [ "
            + OUString::number( GetMarkPos().nNode.GetIndex( ) )  + ", "
            + OUString::number( GetMarkPos().nContent.GetIndex( ) ) + " ] )";
    }

    void MarkBase::dumpAsXml(xmlTextWriterPtr pWriter) const
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("MarkBase"));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(m_aName.toUtf8().getStr()));
        xmlTextWriterStartElement(pWriter, BAD_CAST("markPos"));
        GetMarkPos().dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
        if (IsExpanded())
        {
            xmlTextWriterStartElement(pWriter, BAD_CAST("otherMarkPos"));
            GetOtherMarkPos().dumpAsXml(pWriter);
            xmlTextWriterEndElement(pWriter);
        }
        xmlTextWriterEndElement(pWriter);
    }

    MarkBase::~MarkBase()
    { }

    OUString MarkBase::GenerateNewName(const OUString& rPrefix)
    {
        static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

        if (bHack)
        {
            static sal_Int64 nIdCounter = SAL_CONST_INT64(6000000000);
            return rPrefix + OUString::number(nIdCounter++);
        }
        else
        {
            static OUString sUniquePostfix;
            static sal_Int32 nCount = SAL_MAX_INT32;
            OUStringBuffer aResult(rPrefix);
            if(nCount == SAL_MAX_INT32)
            {
                unsigned int const n(comphelper::rng::uniform_uint_distribution(0,
                                    std::numeric_limits<unsigned int>::max()));
                sUniquePostfix = "_" + OUString::number(n);
                nCount = 0;
            }
            // putting the counter in front of the random parts will speed up string comparisons
            return aResult.append(nCount++).append(sUniquePostfix).makeStringAndClear();
        }
    }

    void MarkBase::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
    {
        NotifyClients(pOld, pNew);
        if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
        {   // invalidate cached uno object
            SetXBookmark(uno::Reference<text::XTextContent>(nullptr));
        }
    }

    // TODO: everything else uses MarkBase::GenerateNewName ?
    NavigatorReminder::NavigatorReminder(const SwPaM& rPaM)
        : MarkBase(rPaM, "__NavigatorReminder__")
    { }

    UnoMark::UnoMark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName("__UnoMark__"))
    { }

    DdeBookmark::DdeBookmark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName("__DdeLink__"))
    { }

    void DdeBookmark::SetRefObject(SwServerObject* pObj)
    {
        m_aRefObj = pObj;
    }

    void DdeBookmark::DeregisterFromDoc(SwDoc* const pDoc)
    {
        if(m_aRefObj.is())
            pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer(m_aRefObj.get());
    }

    DdeBookmark::~DdeBookmark()
    {
        if( m_aRefObj.is() )
        {
            if(m_aRefObj->HasDataLinks())
            {
                ::sfx2::SvLinkSource* p = m_aRefObj.get();
                p->SendDataChanged();
            }
            m_aRefObj->SetNoServer();
        }
    }

    Bookmark::Bookmark(const SwPaM& aPaM,
        const vcl::KeyCode& rCode,
        const OUString& rName)
        : DdeBookmark(aPaM)
        , ::sfx2::Metadatable()
        , m_aCode(rCode)
        , m_bHidden(false)
    {
        m_aName = rName;
    }

    void Bookmark::InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode const)
    {
        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo(
                    o3tl::make_unique<SwUndoInsBookmark>(*this));
        }
        io_pDoc->getIDocumentState().SetModified();
    }

    void Bookmark::DeregisterFromDoc(SwDoc* const io_pDoc)
    {
        DdeBookmark::DeregisterFromDoc(io_pDoc);

        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo(
                    o3tl::make_unique<SwUndoDeleteBookmark>(*this));
        }
        io_pDoc->getIDocumentState().SetModified();
    }

    ::sfx2::IXmlIdRegistry& Bookmark::GetRegistry()
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        assert(pDoc);
        return pDoc->GetXmlIdRegistry();
    }

    bool Bookmark::IsInClipboard() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        assert(pDoc);
        return pDoc->IsClipBoard();
    }

    bool Bookmark::IsInUndo() const
    {
        return false;
    }

    bool Bookmark::IsInContent() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        assert(pDoc);
        return !pDoc->IsInHeaderFooter( GetMarkPos().nNode );
    }

    uno::Reference< rdf::XMetadatable > Bookmark::MakeUnoObject()
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        assert(pDoc);
        const uno::Reference< rdf::XMetadatable> xMeta(
                SwXBookmark::CreateXBookmark(*pDoc, this), uno::UNO_QUERY);
        return xMeta;
    }

    Fieldmark::Fieldmark(const SwPaM& rPaM)
        : MarkBase(rPaM, MarkBase::GenerateNewName("__Fieldmark__"))
    {
        if(!IsExpanded())
            SetOtherMarkPos(GetMarkPos());
    }

    void Fieldmark::SetMarkStartPos( const SwPosition& rNewStartPos )
    {
        if ( GetMarkPos( ) <= GetOtherMarkPos( ) )
            return SetMarkPos( rNewStartPos );
        else
            return SetOtherMarkPos( rNewStartPos );
    }

    void Fieldmark::SetMarkEndPos( const SwPosition& rNewEndPos )
    {
        if ( GetMarkPos( ) <= GetOtherMarkPos( ) )
            return SetOtherMarkPos( rNewEndPos );
        else
            return SetMarkPos( rNewEndPos );
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
        SwPaM aPaM( GetMarkPos(), GetOtherMarkPos() );
        aPaM.InvalidatePaM();
    }

    void Fieldmark::dumpAsXml(xmlTextWriterPtr pWriter) const
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("Fieldmark"));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("fieldname"), BAD_CAST(m_aFieldname.toUtf8().getStr()));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("fieldHelptext"), BAD_CAST(m_aFieldHelptext.toUtf8().getStr()));
        MarkBase::dumpAsXml(pWriter);
        xmlTextWriterStartElement(pWriter, BAD_CAST("parameters"));
        for (auto& rParam : m_vParams)
        {
            xmlTextWriterStartElement(pWriter, BAD_CAST("parameter"));
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(rParam.first.toUtf8().getStr()));
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(comphelper::anyToString(rParam.second).toUtf8().getStr()));
            xmlTextWriterEndElement(pWriter);
        }
        xmlTextWriterEndElement(pWriter);
        xmlTextWriterEndElement(pWriter);
    }

    TextFieldmark::TextFieldmark(const SwPaM& rPaM, const OUString& rName)
        : Fieldmark(rPaM)
    {
        if ( !rName.isEmpty() )
            m_aName = rName;
    }

    void TextFieldmark::InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode const eMode)
    {
        if (eMode == sw::mark::InsertMode::New)
        {
            lcl_SetFieldMarks(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
        }
        else
        {
            lcl_AssertFieldMarksSet(this, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
        }
    }

    void TextFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    NonTextFieldmark::NonTextFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void NonTextFieldmark::InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode const eMode)
    {
        if (eMode == sw::mark::InsertMode::New)
        {
            lcl_SetFieldMarks(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);

            // For some reason the end mark is moved from 1 by the Insert:
            // we don't want this for checkboxes
            SwPosition aNewEndPos = GetMarkEnd();
            aNewEndPos.nContent--;
            SetMarkEndPos( aNewEndPos );
        }
        else
        {
            lcl_AssertFieldMarksSet(this, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);
        }
    }

    void NonTextFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc,
                CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);
    }


    CheckboxFieldmark::CheckboxFieldmark(const SwPaM& rPaM)
        : NonTextFieldmark(rPaM)
    { }

    void CheckboxFieldmark::SetChecked(bool checked)
    {
        if ( IsChecked() != checked )
        {
            (*GetParameters())[OUString(ODF_FORMCHECKBOX_RESULT)] <<= checked;
            // mark document as modified
            SwDoc *const pDoc( GetMarkPos().GetDoc() );
            if ( pDoc )
                pDoc->getIDocumentState().SetModified();
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

    FieldmarkWithDropDownButton::FieldmarkWithDropDownButton(const SwPaM& rPaM)
        : NonTextFieldmark(rPaM)
        , m_pButton(nullptr)
    {
    }

    FieldmarkWithDropDownButton::~FieldmarkWithDropDownButton()
    {
        m_pButton.disposeAndClear();
    }

    void FieldmarkWithDropDownButton::HideButton()
    {
        if(m_pButton)
            m_pButton->Show(false);
    }

    void FieldmarkWithDropDownButton::RemoveButton()
    {
        if(m_pButton)
            m_pButton.disposeAndClear();
    }

    DropDownFieldmark::DropDownFieldmark(const SwPaM& rPaM)
        : FieldmarkWithDropDownButton(rPaM)
    {
    }

    DropDownFieldmark::~DropDownFieldmark()
    {
        SendLOKMessage("hide");
    }

    void DropDownFieldmark::ShowButton(SwEditWin* pEditWin)
    {
        if(pEditWin)
        {
            if(!m_pButton)
                m_pButton = VclPtr<DropDownFormFieldButton>::Create(pEditWin, *this);
            m_pButton->CalcPosAndSize(m_aPortionPaintArea);
            m_pButton->Show();
            SendLOKMessage("show");
        }
    }

    void DropDownFieldmark::HideButton()
    {
        SendLOKMessage("hide");
        FieldmarkWithDropDownButton::HideButton();
    }

    void DropDownFieldmark::RemoveButton()
    {
        SendLOKMessage("hide");
        FieldmarkWithDropDownButton::RemoveButton();
    }

    void DropDownFieldmark::SetPortionPaintArea(const SwRect& rPortionPaintArea)
    {
        m_aPortionPaintArea = rPortionPaintArea;
        if(m_pButton)
        {
            m_pButton->Show();
            m_pButton->CalcPosAndSize(m_aPortionPaintArea);
            SendLOKMessage("show");
        }
    }

    void DropDownFieldmark::SendLOKMessage(const OString& sAction)
    {
        const SfxViewShell* pViewShell = SfxViewShell::Current();
        if (pViewShell && pViewShell->isLOKMobilePhone())
        {
              return;
        }

        if (comphelper::LibreOfficeKit::isActive())
        {
            if (!m_pButton)
              return;

            SwEditWin* pEditWin = dynamic_cast<SwEditWin*>(m_pButton->GetParent());
            if (!pEditWin)
                return;

            OStringBuffer sPayload;
            if (sAction == "show")
            {
                if(m_aPortionPaintArea.IsEmpty())
                    return;

                sPayload = OStringLiteral("{\"action\": \"show\","
                           " \"type\": \"drop-down\", \"textArea\": \"") +
                           m_aPortionPaintArea.SVRect().toString() + "\",";
                // Add field params to the message
                sPayload.append(" \"params\": { \"items\": [");

                // List items
                auto pParameters = this->GetParameters();
                auto pListEntriesIter = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
                css::uno::Sequence<OUString> vListEntries;
                if (pListEntriesIter != pParameters->end())
                {
                    pListEntriesIter->second >>= vListEntries;
                    for (const OUString& sItem : vListEntries)
                        sPayload.append("\"" + OUStringToOString(sItem, RTL_TEXTENCODING_UTF8) + "\", ");
                    sPayload.setLength(sPayload.getLength() - 2);
                }
                sPayload.append("], ");

                // Selected item
                OUString sResultKey = ODF_FORMDROPDOWN_RESULT;
                auto pSelectedItemIter = pParameters->find(sResultKey);
                sal_Int32 nSelection = -1;
                if (pSelectedItemIter != pParameters->end())
                {
                    pSelectedItemIter->second >>= nSelection;
                }
                sPayload.append("\"selected\": \"" + OString::number(nSelection) + "\", ");

                // Placeholder text
                sPayload.append("\"placeholderText\": \"" + OUStringToOString(SwResId(STR_DROP_DOWN_EMPTY_LIST), RTL_TEXTENCODING_UTF8) + "\"}}");
            }
            else
            {
                sPayload = "{\"action\": \"hide\", \"type\": \"drop-down\"}";
            }
            if (sPayload.toString() != m_sLastSentLOKMsg) {
                m_sLastSentLOKMsg = sPayload.toString();
                pEditWin->GetView().GetWrtShell().GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_FORM_FIELD_BUTTON, m_sLastSentLOKMsg.getStr());
            }
        }
    }

    DateFieldmark::DateFieldmark(const SwPaM& rPaM)
        : FieldmarkWithDropDownButton(rPaM)
        , m_pNumberFormatter(nullptr)
        , m_pDocumentContentOperationsManager(nullptr)
    {
    }

    DateFieldmark::~DateFieldmark()
    {
    }

    void DateFieldmark::InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode eMode)
    {
        m_pNumberFormatter = io_pDoc->GetNumberFormatter();
        m_pDocumentContentOperationsManager = &io_pDoc->GetDocumentContentOperationsManager();
        if (eMode == sw::mark::InsertMode::New)
        {
            lcl_SetFieldMarks(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
        }
        else
        {
            lcl_AssertFieldMarksSet(this, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
        }
    }

    void DateFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    void DateFieldmark::ShowButton(SwEditWin* pEditWin)
    {
        if(pEditWin)
        {
            if(!m_pButton)
                m_pButton = VclPtr<DateFormFieldButton>::Create(pEditWin, *this, m_pNumberFormatter);
            SwRect aPaintArea(m_aPaintAreaStart.TopLeft(), m_aPaintAreaEnd.BottomRight());
            m_pButton->CalcPosAndSize(aPaintArea);
            m_pButton->Show();
        }
    }

    void DateFieldmark::SetPortionPaintAreaStart(const SwRect& rPortionPaintArea)
    {
        if (rPortionPaintArea.IsEmpty())
            return;

        m_aPaintAreaStart = rPortionPaintArea;
        InvalidateCurrentDateParam();
    }

    void DateFieldmark::SetPortionPaintAreaEnd(const SwRect& rPortionPaintArea)
    {
        if (rPortionPaintArea.IsEmpty())
            return;

        if(m_aPaintAreaEnd == rPortionPaintArea &&
           m_pButton && m_pButton->IsVisible())
            return;

        m_aPaintAreaEnd = rPortionPaintArea;
        if(m_pButton)
        {
            m_pButton->Show();
            SwRect aPaintArea(m_aPaintAreaStart.TopLeft(), m_aPaintAreaEnd.BottomRight());
            m_pButton->CalcPosAndSize(aPaintArea);
            m_pButton->Invalidate();
        }
        InvalidateCurrentDateParam();
    }

    OUString DateFieldmark::GetContent() const
    {
        const SwTextNode* const pTextNode = GetMarkEnd().nNode.GetNode().GetTextNode();
        const sal_Int32 nStart(GetMarkStart().nContent.GetIndex());
        const sal_Int32 nEnd  (GetMarkEnd().nContent.GetIndex());

        OUString sContent;
        if(nStart + 1 < pTextNode->GetText().getLength() && nEnd <= pTextNode->GetText().getLength() &&
           nEnd > nStart + 2)
            sContent = pTextNode->GetText().copy(nStart + 1, nEnd - nStart - 2);
        return sContent;
    }

    void DateFieldmark::ReplaceContent(const OUString& sNewContent)
    {
        if(!m_pDocumentContentOperationsManager)
            return;

        const SwTextNode* const pTextNode = GetMarkEnd().nNode.GetNode().GetTextNode();
        const sal_Int32 nStart(GetMarkStart().nContent.GetIndex());
        const sal_Int32 nEnd  (GetMarkEnd().nContent.GetIndex());

        if(nStart + 1 < pTextNode->GetText().getLength() && nEnd <= pTextNode->GetText().getLength() &&
           nEnd > nStart + 2)
        {
            SwPaM aFieldPam(GetMarkStart().nNode, nStart + 1,
                            GetMarkStart().nNode, nEnd - 1);
            m_pDocumentContentOperationsManager->ReplaceRange(aFieldPam, sNewContent, false);
        }
        else
        {
            SwPaM aFieldStartPam(GetMarkStart().nNode, nStart + 1);
            m_pDocumentContentOperationsManager->InsertString(aFieldStartPam, sNewContent);
        }

    }

    std::pair<bool, double> DateFieldmark::GetCurrentDate() const
    {
        // Check current date param first
        std::pair<bool, double> aResult = ParseCurrentDateParam();
        if(aResult.first)
            return aResult;

        const sw::mark::IFieldmark::parameter_map_t* pParameters = GetParameters();
        bool bFoundValidDate = false;
        double dCurrentDate = 0;
        OUString sDateFormat;
        auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sDateFormat;
        }

        OUString sLang;
        pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sLang;
        }

        // Get current content of the field
        OUString sContent = GetContent();

        sal_uInt32 nFormat = m_pNumberFormatter->GetEntryKey(sDateFormat, LanguageTag(sLang).getLanguageType());
        if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            m_pNumberFormatter->PutEntry(sDateFormat,
                                         nCheckPos,
                                         nType,
                                         nFormat,
                                         LanguageTag(sLang).getLanguageType());
        }

        if (nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            bFoundValidDate = m_pNumberFormatter->IsNumberFormat(sContent, nFormat, dCurrentDate);
        }
        return std::pair<bool, double>(bFoundValidDate, dCurrentDate);
    }

    void DateFieldmark::SetCurrentDate(double fDate)
    {
        // Replace current content with the selected date
        ReplaceContent(GetDateInCurrentDateFormat(fDate));

        // Also save the current date in a standard format
        sw::mark::IFieldmark::parameter_map_t* pParameters = GetParameters();
        (*pParameters)[ODF_FORMDATE_CURRENTDATE] <<= GetDateInStandardDateFormat(fDate);
    }

    OUString DateFieldmark::GetDateInStandardDateFormat(double fDate) const
    {
        OUString sCurrentDate;
        sal_uInt32 nFormat = m_pNumberFormatter->GetEntryKey(ODF_FORMDATE_CURRENTDATE_FORMAT, ODF_FORMDATE_CURRENTDATE_LANGUAGE);
        if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            OUString sFormat = ODF_FORMDATE_CURRENTDATE_FORMAT;
            m_pNumberFormatter->PutEntry(sFormat,
                                         nCheckPos,
                                         nType,
                                         nFormat,
                                         ODF_FORMDATE_CURRENTDATE_LANGUAGE);
        }

        if (nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            Color* pCol = nullptr;
            m_pNumberFormatter->GetOutputString(fDate, nFormat, sCurrentDate, &pCol, false);
        }
        return sCurrentDate;
    }

    std::pair<bool, double> DateFieldmark::ParseCurrentDateParam() const
    {
        bool bFoundValidDate = false;
        double dCurrentDate = 0;

        const sw::mark::IFieldmark::parameter_map_t* pParameters = GetParameters();
        auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
        OUString sCurrentDate;
        if (pResult != pParameters->end())
        {
            pResult->second >>= sCurrentDate;
        }
        if(!sCurrentDate.isEmpty())
        {
            sal_uInt32 nFormat = m_pNumberFormatter->GetEntryKey(ODF_FORMDATE_CURRENTDATE_FORMAT, ODF_FORMDATE_CURRENTDATE_LANGUAGE);
            if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
            {
                sal_Int32 nCheckPos = 0;
                SvNumFormatType nType;
                OUString sFormat = ODF_FORMDATE_CURRENTDATE_FORMAT;
                m_pNumberFormatter->PutEntry(sFormat,
                                             nCheckPos,
                                             nType,
                                             nFormat,
                                             ODF_FORMDATE_CURRENTDATE_LANGUAGE);
            }

            if(nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
            {
                bFoundValidDate = m_pNumberFormatter->IsNumberFormat(sCurrentDate, nFormat, dCurrentDate);
            }
        }
        return std::pair<bool, double>(bFoundValidDate, dCurrentDate);
    }


    OUString DateFieldmark::GetDateInCurrentDateFormat(double fDate) const
    {
        // Get current date format and language
        OUString sDateFormat;
        const sw::mark::IFieldmark::parameter_map_t* pParameters = GetParameters();
        auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sDateFormat;
        }

        OUString sLang;
        pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sLang;
        }

        // Fill the content with the specified format
        OUString sCurrentContent;
        sal_uInt32 nFormat = m_pNumberFormatter->GetEntryKey(sDateFormat, LanguageTag(sLang).getLanguageType());
        if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            OUString sFormat = sDateFormat;
            m_pNumberFormatter->PutEntry(sFormat,
                                         nCheckPos,
                                         nType,
                                         nFormat,
                                         LanguageTag(sLang).getLanguageType());
        }

        if (nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            Color* pCol = nullptr;
            m_pNumberFormatter->GetOutputString(fDate, nFormat, sCurrentContent, &pCol, false);
        }
        return sCurrentContent;
    }

    void DateFieldmark::InvalidateCurrentDateParam()
    {
        std::pair<bool, double> aResult = ParseCurrentDateParam();
        if(!aResult.first)
            return;

        // Current date became invalid
        if(GetDateInCurrentDateFormat(aResult.second) != GetContent())
        {
            sw::mark::IFieldmark::parameter_map_t* pParameters = GetParameters();
            (*pParameters)[ODF_FORMDATE_CURRENTDATE] <<= OUString();
        }
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
