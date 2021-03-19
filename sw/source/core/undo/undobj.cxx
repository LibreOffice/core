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

#include <libxml/xmlwriter.h>

#include <IShellCursorSupplier.hxx>
#include <txtftn.hxx>
#include <fmtanchr.hxx>
#include <ftnidx.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <ndnotxt.hxx>
#include <IMark.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <crossrefbookmark.hxx>
#include <strings.hrc>
#include <docsh.hxx>
#include <view.hxx>
#include <frameformats.hxx>
#include <sal/log.hxx>

// This class saves the Pam as integers and can recompose those into a PaM
SwUndRng::SwUndRng()
    : m_nSttNode( 0 ), m_nEndNode( 0 ), m_nSttContent( 0 ), m_nEndContent( 0 )
{
}

SwUndRng::SwUndRng( const SwPaM& rPam )
{
    SetValues( rPam );
}

void SwUndRng::SetValues( const SwPaM& rPam )
{
    const SwPosition *pStt = rPam.Start();
    if( rPam.HasMark() )
    {
        const SwPosition *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();
        m_nEndNode = pEnd->nNode.GetIndex();
        m_nEndContent = pEnd->nContent.GetIndex();
    }
    else
    {
        // no selection !!
        m_nEndNode = 0;
        m_nEndContent = COMPLETE_STRING;
    }

    m_nSttNode = pStt->nNode.GetIndex();
    m_nSttContent = pStt->nContent.GetIndex();
}

void SwUndRng::SetPaM( SwPaM & rPam, bool bCorrToContent ) const
{
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = m_nSttNode;
    SwNode& rNd = rPam.GetNode();
    if( rNd.IsContentNode() )
        rPam.GetPoint()->nContent.Assign( rNd.GetContentNode(), m_nSttContent );
    else if( bCorrToContent )
        rPam.Move( fnMoveForward, GoInContent );
    else
        rPam.GetPoint()->nContent.Assign( nullptr, 0 );

    if( !m_nEndNode && COMPLETE_STRING == m_nEndContent )       // no selection
        return ;

    rPam.SetMark();
    if( m_nSttNode == m_nEndNode && m_nSttContent == m_nEndContent )
        return;                             // nothing left to do

    rPam.GetPoint()->nNode = m_nEndNode;
    if( rPam.GetNode().IsContentNode() )
        rPam.GetPoint()->nContent.Assign( rPam.GetNode().GetContentNode(), m_nEndContent );
    else if( bCorrToContent )
        rPam.Move( fnMoveBackward, GoInContent );
    else
        rPam.GetPoint()->nContent.Assign( nullptr, 0 );
}

SwPaM & SwUndRng::AddUndoRedoPaM(
        ::sw::UndoRedoContext & rContext, bool const bCorrToContent) const
{
    SwPaM & rPaM( rContext.GetCursorSupplier().CreateNewShellCursor() );
    SetPaM( rPaM, bCorrToContent );
    return rPaM;
}

void SwUndo::RemoveIdxFromSection( SwDoc& rDoc, sal_uLong nSttIdx,
                                    const sal_uLong* pEndIdx )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttIdx );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), pEndIdx ? *pEndIdx
                                    : aIdx.GetNode().EndOfSectionIndex() );
    SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
    SwDoc::CorrAbs( aIdx, aEndIdx, aPos, true );
}

void SwUndo::RemoveIdxFromRange( SwPaM& rPam, bool bMoveNext )
{
    const SwPosition* pEnd = rPam.End();
    if( bMoveNext )
    {
        if( pEnd != rPam.GetPoint() )
            rPam.Exchange();

        SwNodeIndex aStt( rPam.GetMark()->nNode );
        SwNodeIndex aEnd( rPam.GetPoint()->nNode );

        if( !rPam.Move( fnMoveForward ) )
        {
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward ) )
            {
                rPam.GetPoint()->nNode = rPam.GetDoc().GetNodes().GetEndOfPostIts();
                rPam.GetPoint()->nContent.Assign( nullptr, 0 );
            }
        }

        SwDoc::CorrAbs( aStt, aEnd, *rPam.GetPoint(), true );
    }
    else
        SwDoc::CorrAbs( rPam, *pEnd, true );
}

void SwUndo::RemoveIdxRel( sal_uLong nIdx, const SwPosition& rPos )
{
    // Move only the Cursor. Bookmarks/TOXMarks/etc. are done by the corresponding
    // JoinNext/JoinPrev
    SwNodeIndex aIdx( rPos.nNode.GetNode().GetNodes(), nIdx );
    ::PaMCorrRel( aIdx, rPos );
}

SwUndo::SwUndo(SwUndoId const nId, const SwDoc* pDoc)
    : m_nId(nId), m_nOrigRedlineFlags(RedlineFlags::NONE)
    , m_nViewShellId(CreateViewShellId(pDoc))
    , m_isRepeatIgnored(false)
    , m_bCacheComment(true)
{
}

ViewShellId SwUndo::CreateViewShellId(const SwDoc* pDoc)
{
    ViewShellId nRet(-1);

    if (const SwDocShell* pDocShell = pDoc->GetDocShell())
    {
        if (const SwView* pView = pDocShell->GetView())
            nRet = pView->GetViewShellId();
    }

    return nRet;
}

bool SwUndo::IsDelBox() const
{
    return GetId() == SwUndoId::COL_DELETE || GetId() == SwUndoId::ROW_DELETE ||
        GetId() == SwUndoId::TABLE_DELBOX;
}

SwUndo::~SwUndo()
{
}

namespace {

class UndoRedoRedlineGuard
{
public:
    UndoRedoRedlineGuard(::sw::UndoRedoContext const & rContext, SwUndo const & rUndo)
        : m_rRedlineAccess(rContext.GetDoc().getIDocumentRedlineAccess())
        , m_eMode(m_rRedlineAccess.GetRedlineFlags())
    {
        RedlineFlags const eTmpMode = rUndo.GetRedlineFlags();
        if ((RedlineFlags::ShowMask & eTmpMode) != (RedlineFlags::ShowMask & m_eMode))
        {
            m_rRedlineAccess.SetRedlineFlags( eTmpMode );
        }
        m_rRedlineAccess.SetRedlineFlags_intern( eTmpMode | RedlineFlags::Ignore );
    }
    ~UndoRedoRedlineGuard()
    {
        m_rRedlineAccess.SetRedlineFlags(m_eMode);
    }
private:
    IDocumentRedlineAccess & m_rRedlineAccess;
    RedlineFlags const m_eMode;
};

}

void SwUndo::Undo()
{
    assert(false); // SwUndo::Undo(): ERROR: must call UndoWithContext instead
}

void SwUndo::Redo()
{
    assert(false); // SwUndo::Redo(): ERROR: must call RedoWithContext instead
}

void SwUndo::UndoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    assert(pContext);
    const UndoRedoRedlineGuard aUndoRedoRedlineGuard(*pContext, *this);
    UndoImpl(*pContext);
}

void SwUndo::RedoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    assert(pContext);
    const UndoRedoRedlineGuard aUndoRedoRedlineGuard(*pContext, *this);
    RedoImpl(*pContext);
}

void SwUndo::Repeat(SfxRepeatTarget & rContext)
{
    if (m_isRepeatIgnored)
    {
        return; // ignore Repeat for multi-selections
    }
    ::sw::RepeatContext *const pRepeatContext(
            dynamic_cast< ::sw::RepeatContext * >(& rContext));
    assert(pRepeatContext);
    RepeatImpl(*pRepeatContext);
}

bool SwUndo::CanRepeat(SfxRepeatTarget & rContext) const
{
    assert(dynamic_cast< ::sw::RepeatContext * >(& rContext));
    (void)rContext;
    // a MultiSelection action that doesn't do anything must still return true
    return (SwUndoId::REPEAT_START <= GetId()) && (GetId() < SwUndoId::REPEAT_END);
}

void SwUndo::RepeatImpl( ::sw::RepeatContext & )
{
}

OUString GetUndoComment(SwUndoId eId)
{
    const char *pId = nullptr;
    switch (eId)
    {
        case SwUndoId::EMPTY:
            pId = STR_CANT_UNDO;
            break;
        case SwUndoId::START:
        case SwUndoId::END:
            break;
        case SwUndoId::DELETE:
            pId = STR_DELETE_UNDO;
            break;
        case SwUndoId::INSERT:
            pId = STR_INSERT_UNDO;
            break;
        case SwUndoId::OVERWRITE:
            pId = STR_OVR_UNDO;
            break;
        case SwUndoId::SPLITNODE:
            pId = STR_SPLITNODE_UNDO;
            break;
        case SwUndoId::INSATTR:
            pId = STR_INSATTR_UNDO;
            break;
        case SwUndoId::SETFMTCOLL:
            pId = STR_SETFMTCOLL_UNDO;
            break;
        case SwUndoId::RESETATTR:
            pId = STR_RESET_ATTR_UNDO;
            break;
        case SwUndoId::INSFMTATTR:
            pId = STR_INSFMT_ATTR_UNDO;
            break;
        case SwUndoId::INSDOKUMENT:
            pId = STR_INSERT_DOC_UNDO;
            break;
        case SwUndoId::COPY:
            pId = STR_COPY_UNDO;
            break;
        case SwUndoId::INSTABLE:
            pId = STR_INSTABLE_UNDO;
            break;
        case SwUndoId::TABLETOTEXT:
            pId = STR_TABLETOTEXT_UNDO;
            break;
        case SwUndoId::TEXTTOTABLE:
            pId = STR_TEXTTOTABLE_UNDO;
            break;
        case SwUndoId::SORT_TXT:
            pId = STR_SORT_TXT;
            break;
        case SwUndoId::INSLAYFMT:
            pId = STR_INSERTFLY;
            break;
        case SwUndoId::TABLEHEADLINE:
            pId = STR_TABLEHEADLINE;
            break;
        case SwUndoId::INSSECTION:
            pId = STR_INSERTSECTION;
            break;
        case SwUndoId::OUTLINE_LR:
            pId = STR_OUTLINE_LR;
            break;
        case SwUndoId::OUTLINE_UD:
            pId = STR_OUTLINE_UD;
            break;
        case SwUndoId::INSNUM:
            pId = STR_INSNUM;
            break;
        case SwUndoId::NUMUP:
            pId = STR_NUMUP;
            break;
        case SwUndoId::MOVENUM:
            pId = STR_MOVENUM;
            break;
        case SwUndoId::INSDRAWFMT:
            pId = STR_INSERTDRAW;
            break;
        case SwUndoId::NUMORNONUM:
            pId = STR_NUMORNONUM;
            break;
        case SwUndoId::INC_LEFTMARGIN:
            pId = STR_INC_LEFTMARGIN;
            break;
        case SwUndoId::DEC_LEFTMARGIN:
            pId = STR_DEC_LEFTMARGIN;
            break;
        case SwUndoId::INSERTLABEL:
            pId = STR_INSERTLABEL;
            break;
        case SwUndoId::SETNUMRULESTART:
            pId = STR_SETNUMRULESTART;
            break;
        case SwUndoId::CHGFTN:
            pId = STR_CHANGEFTN;
            break;
        case SwUndoId::REDLINE:
            SAL_INFO("sw.core", "Should NEVER be used/translated");
            return "$1";
        case SwUndoId::ACCEPT_REDLINE:
            pId = STR_ACCEPT_REDLINE;
            break;
        case SwUndoId::REJECT_REDLINE:
            pId = STR_REJECT_REDLINE;
            break;
        case SwUndoId::SPLIT_TABLE:
            pId = STR_SPLIT_TABLE;
            break;
        case SwUndoId::DONTEXPAND:
            pId = STR_DONTEXPAND;
            break;
        case SwUndoId::AUTOCORRECT:
            pId = STR_AUTOCORRECT;
            break;
        case SwUndoId::MERGE_TABLE:
            pId = STR_MERGE_TABLE;
            break;
        case SwUndoId::TRANSLITERATE:
            pId = STR_TRANSLITERATE;
            break;
        case SwUndoId::PASTE_CLIPBOARD:
            pId = STR_PASTE_CLIPBOARD_UNDO;
            break;
        case SwUndoId::TYPING:
            pId = STR_TYPING_UNDO;
            break;
        case SwUndoId::MOVE:
            pId = STR_MOVE_UNDO;
            break;
        case SwUndoId::INSGLOSSARY:
            pId = STR_INSERT_GLOSSARY;
            break;
        case SwUndoId::DELBOOKMARK:
            pId = STR_DELBOOKMARK;
            break;
        case SwUndoId::INSBOOKMARK:
            pId = STR_INSBOOKMARK;
            break;
        case SwUndoId::SORT_TBL:
            pId = STR_SORT_TBL;
            break;
        case SwUndoId::DELLAYFMT:
            pId = STR_DELETEFLY;
            break;
        case SwUndoId::AUTOFORMAT:
            pId = STR_AUTOFORMAT;
            break;
        case SwUndoId::REPLACE:
            pId = STR_REPLACE;
            break;
        case SwUndoId::DELSECTION:
            pId = STR_DELETESECTION;
            break;
        case SwUndoId::CHGSECTION:
            pId = STR_CHANGESECTION;
            break;
        case SwUndoId::SETDEFTATTR:
            pId = STR_CHANGEDEFATTR;
            break;
        case SwUndoId::DELNUM:
            pId = STR_DELNUM;
            break;
        case SwUndoId::DRAWUNDO:
            pId = STR_DRAWUNDO;
            break;
        case SwUndoId::DRAWGROUP:
            pId = STR_DRAWGROUP;
            break;
        case SwUndoId::DRAWUNGROUP:
            pId = STR_DRAWUNGROUP;
            break;
        case SwUndoId::DRAWDELETE:
            pId = STR_DRAWDELETE;
            break;
        case SwUndoId::REREAD:
            pId = STR_REREAD;
            break;
        case SwUndoId::DELGRF:
            pId = STR_DELGRF;
            break;
        case SwUndoId::TABLE_ATTR:
            pId = STR_TABLE_ATTR;
            break;
        case SwUndoId::TABLE_AUTOFMT:
            pId = STR_UNDO_TABLE_AUTOFMT;
            break;
        case SwUndoId::TABLE_INSCOL:
            pId = STR_UNDO_TABLE_INSCOL;
            break;
        case SwUndoId::TABLE_INSROW:
            pId = STR_UNDO_TABLE_INSROW;
            break;
        case SwUndoId::TABLE_DELBOX:
            pId = STR_UNDO_TABLE_DELBOX;
            break;
        case SwUndoId::TABLE_SPLIT:
            pId = STR_UNDO_TABLE_SPLIT;
            break;
        case SwUndoId::TABLE_MERGE:
            pId = STR_UNDO_TABLE_MERGE;
            break;
        case SwUndoId::TBLNUMFMT:
            pId = STR_TABLE_NUMFORMAT;
            break;
        case SwUndoId::INSTOX:
            pId = STR_INSERT_TOX;
            break;
        case SwUndoId::CLEARTOXRANGE:
            pId = STR_CLEAR_TOX_RANGE;
            break;
        case SwUndoId::TBLCPYTBL:
            pId = STR_TABLE_TBLCPYTBL;
            break;
        case SwUndoId::CPYTBL:
            pId = STR_TABLE_CPYTBL;
            break;
        case SwUndoId::INS_FROM_SHADOWCRSR:
            pId = STR_INS_FROM_SHADOWCRSR;
            break;
        case SwUndoId::CHAINE:
            pId = STR_UNDO_CHAIN;
            break;
        case SwUndoId::UNCHAIN:
            pId = STR_UNDO_UNCHAIN;
            break;
        case SwUndoId::FTNINFO:
            pId = STR_UNDO_FTNINFO;
            break;
        case SwUndoId::COMPAREDOC:
            pId = STR_UNDO_COMPAREDOC;
            break;
        case SwUndoId::SETFLYFRMFMT:
            pId = STR_UNDO_SETFLYFRMFMT;
            break;
        case SwUndoId::SETRUBYATTR:
            pId = STR_UNDO_SETRUBYATTR;
            break;
        case SwUndoId::TOXCHANGE:
            pId = STR_TOXCHANGE;
            break;
        case SwUndoId::CREATE_PAGEDESC:
            pId = STR_UNDO_PAGEDESC_CREATE;
            break;
        case SwUndoId::CHANGE_PAGEDESC:
            pId = STR_UNDO_PAGEDESC;
            break;
        case SwUndoId::DELETE_PAGEDESC:
            pId = STR_UNDO_PAGEDESC_DELETE;
            break;
        case SwUndoId::HEADER_FOOTER:
            pId = STR_UNDO_HEADER_FOOTER;
            break;
        case SwUndoId::FIELD:
            pId = STR_UNDO_FIELD;
            break;
        case SwUndoId::TXTFMTCOL_CREATE:
            pId = STR_UNDO_TXTFMTCOL_CREATE;
            break;
        case SwUndoId::TXTFMTCOL_DELETE:
            pId = STR_UNDO_TXTFMTCOL_DELETE;
            break;
        case SwUndoId::TXTFMTCOL_RENAME:
            pId = STR_UNDO_TXTFMTCOL_RENAME;
            break;
        case SwUndoId::CHARFMT_CREATE:
            pId = STR_UNDO_CHARFMT_CREATE;
            break;
        case SwUndoId::CHARFMT_DELETE:
            pId = STR_UNDO_CHARFMT_DELETE;
            break;
        case SwUndoId::CHARFMT_RENAME:
            pId = STR_UNDO_CHARFMT_RENAME;
            break;
        case SwUndoId::FRMFMT_CREATE:
            pId = STR_UNDO_FRMFMT_CREATE;
            break;
        case SwUndoId::FRMFMT_DELETE:
            pId = STR_UNDO_FRMFMT_DELETE;
            break;
        case SwUndoId::FRMFMT_RENAME:
            pId = STR_UNDO_FRMFMT_RENAME;
            break;
        case SwUndoId::NUMRULE_CREATE:
            pId = STR_UNDO_NUMRULE_CREATE;
            break;
        case SwUndoId::NUMRULE_DELETE:
            pId = STR_UNDO_NUMRULE_DELETE;
            break;
        case SwUndoId::NUMRULE_RENAME:
            pId = STR_UNDO_NUMRULE_RENAME;
            break;
        case SwUndoId::BOOKMARK_RENAME:
            pId = STR_UNDO_BOOKMARK_RENAME;
            break;
        case SwUndoId::INDEX_ENTRY_INSERT:
            pId = STR_UNDO_INDEX_ENTRY_INSERT;
            break;
        case SwUndoId::INDEX_ENTRY_DELETE:
            pId = STR_UNDO_INDEX_ENTRY_DELETE;
            break;
        case SwUndoId::COL_DELETE:
            pId = STR_UNDO_COL_DELETE;
            break;
        case SwUndoId::ROW_DELETE:
            pId = STR_UNDO_ROW_DELETE;
            break;
        case SwUndoId::RENAME_PAGEDESC:
            pId = STR_UNDO_PAGEDESC_RENAME;
            break;
        case SwUndoId::NUMDOWN:
            pId = STR_NUMDOWN;
            break;
        case SwUndoId::FLYFRMFMT_TITLE:
            pId = STR_UNDO_FLYFRMFMT_TITLE;
            break;
        case SwUndoId::FLYFRMFMT_DESCRIPTION:
            pId = STR_UNDO_FLYFRMFMT_DESCRIPTION;
            break;
        case SwUndoId::TBLSTYLE_CREATE:
            pId = STR_UNDO_TBLSTYLE_CREATE;
            break;
        case SwUndoId::TBLSTYLE_DELETE:
            pId = STR_UNDO_TBLSTYLE_DELETE;
            break;
        case SwUndoId::TBLSTYLE_UPDATE:
            pId = STR_UNDO_TBLSTYLE_UPDATE;
            break;
        case SwUndoId::UI_REPLACE:
            pId = STR_REPLACE_UNDO;
            break;
        case SwUndoId::UI_INSERT_PAGE_BREAK:
            pId = STR_INSERT_PAGE_BREAK_UNDO;
            break;
        case SwUndoId::UI_INSERT_COLUMN_BREAK:
            pId = STR_INSERT_COLUMN_BREAK_UNDO;
            break;
        case SwUndoId::UI_INSERT_ENVELOPE:
            pId = STR_INSERT_ENV_UNDO;
            break;
        case SwUndoId::UI_DRAG_AND_COPY:
            pId = STR_DRAG_AND_COPY;
            break;
        case SwUndoId::UI_DRAG_AND_MOVE:
            pId = STR_DRAG_AND_MOVE;
            break;
        case SwUndoId::UI_INSERT_CHART:
            pId = STR_INSERT_CHART;
            break;
        case SwUndoId::UI_INSERT_FOOTNOTE:
            pId = STR_INSERT_FOOTNOTE;
            break;
        case SwUndoId::UI_INSERT_URLBTN:
            pId = STR_INSERT_URLBTN;
            break;
        case SwUndoId::UI_INSERT_URLTXT:
            pId = STR_INSERT_URLTXT;
            break;
        case SwUndoId::UI_DELETE_INVISIBLECNTNT:
            pId = STR_DELETE_INVISIBLECNTNT;
            break;
        case SwUndoId::UI_REPLACE_STYLE:
            pId = STR_REPLACE_STYLE;
            break;
        case SwUndoId::UI_DELETE_PAGE_BREAK:
            pId = STR_DELETE_PAGE_BREAK;
            break;
        case SwUndoId::UI_TEXT_CORRECTION:
            pId = STR_TEXT_CORRECTION;
            break;
        case SwUndoId::UI_TABLE_DELETE:
            pId = STR_UNDO_TABLE_DELETE;
            break;
        case SwUndoId::CONFLICT:
            break;
        case SwUndoId::PARA_SIGN_ADD:
            pId = STR_PARAGRAPH_SIGN_UNDO;
            break;
        case SwUndoId::INSERT_FORM_FIELD:
            pId = STR_UNDO_INSERT_FORM_FIELD;
            break;
    }

    assert(pId);
    return SwResId(pId);
}

OUString SwUndo::GetComment() const
{
    OUString aResult;

    if (m_bCacheComment)
    {
        if (! maComment)
        {
            maComment = GetUndoComment(GetId());

            SwRewriter aRewriter = GetRewriter();

            maComment = aRewriter.Apply(*maComment);
        }

        aResult = *maComment;
    }
    else
    {
        aResult = GetUndoComment(GetId());

        SwRewriter aRewriter = GetRewriter();

        aResult = aRewriter.Apply(aResult);
    }

    return aResult;
}

ViewShellId SwUndo::GetViewShellId() const
{
    return m_nViewShellId;
}

SwRewriter SwUndo::GetRewriter() const
{
    SwRewriter aResult;

    return aResult;
}

SwUndoSaveContent::SwUndoSaveContent()
{}

SwUndoSaveContent::~SwUndoSaveContent() COVERITY_NOEXCEPT_FALSE
{
}

void SwUndoSaveContent::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoSaveContent"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    if (m_pHistory)
    {
        m_pHistory->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

// This is needed when deleting content. For REDO all contents will be moved
// into the UndoNodesArray. These methods always create a new node to insert
// content. As a result, the attributes will not be expanded.
// - MoveTo   moves from NodesArray into UndoNodesArray
// - MoveFrom moves from UndoNodesArray into NodesArray

// If pEndNdIdx is given, Undo/Redo calls -Ins/DelFly. In that case the whole
// section should be moved.
void SwUndoSaveContent::MoveToUndoNds( SwPaM& rPaM, SwNodeIndex* pNodeIdx,
                    sal_uLong* pEndNdIdx )
{
    SwDoc& rDoc = rPaM.GetDoc();
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    SwNoTextNode* pCpyNd = rPaM.GetNode().GetNoTextNode();

    // here comes the actual delete (move)
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
    SwPosition aPos( pEndNdIdx ? rNds.GetEndOfPostIts()
                               : rNds.GetEndOfExtras() );

    const SwPosition* pStt = rPaM.Start(), *pEnd = rPaM.End();

    sal_uLong nTmpMvNode = aPos.nNode.GetIndex();

    if( pCpyNd || pEndNdIdx )
    {
        SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
        rDoc.GetNodes().MoveNodes( aRg, rNds, aPos.nNode, true );
        aPos.nContent = 0;
        --aPos.nNode;
    }
    else
    {
        rDoc.GetNodes().MoveRange( rPaM, aPos, rNds );
    }
    if( pEndNdIdx )
        *pEndNdIdx = aPos.nNode.GetIndex();

    // old position
    aPos.nNode = nTmpMvNode;
    if( pNodeIdx )
        *pNodeIdx = aPos.nNode;
}

void SwUndoSaveContent::MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                            SwPosition& rInsPos,
            const sal_uLong* pEndNdIdx, bool const bForceCreateFrames)
{
    // here comes the recovery
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
    if( nNodeIdx == rNds.GetEndOfPostIts().GetIndex() )
        return;     // nothing saved

    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    SwPaM aPaM( rInsPos );
    if( pEndNdIdx )         // than get the section from it
        aPaM.GetPoint()->nNode.Assign( rNds, *pEndNdIdx );
    else
    {
        aPaM.GetPoint()->nNode = rNds.GetEndOfExtras();
        GoInContent( aPaM, fnMoveBackward );
    }

    SwTextNode* pTextNd = aPaM.GetNode().GetTextNode();
    if (!pEndNdIdx && pTextNd)
    {
        aPaM.SetMark();
        aPaM.GetPoint()->nNode = nNodeIdx;
        aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 0);

        SaveRedlEndPosForRestore aRedlRest( rInsPos.nNode, rInsPos.nContent.GetIndex() );

        rNds.MoveRange( aPaM, rInsPos, rDoc.GetNodes() );

        // delete the last Node as well
        if( !aPaM.GetPoint()->nContent.GetIndex() ||
            ( aPaM.GetPoint()->nNode++ &&       // still empty Nodes at the end?
            &rNds.GetEndOfExtras() != &aPaM.GetPoint()->nNode.GetNode() ))
        {
            aPaM.GetPoint()->nContent.Assign( nullptr, 0 );
            aPaM.SetMark();
            rNds.Delete( aPaM.GetPoint()->nNode,
                        rNds.GetEndOfExtras().GetIndex() -
                        aPaM.GetPoint()->nNode.GetIndex() );
        }

        aRedlRest.Restore();
    }
    else
    {
        SwNodeRange aRg( rNds, nNodeIdx, rNds, (pEndNdIdx
                        ? ((*pEndNdIdx) + 1)
                        : rNds.GetEndOfExtras().GetIndex() ) );
        rNds.MoveNodes(aRg, rDoc.GetNodes(), rInsPos.nNode, nullptr == pEndNdIdx || bForceCreateFrames);

    }
}

// These two methods move the Point of Pam backwards/forwards. With that, one
// can span an area for a Undo/Redo. (The Point is then positioned in front of
// the area to manipulate!)
// The flag indicates if there is still content in front of Point.
bool SwUndoSaveContent::MovePtBackward( SwPaM& rPam )
{
    rPam.SetMark();
    if( rPam.Move( fnMoveBackward ))
        return true;

    // If there is no content onwards, set Point simply to the previous position
    // (Node and Content, so that Content will be detached!)
    --rPam.GetPoint()->nNode;
    rPam.GetPoint()->nContent.Assign( nullptr, 0 );
    return false;
}

void SwUndoSaveContent::MovePtForward( SwPaM& rPam, bool bMvBkwrd )
{
    // Was there content before this position?
    if( bMvBkwrd )
        rPam.Move( fnMoveForward );
    else
    {
        ++rPam.GetPoint()->nNode;
        SwContentNode* pCNd = rPam.GetContentNode();
        if( pCNd )
            pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
        else
            rPam.Move( fnMoveForward );
    }
}

// Delete all objects that have ContentIndices to the given area.
// Currently (1994) these exist:
//                  - Footnotes
//                  - Flys
//                  - Bookmarks

// #i81002# - extending method
// delete certain (not all) cross-reference bookmarks at text node of <rMark>
// and at text node of <rPoint>, if these text nodes aren't the same.
void SwUndoSaveContent::DelContentIndex( const SwPosition& rMark,
                                     const SwPosition& rPoint,
                                     DelContentType nDelContentType )
{
    const SwPosition *pStt = rMark < rPoint ? &rMark : &rPoint,
                    *pEnd = &rMark == pStt ? &rPoint : &rMark;

    SwDoc& rDoc = rMark.nNode.GetNode().GetDoc();

    // if it's not in the doc array, probably missing some invalidation somewhere
    assert(&rPoint.nNode.GetNodes() == &rDoc.GetNodes());
    assert(&rMark.nNode.GetNodes() == &rDoc.GetNodes());

    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    // 1. Footnotes
    if( DelContentType::Ftn & nDelContentType )
    {
        SwFootnoteIdxs& rFootnoteArr = rDoc.GetFootnoteIdxs();
        if( !rFootnoteArr.empty() )
        {
            const SwNode* pFootnoteNd;
            size_t nPos = 0;
            rFootnoteArr.SeekEntry( pStt->nNode, &nPos );
            SwTextFootnote* pSrch;

            // for now delete all that come afterwards
            while( nPos < rFootnoteArr.size() && ( pFootnoteNd =
                &( pSrch = rFootnoteArr[ nPos ] )->GetTextNode())->GetIndex()
                        <= pEnd->nNode.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( (DelContentType::CheckNoCntnt & nDelContentType )
                    ? (&pEnd->nNode.GetNode() == pFootnoteNd )
                    : (( &pStt->nNode.GetNode() == pFootnoteNd &&
                    pStt->nContent.GetIndex() > nFootnoteSttIdx) ||
                    ( &pEnd->nNode.GetNode() == pFootnoteNd &&
                    nFootnoteSttIdx >= pEnd->nContent.GetIndex() )) )
                {
                    ++nPos;     // continue searching
                    continue;
                }

// FIXME: duplicated code here and below -> refactor?
                // Unfortunately an index needs to be created. Otherwise there
                // will be problems with TextNode because the index will be
                // deleted in the DTOR of SwFootnote!
                SwTextNode* pTextNd = const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pFootnoteNd));
                if( !m_pHistory )
                    m_pHistory.reset( new SwHistory );
                SwTextAttr* const pFootnoteHint =
                    pTextNd->GetTextAttrForCharAt( nFootnoteSttIdx );
                assert(pFootnoteHint);
                SwIndex aIdx( pTextNd, nFootnoteSttIdx );
                m_pHistory->Add( pFootnoteHint, pTextNd->GetIndex(), false );
                pTextNd->EraseText( aIdx, 1 );
            }

            while( nPos-- && ( pFootnoteNd = &( pSrch = rFootnoteArr[ nPos ] )->
                    GetTextNode())->GetIndex() >= pStt->nNode.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( !(DelContentType::CheckNoCntnt & nDelContentType) && (
                    ( &pStt->nNode.GetNode() == pFootnoteNd &&
                    pStt->nContent.GetIndex() > nFootnoteSttIdx ) ||
                    ( &pEnd->nNode.GetNode() == pFootnoteNd &&
                    nFootnoteSttIdx >= pEnd->nContent.GetIndex() )))
                    continue;               // continue searching

                // Unfortunately an index needs to be created. Otherwise there
                // will be problems with TextNode because the index will be
                // deleted in the DTOR of SwFootnote!
                SwTextNode* pTextNd = const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pFootnoteNd));
                if( !m_pHistory )
                    m_pHistory.reset( new SwHistory );
                SwTextAttr* const pFootnoteHint =
                    pTextNd->GetTextAttrForCharAt( nFootnoteSttIdx );
                assert(pFootnoteHint);
                SwIndex aIdx( pTextNd, nFootnoteSttIdx );
                m_pHistory->Add( pFootnoteHint, pTextNd->GetIndex(), false );
                pTextNd->EraseText( aIdx, 1 );
            }
        }
    }

    // 2. Flys
    if( DelContentType::Fly & nDelContentType )
    {
        sal_uInt16 nChainInsPos = m_pHistory ? m_pHistory->Count() : 0;
        const SwFrameFormats& rSpzArr = *rDoc.GetSpzFrameFormats();
        if( !rSpzArr.empty() )
        {
            SwFrameFormat* pFormat;
            const SwFormatAnchor* pAnchor;
            size_t n = rSpzArr.size();
            const SwPosition* pAPos;

            while( n && !rSpzArr.empty() )
            {
                pFormat = rSpzArr[--n];
                pAnchor = &pFormat->GetAnchor();
                switch( pAnchor->GetAnchorId() )
                {
                case RndStdIds::FLY_AS_CHAR:
                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        (( DelContentType::CheckNoCntnt & nDelContentType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( *pStt <= *pAPos && *pAPos < *pEnd )) )
                    {
                        if( !m_pHistory )
                            m_pHistory.reset( new SwHistory );
                        SwTextNode *const pTextNd =
                            pAPos->nNode.GetNode().GetTextNode();
                        SwTextAttr* const pFlyHint = pTextNd->GetTextAttrForCharAt(
                            pAPos->nContent.GetIndex());
                        assert(pFlyHint);
                        m_pHistory->Add( pFlyHint, 0, false );
                        // reset n so that no Format is skipped
                        n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                    }
                    break;
                case RndStdIds::FLY_AT_PARA:
                    {
                        pAPos =  pAnchor->GetContentAnchor();
                        if (pAPos &&
                            pStt->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode)
                        {
                            if (!m_pHistory)
                                m_pHistory.reset( new SwHistory );

                            if (!(DelContentType::Replace & nDelContentType)
                                && IsSelectFrameAnchoredAtPara(*pAPos, *pStt, *pEnd, nDelContentType))
                            {
                                m_pHistory->AddDeleteFly(*pFormat, nChainInsPos);
                                // reset n so that no Format is skipped
                                n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                            }
                            // Moving the anchor?
                            else if (!((DelContentType::CheckNoCntnt|DelContentType::ExcludeFlyAtStartEnd)
                                    & nDelContentType) &&
                                // at least for calls from SwUndoDelete,
                                // this should work - other Undos don't
                                // remember the order of the cursor
                                (rPoint.nNode.GetIndex() == pAPos->nNode.GetIndex())
                                // Do not try to move the anchor to a table!
                                && rMark.nNode.GetNode().IsTextNode())
                            {
                                m_pHistory->AddChangeFlyAnchor(*pFormat);
                                SwFormatAnchor aAnch( *pAnchor );
                                SwPosition aPos( rMark.nNode );
                                aAnch.SetAnchor( &aPos );
                                pFormat->SetFormatAttr( aAnch );
                            }
                        }
                    }
                    break;
                case RndStdIds::FLY_AT_CHAR:
                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        ( pStt->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode ) )
                    {
                        if( !m_pHistory )
                            m_pHistory.reset( new SwHistory );
                        if (!(DelContentType::Replace & nDelContentType)
                            && IsDestroyFrameAnchoredAtChar(
                                *pAPos, *pStt, *pEnd, nDelContentType))
                        {
                            m_pHistory->AddDeleteFly(*pFormat, nChainInsPos);
                            n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                        }
                        else if (!((DelContentType::CheckNoCntnt |
                                    DelContentType::ExcludeFlyAtStartEnd)
                                    & nDelContentType))
                        {
                            if( *pStt <= *pAPos && *pAPos < *pEnd )
                            {
                                // These are the objects anchored
                                // between section start and end position
                                // Do not try to move the anchor to a table!
                                if( rMark.nNode.GetNode().GetTextNode() )
                                {
                                    m_pHistory->AddChangeFlyAnchor(*pFormat);
                                    SwFormatAnchor aAnch( *pAnchor );
                                    aAnch.SetAnchor( &rMark );
                                    pFormat->SetFormatAttr( aAnch );
                                }
                            }
                        }
                    }
                    break;
                case RndStdIds::FLY_AT_FLY:

                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        pStt->nNode == pAPos->nNode )
                    {
                        if( !m_pHistory )
                            m_pHistory.reset( new SwHistory );

                        m_pHistory->AddDeleteFly(*pFormat, nChainInsPos);

                        // reset n so that no Format is skipped
                        n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                    }
                    break;
                default: break;
                }
            }
        }
    }

    // 3. Bookmarks
    if( !(DelContentType::Bkm & nDelContentType) )
        return;

    IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
    if( !pMarkAccess->getAllMarksCount() )
        return;

    for( sal_Int32 n = 0; n < pMarkAccess->getAllMarksCount(); ++n )
    {
        // #i81002#
        bool bSavePos = false;
        bool bSaveOtherPos = false;
        const ::sw::mark::IMark *const pBkmk = pMarkAccess->getAllMarksBegin()[n];
        auto const type(IDocumentMarkAccess::GetType(*pBkmk));

        if( DelContentType::CheckNoCntnt & nDelContentType )
        {
            if ( pStt->nNode <= pBkmk->GetMarkPos().nNode
                 && pBkmk->GetMarkPos().nNode < pEnd->nNode )
            {
                bSavePos = true;
            }
            if ( pBkmk->IsExpanded()
                 && pStt->nNode <= pBkmk->GetOtherMarkPos().nNode
                 && pBkmk->GetOtherMarkPos().nNode < pEnd->nNode )
            {
                bSaveOtherPos = true;
            }
        }
        else
        {
            // #i92125#
            // keep cross-reference bookmarks, if content inside one paragraph is deleted.
            if ( rMark.nNode == rPoint.nNode
                && (   type == IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK
                    || type == IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK))
            {
                continue;
            }

            bool bMaybe = false;
            if ( *pStt <= pBkmk->GetMarkPos() && pBkmk->GetMarkPos() <= *pEnd )
            {
                if ( pBkmk->GetMarkPos() == *pEnd
                     || ( *pStt == pBkmk->GetMarkPos() && pBkmk->IsExpanded() ) )
                    bMaybe = true;
                else
                    bSavePos = true;
            }
            if( pBkmk->IsExpanded() &&
                *pStt <= pBkmk->GetOtherMarkPos() && pBkmk->GetOtherMarkPos() <= *pEnd )
            {
                assert(!bSaveOtherPos);
                if (   bSavePos
                    || (*pStt < pBkmk->GetOtherMarkPos() && pBkmk->GetOtherMarkPos() < *pEnd)
                    || (bMaybe
                        && (   type == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK
                            || type == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK
                            || type == IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK
                            || type == IDocumentMarkAccess::MarkType::DATE_FIELDMARK)))
                {
                    if( bMaybe )
                        bSavePos = true;
                    bSaveOtherPos = true;
                }
            }

            if ( !bSavePos && !bSaveOtherPos
                 && dynamic_cast< const ::sw::mark::CrossRefBookmark* >(pBkmk) )
            {
                // certain special handling for cross-reference bookmarks
                const bool bDifferentTextNodesAtMarkAndPoint =
                    rMark.nNode != rPoint.nNode
                    && rMark.nNode.GetNode().GetTextNode()
                    && rPoint.nNode.GetNode().GetTextNode();
                if ( bDifferentTextNodesAtMarkAndPoint )
                {
                    // delete cross-reference bookmark at <pStt>, if only part of
                    // <pEnd> text node content is deleted.
                    if( pStt->nNode == pBkmk->GetMarkPos().nNode
                        && pEnd->nContent.GetIndex() != pEnd->nNode.GetNode().GetTextNode()->Len() )
                    {
                        bSavePos = true;
                        bSaveOtherPos = false; // cross-reference bookmarks are not expanded
                    }
                    // delete cross-reference bookmark at <pEnd>, if only part of
                    // <pStt> text node content is deleted.
                    else if( pEnd->nNode == pBkmk->GetMarkPos().nNode &&
                        pStt->nContent.GetIndex() != 0 )
                    {
                        bSavePos = true;
                        bSaveOtherPos = false; // cross-reference bookmarks are not expanded
                    }
                }
            }
            else if (type == IDocumentMarkAccess::MarkType::ANNOTATIONMARK)
            {
                // delete annotation marks, if its end position is covered by the deletion
                const SwPosition& rAnnotationEndPos = pBkmk->GetMarkEnd();
                if ( *pStt < rAnnotationEndPos && rAnnotationEndPos <= *pEnd )
                {
                    bSavePos = true;
                    bSaveOtherPos = pBkmk->IsExpanded(); //tdf#90138, only save the other pos if there is one
                }
            }
        }

        if ( bSavePos || bSaveOtherPos )
        {
            if (type != IDocumentMarkAccess::MarkType::UNO_BOOKMARK)
            {
                if( !m_pHistory )
                    m_pHistory.reset( new SwHistory );
                m_pHistory->Add( *pBkmk, bSavePos, bSaveOtherPos );
            }
            if ( bSavePos
                 && ( bSaveOtherPos
                      || !pBkmk->IsExpanded() ) )
            {
                pMarkAccess->deleteMark(pMarkAccess->getAllMarksBegin()+n);
                n--;
            }
        }
    }
}

// save a complete section into UndoNodes array
SwUndoSaveSection::SwUndoSaveSection()
    : m_nMoveLen( 0 ), m_nStartPos( ULONG_MAX )
{
}

SwUndoSaveSection::~SwUndoSaveSection()
{
    if (m_pMovedStart) // delete also the section from UndoNodes array
    {
        // SaveSection saves the content in the PostIt section.
        SwNodes& rUNds = m_pMovedStart->GetNode().GetNodes();
        rUNds.Delete( *m_pMovedStart, m_nMoveLen );

        m_pMovedStart.reset();
    }
    m_pRedlineSaveData.reset();
}

void SwUndoSaveSection::SaveSection( const SwNodeIndex& rSttIdx )
{
    SwNodeRange aRg( rSttIdx.GetNode(), *rSttIdx.GetNode().EndOfSectionNode() );
    SaveSection( aRg );
}

void SwUndoSaveSection::SaveSection(
    const SwNodeRange& rRange, bool const bExpandNodes)
{
    SwPaM aPam( rRange.aStart, rRange.aEnd );

    // delete all footnotes, fly frames, bookmarks
    DelContentIndex( *aPam.GetMark(), *aPam.GetPoint() );

    // redlines *before* CorrAbs, because DelBookmarks will make them 0-length
    // but *after* DelContentIndex because that also may use FillSaveData (in
    // flys) and that will be restored *after* this one...
    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if (!SwUndo::FillSaveData( aPam, *m_pRedlineSaveData ))
    {
        m_pRedlineSaveData.reset();
    }

    {
        // move certain indexes out of deleted range
        SwNodeIndex aSttIdx( aPam.Start()->nNode.GetNode() );
        SwNodeIndex aEndIdx( aPam.End()->nNode.GetNode() );
        SwNodeIndex aMvStt( aEndIdx, 1 );
        SwDoc::CorrAbs( aSttIdx, aEndIdx, SwPosition( aMvStt ), true );
    }

    m_nStartPos = rRange.aStart.GetIndex();

    if (bExpandNodes)
    {
        --aPam.GetPoint()->nNode;
        ++aPam.GetMark()->nNode;
    }

    SwContentNode* pCNd = aPam.GetContentNode( false );
    if( pCNd )
        aPam.GetMark()->nContent.Assign( pCNd, 0 );
    pCNd = aPam.GetContentNode();
    if( nullptr != pCNd )
        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    // Keep positions as SwIndex so that this section can be deleted in DTOR
    sal_uLong nEnd;
    m_pMovedStart.reset(new SwNodeIndex(rRange.aStart));
    MoveToUndoNds(aPam, m_pMovedStart.get(), &nEnd);
    m_nMoveLen = nEnd - m_pMovedStart->GetIndex() + 1;
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx,
                                        sal_uInt16 nSectType )
{
    if( ULONG_MAX == m_nStartPos )        // was there any content?
        return;

    // check if the content is at the old position
    SwNodeIndex aSttIdx( pDoc->GetNodes(), m_nStartPos );

    // move the content from UndoNodes array into Fly
    SwStartNode* pSttNd = SwNodes::MakeEmptySection( aSttIdx,
                                            static_cast<SwStartNodeType>(nSectType) );

    RestoreSection( pDoc, SwNodeIndex( *pSttNd->EndOfSectionNode() ));

    if( pIdx )
        *pIdx = *pSttNd;
}

void SwUndoSaveSection::RestoreSection(
        SwDoc *const pDoc, const SwNodeIndex& rInsPos, bool bForceCreateFrames)
{
    if( ULONG_MAX == m_nStartPos )        // was there any content?
        return;

    SwPosition aInsPos( rInsPos );
    sal_uLong nEnd = m_pMovedStart->GetIndex() + m_nMoveLen - 1;
    MoveFromUndoNds(*pDoc, m_pMovedStart->GetIndex(), aInsPos, &nEnd, bForceCreateFrames);

    // destroy indices again, content was deleted from UndoNodes array
    m_pMovedStart.reset();
    m_nMoveLen = 0;

    if( m_pRedlineSaveData )
    {
        SwUndo::SetSaveData( *pDoc, *m_pRedlineSaveData );
        m_pRedlineSaveData.reset();
    }
}

void SwUndoSaveSection::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    SwUndoSaveContent::dumpAsXml(pWriter);
}

// save and set the RedlineData
SwRedlineSaveData::SwRedlineSaveData(
    SwComparePosition eCmpPos,
    const SwPosition& rSttPos,
    const SwPosition& rEndPos,
    SwRangeRedline& rRedl,
    bool bCopyNext )
    : SwUndRng( rRedl )
    , SwRedlineData( rRedl.GetRedlineData(), bCopyNext )
{
    assert( SwComparePosition::Outside == eCmpPos ||
            !rRedl.GetContentIdx() ); // "Redline with Content"

    switch (eCmpPos)
    {
    case SwComparePosition::OverlapBefore:        // Pos1 overlaps Pos2 at the beginning
        m_nEndNode = rEndPos.nNode.GetIndex();
        m_nEndContent = rEndPos.nContent.GetIndex();
        break;

    case SwComparePosition::OverlapBehind:        // Pos1 overlaps Pos2 at the end
        m_nSttNode = rSttPos.nNode.GetIndex();
        m_nSttContent = rSttPos.nContent.GetIndex();
        break;

    case SwComparePosition::Inside:                // Pos1 lays completely in Pos2
        m_nSttNode = rSttPos.nNode.GetIndex();
        m_nSttContent = rSttPos.nContent.GetIndex();
        m_nEndNode = rEndPos.nNode.GetIndex();
        m_nEndContent = rEndPos.nContent.GetIndex();
        break;

    case SwComparePosition::Outside:               // Pos2 lays completely in Pos1
        if ( rRedl.GetContentIdx() )
        {
            // than move section into UndoArray and memorize it
            SaveSection( *rRedl.GetContentIdx() );
            rRedl.SetContentIdx( nullptr );
        }
        break;

    case SwComparePosition::Equal:                 // Pos1 is exactly as big as Pos2
        break;

    default:
        assert(false);
    }

#if OSL_DEBUG_LEVEL > 0
    m_nRedlineCount = rSttPos.nNode.GetNode().GetDoc().getIDocumentRedlineAccess().GetRedlineTable().size();
#endif
}

SwRedlineSaveData::~SwRedlineSaveData()
{
}

void SwRedlineSaveData::RedlineToDoc( SwPaM const & rPam )
{
    SwDoc& rDoc = rPam.GetDoc();
    SwRangeRedline* pRedl = new SwRangeRedline( *this, rPam );

    if( GetMvSttIdx() )
    {
        SwNodeIndex aIdx( rDoc.GetNodes() );
        RestoreSection( &rDoc, &aIdx, SwNormalStartNode );
        if( GetHistory() )
            GetHistory()->Rollback( &rDoc );
        pRedl->SetContentIdx( &aIdx );
    }
    SetPaM( *pRedl );
    // First, delete the "old" so that in an Append no unexpected things will
    // happen, e.g. a delete in an insert. In the latter case the just restored
    // content will be deleted and not the one you originally wanted.
    rDoc.getIDocumentRedlineAccess().DeleteRedline( *pRedl, false, RedlineType::Any );

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld | RedlineFlags::DontCombineRedlines );
    //#i92154# let UI know about a new redline with comment
    if (rDoc.GetDocShell() && (!pRedl->GetComment().isEmpty()) )
        rDoc.GetDocShell()->Broadcast(SwRedlineHint());

    auto const result(rDoc.getIDocumentRedlineAccess().AppendRedline(pRedl, true));
    assert(result != IDocumentRedlineAccess::AppendResult::IGNORED); // SwRedlineSaveData::RedlineToDoc: insert redline failed
    (void) result; // unused in non-debug
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

bool SwUndo::FillSaveData(
    const SwPaM& rRange,
    SwRedlineSaveDatas& rSData,
    bool bDelRange,
    bool bCopyNext )
{
    rSData.clear();

    const SwPosition* pStt = rRange.Start();
    const SwPosition* pEnd = rRange.End();
    const SwRedlineTable& rTable = rRange.GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
    SwRedlineTable::size_type n = 0;
    rRange.GetDoc().getIDocumentRedlineAccess().GetRedline( *pStt, &n );
    for ( ; n < rTable.size(); ++n )
    {
        SwRangeRedline* pRedl = rTable[n];

        const SwComparePosition eCmpPos =
            ComparePosition( *pStt, *pEnd, *pRedl->Start(), *pRedl->End() );
        if ( eCmpPos != SwComparePosition::Before
             && eCmpPos != SwComparePosition::Behind
             && eCmpPos != SwComparePosition::CollideEnd
             && eCmpPos != SwComparePosition::CollideStart )
        {

            rSData.push_back(std::unique_ptr<SwRedlineSaveData, o3tl::default_delete<SwRedlineSaveData>>(new SwRedlineSaveData(eCmpPos, *pStt, *pEnd, *pRedl, bCopyNext)));
        }
    }
    if( !rSData.empty() && bDelRange )
    {
        rRange.GetDoc().getIDocumentRedlineAccess().DeleteRedline( rRange, false, RedlineType::Any );
    }
    return !rSData.empty();
}

bool SwUndo::FillSaveDataForFormat(
    const SwPaM& rRange,
    SwRedlineSaveDatas& rSData )
{
    rSData.clear();

    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTable& rTable = rRange.GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
    SwRedlineTable::size_type n = 0;
    rRange.GetDoc().getIDocumentRedlineAccess().GetRedline( *pStt, &n );
    for ( ; n < rTable.size(); ++n )
    {
        SwRangeRedline* pRedl = rTable[n];
        if ( RedlineType::Format == pRedl->GetType() )
        {
            const SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRedl->Start(), *pRedl->End() );
            if ( eCmpPos != SwComparePosition::Before
                 && eCmpPos != SwComparePosition::Behind
                 && eCmpPos != SwComparePosition::CollideEnd
                 && eCmpPos != SwComparePosition::CollideStart )
            {
                rSData.push_back(std::unique_ptr<SwRedlineSaveData, o3tl::default_delete<SwRedlineSaveData>>(new SwRedlineSaveData(eCmpPos, *pStt, *pEnd, *pRedl, true)));
            }

        }
    }
    return !rSData.empty();
}


void SwUndo::SetSaveData( SwDoc& rDoc, SwRedlineSaveDatas& rSData )
{
    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );
    SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );

    for( size_t n = rSData.size(); n; )
        rSData[ --n ].RedlineToDoc( aPam );

#if OSL_DEBUG_LEVEL > 0
    // check redline count against count saved in RedlineSaveData object
    assert(rSData.empty() ||
           (rSData[0].m_nRedlineCount == rDoc.getIDocumentRedlineAccess().GetRedlineTable().size()));
            // "redline count not restored properly"
#endif

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

bool SwUndo::HasHiddenRedlines( const SwRedlineSaveDatas& rSData )
{
    for( size_t n = rSData.size(); n; )
        if( rSData[ --n ].GetMvSttIdx() )
            return true;
    return false;
}

bool SwUndo::CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                        const SwRedlineSaveDatas& rCheck, bool bCurrIsEnd )
{
    if( rCurr.size() != rCheck.size() )
        return false;

    for( size_t n = 0; n < rCurr.size(); ++n )
    {
        const SwRedlineSaveData& rSet = rCurr[ n ];
        const SwRedlineSaveData& rGet = rCheck[ n ];
        if( rSet.m_nSttNode != rGet.m_nSttNode ||
            rSet.GetMvSttIdx() || rGet.GetMvSttIdx() ||
            ( bCurrIsEnd ? rSet.m_nSttContent != rGet.m_nEndContent
                            : rSet.m_nEndContent != rGet.m_nSttContent ) ||
            !rGet.CanCombine( rSet ) )
        {
            return false;
        }
    }

    for( size_t n = 0; n < rCurr.size(); ++n )
    {
        SwRedlineSaveData& rSet = rCurr[ n ];
        const SwRedlineSaveData& rGet = rCheck[ n ];
        if( bCurrIsEnd )
            rSet.m_nSttContent = rGet.m_nSttContent;
        else
            rSet.m_nEndContent = rGet.m_nEndContent;
    }
    return true;
}

OUString ShortenString(const OUString & rStr, sal_Int32 nLength, const OUString & rFillStr)
{
    assert(nLength - rFillStr.getLength() >= 2);

    if (rStr.getLength() <= nLength)
        return rStr;

    nLength -= rFillStr.getLength();
    if ( nLength < 2 )
        nLength = 2;

    const sal_Int32 nFrontLen = nLength - nLength / 2;
    const sal_Int32 nBackLen = nLength - nFrontLen;

    return rStr.subView(0, nFrontLen)
           + rFillStr
           + rStr.subView(rStr.getLength() - nBackLen);
}

static bool IsAtEndOfSection(SwPosition const& rAnchorPos)
{
    SwNodeIndex node(*rAnchorPos.nNode.GetNode().EndOfSectionNode());
    SwContentNode *const pNode(SwNodes::GoPrevious(&node));
    assert(pNode);
    assert(rAnchorPos.nNode <= node); // last valid anchor pos is last content
    return node == rAnchorPos.nNode
        // at-para fly has no SwIndex!
        && (rAnchorPos.nContent == pNode->Len() || rAnchorPos.nContent.GetIdxReg() == nullptr);
}

static bool IsAtStartOfSection(SwPosition const& rAnchorPos)
{
    SwNodes const& rNodes(rAnchorPos.nNode.GetNodes());
    SwNodeIndex node(*rAnchorPos.nNode.GetNode().StartOfSectionNode());
    SwContentNode *const pNode(rNodes.GoNext(&node));
    assert(pNode);
    (void) pNode;
    assert(node <= rAnchorPos.nNode);
    return node == rAnchorPos.nNode && rAnchorPos.nContent == 0;
}

/// passed start / end position could be on section start / end node
static bool IsAtEndOfSection2(SwPosition const& rPos)
{
    return rPos.nNode.GetNode().IsEndNode()
        || IsAtEndOfSection(rPos);
}

static bool IsAtStartOfSection2(SwPosition const& rPos)
{
    return rPos.nNode.GetNode().IsStartNode()
        || IsAtStartOfSection(rPos);
}

static bool IsNotBackspaceHeuristic(
        SwPosition const& rStart, SwPosition const& rEnd)
{
    // check if the selection is backspace/delete created by DelLeft/DelRight
    if (rStart.nNode.GetIndex() + 1 != rEnd.nNode.GetIndex())
        return true;
    if (rEnd.nContent != 0)
        return true;
    const SwTextNode* pTextNode = rStart.nNode.GetNode().GetTextNode();
    if (!pTextNode || rStart.nContent != pTextNode->Len())
        return true;
    return false;
}

bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelContentType const nDelContentType)
{
    assert(rStart <= rEnd);

    // CheckNoCntnt means DelFullPara which is obvious to handle
    if (DelContentType::CheckNoCntnt & nDelContentType)
    {   // exclude selection end node because it won't be deleted
        return (rAnchorPos.nNode < rEnd.nNode)
            && (rStart.nNode <= rAnchorPos.nNode);
    }

    if ((nDelContentType & DelContentType::WriterfilterHack)
        && rAnchorPos.GetDoc().IsInWriterfilterImport())
    {   // FIXME hack for writerfilter RemoveLastParagraph() and MakeFlyAndMove(); can't test file format more specific?
        return (rStart < rAnchorPos) && (rAnchorPos < rEnd);
    }

    if (nDelContentType & DelContentType::ExcludeFlyAtStartEnd)
    {   // exclude selection start and end node
        return (rAnchorPos.nNode < rEnd.nNode)
            && (rStart.nNode < rAnchorPos.nNode);
    }

    // in general, exclude the start and end position
    return ((rStart < rAnchorPos)
            || (rStart == rAnchorPos
                // special case: fully deleted node
                && ((rStart.nNode != rEnd.nNode && rStart.nContent == 0
                        // but not if the selection is backspace/delete!
                        && IsNotBackspaceHeuristic(rStart, rEnd))
                    || (IsAtStartOfSection(rAnchorPos) && IsAtEndOfSection2(rEnd)))))
        && ((rAnchorPos < rEnd)
            || (rAnchorPos == rEnd
                // special case: fully deleted node
                && ((rEnd.nNode != rStart.nNode && rEnd.nContent == rEnd.nNode.GetNode().GetTextNode()->Len()
                        && IsNotBackspaceHeuristic(rStart, rEnd))
                    || (IsAtEndOfSection(rAnchorPos) && IsAtStartOfSection2(rStart)))));
}

bool IsSelectFrameAnchoredAtPara(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelContentType const nDelContentType)
{
    assert(rStart <= rEnd);

    // CheckNoCntnt means DelFullPara which is obvious to handle
    if (DelContentType::CheckNoCntnt & nDelContentType)
    {   // exclude selection end node because it won't be deleted
        return (rAnchorPos.nNode < rEnd.nNode)
            && (rStart.nNode <= rAnchorPos.nNode);
    }

    if ((nDelContentType & DelContentType::WriterfilterHack)
        && rAnchorPos.GetDoc().IsInWriterfilterImport())
    {   // FIXME hack for writerfilter RemoveLastParagraph() and MakeFlyAndMove(); can't test file format more specific?
        // but it MUST NOT be done during the SetRedlineFlags at the end of ODF
        // import, where the IsInXMLImport() cannot be checked because the
        // stupid code temp. overrides it - instead rely on setting the ALLFLYS
        // flag in MoveFromSection() and converting that to CheckNoCntnt with
        // adjusted cursor!
        return (rStart.nNode < rAnchorPos.nNode) && (rAnchorPos.nNode < rEnd.nNode);
    }

    // in general, exclude the start and end position
    return ((rStart.nNode < rAnchorPos.nNode)
            || (rStart.nNode == rAnchorPos.nNode
                && !(nDelContentType & DelContentType::ExcludeFlyAtStartEnd)
                // special case: fully deleted node
                && ((rStart.nNode != rEnd.nNode && rStart.nContent == 0
                        // but not if the selection is backspace/delete!
                        && IsNotBackspaceHeuristic(rStart, rEnd))
                    || (IsAtStartOfSection2(rStart) && IsAtEndOfSection2(rEnd)))))
        && ((rAnchorPos.nNode < rEnd.nNode)
            || (rAnchorPos.nNode == rEnd.nNode
                && !(nDelContentType & DelContentType::ExcludeFlyAtStartEnd)
                // special case: fully deleted node
                && ((rEnd.nNode != rStart.nNode && rEnd.nContent == rEnd.nNode.GetNode().GetTextNode()->Len()
                        && IsNotBackspaceHeuristic(rStart, rEnd))
                    || (IsAtEndOfSection2(rEnd) && IsAtStartOfSection2(rStart)))));
}

bool IsFlySelectedByCursor(SwDoc const & rDoc,
        SwPosition const & rStart, SwPosition const & rEnd)
{
    for (SwFrameFormat const*const pFly : *rDoc.GetSpzFrameFormats())
    {
        SwFormatAnchor const& rAnchor(pFly->GetAnchor());
        switch (rAnchor.GetAnchorId())
        {
            case RndStdIds::FLY_AT_CHAR:
            case RndStdIds::FLY_AT_PARA:
            {
                SwPosition const*const pAnchorPos(rAnchor.GetContentAnchor());
                // can this really be null?
                if (pAnchorPos != nullptr
                    && ((rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
                        ? IsDestroyFrameAnchoredAtChar(*pAnchorPos, rStart, rEnd)
                        : IsSelectFrameAnchoredAtPara(*pAnchorPos, rStart, rEnd)))
                {
                    return true;
                }
            }
            break;
            default: // other types not relevant
            break;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
