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
//==============================================================================

//  TYPEINIT1(OTableEditorCtrl, DBView);
DBG_NAME(OTableEditorCtrl)

//==============================================================================

#define HANDLE_ID       0

// default Spaltenbreiten
#define FIELDNAME_WIDTH     100
#define FIELDTYPE_WIDTH     150
#define FIELDDESCR_WIDTH    300

// Maximale Eingabelaenge im Beschreibungsfeld
#define MAX_DESCR_LEN       256


#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   5
#define CONTROL_HEIGHT      20
#define CONTROL_WIDTH_1     140 // 100
#define CONTROL_WIDTH_2     100 // 60
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - CONTROL_HEIGHT - 5)


//==================================================================
DBG_NAME(ClipboardInvalidator)
//------------------------------------------------------------------
OTableEditorCtrl::ClipboardInvalidator::ClipboardInvalidator(sal_uLong nTimeout,OTableEditorCtrl* _pOwner)
: m_pOwner(_pOwner)
{
    DBG_CTOR(ClipboardInvalidator,NULL);

    m_aInvalidateTimer.SetTimeout(nTimeout);
    m_aInvalidateTimer.SetTimeoutHdl(LINK(this, OTableEditorCtrl::ClipboardInvalidator, OnInvalidate));
    m_aInvalidateTimer.Start();
}

//------------------------------------------------------------------
OTableEditorCtrl::ClipboardInvalidator::~ClipboardInvalidator()
{
    m_aInvalidateTimer.Stop();

    DBG_DTOR(ClipboardInvalidator,NULL);
}

//------------------------------------------------------------------
IMPL_LINK_NOARG(OTableEditorCtrl::ClipboardInvalidator, OnInvalidate)
{
    m_pOwner->GetView()->getController().InvalidateFeature(SID_CUT);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_COPY);
    m_pOwner->GetView()->getController().InvalidateFeature(SID_PASTE);
    return 0L;
}

//==================================================================
void OTableEditorCtrl::Init()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OTableRowView::Init();

    //////////////////////////////////////////////////////////////////////
    // Soll der Entwurf ReadOnly geoeffnet werden ?
    sal_Bool bRead(GetView()->getController().isReadOnly());

    SetReadOnly( bRead );

    //////////////////////////////////////////////////////////////////////
    // Spalten einfuegen
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

    //////////////////////////////////////////////////////////////////////
    // Zeilen einfuegen
    RowInserted(0, m_pRowList->size(), sal_True);
}

//==================================================================
void OTableEditorCtrl::UpdateAll()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    RowRemoved(0, GetRowCount(), sal_False);
    m_nDataPos = 0;

    InvalidateFeatures();
    Invalidate();
}
//==================================================================
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

//------------------------------------------------------------------------------
SfxUndoManager& OTableEditorCtrl::GetUndoManager() const
{
    return GetView()->getController().GetUndoManager();
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::IsReadOnly()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    return bReadOnly;
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::SetReadOnly( sal_Bool bRead )
{
    // nix zu tun ?
    if (bRead == IsReadOnly())
        // diese Abfrage ist wichtig, da die zugrundeliegende Def sonst im folgenden gelockt oder ge-unlocked wird, obwohl es
        // nicht notwendig waere (und was schlimmer ist, das wuerde dann auch nicht wieder rueckgaengig gemacht)
        return;

    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    bReadOnly = bRead;

    //////////////////////////////////////////////////////////////////////
    // Aktive Zelle disablen
    long nRow(GetCurRow());
    sal_uInt16 nCol(GetCurColumnId());
    DeactivateCell();

    //////////////////////////////////////////////////////////////////////
    // Cursor des Browsers anpassen
    BrowserMode nMode(BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                      BROWSER_HLINESFULL      | BROWSER_VLINESFULL|BROWSER_AUTOSIZE_LASTCOL);
    if( !bReadOnly )
        nMode |= BROWSER_HIDECURSOR;
    SetMode(nMode);

    if( !bReadOnly )
        ActivateCell( nRow, nCol );
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::InitCellController()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Zelle Feldname
    xub_StrLen nMaxTextLen = EDIT_NOLIMIT;
    ::rtl::OUString sExtraNameChars;
    Reference<XConnection> xCon;
    try
    {
        xCon = GetView()->getController().getConnection();
        Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

        nMaxTextLen = ((xub_StrLen)xMetaData.is() ? static_cast<xub_StrLen>(xMetaData->getMaxColumnNameLength()) : 0);

        if( nMaxTextLen == 0 )
            nMaxTextLen = EDIT_NOLIMIT;
        sExtraNameChars = xMetaData.is() ? xMetaData->getExtraNameCharacters() : ::rtl::OUString();

    }
    catch(SQLException&)
    {
        OSL_FAIL("getMaxColumnNameLength");
    }

    pNameCell = new OSQLNameEdit( &GetDataWindow(), sExtraNameChars,WB_LEFT );
    pNameCell->SetMaxTextLen( nMaxTextLen );
    pNameCell->setCheck( isSQL92CheckEnabled(xCon) );


    //////////////////////////////////////////////////////////////////////
    // Zelle Typ
    pTypeCell = new ListBoxControl( &GetDataWindow() );
    pTypeCell->SetDropDownLineCount( 15 );

    //////////////////////////////////////////////////////////////////////
    // Zelle Beschreibung
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

//------------------------------------------------------------------------------
void OTableEditorCtrl::ClearModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    pNameCell->ClearModifyFlag();
    pDescrCell->ClearModifyFlag();
    pHelpTextCell->ClearModifyFlag();
    pTypeCell->SaveValue();
}

//------------------------------------------------------------------------------
OTableEditorCtrl::~OTableEditorCtrl()
{
    DBG_DTOR(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Undo-Manager zuruecksetzen
    GetUndoManager().Clear();

    //////////////////////////////////////////////////////////////////////
    // Moegliche Events aus Queue entfernen
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

    //////////////////////////////////////////////////////////////////////
    // Controltypen zerstoeren
    delete pNameCell;
    delete pTypeCell;
    delete pDescrCell;
    delete pHelpTextCell;
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::SetDataPtr( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if(nRow == -1)
        return sal_False;

    OSL_ENSURE((xub_StrLen)nRow < m_pRowList->size(),"Row is greater than size!");
    if(nRow >= (long)m_pRowList->size())
        return sal_False;
    pActRow = (*m_pRowList)[nRow];
    return pActRow != NULL;
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::SeekRow(long _nRow)
{
    // die Basisklasse braucht den Aufruf, da sie sich dort merkt, welche Zeile gepainted wird
    EditBrowseBox::SeekRow(_nRow);

    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    m_nCurrentPos = _nRow;
    return SetDataPtr(_nRow);
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                   sal_uInt16 nColumnId ) const
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    const String aText( GetCellText( m_nCurrentPos, nColumnId ));
    const Size TxtSize(GetDataWindow().GetTextWidth(aText), GetDataWindow().GetTextHeight());

    rDev.Push( PUSH_CLIPREGION );
    rDev.SetClipRegion( rRect );
    rDev.DrawText( rRect, aText, TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER );
    rDev.Pop();
}

//------------------------------------------------------------------------------
CellController* OTableEditorCtrl::GetController(long nRow, sal_uInt16 nColumnId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Wenn EditorCtrl ReadOnly ist, darf nicht editiert werden
    Reference<XPropertySet> xTable = GetView()->getController().getTable();
    if (IsReadOnly() || (   xTable.is() &&
                            xTable->getPropertySetInfo()->hasPropertyByName(PROPERTY_TYPE) &&
                            ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"))))
        return NULL;

    //////////////////////////////////////////////////////////////////////
    // Wenn Zeile ReadOnly ist, darf sie nicht editiert werden
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

//------------------------------------------------------------------------------
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

                //////////////////////////////////////////////////////////////
                // Anpassen des ComboBoxInhalts
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::SaveCurRow()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if (GetFieldDescr(GetCurRow()) == NULL)
        // in der Zeile, in der ich mich i.A. befinde, stehen keine Daten
        return sal_True;
    if (!SaveModified())
        return sal_False;

    SetDataPtr(GetCurRow());
    pDescrWin->SaveData( pActRow->GetActFieldDescr() );
    return sal_True;
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::DisplayData(long nRow, sal_Bool bGrabFocus)
{
    // zur richtigen Zelle fahren
    SetDataPtr(nRow);

    // Editier-Modus temporaer aus
    sal_Bool bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    CellControllerRef aTemp;
    InitController(aTemp, nRow, FIELD_NAME);
    InitController(aTemp, nRow, FIELD_TYPE);
    InitController(aTemp, nRow, COLUMN_DESCRIPTION);
    InitController(aTemp, nRow, HELP_TEXT);

    GoToRow(nRow);
    // das Description-Window aktualisieren
    GetView()->GetDescWin()->DisplayData(GetFieldDescr(nRow));
    // neu zeichnen
    RowModified(nRow);

    // wieder an
    if (bWasEditing || bGrabFocus)
        ActivateCell(nRow, GetCurColumnId(), bGrabFocus);
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::CursorMoved()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Zeilenwechsel ?
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

//------------------------------------------------------------------------------
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
// --------------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::SaveData(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////
    // Zellinhalte in Datenstruktur speichern
    SetDataPtr( nRow == -1 ? GetCurRow() : nRow);
    OFieldDescription* pActFieldDescr = pActRow->GetActFieldDescr();

    switch( nColId)
    {
        //////////////////////////////////////////////////////////////
        // Speichern Inhalt NameCell
        case FIELD_NAME:
        {
            //////////////////////////////////////////////////////////////
            // Wenn kein Name, nichts machen
            String aName(pNameCell->GetText());

            if( !aName.Len() )
            {
                //////////////////////////////////////////////////////////////
                // Wenn FieldDescr existiert, wurde Feld geloescht und alter Inhalt wird wiederhergestellt
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

        //////////////////////////////////////////////////////////////
        // Speichern Inhalt TypeCell
        case FIELD_TYPE:
            break;

        //////////////////////////////////////////////////////////////
        // Speichern Inhalt DescrCell
        case HELP_TEXT:
        {
            //////////////////////////////////////////////////////////////
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
            //////////////////////////////////////////////////////////////
            // Wenn aktuelle Feldbeschreibung NULL, Default setzen
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

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::SaveModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    sal_uInt16 nColId = GetCurColumnId();

    switch( nColId )
    {
        //////////////////////////////////////////////////////////////
        // TypeCell
        case FIELD_TYPE:
        {
            //////////////////////////////////////////////////////////////////////
            // Type umstellen
            resetType();
        } break;
    }

    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    if (!EditBrowseBox::CursorMoving(nNewRow, nNewCol))
        return sal_False;

    //////////////////////////////////////////////////////////////////////
    // Wird nach SaveModified() gerufen, aktuelle Zeile ist noch die alte
    m_nDataPos = nNewRow;
    nOldDataPos = GetCurRow();

    //////////////////////////////////////////////////////////////////////
    // Marker umsetzen
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );

    //////////////////////////////////////////////////////////////////////
    // Daten des Propertyfensters speichern
    if( SetDataPtr(nOldDataPos) && pDescrWin)
        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

    //////////////////////////////////////////////////////////////////////
    // Neue Daten im Propertyfenster anzeigen
    if( SetDataPtr(m_nDataPos) && pDescrWin)
        pDescrWin->DisplayData( pActRow->GetActFieldDescr() );

    return sal_True;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableEditorCtrl, InvalidateFieldType, void*, /*EMPTYTAG*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    nInvalidateTypeEvent = 0;
    Invalidate( GetFieldRectPixel(nOldDataPos, FIELD_TYPE) );

    return 0;
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::CellModified( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    //////////////////////////////////////////////////////////////
    // Wenn aktuelle Feldbeschreibung NULL, Default setzen
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

    //////////////////////////////////////////////////////////////////////
    // Das ModifyFlag setzen
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}
// -----------------------------------------------------------------------------
void OTableEditorCtrl::resetType()
{
    sal_uInt16 nPos = pTypeCell->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        SwitchType( GetView()->getController().getTypeInfo(nPos) );
    else
        SwitchType(TOTypeInfoSP());
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::CellModified()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    CellModified( GetCurRow(), GetCurColumnId() );
}
// -----------------------------------------------------------------------------
void OTableEditorCtrl::InvalidateFeatures()
{
    GetView()->getController().InvalidateFeature(SID_UNDO);
    GetView()->getController().InvalidateFeature(SID_REDO);
    GetView()->getController().InvalidateFeature(SID_SAVEDOC);
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::Undo()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    InvalidateFeatures();
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::Redo()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    InvalidateFeatures();
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::CopyRows()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // set to the right row and save it
    if( SetDataPtr(m_nDataPos) )
        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

    //////////////////////////////////////////////////////////////////////
    // Selektierte Zeilen in die ClipboardListe kopieren
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

//------------------------------------------------------------------------------
String OTableEditorCtrl::GenerateName( const String& rName )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Basisnamen zum Anhaengen einer Numerierung erstellen
    String aBaseName;
    Reference<XConnection> xCon = GetView()->getController().getConnection();
    Reference< XDatabaseMetaData> xMetaData = xCon.is() ? xCon->getMetaData() : Reference< XDatabaseMetaData>();

    xub_StrLen nMaxTextLen((xub_StrLen)( xMetaData.is() ? xMetaData->getMaxColumnNameLength() : 0));

    if( (rName.Len()+2) >nMaxTextLen )
        aBaseName = rName.Copy( 0, nMaxTextLen-2 );
    else
        aBaseName = rName;

    //////////////////////////////////////////////////////////////////////
    // Namen durchnumerieren (bis 99)
    String aFieldName( rName);
    sal_Int32 i=1;
    while( HasFieldName(aFieldName) )
    {
        aFieldName = aBaseName;
        aFieldName += OUString::number(i);
        i++;
    }

    return aFieldName;
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::InsertRows( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    ::std::vector<  ::boost::shared_ptr<OTableRow> > vInsertedUndoRedoRows; // need for undo/redo handling
    //////////////////////////////////////////////////////////////////////
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
                //////////////////////////////////////////////////////////////////////
                // Anpassen des Feldnamens
                aFieldName = GenerateName( pRow->GetActFieldDescr()->GetName() );
                pRow->GetActFieldDescr()->SetName( aFieldName );
                pRow->SetPos(nInsertRow);
                m_pRowList->insert( m_pRowList->begin()+nInsertRow,pRow );
                vInsertedUndoRedoRows.push_back(::boost::shared_ptr<OTableRow>(new OTableRow(*pRow)));
                nInsertRow++;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////
    // Beim RowInserted wird CursorMoved gerufen.
    // Die UI-Daten duerfen hier beim CursorMoved nicht gespeichert werden.
    bSaveOnMove = sal_False;
    RowInserted( nRow,vInsertedUndoRedoRows.size(),sal_True );
    bSaveOnMove = sal_True;

    //////////////////////////////////////////////////////////////////////
    // Undo-Action erzeugen
    GetUndoManager().AddUndoAction( new OTableEditorInsUndoAct(this, nRow,vInsertedUndoRedoRows) );
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::DeleteRows()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OSL_ENSURE(GetView()->getController().isDropAllowed(),"Call of DeleteRows not valid here. Please check isDropAllowed!");
    //////////////////////////////////////////////////////////////////////
    // Undo-Action erzeugen
    GetUndoManager().AddUndoAction( new OTableEditorDelUndoAct(this) );


    //////////////////////////////////////////////////////////////////////
    // Alle markierten Zeilen loeschen
    long nIndex = FirstSelectedRow();
    nOldDataPos = nIndex;
    bSaveOnMove = sal_False;

    while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
    {
        //////////////////////////////////////////////////////////////////////
        // Zeile entfernen
        m_pRowList->erase( m_pRowList->begin()+nIndex );
        RowRemoved( nIndex, 1, sal_True );

        //////////////////////////////////////////////////////////////////////
        // Leerzeile am Ende wieder einfuegen
        m_pRowList->push_back( ::boost::shared_ptr<OTableRow>(new OTableRow()));
        RowInserted( GetRowCount()-1, 1, sal_True );

        nIndex = FirstSelectedRow();
    }

    bSaveOnMove = sal_True;

    //////////////////////////////////////////////////////////////////////
    // Erzwingen, dass der aktuelle Datensatz angezeigt wird
    m_nDataPos = GetCurRow();
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );
    SetDataPtr( m_nDataPos );
    ActivateCell();
    pDescrWin->DisplayData( pActRow->GetActFieldDescr() );
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::InsertNewRows( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OSL_ENSURE(GetView()->getController().isAddAllowed(),"Call of InsertNewRows not valid here. Please check isAppendAllowed!");
    //////////////////////////////////////////////////////////////////////
    // Undo-Action erzeugen
    long nInsertRows = GetSelectRowCount();
    if( !nInsertRows )
        nInsertRows = 1;
    GetUndoManager().AddUndoAction( new OTableEditorInsNewUndoAct(this, nRow, nInsertRows) );
    //////////////////////////////////////////////////////////////////////
    // Zahl der selektierten Zeilen werden neu eingefuegt
    for( long i=nRow; i<(nRow+nInsertRows); i++ )
        m_pRowList->insert( m_pRowList->begin()+i ,::boost::shared_ptr<OTableRow>(new OTableRow()));
    RowInserted( nRow, nInsertRows, sal_True );

    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

//------------------------------------------------------------------------------
String OTableEditorCtrl::GetControlText( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Controls des Browsers auslesen
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

    //////////////////////////////////////////////////////////////////////
    // Controls der Tabpage Auslesen
    else
        return pDescrWin->GetControlText( nColId );
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::SetControlText( long nRow, sal_uInt16 nColId, const String& rText )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Controls des Browsers setzen
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

    //////////////////////////////////////////////////////////////////////
    // Controls der Tabpage setzen
    else
    {
        pDescrWin->SetControlText( nColId, rText );
    }
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Aktuellen Datenzeiger umsetzen
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    //////////////////////////////////////////////////////////////////////
    // Einzelne Felder setzen
    switch( nColId )
    {
        case FIELD_TYPE:
            SwitchType( _pTypeInfo );
            break;
        default:
            OSL_FAIL("OTableEditorCtrl::SetCellData: invalid column!");
    }
    SetControlText(nRow,nColId,_pTypeInfo.get() ? _pTypeInfo->aUIName : ::rtl::OUString());
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::SetCellData( long nRow, sal_uInt16 nColId, const ::com::sun::star::uno::Any& _rNewData )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Aktuellen Datenzeiger umsetzen
    if( nRow == -1 )
        nRow = GetCurRow();
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr && nColId != FIELD_TYPE)
        return;

    String sValue;
    //////////////////////////////////////////////////////////////////////
    // Einzelne Felder setzen
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
            pFieldDescr->SetControlDefault(makeAny(::rtl::OUString(sValue)));
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

//------------------------------------------------------------------------------
Any OTableEditorCtrl::GetCellData( long nRow, sal_uInt16 nColId )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    OFieldDescription* pFieldDescr = GetFieldDescr( nRow );
    if( !pFieldDescr )
        return Any();

    //////////////////////////////////////////////////////////////////////
    // Aktuellen Datenzeiger umsetzen
    if( nRow==-1 )
        nRow = GetCurRow();
    SetDataPtr( nRow );

    static const String strYes(ModuleRes(STR_VALUE_YES));
    static const String strNo(ModuleRes(STR_VALUE_NO));
    ::rtl::OUString sValue;
    //////////////////////////////////////////////////////////////////////
    // Einzelne Felder auslesen
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

//------------------------------------------------------------------------------
String OTableEditorCtrl::GetCellText( long nRow, sal_uInt16 nColId ) const
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    ::rtl::OUString sCellText;
    const_cast< OTableEditorCtrl* >( this )->GetCellData( nRow, nColId ) >>= sCellText;
    return sCellText;
}

//------------------------------------------------------------------------------
sal_uInt32 OTableEditorCtrl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    return GetTextWidth(GetCellText(nRow, nColId)) + 2 * GetTextWidth(rtl::OUString('0'));
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

        //////////////////////////////////////////////////////////////////////
        // Wenn eine der markierten Zeilen leer ist, kein Copy moeglich
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::IsDeleteAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    return GetSelectRowCount() != 0 && GetView()->getController().isDropAllowed();
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::IsInsertNewAllowed( long nRow )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);

    sal_Bool bInsertNewAllowed = GetView()->getController().isAddAllowed();
    //////////////////////////////////////////////////////////////
    // Wenn nur Felder hinzugefuegt werden duerfen, Paste nur in neue Felder
    if (bInsertNewAllowed && !GetView()->getController().isDropAllowed())
    {
        SetDataPtr(nRow);
        if( GetActRow()->IsReadOnly() )
            return sal_False;
    }

    return bInsertNewAllowed;
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::IsPrimaryKeyAllowed( long /*nRow*/ )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    if( !GetSelectRowCount() )
        return sal_False;

    OTableController& rController = GetView()->getController();
    if ( !rController.getSdbMetaData().supportsPrimaryKeys() )
        return sal_False;

    Reference<XPropertySet> xTable = rController.getTable();
    //////////////////////////////////////////////////////////////
    // Key darf nicht veraendert werden
    // Dies gilt jedoch nur, wenn die Tabelle nicht neu ist und keine ::com::sun::star::sdbcx::View. Ansonsten wird kein DROP ausgefuehrt

    if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW")
        return sal_False;
    //////////////////////////////////////////////////////////////
    // Wenn leeres Feld, kein PrimKey
    // Eintrag wird nur erlaubt, wenn
    // - kein leerer Eintrag in der Selection ist
    // - kein Eintrag vom Typ Memo oder Image ist
    // - kein DROP erlaubt ist (s.o.) und die Spalte noch kein Required (not null) gesetzt hatte.
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
            //////////////////////////////////////////////////////////////
            // Wenn Feldtyp Memo oder Image, kein PrimKey
            // oder wenn Spalten nicht gedroped werden k�nnen und das Required Flag ist nicht gesetzt
            // oder wenn eine ::com::sun::star::sdbcx::View vorhanden ist und das Required Flag nicht gesetzt ist
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

//------------------------------------------------------------------------------
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

            //////////////////////////////////////////////////////////////
            // Kontextmenu einblenden
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

                    // jetzt alles, was disabled wurde, wech
                    aContextMenu.RemoveDisabledEntries(sal_True, sal_True);

                    if( SetDataPtr(m_nDataPos) )
                        pDescrWin->SaveData( pActRow->GetActFieldDescr() );

                    //////////////////////////////////////////////////////////////
                    // Alle Aktionen, die die Zeilenzahl veraendern, muessen asynchron
                    // ausgefuehrt werden->sonst Probleme zwischen Kontextmenu u. Browser
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

//------------------------------------------------------------------------------
IMPL_LINK( OTableEditorCtrl, DelayedCut, void*, /*EMPTYTAG*/ )
{
    nCutEvent = 0;
    OTableRowView::cut();
    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableEditorCtrl, DelayedPaste, void*, /*EMPTYTAG*/ )
{
    nPasteEvent = 0;

    sal_Int32 nPastePosition = GetView()->getController().getFirstEmptyRowPosition();
    if ( !GetView()->getController().getTable().is() )
        nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : GetCurRow();

    if (!IsInsertNewAllowed(nPastePosition))
    {   // kein Einfuegen erlaubt, sondern nur anhaengen, also testen, ob hinter der PastePosition noch
        // belegte Zeilen erscheinen

        sal_Int32 nFreeFromPos; // ab da nur freie Zeilen
        ::std::vector< ::boost::shared_ptr<OTableRow> >::reverse_iterator aIter = m_pRowList->rbegin();
        for(nFreeFromPos = m_pRowList->size();
            aIter != m_pRowList->rend() && (!(*aIter) || !(*aIter)->GetActFieldDescr() || (*aIter)->GetActFieldDescr()->GetName().isEmpty());
            --nFreeFromPos, ++aIter)
            ;
        if (nPastePosition < nFreeFromPos)  // es gibt mindestens eine belegte hinter PastePosition -> ganz nach hinten
            nPastePosition = nFreeFromPos;
    }

    OTableRowView::Paste( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OTableEditorCtrl, DelayedDelete, void*, /*EMPTYTAG*/ )
{
    nDeleteEvent = 0;
    DeleteRows();
    return 0;
}

//------------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
    //////////////////////////////////////////////////////////////////////
    // update field description
    pDescrWin->DisplayData(_pFieldDesc);

    _rMultiSel.Insert( _nPos );
    _rMultiSel.Select( _nPos );
}
//------------------------------------------------------------------------------
void OTableEditorCtrl::SetPrimaryKey( sal_Bool bSet )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Evtl. vorhandene Primary Keys loeschen
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

    //////////////////////////////////////////////////////////////////////
    // Die Primary Keys der markierten Zeilen setzen
    MultiSelection aInsertedPrimKeys;
    aInsertedPrimKeys.SetTotalRange( Range(0,GetRowCount()) );
    if( bSet )
    {
        long nIndex = FirstSelectedRow();
        while( nIndex >= 0 && nIndex < static_cast<long>(m_pRowList->size()) )
        {
            //////////////////////////////////////////////////////////////////////
            // Key setzen
             ::boost::shared_ptr<OTableRow>  pRow = (*m_pRowList)[nIndex];
            OFieldDescription* pFieldDescr = pRow->GetActFieldDescr();
            if(pFieldDescr)
                AdjustFieldDescription(pFieldDescr,aInsertedPrimKeys,nIndex,sal_False,sal_True);

            nIndex = NextSelectedRow();
        }
    }

    GetUndoManager().AddUndoAction( new OPrimKeyUndoAct(this, aDeletedPrimKeys, aInsertedPrimKeys) );

    //////////////////////////////////////////////////////////////////////
    // Handle-Spalte invalidieren
    InvalidateHandleColumn();


    //////////////////////////////////////////////////////////////////////
    // Das ModifyFlag der TableDocSh setzen
    GetView()->getController().setModified( sal_True );
    InvalidateFeatures();
}

//------------------------------------------------------------------------------
sal_Bool OTableEditorCtrl::IsPrimaryKey()
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Gehoeren alle markierten Felder zu einem Primary Key ?
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

    //////////////////////////////////////////////////////////////////////
    // Gibt es unselektierte Felder, die noch zu dem Key gehoeren ?
    return GetSelectRowCount() == nPrimaryKeys;
}

//------------------------------------------------------------------------------
void OTableEditorCtrl::SwitchType( const TOTypeInfoSP& _pType )
{
    DBG_CHKTHIS(OTableEditorCtrl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Wenn noch kein Feldname vergeben wurde
    long nRow(GetCurRow());
    OFieldDescription* pActFieldDescr = GetFieldDescr( nRow );
    if( pActFieldDescr )
        //////////////////////////////////////////////////////////////////////
        // Alte Beschreibung speichern
        pDescrWin->SaveData( pActFieldDescr );

    if ( nRow < 0 || nRow > static_cast<long>(m_pRowList->size()) )
        return;
    //////////////////////////////////////////////////////////////////////
    // Neue Beschreibung darstellen
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
// -----------------------------------------------------------------------------
OTableDesignView* OTableEditorCtrl::GetView() const
{
    return static_cast<OTableDesignView*>(GetParent()->GetParent());
}
// -----------------------------------------------------------------------------
void OTableEditorCtrl::DeactivateCell(sal_Bool bUpdate)
{
    OTableRowView::DeactivateCell(bUpdate);
    // now we have to deactivate the field description
    long nRow(GetCurRow());
    if (pDescrWin)
        pDescrWin->SetReadOnly(bReadOnly || !SetDataPtr(nRow) || GetActRow()->IsReadOnly());
}
//------------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
