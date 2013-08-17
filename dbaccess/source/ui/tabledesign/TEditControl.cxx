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

#include "TEditControl.hxx"
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include "dbu_tbl.hrc"
#include "dbustrings.hrc"
#include "browserids.hxx"
#include "dbaccess_helpid.hrc"
#include <comphelper/types.hxx>
#include "FieldDescControl.hxx"
#include "FieldDescriptions.hxx"
#include <vcl/msgbox.hxx>
#include "TableUndo.hxx"
#include "TableController.hxx"
#include <connectivity/dbtools.hxx>
#include "SqlNameEdit.hxx"
#include "TableRowExchange.hxx"
#include <sot/storage.hxx>
#include "UITools.hxx"
#include "TableFieldControl.hxx"
#include "dsntypes.hxx"

#include "dbaccess_slotid.hrc"

using namespace ::dbaui;
using namespace ::comphelper;
using namespace ::svt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;

namespace dbaui
{
    extern String GetTypeString( sal_uInt16 nType );
}

//  TYPEINIT1(OTableEditorCtrl, DBView);
DBG_NAME(OTableEditorCtrl)

#define HANDLE_ID       0

// default field widths
#define FIELDNAME_WIDTH     100
#define FIELDTYPE_WIDTH     150
#define FIELDDESCR_WIDTH    300

// Maximum length in description field
#define MAX_DESCR_LEN       256

DBG_NAME(ClipboardInvalidator)
OTableEditorCtrl::ClipboardInvalidator::ClipboardInvalidator(sal_uLong nTimeout,OTableEditorCtrl* _pOwner)
: m_pOwner(_pOwner)
{
    DBG_CTOR(ClipboardInvalidator,NULL);

    m_aInvalidateTimer.SetTimeout(nTimeout);
    m_aInvalidateTimer.SetTimeoutHdl(LINK(this, OTableEditorCtrl::ClipboardInvalidator, OnInvalidate));
    m_aInvalidateTimer.Start();
}

OTableEditorCtrl::ClipboardInvalidator::~ClipboardInvalidator()
{
    m_aInvalidateTimer.Stop();

    DBG_DTOR(ClipboardInvalidator,NULL);
}

IMPL_LINK_NOARG(OTableEditorCtrl::ClipboardInvalidator, OnInvalidate)
{
    m_pOwner->GetView()->getController().InvalidateFeature(SID_CUT);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_COPY);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_PASTE);
    return 0L;
}

void OTableEditorCtrl::Init()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OTableRowView::Init();

    // Should it be opened ReadOnly ?
    sal_Bool bRead(GetView()->getController().isReadOnly());

    SetReadOnly( bRead );

    // Insert the columns
    String aColumnName( ModuleRes(STR_TAB_FIELD_COLUMN_NAME) );
    InsertDataColumn( FIELD_NAME, aColumnName, FIELDNAME_WIDTH );

    aColumnName = String( ModuleRes(STR_TAB_FIELD_COLUMN_DATATYPE) );
    InsertDataColumn( FIELD_TYPE, aColumnName, FIELDTYPE_WIDTH );

    ::dbaccess::ODsnTypeCollection aDsnTypes(GetView()->getController().getORB());
    sal_Bool bShowColumnDescription = aDsnTypes.supportsColumnDescription(::comphelper::getString(GetView()->getController().getDataSource()->getPropertyValue(PROPERTY_URL)));
    aColumnName = String( ModuleRes(STR_TAB_HELP_TEXT) );
    InsertDataColumn( HELP_TEXT, aColumnName, bShowColumnDescription ? FIELDTYPE_WIDTH : FIELDDESCR_WIDTH );

    if ( bShowColumnDescription )
    {
        aColumnName = String( ModuleRes(STR_COLUMN_DESCRIPTION) );
        InsertDataColumn( COLUMN_DESCRIPTION, aColumnName, FIELDTYPE_WIDTH );
    }

    InitCellController();

    // Insert the rows
    RowInserted(0, m_pRowList->size(), sal_True);
}

void OTableEditorCtrl::UpdateAll()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    RowRemoved(0, GetRowCount(), sal_False);
    m_nDataPos = 0;

    InvalidateFeatures();
    Invalidate();
}

OTableEditorCtrl::OTableEditorCtrl(Window* pWindow)
    :OTableRowView(pWindow)
    ,pNameCell(NULL)
    ,pTypeCell(NULL)
    ,pHelpTextCell(NULL)
    ,pDescrCell(NULL)
    ,pDescrWin(NULL)
    ,nIndexEvent(0)
    ,nCutEvent(0)
    ,nPasteEvent(0)
    ,nDeleteEvent(0)
    ,nInsNewRowsEvent(0)
    ,nInvalidateTypeEvent(0)
    ,m_eChildFocus(NONE)
    ,nOldDataPos(-1)
    ,bSaveOnMove(sal_True)
    ,bReadOnly(sal_True)
    ,m_aInvalidate(500,this)
{
    DBG_CTOR(OTableEditorCtrl,NULL);

    SetHelpId(HID_TABDESIGN_BACKGROUND);
    GetDataWindow().SetHelpId(HID_CTL_TABLEEDIT);

    m_pRowList = GetView()->getController().getRows();
    m_nDataPos = 0;
}

SfxUndoManager& OTableEditorCtrl::GetUndoManager() const
{
    return GetView()->getController().GetUndoManager();
}

sal_Bool OTableEditorCtrl::IsReadOnly()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    return bReadOnly;
}

void OTableEditorCtrl::SetReadOnly( sal_Bool bRead )
{
    // nothing to do?
    if (bRead == IsReadOnly())
        // This check is important, as the underlying Def may be unnecessarily locked or unlocked
        // or worse, this action may not be reversed afterwards
        return;

    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    bReadOnly = bRead;

    // Disable active cells
    long nRow(GetCurRow());
    sal_uInt16 nCol(GetCurColumnId());
    DeactivateCell();

    // Select the correct Browsers cursor
    BrowserMode nMode(BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                      BROWSER_HLINESFULL      | BROWSER_VLINESFULL|BROWSER_AUTOSIZE_LASTCOL);
    if( !bReadOnly )
        nMode |= BROWSER_HIDECURSOR;
    SetMode(nMode);

    if( !bReadOnly )
        ActivateCell( nRow, nCol );
}

void OTableEditorCtrl::InitCellController()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Cell Field name
    xub_StrLen nMaxTextLen = EDIT_NOLIMIT;
    OUString sExtraNameChars;
    Reference<XConnection> xCon;
    try
    {
        xCon = GetView()->getController().getConnection();
        Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

        nMaxTextLen = ((xub_StrLen)xMetaData.is() ? static_cast<xub_StrLen>(xMetaData->getMaxColumnNameLength()) : 0);

        if( nMaxTextLen == 0 )
            nMaxTextLen = EDIT_NOLIMIT;
        sExtraNameChars = xMetaData.is() ? xMetaData->getExtraNameCharacters() : OUString();

    }
    catch(SQLException&)
    {
        OSL_FAIL("getMaxColumnNameLength");
    }

    pNameCell = new OSQLNameEdit( &GetDataWindow(), sExtraNameChars,WB_LEFT );
    pNameCell->SetMaxTextLen( nMaxTextLen );
    pNameCell->setCheck( isSQL92CheckEnabled(xCon) );

    // Cell type
    pTypeCell = new ListBoxControl( &GetDataWindow() );
    pTypeCell->SetDropDownLineCount( 15 );

    // Cell description
    pDescrCell = new Edit( &GetDataWindow(), WB_LEFT );
    pDescrCell->SetMaxTextLen( MAX_DESCR_LEN );

    pHelpTextCell = new Edit( &GetDataWindow(), WB_LEFT );
    pHelpTextCell->SetMaxTextLen( MAX_DESCR_LEN );

    pNameCell->SetHelpId(HID_TABDESIGN_NAMECELL);
    pTypeCell->SetHelpId(HID_TABDESIGN_TYPECELL);
    pDescrCell->SetHelpId(HID_TABDESIGN_COMMENTCELL);
    pHelpTextCell->SetHelpId(HID_TABDESIGN_HELPTEXT);

    Size aHeight;
    const Control* pControls[] = { pTypeCell,pDescrCell,pNameCell,pHelpTextCell};
    for(sal_Size i= 0; i < sizeof(pControls) / sizeof(pControls[0]);++i)
    {
        const Size aTemp(pControls[i]->GetOptimalSize());
        if ( aTemp.Height() > aHeight.Height() )
            aHeight.Height() = aTemp.Height();
    }
    SetDataRowHeight(aHeight.Height());

    ClearModified();
}

void OTableEditorCtrl::ClearModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    pNameCell->ClearModifyFlag();
    pDescrCell->ClearModifyFlag();
    pHelpTextCell->ClearModifyFlag();
    pTypeCell->SaveValue();
}

OTableEditorCtrl::~OTableEditorCtrl()
{
    DBG_DTOR(OTableEditorCtrl,NULL);
    // Reset the Undo-Manager
    GetUndoManager().Clear();

    // Take possible Events from the queue
    if( nCutEvent )
        Application::RemoveUserEvent( nCutEvent );
    if( nPasteEvent )
        Application::RemoveUserEvent( nPasteEvent );
    if( nDeleteEvent )
        Application::RemoveUserEvent( nDeleteEvent );
    if( nInsNewRowsEvent )
        Application::RemoveUserEvent( nInsNewRowsEvent );
    if( nInvalidateTypeEvent )
        Application::RemoveUserEvent( nInvalidateTypeEvent );

    // Delete the control types
    delete pNameCell;
    delete pTypeCell;
    delete pDescrCell;
    delete pHelpTextCell;
}

sal_Bool OTableEditorCtrl::SetDataPtr( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if(nRow == -1)
        return sal_False;

    OSL_ENSURE((xub_StrLen)nRow < m_pRowList->size(),"Row is greater than size!");
    if(nRow >= (long)m_pRowList->size())
        return sal_False;
    pActRow = (*m_pRowList)[nRow];
    return pActRow != 0;
}

sal_Bool OTableEditorCtrl::SeekRow(long _nRow)
{
    // Call the Base class to remember which row must be repainted
    EditBrowseBox::SeekRow(_nRow);

    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    m_nCurrentPos = _nRow;
    return SetDataPtr(_nRow);
}

void OTableEditorCtrl::PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                   sal_uInt16 nColumnId ) const
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    const String aText( GetCellText( m_nCurrentPos, nColumnId ));

    rDev.Push( PUSH_CLIPREGION );
    rDev.SetClipRegion(Region(rRect));
    rDev.DrawText( rRect, aText, TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER );
    rDev.Pop();
}

CellController* OTableEditorCtrl::GetController(long nRow, sal_uInt16 nColumnId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // If EditorCtrl is ReadOnly, editing is forbidden
    Reference<XPropertySet> xTable = GetView()->getController().getTable();
    if (IsReadOnly() || (   xTable.is() &&
                            xTable->getPropertySetInfo()->hasPropertyByName(PROPERTY_TYPE) &&
                            ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == OUString("VIEW")))
        return NULL;

    // If the row is ReadOnly, editing is forbidden
    SetDataPtr( nRow );
    if( pActRow->IsReadOnly() )
        return NULL;

    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();
    switch (nColumnId)
    {
        case FIELD_NAME:
            return new EditCellController( pNameCell );
        case FIELD_TYPE:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new ListBoxCellController( pTypeCell );
            else return NULL;
        case HELP_TEXT:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new EditCellController( pHelpTextCell );
            else
                return NULL;
        case COLUMN_DESCRIPTION:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new EditCellController( pDescrCell );
            else
                return NULL;
        default:
            return NULL;
    }
}

void OTableEditorCtrl::InitController(CellControllerRef&, long nRow, sal_uInt16 nColumnId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    SeekRow( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();
    String aInitString;

    switch (nColumnId)
    {
        case FIELD_NAME:
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetName();
            pNameCell->SetText( aInitString );
            pNameCell->SaveValue();
            break;
        case FIELD_TYPE:
            {
                if ( pActFieldDescr && pActFieldDescr->getTypeInfo() )
                    aInitString = pActFieldDescr->getTypeInfo()->aUIName;

                // Set the ComboBox contents
                pTypeCell->Clear();
                if( !pActFieldDescr )
                    break;

                const OTypeInfoMap* pTypeInfo = GetView()->getController().getTypeInfo();
                OTypeInfoMap::const_iterator aIter = pTypeInfo->begin();
                OTypeInfoMap::const_iterator aEnd = pTypeInfo->end();
                for(;aIter != aEnd;++aIter)
                    pTypeCell->InsertEntry( aIter->second->aUIName );
                pTypeCell->SelectEntry( aInitString );
            }

            break;
        case HELP_TEXT:
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetHelpText();
            pHelpTextCell->SetText( aInitString );
            pHelpTextCell->SaveValue();
            break;
        case COLUMN_DESCRIPTION:
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetDescription();
            pDescrCell->SetText( aInitString );
            pDescrCell->SaveValue();
            break;

    }
}

EditBrowseBox::RowStatus OTableEditorCtrl::GetRowStatus(long nRow) const
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    const_cast<OTableEditorCtrl*>(this)->SetDataPtr( nRow );
    if( !pActRow )
        return EditBrowseBox::CLEAN;
    if (nRow >= 0 && nRow == m_nDataPos)
    {
        if( pActRow->IsPrimaryKey() )
            return EditBrowseBox::CURRENT_PRIMARYKEY;
        return EditBrowseBox::CURRENT;
    }
    else
    {
        if( pActRow->IsPrimaryKey() )
            return EditBrowseBox::PRIMARYKEY;
        return EditBrowseBox::CLEAN;
    }
}

sal_Bool OTableEditorCtrl::SaveCurRow()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if (GetFieldDescr(GetCurRow()) == NULL)
        // there is no data in the current row
        return sal_True;
    if (!SaveModified())
        return sal_False;

    SetDataPtr(GetCurRow());
    pDescrWin->SaveData( pActRow->GetActFieldDescr() );
    return sal_True;
}

void OTableEditorCtrl::DisplayData(long nRow, sal_Bool bGrabFocus)
{
    // go to the correct cell
    SetDataPtr(nRow);

    // Disable Edit-Mode temporarily
    sal_Bool bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    CellControllerRef aTemp;
    InitController(aTemp, nRow, FIELD_NAME);
    InitController(aTemp, nRow, FIELD_TYPE);
    InitController(aTemp, nRow, COLUMN_DESCRIPTION);
    InitController(aTemp, nRow, HELP_TEXT);

    GoToRow(nRow);
    // Update the Description-Window
    GetView()->GetDescWin()->DisplayData(GetFieldDescr(nRow));
    // redraw the row
    RowModified(nRow);

    // and re-enable edit mode
    if (bWasEditing || bGrabFocus)
        ActivateCell(nRow, GetCurColumnId(), bGrabFocus);
}

void OTableEditorCtrl::CursorMoved()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // New line ?
    m_nDataPos = GetCurRow();
    if( m_nDataPos != nOldDataPos && m_nDataPos != -1)
    {
        CellControllerRef aTemp;
        InitController(aTemp,m_nDataPos,FIELD_NAME);
        InitController(aTemp,m_nDataPos,FIELD_TYPE);
        InitController(aTemp,m_nDataPos,COLUMN_DESCRIPTION);
        InitController(aTemp,m_nDataPos,HELP_TEXT);
    }

    OTableRowView::CursorMoved();
}

sal_Int32 OTableEditorCtrl::HasFieldName( const String& rFieldName )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    ::comphelper::UStringMixEqual bCase(xMetaData.is() ? xMetaData->supportsMixedCaseQuotedIdentifiers() : sal_True);

    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aIter = m_pRowList->begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::iterator aEnd = m_pRowList->end();
    OFieldDescription* pFieldDescr;
    sal_Int32 nCount(0);
    for(;aIter != aEnd;++aIter)
    {
        pFieldDescr = (*aIter)->GetActFieldDescr();
        if( pFieldDescr && bCase(rFieldName,pFieldDescr->GetName()))
            nCount++;
    }
    return nCount;
}

sal_Bool OTableEditorCtrl::SaveData(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Store the cell content
    SetDataPtr( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    switch( nColId)
    {
        // Store NameCell
        case FIELD_NAME:
        {
            // If there is no name, do nothing
            String aName(pNameCell->GetText());

            if( !aName.Len() )
            {
                // If FieldDescr exists, the field is deleted and the old content restored
                if (pActFieldDescr)
                {
                    GetUndoManager().AddUndoAction(new OTableEditorTypeSelUndoAct(this, nRow, FIELD_TYPE, pActFieldDescr->getTypeInfo()));
                    SwitchType(TOTypeInfoSP());
                    pActFieldDescr = pActRow->GetActFieldDescr();
                }
                else
                    return sal_True;
            }
            if(pActFieldDescr)
                pActFieldDescr->SetName( aName );
            pNameCell->ClearModifyFlag();

            break;
        }

        // Store the field type
        case FIELD_TYPE:
            break;

        // Store DescrCell
        case HELP_TEXT:
        {
            // Wenn aktuelle Feldbeschreibung NULL, Default setzen
            if( !pActFieldDescr )
            {
                pHelpTextCell->SetText(String());
                pHelpTextCell->ClearModifyFlag();
            }
            else
                pActFieldDescr->SetHelpText( pHelpTextCell->GetText() );
            break;
        }
        case COLUMN_DESCRIPTION:
        {
            // Set the default if the field description is null
            if( !pActFieldDescr )
            {
                pDescrCell->SetText(String());
                pDescrCell->ClearModifyFlag();
            }
            else
                pActFieldDescr->SetDescription( pDescrCell->GetText() );
            break;
        }
        case FIELD_PROPERTY_DEFAULT:
        case FIELD_PROPERTY_REQUIRED:
        case FIELD_PROPERTY_TEXTLEN:
        case FIELD_PROPERTY_NUMTYPE:
        case FIELD_PROPERTY_AUTOINC:
        case FIELD_PROPERTY_LENGTH:
        case FIELD_PROPERTY_SCALE:
        case FIELD_PROPERTY_BOOL_DEFAULT:
            pDescrWin->SaveData(pActFieldDescr);

            if ( FIELD_PROPERTY_AUTOINC == nColId && pActFieldDescr->IsAutoIncrement() )
            {
                OTableController& rController = GetView()->getController();
                if ( rController.isAutoIncrementPrimaryKey() )
                {
                    pActFieldDescr->SetPrimaryKey( true );
                    InvalidateHandleColumn();
                    Invalidate();
                }
            }
            break;
    }
    return sal_True;
}

sal_Bool OTableEditorCtrl::SaveModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    sal_uInt16 nColId = GetCurColumnId();

    switch( nColId )
    {
        // Fieled type
        case FIELD_TYPE:
        {
            // Reset the type
            resetType();
        } break;
    }

    return sal_True;
}

sal_Bool OTableEditorCtrl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    if (!EditBrowseBox::CursorMoving(nNewRow, nNewCol))
        return sal_False;

    // Called after SaveModified(), current row is still the old one
    m_nDataPos = nNewRow;
    nOldDataPos = GetCurRow();

    // Reset the markers
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );

    // Store the data from the Property window
    if( SetDataPtr(nOldDataPos) && pDescrWin)
        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

    // Show new data in the Property window
    if( SetDataPtr(m_nDataPos) && pDescrWin)
        pDescrWin->DisplayData( pActRow->GetActFieldDescr() );

    return sal_True;
}

IMPL_LINK( OTableEditorCtrl, InvalidateFieldType, void*, /*EMPTYTAG*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    nInvalidateTypeEvent = 0;
    Invalidate( GetFieldRectPixel(nOldDataPos, FIELD_TYPE) );

    return 0;
}

void OTableEditorCtrl::CellModified( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    // If the description is null, use the default
    if(nRow == -1)
        nRow = GetCurRow();
    SetDataPtr( nRow );
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    String sActionDescription;
    switch ( nColId )
    {
    case FIELD_NAME:    sActionDescription = String( ModuleRes( STR_CHANGE_COLUMN_NAME ) ); break;
    case FIELD_TYPE:    sActionDescription = String( ModuleRes( STR_CHANGE_COLUMN_TYPE ) ); break;
    case HELP_TEXT:
    case COLUMN_DESCRIPTION:   sActionDescription = String( ModuleRes( STR_CHANGE_COLUMN_DESCRIPTION ) ); break;
    default:            sActionDescription = String( ModuleRes( STR_CHANGE_COLUMN_ATTRIBUTE ) ); break;
    }

    GetUndoManager().EnterListAction( sActionDescription, String() );
    if (!pActFieldDescr)
    {
        const OTypeInfoMap* pTypeInfoMap = GetView()->getController().getTypeInfo();
        if ( !pTypeInfoMap->empty() )
        {
            OTypeInfoMap::const_iterator aTypeIter = pTypeInfoMap->find(DataType::VARCHAR);
            if ( aTypeIter == pTypeInfoMap->end() )
                aTypeIter = pTypeInfoMap->begin();
            pActRow->SetFieldType( aTypeIter->second );
        }
        else
            pActRow->SetFieldType( GetView()->getController().getTypeInfoFallBack() );

        nInvalidateTypeEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, InvalidateFieldType) );
        pActFieldDescr = pActRow->GetActFieldDescr();
        pDescrWin->DisplayData( pActFieldDescr );
        GetUndoManager().AddUndoAction( new OTableEditorTypeSelUndoAct(this, nRow, nColId+1, TOTypeInfoSP()) );
    }

    if( nColId != FIELD_TYPE )
        GetUndoManager().AddUndoAction( new OTableDesignCellUndoAct(this, nRow, nColId) );
    else
    {
        GetUndoManager().AddUndoAction(new OTableEditorTypeSelUndoAct(this, GetCurRow(), nColId, GetFieldDescr(GetCurRow())->getTypeInfo()));
        resetType();
    }

    SaveData(nRow,nColId);
    // SaveData could create a undo action as well
    GetUndoManager().LeaveListAction();
    RowModified(nRow);
    CellControllerRef xController(Controller());
    if(xController.Is())
        xController->SetModified();

    // Set the Modify flag
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

void OTableEditorCtrl::resetType()
{
    sal_uInt16 nPos = pTypeCell->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        SwitchType( GetView()->getController().getTypeInfo(nPos) );
    else
        SwitchType(TOTypeInfoSP());
}

void OTableEditorCtrl::CellModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    CellModified( GetCurRow(), GetCurColumnId() );
}

void OTableEditorCtrl::InvalidateFeatures()
{
    GetView()->getController().InvalidateFeature(SID_UNDO);
    GetView()->getController().InvalidateFeature(SID_REDO);
    GetView()->getController().InvalidateFeature(SID_SAVEDOC);
}

void OTableEditorCtrl::Undo()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    InvalidateFeatures();
}

void OTableEditorCtrl::Redo()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    InvalidateFeatures();
}

void OTableEditorCtrl::CopyRows()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // set to the right row and save it
    if( SetDataPtr(m_nDataPos) )
        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

    // Copy selected rows to the ClipboardList
     ::boost::shared_ptr<OTableRow>  pClipboardRow;
     ::boost::shared_ptr<OTableRow>  pRow;
    ::std::vector< ::boost::shared_ptr<OTableRow> > vClipboardList;
    vClipboardList.reserve(GetSelectRowCount());

    for( long nIndex=FirstSelectedRow(); nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()); nIndex=NextSelectedRow() )
    {
        pRow = (*m_pRowList)[nIndex];
        OSL_ENSURE(pRow,"OTableEditorCtrl::CopyRows: Row is NULL!");
        if ( pRow && pRow->GetActFieldDescr() )
        {
            pClipboardRow.reset(new OTableRow( *pRow ));
            vClipboardList.push_back( pClipboardRow);
        }
    }
    if(!vClipboardList.empty())
    {
        OTableRowExchange* pData = new OTableRowExchange(vClipboardList);
        Reference< ::com::sun::star::datatransfer::XTransferable> xRef = pData;
        pData->CopyToClipboard(GetParent());
    }
}

String OTableEditorCtrl::GenerateName( const String& rName )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Create a base name for appending numbers to
    String aBaseName;
    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    xub_StrLen nMaxTextLen((xub_StrLen)( xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0));

    if( (rName.Len()+2) >nMaxTextLen )
        aBaseName = rName.Copy( 0, nMaxTextLen-2 );
    else
        aBaseName = rName;

    // append a sequential number to the base name (up to 99)
    String aFieldName( rName);
    sal_Int32 i=1;
    while( HasFieldName(aFieldName) )
    {
        aFieldName = aBaseName + OUString::number(i);
        i++;
    }

    return aFieldName;
}

void OTableEditorCtrl::InsertRows( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    ::std::vector<  ::boost::shared_ptr<OTableRow> > vInsertedUndoRedoRows; // need for undo/redo handling
    // get rows from clipboard
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(SOT_FORMATSTR_ID_SBA_TABED))
    {
        SotStorageStreamRef aStreamRef;
        aTransferData.GetSotStorageStream(SOT_FORMATSTR_ID_SBA_TABED,aStreamRef);
        if(aStreamRef.Is())
        {
            aStreamRef->Seek(STREAM_SEEK_TO_BEGIN);
            aStreamRef->ResetError();
            long nInsertRow = nRow;
            String aFieldName;
             ::boost::shared_ptr<OTableRow>  pRow;
            sal_Int32 nSize = 0;
            (*aStreamRef) >> nSize;
            vInsertedUndoRedoRows.reserve(nSize);
            for(sal_Int32 i=0;i < nSize;++i)
            {
                pRow.reset(new OTableRow());
                (*aStreamRef) >> *pRow;
                pRow->SetReadOnly( sal_False );
                sal_Int32 nType = pRow->GetActFieldDescr()->GetType();
                if ( pRow->GetActFieldDescr() )
                    pRow->GetActFieldDescr()->SetType(GetView()->getController().getTypeInfoByType(nType));
                // Adjust the field names
                aFieldName = GenerateName( pRow->GetActFieldDescr()->GetName() );
                pRow->GetActFieldDescr()->SetName( aFieldName );
                pRow->SetPos(nInsertRow);
                m_pRowList->insert( m_pRowList->begin()+nInsertRow,pRow );
                vInsertedUndoRedoRows.push_back(::boost::shared_ptr<OTableRow>(new OTableRow(*pRow)));
                nInsertRow++;
            }
        }
    }
    // RowInserted calls CursorMoved.
    // The UI data should not be stored here.
    bSaveOnMove = sal_False;
    RowInserted( nRow,vInsertedUndoRedoRows.size(),sal_True );
    bSaveOnMove = sal_True;

    // Create the Undo-Action
    GetUndoManager().AddUndoAction( new OTableEditorInsUndoAct(this, nRow,vInsertedUndoRedoRows) );
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

void OTableEditorCtrl::DeleteRows()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OSL_ENSURE(GetView()->getController().isDropAllowed(),"Call of DeleteRows not valid here. Please check isDropAllowed!");
    // Create the Undo-Action
    GetUndoManager().AddUndoAction( new OTableEditorDelUndoAct(this) );

    // Delete all marked rows
    long nIndex = FirstSelectedRow();
    nOldDataPos = nIndex;
    bSaveOnMove = sal_False;

    while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
    {
        // Remove rows
        m_pRowList->erase( m_pRowList->begin()+nIndex );
        RowRemoved( nIndex, 1, sal_True );

        // Insert the empty row at the end
        m_pRowList->push_back( ::boost::shared_ptr<OTableRow>(new OTableRow()));
        RowInserted( GetRowCount()-1, 1, sal_True );

        nIndex = FirstSelectedRow();
    }

    bSaveOnMove = sal_True;

    // Force the current record to be displayed
    m_nDataPos = GetCurRow();
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );
    SetDataPtr( m_nDataPos );
    ActivateCell();
    pDescrWin->DisplayData( pActRow->GetActFieldDescr() );
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

void OTableEditorCtrl::InsertNewRows( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OSL_ENSURE(GetView()->getController().isAddAllowed(),"Call of InsertNewRows not valid here. Please check isAppendAllowed!");
    // Create Undo-Action
    long nInsertRows = GetSelectRowCount();
    if( !nInsertRows )
        nInsertRows = 1;
    GetUndoManager().AddUndoAction( new OTableEditorInsNewUndoAct(this, nRow, nInsertRows) );
    // Insert the number of of selected rows
    for( long i=nRow; i<(nRow+nInsertRows); i++ )
        m_pRowList->insert( m_pRowList->begin()+i ,::boost::shared_ptr<OTableRow>(new OTableRow()));
    RowInserted( nRow, nInsertRows, sal_True );

    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

String OTableEditorCtrl::GetControlText( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Read the Browser Controls
    if( nColId < FIELD_FIRST_VIRTUAL_COLUMN )
    {
        GoToRow( nRow );
        GoToColumnId( nColId );
        CellControllerRef xController = Controller();
        if(xController.Is())
            return xController->GetWindow().GetText();
        else
            return GetCellText(nRow,nColId);
    }

    // Read the Controls on the Tabpage
    else
        return pDescrWin->GetControlText( nColId );
}

void OTableEditorCtrl::SetControlText( long nRow, sal_uInt16 nColId, const String& rText )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Set the Browser Controls
    if( nColId < FIELD_FIRST_VIRTUAL_COLUMN )
    {
        GoToRow( nRow );
        GoToColumnId( nColId );
        CellControllerRef xController = Controller();
        if(xController.Is())
            xController->GetWindow().SetText( rText );
        else
            RowModified(nRow,nColId);
    }

    // Set the Tabpage controls
    else
    {
        pDescrWin->SetControlText( nColId, rText );
    }
}

void OTableEditorCtrl::SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Relocate the current pointer
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    // Set individual fields
    switch( nColId )
    {
        case FIELD_TYPE:
            SwitchType( _pTypeInfo );
            break;
        default:
            OSL_FAIL("OTableEditorCtrl::SetCellData: invalid column!");
    }
    SetControlText(nRow,nColId,_pTypeInfo.get() ? _pTypeInfo->aUIName : OUString());
}

void OTableEditorCtrl::SetCellData( long nRow, sal_uInt16 nColId, const ::com::sun::star::uno::Any& _rNewData )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Relocate the current pointer
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    String sValue;
    // Set indvidual fields
    switch( nColId )
    {
        case FIELD_NAME:
            sValue = ::comphelper::getString(_rNewData);
            pFieldDescr->SetName( sValue );
            break;

        case FIELD_TYPE:
            OSL_FAIL("OTableEditorCtrl::SetCellData: invalid column!");
            break;

        case COLUMN_DESCRIPTION:
            pFieldDescr->SetDescription( sValue = ::comphelper::getString(_rNewData) );
            break;

        case FIELD_PROPERTY_DEFAULT:
            pFieldDescr->SetControlDefault( _rNewData );
            sValue = GetView()->GetDescWin()->getGenPage()->getFieldControl()->getControlDefault(pFieldDescr);
            break;

        case FIELD_PROPERTY_REQUIRED:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetIsNullable( sValue.ToInt32() );
            }
            break;

        case FIELD_PROPERTY_TEXTLEN:
        case FIELD_PROPERTY_LENGTH:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetPrecision( sValue.ToInt32() );
            }
            break;

        case FIELD_PROPERTY_NUMTYPE:
            OSL_FAIL("OTableEditorCtrl::SetCellData: invalid column!");
            break;

        case FIELD_PROPERTY_AUTOINC:
            {
                String strYes(ModuleRes(STR_VALUE_YES));
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetAutoIncrement(sValue.Equals(strYes));
            }
            break;
        case FIELD_PROPERTY_SCALE:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetScale(sValue.ToInt32());
            }
            break;

        case FIELD_PROPERTY_BOOL_DEFAULT:
            sValue = GetView()->GetDescWin()->BoolStringPersistent(::comphelper::getString(_rNewData));
            pFieldDescr->SetControlDefault(makeAny(OUString(sValue)));
            break;

        case FIELD_PROPERTY_FORMAT:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetFormatKey(sValue.ToInt32());
            }
            break;
    }

    SetControlText(nRow,nColId,sValue);
}

Any OTableEditorCtrl::GetCellData( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr )
        return Any();

    // Relocate the current pointer
    if( nRow==-1 )
        nRow = GetCurRow();
    SetDataPtr( nRow );

    static const String strYes(ModuleRes(STR_VALUE_YES));
    static const String strNo(ModuleRes(STR_VALUE_NO));
    OUString sValue;
    // Read out the fields
    switch( nColId )
    {
        case FIELD_NAME:
            sValue = pFieldDescr->GetName();
            break;

        case FIELD_TYPE:
            if ( pFieldDescr->getTypeInfo() )
                sValue = pFieldDescr->getTypeInfo()->aUIName;
            break;

        case COLUMN_DESCRIPTION:
            sValue = pFieldDescr->GetDescription();
            break;
        case HELP_TEXT:
            sValue = pFieldDescr->GetHelpText();
            break;

        case FIELD_PROPERTY_DEFAULT:
            return pFieldDescr->GetControlDefault();

        case FIELD_PROPERTY_REQUIRED:
            sValue = pFieldDescr->GetIsNullable() == ColumnValue::NULLABLE ? strYes : strNo;
            break;

        case FIELD_PROPERTY_TEXTLEN:
        case FIELD_PROPERTY_LENGTH:
            sValue = OUString::number(pFieldDescr->GetPrecision());
            break;

        case FIELD_PROPERTY_NUMTYPE:
            OSL_FAIL("OTableEditorCtrl::GetCellData: invalid column!");
            break;

        case FIELD_PROPERTY_AUTOINC:
            sValue = pFieldDescr->IsAutoIncrement() ? strYes : strNo;
            break;

        case FIELD_PROPERTY_SCALE:
            sValue = OUString::number(pFieldDescr->GetScale());
            break;

        case FIELD_PROPERTY_BOOL_DEFAULT:
            sValue = GetView()->GetDescWin()->BoolStringUI(::comphelper::getString(pFieldDescr->GetControlDefault()));
            break;

        case FIELD_PROPERTY_FORMAT:
            sValue = OUString::number(pFieldDescr->GetFormatKey());
            break;
    }

    return makeAny(sValue);
}

OUString OTableEditorCtrl::GetCellText( long nRow, sal_uInt16 nColId ) const
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OUString sCellText;
    const_cast< OTableEditorCtrl* >( this )->GetCellData( nRow, nColId ) >>= sCellText;
    return sCellText;
}

sal_uInt32 OTableEditorCtrl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    return GetTextWidth(GetCellText(nRow, nColId)) + 2 * GetTextWidth(OUString('0'));
}

OFieldDescription* OTableEditorCtrl::GetFieldDescr( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    std::vector< ::boost::shared_ptr<OTableRow> >::size_type nListCount(
        m_pRowList->size());
    if( (nRow<0) || (sal::static_int_cast< unsigned long >(nRow)>=nListCount) )
    {
        OSL_FAIL("(nRow<0) || (nRow>=nListCount)");
        return NULL;
    }
     ::boost::shared_ptr<OTableRow>  pRow = (*m_pRowList)[ nRow ];
    if( !pRow )
        return NULL;
    return pRow->GetActFieldDescr();
}

sal_Bool OTableEditorCtrl::IsCutAllowed( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    sal_Bool bIsCutAllowed = (GetView()->getController().isAddAllowed() && GetView()->getController().isDropAllowed()) ||
                            GetView()->getController().isAlterAllowed();

    if(bIsCutAllowed)
    {
        switch(m_eChildFocus)
        {
            case DESCRIPTION:
                bIsCutAllowed = !pDescrCell->GetSelected().isEmpty();
                break;
            case HELPTEXT:
                bIsCutAllowed = !pHelpTextCell->GetSelected().isEmpty();
                break;
            case NAME:
                bIsCutAllowed = !pNameCell->GetSelected().isEmpty();
                break;
            case ROW:
                bIsCutAllowed = IsCopyAllowed(nRow);
                break;
            default:
                bIsCutAllowed = sal_False;
                break;
        }
    }

    return bIsCutAllowed;
}

sal_Bool OTableEditorCtrl::IsCopyAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    sal_Bool bIsCopyAllowed = sal_False;
    if(m_eChildFocus == DESCRIPTION )
        bIsCopyAllowed = !pDescrCell->GetSelected().isEmpty();
    else if(HELPTEXT == m_eChildFocus )
        bIsCopyAllowed = !pHelpTextCell->GetSelected().isEmpty();
    else if(m_eChildFocus == NAME)
        bIsCopyAllowed = !pNameCell->GetSelected().isEmpty();
    else if(m_eChildFocus == ROW)
    {
        Reference<XPropertySet> xTable = GetView()->getController().getTable();
        if( !GetSelectRowCount() || (xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW"))
            return sal_False;

        // If one of the selected rows is empty, Copy is not possible
         ::boost::shared_ptr<OTableRow>  pRow;
        long nIndex = FirstSelectedRow();
        while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
        {
            pRow = (*m_pRowList)[nIndex];
            if( !pRow->GetActFieldDescr() )
                return sal_False;

            nIndex = NextSelectedRow();
        }

        bIsCopyAllowed = sal_True;
    }

    return bIsCopyAllowed;
}

sal_Bool OTableEditorCtrl::IsPasteAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    sal_Bool bAllowed = GetView()->getController().isAddAllowed();
    if ( bAllowed )
    {
        TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
        sal_Bool bRowFormat = aTransferData.HasFormat(SOT_FORMATSTR_ID_SBA_TABED);
        if ( m_eChildFocus == ROW )
            bAllowed = bRowFormat;
        else
            bAllowed = !bRowFormat && aTransferData.HasFormat(SOT_FORMAT_STRING);
    }

    return bAllowed;
}

void OTableEditorCtrl::cut()
{
    if(m_eChildFocus == NAME)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            SaveData(-1,FIELD_NAME);
            pNameCell->Cut();
            CellModified(-1,FIELD_NAME);
        }
    }
    else if(m_eChildFocus == DESCRIPTION)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            SaveData(-1,COLUMN_DESCRIPTION);
            pDescrCell->Cut();
            CellModified(-1,COLUMN_DESCRIPTION);
        }
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        if(GetView()->getController().isAlterAllowed())
        {
            SaveData(-1,HELP_TEXT);
            pHelpTextCell->Cut();
            CellModified(-1,HELP_TEXT);
        }
    }
    else if(m_eChildFocus == ROW)
    {
        if (nCutEvent)
            Application::RemoveUserEvent(nCutEvent);
        nCutEvent = Application::PostUserEvent(LINK(this, OTableEditorCtrl, DelayedCut));
    }
}

void OTableEditorCtrl::copy()
{
    if(GetSelectRowCount())
        OTableRowView::copy();
    else if(m_eChildFocus == NAME)
        pNameCell->Copy();
    else if(HELPTEXT == m_eChildFocus )
        pHelpTextCell->Copy();
    else if(m_eChildFocus == DESCRIPTION )
        pDescrCell->Copy();
}

void OTableEditorCtrl::paste()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(SOT_FORMATSTR_ID_SBA_TABED))
    {
        if( nPasteEvent )
            Application::RemoveUserEvent( nPasteEvent );
        nPasteEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedPaste) );
    }
    else if(m_eChildFocus == NAME)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            pNameCell->Paste();
            CellModified();
        }
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        if(GetView()->getController().isAlterAllowed())
        {
            pHelpTextCell->Paste();
            CellModified();
        }
    }
    else if(m_eChildFocus == DESCRIPTION)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            pDescrCell->Paste();
            CellModified();
        }
    }
}

sal_Bool OTableEditorCtrl::IsDeleteAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    return GetSelectRowCount() != 0 && GetView()->getController().isDropAllowed();
}

sal_Bool OTableEditorCtrl::IsInsertNewAllowed( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    sal_Bool bInsertNewAllowed = GetView()->getController().isAddAllowed();
    // If fields can be added, Paste in the new fields
    if (bInsertNewAllowed && !GetView()->getController().isDropAllowed())
    {
        SetDataPtr(nRow);
        if( GetActRow()->IsReadOnly() )
            return sal_False;
    }

    return bInsertNewAllowed;
}

sal_Bool OTableEditorCtrl::IsPrimaryKeyAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if( !GetSelectRowCount() )
        return sal_False;

    OTableController& rController = GetView()->getController();
    if ( !rController.getSdbMetaData().supportsPrimaryKeys() )
        return sal_False;

    Reference<XPropertySet> xTable = rController.getTable();
    // Key must not be changed
    // This applies only if the table is not new and not a  ::com::sun::star::sdbcx::View. Otherwise no DROP is executed

    if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW")
        return sal_False;
    // If there is an empty field, no primary key
    // The entry is only permitted if
    // - there are no empty entries in the selection
    // - No Memo or Image entries
    // - DROP is not permitted (see above) and the column is not Required (not null flag is not set).
    long nIndex = FirstSelectedRow();
     ::boost::shared_ptr<OTableRow>  pRow;
    while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
    {
        pRow = (*m_pRowList)[nIndex];
        OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
        if(!pFieldDescr)
            return sal_False;
        else
        {
            // Memo and Image fields cannot be primary keys
            // or if the columne cannot be dropped and the Required flag is not set
            // or if a ::com::sun::star::sdbcx::View is avalable and the Required flag is not set
            TOTypeInfoSP pTypeInfo = pFieldDescr->getTypeInfo();
            if(     pTypeInfo->nSearchType == ColumnSearch::NONE
                || (pFieldDescr->IsNullable() && pRow->IsReadOnly())
              )
                return sal_False;
        }

        nIndex = NextSelectedRow();
    }

    return sal_True;
}

void OTableEditorCtrl::Command(const CommandEvent& rEvt)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            Point aMenuPos( rEvt.GetMousePosPixel() );
            if (!rEvt.IsMouseEvent())
            {
                if  ( 1 == GetSelectColumnCount() )
                {
                    sal_uInt16 nSelId = GetColumnId(
                        sal::static_int_cast< sal_uInt16 >(
                            FirstSelectedColumn() ) );
                    ::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, sal_False ) );

                    aMenuPos = aColRect.TopCenter();
                }
                else if ( GetSelectRowCount() > 0 )
                {
                    ::Rectangle aColRect( GetFieldRectPixel( FirstSelectedRow(), HANDLE_ID, sal_True ) );

                    aMenuPos = aColRect.TopCenter();
                }
                else
                {
                    OTableRowView::Command(rEvt);
                    return;
                }
            }

            // Show the Context menu
            if( !IsReadOnly() )
            {
                sal_uInt16 nColId = GetColumnAtXPosPixel(aMenuPos.X());
                long   nRow = GetRowAtYPosPixel(aMenuPos.Y());

                if ( HANDLE_ID != nColId )
                {
                    if ( nRow < 0 && nColId != BROWSER_INVALIDID )
                    {   // hit the header
                        if ( 3 != nColId )
                        {   // 3 would mean the last column, and this last column is auto-sized
                            if ( !IsColumnSelected( nColId ) )
                                SelectColumnId( nColId );

                            PopupMenu aContextMenu( ModuleRes( RID_QUERYCOLPOPUPMENU ) );
                            aContextMenu.EnableItem( SID_DELETE, sal_False );
                            aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
                            switch ( aContextMenu.Execute( this, aMenuPos ) )
                            {
                                case ID_BROWSER_COLWIDTH:
                                    adjustBrowseBoxColumnWidth( this, nColId );
                                    break;
                            }
                        }
                    }
                }
                else
                {
                    PopupMenu aContextMenu(ModuleRes(RID_TABLEDESIGNROWPOPUPMENU));

                    aContextMenu.EnableItem( SID_CUT, IsCutAllowed(nRow) );
                    aContextMenu.EnableItem( SID_COPY, IsCopyAllowed(nRow) );
                    aContextMenu.EnableItem( SID_PASTE, IsPasteAllowed(nRow) );
                    aContextMenu.EnableItem( SID_DELETE, IsDeleteAllowed(nRow) );
                    aContextMenu.EnableItem( SID_TABLEDESIGN_TABED_PRIMARYKEY, IsPrimaryKeyAllowed(nRow) );
                    aContextMenu.EnableItem( SID_TABLEDESIGN_INSERTROWS, IsInsertNewAllowed(nRow) );
                    aContextMenu.CheckItem( SID_TABLEDESIGN_TABED_PRIMARYKEY, IsRowSelected(GetCurRow()) && IsPrimaryKey() );

                    // remove all the disable entries
                    aContextMenu.RemoveDisabledEntries(sal_True, sal_True);

                    if( SetDataPtr(m_nDataPos) )
                        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

                    // All actions which change the number of rows must be run asynchronously
                    // otherwise there may be problems between the Context menu and the Browser
                    m_nDataPos = GetCurRow();
                    switch (aContextMenu.Execute(this, aMenuPos))
                    {
                        case SID_CUT:
                            cut();
                            break;
                        case SID_COPY:
                            copy();
                            break;
                        case SID_PASTE:
                            paste();
                            break;
                        case SID_DELETE:
                            if( nDeleteEvent )
                                Application::RemoveUserEvent( nDeleteEvent );
                            nDeleteEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedDelete) );
                            break;
                        case SID_TABLEDESIGN_INSERTROWS:
                            if( nInsNewRowsEvent )
                                Application::RemoveUserEvent( nInsNewRowsEvent );
                            nInsNewRowsEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedInsNewRows) );
                            break;
                        case SID_TABLEDESIGN_TABED_PRIMARYKEY:
                            SetPrimaryKey( !IsPrimaryKey() );
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        break;
        default:
            OTableRowView::Command(rEvt);
    }

}

IMPL_LINK( OTableEditorCtrl, DelayedCut, void*, /*EMPTYTAG*/ )
{
    nCutEvent = 0;
    OTableRowView::cut();
    return 0;
}

IMPL_LINK( OTableEditorCtrl, DelayedPaste, void*, /*EMPTYTAG*/ )
{
    nPasteEvent = 0;

    sal_Int32 nPastePosition = GetView()->getController().getFirstEmptyRowPosition();
    if ( !GetView()->getController().getTable().is() )
        nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : GetCurRow();

    if (!IsInsertNewAllowed(nPastePosition))
    {   // Insertion is not allowed, only appending, so test if there are full cells after the PastePosition

        sal_Int32 nFreeFromPos; // from here on there are only empty rows
        ::std::vector< ::boost::shared_ptr<OTableRow> >::reverse_iterator aIter = m_pRowList->rbegin();
        for(nFreeFromPos = m_pRowList->size();
            aIter != m_pRowList->rend() && (!(*aIter) || !(*aIter)->GetActFieldDescr() || (*aIter)->GetActFieldDescr()->GetName().isEmpty());
            --nFreeFromPos, ++aIter)
            ;
        if (nPastePosition < nFreeFromPos)  // if at least one PastePosition is full, go right to the end
            nPastePosition = nFreeFromPos;
    }

    OTableRowView::Paste( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );

    return 0;
}

IMPL_LINK( OTableEditorCtrl, DelayedDelete, void*, /*EMPTYTAG*/ )
{
    nDeleteEvent = 0;
    DeleteRows();
    return 0;
}

IMPL_LINK( OTableEditorCtrl, DelayedInsNewRows, void*, /*EMPTYTAG*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    nInsNewRowsEvent = 0;
    sal_Int32 nPastePosition = GetView()->getController().getFirstEmptyRowPosition();
    if ( !GetView()->getController().getTable().is() )
        nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : m_nDataPos;

    InsertNewRows( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );

    return 0;
}

void OTableEditorCtrl::AdjustFieldDescription(OFieldDescription* _pFieldDesc,
                                         MultiSelection& _rMultiSel,
                                         sal_Int32 _nPos,
                                         sal_Bool _bSet,
                                         sal_Bool _bPrimaryKey)
{
    _pFieldDesc->SetPrimaryKey( _bPrimaryKey );
    if(!_bSet && _pFieldDesc->getTypeInfo()->bNullable)
    {
        _pFieldDesc->SetIsNullable(ColumnValue::NO_NULLS);
        _pFieldDesc->SetControlDefault(Any());
    }
    if ( _pFieldDesc->IsAutoIncrement() && !_bPrimaryKey )
    {
        OTableController& rController = GetView()->getController();
        if ( rController.isAutoIncrementPrimaryKey() )
        {
            _pFieldDesc->SetAutoIncrement(false);
        }
    }
    // update field description
    pDescrWin->DisplayData(_pFieldDesc);

    _rMultiSel.Insert( _nPos );
    _rMultiSel.Select( _nPos );
}

void OTableEditorCtrl::SetPrimaryKey( sal_Bool bSet )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Delete any existing Primary Keys
    MultiSelection aDeletedPrimKeys;
    aDeletedPrimKeys.SetTotalRange( Range(0,GetRowCount()) );

    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = m_pRowList->begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = m_pRowList->end();
    for(sal_Int32 nRow = 0;aIter != aEnd;++aIter,++nRow)
    {
        OFieldDescription* pFieldDescr = (*aIter)->GetActFieldDescr();
        if( pFieldDescr && (*aIter)->IsPrimaryKey() && (!bSet || !IsRowSelected(nRow)) )
        {
            AdjustFieldDescription(pFieldDescr,aDeletedPrimKeys,nRow,bSet,sal_False);
        }
    }

    // Set the primary keys of the marked rows
    MultiSelection aInsertedPrimKeys;
    aInsertedPrimKeys.SetTotalRange( Range(0,GetRowCount()) );
    if( bSet )
    {
        long nIndex = FirstSelectedRow();
        while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
        {
            // Set the key
             ::boost::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nIndex];
            OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
            if(pFieldDescr)
                AdjustFieldDescription(pFieldDescr,aInsertedPrimKeys,nIndex,sal_False,sal_True);

            nIndex = NextSelectedRow();
        }
    }

    GetUndoManager().AddUndoAction( new OPrimKeyUndoAct(this, aDeletedPrimKeys, aInsertedPrimKeys) );

    // Invalidate the handle-columns
    InvalidateHandleColumn();

    // Set the TableDocSh's ModifyFlag
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

sal_Bool OTableEditorCtrl::IsPrimaryKey()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // Are all marked fields part of the Primary Key ?
    long nPrimaryKeys = 0;
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = m_pRowList->begin();
    ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = m_pRowList->end();
    for(sal_Int32 nRow=0;aIter != aEnd;++aIter,++nRow)
    {
        if( IsRowSelected(nRow) && !(*aIter)->IsPrimaryKey() )
            return sal_False;
        if( (*aIter)->IsPrimaryKey() )
            ++nPrimaryKeys;
    }

    // Are there any unselected fields that are part of the Key ?
    return GetSelectRowCount() == nPrimaryKeys;
}

void OTableEditorCtrl::SwitchType( const TOTypeInfoSP& _pType )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    // if there is no assigned field name
    long nRow(GetCurRow());
    OFieldDescription* pActFieldDescr = GetFieldDescr( nRow );
    if( pActFieldDescr )
        // Store the old description
        pDescrWin->SaveData( pActFieldDescr );

    if ( nRow < 0 || nRow > static_cast<long>(m_pRowList->size()) )
        return;
    // Show the new description
     ::boost::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nRow];
    pRow->SetFieldType( _pType, sal_True );
    if ( _pType.get() )
    {
        const sal_uInt16 nCurrentlySelected = pTypeCell->GetSelectEntryPos();

        if  (   ( LISTBOX_ENTRY_NOTFOUND == nCurrentlySelected )
            ||  ( GetView()->getController().getTypeInfo( nCurrentlySelected ) != _pType )
            )
        {
            sal_uInt16 nEntryPos = 0;
            const OTypeInfoMap* pTypeInfo = GetView()->getController().getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pTypeInfo->begin();
            OTypeInfoMap::const_iterator aEnd = pTypeInfo->end();
            for(;aIter != aEnd;++aIter,++nEntryPos)
            {
                if(aIter->second == _pType)
                    break;
            }
            if (nEntryPos < pTypeCell->GetEntryCount())
                pTypeCell->SelectEntryPos( nEntryPos );
        }
    }

    pActFieldDescr = pRow->GetActFieldDescr();
    if (pActFieldDescr != NULL && !pActFieldDescr->GetFormatKey())
    {
        sal_Int32 nFormatKey = ::dbtools::getDefaultNumberFormat( pActFieldDescr->GetType(),
            pActFieldDescr->GetScale(),
            pActFieldDescr->IsCurrency(),
            Reference< XNumberFormatTypes>(GetView()->getController().getNumberFormatter()->getNumberFormatsSupplier()->getNumberFormats(),UNO_QUERY),
            GetView()->getLocale());

        pActFieldDescr->SetFormatKey(nFormatKey);
    }

    pDescrWin->DisplayData( pActFieldDescr );
}

OTableDesignView* OTableEditorCtrl::GetView() const
{
    return static_cast<OTableDesignView*>(GetParent()->GetParent());
}

void OTableEditorCtrl::DeactivateCell(sal_Bool bUpdate)
{
    OTableRowView::DeactivateCell(bUpdate);
    // now we have to deactivate the field description
    long nRow(GetCurRow());
    if (pDescrWin)
        pDescrWin->SetReadOnly(bReadOnly || !SetDataPtr(nRow) || GetActRow()->IsReadOnly());
}

long OTableEditorCtrl::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == EVENT_GETFOCUS)
    {
        if( pHelpTextCell && pHelpTextCell->HasChildPathFocus() )
            m_eChildFocus = HELPTEXT;
        else if( pDescrCell && pDescrCell->HasChildPathFocus() )
            m_eChildFocus = DESCRIPTION;
        else if(pNameCell && pNameCell->HasChildPathFocus() )
            m_eChildFocus = NAME;
        else
            m_eChildFocus = ROW;
    }

    return OTableRowView::PreNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
