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
#include <GroupsSorting.hxx>
#include <svtools/editbrowsebox.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <strings.hrc>
#include <rptui_slotid.hrc>
#include <core_resource.hxx>
#include <helpids.h>
#include "GroupExchange.hxx"
#include <UITools.hxx>
#include <UndoActions.hxx>
#include <strings.hxx>
#include <ReportController.hxx>
#include <ColumnInfo.hxx>

#include <cppuhelper/implbase.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>

#define HANDLE_ID           0
#define FIELD_EXPRESSION    1
#define GROUPS_START_LEN    5
#define NO_GROUP            -1

namespace rptui
{
using namespace ::com::sun::star;
using namespace svt;
using namespace ::comphelper;

    static void lcl_addToList_throw( weld::ComboBox& _rListBox, ::std::vector<ColumnInfo>& o_aColumnList,const uno::Reference< container::XNameAccess>& i_xColumns )
    {
        const uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
        for ( const OUString& rEntry : aEntries )
        {
            uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(rEntry),uno::UNO_QUERY_THROW);
            OUString sLabel;
            if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            o_aColumnList.emplace_back(rEntry,sLabel );
            if ( !sLabel.isEmpty() )
                _rListBox.append_text( sLabel );
            else
                _rListBox.append_text( rEntry );
        }
    }

/**
  * Separated out from OFieldExpressionControl to prevent collision of ref-counted base classes
  */
class OFieldExpressionControl;

namespace {

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

}

class OFieldExpressionControl : public ::svt::EditBrowseBox
{
    ::osl::Mutex                    m_aMutex;
    ::std::vector<sal_Int32>        m_aGroupPositions;
    ::std::vector<ColumnInfo>       m_aColumnInfo;
    VclPtr< ::svt::ComboBoxControl>  m_pComboCell;
    sal_Int32                       m_nDataPos;
    sal_Int32                       m_nCurrentPos;
    ImplSVEvent *                   m_nDeleteEvent;
    OGroupsSortingDialog*           m_pParent;
    bool                            m_bIgnoreEvent;
    rtl::Reference<OFieldExpressionControlContainerListener> aContainerListener;

public:
    OFieldExpressionControl(OGroupsSortingDialog* pParentDialog, const css::uno::Reference<css::awt::XWindow> &rParent);
    virtual ~OFieldExpressionControl() override;
    virtual void dispose() override;

    // XContainerListener
    /// @throws css::uno::RuntimeException
    void elementInserted(const css::container::ContainerEvent& rEvent);
    /// @throws css::uno::RuntimeException
    void elementRemoved(const css::container::ContainerEvent& rEvent);

    virtual Size GetOptimalSize() const override;

    void        fillColumns(const uno::Reference< container::XNameAccess>& _xColumns);
    void        lateInit();
    bool    IsDeleteAllowed( ) const;
    void        DeleteRows();

    sal_Int32   getGroupPosition(sal_Int32 _nRow) const { return _nRow != BROWSER_ENDOFSELECTION ? m_aGroupPositions[_nRow] : sal_Int32(NO_GROUP); }

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
    virtual void PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId ) const override;
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

    DECL_LINK( CBChangeHdl, weld::ComboBox&, void);

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

OFieldExpressionControl::OFieldExpressionControl(OGroupsSortingDialog* pParentDialog, const css::uno::Reference<css::awt::XWindow> &rParent)
    :EditBrowseBox( VCLUnoHelper::GetWindow(rParent), EditBrowseBoxFlags::NONE, WB_TABSTOP,
                    BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::AUTOSIZE_LASTCOL |
                              BrowserMode::KEEPHIGHLIGHT | BrowserMode::HLINES | BrowserMode::VLINES)
    ,m_aGroupPositions(GROUPS_START_LEN,-1)
    ,m_pComboCell(nullptr)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nDeleteEvent(nullptr)
    ,m_pParent(pParentDialog)
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
    if( m_nDeleteEvent )
        Application::RemoveUserEvent( m_nDeleteEvent );

    m_pComboCell.disposeAndClear();
    m_pParent = nullptr;
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
        for( long nIndex=FirstSelectedRow(); nIndex != SFX_ENDOFSELECTION; nIndex=NextSelectedRow() )
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
            aList = uno::Sequence< uno::Any >(vClipboardList.data(), vClipboardList.size());
    }
    return aList;
}

void OFieldExpressionControl::StartDrag( sal_Int8 /*_nAction*/ , const Point& /*_rPosPixel*/ )
{
    if ( m_pParent && !m_pParent->isReadOnly( ) )
    {
        uno::Sequence<uno::Any> aClipboardList = fillSelectedGroups();

        if( aClipboardList.hasElements() )
        {
            rtl::Reference<OGroupExchange> pData = new OGroupExchange(aClipboardList);
            pData->StartDrag(this, DND_ACTION_MOVE );
        }
    }
}

sal_Int8 OFieldExpressionControl::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    sal_Int8 nAction = DND_ACTION_NONE;
    if ( IsEditing() )
    {
        weld::ComboBox& rComboBox = m_pComboCell->get_widget();
        sal_Int32 nPos = rComboBox.get_active();
        if (nPos != -1 || !rComboBox.get_active_text().isEmpty())
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
        if ( aGroups.hasElements() )
        {
            moveGroups(aGroups,nRow);
            nAction = DND_ACTION_MOVE;
        }
    }
    return nAction;
}

void OFieldExpressionControl::moveGroups(const uno::Sequence<uno::Any>& _aGroups,sal_Int32 _nRow,bool _bSelect)
{
    if ( !_aGroups.hasElements() )
        return;

    m_bIgnoreEvent = true;
    {
        sal_Int32 nRow = _nRow;
        const OUString sUndoAction(RptResId(RID_STR_UNDO_MOVE_GROUP));
        const UndoContext aUndoContext( m_pParent->m_pController->getUndoManager(), sUndoAction );

        uno::Reference< report::XGroups> xGroups = m_pParent->getGroups();
        for(const uno::Any& rGroup : _aGroups)
        {
            uno::Reference< report::XGroup> xGroup(rGroup,uno::UNO_QUERY);
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

void OFieldExpressionControl::fillColumns(const uno::Reference< container::XNameAccess>& _xColumns)
{
    weld::ComboBox& rComboBox = m_pComboCell->get_widget();
    rComboBox.clear();
    if ( _xColumns.is() )
        lcl_addToList_throw(rComboBox, m_aColumnInfo, _xColumns);
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
        InsertDataColumn( FIELD_EXPRESSION, RptResId(STR_RPT_EXPRESSION), 100);

        m_pComboCell = VclPtr<ComboBoxControl>::Create( &GetDataWindow() );
        weld::ComboBox& rComboBox = m_pComboCell->get_widget();
        rComboBox.connect_changed(LINK(this,OFieldExpressionControl,CBChangeHdl));
        m_pComboCell->SetHelpId(HID_RPT_FIELDEXPRESSION);

        rComboBox.connect_focus_in(LINK(m_pParent, OGroupsSortingDialog, OnControlFocusGot));


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

IMPL_LINK_NOARG( OFieldExpressionControl, CBChangeHdl, weld::ComboBox&, void )
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
                OUString sUndoAction(RptResId(RID_STR_UNDO_APPEND_GROUP));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, OUString(), 0, ViewShellId(-1) );
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
                weld::ComboBox& rComboBox = m_pComboCell->get_widget();
                sal_Int32 nPos = rComboBox.get_active();
                OUString sExpression;
                if (nPos == -1)
                    sExpression = rComboBox.get_active_text();
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

            auto aIter = std::find_if(m_aColumnInfo.begin(), m_aColumnInfo.end(),
                [&sExpression](const ColumnInfo& rColumnInfo) { return rColumnInfo.sColumnName == sExpression; });
            if (aIter != m_aColumnInfo.end() && !aIter->sLabel.isEmpty())
                sExpression = aIter->sLabel;
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
    weld::ComboBox& rComboBox = m_pComboCell->get_widget();
    rComboBox.set_entry_text(GetCellText(nRow, nColumnId));
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
    pCellController->GetComboBox().set_entry_editable(m_pParent->m_pController->isEditable());
    return pCellController;
}

bool OFieldExpressionControl::SeekRow( long _nRow )
{
    // the basis class needs the call, because that's how the class knows which line will be painted
    EditBrowseBox::SeekRow(_nRow);
    m_nCurrentPos = _nRow;
    return true;
}

void OFieldExpressionControl::PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId ) const
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
            OSL_FAIL("Exception caught while try to get a group!");
        }
    }
    return EditBrowseBox::CLEAN;
}

// XContainerListener

void OFieldExpressionControl::elementInserted(const container::ContainerEvent& evt)
{
    if ( m_bIgnoreEvent )
        return;
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );
    sal_Int32 nGroupPos = 0;
    if ( !(evt.Accessor >>= nGroupPos) )
        return;

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

void OFieldExpressionControl::elementRemoved(const container::ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bIgnoreEvent )
        return;

    sal_Int32 nGroupPos = 0;
    if ( !(evt.Accessor >>= nGroupPos) )
        return;

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

bool OFieldExpressionControl::IsDeleteAllowed( ) const
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

            sal_uInt16 nColId = GetColumnId(GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X()));

            if ( nColId == HANDLE_ID )
            {
                bool bEnable = false;
                long nIndex = FirstSelectedRow();
                while( nIndex != SFX_ENDOFSELECTION && !bEnable )
                {
                    if ( m_aGroupPositions[nIndex] != NO_GROUP )
                        bEnable = true;
                    nIndex = NextSelectedRow();
                }
                VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/dbreport/ui/groupsortmenu.ui", "");
                VclPtr<PopupMenu> aContextMenu(aBuilder.get_menu("menu"));
                aContextMenu->EnableItem(aContextMenu->GetItemId("delete"), IsDeleteAllowed() && bEnable);
                if (aContextMenu->Execute(this, rEvt.GetMousePosPixel()))
                {
                    if( m_nDeleteEvent )
                        Application::RemoveUserEvent( m_nDeleteEvent );
                    m_nDeleteEvent = Application::PostUserEvent( LINK(this, OFieldExpressionControl, DelayedDelete), nullptr, true );
                }
            }
            [[fallthrough]];
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
    if (nIndex == SFX_ENDOFSELECTION)
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
                OUString sUndoAction(RptResId(RID_STR_UNDO_REMOVE_SELECTION));
                m_pParent->m_pController->getUndoManager().EnterListAction( sUndoAction, OUString(), 0, ViewShellId(-1) );
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

IMPL_LINK_NOARG( OFieldExpressionControl, DelayedDelete, void*, void )
{
    m_nDeleteEvent = nullptr;
    DeleteRows();
}

Size OFieldExpressionControl::GetOptimalSize() const
{
    return LogicToPixel(Size(106, 75), MapMode(MapUnit::MapAppFont));
}

OGroupsSortingDialog::OGroupsSortingDialog(weld::Window* pParent, bool bReadOnly,
                                           OReportController* pController)
    : GenericDialogController(pParent, "modules/dbreport/ui/floatingsort.ui", "FloatingSort")
    , OPropertyChangeListener(m_aMutex)
    , m_pController(pController)
    , m_xGroups(m_pController->getReportDefinition()->getGroups())
    , m_bReadOnly(bReadOnly)
    , m_xToolBox(m_xBuilder->weld_toolbar("toolbox"))
    , m_xProperties(m_xBuilder->weld_widget("properties"))
    , m_xOrderLst(m_xBuilder->weld_combo_box("sorting"))
    , m_xHeaderLst(m_xBuilder->weld_combo_box("header"))
    , m_xFooterLst(m_xBuilder->weld_combo_box("footer"))
    , m_xGroupOnLst(m_xBuilder->weld_combo_box("group"))
    , m_xGroupIntervalEd(m_xBuilder->weld_spin_button("interval"))
    , m_xKeepTogetherLst(m_xBuilder->weld_combo_box("keep"))
    , m_xHelpWindow(m_xBuilder->weld_label("helptext"))
    , m_xBox(m_xBuilder->weld_container("box"))
    , m_xTableCtrlParent(m_xBox->CreateChildFrame())
    , m_xFieldExpression(VclPtr<OFieldExpressionControl>::Create(this, m_xTableCtrlParent))
{
    m_xHelpWindow->set_size_request(-1, m_xHelpWindow->get_text_height() * 4);
    m_xFieldExpression->set_hexpand(true);
    m_xFieldExpression->set_vexpand(true);

    weld::Widget* pControlsLst[] = { m_xHeaderLst.get(), m_xFooterLst.get(), m_xGroupOnLst.get(),
                                     m_xKeepTogetherLst.get(), m_xOrderLst.get(), m_xGroupIntervalEd.get() };
    for (weld::Widget* i : pControlsLst)
    {
        i->connect_focus_in(LINK(this, OGroupsSortingDialog, OnWidgetFocusGot));
        i->show();
    }

    m_xGroupIntervalEd->connect_focus_out(LINK(this, OGroupsSortingDialog, OnWidgetFocusLost));

    for (size_t i = 0; i < SAL_N_ELEMENTS(pControlsLst) - 1; ++i)
        dynamic_cast<weld::ComboBox&>(*pControlsLst[i]).connect_changed(LINK(this,OGroupsSortingDialog,LBChangeHdl));

    m_pReportListener = new OPropertyChangeMultiplexer(this,m_pController->getReportDefinition().get());
    m_pReportListener->addProperty(PROPERTY_COMMAND);
    m_pReportListener->addProperty(PROPERTY_COMMANDTYPE);

    m_xFieldExpression->lateInit();
    fillColumns();
    Size aPrefSize = m_xFieldExpression->GetOptimalSize();
    m_xBox->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    m_xFieldExpression->Show();

    m_xToolBox->connect_clicked(LINK(this, OGroupsSortingDialog, OnFormatAction));

    checkButtons(0);
}

OGroupsSortingDialog::~OGroupsSortingDialog()
{
    m_pReportListener->dispose();
    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
    m_xFieldExpression.disposeAndClear();
    m_xTableCtrlParent->dispose();
    m_xTableCtrlParent.clear();
}

void OGroupsSortingDialog::UpdateData( )
{
    m_xFieldExpression->Invalidate();
    long nCurRow = m_xFieldExpression->GetCurRow();
    m_xFieldExpression->DeactivateCell();
    m_xFieldExpression->ActivateCell(nCurRow, m_xFieldExpression->GetCurColumnId());
    DisplayData(nCurRow);
}

void OGroupsSortingDialog::DisplayData( sal_Int32 _nRow )
{
    const sal_Int32 nGroupPos = m_xFieldExpression->getGroupPosition(_nRow);
    const bool bEmpty = nGroupPos == NO_GROUP;
    m_xProperties->set_sensitive(!bEmpty);

    checkButtons(_nRow);

    if ( m_pCurrentGroupListener.is() )
        m_pCurrentGroupListener->dispose();
    m_pCurrentGroupListener = nullptr;
    if (!bEmpty)
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
    sal_Int32 nGroupPos = m_xFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos == NO_GROUP )
        return;

    uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
    if (m_xHeaderLst->get_value_changed_from_saved())
        xGroup->setHeaderOn( m_xHeaderLst->get_active() == 0 );
    if (m_xFooterLst->get_value_changed_from_saved())
        xGroup->setFooterOn( m_xFooterLst->get_active() == 0 );
    if (m_xKeepTogetherLst->get_value_changed_from_saved())
        xGroup->setKeepTogether( m_xKeepTogetherLst->get_active() );
    if (m_xGroupOnLst->get_value_changed_from_saved())
    {
        auto nGroupOn = m_xGroupOnLst->get_active_id().toInt32();
        xGroup->setGroupOn( nGroupOn );
    }
    if (m_xGroupIntervalEd->get_value_changed_from_saved())
    {
        xGroup->setGroupInterval(m_xGroupIntervalEd->get_value());
        m_xGroupIntervalEd->save_value();
    }
    if ( m_xOrderLst->get_value_changed_from_saved() )
        xGroup->setSortAscending( m_xOrderLst->get_active() == 0 );

    weld::ComboBox* pControls[] = { m_xHeaderLst.get(), m_xFooterLst.get(), m_xGroupOnLst.get(),
                                    m_xKeepTogetherLst.get(), m_xOrderLst.get() };
    for (weld::ComboBox* pControl : pControls)
        pControl->save_value();
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

IMPL_LINK_NOARG(OGroupsSortingDialog, OnControlFocusGot, weld::Widget&, void )
{
    m_xHelpWindow->set_label(RptResId(STR_RPT_HELP_FIELD));
}

IMPL_LINK(OGroupsSortingDialog, OnWidgetFocusGot, weld::Widget&, rControl, void )
{
    const std::pair<weld::Widget*, const char*> pControls[] = {
        { m_xHeaderLst.get(), STR_RPT_HELP_HEADER },
        { m_xFooterLst.get(), STR_RPT_HELP_FOOTER },
        { m_xGroupOnLst.get(), STR_RPT_HELP_GROUPON },
        { m_xGroupIntervalEd.get(), STR_RPT_HELP_INTERVAL },
        { m_xKeepTogetherLst.get(), STR_RPT_HELP_KEEP },
        { m_xOrderLst.get(), STR_RPT_HELP_SORT }
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(pControls); ++i)
    {
        if (&rControl == pControls[i].first)
        {
            weld::ComboBox* pListBox = dynamic_cast<weld::ComboBox*>( &rControl );
            if ( pListBox )
                pListBox->save_value();
            weld::SpinButton* pNumericField = dynamic_cast<weld::SpinButton*>(&rControl);
            if ( pNumericField )
                pNumericField->save_value();
            //shows the text given by the id in the multiline edit
            m_xHelpWindow->set_label(RptResId(pControls[i].second));
            break;
        }
    }
}

IMPL_LINK_NOARG(OGroupsSortingDialog, OnWidgetFocusLost, weld::Widget&, void)
{
    if (m_xFieldExpression)
    {
        if (m_xGroupIntervalEd->get_value_changed_from_saved())
            SaveData(m_xFieldExpression->GetCurRow());
    }
}

IMPL_LINK(OGroupsSortingDialog, OnFormatAction, const OString&, rCommand, void)
{
    if ( !m_xFieldExpression )
        return;

    long nIndex = m_xFieldExpression->GetCurrRow();
    sal_Int32 nGroupPos = m_xFieldExpression->getGroupPosition(nIndex);
    uno::Sequence<uno::Any> aClipboardList;
    if ( nIndex >= 0 && nGroupPos != NO_GROUP )
    {
        aClipboardList.realloc(1);
        aClipboardList[0] = m_xGroups->getByIndex(nGroupPos);
    }
    if (rCommand == "up")
    {
        --nIndex;
    }
    if (rCommand == "down")
    {
        ++nIndex;
    }
    if (rCommand == "delete")
    {
        Application::PostUserEvent(LINK(m_xFieldExpression, OFieldExpressionControl, DelayedDelete));
    }
    else
    {
        if ( nIndex >= 0 && aClipboardList.hasElements() )
        {
            m_xFieldExpression->SetNoSelection();
            m_xFieldExpression->moveGroups(aClipboardList,nIndex,false);
            m_xFieldExpression->DeactivateCell();
            m_xFieldExpression->GoToRow(nIndex);
            m_xFieldExpression->ActivateCell(nIndex, m_xFieldExpression->GetCurColumnId());
            DisplayData(nIndex);
        }
    }
}

IMPL_LINK( OGroupsSortingDialog, LBChangeHdl, weld::ComboBox&, rListBox, void )
{
    if ( !rListBox.get_value_changed_from_saved() )
        return;

    sal_Int32 nRow = m_xFieldExpression->GetCurRow();
    sal_Int32 nGroupPos = m_xFieldExpression->getGroupPosition(nRow);
    if (&rListBox != m_xHeaderLst.get() && &rListBox != m_xFooterLst.get())
    {
        if ( rListBox.get_value_changed_from_saved() )
            SaveData(nRow);
        if ( &rListBox == m_xGroupOnLst.get() )
            m_xGroupIntervalEd->set_sensitive(rListBox.get_active() != 0);
    }
    else if ( nGroupPos != NO_GROUP )
    {
        uno::Reference< report::XGroup> xGroup = getGroup(nGroupPos);
        uno::Sequence< beans::PropertyValue > aArgs(2);
        aArgs[1].Name = PROPERTY_GROUP;
        aArgs[1].Value <<= xGroup;

        if ( m_xHeaderLst.get() == &rListBox )
            aArgs[0].Name = PROPERTY_HEADERON;
        else
            aArgs[0].Name = PROPERTY_FOOTERON;

        aArgs[0].Value <<= rListBox.get_active() == 0;
        m_pController->executeChecked(m_xHeaderLst.get() == &rListBox ? SID_GROUPHEADER : SID_GROUPFOOTER, aArgs);
        m_xFieldExpression->InvalidateHandleColumn();
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
    m_xFieldExpression->fillColumns(m_xColumns);
}

void OGroupsSortingDialog::displayGroup(const uno::Reference<report::XGroup>& _xGroup)
{
    m_xHeaderLst->set_active(_xGroup->getHeaderOn() ? 0 : 1 );
    m_xFooterLst->set_active(_xGroup->getFooterOn() ? 0 : 1 );
    sal_Int32 nDataType = getColumnDataType(_xGroup->getExpression());

    // first clear whole group on list
    while (m_xGroupOnLst->get_count() > 1 )
    {
        m_xGroupOnLst->remove(1);
    }

    switch(nDataType)
    {
        case sdbc::DataType::LONGVARCHAR:
        case sdbc::DataType::VARCHAR:
        case sdbc::DataType::CHAR:
            m_xGroupOnLst->append(OUString::number(report::GroupOn::PREFIX_CHARACTERS), RptResId(STR_RPT_PREFIXCHARS));
            break;
        case sdbc::DataType::DATE:
        case sdbc::DataType::TIME:
        case sdbc::DataType::TIMESTAMP:
            {
                const char* aIds[] = { STR_RPT_YEAR, STR_RPT_QUARTER,STR_RPT_MONTH,STR_RPT_WEEK,STR_RPT_DAY,STR_RPT_HOUR,STR_RPT_MINUTE };
                for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
                {
                    m_xGroupOnLst->append(OUString::number(i+2), RptResId(aIds[i]));
                }
            }
            break;
        default:
            m_xGroupOnLst->append(OUString::number(report::GroupOn::INTERVAL), RptResId(STR_RPT_INTERVAL));
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
    m_xGroupOnLst->set_active(nPos);
    m_xGroupIntervalEd->set_value(_xGroup->getGroupInterval());
    m_xGroupIntervalEd->save_value();
    m_xGroupIntervalEd->set_sensitive( nPos != 0 );
    m_xKeepTogetherLst->set_active(_xGroup->getKeepTogether());
    m_xOrderLst->set_active(_xGroup->getSortAscending() ? 0 : 1);

    weld::ComboBox* pControls[] = { m_xHeaderLst.get(), m_xFooterLst.get(), m_xGroupOnLst.get(),
                                    m_xKeepTogetherLst.get(), m_xOrderLst.get() };
    for (weld::ComboBox* pControl : pControls)
        pControl->save_value();

    bool bReadOnly = !m_pController->isEditable();
    for (weld::ComboBox* pControl : pControls)
        pControl->set_sensitive(!bReadOnly);
    m_xGroupIntervalEd->set_editable(!bReadOnly);
}

void OGroupsSortingDialog::checkButtons(sal_Int32 _nRow)
{
    sal_Int32 nGroupCount = m_xGroups->getCount();
    sal_Int32 nRowCount = m_xFieldExpression->GetRowCount();
    bool bEnabled = nGroupCount > 1;

    if (bEnabled && _nRow > 0 )
    {
        m_xToolBox->set_item_sensitive("up", true);
    }
    else
    {
        m_xToolBox->set_item_sensitive("up", false);
    }
    if (bEnabled && _nRow < (nRowCount - 1) )
    {
        m_xToolBox->set_item_sensitive("down", true);
    }
    else
    {
        m_xToolBox->set_item_sensitive("down", false);
    }

    sal_Int32 nGroupPos = m_xFieldExpression->getGroupPosition(_nRow);
    if ( nGroupPos != NO_GROUP )
    {
        bool bEnableDelete = nGroupCount > 0;
        m_xToolBox->set_item_sensitive("delete", bEnableDelete);
    }
    else
    {
        m_xToolBox->set_item_sensitive("delete", false);
    }
}

} // rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
