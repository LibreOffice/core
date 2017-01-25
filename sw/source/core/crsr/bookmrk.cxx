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

    void lcl_AssureFieldMarksSet(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, nullptr);

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

        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, nullptr);
    };

    void lcl_RemoveFieldMarks(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, nullptr);

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

        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, nullptr);
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
        , m_aRefObj(nullptr)
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
        io_pDoc->getIDocumentState().SetModified();
    }

    void Bookmark::DeregisterFromDoc(SwDoc* const io_pDoc)
    {
        DdeBookmark::DeregisterFromDoc(io_pDoc);

        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo(
                    new SwUndoDeleteBookmark(*this));
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
        return !pDoc->IsInHeaderFooter( SwNodeIndex(GetMarkPos().nNode) );
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
        SwPaM aPaM( this->GetMarkPos(), this->GetOtherMarkPos() );
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
        SwPosition aNewEndPos = this->GetMarkEnd();
        aNewEndPos.nContent--;
        SetMarkEndPos( aNewEndPos );
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
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
