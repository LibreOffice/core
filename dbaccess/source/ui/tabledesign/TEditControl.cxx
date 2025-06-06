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
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <helpids.h>
#include <comphelper/types.hxx>
#include <FieldDescControl.hxx>
#include <FieldDescriptions.hxx>
#include "TableUndo.hxx"
#include <TableController.hxx>
#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbtools.hxx>
#include <SqlNameEdit.hxx>
#include <TableRowExchange.hxx>
#include <o3tl/safeint.hxx>
#include <sot/storage.hxx>
#include <svx/svxids.hrc>
#include <UITools.hxx>
#include "TableFieldControl.hxx"
#include <dsntypes.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>

using namespace ::dbaui;
using namespace ::comphelper;
using namespace ::svt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;


#define HANDLE_ID       0

// default field widths
#define FIELDNAME_WIDTH     100
#define FIELDTYPE_WIDTH     150
#define FIELDDESCR_WIDTH    300

// Maximum length in description field
#define MAX_DESCR_LEN       256

OTableEditorCtrl::ClipboardInvalidator::ClipboardInvalidator(OTableEditorCtrl* _pOwner)
: m_aInvalidateTimer("dbaccess ClipboardInvalidator")
, m_pOwner(_pOwner)
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
    InsertDataColumn( FIELD_NAME, DBA_RES(STR_TAB_FIELD_COLUMN_NAME), FIELDNAME_WIDTH );

    InsertDataColumn( FIELD_TYPE, DBA_RES(STR_TAB_FIELD_COLUMN_DATATYPE), FIELDTYPE_WIDTH );

    ::dbaccess::ODsnTypeCollection aDsnTypes(GetView()->getController().getORB());
    bool bShowColumnDescription = aDsnTypes.supportsColumnDescription(::comphelper::getString(GetView()->getController().getDataSource()->getPropertyValue(PROPERTY_URL)));
    InsertDataColumn( HELP_TEXT, DBA_RES(STR_TAB_HELP_TEXT), bShowColumnDescription ? FIELDTYPE_WIDTH : FIELDDESCR_WIDTH );

    if ( bShowColumnDescription )
    {
        InsertDataColumn( COLUMN_DESCRIPTION, DBA_RES(STR_COLUMN_DESCRIPTION), FIELDTYPE_WIDTH );
    }

    InitCellController();

    // Insert the rows
    RowInserted(0, m_pRowList->size());
}

OTableEditorCtrl::OTableEditorCtrl(vcl::Window* pWindow, OTableDesignView* pView)
    :OTableRowView(pWindow)
    ,m_pView(pView)
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
    sal_Int32 nRow(GetCurRow());
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
    sal_Int32 nMaxTextLen = 0;
    OUString sExtraNameChars;
    Reference<XConnection> xCon;
    try
    {
        xCon = GetView()->getController().getConnection();
        Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

        // length 0 is treated by Entry::set_max_length as unlimited
        nMaxTextLen = xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0;

        sExtraNameChars = xMetaData.is() ? xMetaData->getExtraNameCharacters() : OUString();

    }
    catch(SQLException&)
    {
        OSL_FAIL("getMaxColumnNameLength");
    }

    pNameCell = VclPtr<OSQLNameEditControl>::Create(&GetDataWindow(), sExtraNameChars);
    pNameCell->get_widget().set_max_length(nMaxTextLen);
    pNameCell->setCheck( isSQL92CheckEnabled(xCon) );

    // Cell type
    pTypeCell = VclPtr<ListBoxControl>::Create( &GetDataWindow() );

    // Cell description
    pDescrCell = VclPtr<EditControl>::Create(&GetDataWindow());
    pDescrCell->get_widget().set_max_length(MAX_DESCR_LEN);

    pHelpTextCell = VclPtr<EditControl>::Create(&GetDataWindow());
    pHelpTextCell->get_widget().set_max_length(MAX_DESCR_LEN);

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
            aHeight.setHeight( aTemp.Height() );
    }
    SetDataRowHeight(aHeight.Height());

    ClearModified();
}

void OTableEditorCtrl::ClearModified()
{
    pNameCell->get_widget().save_value();
    pDescrCell->get_widget().save_value();
    pHelpTextCell->get_widget().save_value();
    pTypeCell->get_widget().save_value();
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
    pDescrWin = nullptr;
    m_pView.reset();
    OTableRowView::dispose();
}

bool OTableEditorCtrl::SetDataPtr( sal_Int32 nRow )
{
    if(nRow == -1)
        return false;

    OSL_ENSURE(nRow < static_cast<tools::Long>(m_pRowList->size()),"Row is greater than size!");
    if(nRow >= static_cast<tools::Long>(m_pRowList->size()))
        return false;
    pActRow = (*m_pRowList)[nRow];
    return pActRow != nullptr;
}

bool OTableEditorCtrl::SeekRow(sal_Int32 _nRow)
{
    // Call the Base class to remember which row must be repainted
    EditBrowseBox::SeekRow(_nRow);

    m_nCurrentPos = _nRow;
    return SetDataPtr(_nRow);
}

void OTableEditorCtrl::PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect,
                                   sal_uInt16 nColumnId ) const
{
    const OUString aText( GetCellText( m_nCurrentPos, nColumnId ));

    rDev.Push( vcl::PushFlags::CLIPREGION );
    rDev.SetClipRegion(vcl::Region(rRect));
    rDev.DrawText( rRect, aText, DrawTextFlags::Left | DrawTextFlags::VCenter );
    rDev.Pop();
}

CellController* OTableEditorCtrl::GetController(sal_Int32 nRow, sal_uInt16 nColumnId)
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

void OTableEditorCtrl::InitController(CellControllerRef&, sal_Int32 nRow, sal_uInt16 nColumnId)
{
    SeekRow( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();
    OUString aInitString;

    switch (nColumnId)
    {
        case FIELD_NAME:
        {
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetName();

            weld::Entry& rEntry = pNameCell->get_widget();
            rEntry.set_text(aInitString);
            rEntry.save_value();
            break;
        }
        case FIELD_TYPE:
            {
                if ( pActFieldDescr && pActFieldDescr->getTypeInfo() )
                    aInitString = pActFieldDescr->getTypeInfo()->aUIName;

                // Set the ComboBox contents
                weld::ComboBox& rTypeList = pTypeCell->get_widget();
                rTypeList.clear();
                if( !pActFieldDescr )
                    break;

                const OTypeInfoMap& rTypeInfo = GetView()->getController().getTypeInfo();
                for (auto const& elem : rTypeInfo)
                    rTypeList.append_text(elem.second->aUIName);
                rTypeList.set_active_text(aInitString);
            }

            break;
        case HELP_TEXT:
        {
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetHelpText();
            weld::Entry& rEntry = pHelpTextCell->get_widget();
            rEntry.set_text(aInitString);
            rEntry.save_value();
            break;
        }
        case COLUMN_DESCRIPTION:
        {
            if( pActFieldDescr )
                aInitString = pActFieldDescr->GetDescription();
            weld::Entry& rEntry = pDescrCell->get_widget();
            rEntry.set_text(aInitString);
            rEntry.save_value();
            break;
        }
    }
}

EditBrowseBox::RowStatus OTableEditorCtrl::GetRowStatus(sal_Int32 nRow) const
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

void OTableEditorCtrl::DisplayData(sal_Int32 nRow)
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

sal_Int32 OTableEditorCtrl::HasFieldName( std::u16string_view rFieldName )
{

    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    ::comphelper::UStringMixEqual bCase(!xMetaData.is() || xMetaData->supportsMixedCaseQuotedIdentifiers());

    sal_Int32 nCount(0);
    for (auto const& row : *m_pRowList)
    {
        OFieldDescription* pFieldDescr = row->GetActFieldDescr();
        if( pFieldDescr && bCase(rFieldName,pFieldDescr->GetName()))
            nCount++;
    }
    return nCount;
}

void OTableEditorCtrl::SaveData(sal_Int32 nRow, sal_uInt16 nColId)
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
            weld::Entry& rEntry = pNameCell->get_widget();
            const OUString aName(rEntry.get_text());

            if( aName.isEmpty() )
            {
                // If FieldDescr exists, the field is deleted and the old content restored
                if (pActFieldDescr)
                {
                    GetUndoManager().AddUndoAction(std::make_unique<OTableEditorTypeSelUndoAct>(this, nRow, FIELD_TYPE, pActFieldDescr->getTypeInfo()));
                    SwitchType(TOTypeInfoSP());
                    pActFieldDescr = pActRow->GetActFieldDescr();
                }
                else
                    return;
            }
            if(pActFieldDescr)
                pActFieldDescr->SetName( aName );
            rEntry.save_value();

            break;
        }

        // Store the field type
        case FIELD_TYPE:
            break;

        // Store DescrCell
        case HELP_TEXT:
        {
            // if the current field description is NULL, set Default
            weld::Entry& rEntry = pHelpTextCell->get_widget();
            if( !pActFieldDescr )
            {
                rEntry.set_text(OUString());
                rEntry.save_value();
            }
            else
                pActFieldDescr->SetHelpText(rEntry.get_text());
            break;
        }
        case COLUMN_DESCRIPTION:
        {
            // Set the default if the field description is null
            weld::Entry& rEntry = pDescrCell->get_widget();
            if( !pActFieldDescr )
            {
                rEntry.set_text(OUString());
                rEntry.save_value();
            }
            else
                pActFieldDescr->SetDescription(rEntry.get_text());
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

bool OTableEditorCtrl::CursorMoving(sal_Int32 nNewRow, sal_uInt16 nNewCol)
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

void OTableEditorCtrl::CellModified( sal_Int32 nRow, sal_uInt16 nColId )
{

    // If the description is null, use the default
    if(nRow == -1)
        nRow = GetCurRow();
    SetDataPtr( nRow );
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    OUString sActionDescription;
    switch ( nColId )
    {
    case FIELD_NAME:    sActionDescription = DBA_RES( STR_CHANGE_COLUMN_NAME ); break;
    case FIELD_TYPE:    sActionDescription = DBA_RES( STR_CHANGE_COLUMN_TYPE ); break;
    case HELP_TEXT:
    case COLUMN_DESCRIPTION:   sActionDescription = DBA_RES( STR_CHANGE_COLUMN_DESCRIPTION ); break;
    default:            sActionDescription = DBA_RES( STR_CHANGE_COLUMN_ATTRIBUTE ); break;
    }

    GetUndoManager().EnterListAction( sActionDescription, OUString(), 0, ViewShellId(-1) );
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
        GetUndoManager().AddUndoAction( std::make_unique<OTableEditorTypeSelUndoAct>(this, nRow, nColId+1, TOTypeInfoSP()) );
    }

    if( nColId != FIELD_TYPE )
        GetUndoManager().AddUndoAction( std::make_unique<OTableDesignCellUndoAct>(this, nRow, nColId) );
    else
    {
        GetUndoManager().AddUndoAction(std::make_unique<OTableEditorTypeSelUndoAct>(this, GetCurRow(), nColId, GetFieldDescr(GetCurRow())->getTypeInfo()));
        resetType();
    }

    SaveData(nRow,nColId);
    // SaveData could create an undo action as well
    GetUndoManager().LeaveListAction();
    RowModified(nRow);

    // Set the Modify flag
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::resetType()
{
    sal_Int32 nPos = pTypeCell->get_widget().get_active();
    if(nPos != -1)
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
    std::vector< std::shared_ptr<OTableRow> > vClipboardList;
    vClipboardList.reserve(GetSelectRowCount());

    for( tools::Long nIndex=FirstSelectedRow(); nIndex != SFX_ENDOFSELECTION; nIndex=NextSelectedRow() )
    {
        pRow = (*m_pRowList)[nIndex];
        OSL_ENSURE(pRow,"OTableEditorCtrl::CopyRows: Row is NULL!");
        if ( pRow && pRow->GetActFieldDescr() )
        {
            pClipboardRow = std::make_shared<OTableRow>( *pRow );
            vClipboardList.push_back( pClipboardRow);
        }
    }
    if(!vClipboardList.empty())
    {
        rtl::Reference<OTableRowExchange> pData = new OTableRowExchange(std::move(vClipboardList));
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

void OTableEditorCtrl::InsertRows( sal_Int32 nRow )
{

    std::vector<  std::shared_ptr<OTableRow> > vInsertedUndoRedoRows; // need for undo/redo handling
    // get rows from clipboard
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(SotClipboardFormatId::SBA_TABED))
    {
        std::unique_ptr<SvStream> aStreamRef = aTransferData.GetSotStorageStream(SotClipboardFormatId::SBA_TABED);
        if (aStreamRef)
        {
            aStreamRef->Seek(STREAM_SEEK_TO_BEGIN);
            aStreamRef->ResetError();
            sal_Int32 nInsertRow = nRow;
            std::shared_ptr<OTableRow>  pRow;
            sal_Int32 nSize = 0;
            (*aStreamRef).ReadInt32( nSize );
            vInsertedUndoRedoRows.reserve(nSize);
            for(sal_Int32 i=0;i < nSize;++i)
            {
                pRow = std::make_shared<OTableRow>();
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
    RowInserted( nRow,vInsertedUndoRedoRows.size() );

    // Create the Undo-Action
    GetUndoManager().AddUndoAction( std::make_unique<OTableEditorInsUndoAct>(this, nRow, std::move(vInsertedUndoRedoRows)) );
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::DeleteRows()
{
    OSL_ENSURE(GetView()->getController().isDropAllowed(),"Call of DeleteRows not valid here. Please check isDropAllowed!");
    // Create the Undo-Action
    GetUndoManager().AddUndoAction( std::make_unique<OTableEditorDelUndoAct>(this) );

    // Delete all marked rows
    tools::Long nIndex = FirstSelectedRow();
    nOldDataPos = nIndex;

    while( nIndex != SFX_ENDOFSELECTION )
    {
        // Remove rows
        m_pRowList->erase( m_pRowList->begin()+nIndex );
        RowRemoved( nIndex );

        // Insert the empty row at the end
        m_pRowList->push_back( std::make_shared<OTableRow>());
        RowInserted( GetRowCount()-1 );

        nIndex = FirstSelectedRow();
    }

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

void OTableEditorCtrl::InsertNewRows( sal_Int32 nRow )
{
    OSL_ENSURE(GetView()->getController().isAddAllowed(),"Call of InsertNewRows not valid here. Please check isAppendAllowed!");
    // Create Undo-Action
    sal_Int32 nInsertRows = GetSelectRowCount();
    if( !nInsertRows )
        nInsertRows = 1;
    GetUndoManager().AddUndoAction( std::make_unique<OTableEditorInsNewUndoAct>(this, nRow, nInsertRows) );
    // Insert the number of selected rows
    for( tools::Long i=nRow; i<(nRow+nInsertRows); i++ )
        m_pRowList->insert( m_pRowList->begin()+i ,std::make_shared<OTableRow>());
    RowInserted( nRow, nInsertRows );

    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

void OTableEditorCtrl::SetControlText( sal_Int32 nRow, sal_uInt16 nColId, const OUString& rText )
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

void OTableEditorCtrl::SetCellData( sal_Int32 nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo )
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
    SetControlText(nRow,nColId,_pTypeInfo ? _pTypeInfo->aUIName : OUString());
}

void OTableEditorCtrl::SetCellData( sal_Int32 nRow, sal_uInt16 nColId, const css::uno::Any& _rNewData )
{
    // Relocate the current pointer
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    OUString sValue;
    // Set individual fields
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
            sValue = ::comphelper::getString(_rNewData);
            pFieldDescr->SetDescription( sValue );
            break;

        case FIELD_PROPERTY_DEFAULT:
            pFieldDescr->SetControlDefault( _rNewData );
            sValue = GetView()->GetDescWin()->getGenPage()->getControlDefault(pFieldDescr);
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
                pFieldDescr->SetAutoIncrement(sValue == DBA_RES(STR_VALUE_YES));
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
            pFieldDescr->SetControlDefault(Any(sValue));
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

Any OTableEditorCtrl::GetCellData( sal_Int32 nRow, sal_uInt16 nColId )
{
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr )
        return Any();

    // Relocate the current pointer
    if( nRow==-1 )
        nRow = GetCurRow();
    SetDataPtr( nRow );

    static const OUString strYes(DBA_RES(STR_VALUE_YES));
    static const OUString strNo(DBA_RES(STR_VALUE_NO));
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

    return Any(sValue);
}

OUString OTableEditorCtrl::GetCellText( sal_Int32 nRow, sal_uInt16 nColId ) const
{
    OUString sCellText;
    const_cast< OTableEditorCtrl* >( this )->GetCellData( nRow, nColId ) >>= sCellText;
    return sCellText;
}

sal_uInt32 OTableEditorCtrl::GetTotalCellWidth(sal_Int32 nRow, sal_uInt16 nColId)
{
    return GetTextWidth(GetCellText(nRow, nColId)) + 2 * GetTextWidth(u"0"_ustr);
}

OFieldDescription* OTableEditorCtrl::GetFieldDescr( sal_Int32 nRow )
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

bool OTableEditorCtrl::IsCutAllowed()
{
    bool bIsCutAllowed = (GetView()->getController().isAddAllowed() && GetView()->getController().isDropAllowed()) ||
                            GetView()->getController().isAlterAllowed();

    if (bIsCutAllowed)
    {
        int nStartPos, nEndPos;
        switch(m_eChildFocus)
        {
            case DESCRIPTION:
            {
                weld::Entry& rEntry = pDescrCell->get_widget();
                bIsCutAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
                break;
            }
            case HELPTEXT:
            {
                weld::Entry& rEntry = pHelpTextCell->get_widget();
                bIsCutAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
                break;
            }
            case NAME:
            {
                weld::Entry& rEntry = pNameCell->get_widget();
                bIsCutAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
                break;
            }
            case ROW:
                bIsCutAllowed = IsCopyAllowed();
                break;
            default:
                bIsCutAllowed = false;
                break;
        }
    }

    return bIsCutAllowed;
}

bool OTableEditorCtrl::IsCopyAllowed()
{
    bool bIsCopyAllowed = false;
    int nStartPos, nEndPos;
    if (m_eChildFocus == DESCRIPTION )
    {
        weld::Entry& rEntry = pDescrCell->get_widget();
        bIsCopyAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        weld::Entry& rEntry = pHelpTextCell->get_widget();
        bIsCopyAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
    }
    else if(m_eChildFocus == NAME)
    {
        weld::Entry& rEntry = pNameCell->get_widget();
        bIsCopyAllowed = rEntry.get_selection_bounds(nStartPos, nEndPos);
    }
    else if(m_eChildFocus == ROW)
    {
        Reference<XPropertySet> xTable = GetView()->getController().getTable();
        if( !GetSelectRowCount() || (xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW"))
            return false;

        // If one of the selected rows is empty, Copy is not possible
        std::shared_ptr<OTableRow>  pRow;
        tools::Long nIndex = FirstSelectedRow();
        while( nIndex != SFX_ENDOFSELECTION )
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

bool OTableEditorCtrl::IsPasteAllowed() const
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
            pNameCell->get_widget().cut_clipboard();
            CellModified(-1,FIELD_NAME);
        }
    }
    else if(m_eChildFocus == DESCRIPTION)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            SaveData(-1,COLUMN_DESCRIPTION);
            pDescrCell->get_widget().cut_clipboard();
            CellModified(-1,COLUMN_DESCRIPTION);
        }
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        if(GetView()->getController().isAlterAllowed())
        {
            SaveData(-1,HELP_TEXT);
            pHelpTextCell->get_widget().cut_clipboard();
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
    if (GetSelectRowCount())
        OTableRowView::copy();
    else if(m_eChildFocus == NAME)
    {
        weld::Entry& rEntry = pNameCell->get_widget();
        rEntry.copy_clipboard();
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        weld::Entry& rEntry = pHelpTextCell->get_widget();
        rEntry.copy_clipboard();
    }
    else if(m_eChildFocus == DESCRIPTION )
    {
        weld::Entry& rEntry = pDescrCell->get_widget();
        rEntry.copy_clipboard();
    }
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
            pNameCell->get_widget().paste_clipboard();
            CellModified();
        }
    }
    else if(HELPTEXT == m_eChildFocus )
    {
        if(GetView()->getController().isAlterAllowed())
        {
            pHelpTextCell->get_widget().paste_clipboard();
            CellModified();
        }
    }
    else if(m_eChildFocus == DESCRIPTION)
    {
        if(GetView()->getController().isAlterAllowed())
        {
            pDescrCell->get_widget().paste_clipboard();
            CellModified();
        }
    }
}

bool OTableEditorCtrl::IsDeleteAllowed()
{

    return GetSelectRowCount() != 0 && GetView()->getController().isDropAllowed();
}

bool OTableEditorCtrl::IsInsertNewAllowed( sal_Int32 nRow )
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

bool OTableEditorCtrl::IsPrimaryKeyAllowed()
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
    tools::Long nIndex = FirstSelectedRow();
    std::shared_ptr<OTableRow>  pRow;
    while( nIndex != SFX_ENDOFSELECTION )
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
            const TOTypeInfoSP& pTypeInfo = pFieldDescr->getTypeInfo();
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
                    ::tools::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, false ) );

                    aMenuPos = aColRect.TopCenter();
                }
                else if ( GetSelectRowCount() > 0 )
                {
                    ::tools::Rectangle aColRect( GetFieldRectPixel( FirstSelectedRow(), HANDLE_ID ) );

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
                sal_uInt16 nColId = GetColumnId(GetColumnAtXPosPixel(aMenuPos.X()));
                sal_Int32  nRow = GetRowAtYPosPixel(aMenuPos.Y());

                if ( HANDLE_ID != nColId )
                {
                    if ( nRow < 0 && nColId != BROWSER_INVALIDID )
                    {   // hit the header
                        if ( 3 != nColId )
                        {   // 3 would mean the last column, and this last column is auto-sized
                            if ( !IsColumnSelected( nColId ) )
                                SelectColumnId( nColId );

                            ::tools::Rectangle aRect(aMenuPos, Size(1, 1));
                            weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);
                            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"dbaccess/ui/querycolmenu.ui"_ustr));
                            std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu(u"menu"_ustr));
                            xContextMenu->remove(u"delete"_ustr);
                            xContextMenu->remove(u"separator"_ustr);
                            if (xContextMenu->popup_at_rect(pPopupParent, aRect) == "width")
                                adjustBrowseBoxColumnWidth( this, nColId );
                        }
                    }
                }
                else
                {
                    ::tools::Rectangle aRect(aMenuPos, Size(1, 1));
                    weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);
                    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"dbaccess/ui/tabledesignrowmenu.ui"_ustr));
                    std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu(u"menu"_ustr));

                    if (!IsCutAllowed())
                        xContextMenu->remove(u"cut"_ustr);
                    if (!IsCopyAllowed())
                        xContextMenu->remove(u"copy"_ustr);
                    if (!IsPasteAllowed())
                        xContextMenu->remove(u"paste"_ustr);
                    if (!IsDeleteAllowed())
                        xContextMenu->remove(u"delete"_ustr);
                    // tdf#71224: WORKAROUND for the moment, we don't implement insert field at specific position
                    // It's not SQL standard and each database has made its choice (some use "BEFORE", other "FIRST" and "AFTER")
                    // and some, like Postgresql, don't allow this.
                    // So for the moment, test if the table already exists (and so it's an edition), in this case only
                    // we remove "Insert Fields" entry. Indeed, in case of new table, there's no pb.
                    //
                    // The real fix is to implement the insert for each database + error message for those which don't support this
                    //if (!IsInsertNewAllowed(nRow))
                    if ( GetView()->getController().getTable().is() )
                        xContextMenu->remove(u"insert"_ustr);

                    if (IsPrimaryKeyAllowed())
                    {
                        xContextMenu->set_active(u"primarykey"_ustr, IsRowSelected(GetCurRow()) && IsPrimaryKey());
                    }
                    else
                    {
                        xContextMenu->remove(u"primarykey"_ustr);
                    }

                    if( SetDataPtr(m_nDataPos) )
                        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

                    // All actions which change the number of rows must be run asynchronously
                    // otherwise there may be problems between the Context menu and the Browser
                    m_nDataPos = GetCurRow();
                    OUString sIdent = xContextMenu->popup_at_rect(pPopupParent, aRect);
                    if (sIdent == "cut")
                        cut();
                    else if (sIdent == "copy")
                        copy();
                    else if (sIdent == "paste")
                        paste();
                    else if (sIdent == "delete")
                    {
                        if( nDeleteEvent )
                            Application::RemoveUserEvent( nDeleteEvent );
                        nDeleteEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedDelete), nullptr, true );
                    }
                    else if (sIdent == "insert")
                    {
                        if( nInsNewRowsEvent )
                            Application::RemoveUserEvent( nInsNewRowsEvent );
                        nInsNewRowsEvent = Application::PostUserEvent( LINK(this, OTableEditorCtrl, DelayedInsNewRows), nullptr, true );
                    }
                    else if (sIdent == "primarykey")
                    {
                        SetPrimaryKey( !IsPrimaryKey() );
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

        auto aIter = std::find_if(m_pRowList->rbegin(), m_pRowList->rend(), [](const std::shared_ptr<OTableRow>& rxRow) {
            return rxRow && rxRow->GetActFieldDescr() && !rxRow->GetActFieldDescr()->GetName().isEmpty(); });
        auto nFreeFromPos = static_cast<sal_Int32>(std::distance(aIter, m_pRowList->rend())); // from here on there are only empty rows
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

    sal_Int32 nRow = 0;
    for (auto const& row : *m_pRowList)
    {
        OFieldDescription* pFieldDescr = row->GetActFieldDescr();
        if( pFieldDescr && row->IsPrimaryKey() && (!bSet || !IsRowSelected(nRow)) )
        {
            AdjustFieldDescription(pFieldDescr,aDeletedPrimKeys,nRow,bSet,false);
        }
        ++nRow;
    }

    // Set the primary keys of the marked rows
    MultiSelection aInsertedPrimKeys;
    aInsertedPrimKeys.SetTotalRange( Range(0,GetRowCount()) );
    if( bSet )
    {
        tools::Long nIndex = FirstSelectedRow();
        while( nIndex != SFX_ENDOFSELECTION )
        {
            // Set the key
            std::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nIndex];
            OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
            if(pFieldDescr)
                AdjustFieldDescription(pFieldDescr,aInsertedPrimKeys,nIndex,false,true);

            nIndex = NextSelectedRow();
        }
    }

    GetUndoManager().AddUndoAction( std::make_unique<OPrimKeyUndoAct>(this, aDeletedPrimKeys, aInsertedPrimKeys) );

    // Invalidate the handle-columns
    InvalidateHandleColumn();

    // Set the TableDocSh's ModifyFlag
    GetView()->getController().setModified( true );
    InvalidateFeatures();
}

bool OTableEditorCtrl::IsPrimaryKey()
{
    // Are all marked fields part of the Primary Key ?
    tools::Long nPrimaryKeys = 0;
    sal_Int32 nRow=0;
    for (auto const& row : *m_pRowList)
    {
        if( IsRowSelected(nRow) && !row->IsPrimaryKey() )
            return false;
        if( row->IsPrimaryKey() )
            ++nPrimaryKeys;
        ++nRow;
    }

    // Are there any unselected fields that are part of the Key ?
    return GetSelectRowCount() == nPrimaryKeys;
}

void OTableEditorCtrl::SwitchType( const TOTypeInfoSP& _pType )
{
    // if there is no assigned field name
    sal_Int32 nRow(GetCurRow());
    OFieldDescription* pActFieldDescr = GetFieldDescr( nRow );
    if( pActFieldDescr )
        // Store the old description
        pDescrWin->SaveData( pActFieldDescr );

    if ( nRow < 0 || o3tl::make_unsigned(nRow) > m_pRowList->size() )
        return;
    // Show the new description
    std::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nRow];
    pRow->SetFieldType( _pType, true );
    if ( _pType )
    {
        weld::ComboBox& rTypeList = pTypeCell->get_widget();
        const sal_Int32 nCurrentlySelected = rTypeList.get_active();

        if  (   ( nCurrentlySelected == -1 )
            ||  ( GetView()->getController().getTypeInfo( nCurrentlySelected ) != _pType )
            )
        {
            sal_Int32 nEntryPos = 0;
            const OTypeInfoMap& rTypeInfo = GetView()->getController().getTypeInfo();
            for (auto const& elem : rTypeInfo)
            {
                if(elem.second == _pType)
                    break;
                ++nEntryPos;
            }
            if (nEntryPos < rTypeList.get_count())
                rTypeList.set_active(nEntryPos);
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
    return m_pView;
}

void OTableEditorCtrl::DeactivateCell(bool bUpdate)
{
    OTableRowView::DeactivateCell(bUpdate);
    // now we have to deactivate the field description
    sal_Int32 nRow(GetCurRow());
    if (pDescrWin)
        pDescrWin->SetReadOnly(bReadOnly || !SetDataPtr(nRow) || GetActRow()->IsReadOnly());
}

bool OTableEditorCtrl::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == NotifyEventType::GETFOCUS)
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
