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
#include <connectivity/dbmetadata.hxx>
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


#define HANDLE_ID       0

// default field widths
#define FIELDNAME_WIDTH     100
#define FIELDTYPE_WIDTH     150
#define FIELDDESCR_WIDTH    300

// Maximum length in description field
#define MAX_DESCR_LEN       256

OTableEditorCtrl::ClipboardInvalidator::ClipboardInvalidator(OTableEditorCtrl* _pOwner)
: m_pOwner(_pOwner)
{

    m_aInvalidateTimer.SetTimeout(500);
    m_aInvalidateTimer.SetInvokeHandler(LINK(this, OTableEditorCtrl::ClipboardInvalidator, OnInvalidate));
    m_aInvalidateTimer.Start();
}

OTableEditorCtrl::ClipboardInvalidator::~ClipboardInvalidator()
{
    m_aInvalidateTimer.Stop();
}

void OTableEditorCtrl::ClipboardInvalidator::Stop()
{
    m_aInvalidateTimer.Stop();
}

IMPL_LINK_NOARG(OTableEditorCtrl::ClipboardInvalidator, OnInvalidate, Timer *, void)
{
    m_pOwner->GetView()->getController().InvalidateFeature(SID_CUT);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_COPY);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_PASTE);
}

void OTableEditorCtrl::Init()
{
    OTableRowView::Init();

    // Should it be opened ReadOnly?
    bool bRead(GetView()->getController().isReadOnly());

    SetReadOnly( bRead );

    // Insert the columns
    InsertDataColumn( FIELD_NAME, ModuleRes(STR_TAB_FIELD_COLUMN_NAME), FIELDNAME_WIDTH );

    InsertDataColumn( FIELD_TYPE, ModuleRes(STR_TAB_FIELD_COLUMN_DATATYPE), FIELDTYPE_WIDTH );

    ::dbaccess::ODsnTypeCollection aDsnTypes(GetView()->getController().getORB());
    bool bShowColumnDescription = aDsnTypes.supportsColumnDescription(::comphelper::getString(GetView()->getController().getDataSource()->getPropertyValue(PROPERTY_URL)));
    InsertDataColumn( HELP_TEXT, ModuleRes(STR_TAB_HELP_TEXT), bShowColumnDescription ? FIELDTYPE_WIDTH : FIELDDESCR_WIDTH );

    if ( bShowColumnDescription )
    {
        InsertDataColumn( COLUMN_DESCRIPTION, ModuleRes(STR_COLUMN_DESCRIPTION), FIELDTYPE_WIDTH );
    }

    InitCellController();

    // Insert the rows
    RowInserted(0, m_pRowList->size());
}

OTableEditorCtrl::OTableEditorCtrl(vcl::Window* pWindow)
    :OTableRowView(pWindow)
    ,pNameCell(nullptr)
    ,pTypeCell(nullptr)
    ,pHelpTextCell(nullptr)
    ,pDescrCell(nullptr)
    ,pDescrWin(nullptr)
    ,nCutEvent(nullptr)
    ,nPasteEvent(nullptr)
    ,nDeleteEvent(nullptr)
    ,nInsNewRowsEvent(nullptr)
    ,nInvalidateTypeEvent(nullptr)
    ,m_eChildFocus(NONE)
    ,nOldDataPos(-1)
    ,bSaveOnMove(true)
    ,bReadOnly(true)
    ,m_aInvalidate(this)
{

    SetHelpId(HID_TABDESIGN_BACKGROUND);
    GetDataWindow().SetHelpId(HID_CTL_TABLEEDIT);

    m_pRowList = &GetView()->getController().getRows();
    m_nDataPos = 0;
}

SfxUndoManager& OTableEditorCtrl::GetUndoManager() const
{
    return GetView()->getController().GetUndoManager();
}


void OTableEditorCtrl::SetReadOnly( bool bRead )
{
    // nothing to do?
    if (bRead == IsReadOnly())
        // This check is important, as the underlying Def may be unnecessarily locked or unlocked
        // or worse, this action may not be reversed afterwards
        return;

    bReadOnly = bRead;

    // Disable active cells
    long nRow(GetCurRow());
    sal_uInt16 nCol(GetCurColumnId());
    DeactivateCell();

    // Select the correct Browsers cursor
    BrowserMode nMode(BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::KEEPHIGHLIGHT |
                      BrowserMode::HLINES      | BrowserMode::VLINES|BrowserMode::AUTOSIZE_LASTCOL);
    if( !bReadOnly )
        nMode |= BrowserMode::HIDECURSOR;
    SetMode(nMode);

    if( !bReadOnly )
        ActivateCell( nRow, nCol );
}

void OTableEditorCtrl::InitCellController()
{
    // Cell Field name
    sal_Int32 nMaxTextLen = EDIT_NOLIMIT;
    OUString sExtraNameChars;
    Reference<XConnection> xCon;
    try
    {
        xCon = GetView()->getController().getConnection();
        Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

        nMaxTextLen = xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0;

        if( nMaxTextLen == 0 )
            nMaxTextLen = EDIT_NOLIMIT;
        sExtraNameChars = xMetaData.is() ? xMetaData->getExtraNameCharacters() : OUString();

    }
    catch(SQLException&)
    {
        OSL_FAIL("getMaxColumnNameLength");
    }

    pNameCell = VclPtr<OSQLNameEdit>::Create(&GetDataWindow(), WB_LEFT, sExtraNameChars);
    pNameCell->SetMaxTextLen( nMaxTextLen );
    pNameCell->setCheck( isSQL92CheckEnabled(xCon) );

    // Cell type
    pTypeCell = VclPtr<ListBoxControl>::Create( &GetDataWindow() );
    pTypeCell->SetDropDownLineCount( 15 );

    // Cell description
    pDescrCell = VclPtr<Edit>::Create( &GetDataWindow(), WB_LEFT );
    pDescrCell->SetMaxTextLen( MAX_DESCR_LEN );

    pHelpTextCell = VclPtr<Edit>::Create( &GetDataWindow(), WB_LEFT );
    pHelpTextCell->SetMaxTextLen( MAX_DESCR_LEN );

    pNameCell->SetHelpId(HID_TABDESIGN_NAMECELL);
    pTypeCell->SetHelpId(HID_TABDESIGN_TYPECELL);
    pDescrCell->SetHelpId(HID_TABDESIGN_COMMENTCELL);
    pHelpTextCell->SetHelpId(HID_TABDESIGN_HELPTEXT);

    Size aHeight;
    const Control* pControls[] = { pTypeCell,pDescrCell,pNameCell,pHelpTextCell};
    for(const Control* pControl : pControls)
    {
        const Size aTemp(pControl->GetOptimalSize());
        if ( aTemp.Height() > aHeight.Height() )
            aHeight.Height() = aTemp.Height();
    }
    SetDataRowHeight(aHeight.Height());

    ClearModified();
}

void OTableEditorCtrl::ClearModified()
{
    pNameCell->ClearModifyFlag();
    pDescrCell->ClearModifyFlag();
    pHelpTextCell->ClearModifyFlag();
    pTypeCell->SaveValue();
}

OTableEditorCtrl::~OTableEditorCtrl()
{
    disposeOnce();
}

void OTableEditorCtrl::dispose()
{
    // Reset the Undo-Manager
    GetUndoManager().Clear();

    m_aInvalidate.Stop();

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
    pNameCell.disposeAndClear();
    pTypeCell.disposeAndClear();
    pDescrCell.disposeAndClear();
    pHelpTextCell.disposeAndClear();
    pDescrWin.clear();
    OTableRowView::dispose();
}

bool OTableEditorCtrl::SetDataPtr( long nRow )
{
    if(nRow == -1)
        return false;

    OSL_ENSURE(nRow < (long)m_pRowList->size(),"Row is greater than size!");
    if(nRow >= (long)m_pRowList->size())
        return false;
    pActRow = (*m_pRowList)[nRow];
    return pActRow != nullptr;
}

bool OTableEditorCtrl::SeekRow(long _nRow)
{
    // Call the Base class to remember which row must be repainted
    EditBrowseBox::SeekRow(_nRow);

    m_nCurrentPos = _nRow;
    return SetDataPtr(_nRow);
}

void OTableEditorCtrl::PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                   sal_uInt16 nColumnId ) const
{
    const OUString aText( GetCellText( m_nCurrentPos, nColumnId ));

    rDev.Push( PushFlags::CLIPREGION );
    rDev.SetClipRegion(vcl::Region(rRect));
    rDev.DrawText( rRect, aText, DrawTextFlags::Left | DrawTextFlags::VCenter );
    rDev.Pop();
}

CellController* OTableEditorCtrl::GetController(long nRow, sal_uInt16 nColumnId)
{
    // If EditorCtrl is ReadOnly, editing is forbidden
    Reference<XPropertySet> xTable = GetView()->getController().getTable();
    if (IsReadOnly() || (   xTable.is() &&
                            xTable->getPropertySetInfo()->hasPropertyByName(PROPERTY_TYPE) &&
                            ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW"))
        return nullptr;

    // If the row is ReadOnly, editing is forbidden
    SetDataPtr( nRow );
    if( pActRow->IsReadOnly() )
        return nullptr;

    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();
    switch (nColumnId)
    {
        case FIELD_NAME:
            return new EditCellController( pNameCell );
        case FIELD_TYPE:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new ListBoxCellController( pTypeCell );
            else return nullptr;
        case HELP_TEXT:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new EditCellController( pHelpTextCell );
            else
                return nullptr;
        case COLUMN_DESCRIPTION:
            if (pActFieldDescr && !pActFieldDescr->GetName().isEmpty())
                return new EditCellController( pDescrCell );
            else
                return nullptr;
        default:
            return nullptr;
    }
}

void OTableEditorCtrl::InitController(CellControllerRef&, long nRow, sal_uInt16 nColumnId)
{
    SeekRow( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();
    OUString aInitString;

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

                const OTypeInfoMap& rTypeInfo = GetView()->getController().getTypeInfo();
                OTypeInfoMap::const_iterator aIter = rTypeInfo.begin();
                OTypeInfoMap::const_iterator aEnd = rTypeInfo.end();
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

void OTableEditorCtrl::SaveCurRow()
{
    if (GetFieldDescr(GetCurRow()) == nullptr)
        // there is no data in the current row
        return;
    if (!SaveModified())
        return;

    SetDataPtr(GetCurRow());
    pDescrWin->SaveData( pActRow->GetActFieldDescr() );
}

void OTableEditorCtrl::DisplayData(long nRow)
{
    // go to the correct cell
    SetDataPtr(nRow);

    // Disable Edit-Mode temporarily
    bool bWasEditing = IsEditing();
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
    ActivateCell(nRow, GetCurColumnId());
}

void OTableEditorCtrl::CursorMoved()
{
    // New line?
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

sal_Int32 OTableEditorCtrl::HasFieldName( const OUString& rFieldName )
{

    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    ::comphelper::UStringMixEqual bCase(!xMetaData.is() || xMetaData->supportsMixedCaseQuotedIdentifiers());

    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aIter = m_pRowList->begin();
    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aEnd = m_pRowList->end();
    sal_Int32 nCount(0);
    for(;aIter != aEnd;++aIter)
    {
        OFieldDescription* pFieldDescr = (*aIter)->GetActFieldDescr();
        if( pFieldDescr && bCase(rFieldName,pFieldDescr->GetName()))
            nCount++;
    }
    return nCount;
}

void OTableEditorCtrl::SaveData(long nRow, sal_uInt16 nColId)
{
    // Store the cell content
    SetDataPtr( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    switch( nColId)
    {
        // Store NameCell
        case FIELD_NAME:
        {
            // If there is no name, do nothing
            const OUString aName(pNameCell->GetText());

            if( aName.isEmpty() )
            {
                // If FieldDescr exists, the field is deleted and the old content restored
                if (pActFieldDescr)
                {
                    GetUndoManager().AddUndoAction(new OTableEditorTypeSelUndoAct(this, nRow, FIELD_TYPE, pActFieldDescr->getTypeInfo()));
                    SwitchType(TOTypeInfoSP());
                    pActFieldDescr = pActRow->GetActFieldDescr();
                }
                else
                    return;
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
            // if the current field description is NULL, set Default
            if( !pActFieldDescr )
            {
                pHelpTextCell->SetText(OUString());
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
                pDescrCell->SetText(OUString());
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
}

bool OTableEditorCtrl::SaveModified()
{
    sal_uInt16 nColId = GetCurColumnId();

    switch( nColId )
    {
        // Field type
        case FIELD_TYPE:
        {
            // Reset the type
            resetType();
        } break;
    }

    return true;
}

bool OTableEditorCtrl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{

    if (!EditBrowseBox::CursorMoving(nNewRow, nNewCol))
        return false;

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

    return true;
}

IMPL_LINK_NOARG( OTableEditorCtrl, InvalidateFieldType, void*, void )
{
    nInvalidateTypeEvent = nullptr;
    Invalidate( GetFieldRectPixel(nOldDataPos, FIELD_TYPE) );
}

void OTableEditorCtrl::CellModified( long nRow, sal_uInt16 nColId )
{

    // If the description is null, use the default
    if(nRow == -1)
        nRow = GetCurRow();
    SetDataPtr( nRow );
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    OUString sActionDescription;
    switch ( nColId )
    {
    case FIELD_NAME:    sActionDescription = ModuleRes( STR_CHANGE_COLUMN_NAME ); break;
    case FIELD_TYPE:    sActionDescription = ModuleRes( STR_CHANGE_COLUMN_TYPE ); break;
    case HELP_TEXT:
    case COLUMN_DESCRIPTION:   sActionDescription = ModuleRes( STR_CHANGE_COLUMN_DESCRIPTION ); break;
    default:            sActionDescription = ModuleRes( STR_CHANGE_COLUMN_ATTRIBUTE ); break;
    }

    GetUndoManager().EnterListAction( sActionDescription, OUString(),0,-1 );
    if (!pActFieldDescr)
    {
        const OTypeInfoMap& rTypeInfoMap = GetView()->getController().getTypeInfo();
        if ( !rTypeInfoMap.empty() )
        {
            OTypeInfoMap::const_iterator aTypeIter = rTypeInfoMap.find(DataType::VARCHAR);
            if ( aTypeIter == rTypeInfoMap.end() )
                aTypeIter = rTypeInfoMap.begin();
            pActRow->SetFieldType( aTypeIter->second );
        }
        else
            pActRow->SetFieldType( GetView()->getController().getTypeInfoFallBack() );

        nInvalidateTypeEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, InvalidateFieldType), nullptr, true );
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
    if(xController.is())
        xController->SetModified();

    // Set the Modify flag
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::resetType()
{
    sal_Int32 nPos = pTypeCell->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        SwitchType( GetView()->getController().getTypeInfo(nPos) );
    else
        SwitchType(TOTypeInfoSP());
}

void OTableEditorCtrl::CellModified()
{
    CellModified( GetCurRow(), GetCurColumnId() );
}

void OTableEditorCtrl::InvalidateFeatures()
{
    GetView()->getController().InvalidateFeature(SID_UNDO);
    GetView()->getController().InvalidateFeature(SID_REDO);
    GetView()->getController().InvalidateFeature(SID_SAVEDOC);
}

void OTableEditorCtrl::CopyRows()
{
    // set to the right row and save it
    if( SetDataPtr(m_nDataPos) )
        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

    // Copy selected rows to the ClipboardList
     std::shared_ptr<OTableRow>  pClipboardRow;
     std::shared_ptr<OTableRow>  pRow;
    ::std::vector< std::shared_ptr<OTableRow> > vClipboardList;
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
        Reference< css::datatransfer::XTransferable> xRef = pData;
        pData->CopyToClipboard(GetParent());
    }
}

OUString OTableEditorCtrl::GenerateName( const OUString& rName )
{
    // Create a base name for appending numbers to
    OUString aBaseName;
    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    sal_Int32 nMaxTextLen(xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0);

    if( (rName.getLength()+2) >nMaxTextLen )
        aBaseName = rName.copy( 0, nMaxTextLen-2 );
    else
        aBaseName = rName;

    // append a sequential number to the base name (up to 99)
    OUString aFieldName( rName);
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

    ::std::vector<  std::shared_ptr<OTableRow> > vInsertedUndoRedoRows; // need for undo/redo handling
    // get rows from clipboard
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(SotClipboardFormatId::SBA_TABED))
    {
        ::tools::SvRef<SotStorageStream> aStreamRef;
        bool bOk = aTransferData.GetSotStorageStream(SotClipboardFormatId::SBA_TABED,aStreamRef);
        if (bOk && aStreamRef.is())
        {
            aStreamRef->Seek(STREAM_SEEK_TO_BEGIN);
            aStreamRef->ResetError();
            long nInsertRow = nRow;
             std::shared_ptr<OTableRow>  pRow;
            sal_Int32 nSize = 0;
            (*aStreamRef).ReadInt32( nSize );
            vInsertedUndoRedoRows.reserve(nSize);
            for(sal_Int32 i=0;i < nSize;++i)
            {
                pRow.reset(new OTableRow());
                ReadOTableRow( *aStreamRef, *pRow );
                pRow->SetReadOnly( false );
                sal_Int32 nType = pRow->GetActFieldDescr()->GetType();
                if ( pRow->GetActFieldDescr() )
                    pRow->GetActFieldDescr()->SetType(GetView()->getController().getTypeInfoByType(nType));
                // Adjust the field names
                pRow->GetActFieldDescr()->SetName( GenerateName( pRow->GetActFieldDescr()->GetName() ) );
                pRow->SetPos(nInsertRow);
                m_pRowList->insert( m_pRowList->begin()+nInsertRow,pRow );
                vInsertedUndoRedoRows.push_back(std::make_shared<OTableRow>(*pRow));
                nInsertRow++;
            }
        }
    }
    // RowInserted calls CursorMoved.
    // The UI data should not be stored here.
    bSaveOnMove = false;
    RowInserted( nRow,vInsertedUndoRedoRows.size() );
    bSaveOnMove = true;

    // Create the Undo-Action
    GetUndoManager().AddUndoAction( new OTableEditorInsUndoAct(this, nRow,vInsertedUndoRedoRows) );
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::DeleteRows()
{
    OSL_ENSURE(GetView()->getController().isDropAllowed(),"Call of DeleteRows not valid here. Please check isDropAllowed!");
    // Create the Undo-Action
    GetUndoManager().AddUndoAction( new OTableEditorDelUndoAct(this) );

    // Delete all marked rows
    long nIndex = FirstSelectedRow();
    nOldDataPos = nIndex;
    bSaveOnMove = false;

    while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
    {
        // Remove rows
        m_pRowList->erase( m_pRowList->begin()+nIndex );
        RowRemoved( nIndex );

        // Insert the empty row at the end
        m_pRowList->push_back( std::make_shared<OTableRow>());
        RowInserted( GetRowCount()-1 );

        nIndex = FirstSelectedRow();
    }

    bSaveOnMove = true;

    // Force the current record to be displayed
    m_nDataPos = GetCurRow();
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );
    SetDataPtr( m_nDataPos );
    ActivateCell();
    pDescrWin->DisplayData( pActRow->GetActFieldDescr() );
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::InsertNewRows( long nRow )
{
    OSL_ENSURE(GetView()->getController().isAddAllowed(),"Call of InsertNewRows not valid here. Please check isAppendAllowed!");
    // Create Undo-Action
    long nInsertRows = GetSelectRowCount();
    if( !nInsertRows )
        nInsertRows = 1;
    GetUndoManager().AddUndoAction( new OTableEditorInsNewUndoAct(this, nRow, nInsertRows) );
    // Insert the number of selected rows
    for( long i=nRow; i<(nRow+nInsertRows); i++ )
        m_pRowList->insert( m_pRowList->begin()+i ,std::make_shared<OTableRow>());
    RowInserted( nRow, nInsertRows );

    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::SetControlText( long nRow, sal_uInt16 nColId, const OUString& rText )
{
    // Set the Browser Controls
    if( nColId < FIELD_FIRST_VIRTUAL_COLUMN )
    {
        GoToRow( nRow );
        GoToColumnId( nColId );
        CellControllerRef xController = Controller();
        if(xController.is())
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

void OTableEditorCtrl::SetCellData( long nRow, sal_uInt16 nColId, const css::uno::Any& _rNewData )
{
    // Relocate the current pointer
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    OUString sValue;
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
                pFieldDescr->SetIsNullable( sValue.toInt32() );
            }
            break;

        case FIELD_PROPERTY_TEXTLEN:
        case FIELD_PROPERTY_LENGTH:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetPrecision( sValue.toInt32() );
            }
            break;

        case FIELD_PROPERTY_NUMTYPE:
            OSL_FAIL("OTableEditorCtrl::SetCellData: invalid column!");
            break;

        case FIELD_PROPERTY_AUTOINC:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetAutoIncrement(sValue == ModuleRes(STR_VALUE_YES).toString());
            }
            break;
        case FIELD_PROPERTY_SCALE:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetScale(sValue.toInt32());
            }
            break;

        case FIELD_PROPERTY_BOOL_DEFAULT:
            sValue = GetView()->GetDescWin()->BoolStringPersistent(::comphelper::getString(_rNewData));
            pFieldDescr->SetControlDefault(makeAny(sValue));
            break;

        case FIELD_PROPERTY_FORMAT:
            {
                sValue = ::comphelper::getString(_rNewData);
                pFieldDescr->SetFormatKey(sValue.toInt32());
            }
            break;
    }

    SetControlText(nRow,nColId,sValue);
}

Any OTableEditorCtrl::GetCellData( long nRow, sal_uInt16 nColId )
{
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr )
        return Any();

    // Relocate the current pointer
    if( nRow==-1 )
        nRow = GetCurRow();
    SetDataPtr( nRow );

    static const OUString strYes(ModuleRes(STR_VALUE_YES));
    static const OUString strNo(ModuleRes(STR_VALUE_NO));
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
    OUString sCellText;
    const_cast< OTableEditorCtrl* >( this )->GetCellData( nRow, nColId ) >>= sCellText;
    return sCellText;
}

sal_uInt32 OTableEditorCtrl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
{
    return GetTextWidth(GetCellText(nRow, nColId)) + 2 * GetTextWidth("0");
}

OFieldDescription* OTableEditorCtrl::GetFieldDescr( long nRow )
{
    std::vector< std::shared_ptr<OTableRow> >::size_type nListCount(
        m_pRowList->size());
    if( (nRow<0) || (sal::static_int_cast< unsigned long >(nRow)>=nListCount) )
    {
        OSL_FAIL("(nRow<0) || (nRow>=nListCount)");
        return nullptr;
    }
     std::shared_ptr<OTableRow>  pRow = (*m_pRowList)[ nRow ];
    if( !pRow )
        return nullptr;
    return pRow->GetActFieldDescr();
}

bool OTableEditorCtrl::IsCutAllowed( long nRow )
{
    bool bIsCutAllowed = (GetView()->getController().isAddAllowed() && GetView()->getController().isDropAllowed()) ||
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
                bIsCutAllowed = false;
                break;
        }
    }

    return bIsCutAllowed;
}

bool OTableEditorCtrl::IsCopyAllowed( long /*nRow*/ )
{
    bool bIsCopyAllowed = false;
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
            return false;

        // If one of the selected rows is empty, Copy is not possible
         std::shared_ptr<OTableRow>  pRow;
        long nIndex = FirstSelectedRow();
        while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
        {
            pRow = (*m_pRowList)[nIndex];
            if( !pRow->GetActFieldDescr() )
                return false;

            nIndex = NextSelectedRow();
        }

        bIsCopyAllowed = true;
    }

    return bIsCopyAllowed;
}

bool OTableEditorCtrl::IsPasteAllowed( long /*nRow*/ )
{
    bool bAllowed = GetView()->getController().isAddAllowed();
    if ( bAllowed )
    {
        TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
        bool bRowFormat = aTransferData.HasFormat(SotClipboardFormatId::SBA_TABED);
        if ( m_eChildFocus == ROW )
            bAllowed = bRowFormat;
        else
            bAllowed = !bRowFormat && aTransferData.HasFormat(SotClipboardFormatId::STRING);
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
        nCutEvent = Application::PostUserEvent(LINK(this, OTableEditorCtrl, DelayedCut), nullptr, true);
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
    if(aTransferData.HasFormat(SotClipboardFormatId::SBA_TABED))
    {
        if( nPasteEvent )
            Application::RemoveUserEvent( nPasteEvent );
        nPasteEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedPaste), nullptr, true );
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

bool OTableEditorCtrl::IsDeleteAllowed( long /*nRow*/ )
{

    return GetSelectRowCount() != 0 && GetView()->getController().isDropAllowed();
}

bool OTableEditorCtrl::IsInsertNewAllowed( long nRow )
{

    bool bInsertNewAllowed = GetView()->getController().isAddAllowed();
    // If fields can be added, Paste in the new fields
    if (bInsertNewAllowed && !GetView()->getController().isDropAllowed())
    {
        SetDataPtr(nRow);
        if( GetActRow()->IsReadOnly() )
            return false;
    }

    return bInsertNewAllowed;
}

bool OTableEditorCtrl::IsPrimaryKeyAllowed( long /*nRow*/ )
{
    if( !GetSelectRowCount() )
        return false;

    OTableController& rController = GetView()->getController();
    if ( !rController.getSdbMetaData().supportsPrimaryKeys() )
        return false;

    Reference<XPropertySet> xTable = rController.getTable();
    // Key must not be changed
    // This applies only if the table is not new and not a  css::sdbcx::View. Otherwise no DROP is executed

    if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW")
        return false;
    // If there is an empty field, no primary key
    // The entry is only permitted if
    // - there are no empty entries in the selection
    // - No Memo or Image entries
    // - DROP is not permitted (see above) and the column is not Required (not null flag is not set).
    long nIndex = FirstSelectedRow();
     std::shared_ptr<OTableRow>  pRow;
    while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
    {
        pRow = (*m_pRowList)[nIndex];
        OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
        if(!pFieldDescr)
            return false;
        else
        {
            // Memo and Image fields cannot be primary keys
            // or if the column cannot be dropped and the Required flag is not set
            // or if a css::sdbcx::View is available and the Required flag is not set
            TOTypeInfoSP pTypeInfo = pFieldDescr->getTypeInfo();
            if(     pTypeInfo->nSearchType == ColumnSearch::NONE
                || (pFieldDescr->IsNullable() && pRow->IsReadOnly())
              )
                return false;
        }

        nIndex = NextSelectedRow();
    }

    return true;
}

void OTableEditorCtrl::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            Point aMenuPos( rEvt.GetMousePosPixel() );
            if (!rEvt.IsMouseEvent())
            {
                if  ( 1 == GetSelectColumnCount() )
                {
                    sal_uInt16 nSelId = GetColumnId(
                        sal::static_int_cast< sal_uInt16 >(
                            FirstSelectedColumn() ) );
                    ::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, false ) );

                    aMenuPos = aColRect.TopCenter();
                }
                else if ( GetSelectRowCount() > 0 )
                {
                    ::Rectangle aColRect( GetFieldRectPixel( FirstSelectedRow(), HANDLE_ID ) );

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

                            ScopedVclPtrInstance<PopupMenu> aContextMenu( ModuleRes( RID_QUERYCOLPOPUPMENU ) );
                            aContextMenu->EnableItem( SID_DELETE, false );
                            aContextMenu->RemoveDisabledEntries(true, true);
                            switch ( aContextMenu->Execute( this, aMenuPos ) )
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
                    ScopedVclPtrInstance<PopupMenu> aContextMenu(ModuleRes(RID_TABLEDESIGNROWPOPUPMENU));

                    aContextMenu->EnableItem( SID_CUT, IsCutAllowed(nRow) );
                    aContextMenu->EnableItem( SID_COPY, IsCopyAllowed(nRow) );
                    aContextMenu->EnableItem( SID_PASTE, IsPasteAllowed(nRow) );
                    aContextMenu->EnableItem( SID_DELETE, IsDeleteAllowed(nRow) );
                    aContextMenu->EnableItem( SID_TABLEDESIGN_TABED_PRIMARYKEY, IsPrimaryKeyAllowed(nRow) );
                    aContextMenu->EnableItem( SID_TABLEDESIGN_INSERTROWS, IsInsertNewAllowed(nRow) );
                    aContextMenu->CheckItem( SID_TABLEDESIGN_TABED_PRIMARYKEY, IsRowSelected(GetCurRow()) && IsPrimaryKey() );

                    // remove all the disable entries
                    aContextMenu->RemoveDisabledEntries(true, true);

                    if( SetDataPtr(m_nDataPos) )
                        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

                    // All actions which change the number of rows must be run asynchronously
                    // otherwise there may be problems between the Context menu and the Browser
                    m_nDataPos = GetCurRow();
                    switch (aContextMenu->Execute(this, aMenuPos))
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
                            nDeleteEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedDelete), nullptr, true );
                            break;
                        case SID_TABLEDESIGN_INSERTROWS:
                            if( nInsNewRowsEvent )
                                Application::RemoveUserEvent( nInsNewRowsEvent );
                            nInsNewRowsEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedInsNewRows), nullptr, true );
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

IMPL_LINK_NOARG( OTableEditorCtrl, DelayedCut, void*, void )
{
    nCutEvent = nullptr;
    OTableRowView::cut();
}

IMPL_LINK_NOARG( OTableEditorCtrl, DelayedPaste, void*, void )
{
    nPasteEvent = nullptr;

    sal_Int32 nPastePosition = GetView()->getController().getFirstEmptyRowPosition();
    if ( !GetView()->getController().getTable().is() )
        nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : GetCurRow();

    if (!IsInsertNewAllowed(nPastePosition))
    {   // Insertion is not allowed, only appending, so test if there are full cells after the PastePosition

        sal_Int32 nFreeFromPos; // from here on there are only empty rows
        ::std::vector< std::shared_ptr<OTableRow> >::const_reverse_iterator aIter = m_pRowList->rbegin();
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
}

IMPL_LINK_NOARG( OTableEditorCtrl, DelayedDelete, void*, void )
{
    nDeleteEvent = nullptr;
    DeleteRows();
}

IMPL_LINK_NOARG( OTableEditorCtrl, DelayedInsNewRows, void*, void )
{
    nInsNewRowsEvent = nullptr;
    sal_Int32 nPastePosition = GetView()->getController().getFirstEmptyRowPosition();
    if ( !GetView()->getController().getTable().is() )
        nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : m_nDataPos;

    InsertNewRows( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );
}

void OTableEditorCtrl::AdjustFieldDescription(OFieldDescription* _pFieldDesc,
                                         MultiSelection& _rMultiSel,
                                         sal_Int32 _nPos,
                                         bool _bSet,
                                         bool _bPrimaryKey)
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

void OTableEditorCtrl::SetPrimaryKey( bool bSet )
{
    // Delete any existing Primary Keys
    MultiSelection aDeletedPrimKeys;
    aDeletedPrimKeys.SetTotalRange( Range(0,GetRowCount()) );

    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aIter = m_pRowList->begin();
    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aEnd = m_pRowList->end();
    for(sal_Int32 nRow = 0;aIter != aEnd;++aIter,++nRow)
    {
        OFieldDescription* pFieldDescr = (*aIter)->GetActFieldDescr();
        if( pFieldDescr && (*aIter)->IsPrimaryKey() && (!bSet || !IsRowSelected(nRow)) )
        {
            AdjustFieldDescription(pFieldDescr,aDeletedPrimKeys,nRow,bSet,false);
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
             std::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nIndex];
            OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
            if(pFieldDescr)
                AdjustFieldDescription(pFieldDescr,aInsertedPrimKeys,nIndex,false,true);

            nIndex = NextSelectedRow();
        }
    }

    GetUndoManager().AddUndoAction( new OPrimKeyUndoAct(this, aDeletedPrimKeys, aInsertedPrimKeys) );

    // Invalidate the handle-columns
    InvalidateHandleColumn();

    // Set the TableDocSh's ModifyFlag
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

bool OTableEditorCtrl::IsPrimaryKey()
{
    // Are all marked fields part of the Primary Key ?
    long nPrimaryKeys = 0;
    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aIter = m_pRowList->begin();
    ::std::vector< std::shared_ptr<OTableRow> >::const_iterator aEnd = m_pRowList->end();
    for(sal_Int32 nRow=0;aIter != aEnd;++aIter,++nRow)
    {
        if( IsRowSelected(nRow) && !(*aIter)->IsPrimaryKey() )
            return false;
        if( (*aIter)->IsPrimaryKey() )
            ++nPrimaryKeys;
    }

    // Are there any unselected fields that are part of the Key ?
    return GetSelectRowCount() == nPrimaryKeys;
}

void OTableEditorCtrl::SwitchType( const TOTypeInfoSP& _pType )
{
    // if there is no assigned field name
    long nRow(GetCurRow());
    OFieldDescription* pActFieldDescr = GetFieldDescr( nRow );
    if( pActFieldDescr )
        // Store the old description
        pDescrWin->SaveData( pActFieldDescr );

    if ( nRow < 0 || nRow > static_cast<long>(m_pRowList->size()) )
        return;
    // Show the new description
     std::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nRow];
    pRow->SetFieldType( _pType, true );
    if ( _pType.get() )
    {
        const sal_Int32 nCurrentlySelected = pTypeCell->GetSelectEntryPos();

        if  (   ( LISTBOX_ENTRY_NOTFOUND == nCurrentlySelected )
            ||  ( GetView()->getController().getTypeInfo( nCurrentlySelected ) != _pType )
            )
        {
            sal_Int32 nEntryPos = 0;
            const OTypeInfoMap& rTypeInfo = GetView()->getController().getTypeInfo();
            OTypeInfoMap::const_iterator aIter = rTypeInfo.begin();
            OTypeInfoMap::const_iterator aEnd = rTypeInfo.end();
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
    if (pActFieldDescr != nullptr && !pActFieldDescr->GetFormatKey())
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

void OTableEditorCtrl::DeactivateCell(bool bUpdate)
{
    OTableRowView::DeactivateCell(bUpdate);
    // now we have to deactivate the field description
    long nRow(GetCurRow());
    if (pDescrWin)
        pDescrWin->SetReadOnly(bReadOnly || !SetDataPtr(nRow) || GetActRow()->IsReadOnly());
}

bool OTableEditorCtrl::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
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
