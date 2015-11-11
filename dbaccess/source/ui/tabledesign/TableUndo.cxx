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

#include "TableUndo.hxx"
#include "dbu_tbl.hrc"
#include "TEditControl.hxx"
#include "TableRow.hxx"
#include "browserids.hxx"
#include "TableController.hxx"
#include "TableDesignView.hxx"
#include "FieldDescriptions.hxx"
#include <tools/debug.hxx>

using namespace dbaui;
using namespace ::svt;


// class OTableDesignUndoAct
OTableDesignUndoAct::OTableDesignUndoAct( OTableRowView* pOwner,sal_uInt16 nCommentID ) : OCommentUndoAction(nCommentID)
    ,m_pTabDgnCtrl(  pOwner )
{
    m_pTabDgnCtrl->m_nCurUndoActId++;
}

OTableDesignUndoAct::~OTableDesignUndoAct()
{
}

void OTableDesignUndoAct::Undo()
{
    m_pTabDgnCtrl->m_nCurUndoActId--;

    // doc has not been modified if first undo was reverted
    if( m_pTabDgnCtrl->m_nCurUndoActId == 0 )
    {
        m_pTabDgnCtrl->GetView()->getController().setModified(sal_False);
        m_pTabDgnCtrl->GetView()->getController().InvalidateFeature(SID_SAVEDOC);
    }
}

void OTableDesignUndoAct::Redo()
{
    m_pTabDgnCtrl->m_nCurUndoActId++;

    // restore Modified-flag after Redo of first Undo-action
    if( m_pTabDgnCtrl->m_nCurUndoActId > 0 )
    {
        m_pTabDgnCtrl->GetView()->getController().setModified(sal_True);
        m_pTabDgnCtrl->GetView()->getController().InvalidateFeature(SID_SAVEDOC);
    }
}

// class OTableDesignCellUndoAct
OTableDesignCellUndoAct::OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, sal_uInt16 nColumn ) :
     OTableDesignUndoAct( pOwner ,STR_TABED_UNDO_CELLMODIFIED)
    ,m_nCol( nColumn )
    ,m_nRow( nRowID )
{
    // read text at position (m_nRow, m_nCol)
    m_sOldText = m_pTabDgnCtrl->GetCellData( m_nRow, m_nCol );
}

OTableDesignCellUndoAct::~OTableDesignCellUndoAct()
{
}

void OTableDesignCellUndoAct::Undo()
{
    // store text at old line and restore the old one
    m_pTabDgnCtrl->ActivateCell( m_nRow, m_nCol );
    m_sNewText = m_pTabDgnCtrl->GetCellData( m_nRow, m_nCol );
    m_pTabDgnCtrl->SetCellData( m_nRow, m_nCol, m_sOldText );
    // line has not been modified if the first Undo was reverted
    if (m_pTabDgnCtrl->GetCurUndoActId() == 1)
    {
        CellControllerRef xController = m_pTabDgnCtrl->Controller();
        if ( xController.Is() )
            xController->ClearModified();
        m_pTabDgnCtrl->GetView()->getController().setModified(sal_False);

    }

    OTableDesignUndoAct::Undo();
}

void OTableDesignCellUndoAct::Redo()
{
    // restore new text
    m_pTabDgnCtrl->ActivateCell( m_nRow, m_nCol );
    m_pTabDgnCtrl->SetCellData( m_nRow, m_nCol, m_sNewText );

    OTableDesignUndoAct::Redo();
}

// class OTableEditorUndoAct
OTableEditorUndoAct::OTableEditorUndoAct( OTableEditorCtrl* pOwner,sal_uInt16 _nCommentID ) :
     OTableDesignUndoAct(  pOwner ,_nCommentID)
    ,pTabEdCtrl(pOwner)
{
}

OTableEditorUndoAct::~OTableEditorUndoAct()
{
}

// class OTableEditorTypeSelUndoAct
OTableEditorTypeSelUndoAct::OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, sal_uInt16 nColumn, const TOTypeInfoSP& _pOldType )
    :OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_TYPE_CHANGED)
    ,m_nCol( nColumn )
    ,m_nRow( nRowID )
    ,m_pOldType( _pOldType )
{
}

OTableEditorTypeSelUndoAct::~OTableEditorTypeSelUndoAct()
{
}

void OTableEditorTypeSelUndoAct::Undo()
{
    // restore type
    OFieldDescription* pFieldDesc = pTabEdCtrl->GetFieldDescr(m_nRow);
    if(pFieldDesc)
        m_pNewType = pFieldDesc->getTypeInfo();
    else
        m_pNewType = TOTypeInfoSP();
    pTabEdCtrl->SetCellData(m_nRow,m_nCol,m_pOldType);
    pTabEdCtrl->SwitchType( m_pOldType );

    OTableEditorUndoAct::Undo();
}

void OTableEditorTypeSelUndoAct::Redo()
{
    // new type
    pTabEdCtrl->GoToRowColumnId( m_nRow ,m_nCol);
    pTabEdCtrl->SetCellData(m_nRow,m_nCol,m_pNewType);

    OTableEditorUndoAct::Redo();
}

// class OTableEditorDelUndoAct
OTableEditorDelUndoAct::OTableEditorDelUndoAct( OTableEditorCtrl* pOwner) :
     OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_ROWDELETED)
{
    // fill DeletedRowList
    ::std::vector< std::shared_ptr<OTableRow> >* pOriginalRows = pOwner->GetRowList();
    long nIndex = pOwner->FirstSelectedRow();
     std::shared_ptr<OTableRow>  pOriginalRow;
     std::shared_ptr<OTableRow>  pNewRow;

    while( nIndex >= 0 )
    {
        pOriginalRow = (*pOriginalRows)[nIndex];
        pNewRow.reset(new OTableRow( *pOriginalRow, nIndex ));
        m_aDeletedRows.push_back( pNewRow);

        nIndex = pOwner->NextSelectedRow();
    }
}

OTableEditorDelUndoAct::~OTableEditorDelUndoAct()
{
    m_aDeletedRows.clear();
}

void OTableEditorDelUndoAct::Undo()
{
    // Insert the deleted line
    sal_uLong nPos;
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aIter = m_aDeletedRows.begin();
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aEnd = m_aDeletedRows.end();

     std::shared_ptr<OTableRow>  pNewOrigRow;
    ::std::vector< std::shared_ptr<OTableRow> >* pOriginalRows = pTabEdCtrl->GetRowList();

    for(;aIter != aEnd;++aIter)
    {
        pNewOrigRow.reset(new OTableRow( **aIter ));
        nPos = (*aIter)->GetPos();
        pOriginalRows->insert( pOriginalRows->begin()+nPos,pNewOrigRow);
    }

    pTabEdCtrl->DisplayData(pTabEdCtrl->GetCurRow());
    pTabEdCtrl->Invalidate();
    OTableEditorUndoAct::Undo();
}

void OTableEditorDelUndoAct::Redo()
{
    // delete line again
    sal_uLong nPos;
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aIter = m_aDeletedRows.begin();
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aEnd = m_aDeletedRows.end();
    ::std::vector< std::shared_ptr<OTableRow> >* pOriginalRows = pTabEdCtrl->GetRowList();

    for(;aIter != aEnd;++aIter)
    {
        nPos = (*aIter)->GetPos();
        pOriginalRows->erase( pOriginalRows->begin()+nPos );
    }

    pTabEdCtrl->DisplayData(pTabEdCtrl->GetCurRow());
    pTabEdCtrl->Invalidate();
    OTableEditorUndoAct::Redo();
}

// class OTableEditorInsUndoAct
OTableEditorInsUndoAct::OTableEditorInsUndoAct( OTableEditorCtrl* pOwner,
                                               long nInsertPosition ,
                                               const ::std::vector<  std::shared_ptr<OTableRow> >& _vInsertedRows)
    :OTableEditorUndoAct( pOwner,STR_TABED_UNDO_ROWINSERTED )
    ,m_vInsertedRows(_vInsertedRows)
    ,m_nInsPos( nInsertPosition )
{
}

OTableEditorInsUndoAct::~OTableEditorInsUndoAct()
{
    m_vInsertedRows.clear();
}

void OTableEditorInsUndoAct::Undo()
{
    // delete lines again
    ::std::vector< std::shared_ptr<OTableRow> >* pOriginalRows = pTabEdCtrl->GetRowList();
    for( long i=(m_nInsPos+m_vInsertedRows.size()-1); i>(m_nInsPos-1); i-- )
    {
        pOriginalRows->erase(pOriginalRows->begin()+i);
    }

    pTabEdCtrl->RowRemoved( m_nInsPos, m_vInsertedRows.size() );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Undo();
}

void OTableEditorInsUndoAct::Redo()
{
    // insert lines again
    long nInsertRow = m_nInsPos;
     std::shared_ptr<OTableRow>  pRow;
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aIter = m_vInsertedRows.begin();
    ::std::vector< std::shared_ptr<OTableRow> >::iterator aEnd = m_vInsertedRows.end();
    ::std::vector< std::shared_ptr<OTableRow> >* pRowList = pTabEdCtrl->GetRowList();
    for(;aIter != aEnd;++aIter)
    {
        pRow.reset(new OTableRow( **aIter ));
        pRowList->insert( pRowList->begin()+nInsertRow ,pRow );
        nInsertRow++;
    }

    pTabEdCtrl->RowInserted( m_nInsPos, m_vInsertedRows.size() );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Redo();
}

// class OTableEditorInsNewUndoAct
OTableEditorInsNewUndoAct::OTableEditorInsNewUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition, long nInsertedRows ) :
     OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_NEWROWINSERTED)
    ,m_nInsPos( nInsertPosition )
    ,m_nInsRows( nInsertedRows )
{
}

OTableEditorInsNewUndoAct::~OTableEditorInsNewUndoAct()
{
}

void OTableEditorInsNewUndoAct::Undo()
{
    // delete inserted lines
    ::std::vector< std::shared_ptr<OTableRow> >* pOriginalRows = pTabEdCtrl->GetRowList();

    for( long i=(m_nInsPos+m_nInsRows-1); i>(m_nInsPos-1); i-- )
    {
        pOriginalRows->erase(pOriginalRows->begin()+i);
    }

    pTabEdCtrl->RowRemoved( m_nInsPos, m_nInsRows );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Undo();
}

void OTableEditorInsNewUndoAct::Redo()
{
    // insert lines again
    ::std::vector< std::shared_ptr<OTableRow> >* pRowList = pTabEdCtrl->GetRowList();

    for( long i=m_nInsPos; i<(m_nInsPos+m_nInsRows); i++ )
        pRowList->insert( pRowList->begin()+i,std::shared_ptr<OTableRow>(new OTableRow()) );

    pTabEdCtrl->RowInserted( m_nInsPos, m_nInsRows );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Redo();
}

// class OPrimKeyUndoAct
OPrimKeyUndoAct::OPrimKeyUndoAct( OTableEditorCtrl* pOwner, const MultiSelection& aDeletedKeys, const MultiSelection& aInsertedKeys) :
     OTableEditorUndoAct( pOwner ,STR_TABLEDESIGN_UNDO_PRIMKEY)
    ,m_aDelKeys( aDeletedKeys )
    ,m_aInsKeys( aInsertedKeys )
    ,m_pEditorCtrl( pOwner )
{
}

OPrimKeyUndoAct::~OPrimKeyUndoAct()
{
}

void OPrimKeyUndoAct::Undo()
{
    ::std::vector< std::shared_ptr<OTableRow> >* pRowList = pTabEdCtrl->GetRowList();
    std::shared_ptr<OTableRow>  pRow;
    long nIndex;

    // delete inserted keys
    for( nIndex = m_aInsKeys.FirstSelected(); nIndex != (long)SFX_ENDOFSELECTION; nIndex=m_aInsKeys.NextSelected() )
    {
        OSL_ENSURE(nIndex <= static_cast<long>(pRowList->size()),"Index for undo isn't valid!");
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( false );
    }

    // restore deleted keys
    for( nIndex = m_aDelKeys.FirstSelected(); nIndex != (long)SFX_ENDOFSELECTION; nIndex=m_aDelKeys.NextSelected() )
    {
        OSL_ENSURE(nIndex <= static_cast<long>(pRowList->size()),"Index for undo isn't valid!");
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( true );
    }

    m_pEditorCtrl->InvalidateHandleColumn();
    OTableEditorUndoAct::Undo();
}

void OPrimKeyUndoAct::Redo()
{
    ::std::vector< std::shared_ptr<OTableRow> >* pRowList = pTabEdCtrl->GetRowList();
    long nIndex;

    // delete the deleted keys
    for( nIndex = m_aDelKeys.FirstSelected(); nIndex != (long)SFX_ENDOFSELECTION; nIndex=m_aDelKeys.NextSelected() )
        (*pRowList)[nIndex]->SetPrimaryKey( false );

    // restore the inserted keys
    for( nIndex = m_aInsKeys.FirstSelected(); nIndex != (long)SFX_ENDOFSELECTION; nIndex=m_aInsKeys.NextSelected() )
        (*pRowList)[nIndex]->SetPrimaryKey( true );

    m_pEditorCtrl->InvalidateHandleColumn();
    OTableEditorUndoAct::Redo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
