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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
    };

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
        const sal_Unicode ch_start=pStartTxtNode->GetTxt().GetChar(rStart.nContent.GetIndex());
        xub_StrLen nEndPos = rEnd == rStart ? rEnd.nContent.GetIndex() : rEnd.nContent.GetIndex() - 1;
        const sal_Unicode ch_end=pEndTxtNode->GetTxt().GetChar( nEndPos );
        SwPaM aStartPaM(rStart);
        SwPaM aEndPaM(rEnd);
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, NULL);
        if( ( ch_start != aStartMark ) && ( aEndMark != CH_TXT_ATR_FORMELEMENT ) )
        {
            io_pDoc->InsertString(aStartPaM, aStartMark);
            rStart.nContent--;
        }
        if ( aEndMark && ( ch_end != aEndMark ) )
        {
            io_pDoc->InsertString(aEndPaM, aEndMark);
            rEnd.nContent++;
        }
        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, NULL);
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

    rtl::OUString MarkBase::ToString( ) const
    {
        rtl::OUStringBuffer buf;
        buf.appendAscii( "Mark: ( Name, [ Node1, Index1 ] ): ( " );
        buf.append( m_aName ).appendAscii( ", [ " );
        buf.append( sal_Int32( GetMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetMarkPos().nContent.GetIndex( ) ) ).appendAscii( " ] )" );

        return buf.makeStringAndClear( );
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


    void MarkBase::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
    {
        NotifyClients(pOld, pNew);
        if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
        {   // invalidate cached uno object
            SetXBookmark(uno::Reference<text::XTextContent>(0));
        }
    }


    NavigatorReminder::NavigatorReminder(const SwPaM& rPaM)
        : MarkBase(rPaM, our_sNamePrefix)
    { }

    const ::rtl::OUString NavigatorReminder::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__NavigatorReminder__"));

    UnoMark::UnoMark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    { }

    const ::rtl::OUString UnoMark::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__UnoMark__"));

    DdeBookmark::DdeBookmark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
        , m_aRefObj(NULL)
    { }

    void DdeBookmark::SetRefObject(SwServerObject* pObj)
    {
        m_aRefObj = pObj;
    }

    const ::rtl::OUString DdeBookmark::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__DdeLink__"));

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

    // ::sfx2::Metadatable
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
        // create new SwXBookmark
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::MakeUnoObject: no doc?");
        const uno::Reference< rdf::XMetadatable> xMeta(
                SwXBookmark::CreateXBookmark(*pDoc, *this), uno::UNO_QUERY);
        return xMeta;
    }


    Fieldmark::Fieldmark(const SwPaM& rPaM)
        : MarkBase(rPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    {
        if(!IsExpanded())
            SetOtherMarkPos(GetMarkPos());
    }

    rtl::OUString Fieldmark::ToString( ) const
    {
        rtl::OUStringBuffer buf;
        buf.appendAscii( "Fieldmark: ( Name, Type, [ Nd1, Id1 ], [ Nd2, Id2 ] ): ( " );
        buf.append( m_aName ).appendAscii( ", " );
        buf.append( m_aFieldname ).appendAscii( ", [ " );
        buf.append( sal_Int32( GetMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ], [" );
        buf.append( sal_Int32( GetOtherMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetOtherMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ] ) " );

        return buf.makeStringAndClear( );
    }

    void Fieldmark::Invalidate( )
    {
        // @TODO: Does exist a better solution to trigger a format of the
        //        fieldmark portion? If yes, please use it.
        SwPaM aPaM( this->GetMarkPos(), this->GetOtherMarkPos() );
        aPaM.InvalidatePaM();
    }

    const ::rtl::OUString Fieldmark::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__Fieldmark__"));

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
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FORMELEMENT);

        // For some reason the end mark is moved from 1 by the Insert: we don't
        // want this for checkboxes
        this->GetMarkEnd( ).nContent--;
    }
    void CheckboxFieldmark::SetChecked(bool checked)
    {
        if ( IsChecked() != checked )
        {
            (*GetParameters())[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMCHECKBOX_RESULT))] = makeAny(checked);
            // mark document as modified
            SwDoc *const pDoc( GetMarkPos().GetDoc() );
            if ( pDoc )
                pDoc->SetModified();
        }
    }

    bool CheckboxFieldmark::IsChecked() const
    {
        bool bResult = false;
        parameter_map_t::const_iterator pResult = GetParameters()->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMCHECKBOX_RESULT)));
        if(pResult != GetParameters()->end())
            pResult->second >>= bResult;
        return bResult;
    }

    rtl::OUString CheckboxFieldmark::toString( ) const
    {
        rtl::OUStringBuffer buf;
        buf.appendAscii( "CheckboxFieldmark: ( Name, Type, [ Nd1, Id1 ], [ Nd2, Id2 ] ): ( " );
        buf.append( m_aName ).appendAscii( ", " );
        buf.append( GetFieldname() ).appendAscii( ", [ " );
        buf.append( sal_Int32( GetMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ], [" );
        buf.append( sal_Int32( GetOtherMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetOtherMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ] ) " );

        return buf.makeStringAndClear( );
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
