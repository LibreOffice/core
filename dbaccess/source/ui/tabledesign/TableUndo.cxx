/*************************************************************************
 *
 *  $RCSfile: TableUndo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEUNDO_HXX
#include "TableUndo.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#include "TEditControl.hxx"
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
using namespace dbaui;

//==============================================================================
// class OTableDesignUndoAct
//==============================================================================
OTableDesignUndoAct::OTableDesignUndoAct( OTableRowView* pOwner,USHORT nCommentID ) : OCommentUndoAction(nCommentID)
    ,m_pTabDgnCtrl(  pOwner )
{
    m_pTabDgnCtrl->m_nCurUndoActId++;
}

//-------------------------------------------------------------------------
OTableDesignUndoAct::~OTableDesignUndoAct()
{
}

//-------------------------------------------------------------------------
void OTableDesignUndoAct::Undo()
{
    m_pTabDgnCtrl->m_nCurUndoActId--;

    //////////////////////////////////////////////////////////////////////
    // Wenn erstes Undo zurueckgenommen wurde, ist Doc nicht modifiziert worden
    if( m_pTabDgnCtrl->m_nCurUndoActId == 0 )
    {
        m_pTabDgnCtrl->GetView()->getController()->setModified(sal_False);
        m_pTabDgnCtrl->GetView()->getController()->InvalidateFeature(SID_SAVEDOC);
    }
}

//-------------------------------------------------------------------------
void OTableDesignUndoAct::Redo()
{
    m_pTabDgnCtrl->m_nCurUndoActId++;

    //////////////////////////////////////////////////////////////////////
    // Wenn Redo fuer erste Undo-Action, muss Modified-Flag wieder gesetzt werden
    if( m_pTabDgnCtrl->m_nCurUndoActId > 0 )
    {
        m_pTabDgnCtrl->GetView()->getController()->setModified(sal_True);
        m_pTabDgnCtrl->GetView()->getController()->InvalidateFeature(SID_SAVEDOC);
    }
}
//==============================================================================
// class OTableDesignCellUndoAct
//==============================================================================
OTableDesignCellUndoAct::OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, USHORT nColumn ) :
     OTableDesignUndoAct( pOwner ,STR_TABED_UNDO_CELLMODIFIED)
    ,m_nRow( nRowID )
    ,m_nCol( nColumn )
{
    //////////////////////////////////////////////////////////////////////
    // Text an der Position (m_nRow, m_nCol) auslesen
    m_sOldText = m_pTabDgnCtrl->GetData( m_nRow, m_nCol );
}

//-------------------------------------------------------------------------
OTableDesignCellUndoAct::~OTableDesignCellUndoAct()
{
}

//-------------------------------------------------------------------------
void OTableDesignCellUndoAct::Undo()
{
    //////////////////////////////////////////////////////////////////////
    // Neuen Text der alten Zelle speichern und alten wieder einsetzen
    m_pTabDgnCtrl->ActivateCell( m_nRow, m_nCol );
    m_sNewText = m_pTabDgnCtrl->GetData( m_nRow, m_nCol );
    m_pTabDgnCtrl->SetData( m_nRow, m_nCol, m_sOldText );
    //////////////////////////////////////////////////////////////////////
    // Wenn erstes Undo zurueckgenommen wurde, ist Zelle nicht mehr modifiziert
    if (m_pTabDgnCtrl->GetCurUndoActId() == 1)
    {
        DbCellControllerRef xController = m_pTabDgnCtrl->Controller();
        if(xController.Is())
            xController->ClearModified();
        m_pTabDgnCtrl->GetView()->getController()->setModified(sal_False);

    }

    OTableDesignUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OTableDesignCellUndoAct::Redo()
{
    //////////////////////////////////////////////////////////////////////
    // Neuen Text wieder einseten
    m_pTabDgnCtrl->ActivateCell( m_nRow, m_nCol );
    m_pTabDgnCtrl->SetData( m_nRow, m_nCol, m_sNewText );

    OTableDesignUndoAct::Redo();
}

//==============================================================================
// class OTableEditorUndoAct
//==============================================================================
OTableEditorUndoAct::OTableEditorUndoAct( OTableEditorCtrl* pOwner,USHORT _nCommentID ) :
     OTableDesignUndoAct(  pOwner ,_nCommentID)
    ,pTabEdCtrl(pOwner)
{
}

//-------------------------------------------------------------------------
OTableEditorUndoAct::~OTableEditorUndoAct()
{
}

//==============================================================================
// class OTableEditorTypeSelUndoAct
//==============================================================================
OTableEditorTypeSelUndoAct::OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, USHORT nColumn, const OTypeInfo* _pOldType ) :
     OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_TYPE_CHANGED)
    ,m_nRow( nRowID )
    ,m_nCol( nColumn )
    ,m_pOldType( _pOldType )
{
}

//-------------------------------------------------------------------------
OTableEditorTypeSelUndoAct::~OTableEditorTypeSelUndoAct()
{
}

//-------------------------------------------------------------------------
void OTableEditorTypeSelUndoAct::Undo()
{
    //////////////////////////////////////////////////////////////////////
    // Typ zuruecksetzen
    pTabEdCtrl->GoToRow( m_nRow );
    pTabEdCtrl->GoToColumnId( m_nCol );
    OFieldDescription* pFieldDesc = pTabEdCtrl->GetActRow()->GetActFieldDescr();
    if(pFieldDesc)
        m_pNewType = pFieldDesc->getTypeInfo();
    pTabEdCtrl->SetData(m_nRow,m_nCol,m_pOldType);
    pTabEdCtrl->SwitchType( m_pOldType );

    OTableEditorUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OTableEditorTypeSelUndoAct::Redo()
{
    //////////////////////////////////////////////////////////////////////
    // Neuen Typ
    pTabEdCtrl->GoToRow( m_nRow );
    pTabEdCtrl->GoToColumnId( m_nCol );
    pTabEdCtrl->SetData(m_nRow,m_nCol,m_pNewType);

    OTableEditorUndoAct::Redo();
}

//==============================================================================
// class OTableEditorDelUndoAct
//==============================================================================
OTableEditorDelUndoAct::OTableEditorDelUndoAct( OTableEditorCtrl* pOwner) :
     OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_ROWDELETED)
{
    //////////////////////////////////////////////////////////////////////
    // DeletedRowList fuellen
    ::std::vector<OTableRow*>* pOriginalRows = pOwner->GetRowList();
    long nIndex = pOwner->FirstSelectedRow();
    OTableRow* pOriginalRow;
    OTableRow* pNewRow;

    while( nIndex >= 0 )
    {
        pOriginalRow = (*pOriginalRows)[nIndex];
        pNewRow = new OTableRow( *pOriginalRow, nIndex );
        m_aDeletedRows.push_back( pNewRow);

        nIndex = pOwner->NextSelectedRow();
    }
}

//-------------------------------------------------------------------------
OTableEditorDelUndoAct::~OTableEditorDelUndoAct()
{
    //////////////////////////////////////////////////////////////////////
    // DeletedRowList loeschen
    ::std::vector<OTableRow*>::iterator aIter = m_aDeletedRows.begin();
    for(;aIter != m_aDeletedRows.end();++aIter)
        delete *aIter;

    m_aDeletedRows.clear();
}

//-------------------------------------------------------------------------
void OTableEditorDelUndoAct::Undo()
{
    //////////////////////////////////////////////////////////////////////
    // Geloeschte Zeilen wieder einfuegen
    ULONG nPos;
    ::std::vector<OTableRow*>::iterator aIter = m_aDeletedRows.begin();

    OTableRow* pNewOrigRow;
    ::std::vector<OTableRow*>* pOriginalRows = pTabEdCtrl->GetRowList();

    for(;aIter != m_aDeletedRows.end();++aIter)
    {
        pNewOrigRow = new OTableRow( **aIter );
        nPos = (*aIter)->GetPos();
        pOriginalRows->insert( pOriginalRows->begin()+nPos,pNewOrigRow);
    }

    pTabEdCtrl->Invalidate();
    OTableEditorUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OTableEditorDelUndoAct::Redo()
{
    //////////////////////////////////////////////////////////////////////
    // Zeilen wieder loeschen
    ULONG nPos;
    ::std::vector<OTableRow*>::iterator aIter = m_aDeletedRows.begin();
    ::std::vector<OTableRow*>* pOriginalRows = pTabEdCtrl->GetRowList();

    for(;aIter != m_aDeletedRows.end();++aIter)
    {
        nPos = (*aIter)->GetPos();
        delete (*pOriginalRows)[ nPos];
        pOriginalRows->erase( pOriginalRows->begin()+nPos );
    }

    pTabEdCtrl->Invalidate();
    OTableEditorUndoAct::Redo();
}

//-------------------------------------------------------------------------
//==============================================================================
// class OTableEditorInsUndoAct
//==============================================================================
OTableEditorInsUndoAct::OTableEditorInsUndoAct( OTableEditorCtrl* pOwner,
                                               long nInsertPosition ,
                                               const ::std::vector< OTableRow*>& _vInsertedRows)
    :OTableEditorUndoAct( pOwner,STR_TABED_UNDO_ROWINSERTED )
    ,m_nInsPos( nInsertPosition )
    ,m_vInsertedRows(_vInsertedRows)
{
}

//-------------------------------------------------------------------------
OTableEditorInsUndoAct::~OTableEditorInsUndoAct()
{
    //////////////////////////////////////////////////////////////////////
    // InsertedRowList loeschen
    ::std::vector<OTableRow*>::iterator aIter = m_vInsertedRows.begin();
    for(;aIter != m_vInsertedRows.end();++aIter)
        delete *aIter;

    m_vInsertedRows.clear();
}

//-------------------------------------------------------------------------
void OTableEditorInsUndoAct::Undo()
{
    //////////////////////////////////////////////////////////////////////
    // Eingefuegte Zeilen wieder loeschen
    ::std::vector<OTableRow*>* pOriginalRows = pTabEdCtrl->GetRowList();
    for( long i=(m_nInsPos+m_vInsertedRows.size()-1); i>(m_nInsPos-1); i-- )
    {
        delete (*pOriginalRows)[i];
        pOriginalRows->erase(pOriginalRows->begin()+i);
    }

    pTabEdCtrl->RowRemoved( m_nInsPos, m_vInsertedRows.size(), TRUE );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OTableEditorInsUndoAct::Redo()
{
    //////////////////////////////////////////////////////////////////////
    // Zeilen wieder einfuegen
    long nInsertRow = m_nInsPos;
    OTableRow* pRow;
    ::std::vector<OTableRow*>::iterator aIter = m_vInsertedRows.begin();
    ::std::vector<OTableRow*>* pRowList = pTabEdCtrl->GetRowList();
    for(;aIter != m_vInsertedRows.end();++aIter)
    {
        pRow = new OTableRow( **aIter );
        pRowList->insert( pRowList->begin()+nInsertRow ,pRow );
        nInsertRow++;
    }

    pTabEdCtrl->RowInserted( m_nInsPos, m_vInsertedRows.size(), TRUE );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Redo();
}

//==============================================================================
// class OTableEditorInsNewUndoAct
//==============================================================================
OTableEditorInsNewUndoAct::OTableEditorInsNewUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition, long nInsertedRows ) :
     OTableEditorUndoAct( pOwner ,STR_TABED_UNDO_NEWROWINSERTED)
    ,m_nInsPos( nInsertPosition )
    ,m_nInsRows( nInsertedRows )
{
}

//-------------------------------------------------------------------------
OTableEditorInsNewUndoAct::~OTableEditorInsNewUndoAct()
{
}

//-------------------------------------------------------------------------
void OTableEditorInsNewUndoAct::Undo()
{
    //////////////////////////////////////////////////////////////////////
    // Eingefuegte Zeilen wieder loeschen
    ::std::vector<OTableRow*>* pOriginalRows = pTabEdCtrl->GetRowList();

    for( long i=(m_nInsPos+m_nInsRows-1); i>(m_nInsPos-1); i-- )
    {
        delete (*pOriginalRows)[i];
        pOriginalRows->erase(pOriginalRows->begin()+i);
    }

    pTabEdCtrl->RowRemoved( m_nInsPos, m_nInsRows, TRUE );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OTableEditorInsNewUndoAct::Redo()
{
    //////////////////////////////////////////////////////////////////////
    // Zeilen wieder einfuegen
    ::std::vector<OTableRow*>* pRowList = pTabEdCtrl->GetRowList();

    for( long i=m_nInsPos; i<(m_nInsPos+m_nInsRows); i++ )
        pRowList->insert( pRowList->begin()+i,new OTableRow() );

    pTabEdCtrl->RowInserted( m_nInsPos, m_nInsRows, TRUE );
    pTabEdCtrl->InvalidateHandleColumn();

    OTableEditorUndoAct::Redo();
}

//-------------------------------------------------------------------------
//========================================================================
// class OPrimKeyUndoAct
//========================================================================

//-------------------------------------------------------------------------
OPrimKeyUndoAct::OPrimKeyUndoAct( OTableEditorCtrl* pOwner, MultiSelection aDeletedKeys, MultiSelection aInsertedKeys) :
     OTableEditorUndoAct( pOwner ,STR_TABLEDESIGN_UNDO_PRIMKEY)
    ,m_aDelKeys( aDeletedKeys )
    ,m_aInsKeys( aInsertedKeys )
    ,m_pEditorCtrl( pOwner )
{
}

//-------------------------------------------------------------------------
OPrimKeyUndoAct::~OPrimKeyUndoAct()
{
}

//-------------------------------------------------------------------------
void OPrimKeyUndoAct::Undo()
{
    ::std::vector<OTableRow*>* pRowList = pTabEdCtrl->GetRowList();
    OTableRow* pRow = NULL;

    //////////////////////////////////////////////////////////////////////
    // Die eingefuegten Keys loeschen
    for( long nIndex = m_aInsKeys.FirstSelected(); nIndex != SFX_ENDOFSELECTION; nIndex=m_aInsKeys.NextSelected() )
    {
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( FALSE );
    }

    //////////////////////////////////////////////////////////////////////
    // Die geloeschten Keys herstellen
    for( nIndex = m_aDelKeys.FirstSelected(); nIndex != SFX_ENDOFSELECTION; nIndex=m_aDelKeys.NextSelected() )
    {
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( TRUE );
    }

    m_pEditorCtrl->InvalidateHandleColumn();
    OTableEditorUndoAct::Undo();
}

//-------------------------------------------------------------------------
void OPrimKeyUndoAct::Redo()
{
    ::std::vector<OTableRow*>* pRowList = pTabEdCtrl->GetRowList();
    OTableRow* pRow = NULL;

    //////////////////////////////////////////////////////////////////////
    // Die geloeschten Keys loeschen
    for( long nIndex = m_aDelKeys.FirstSelected(); nIndex != SFX_ENDOFSELECTION; nIndex=m_aDelKeys.NextSelected() )
    {
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( FALSE );
    }

    //////////////////////////////////////////////////////////////////////
    // Die eingefuegten Keys herstellen
    for( nIndex = m_aInsKeys.FirstSelected(); nIndex != SFX_ENDOFSELECTION; nIndex=m_aInsKeys.NextSelected() )
    {
        pRow = (*pRowList)[nIndex];
        pRow->SetPrimaryKey( TRUE );
    }

    m_pEditorCtrl->InvalidateHandleColumn();
    OTableEditorUndoAct::Redo();
}




