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
#include <connectivity/dbtools.hxx>
#include <sot/exchange.hxx>
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

#include <cppuhelper/implbase.hxx>
#include <comphelper/property.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

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

/**
  * Separated out from OFieldExpressionControl to prevent collision of ref-counted base classes
  */
class OFieldExpressionControl;
class OFieldExpressionControlContainerListener : public ::cppu::WeakImplHelper< container::XContainerListener >
{
    VclPtr<OFieldExpressionControl> mpParent;
public:
    explicit OFieldExpressionControlContainerListener(OFieldExpressionControl* pParent) : mpParent(pParent) {}

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
    // XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) override;
};

class OFieldExpressionControl : public ::svt::EditBrowseBox
{
    ::osl::Mutex                    m_aMutex;
    ::std::vector<sal_Int32>        m_aGroupPositions;
    ::std::vector<ColumnInfo>       m_aColumnInfo;
    VclPtr< ::svt::ComboBoxControl>  m_pComboCell;
    sal_Int32                       m_nDataPos;
    sal_Int32                       m_nCurrentPos;
    ImplSVEvent *                   m_nPasteEvent;
    ImplSVEvent *                   m_nDeleteEvent;
    VclPtr<OGroupsSortingDialog>    m_pParent;
    bool                            m_bIgnoreEvent;
    rtl::Reference<OFieldExpressionControlContainerListener> aContainerListener;

public:
    OFieldExpressionControl(OGroupsSortingDialog* _pParentDialog, vcl::Window *_pParent);
    virtual ~OFieldExpressionControl() override;
    virtual void dispose() override;

    // XContainerListener
    /// @throws css::uno::RuntimeException
    void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent);
    /// @throws css::uno::RuntimeException
    void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent);

    virtual Size GetOptimalSize() const override;

    void        fillColumns(const uno::Reference< container::XNameAccess>& _xColumns);
    void        lateInit();
    bool    IsDeleteAllowed( );
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
    void moveGroups(const uno::Sequence<uno::Any>& _aGroups,sal_Int32 _nRow,bool _bSelect = true);

    virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol) override;
    using ::svt::EditBrowseBox::GetRowCount;
protected:
    virtual bool IsTabAllowed(bool bForward) const override;

    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) override;
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) override;
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const override;
    virtual bool SeekRow( long nRow ) override;
    virtual bool SaveModified() override;
    virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const override;
    virtual RowStatus GetRowStatus(long nRow) const override;

    virtual void KeyInput(const KeyEvent& rEvt) override;
    virtual void Command( const CommandEvent& rEvt ) override;

    // D&D
    virtual void     StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual sal_Int8 AcceptDrop( const BrowserAcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const BrowserExecuteDropEvent& rEvt ) override;

    using BrowseBox::AcceptDrop;
    using BrowseBox::ExecuteDrop;

private:

    DECL_LINK( DelayedPaste, void*, void );
    DECL_LINK( CBChangeHdl, ComboBox&, void);

    void InsertRows( long nRow );

public:
    DECL_LINK( DelayedDelete, void*, void );

};


void OFieldExpressionControlContainerListener::disposing(const css::lang::EventObject& )
{}

void OFieldExpressionControlContainerListener::elementInserted(const css::container::ContainerEvent& rEvent)
{ mpParent->elementInserted(rEvent); }

void OFieldExpressionControlContainerListener::elementReplaced(const css::container::ContainerEvent& )
{}

void OFieldExpressionControlContainerListener::elementRemoved(const css::container::ContainerEvent& rEvent)
{ mpParent->elementRemoved(rEvent); }


// class OFieldExpressionControl
OFieldExpressionControl::OFieldExpressionControl(OGroupsSortingDialog* _pParentDialog, vcl::Window *_pParent)
    :EditBrowseBox( _pParent, EditBrowseBoxFlags::NONE, WB_TABSTOP,
                    BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::AUTOSIZE_LASTCOL |
                              BrowserMode::KEEPHIGHLIGHT | BrowserMode::HLINES | BrowserMode::VLINES)
    ,m_aGroupPositions(GROUPS_START_LEN,-1)
    ,m_pComboCell(nullptr)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nPasteEvent(nullptr)
    ,m_nDeleteEvent(nullptr)
    ,m_pParent(_pParentDialog)
    ,m_bIgnoreEvent(false)
    ,aContainerListener(new OFieldExpressionControlContainerListener(this))
{
    SetBorderStyle(WindowBorderStyle::MONO);
}


OFieldExpressionControl::~OFieldExpressionControl()
{
    disposeOnce();
}


void OFieldExpressionControl::dispose()
{
    uno::Reference< report::XGroups > xGroups = m_pParent->getGroups();
    xGroups->removeContainerListener(aContainerListener.get());

    // delete events from queue
    if( m_nPasteEvent )
        Application::RemoveUserEvent( m_nPasteEvent );
    if( m_nDeleteEvent )
        Application::RemoveUserEvent( m_nDeleteEvent );

    m_pComboCell.disposeAndClear();
    m_pParent.clear();
    ::svt::EditBrowseBox::dispose();
}

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

void OFieldExpressionControl::StartDrag( sal_Int8 /*_nAction*/ , const Point& /*_rPosPixel*/ )
{
    if ( m_pParent && !m_pParent->isReadOnly( ) )
    {
        uno::Sequence<uno::Any> aClipboardList = fillSelectedGroups();

        if( aClipboardList.getLength() )
        {
            OGroupExchange* pData = new OGroupExchange(aClipboardList);
            uno::Reference< css::datatransfer::XTransferable> xRef = pData;
            pData->StartDrag(this, DND_ACTION_MOVE );
        }
    }
}

sal_Int8 OFieldExpressionControl::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    sal_Int8 nAction = DND_ACTION_NONE;
    if ( IsEditing() )
    {
        sal_Int32 nPos = m_pComboCell->GetSelectEntryPos();
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

sal_Int8 OFieldExpressionControl::ExecuteDrop( const BrowserExecuteDropEvent& rEvt )
{
    sal_Int8 nAction = DND_ACTION_NONE;
    if ( IsDropFormatSupported( OGroupExchange::getReportGroupId() ) )
    {
        sal_Int32   nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), false);
        SetNoSelection();

        TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
        uno::Any aDrop = aDropped.GetAny(OGroupExchange::getReportGroupId(), OUString());
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

void OFieldExpressionControl::moveGroups(const uno::Sequence<uno::Any>& _aGroups,sal_Int32 _nRow,bool _bSelect)
{
    if ( _aGroups.getLength() )
    {
        m_bIgnoreEvent = true;
        {
            sal_Int32 nRow = _nRow;
            const OUString sUndoAction(ModuleRes(RID_STR_UNDO_MOVE_GROUP));
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

void OFieldExpressionControl::fillColumns(const uno::Reference< container::XNameAccess>& _xColumns)
{
    m_pComboCell->Clear();
    if ( _xColumns.is() )
        lcl_addToList_throw(*m_pComboCell,m_aColumnInfo,_xColumns);
}

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
        vcl::Font aFont( GetDataWindow().GetFont() );
        aFont.SetWeight( WEIGHT_NORMAL );
        GetDataWindow().SetFont( aFont );

        // Set font of the headline to light
        aFont = GetFont();
        aFont.SetWeight( WEIGHT_LIGHT );
        SetFont(aFont);

        InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth(OUString('0')) * 4)/*, sal_True */);
        InsertDataColumn( FIELD_EXPRESSION, OUString(ModuleRes(STR_RPT_EXPRESSION)), 100);

        m_pComboCell = VclPtr<ComboBoxControl>::Create( &GetDataWindow() );
        m_pComboCell->SetSelectHdl(LINK(this,OFieldExpressionControl,CBChangeHdl));
        m_pComboCell->SetHelpId(HID_RPT_FIELDEXPRESSION);

        m_pComboCell->SetGetFocusHdl(LINK(m_pParent, OGroupsSortingDialog, OnControlFocusGot));
        m_pComboCell->SetLoseFocusHdl(LINK(m_pParent, OGroupsSortingDialog, OnControlFocusLost));


        // set browse mode
        BrowserMode nMode(BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION  | BrowserMode::KEEPHIGHLIGHT |
                          BrowserMode::HLINES | BrowserMode::VLINES       | BrowserMode::AUTOSIZE_LASTCOL | BrowserMode::AUTO_VSCROLL | BrowserMode::AUTO_HSCROLL);
        if( m_pParent->isReadOnly() )
            nMode |= BrowserMode::HIDECURSOR;
        SetMode(nMode);
        xGroups->addContainerListener(aContainerListener.get());
    }
    else
        // not the first call
        RowRemoved(0, GetRowCount());

    RowInserted(0, m_aGroupPositions.size());
}


IMPL_LINK_NOARG( OFieldExpressionControl, CBChangeHdl, ComboBox&, void )
{

    SaveModified();
}


bool OFieldExpressionControl::IsTabAllowed(bool /*bForward*/) const
{
    return false;
}


bool OFieldExpressionControl::SaveModified()
{
    sal_Int32 nRow = GetCurRow();
    if ( nRow != BROWSER_ENDOFSELECTION )
    {
        try
        {
            bool bAppend = false;
            uno::Reference< report::XGroup> xGroup;
            if ( m_aGroupPositions[nRow] == NO_GROUP )
            {
                bAppend = true;
                OUString sUndoAction(ModuleRes(RID_STR_UNDO_APPEND_GROUP));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, OUString(), 0, -1 );
                xGroup = m_pParent->getGroups()->createGroup();
                xGroup->setHeaderOn(true);

                uno::Sequence< beans::PropertyValue > aArgs(2);
                aArgs[0].Name = PROPERTY_GROUP;
                aArgs[0].Value <<= xGroup;
                // find position where to insert the new group
                sal_Int32 nGroupPos = 0;
                ::std::vector<sal_Int32>::iterator aIter = m_aGroupPositions.begin();
                ::std::vector<sal_Int32>::const_iterator aEnd  = m_aGroupPositions.begin() + nRow;
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
                sal_Int32 nPos = m_pComboCell->GetSelectEntryPos();
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

            if ( Controller().is() )
                Controller()->ClearModified();
            if ( GetRowCount() == m_pParent->getGroups()->getCount() )
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

    return true;
}

OUString OFieldExpressionControl::GetCellText( long nRow, sal_uInt16 /*nColId*/ ) const
{
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


void OFieldExpressionControl::InitController( CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColumnId )
{

    m_pComboCell->SetText( GetCellText( nRow, nColumnId ) );
}

bool OFieldExpressionControl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{

    if (!EditBrowseBox::CursorMoving(nNewRow, nNewCol))
        return false;
    m_nDataPos = nNewRow;
    long nOldDataPos = GetCurRow();
    InvalidateStatusCell( m_nDataPos );
    InvalidateStatusCell( nOldDataPos );

    m_pParent->SaveData( nOldDataPos );
    m_pParent->DisplayData( m_nDataPos );
    return true;
}

CellController* OFieldExpressionControl::GetController( long /*nRow*/, sal_uInt16 /*nColumnId*/ )
{
    ComboBoxCellController* pCellController = new ComboBoxCellController( m_pComboCell );
    pCellController->GetComboBox().SetReadOnly(!m_pParent->m_pController->isEditable());
    return pCellController;
}


bool OFieldExpressionControl::SeekRow( long _nRow )
{
    // the basis class needs the call, because that's how the class knows which line will be painted
    EditBrowseBox::SeekRow(_nRow);
    m_nCurrentPos = _nRow;
    return true;
}


void OFieldExpressionControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const
{
    OUString aText  =GetCellText( m_nCurrentPos, nColumnId );

    Point aPos( rRect.TopLeft() );
    Size aTextSize( GetDataWindow().GetTextWidth( aText ), GetDataWindow().GetTextHeight() );

    if( aPos.X() < rRect.Left() || aPos.X() + aTextSize.Width() > rRect.Right() ||
        aPos.Y() < rRect.Top() || aPos.Y() + aTextSize.Height() > rRect.Bottom() )
        rDev.SetClipRegion(vcl::Region(rRect));

    rDev.DrawText( aPos, aText );

    if( rDev.IsClipRegion() )
        rDev.SetClipRegion();
}

EditBrowseBox::RowStatus OFieldExpressionControl::GetRowStatus(long nRow) const
{
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
            OSL_FAIL("Exception catched while try to get a group!");
        }
    }
    return EditBrowseBox::CLEAN;
}

// XContainerListener

void SAL_CALL OFieldExpressionControl::elementInserted(const container::ContainerEvent& evt)
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

                ::std::vector<sal_Int32>::const_iterator aEnd  = m_aGroupPositions.end();
                for(++aFind;aFind != aEnd;++aFind)
                    if ( *aFind != NO_GROUP )
                        ++*aFind;
            }
        }
        Invalidate();
    }
}

void SAL_CALL OFieldExpressionControl::elementRemoved(const container::ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bIgnoreEvent )
        return;

    sal_Int32 nGroupPos = 0;
    if ( evt.Accessor >>= nGroupPos )
    {
        std::vector<sal_Int32>::iterator aEnd = m_aGroupPositions.end();
        std::vector<sal_Int32>::iterator aFind = std::find(m_aGroupPositions.begin(), aEnd, nGroupPos);
        if (aFind != aEnd)
        {
            *aFind = NO_GROUP;
            for(++aFind;aFind != aEnd;++aFind)
                if ( *aFind != NO_GROUP )
                    --*aFind;
            Invalidate();
        }
    }
}

bool OFieldExpressionControl::IsDeleteAllowed( )
{
    return !m_pParent->isReadOnly() && GetSelectRowCount() > 0;
}

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

void OFieldExpressionControl::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            if (!rEvt.IsMouseEvent())
            {
                EditBrowseBox::Command(rEvt);
                return;
            }

            sal_uInt16 nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());

            if ( nColId == HANDLE_ID )
            {
                ScopedVclPtrInstance<PopupMenu> aContextMenu(ModuleRes(RID_GROUPSROWPOPUPMENU));
                bool bEnable = false;
                long nIndex = FirstSelectedRow();
                while( nIndex >= 0 && !bEnable )
                {
                    if ( m_aGroupPositions[nIndex] != NO_GROUP )
                        bEnable = true;
                    nIndex = NextSelectedRow();
                }
                aContextMenu->EnableItem( SID_DELETE, IsDeleteAllowed() && bEnable );
                switch (aContextMenu->Execute(this, rEvt.GetMousePosPixel()))
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
                        m_nDeleteEvent = Application::PostUserEvent( LINK(this, OFieldExpressionControl, DelayedDelete), nullptr, true );
                        break;
                    default:
                        break;
                }
            }
            SAL_FALLTHROUGH;
        }
        default:
            EditBrowseBox::Command(rEvt);
    }

}

void OFieldExpressionControl::DeleteRows()
{

    bool bIsEditing = IsEditing();
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
                OUString sUndoAction(ModuleRes(RID_STR_UNDO_REMOVE_SELECTION));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, OUString(), 0, -1 );
            }

            sal_Int32 nGroupPos = m_aGroupPositions[nIndex];
            uno::Reference< report::XGroup> xGroup = m_pParent->getGroup(nGroupPos);
            aArgs[0].Value <<= xGroup;
            // we use this way to create undo actions
            m_pParent->m_pController->executeChecked(SID_GROUP_REMOVE,aArgs);

            std::vector<sal_Int32>::iterator aEnd  = m_aGroupPositions.end();
            std::vector<sal_Int32>::iterator aFind = std::find(m_aGroupPositions.begin(), aEnd, nGroupPos);
            if (aFind != aEnd)
            {
                *aFind = NO_GROUP;
                for(++aFind;aFind != aEnd;++aFind)
                    if ( *aFind != NO_GROUP )
                        --*aFind;
            }
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


void OFieldExpressionControl::cut()
{
    copy();
    DeleteRows();
}


void OFieldExpressionControl::copy()
{

    // set to the right row and save it
    m_pParent->SaveData( m_nDataPos );

    uno::Sequence<uno::Any> aClipboardList = fillSelectedGroups();

    if( aClipboardList.getLength() )
    {
        OGroupExchange* pData = new OGroupExchange(aClipboardList);
        uno::Reference< css::datatransfer::XTransferable> xRef = pData;
        pData->CopyToClipboard(GetParent());
    }
}


void OFieldExpressionControl::paste()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(OGroupExchange::getReportGroupId()))
    {
        if( m_nPasteEvent )
            Application::RemoveUserEvent( m_nPasteEvent );
        m_nPasteEvent = Application::PostUserEvent( LINK(this, OFieldExpressionControl, DelayedPaste), nullptr, true );
    }
}

IMPL_LINK_NOARG( OFieldExpressionControl, DelayedPaste, void*, void )
{
    m_nPasteEvent = nullptr;

    sal_Int32 nPastePosition = GetSelectRowCount() ? FirstSelectedRow() : GetCurRow();

    InsertRows( nPastePosition );
    SetNoSelection();
    GoToRow( nPastePosition );
}

IMPL_LINK_NOARG( OFieldExpressionControl, DelayedDelete, void*, void )
{
    m_nDeleteEvent = nullptr;
    DeleteRows();
}

void OFieldExpressionControl::InsertRows( long nRow )
{

    sal_Int32 nSize = 0;

    // get rows from clipboard
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
    if(aTransferData.HasFormat(OGroupExchange::getReportGroupId()))
    {
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor(OGroupExchange::getReportGroupId(), aFlavor);
        uno::Sequence< uno::Any > aGroups;

        if ((aTransferData.GetAny(aFlavor, OUString()) >>= aGroups) && aGroups.getLength())
        {
            m_bIgnoreEvent = false;
            {
                const OUString sUndoAction(ModuleRes(RID_STR_UNDO_APPEND_GROUP));
                const UndoContext aUndoContext( m_pParent->m_pController->getUndoManager(), sUndoAction );

                uno::Reference<report::XGroups> xGroups = m_pParent->getGroups();
                sal_Int32 nGroupPos = 0;
                ::std::vector<sal_Int32>::iterator aIter = m_aGroupPositions.begin();
                ::std::vector<sal_Int32>::size_type nRowPos = static_cast< ::std::vector<sal_Int32>::size_type >(nRow);
                if ( nRowPos < m_aGroupPositions.size() )
                {
                    ::std::vector<sal_Int32>::const_iterator aEnd  = m_aGroupPositions.begin() + nRowPos;
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
                    ::std::vector<sal_Int32>::const_iterator aEnd  = m_aGroupPositions.end();
                    for(;aInsertPos != aEnd;++aInsertPos)
                        if ( *aInsertPos != NO_GROUP )
                            ++*aInsertPos;
                }
            }
            m_bIgnoreEvent = true;
        }
    }

    RowInserted( nRow, nSize );
}

Size OFieldExpressionControl::GetOptimalSize() const
{
    return LogicToPixel(Size(106, 75), MapUnit::MapAppFont);
}

// class OGroupsSortingDialog
OGroupsSortingDialog::OGroupsSortingDialog(vcl::Window* _pParent, bool _bReadOnly,
                                           OReportController* _pController)
    : FloatingWindow(_pParent, "FloatingSort", "modules/dbreport/ui/floatingsort.ui")
    , OPropertyChangeListener(m_aMutex)
    , m_pController(_pController)
    , m_pCurrentGroupListener(nullptr)
    , m_xGroups(m_pController->getReportDefinition()->getGroups())
    , m_bReadOnly(_bReadOnly)
{
    get(m_pToolBox, "toolbox");
    m_nMoveUpId = m_pToolBox->GetItemId(0);
    m_nMoveDownId = m_pToolBox->GetItemId(1);
    m_nDeleteId = m_pToolBox->GetItemId(2);
    get(m_pOrderLst, "sorting");
    get(m_pHeaderLst, "header");
    get(m_pFooterLst, "footer");
    get(m_pGroupOnLst, "group");
    get(m_pGroupIntervalEd, "interval");
    get(m_pKeepTogetherLst, "keep");
    get(m_pHelpWindow, "helptext");
    m_pHelpWindow->set_height_request(GetTextHeight() * 4);
    get(m_pProperties, "properties");
    m_pFieldExpression = VclPtr<OFieldExpressionControl>::Create(this, get<vcl::Window>("box"));
    m_pFieldExpression->set_hexpand(true);
    m_pFieldExpression->set_vexpand(true);

    Control* pControlsLst[] = { m_pHeaderLst, m_pFooterLst, m_pGroupOnLst, m_pKeepTogetherLst, m_pOrderLst, m_pGroupIntervalEd};
    for (Control* i : pControlsLst)
    {
        i->SetGetFocusHdl(LINK(this, OGroupsSortingDialog, OnControlFocusGot));
        i->SetLoseFocusHdl(LINK(this, OGroupsSortingDialog, OnControlFocusLost));
        i->Show();
    }

    for (size_t i = 0; i < SAL_N_ELEMENTS(pControlsLst) - 1; ++i)
        static_cast<ListBox*>(pControlsLst[i])->SetSelectHdl(LINK(this,OGroupsSortingDialog,LBChangeHdl));

    m_pReportListener = new OPropertyChangeMultiplexer(this,m_pController->getReportDefinition().get());
    m_pReportListener->addProperty(PROPERTY_COMMAND);
    m_pReportListener->addProperty(PROPERTY_COMMANDTYPE);

    m_pFieldExpression->lateInit();
    fillColumns();
    m_pFieldExpression->Show();

    m_pHelpWindow->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );

    m_pToolBox->SetStyle(m_pToolBox->GetStyle()|WB_LINESPACING);
    m_pToolBox->SetSelectHdl(LINK(this, OGroupsSortingDialog, OnFormatAction));

    checkButtons(0);

    Show();
}

OGroupsSortingDialog::~OGroupsSortingDialog()
{
    disposeOnce();
}

void OGroupsSortingDialog::dispose()
{
    m_xColumns.clear();
    m_pReportListener->dispose();
    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
    m_pToolBox.clear();
    m_pProperties.clear();
    m_pOrderLst.clear();
    m_pHeaderLst.clear();
    m_pFooterLst.clear();
    m_pGroupOnLst.clear();
    m_pGroupIntervalEd.clear();
    m_pKeepTogetherLst.clear();
    m_pHelpWindow.clear();
    m_pFieldExpression.disposeAndClear();
    FloatingWindow::dispose();
}

void OGroupsSortingDialog::UpdateData( )
{
    m_pFieldExpression->Invalidate();
    long nCurRow = m_pFieldExpression->GetCurRow();
    m_pFieldExpression->DeactivateCell();
    m_pFieldExpression->ActivateCell(nCurRow, m_pFieldExpression->GetCurColumnId());
    DisplayData(nCurRow);
}

void OGroupsSortingDialog::DisplayData( sal_Int32 _nRow )
{
    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    bool bEmpty = nGroupPos == NO_GROUP;
    m_pProperties->Enable(!bEmpty);

    checkButtons(_nRow);

    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
    m_pCurrentGroupListener = nullptr;
    if ( !bEmpty && nGroupPos != NO_GROUP )
    {
        uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);

        m_pCurrentGroupListener = new OPropertyChangeMultiplexer(this,xGroup.get());
        m_pCurrentGroupListener->addProperty(PROPERTY_HEADERON);
        m_pCurrentGroupListener->addProperty(PROPERTY_FOOTERON);

        displayGroup(xGroup);
    }
}

void OGroupsSortingDialog::SaveData( sal_Int32 _nRow)
{
    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos == NO_GROUP )
        return;

    uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
    if ( m_pHeaderLst->IsValueChangedFromSaved() )
        xGroup->setHeaderOn( m_pHeaderLst->GetSelectEntryPos() == 0 );
    if ( m_pFooterLst->IsValueChangedFromSaved() )
        xGroup->setFooterOn( m_pFooterLst->GetSelectEntryPos() == 0 );
    if ( m_pKeepTogetherLst->IsValueChangedFromSaved() )
        xGroup->setKeepTogether( m_pKeepTogetherLst->GetSelectEntryPos() );
    if ( m_pGroupOnLst->IsValueChangedFromSaved() )
    {
        sal_Int16 nGroupOn = static_cast<sal_Int16>(reinterpret_cast<sal_IntPtr>(m_pGroupOnLst->GetSelectEntryData()));
        xGroup->setGroupOn( nGroupOn );
    }
    if ( m_pGroupIntervalEd->IsValueChangedFromSaved() )
    {
        xGroup->setGroupInterval( static_cast<sal_Int32>(m_pGroupIntervalEd->GetValue()) );
        m_pGroupIntervalEd->SaveValue();
    }
    if ( m_pOrderLst->IsValueChangedFromSaved() )
        xGroup->setSortAscending( m_pOrderLst->GetSelectEntryPos() == 0 );

    ListBox* pControls[] = { m_pHeaderLst, m_pFooterLst, m_pGroupOnLst, m_pKeepTogetherLst, m_pOrderLst};
    for (ListBox* pControl : pControls)
        pControl->SaveValue();
}


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
        OSL_FAIL("Exception caught while getting the type of a column");
    }

    return nDataType;
}

IMPL_LINK(OGroupsSortingDialog, OnControlFocusGot, Control&, rControl, void )
{
    if ( m_pFieldExpression && m_pFieldExpression->getExpressionControl() )
    {
        Control* pControls[] = { m_pFieldExpression->getExpressionControl(), m_pHeaderLst, m_pFooterLst, m_pGroupOnLst, m_pGroupIntervalEd, m_pKeepTogetherLst, m_pOrderLst};
        for (size_t i = 0; i < SAL_N_ELEMENTS(pControls); ++i)
        {
            if ( &rControl == pControls[i] )
            {
                ListBox* pListBox = dynamic_cast< ListBox* >( &rControl );
                if ( pListBox )
                    pListBox->SaveValue();
                NumericField* pNumericField = dynamic_cast< NumericField* >( &rControl );
                if ( pNumericField )
                    pNumericField->SaveValue();
                //shows the text given by the id in the multiline edit
                m_pHelpWindow->SetText(OUString(ModuleRes(static_cast<sal_uInt16>(i+STR_RPT_HELP_FIELD))));
                break;
            }
        }
    }
}

IMPL_LINK(OGroupsSortingDialog, OnControlFocusLost, Control&, rControl, void )
{
    if (m_pFieldExpression && &rControl == m_pGroupIntervalEd)
    {
        if ( m_pGroupIntervalEd->IsModified() )
            SaveData(m_pFieldExpression->GetCurRow());
    }
}

IMPL_LINK_NOARG( OGroupsSortingDialog, OnFormatAction, ToolBox*, void )
{

    sal_uInt16 nCommand = m_pToolBox->GetCurItemId();

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
        if ( nCommand == m_nMoveUpId )
        {
            --nIndex;
        }
        if ( nCommand == m_nMoveDownId )
        {
            ++nIndex;
        }
        if ( nCommand == m_nDeleteId )
        {
            Application::PostUserEvent( LINK(m_pFieldExpression, OFieldExpressionControl, DelayedDelete), nullptr, true );
        }
        else
        {
            if ( nIndex >= 0 && aClipboardList.getLength() )
            {
                m_pFieldExpression->SetNoSelection();
                m_pFieldExpression->moveGroups(aClipboardList,nIndex,false);
                m_pFieldExpression->DeactivateCell();
                m_pFieldExpression->GoToRow(nIndex);
                m_pFieldExpression->ActivateCell(nIndex, m_pFieldExpression->GetCurColumnId());
                DisplayData(nIndex);
            }
        }
    }
}

IMPL_LINK( OGroupsSortingDialog, LBChangeHdl, ListBox&, rListBox, void )
{
    if ( rListBox.IsValueChangedFromSaved() )
    {
        sal_Int32 nRow = m_pFieldExpression->GetCurRow();
        sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(nRow);
        if (&rListBox != m_pHeaderLst && &rListBox != m_pFooterLst)
        {
            if ( rListBox.IsValueChangedFromSaved() )
                SaveData(nRow);
            if ( &rListBox == m_pGroupOnLst )
                m_pGroupIntervalEd->Enable( rListBox.GetSelectEntryPos() != 0 );
        }
        else if ( nGroupPos != NO_GROUP )
        {
            uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
            uno::Sequence< beans::PropertyValue > aArgs(2);
            aArgs[1].Name = PROPERTY_GROUP;
            aArgs[1].Value <<= xGroup;

            if ( m_pHeaderLst  == &rListBox )
                aArgs[0].Name = PROPERTY_HEADERON;
            else
                aArgs[0].Name = PROPERTY_FOOTERON;

            aArgs[0].Value <<= rListBox.GetSelectEntryPos() == 0;
            m_pController->executeChecked(m_pHeaderLst  == &rListBox ? SID_GROUPHEADER : SID_GROUPFOOTER, aArgs);
            m_pFieldExpression->InvalidateHandleColumn();
        }
    }
}

void OGroupsSortingDialog::_propertyChanged(const beans::PropertyChangeEvent& _rEvent)
{
    uno::Reference< report::XGroup > xGroup(_rEvent.Source,uno::UNO_QUERY);
    if ( xGroup.is() )
        displayGroup(xGroup);
    else
        fillColumns();
}

void OGroupsSortingDialog::fillColumns()
{
    m_xColumns = m_pController->getColumns();
    m_pFieldExpression->fillColumns(m_xColumns);
}

void OGroupsSortingDialog::displayGroup(const uno::Reference<report::XGroup>& _xGroup)
{
    m_pHeaderLst->SelectEntryPos(_xGroup->getHeaderOn() ? 0 : 1 );
    m_pFooterLst->SelectEntryPos(_xGroup->getFooterOn() ? 0 : 1 );
    sal_Int32 nDataType = getColumnDataType(_xGroup->getExpression());

    // first clear whole group on list
    while(m_pGroupOnLst->GetEntryCount() > 1 )
    {
        m_pGroupOnLst->RemoveEntry(1);
    }

    switch(nDataType)
    {
        case sdbc::DataType::LONGVARCHAR:
        case sdbc::DataType::VARCHAR:
        case sdbc::DataType::CHAR:
            m_pGroupOnLst->InsertEntry(OUString(ModuleRes(STR_RPT_PREFIXCHARS)));
            m_pGroupOnLst->SetEntryData(1,reinterpret_cast<void*>(report::GroupOn::PREFIX_CHARACTERS));
            break;
        case sdbc::DataType::DATE:
        case sdbc::DataType::TIME:
        case sdbc::DataType::TIMESTAMP:
            {
                const sal_uInt16 nIds[] = { STR_RPT_YEAR, STR_RPT_QUARTER,STR_RPT_MONTH,STR_RPT_WEEK,STR_RPT_DAY,STR_RPT_HOUR,STR_RPT_MINUTE };
                for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(nIds); ++i)
                {
                    m_pGroupOnLst->InsertEntry(OUString(ModuleRes(nIds[i])));
                    m_pGroupOnLst->SetEntryData(i+1,reinterpret_cast<void*>(i+2));
                }
            }
            break;
        default:
            m_pGroupOnLst->InsertEntry(OUString(ModuleRes(STR_RPT_INTERVAL)));
            m_pGroupOnLst->SetEntryData(1,reinterpret_cast<void*>(report::GroupOn::INTERVAL));
            break;
    }
    sal_Int32 nPos = 0;
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
    m_pGroupOnLst->SelectEntryPos(nPos);
    m_pGroupIntervalEd->SetText(OUString::number(_xGroup->getGroupInterval()));
    m_pGroupIntervalEd->SaveValue();
    m_pGroupIntervalEd->Enable( nPos != 0 );
    m_pKeepTogetherLst->SelectEntryPos(_xGroup->getKeepTogether());
    m_pOrderLst->SelectEntryPos(_xGroup->getSortAscending() ? 0 : 1);

    ListBox* pControls[] = { m_pHeaderLst, m_pFooterLst, m_pGroupOnLst, m_pKeepTogetherLst, m_pOrderLst};
    for (ListBox* pControl : pControls)
        pControl->SaveValue();

    ListBox* pControlsLst2[] = { m_pHeaderLst, m_pFooterLst, m_pGroupOnLst, m_pKeepTogetherLst, m_pOrderLst};
    bool bReadOnly = !m_pController->isEditable();
    for (ListBox* i : pControlsLst2)
        i->SetReadOnly(bReadOnly);
    m_pGroupIntervalEd->SetReadOnly(bReadOnly);
}

void OGroupsSortingDialog::checkButtons(sal_Int32 _nRow)
{
    sal_Int32 nGroupCount = m_xGroups->getCount();
    sal_Int32 nRowCount = m_pFieldExpression->GetRowCount();
    bool bEnabled = nGroupCount > 1;

    if (bEnabled && _nRow > 0 )
    {
        m_pToolBox->EnableItem(m_nMoveUpId);
    }
    else
    {
        m_pToolBox->EnableItem(m_nMoveUpId, false);
    }
    if (bEnabled && _nRow < (nRowCount - 1) )
    {
        m_pToolBox->EnableItem(m_nMoveDownId);
    }
    else
    {
        m_pToolBox->EnableItem(m_nMoveDownId, false);
    }

    sal_Int32 nGroupPos = m_pFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos != NO_GROUP )
    {
        bool bEnableDelete = nGroupCount > 0;
        m_pToolBox->EnableItem(m_nDeleteId, bEnableDelete);
    }
    else
    {
        m_pToolBox->EnableItem(m_nDeleteId, false);
    }
}

} // rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
