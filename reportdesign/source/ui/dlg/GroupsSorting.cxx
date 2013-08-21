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
#include "GroupsSorting.hxx"
#include "GroupsSorting.hrc"
#include <connectivity/dbtools.hxx>
#include <svtools/editbrowsebox.hxx>
#include <svtools/imgdef.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <tools/debug.hxx>
#include "RptResId.hrc"
#include "rptui_slotid.hrc"
#include "ModuleHelper.hxx"
#include "helpids.hrc"
#include "GroupExchange.hxx"
#include "UITools.hxx"
#include "UndoActions.hxx"
#include "uistrings.hrc"
#include "ReportController.hxx"
#include "ColumnInfo.hxx"

#include <cppuhelper/implbase1.hxx>
#include <comphelper/property.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/msgbox.hxx>
#include <algorithm>

#include <cppuhelper/bootstrap.hxx>

#define HANDLE_ID           0
#define FIELD_EXPRESSION    1
#define GROUPS_START_LEN    5
#define NO_GROUP            -1

namespace rptui
{
using namespace ::com::sun::star;
using namespace svt;
using namespace ::comphelper;

    void lcl_addToList_throw( ComboBoxControl& _rListBox, ::std::vector<ColumnInfo>& o_aColumnList,const uno::Reference< container::XNameAccess>& i_xColumns )
    {
        uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
        const OUString* pEntries = aEntries.getConstArray();
        sal_Int32 nEntries = aEntries.getLength();
        for ( sal_Int32 i = 0; i < nEntries; ++i, ++pEntries )
        {
            uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(*pEntries),uno::UNO_QUERY_THROW);
            OUString sLabel;
            if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            o_aColumnList.push_back( ColumnInfo(*pEntries,sLabel) );
            if ( !sLabel.isEmpty() )
                _rListBox.InsertEntry( sLabel );
            else
                _rListBox.InsertEntry( *pEntries );
        }
    }

typedef ::svt::EditBrowseBox OFieldExpressionControl_Base;
typedef ::cppu::WeakImplHelper1< container::XContainerListener > TContainerListenerBase;
class OFieldExpressionControl : public TContainerListenerBase
                               ,public OFieldExpressionControl_Base
{
    ::osl::Mutex                    m_aMutex;
    ::std::vector<sal_Int32>        m_aGroupPositions;
    ::std::vector<ColumnInfo>       m_aColumnInfo;
    ::svt::ComboBoxControl*         m_pComboCell;
    sal_Int32                       m_nDataPos;
    sal_Int32                       m_nCurrentPos;
    sal_uLong                           m_nPasteEvent;
    sal_uLong                           m_nDeleteEvent;
    OGroupsSortingDialog*           m_pParent;
    bool                            m_bIgnoreEvent;

    void fillListBox(const uno::Reference< beans::XPropertySet>& _xDest,long nRow,sal_uInt16 nColumnId);
    sal_Bool SaveModified(bool _bAppend);

    OFieldExpressionControl(const OFieldExpressionControl&); // NO COPY
    void operator =(const OFieldExpressionControl&);         // NO ASSIGN
public:
    OFieldExpressionControl( OGroupsSortingDialog* _pParent,const ResId& _rResId);
    virtual ~OFieldExpressionControl();

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );
    // XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);

    void        fillColumns(const uno::Reference< container::XNameAccess>& _xColumns);
    void        lateInit();
    sal_Bool    IsDeleteAllowed( );
    void        DeleteRows();
    void        cut();
    void        copy();
    void        paste();

    inline sal_Int32   getGroupPosition(sal_Int32 _nRow) const { return _nRow != BROWSER_ENDOFSELECTION ? m_aGroupPositions[_nRow] : sal_Int32(NO_GROUP); }

    inline ::svt::ComboBoxControl*  getExpressionControl() const { return m_pComboCell; }

    /** returns the sequence with the selected groups
    */
    uno::Sequence<uno::Any> fillSelectedGroups();

    /** move groups given by _aGroups
    */
    void moveGroups(const uno::Sequence<uno::Any>& _aGroups,sal_Int32 _nRow,sal_Bool _bSelect = sal_True);

    virtual sal_Bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);
    using OFieldExpressionControl_Base::GetRowCount;
protected:
    virtual sal_Bool IsTabAllowed(sal_Bool bForward) const;

    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol );
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol );
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const;
    virtual sal_Bool SeekRow( long nRow );
    virtual sal_Bool SaveModified();
    virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const;
    virtual RowStatus GetRowStatus(long nRow) const;

    virtual void KeyInput(const KeyEvent& rEvt);
    virtual void Command( const CommandEvent& rEvt );

    // D&D
    virtual void     StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual sal_Int8 AcceptDrop( const BrowserAcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const BrowserExecuteDropEvent& rEvt );

    using BrowseBox::AcceptDrop;
    using BrowseBox::ExecuteDrop;

private:

    DECL_LINK( DelayedPaste, void* );
    DECL_LINK( CBChangeHdl,ComboBox*);

    void InsertRows( long nRow );

public:
    DECL_LINK( DelayedDelete, void* );

};
//========================================================================
// class OFieldExpressionControl
//========================================================================
DBG_NAME( rpt_OFieldExpressionControl )
//------------------------------------------------------------------------
OFieldExpressionControl::OFieldExpressionControl( OGroupsSortingDialog* _pParent,const ResId& _rResId )
    :EditBrowseBox( _pParent, _rResId,EBBF_NONE, WB_TABSTOP | BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_AUTOSIZE_LASTCOL |
                                  BROWSER_KEEPSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL)
    ,m_aGroupPositions(GROUPS_START_LEN,-1)
    ,m_pComboCell(NULL)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nPasteEvent(0)
    ,m_nDeleteEvent(0)
    ,m_pParent(_pParent)
    ,m_bIgnoreEvent(false)
{
    DBG_CTOR( rpt_OFieldExpressionControl,NULL);
    SetBorderStyle(WINDOW_BORDER_MONO);
}

//------------------------------------------------------------------------
OFieldExpressionControl::~OFieldExpressionControl()
{
    acquire();
    uno::Reference< report::XGroups > xGroups = m_pParent->getGroups();
    xGroups->removeContainerListener(this);
    //////////////////////////////////////////////////////////////////////
    // delete events from queue
    if( m_nPasteEvent )
        Application::RemoveUserEvent( m_nPasteEvent );
    if( m_nDeleteEvent )
        Application::RemoveUserEvent( m_nDeleteEvent );

    delete m_pComboCell;
    DBG_DTOR( rpt_OFieldExpressionControl,NULL);
}
//------------------------------------------------------------------------------
uno::Sequence<uno::Any> OFieldExpressionControl::fillSelectedGroups()
{
    uno::Sequence<uno::Any> aList;
    ::std::vector<uno::Any> vClipboardList;
    vClipboardList.reserve(GetSelectRowCount());

    uno::Reference<report::XGroups> xGroups = m_pParent->getGroups();
    sal_Int32 nCount = xGroups->getCount();
    if ( nCount >= 1 )
    {
        for( long nIndex=FirstSelectedRow(); nIndex >= 0 ; nIndex=NextSelectedRow() )
        {
            try
            {
                if ( m_aGroupPositions[nIndex] != NO_GROUP )
                {
                    uno::Reference< report::XGroup> xOrgGroup(xGroups->getByIndex(m_aGroupPositions[nIndex]),uno::UNO_QUERY);
                    /*uno::Reference< report::XGroup> xCopy = xGroups->createGroup();
                    ::comphelper::copyProperties(xOrgGroup.get(),xCopy.get());*/
                    vClipboardList.push_back( uno::makeAny(xOrgGroup) );
                }
            }
            catch(uno::Exception&)
            {
                OSL_FAIL("Can not access group!");
            }
        }
        if ( !vClipboardList.empty() )
            aList = uno::Sequence< uno::Any >(&vClipboardList[0], vClipboardList.size());
    }
    return aList;
}
//------------------------------------------------------------------------------
void OFieldExpressionControl::StartDrag( sal_Int8 /*_nAction*/ , const Point& /*_rPosPixel*/ )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    if ( m_pParent && !m_pParent->isReadOnly( ) )
    {
        uno::Sequence<uno::Any> aClipboardList = fillSelectedGroups();

        if( aClipboardList.getLength() )
        {
            OGroupExchange* pData = new OGroupExchange(aClipboardList);
            uno::Reference< ::com::sun::star::datatransfer::XTransferable> xRef = pData;
            pData->StartDrag(this, DND_ACTION_MOVE );
        }
    }
}
//------------------------------------------------------------------------------
sal_Int8 OFieldExpressionControl::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    sal_Int8 nAction = DND_ACTION_NONE;
    if ( IsEditing() )
    {
        sal_uInt16 nPos = m_pComboCell->GetSelectEntryPos();
        if ( COMBOBOX_ENTRY_NOTFOUND != nPos || !m_pComboCell->GetText().isEmpty() )
            SaveModified();
        DeactivateCell();
    }
    if ( IsDropFormatSupported( OGroupExchange::getReportGroupId() ) && m_pParent->getGroups()->getCount() > 1 && rEvt.GetWindow() == &GetDataWindow() )
    {
        nAction = DND_ACTION_MOVE;
    }
    return nAction;
}
//------------------------------------------------------------------------------
sal_Int8 OFieldExpressionControl::ExecuteDrop( const BrowserExecuteDropEvent& rEvt )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    sal_Int8 nAction = DND_ACTION_NONE;
    if ( IsDropFormatSupported( OGroupExchange::getReportGroupId() ) )
    {
        sal_Int32   nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), sal_False);
        SetNoSelection();

        TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
        uno::Any aDrop = aDropped.GetAny(OGroupExchange::getReportGroupId());
        uno::Sequence< uno::Any > aGroups;
        aDrop >>= aGroups;
        if ( aGroups.getLength() )
        {
            moveGroups(aGroups,nRow);
            nAction = DND_ACTION_MOVE;
        }
    }
    return nAction;
}
//------------------------------------------------------------------------------
void OFieldExpressionControl::moveGroups(const uno::Sequence<uno::Any>& _aGroups,sal_Int32 _nRow,sal_Bool _bSelect)
{
    if ( _aGroups.getLength() )
    {
        m_bIgnoreEvent = true;
        {
            sal_Int32 nRow = _nRow;
            const String sUndoAction(ModuleRes(RID_STR_UNDO_MOVE_GROUP));
            const UndoContext aUndoContext( m_pParent->m_pController->getUndoManager(), sUndoAction );

            uno::Reference< report::XGroups> xGroups = m_pParent->getGroups();
            const uno::Any* pIter = _aGroups.getConstArray();
            const uno::Any* pEnd  = pIter + _aGroups.getLength();
            for(;pIter != pEnd;++pIter)
            {
                uno::Reference< report::XGroup> xGroup(*pIter,uno::UNO_QUERY);
                if ( xGroup.is() )
                {
                    uno::Sequence< beans::PropertyValue > aArgs(1);
                    aArgs[0].Name = PROPERTY_GROUP;
                    aArgs[0].Value <<= xGroup;
                    // we use this way to create undo actions
                    m_pParent->m_pController->executeChecked(SID_GROUP_REMOVE,aArgs);
                    aArgs.realloc(2);
                    if ( nRow > xGroups->getCount() )
                        nRow = xGroups->getCount();
                    if ( _bSelect )
                        SelectRow(nRow);
                    aArgs[1].Name = PROPERTY_POSITIONY;
                    aArgs[1].Value <<= nRow;
                    m_pParent->m_pController->executeChecked(SID_GROUP_APPEND,aArgs);
                    ++nRow;
                }
            }
        }
        m_bIgnoreEvent = false;
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OFieldExpressionControl::fillColumns(const uno::Reference< container::XNameAccess>& _xColumns)
{
    m_pComboCell->Clear();
    if ( _xColumns.is() )
        lcl_addToList_throw(*m_pComboCell,m_aColumnInfo,_xColumns);
}
//------------------------------------------------------------------------------
void OFieldExpressionControl::lateInit()
{
    uno::Reference< report::XGroups > xGroups = m_pParent->getGroups();
    sal_Int32 nGroupsCount = xGroups->getCount();
    m_aGroupPositions.resize(::std::max<sal_Int32>(nGroupsCount,sal_Int32(GROUPS_START_LEN)),NO_GROUP);
    ::std::vector<sal_Int32>::iterator aIter = m_aGroupPositions.begin();
    for (sal_Int32 i = 0; i < nGroupsCount; ++i,++aIter)
        *aIter = i;

    if ( ColCount() == 0 )
    {
        Font aFont( GetDataWindow().GetFont() );
        aFont.SetWeight( WEIGHT_NORMAL );
        GetDataWindow().SetFont( aFont );

        // Font fuer die Ueberschriften auf Light setzen
        aFont = GetFont();
        aFont.SetWeight( WEIGHT_LIGHT );
        SetFont(aFont);

        InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth(OUString('0')) * 4)/*, sal_True */);
        InsertDataColumn( FIELD_EXPRESSION, String(ModuleRes(STR_RPT_EXPRESSION)), 100);

        m_pComboCell = new ComboBoxControl( &GetDataWindow() );
        m_pComboCell->SetSelectHdl(LINK(this,OFieldExpressionControl,CBChangeHdl));
        m_pComboCell->SetHelpId(HID_RPT_FIELDEXPRESSION);

        Control* pControls[] = {m_pComboCell};
        for (size_t i = 0; i < sizeof(pControls)/sizeof(pControls[0]); ++i)
        {
            pControls[i]->SetGetFocusHdl(LINK(m_pParent, OGroupsSortingDialog, OnControlFocusGot));
            pControls[i]->SetLoseFocusHdl(LINK(m_pParent, OGroupsSortingDialog, OnControlFocusLost));
        }

        //////////////////////////////////////////////////////////////////////
        // set browse mode
        BrowserMode nMode(BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION  | BROWSER_KEEPSELECTION |
                          BROWSER_HLINESFULL | BROWSER_VLINESFULL       | BROWSER_AUTOSIZE_LASTCOL | BROWSER_AUTO_VSCROLL | BROWSER_AUTO_HSCROLL);
        if( m_pParent->isReadOnly() )
            nMode |= BROWSER_HIDECURSOR;
        SetMode(nMode);
        xGroups->addContainerListener(this);
    }
    else
        // not the first call
        RowRemoved(0, GetRowCount());

    RowInserted(0, m_aGroupPositions.size(), sal_True);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
IMPL_LINK( OFieldExpressionControl, CBChangeHdl, ComboBox*, /*pComboBox*/ )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);

    SaveModified();
    return 0L;
}

//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::IsTabAllowed(sal_Bool /*bForward*/) const
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::SaveModified()
{
    return SaveModified(true);
}
//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::SaveModified(bool _bAppendRow)
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    sal_Int32 nRow = GetCurRow();
    if ( nRow != BROWSER_ENDOFSELECTION )
    {
        sal_Bool bAppend = sal_False;
        try
        {
            uno::Reference< report::XGroup> xGroup;
            if ( m_aGroupPositions[nRow] == NO_GROUP )
            {
                bAppend = sal_True;
                String sUndoAction(ModuleRes(RID_STR_UNDO_APPEND_GROUP));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, String() );
                xGroup = m_pParent->getGroups()->createGroup();
                xGroup->setHeaderOn(sal_True);

                uno::Sequence< beans::PropertyValue > aArgs(2);
                aArgs[0].Name = PROPERTY_GROUP;
                aArgs[0].Value <<= xGroup;
                // find position where to insert the new group
                sal_Int32 nGroupPos = 0;
                ::std::vector<sal_Int32>::iterator aIter = m_aGroupPositions.begin();
                ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.begin() + nRow;
                for(;aIter != aEnd;++aIter)
                    if ( *aIter != NO_GROUP )
                        nGroupPos = *aIter + 1;
                aArgs[1].Name = PROPERTY_POSITIONY;
                aArgs[1].Value <<= nGroupPos;
                m_bIgnoreEvent = true;
                m_pParent->m_pController->executeChecked(SID_GROUP_APPEND,aArgs);
                m_bIgnoreEvent = false;
                OSL_ENSURE(*aIter == NO_GROUP ,"Illegal iterator!");
                *aIter++ = nGroupPos;

                aEnd  = m_aGroupPositions.end();
                for(;aIter != aEnd;++aIter)
                    if ( *aIter != NO_GROUP )
                        ++*aIter;
            }
            else
                xGroup = m_pParent->getGroup(m_aGroupPositions[nRow]);
            if ( xGroup.is() )
            {
                sal_uInt16 nPos = m_pComboCell->GetSelectEntryPos();
                OUString sExpression;
                if ( COMBOBOX_ENTRY_NOTFOUND == nPos )
                    sExpression = m_pComboCell->GetText();
                else
                {
                    sExpression = m_aColumnInfo[nPos].sColumnName;
                }
                xGroup->setExpression( sExpression );

                ::rptui::adjustSectionName(xGroup,nPos);

                if ( bAppend )
                    m_pParent->m_pController->getUndoManager().LeaveListAction();
            }

            if ( Controller() )
                Controller()->ClearModified();
            if ( _bAppendRow && GetRowCount() == m_pParent->getGroups()->getCount() )
            {
                RowInserted( GetRowCount()-1);
                m_aGroupPositions.push_back(NO_GROUP);
            }

            GoToRow(nRow);
            m_pParent->DisplayData(nRow);
        }
        catch(uno::Exception&)
        {
            OSL_FAIL("OFieldExpressionControl::SaveModified: Exception caught!");
        }
    }

    return sal_True;
}
//------------------------------------------------------------------------------
OUString OFieldExpressionControl::GetCellText( long nRow, sal_uInt16 /*nColId*/ ) const
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    OUString sText;
    if ( nRow != BROWSER_ENDOFSELECTION && m_aGroupPositions[nRow] != NO_GROUP )
    {
        try
        {
            uno::Reference< report::XGroup> xGroup = m_pParent->getGroup(m_aGroupPositions[nRow]);
            OUString sExpression = xGroup->getExpression();

            for(::std::vector<ColumnInfo>::const_iterator aIter = m_aColumnInfo.begin(); aIter != m_aColumnInfo.end();++aIter)
            {
                if ( aIter->sColumnName == sExpression )
                {
                    if ( !aIter->sLabel.isEmpty() )
                        sExpression = aIter->sLabel;
                    break;
                }
            }
            sText = sExpression;
        }
        catch (const uno::Exception&)
        {
            OSL_FAIL("Exception caught while getting expression value from the group");
        }
    }
    return sText;
}

//------------------------------------------------------------------------------
void OFieldExpressionControl::InitController( CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColumnId )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);

    m_pComboCell->SetText( GetCellText( nRow, nColumnId ) );
}
//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);

    if (!EditBrowseBox::CursorMoving(nNewRow, nNewCol))
        return sal_False;
    m_nDataPos = nNewRow;
    long nOldDataPos = GetCurRow();
    InvalidateStatusCell( m_nDataPos );
    InvalidateStatusCell( nOldDataPos );

    m_pParent->SaveData( nOldDataPos );
    m_pParent->DisplayData( m_nDataPos );
    return sal_True;
}
//------------------------------------------------------------------------------
CellController* OFieldExpressionControl::GetController( long /*nRow*/, sal_uInt16 /*nColumnId*/ )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    ComboBoxCellController* pCellController = new ComboBoxCellController( m_pComboCell );
    pCellController->GetComboBox().SetReadOnly(!m_pParent->m_pController->isEditable());
    return pCellController;
}

//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::SeekRow( long _nRow )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    // die Basisklasse braucht den Aufruf, da sie sich dort merkt, welche Zeile gepainted wird
    EditBrowseBox::SeekRow(_nRow);
    m_nCurrentPos = _nRow;
    return sal_True;
}

//------------------------------------------------------------------------------
void OFieldExpressionControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    String aText  =const_cast< OFieldExpressionControl*>(this)->GetCellText( m_nCurrentPos, nColumnId );

    Point aPos( rRect.TopLeft() );
    Size aTextSize( GetDataWindow().GetTextHeight(),GetDataWindow().GetTextWidth( aText ));

    if( aPos.X() < rRect.Right() || aPos.X() + aTextSize.Width() > rRect.Right() ||
        aPos.Y() < rRect.Top() || aPos.Y() + aTextSize.Height() > rRect.Bottom() )
        rDev.SetClipRegion(Region(rRect));

    rDev.DrawText( aPos, aText );

    if( rDev.IsClipRegion() )
        rDev.SetClipRegion();
}
//------------------------------------------------------------------------------
EditBrowseBox::RowStatus OFieldExpressionControl::GetRowStatus(long nRow) const
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    if (nRow >= 0 && nRow == m_nDataPos)
        return EditBrowseBox::CURRENT;
    if ( nRow != BROWSER_ENDOFSELECTION && nRow < static_cast<long>(m_aGroupPositions.size()) && m_aGroupPositions[nRow] != NO_GROUP )
    {
        try
        {
            uno::Reference< report::XGroup> xGroup = m_pParent->getGroup(m_aGroupPositions[nRow]);
            return (xGroup->getHeaderOn() || xGroup->getFooterOn())? EditBrowseBox::HEADERFOOTER : EditBrowseBox::CLEAN;
        }
        catch(uno::Exception&)
        {
            OSL_FAIL("Exception cathced while try to get a group!");
        }
    }
    return EditBrowseBox::CLEAN;
}
//  XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OFieldExpressionControl::disposing(const lang::EventObject& /*e*/) throw( uno::RuntimeException )
{
}
//------------------------------------------------------------------------------
// XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL OFieldExpressionControl::elementInserted(const container::ContainerEvent& evt) throw(uno::RuntimeException)
{
    if ( m_bIgnoreEvent )
        return;
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );
    sal_Int32 nGroupPos = 0;
    if ( evt.Accessor >>= nGroupPos )
    {
        if ( nGroupPos >= GetRowCount() )
        {
            sal_Int32 nAddedRows = nGroupPos - GetRowCount();
            RowInserted(nAddedRows);
            for (sal_Int32 i = 0; i < nAddedRows; ++i)
                m_aGroupPositions.push_back(NO_GROUP);
            m_aGroupPositions[nGroupPos] = nGroupPos;
        }
        else
        {
            ::std::vector<sal_Int32>::iterator aFind = m_aGroupPositions.begin()+ nGroupPos;
            if ( aFind == m_aGroupPositions.end() )
                aFind = ::std::find(m_aGroupPositions.begin(),m_aGroupPositions.end(),NO_GROUP);

            if ( aFind != m_aGroupPositions.end() )
            {
                if ( *aFind != NO_GROUP )
                    aFind = m_aGroupPositions.insert(aFind,nGroupPos);
                else
                    *aFind = nGroupPos;

                ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.end();
                for(++aFind;aFind != aEnd;++aFind)
                    if ( *aFind != NO_GROUP )
                        ++*aFind;
            }
        }
        Invalidate();
    }
}
//------------------------------------------------------------------------------
void SAL_CALL OFieldExpressionControl::elementReplaced(const container::ContainerEvent& /*evt*/) throw(uno::RuntimeException)
{
}
//------------------------------------------------------------------------------
void SAL_CALL OFieldExpressionControl::elementRemoved(const container::ContainerEvent& evt) throw(uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bIgnoreEvent )
        return;

    sal_Int32 nGroupPos = 0;
    if ( evt.Accessor >>= nGroupPos )
    {
        ::std::vector<sal_Int32>::iterator aFind = ::std::find(m_aGroupPositions.begin(),m_aGroupPositions.end(),nGroupPos);
        if ( aFind != m_aGroupPositions.end() )
        {
            *aFind = NO_GROUP;
            ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.end();
            for(++aFind;aFind != aEnd;++aFind)
                if ( *aFind != NO_GROUP )
                    --*aFind;
            Invalidate();
        }
    }
}
//------------------------------------------------------------------------------
sal_Bool OFieldExpressionControl::IsDeleteAllowed( )
{
    return !m_pParent->isReadOnly() && GetSelectRowCount() > 0;
}
//------------------------------------------------------------------------
void OFieldExpressionControl::KeyInput( const KeyEvent& rEvt )
{
    if (IsDeleteAllowed())
    {
        if (rEvt.GetKeyCode().GetCode() == KEY_DELETE &&    // Delete rows
            !rEvt.GetKeyCode().IsShift() &&
            !rEvt.GetKeyCode().IsMod1())
        {
            DeleteRows();
            return;
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}
//------------------------------------------------------------------------
void OFieldExpressionControl::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if (!rEvt.IsMouseEvent())
            {
                EditBrowseBox::Command(rEvt);
                return;
            }

            sal_uInt16 nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());

            if ( nColId == HANDLE_ID )
            {
                PopupMenu aContextMenu(ModuleRes(RID_GROUPSROWPOPUPMENU));
                sal_Bool bEnable = sal_False;
                long nIndex = FirstSelectedRow();
                while( nIndex >= 0 && !bEnable )
                {
                    if ( m_aGroupPositions[nIndex] != NO_GROUP )
                        bEnable = sal_True;
                    nIndex = NextSelectedRow();
                }
                aContextMenu.EnableItem( SID_DELETE, IsDeleteAllowed() && bEnable );
                switch (aContextMenu.Execute(this, rEvt.GetMousePosPixel()))
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
                        if( m_nDeleteEvent )
                            Application::RemoveUserEvent( m_nDeleteEvent );
                        m_nDeleteEvent = Application::PostUserEvent( LINK(this, OFieldExpressionControl, DelayedDelete) );
                        break;
                    default:
                        break;
                }
            }
            // run through
        }
        default:
            EditBrowseBox::Command(rEvt);
    }

}
//------------------------------------------------------------------------------
void OFieldExpressionControl::DeleteRows()
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);

    sal_Bool bIsEditing = IsEditing();
    if (bIsEditing)
    {
        DeactivateCell();
    }
    long nIndex = FirstSelectedRow();
    if (nIndex == -1)
    {
        nIndex = GetCurRow();
    }
    bool bFirstTime = true;

    long nOldDataPos = nIndex;
    uno::Sequence< beans::PropertyValue > aArgs(1);
    aArgs[0].Name = PROPERTY_GROUP;
    m_bIgnoreEvent = true;
    while( nIndex >= 0 )
    {
        if ( m_aGroupPositions[nIndex] != NO_GROUP )
        {
            if ( bFirstTime )
            {
                bFirstTime = false;
                String sUndoAction(ModuleRes(RID_STR_UNDO_REMOVE_SELECTION));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, String() );
            }

            sal_Int32 nGroupPos = m_aGroupPositions[nIndex];
            uno::Reference< report::XGroup> xGroup = m_pParent->getGroup(nGroupPos);
            aArgs[0].Value <<= xGroup;
            // we use this way to create undo actions
            m_pParent->m_pController->executeChecked(SID_GROUP_REMOVE,aArgs);

            ::std::vector<sal_Int32>::iterator aFind = ::std::find(m_aGroupPositions.begin(),m_aGroupPositions.end(),nGroupPos);
            *aFind = NO_GROUP;
            ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.end();
            for(++aFind;aFind != aEnd;++aFind)
                if ( *aFind != NO_GROUP )
                    --*aFind;
        }
        nIndex = NextSelectedRow();
    }

    if ( !bFirstTime )
        m_pParent->m_pController->getUndoManager().LeaveListAction();

    m_nDataPos = GetCurRow();
    InvalidateStatusCell( nOldDataPos );
    InvalidateStatusCell( m_nDataPos );
    ActivateCell();
    m_pParent->DisplayData( m_nDataPos );
    m_bIgnoreEvent = false;
    Invalidate();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OFieldExpressionControl::cut()
{
    copy();
    DeleteRows();
}

//------------------------------------------------------------------------------
void OFieldExpressionControl::copy()
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // set to the right row and save it
    m_pParent->SaveData( m_nDataPos );

    uno::Sequence<uno::Any> aClipboardList = fillSelectedGroups();

    if( aClipboardList.getLength() )
    {
        OGroupExchange* pData = new OGroupExchange(aClipboardList);
        uno::Reference< ::com::sun::star::datatransfer::XTransferable> xRef = pData;
        pData->CopyToClipboard(GetParent());
    }
}

//------------------------------------------------------------------------------
void OFieldExpressionControl::paste()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(OGroupExchange::getReportGroupId()))
    {
        if( m_nPasteEvent )
            Application::RemoveUserEvent( m_nPasteEvent );
        m_nPasteEvent = Application::PostUserEvent( LINK(this, OFieldExpressionControl, DelayedPaste) );
    }
}
//------------------------------------------------------------------------------
IMPL_LINK( OFieldExpressionControl, DelayedPaste, void*,  )
{
    m_nPasteEvent = 0;

    sal_Int32 nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : GetCurRow();

    InsertRows( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );

    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OFieldExpressionControl, DelayedDelete, void*,  )
{
    m_nDeleteEvent = 0;
    DeleteRows();
    return 0;
}
//------------------------------------------------------------------------------
void OFieldExpressionControl::InsertRows( long nRow )
{
    DBG_CHKTHIS( rpt_OFieldExpressionControl,NULL);

    sal_Int32 nSize = 0;
    //////////////////////////////////////////////////////////////////////
    // get rows from clipboard
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(OGroupExchange::getReportGroupId()))
    {
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor(OGroupExchange::getReportGroupId(), aFlavor);
        uno::Sequence< uno::Any > aGroups;

        if( (aTransferData.GetAny(aFlavor) >>= aGroups) && aGroups.getLength() )
        {
            m_bIgnoreEvent = false;
            {
                const String sUndoAction(ModuleRes(RID_STR_UNDO_APPEND_GROUP));
                const UndoContext aUndoContext( m_pParent->m_pController->getUndoManager(), sUndoAction );

                uno::Reference<report::XGroups> xGroups = m_pParent->getGroups();
                sal_Int32 nGroupPos = 0;
                ::std::vector<sal_Int32>::iterator aIter = m_aGroupPositions.begin();
                ::std::vector<sal_Int32>::size_type nRowPos = static_cast< ::std::vector<sal_Int32>::size_type >(nRow);
                if ( nRowPos < m_aGroupPositions.size() )
                {
                    ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.begin() + nRowPos;
                    for(;aIter != aEnd;++aIter)
                    {
                        if ( *aIter != NO_GROUP )
                            nGroupPos = *aIter;
                    }
                }
                for(sal_Int32 i=0;i < aGroups.getLength();++i,++nSize)
                {
                    uno::Sequence< beans::PropertyValue > aArgs(2);
                    aArgs[0].Name = PROPERTY_GROUP;
                    aArgs[0].Value = aGroups[i];
                    aArgs[1].Name = PROPERTY_POSITIONY;
                    aArgs[1].Value <<= nGroupPos;
                    m_pParent->m_pController->executeChecked(SID_GROUP_APPEND,aArgs);

                    ::std::vector<sal_Int32>::iterator aInsertPos = m_aGroupPositions.insert(aIter,nGroupPos);
                    ++aInsertPos;
                    aIter = aInsertPos;
                    ::std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.end();
                    for(;aInsertPos != aEnd;++aInsertPos)
                        if ( *aInsertPos != NO_GROUP )
                            ++*aInsertPos;
                }
            }
            m_bIgnoreEvent = true;
        }
    }

    RowInserted( nRow,nSize,sal_True );
}
//------------------------------------------------------------------------------

DBG_NAME( rpt_OGroupsSortingDialog )
//========================================================================
// class OGroupsSortingDialog
//========================================================================
OGroupsSortingDialog::OGroupsSortingDialog( Window* _pParent
                                           ,sal_Bool _bReadOnly
                                           ,OReportController* _pController)
    : FloatingWindow( _pParent, ModuleRes(RID_GROUPS_SORTING) )
    ,OPropertyChangeListener(m_aMutex)
    ,m_aFL2(this, ModuleRes(FL_SEPARATOR2) )
    ,m_aMove(this, ModuleRes(FT_MOVELABEL) )
    ,m_aToolBox(this, ModuleRes(TB_TOOLBOX) )

    ,m_aFL3(this, ModuleRes(FL_SEPARATOR3) )
    ,m_aOrder(this, ModuleRes(FT_ORDER) )
    ,m_aOrderLst(this, ModuleRes(LST_ORDER) )
    ,m_aHeader(this, ModuleRes(FT_HEADER) )
    ,m_aHeaderLst(this, ModuleRes(LST_HEADERLST) )
    ,m_aFooter(this, ModuleRes(FT_FOOTER) )
    ,m_aFooterLst(this, ModuleRes(LST_FOOTERLST) )
    ,m_aGroupOn(this, ModuleRes(FT_GROUPON) )
    ,m_aGroupOnLst(this, ModuleRes(LST_GROUPONLST) )
    ,m_aGroupInterval(this, ModuleRes(FT_GROUPINTERVAL) )
    ,m_aGroupIntervalEd(this, ModuleRes(ED_GROUPINTERVALLST) )
    ,m_aKeepTogether(this, ModuleRes(FT_KEEPTOGETHER) )
    ,m_aKeepTogetherLst(this, ModuleRes(LST_KEEPTOGETHERLST) )
    ,m_aFL(this, ModuleRes(FL_SEPARATOR1) )
    ,m_aHelpWindow(this, ModuleRes(HELP_FIELD) )
    ,m_pFieldExpression( new OFieldExpressionControl(this,ModuleRes(WND_CONTROL)))
    ,m_pController(_pController)
    ,m_pCurrentGroupListener(NULL)
    ,m_xGroups(m_pController->getReportDefinition()->getGroups())
    ,m_bReadOnly(_bReadOnly)
{
    DBG_CTOR( rpt_OGroupsSortingDialog,NULL);

    Control* pControlsLst[] = { &m_aHeaderLst, &m_aFooterLst, &m_aGroupOnLst, &m_aKeepTogetherLst, &m_aOrderLst, &m_aGroupIntervalEd};
    for (size_t i = 0; i < sizeof (pControlsLst) / sizeof (pControlsLst[0]); ++i)
    {
        pControlsLst[i]->SetGetFocusHdl(LINK(this, OGroupsSortingDialog, OnControlFocusGot));
        pControlsLst[i]->SetLoseFocusHdl(LINK(this, OGroupsSortingDialog, OnControlFocusLost));
        pControlsLst[i]->Show(sal_True);
    }

    for (size_t i = 0; i < (sizeof (pControlsLst) / sizeof (pControlsLst[0])) - 1; ++i)
        static_cast<ListBox*>(pControlsLst[i])->SetSelectHdl(LINK(this,OGroupsSortingDialog,LBChangeHdl));

    Control* pControls[]    = { &m_aHeader, &m_aFooter, &m_aGroupOn, &m_aGroupInterval, &m_aKeepTogether, &m_aOrder
                                , &m_aMove,&m_aFL2};
    sal_Int32 nMaxTextWidth = 0;
    MnemonicGenerator aMnemonicGenerator;
    for (size_t i = 0; i < sizeof (pControlsLst) / sizeof (pControlsLst[0]); ++i)
        aMnemonicGenerator.RegisterMnemonic( pControls[i]->GetText() );

    for (size_t i = 0; i < sizeof (pControlsLst) / sizeof (pControlsLst[0]); ++i)
    {
        pControls[i]->Show(sal_True);
        String sText = pControls[i]->GetText();
        OUString sNewText = aMnemonicGenerator.CreateMnemonic(sText);
        if ( sText != sNewText )
            pControls[i]->SetText(sNewText);
        sal_Int32 nTextWidth = GetTextWidth(sNewText);
        nMaxTextWidth = ::std::max<sal_Int32>(nTextWidth,nMaxTextWidth);
    }

    Size aSize(UNRELATED_CONTROLS, PAGE_HEIGHT);
    Size aSpace = LogicToPixel( aSize, MAP_APPFONT );
    Size aOutSize(nMaxTextWidth + m_aHeader.GetSizePixel().Width() + 3*aSpace.Width(),aSpace.Height());
    SetMinOutputSizePixel(aOutSize);
    SetOutputSizePixel(aOutSize);

    m_pReportListener = new OPropertyChangeMultiplexer(this,m_pController->getReportDefinition().get());
    m_pReportListener->addProperty(PROPERTY_COMMAND);
    m_pReportListener->addProperty(PROPERTY_COMMANDTYPE);

    m_pFieldExpression->lateInit();
    fillColumns();
    m_pFieldExpression->Show();

    m_aHelpWindow.SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );

    m_pFieldExpression->SetZOrder(&m_aFL2, WINDOW_ZORDER_BEHIND);

    m_aMove.SetZOrder(m_pFieldExpression, WINDOW_ZORDER_BEHIND);
    m_aToolBox.SetStyle(m_aToolBox.GetStyle()|WB_LINESPACING);
    m_aToolBox.SetSelectHdl(LINK(this, OGroupsSortingDialog, OnFormatAction));
    m_aToolBox.SetImageListProvider(this);
    setToolBox(&m_aToolBox);

    checkButtons(0);
    Resize();

    FreeResource();
}

//------------------------------------------------------------------------
OGroupsSortingDialog::~OGroupsSortingDialog()
{
    DBG_DTOR( rpt_OGroupsSortingDialog,NULL);
    delete m_pFieldExpression;
    m_xColumns.clear();
    m_pReportListener->dispose();
    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
}
// -----------------------------------------------------------------------------
sal_Bool OGroupsSortingDialog::isReadOnly( ) const
{
    return m_bReadOnly;
}
//------------------------------------------------------------------------------
void OGroupsSortingDialog::UpdateData( )
{
    m_pFieldExpression->Invalidate();
    long nCurRow = m_pFieldExpression->GetCurRow();
    m_pFieldExpression->DeactivateCell();
    m_pFieldExpression->ActivateCell(nCurRow, m_pFieldExpression->GetCurColumnId());
    DisplayData(nCurRow);
}
//------------------------------------------------------------------------------
void OGroupsSortingDialog::DisplayData( sal_Int32 _nRow )
{
    DBG_CHKTHIS( rpt_OGroupsSortingDialog,NULL);
    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    sal_Bool bEmpty = nGroupPos == NO_GROUP;
    m_aHeaderLst.Enable(!bEmpty);
    m_aFooterLst.Enable(!bEmpty);
    m_aGroupOnLst.Enable(!bEmpty);
    m_aGroupIntervalEd.Enable(!bEmpty);
    m_aKeepTogetherLst.Enable(!bEmpty);
    m_aOrderLst.Enable(!bEmpty);

    m_aFL3.Enable(!bEmpty);
    m_aHeader.Enable(!bEmpty);
    m_aFooter.Enable(!bEmpty);
    m_aGroupOn.Enable(!bEmpty);
    m_aGroupInterval.Enable(!bEmpty);
    m_aKeepTogether.Enable(!bEmpty);
    m_aOrder.Enable(!bEmpty);

    checkButtons(_nRow);

    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
    m_pCurrentGroupListener = NULL;
    if ( !bEmpty && nGroupPos != NO_GROUP )
    {
        uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);

        m_pCurrentGroupListener = new OPropertyChangeMultiplexer(this,xGroup.get());
        m_pCurrentGroupListener->addProperty(PROPERTY_HEADERON);
        m_pCurrentGroupListener->addProperty(PROPERTY_FOOTERON);

        displayGroup(xGroup);
    }
}
//------------------------------------------------------------------------------
void OGroupsSortingDialog::SaveData( sal_Int32 _nRow)
{
    DBG_CHKTHIS( rpt_OGroupsSortingDialog,NULL);
    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos == NO_GROUP )
        return;

    uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
    if ( m_aHeaderLst.GetSavedValue() != m_aHeaderLst.GetSelectEntryPos() )
        xGroup->setHeaderOn( m_aHeaderLst.GetSelectEntryPos() == 0 );
    if ( m_aFooterLst.GetSavedValue() != m_aFooterLst.GetSelectEntryPos() )
        xGroup->setFooterOn( m_aFooterLst.GetSelectEntryPos() == 0 );
    if ( m_aKeepTogetherLst.GetSavedValue() != m_aKeepTogetherLst.GetSelectEntryPos() )
        xGroup->setKeepTogether( m_aKeepTogetherLst.GetSelectEntryPos() );
    if ( m_aGroupOnLst.GetSavedValue() != m_aGroupOnLst.GetSelectEntryPos() )
    {
        sal_Int16 nGroupOn = static_cast<sal_Int16>(reinterpret_cast<sal_IntPtr>(m_aGroupOnLst.GetEntryData(m_aGroupOnLst.GetSelectEntryPos())));
        xGroup->setGroupOn( nGroupOn );
    }
    if ( m_aGroupIntervalEd.GetSavedValue().toInt32() != m_aGroupIntervalEd.GetValue() )
    {
        xGroup->setGroupInterval( static_cast<sal_Int32>(m_aGroupIntervalEd.GetValue()) );
        m_aGroupIntervalEd.SaveValue();
    }
    if ( m_aOrderLst.GetSavedValue() != m_aOrderLst.GetSelectEntryPos() )
        xGroup->setSortAscending( m_aOrderLst.GetSelectEntryPos() == 0 );

    ListBox* pControls[] = { &m_aHeaderLst,&m_aFooterLst,&m_aGroupOnLst,&m_aKeepTogetherLst,&m_aOrderLst};
    for (size_t i = 0; i < sizeof(pControls)/sizeof(pControls[0]); ++i)
        pControls[i]->SaveValue();
}

// -----------------------------------------------------------------------------
sal_Int32 OGroupsSortingDialog::getColumnDataType(const OUString& _sColumnName)
{
    sal_Int32 nDataType = sdbc::DataType::VARCHAR;
    try
    {
        if ( !m_xColumns.is() )
            fillColumns();
        if ( m_xColumns.is() && m_xColumns->hasByName(_sColumnName) )
        {
            uno::Reference< beans::XPropertySet> xColumn(m_xColumns->getByName(_sColumnName),uno::UNO_QUERY);
            if ( xColumn.is() )
                xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Eception caught while getting the type of a column");
    }

    return nDataType;
}
//------------------------------------------------------------------------------
IMPL_LINK(OGroupsSortingDialog, OnControlFocusGot, Control*, pControl )
{
    if ( m_pFieldExpression && m_pFieldExpression->getExpressionControl() )
    {
        Control* pControls[] = { m_pFieldExpression->getExpressionControl(),&m_aHeaderLst,&m_aFooterLst,&m_aGroupOnLst,&m_aGroupIntervalEd,&m_aKeepTogetherLst,&m_aOrderLst};
        for (size_t i = 0; i < sizeof(pControls)/sizeof(pControls[0]); ++i)
        {
            if ( pControl == pControls[i] )
            {
                ListBox* pListBox = dynamic_cast< ListBox* >( pControl );
                if ( pListBox )
                    pListBox->SaveValue();
                NumericField* pNumericField = dynamic_cast< NumericField* >( pControl );
                if ( pNumericField )
                    pNumericField->SaveValue();
                showHelpText(static_cast<sal_uInt16>(i+STR_RPT_HELP_FIELD));
                break;
            }
        }
    }
    return 0L;
}
//------------------------------------------------------------------------------
IMPL_LINK(OGroupsSortingDialog, OnControlFocusLost, Control*, pControl )
{
    if ( m_pFieldExpression && pControl == &m_aGroupIntervalEd )
    {
        if ( m_aGroupIntervalEd.IsModified() )
            SaveData(m_pFieldExpression->GetCurRow());
    }
    return 0L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OGroupsSortingDialog, OnFormatAction, ToolBox*, /*NOTINTERESTEDIN*/ )
{
    DBG_CHKTHIS( rpt_OGroupsSortingDialog,NULL);

    sal_uInt16 nCommand = m_aToolBox.GetCurItemId();

    if ( m_pFieldExpression )
    {
        long nIndex = m_pFieldExpression->GetCurrRow();
        sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(nIndex);
        uno::Sequence<uno::Any> aClipboardList;
        if ( nIndex >= 0 && nGroupPos != NO_GROUP )
        {
            aClipboardList.realloc(1);
            aClipboardList[0] = m_xGroups->getByIndex(nGroupPos);
        }
        if ( nCommand == SID_RPT_GROUPSORT_MOVE_UP )
        {
            --nIndex;
        }
        if ( nCommand == SID_RPT_GROUPSORT_MOVE_DOWN )
        {
            ++nIndex;
        }
        if ( nCommand == SID_RPT_GROUPSORT_DELETE )
        {
            Application::PostUserEvent( LINK(m_pFieldExpression, OFieldExpressionControl, DelayedDelete) );
        }
        else
        {
            if ( nIndex >= 0 && aClipboardList.getLength() )
            {
                m_pFieldExpression->SetNoSelection();
                m_pFieldExpression->moveGroups(aClipboardList,nIndex,sal_False);
                m_pFieldExpression->DeactivateCell();
                m_pFieldExpression->GoToRow(nIndex);
                m_pFieldExpression->ActivateCell(nIndex, m_pFieldExpression->GetCurColumnId());
                DisplayData(nIndex);
            }
        }
    }
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OGroupsSortingDialog, LBChangeHdl, ListBox*, pListBox )
{
    DBG_CHKTHIS( rpt_OGroupsSortingDialog,NULL);
    if ( pListBox->GetSavedValue() != pListBox->GetSelectEntryPos() )
    {
        sal_Int32 nRow = m_pFieldExpression->GetCurRow();
        sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(nRow);
        if ( pListBox != &m_aHeaderLst && pListBox != &m_aFooterLst)
        {
            if ( pListBox && pListBox->GetSavedValue() != pListBox->GetSelectEntryPos() )
                SaveData(nRow);
            if ( pListBox == &m_aGroupOnLst )
                m_aGroupIntervalEd.Enable( pListBox->GetSelectEntryPos() != 0 );
        }
        else if ( nGroupPos != NO_GROUP )
        {
            uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
            uno::Sequence< beans::PropertyValue > aArgs(2);
            aArgs[1].Name = PROPERTY_GROUP;
            aArgs[1].Value <<= xGroup;

            if ( &m_aHeaderLst  == pListBox )
                aArgs[0].Name = PROPERTY_HEADERON;
            else
                aArgs[0].Name = PROPERTY_FOOTERON;

            aArgs[0].Value <<= pListBox->GetSelectEntryPos() == 0;
            m_pController->executeChecked(&m_aHeaderLst  == pListBox ? SID_GROUPHEADER : SID_GROUPFOOTER,aArgs);
            if ( m_pFieldExpression )
                m_pFieldExpression->InvalidateHandleColumn();
        }
    }
    return 1L;
}
// -----------------------------------------------------------------------------
void OGroupsSortingDialog::showHelpText(sal_uInt16 _nResId)
{
    m_aHelpWindow.SetText(String(ModuleRes(_nResId)));
}
// -----------------------------------------------------------------------------
void OGroupsSortingDialog::_propertyChanged(const beans::PropertyChangeEvent& _rEvent) throw( uno::RuntimeException)
{
    uno::Reference< report::XGroup > xGroup(_rEvent.Source,uno::UNO_QUERY);
    if ( xGroup.is() )
        displayGroup(xGroup);
    else
        fillColumns();
}
// -----------------------------------------------------------------------------
void OGroupsSortingDialog::fillColumns()
{
    m_xColumns = m_pController->getColumns();
    m_pFieldExpression->fillColumns(m_xColumns);
}
// -----------------------------------------------------------------------------
void OGroupsSortingDialog::displayGroup(const uno::Reference<report::XGroup>& _xGroup)
{
    m_aHeaderLst.SelectEntryPos(_xGroup->getHeaderOn() ? 0 : 1 );
    m_aFooterLst.SelectEntryPos(_xGroup->getFooterOn() ? 0 : 1 );
    sal_Int32 nDataType = getColumnDataType(_xGroup->getExpression());

    // first clear whole group on list
    while(m_aGroupOnLst.GetEntryCount() > 1 )
    {
        m_aGroupOnLst.RemoveEntry(1);
    }

    switch(nDataType)
    {
        case sdbc::DataType::LONGVARCHAR:
        case sdbc::DataType::VARCHAR:
        case sdbc::DataType::CHAR:
            m_aGroupOnLst.InsertEntry(String(ModuleRes(STR_RPT_PREFIXCHARS)));
            m_aGroupOnLst.SetEntryData(1,reinterpret_cast<void*>(report::GroupOn::PREFIX_CHARACTERS));
            break;
        case sdbc::DataType::DATE:
        case sdbc::DataType::TIME:
        case sdbc::DataType::TIMESTAMP:
            {
                sal_uInt16 nIds[] = { STR_RPT_YEAR, STR_RPT_QUARTER,STR_RPT_MONTH,STR_RPT_WEEK,STR_RPT_DAY,STR_RPT_HOUR,STR_RPT_MINUTE };
                for (sal_uInt16 i = 0; i < sizeof (nIds) / sizeof (nIds[0]); ++i)
                {
                    m_aGroupOnLst.InsertEntry(String(ModuleRes(nIds[i])));
                    m_aGroupOnLst.SetEntryData(i+1,reinterpret_cast<void*>(i+2));
                }
            }
            break;
        default:
            m_aGroupOnLst.InsertEntry(String(ModuleRes(STR_RPT_INTERVAL)));
            m_aGroupOnLst.SetEntryData(1,reinterpret_cast<void*>(report::GroupOn::INTERVAL));
            break;
    }
    sal_uInt16 nPos = 0;
    switch(_xGroup->getGroupOn())
    {
        case report::GroupOn::DEFAULT:
            nPos = 0;
            break;
        case report::GroupOn::PREFIX_CHARACTERS:
            nPos = 1;
            break;
        case report::GroupOn::YEAR:
            nPos = 1;
            break;
        case report::GroupOn::QUARTAL:
            nPos = 2;
            break;
        case report::GroupOn::MONTH:
            nPos = 3;
            break;
        case report::GroupOn::WEEK:
            nPos = 4;
            break;
        case report::GroupOn::DAY:
            nPos = 5;
            break;
        case report::GroupOn::HOUR:
            nPos = 6;
            break;
        case report::GroupOn::MINUTE:
            nPos = 7;
            break;
        case report::GroupOn::INTERVAL:
            nPos = 1;
            break;
        default:
            nPos = 0;
    }
    m_aGroupOnLst.SelectEntryPos(nPos);
    m_aGroupIntervalEd.SetText(OUString::number(_xGroup->getGroupInterval()));
    m_aGroupIntervalEd.SaveValue();
    m_aGroupIntervalEd.Enable( nPos != 0 );
    m_aKeepTogetherLst.SelectEntryPos(_xGroup->getKeepTogether());
    m_aOrderLst.SelectEntryPos(_xGroup->getSortAscending() ? 0 : 1);

    ListBox* pControls[] = { &m_aHeaderLst,&m_aFooterLst,&m_aGroupOnLst,&m_aKeepTogetherLst,&m_aOrderLst};
    for (size_t i = 0; i < sizeof(pControls)/sizeof(pControls[0]); ++i)
        pControls[i]->SaveValue();

    ListBox* pControlsLst2[] = { &m_aHeaderLst, &m_aFooterLst,  &m_aGroupOnLst, &m_aKeepTogetherLst,&m_aOrderLst};
    sal_Bool bReadOnly = !m_pController->isEditable();
    for (size_t i = 0; i < sizeof(pControlsLst2)/sizeof(pControlsLst2[0]); ++i)
        pControlsLst2[i]->SetReadOnly(bReadOnly);
    m_aGroupIntervalEd.SetReadOnly(bReadOnly);
}
//------------------------------------------------------------------------------
void OGroupsSortingDialog::Resize()
{
    Window::Resize();
    Size aTotalOutputSize = GetOutputSizePixel();
    Size aSpace = LogicToPixel( Size( UNRELATED_CONTROLS, UNRELATED_CONTROLS ), MAP_APPFONT );
    m_pFieldExpression->SetSizePixel(Size(aTotalOutputSize.Width() - 2*aSpace.Width(),m_pFieldExpression->GetSizePixel().Height()));

    Control* pControlsLst[] = { &m_aHeaderLst,  &m_aFooterLst,  &m_aGroupOnLst, &m_aGroupIntervalEd,&m_aKeepTogetherLst,&m_aOrderLst};
    Control* pControls[]    = { &m_aHeader,     &m_aFooter,     &m_aGroupOn,    &m_aGroupInterval,  &m_aKeepTogether,   &m_aOrder};
    sal_Int32 nMaxTextWidth = 0;
    for (size_t i = 0; i < sizeof (pControlsLst) / sizeof (pControlsLst[0]); ++i)
    {
        nMaxTextWidth = ::std::max<sal_Int32>(static_cast<sal_Int32>(GetTextWidth(pControls[i]->GetText())),nMaxTextWidth);
    }

    for (size_t i = 0; i < sizeof (pControlsLst) / sizeof (pControlsLst[0]); ++i)
    {
        pControls[i]->SetSizePixel(Size(nMaxTextWidth,pControls[i]->GetSizePixel().Height()));
        Point aPos = pControls[i]->GetPosPixel();
        aPos.X() += nMaxTextWidth + aSpace.Width();
        aPos.Y() = pControlsLst[i]->GetPosPixel().Y();

        pControlsLst[i]->SetPosSizePixel(aPos,Size(aTotalOutputSize.Width() - aPos.X() - aSpace.Width(),pControlsLst[i]->GetSizePixel().Height()));
    }

    m_aFL.SetSizePixel(Size(aTotalOutputSize.Width() - aSpace.Width(),m_aFL.GetSizePixel().Height()));
    m_aFL2.SetSizePixel(Size(aTotalOutputSize.Width() - aSpace.Width(),m_aFL2.GetSizePixel().Height()));
    m_aFL3.SetSizePixel(Size(aTotalOutputSize.Width() - aSpace.Width(),m_aFL3.GetSizePixel().Height()));
    sal_Int32 nPos = aTotalOutputSize.Width() - aSpace.Width() - m_aToolBox.GetSizePixel().Width();
    m_aToolBox.SetPosPixel(Point(nPos,m_aToolBox.GetPosPixel().Y()));

    Point aHelpPos = m_aHelpWindow.GetPosPixel();
    m_aHelpWindow.SetSizePixel(Size(aTotalOutputSize.Width() - aHelpPos.X(),aTotalOutputSize.Height() - aHelpPos.Y()));
}
//------------------------------------------------------------------------------
void OGroupsSortingDialog::checkButtons(sal_Int32 _nRow)
{
    sal_Int32 nGroupCount = m_xGroups->getCount();
    sal_Int32 nRowCount = m_pFieldExpression->GetRowCount();
    sal_Bool bEnabled = nGroupCount > 1;

    if (bEnabled && _nRow > 0 )
    {
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_MOVE_UP, sal_True);
    }
    else
    {
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_MOVE_UP, sal_False);
    }
    if (bEnabled && _nRow < (nRowCount - 1) )
    {
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_MOVE_DOWN, sal_True);
    }
    else
    {
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_MOVE_DOWN, sal_False);
    }

    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos != NO_GROUP )
    {
        sal_Bool bEnableDelete = nGroupCount > 0;
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_DELETE, bEnableDelete);
    }
    else
    {
        m_aToolBox.EnableItem(SID_RPT_GROUPSORT_DELETE, sal_False);
    }
}

ImageList OGroupsSortingDialog::getImageList(sal_Int16 _eBitmapSet) const
{
    sal_Int16 nN = IMG_CONDFORMAT_DLG_SC;
    if ( _eBitmapSet == SFX_SYMBOLS_SIZE_LARGE )
        nN = IMG_CONDFORMAT_DLG_LC;
    return ImageList(ModuleRes( nN ));
}

//------------------------------------------------------------------
void OGroupsSortingDialog::resizeControls(const Size& _rDiff)
{
    // we use large images so we must change them
    if ( _rDiff.Width() || _rDiff.Height() )
    {
        Invalidate();
    }
}

//------------------------------------------------------------------
// load the images
ImageList OGroupsSortingDialog::getImageList(vcl::ImageListType) SAL_THROW (( com::sun::star::lang::IllegalArgumentException ))
{
    return ImageList(ModuleRes(IMGLST_GROUPSORT_DLG_SC));
}

// =============================================================================
} // rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
