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

#include <sal/macros.h>
#include <helpids.h>
#include <svx/gridctrl.hxx>
#include <gridcell.hxx>
#include <svx/fmtools.hxx>
#include <svtools/stringtransfer.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>

#include <fmprop.hxx>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/XRowsChangeBroadcaster.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <tools/diagnose_ex.h>
#include <tools/fract.hxx>
#include <vcl/builder.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>

#include <svx/strings.hrc>

#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <fmservs.hxx>
#include <sdbdatacolumn.hxx>

#include <comphelper/property.hxx>
#include <cppuhelper/implbase.hxx>

#include <algorithm>
#include <cstdlib>
#include <map>
#include <memory>

using namespace ::dbtools;
using namespace ::dbtools::DBTypeConversion;
using namespace ::svxform;
using namespace ::svt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::container;
using namespace com::sun::star::accessibility;

#define ROWSTATUS(row) (!row.is() ? "NULL" : row->GetStatus() == GridRowStatus::Clean ? "CLEAN" : row->GetStatus() == GridRowStatus::Modified ? "MODIFIED" : row->GetStatus() == GridRowStatus::Deleted ? "DELETED" : "INVALID")

#define DEFAULT_BROWSE_MODE             \
              BrowserMode::COLUMNSELECTION   \
            | BrowserMode::MULTISELECTION    \
            | BrowserMode::KEEPHIGHLIGHT     \
            | BrowserMode::TRACKING_TIPS     \
            | BrowserMode::HLINES        \
            | BrowserMode::VLINES        \
            | BrowserMode::HEADERBAR_NEW     \

class RowSetEventListener : public ::cppu::WeakImplHelper<XRowsChangeListener>
{
    VclPtr<DbGridControl> m_pControl;
public:
    explicit RowSetEventListener(DbGridControl* i_pControl) : m_pControl(i_pControl)
    {
    }

private:
    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& /*i_aEvt*/) override
    {
    }
    virtual void SAL_CALL rowsChanged(const css::sdb::RowsChangeEvent& i_aEvt) override
    {
        if ( i_aEvt.Action == RowChangeAction::UPDATE )
        {
            ::DbGridControl::GrantControlAccess aAccess;
            CursorWrapper* pSeek = m_pControl->GetSeekCursor(aAccess);
            const DbGridRowRef& rSeekRow = m_pControl->GetSeekRow(aAccess);
            const Any* pIter = i_aEvt.Bookmarks.getConstArray();
            const Any* pEnd  = pIter + i_aEvt.Bookmarks.getLength();
            for(;pIter != pEnd;++pIter)
            {
                pSeek->moveToBookmark(*pIter);
                // get the data
                rSeekRow->SetState(pSeek, true);
                sal_Int32 nSeekPos = pSeek->getRow() - 1;
                m_pControl->SetSeekPos(nSeekPos,aAccess);
                m_pControl->RowModified(nSeekPos);
            }
        }
    }
};

class GridFieldValueListener;
typedef std::map<sal_uInt16, GridFieldValueListener*> ColumnFieldValueListeners;

class GridFieldValueListener : protected ::comphelper::OPropertyChangeListener
{
    osl::Mutex                          m_aMutex;
    DbGridControl&                      m_rParent;
    rtl::Reference<::comphelper::OPropertyChangeMultiplexer> m_pRealListener;
    sal_uInt16                          m_nId;
    sal_Int16                           m_nSuspended;
    bool                                m_bDisposed : 1;

public:
    GridFieldValueListener(DbGridControl& _rParent, const Reference< XPropertySet >& xField, sal_uInt16 _nId);
    virtual ~GridFieldValueListener() override;

    virtual void _propertyChanged(const PropertyChangeEvent& evt) override;

    void suspend() { ++m_nSuspended; }
    void resume() { --m_nSuspended; }

    void dispose();
};

GridFieldValueListener::GridFieldValueListener(DbGridControl& _rParent, const Reference< XPropertySet >& _rField, sal_uInt16 _nId)
    :OPropertyChangeListener(m_aMutex)
    ,m_rParent(_rParent)
    ,m_nId(_nId)
    ,m_nSuspended(0)
    ,m_bDisposed(false)
{
    if (_rField.is())
    {
        m_pRealListener = new ::comphelper::OPropertyChangeMultiplexer(this, _rField);
        m_pRealListener->addProperty(FM_PROP_VALUE);
    }
}

GridFieldValueListener::~GridFieldValueListener()
{
    dispose();
}

void GridFieldValueListener::_propertyChanged(const PropertyChangeEvent& /*_evt*/)
{
    DBG_ASSERT(m_nSuspended>=0, "GridFieldValueListener::_propertyChanged : resume > suspend !");
    if (m_nSuspended <= 0)
        m_rParent.FieldValueChanged(m_nId);
}

void GridFieldValueListener::dispose()
{
    if (m_bDisposed)
    {
        DBG_ASSERT(m_pRealListener.get() == nullptr, "GridFieldValueListener::dispose : inconsistent !");
        return;
    }

    if (m_pRealListener.is())
    {
        m_pRealListener->dispose();
        m_pRealListener.clear();
    }

    m_bDisposed = true;
    m_rParent.FieldListenerDisposing(m_nId);
}

class DisposeListenerGridBridge : public FmXDisposeListener
{
    DbGridControl&          m_rParent;
    rtl::Reference<FmXDisposeMultiplexer>  m_xRealListener;

public:
    DisposeListenerGridBridge(  DbGridControl& _rParent, const Reference< XComponent >& _rxObject);
    virtual ~DisposeListenerGridBridge() override;

    virtual void disposing(const EventObject& /*_rEvent*/, sal_Int16 _nId) override { m_rParent.disposing(_nId); }
};

DisposeListenerGridBridge::DisposeListenerGridBridge(DbGridControl& _rParent, const Reference< XComponent >& _rxObject)
    :FmXDisposeListener()
    ,m_rParent(_rParent)
{

    if (_rxObject.is())
    {
        m_xRealListener = new FmXDisposeMultiplexer(this, _rxObject);
    }
}

DisposeListenerGridBridge::~DisposeListenerGridBridge()
{
    if (m_xRealListener.is())
    {
        m_xRealListener->dispose();
    }
}

static const DbGridControlNavigationBarState ControlMap[] =
    {
        DbGridControlNavigationBarState::Text,
        DbGridControlNavigationBarState::Absolute,
        DbGridControlNavigationBarState::Of,
        DbGridControlNavigationBarState::Count,
        DbGridControlNavigationBarState::First,
        DbGridControlNavigationBarState::Next,
        DbGridControlNavigationBarState::Prev,
        DbGridControlNavigationBarState::Last,
        DbGridControlNavigationBarState::New,
        DbGridControlNavigationBarState::NONE
    };

bool CompareBookmark(const Any& aLeft, const Any& aRight)
{
    return aLeft == aRight;
}

class FmXGridSourcePropListener : public ::comphelper::OPropertyChangeListener
{
    VclPtr<DbGridControl> m_pParent;

    // a DbGridControl has no mutex, so we use our own as the base class expects one
    osl::Mutex          m_aMutex;
    sal_Int16           m_nSuspended;

public:
    explicit FmXGridSourcePropListener(DbGridControl* _pParent);

    void suspend() { ++m_nSuspended; }
    void resume() { --m_nSuspended; }

    virtual void _propertyChanged(const PropertyChangeEvent& evt) override;
};

FmXGridSourcePropListener::FmXGridSourcePropListener(DbGridControl* _pParent)
    :OPropertyChangeListener(m_aMutex)
    ,m_pParent(_pParent)
    ,m_nSuspended(0)
{
    DBG_ASSERT(m_pParent, "FmXGridSourcePropListener::FmXGridSourcePropListener : invalid parent !");
}

void FmXGridSourcePropListener::_propertyChanged(const PropertyChangeEvent& evt)
{
    DBG_ASSERT(m_nSuspended>=0, "FmXGridSourcePropListener::_propertyChanged : resume > suspend !");
    if (m_nSuspended <= 0)
        m_pParent->DataSourcePropertyChanged(evt);
}

DbGridControl::NavigationBar::AbsolutePos::AbsolutePos(vcl::Window* pParent, WinBits nStyle)
                   :NumericField(pParent, nStyle)
{
    SetMin(1);
    SetFirst(1);
    SetSpinSize(1);

    SetDecimalDigits(0);
    SetStrictFormat(true);
}

void DbGridControl::NavigationBar::AbsolutePos::KeyInput(const KeyEvent& rEvt)
{
    if (rEvt.GetKeyCode() == KEY_RETURN && !GetText().isEmpty())
    {
        sal_Int64 nRecord = GetValue();
        if (nRecord < GetMin() || nRecord > GetMax())
            return;
        else
            static_cast<NavigationBar*>(GetParent())->PositionDataSource(static_cast<sal_Int32>(nRecord));
    }
    else if (rEvt.GetKeyCode() == KEY_TAB)
        GetParent()->GetParent()->GrabFocus();
    else
        NumericField::KeyInput(rEvt);
}

void DbGridControl::NavigationBar::AbsolutePos::LoseFocus()
{
    NumericField::LoseFocus();
    sal_Int64 nRecord = GetValue();
    if (nRecord < GetMin() || nRecord > GetMax())
        return;
    else
    {
        static_cast<NavigationBar*>(GetParent())->PositionDataSource(static_cast<sal_Int32>(nRecord));
        static_cast<NavigationBar*>(GetParent())->InvalidateState(DbGridControlNavigationBarState::Absolute);
    }
}

void DbGridControl::NavigationBar::PositionDataSource(sal_Int32 nRecord)
{
    if (m_bPositioning)
        return;
    // the MoveToPosition may cause a LoseFocus which would lead to a second MoveToPosition,
    // so protect against this recursion
    m_bPositioning = true;
    static_cast<DbGridControl*>(GetParent())->MoveToPosition(nRecord - 1);
    m_bPositioning = false;
}

DbGridControl::NavigationBar::NavigationBar(vcl::Window* pParent)
          :Control(pParent, 0)
          ,m_aRecordText(VclPtr<FixedText>::Create(this, WB_VCENTER))
          ,m_aAbsolute(VclPtr<DbGridControl::NavigationBar::AbsolutePos>::Create(this, WB_CENTER | WB_VCENTER))
          ,m_aRecordOf(VclPtr<FixedText>::Create(this, WB_VCENTER))
          ,m_aRecordCount(VclPtr<FixedText>::Create(this, WB_VCENTER))
          ,m_aFirstBtn(VclPtr<ImageButton>::Create(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS))
          ,m_aPrevBtn(VclPtr<ImageButton>::Create(this, WB_REPEAT|WB_RECTSTYLE|WB_NOPOINTERFOCUS))
          ,m_aNextBtn(VclPtr<ImageButton>::Create(this, WB_REPEAT|WB_RECTSTYLE|WB_NOPOINTERFOCUS))
          ,m_aLastBtn(VclPtr<ImageButton>::Create(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS))
          ,m_aNewBtn(VclPtr<ImageButton>::Create(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS))
          ,m_nCurrentPos(-1)
          ,m_bPositioning(false)
{
    m_aFirstBtn->SetSymbol(SymbolType::FIRST);
    m_aPrevBtn->SetSymbol(SymbolType::PREV);
    m_aNextBtn->SetSymbol(SymbolType::NEXT);
    m_aLastBtn->SetSymbol(SymbolType::LAST);
    m_aNewBtn->SetModeImage(static_cast<DbGridControl*>(pParent)->GetImage(EditBrowseBox::NEW));

    m_aFirstBtn->SetHelpId(HID_GRID_TRAVEL_FIRST);
    m_aPrevBtn->SetHelpId(HID_GRID_TRAVEL_PREV);
    m_aNextBtn->SetHelpId(HID_GRID_TRAVEL_NEXT);
    m_aLastBtn->SetHelpId(HID_GRID_TRAVEL_LAST);
    m_aNewBtn->SetHelpId(HID_GRID_TRAVEL_NEW);
    m_aAbsolute->SetHelpId(HID_GRID_TRAVEL_ABSOLUTE);
    m_aRecordCount->SetHelpId(HID_GRID_NUMBEROFRECORDS);

    // set handlers for buttons
    m_aFirstBtn->SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aPrevBtn->SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aNextBtn->SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aLastBtn->SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aNewBtn->SetClickHdl(LINK(this,NavigationBar,OnClick));

    m_aRecordText->SetText(SvxResId(RID_STR_REC_TEXT));
    m_aRecordOf->SetText(SvxResId(RID_STR_REC_FROM_TEXT));
    m_aRecordCount->SetText(OUString('?'));

    m_aFirstBtn->Disable();
    m_aPrevBtn->Disable();
    m_aNextBtn->Disable();
    m_aLastBtn->Disable();
    m_aNewBtn->Disable();
    m_aRecordText->Disable();
    m_aRecordOf->Disable();
    m_aRecordCount->Disable();
    m_aAbsolute->Disable();

    AllSettings aSettings = m_aNextBtn->GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 4);
    aSettings.SetMouseSettings(aMouseSettings);
    m_aNextBtn->SetSettings(aSettings, true);
    m_aPrevBtn->SetSettings(aSettings, true);

    m_aFirstBtn->Show();
    m_aPrevBtn->Show();
    m_aNextBtn->Show();
    m_aLastBtn->Show();
    m_aNewBtn->Show();
    m_aRecordText->Show();
    m_aRecordOf->Show();
    m_aRecordCount->Show();
    m_aAbsolute->Show();
}


DbGridControl::NavigationBar::~NavigationBar()
{
    disposeOnce();
}

void DbGridControl::NavigationBar::dispose()
{
    m_aRecordText.disposeAndClear();
    m_aAbsolute.disposeAndClear();
    m_aRecordOf.disposeAndClear();
    m_aRecordCount.disposeAndClear();
    m_aFirstBtn.disposeAndClear();
    m_aPrevBtn.disposeAndClear();
    m_aNextBtn.disposeAndClear();
    m_aLastBtn.disposeAndClear();
    m_aNewBtn.disposeAndClear();
    Control::dispose();
}

namespace
{
    void SetPosAndSize(Button& _rButton,Point& _rPos,const Size& _rSize)
    {
        _rButton.SetPosPixel( _rPos );
        _rButton.SetSizePixel( _rSize );
        _rPos.AdjustX(static_cast<sal_uInt16>(_rSize.Width()) );
    }
}

sal_uInt16 DbGridControl::NavigationBar::ArrangeControls()
{
    // positioning of the controls
    // calculate base size
    tools::Rectangle   aRect(static_cast<DbGridControl*>(GetParent())->GetControlArea());
    long nH = aRect.GetSize().Height();
    long nW = GetParent()->GetOutputSizePixel().Width();
    Size aBorder = LogicToPixel(Size(2, 2), MapMode(MapUnit::MapAppFont));
    aBorder = Size(CalcZoom(aBorder.Width()), CalcZoom(aBorder.Height()));
    sal_uInt16      nX = 1;
    sal_uInt16      nY = 0;

    // Is the font of this edit larger than the field?
    if (m_aAbsolute->GetTextHeight() > nH)
    {
        vcl::Font aApplFont (m_aAbsolute->GetFont());
        const Size pointAbsoluteSize(m_aAbsolute->PixelToLogic( Size( 0, nH - 2 ), MapMode(MapUnit::MapPoint) ));
        aApplFont.SetFontSize( pointAbsoluteSize );
        m_aAbsolute->SetControlFont( aApplFont );

        aApplFont.SetTransparent( true );
        m_aRecordText->SetControlFont( aApplFont );
        m_aRecordOf->SetControlFont( aApplFont );
        m_aRecordCount->SetControlFont( aApplFont );
    }

    // set size and position of the control
    OUString aText = m_aRecordText->GetText();
    long nTextWidth = m_aRecordText->GetTextWidth(aText);
    m_aRecordText->SetPosPixel(Point(nX,nY));
    m_aRecordText->SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    // count an extra hairspace (U+200A) left and right
    const OUString sevenDigits(m_aAbsolute->CreateFieldText(6000000));
    const OUString hairSpace(u'\x200A');
    OUString textPattern(hairSpace);
    textPattern += sevenDigits;
    textPattern += hairSpace;
    nTextWidth = m_aAbsolute->GetTextWidth(textPattern);
    m_aAbsolute->SetPosPixel(Point(nX,nY));
    m_aAbsolute->SetSizePixel(Size(nTextWidth, nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    aText      = m_aRecordOf->GetText();
    nTextWidth = m_aRecordOf->GetTextWidth(aText);
    m_aRecordOf->SetPosPixel(Point(nX,nY));
    m_aRecordOf->SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    textPattern = sevenDigits + " * (" + sevenDigits + ")";
    nTextWidth = m_aRecordCount->GetTextWidth(textPattern);
    m_aRecordCount->SetPosPixel(Point(nX,nY));
    m_aRecordCount->SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    Point aButtonPos(nX,nY);
    const Size  aButtonSize(nH,nH);
    SetPosAndSize(*m_aFirstBtn.get(), aButtonPos, aButtonSize);
    SetPosAndSize(*m_aPrevBtn.get(), aButtonPos, aButtonSize);
    SetPosAndSize(*m_aNextBtn.get(), aButtonPos, aButtonSize);
    SetPosAndSize(*m_aLastBtn.get(), aButtonPos, aButtonSize);
    SetPosAndSize(*m_aNewBtn.get(), aButtonPos, aButtonSize);

    nX = sal::static_int_cast< sal_uInt16 >(aButtonPos.X() + 1);

    nW -= GetSettings().GetStyleSettings().GetScrollBarSize();

    if (nX > nW)
    {
        aButtonPos.setX( nW-nH );
        m_aNewBtn->SetPosPixel(aButtonPos);
        aButtonPos.AdjustX( -nH );
        m_aLastBtn->SetPosPixel(aButtonPos);
        aButtonPos.AdjustX( -nH );
        m_aNextBtn->SetPosPixel(aButtonPos);
        aButtonPos.AdjustX( -nH );
        m_aPrevBtn->SetPosPixel(aButtonPos);
        aButtonPos.AdjustX( -nH );
        m_aFirstBtn->SetPosPixel(aButtonPos);

        auto nDiff = nX - nW;

        Size aSize = m_aAbsolute->GetSizePixel();
        aSize.AdjustWidth( -(nDiff/3.0) );
        m_aAbsolute->SetSizePixel(aSize);

        aSize = m_aRecordCount->GetSizePixel();
        aSize.AdjustWidth( -(nDiff/3.0*2) );
        m_aRecordCount->SetSizePixel(aSize);

        Point aPos = m_aRecordOf->GetPosPixel();
        aPos.AdjustX( -(nDiff/3.0) );
        m_aRecordOf->SetPosPixel(aPos);

        aPos = m_aRecordCount->GetPosPixel();
        aPos.AdjustX( -(nDiff/3.0) );
        m_aRecordCount->SetPosPixel(aPos);

        vcl::Window* pWindows[] =
        {
            m_aRecordText.get(),
            m_aAbsolute.get(),
            m_aRecordOf.get(),
            m_aRecordCount.get(),
            m_aFirstBtn.get(),
            m_aPrevBtn.get(),
            m_aNextBtn.get(),
            m_aLastBtn.get(),
            m_aNewBtn.get()
        };

        for (vcl::Window* pWindow : pWindows)
        {
            if (pWindow->GetPosPixel().X() < 0)
                pWindow->SetSizePixel(Size(0, nH));
            aSize = pWindow->GetSizePixel();
            auto nExcess = (pWindow->GetPosPixel().X() + aSize.Width()) - nW;
            if (nExcess > 0)
            {
                aSize.AdjustWidth( -nExcess );
                pWindow->SetSizePixel(aSize);
            }
        }

        nX = nW;
    }

    return nX;
}

IMPL_LINK(DbGridControl::NavigationBar, OnClick, Button *, pButton, void )
{
    DbGridControl* pParent = static_cast<DbGridControl*>(GetParent());

    if (pParent->m_aMasterSlotExecutor.IsSet())
    {
        bool lResult = false;
        if (pButton == m_aFirstBtn.get())
            lResult = pParent->m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::First);
        else if( pButton == m_aPrevBtn.get() )
            lResult = pParent->m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::Prev);
        else if( pButton == m_aNextBtn.get() )
            lResult = pParent->m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::Next);
        else if( pButton == m_aLastBtn.get() )
            lResult = pParent->m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::Last);
        else if( pButton == m_aNewBtn.get() )
            lResult = pParent->m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::New);

        if (lResult)
            // the link already handled it
            return;
    }

    if (pButton == m_aFirstBtn.get())
        pParent->MoveToFirst();
    else if( pButton == m_aPrevBtn.get() )
        pParent->MoveToPrev();
    else if( pButton == m_aNextBtn.get() )
        pParent->MoveToNext();
    else if( pButton == m_aLastBtn.get() )
        pParent->MoveToLast();
    else if( pButton == m_aNewBtn.get() )
        pParent->AppendNew();
}

void DbGridControl::NavigationBar::InvalidateAll(sal_Int32 nCurrentPos, bool bAll)
{
    if (m_nCurrentPos != nCurrentPos || nCurrentPos < 0 || bAll)
    {
        DbGridControl* pParent = static_cast<DbGridControl*>(GetParent());

        sal_Int32 nAdjustedRowCount = pParent->GetRowCount() - ((pParent->GetOptions() & DbGridControlOptions::Insert) ? 2 : 1);

        // check if everything needs to be invalidated
        bAll = bAll || m_nCurrentPos <= 0;
        bAll = bAll || nCurrentPos <= 0;
        bAll = bAll || m_nCurrentPos >= nAdjustedRowCount;
        bAll = bAll || nCurrentPos >= nAdjustedRowCount;

        if ( bAll )
        {
            m_nCurrentPos = nCurrentPos;
            int i = 0;
            while (ControlMap[i] != DbGridControlNavigationBarState::NONE)
                SetState(ControlMap[i++]);
        }
        else    // is in the center
        {
            m_nCurrentPos = nCurrentPos;
            SetState(DbGridControlNavigationBarState::Count);
            SetState(DbGridControlNavigationBarState::Absolute);
        }
    }
}

bool DbGridControl::NavigationBar::GetState(DbGridControlNavigationBarState nWhich) const
{
    DbGridControl* pParent = static_cast<DbGridControl*>(GetParent());

    if (!pParent->IsOpen() || pParent->IsDesignMode() || !pParent->IsEnabled()
        || pParent->IsFilterMode() )
        return false;
    else
    {
        // check if we have a master state provider
        if (pParent->m_aMasterStateProvider.IsSet())
        {
            long nState = pParent->m_aMasterStateProvider.Call( nWhich );
            if (nState>=0)
                return (nState>0);
        }

        bool bAvailable = true;

        switch (nWhich)
        {
            case DbGridControlNavigationBarState::First:
            case DbGridControlNavigationBarState::Prev:
                bAvailable = m_nCurrentPos > 0;
                break;
            case DbGridControlNavigationBarState::Next:
                if(pParent->m_bRecordCountFinal)
                {
                    bAvailable = m_nCurrentPos < pParent->GetRowCount() - 1;
                    if (!bAvailable && pParent->GetOptions() & DbGridControlOptions::Insert)
                        bAvailable = (m_nCurrentPos == pParent->GetRowCount() - 2) && pParent->IsModified();
                }
                break;
            case DbGridControlNavigationBarState::Last:
                if(pParent->m_bRecordCountFinal)
                {
                    if (pParent->GetOptions() & DbGridControlOptions::Insert)
                        bAvailable = pParent->IsCurrentAppending() ? pParent->GetRowCount() > 1 :
                                     m_nCurrentPos != pParent->GetRowCount() - 2;
                    else
                        bAvailable = m_nCurrentPos != pParent->GetRowCount() - 1;
                }
                break;
            case DbGridControlNavigationBarState::New:
                bAvailable = (pParent->GetOptions() & DbGridControlOptions::Insert) && pParent->GetRowCount() && m_nCurrentPos < pParent->GetRowCount() - 1;
                break;
            case DbGridControlNavigationBarState::Absolute:
                bAvailable = pParent->GetRowCount() > 0;
                break;
            default: break;
        }
        return bAvailable;
    }
}

void DbGridControl::NavigationBar::SetState(DbGridControlNavigationBarState nWhich)
{
    bool bAvailable = GetState(nWhich);
    DbGridControl* pParent = static_cast<DbGridControl*>(GetParent());
    vcl::Window* pWnd = nullptr;
    switch (nWhich)
    {
        case DbGridControlNavigationBarState::First:
            pWnd = m_aFirstBtn.get();
            break;
        case DbGridControlNavigationBarState::Prev:
            pWnd = m_aPrevBtn.get();
            break;
        case DbGridControlNavigationBarState::Next:
            pWnd = m_aNextBtn.get();
            break;
        case DbGridControlNavigationBarState::Last:
            pWnd = m_aLastBtn.get();
            break;
        case DbGridControlNavigationBarState::New:
            pWnd = m_aNewBtn.get();
            break;
        case DbGridControlNavigationBarState::Absolute:
            pWnd = m_aAbsolute.get();
            if (bAvailable)
            {
                if (pParent->m_nTotalCount >= 0)
                {
                    if (pParent->IsCurrentAppending())
                        m_aAbsolute->SetMax(pParent->m_nTotalCount + 1);
                    else
                        m_aAbsolute->SetMax(pParent->m_nTotalCount);
                }
                else
                    m_aAbsolute->SetMax(LONG_MAX);

                m_aAbsolute->SetValue(m_nCurrentPos + 1);
            }
            else
                m_aAbsolute->SetText(OUString());
            break;
        case DbGridControlNavigationBarState::Text:
            pWnd = m_aRecordText.get();
            break;
        case DbGridControlNavigationBarState::Of:
            pWnd = m_aRecordOf.get();
            break;
        case DbGridControlNavigationBarState::Count:
        {
            pWnd = m_aRecordCount.get();
            OUString aText;
            if (bAvailable)
            {
                if (pParent->GetOptions() & DbGridControlOptions::Insert)
                {
                    if (pParent->IsCurrentAppending() && !pParent->IsModified())
                        aText = m_aAbsolute->CreateFieldText(pParent->GetRowCount());
                    else
                        aText = m_aAbsolute->CreateFieldText(pParent->GetRowCount() - 1);
                }
                else
                    aText = m_aAbsolute->CreateFieldText(pParent->GetRowCount());
                if(!pParent->m_bRecordCountFinal)
                    aText += " *";
            }
            else
                aText.clear();

            // add the number of selected rows, if applicable
            if (pParent->GetSelectRowCount())
            {
                OUString aExtendedInfo(aText);
                aExtendedInfo += " (";
                aExtendedInfo += m_aAbsolute->CreateFieldText(pParent->GetSelectRowCount());
                aExtendedInfo += ")";
                pWnd->SetText(aExtendedInfo);
            }
            else
                pWnd->SetText(aText);

            pParent->SetRealRowCount(aText);
        }   break;
        default: break;
    }
    DBG_ASSERT(pWnd, "no window");
    if (pWnd && (pWnd->IsEnabled() != bAvailable))
        // this "pWnd->IsEnabled() != bAvailable" is a little hack : Window::Enable always generates a user
        // event (ImplGenerateMouseMove) even if nothing happened. This may lead to some unwanted effects, so we
        // do this check.
        // For further explanation see Bug 69900.
        pWnd->Enable(bAvailable);
}

void DbGridControl::NavigationBar::Resize()
{
    Control::Resize();
    ArrangeControls();
}

void DbGridControl::NavigationBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Control::Paint(rRenderContext, rRect);
    Point aAbsolutePos = m_aAbsolute->GetPosPixel();
    Size  aAbsoluteSize = m_aAbsolute->GetSizePixel();

    rRenderContext.DrawLine(Point(aAbsolutePos.X() - 1, 0 ),
                            Point(aAbsolutePos.X() - 1, aAbsolutePos.Y() + aAbsoluteSize.Height()));

    rRenderContext.DrawLine(Point(aAbsolutePos.X() + aAbsoluteSize.Width() + 1, 0 ),
                            Point(aAbsolutePos.X() + aAbsoluteSize.Width() + 1, aAbsolutePos.Y() + aAbsoluteSize.Height()));
}

void DbGridControl::NavigationBar::StateChanged(StateChangedType nType)
{
    Control::StateChanged(nType);

    vcl::Window* pWindows[] =
    {
        m_aRecordText.get(),
        m_aAbsolute.get(),
        m_aRecordOf.get(),
        m_aRecordCount.get(),
        m_aFirstBtn.get(),
        m_aPrevBtn.get(),
        m_aNextBtn.get(),
        m_aLastBtn.get(),
        m_aNewBtn.get()
    };

    switch ( nType )
    {
        case StateChangedType::Mirroring:
        {
            bool bIsRTLEnabled = IsRTLEnabled();
            for (vcl::Window* pWindow : pWindows)
                pWindow->EnableRTL( bIsRTLEnabled );
        }
        break;

        case StateChangedType::Zoom:
        {
            Fraction aZoom = GetZoom();

            // not all of these controls need to know the new zoom, but to be sure ...
            vcl::Font aFont(GetSettings().GetStyleSettings().GetFieldFont());
            if (IsControlFont())
                aFont.Merge(GetControlFont());

            for (vcl::Window* pWindow : pWindows)
            {
                pWindow->SetZoom(aZoom);
                pWindow->SetZoomedPointFont(*pWindow, aFont);
            }

            SetZoomedPointFont(*this, aFont);

            // rearrange the controls
            ArrangeControls();
        }
        break;
        default:;
    }
}

DbGridRow::DbGridRow():m_eStatus(GridRowStatus::Clean), m_bIsNew(true)
{}

DbGridRow::DbGridRow(CursorWrapper* pCur, bool bPaintCursor)
          :m_bIsNew(false)
{

    if (pCur && pCur->Is())
    {
        Reference< XIndexAccess >  xColumns(pCur->getColumns(), UNO_QUERY);
        for (sal_Int32 i = 0; i < xColumns->getCount(); ++i)
        {
            Reference< XPropertySet > xColSet(
                xColumns->getByIndex(i), css::uno::UNO_QUERY);
            m_aVariants.emplace_back( new DataColumn(xColSet) );
        }

        if (pCur->rowDeleted())
            m_eStatus = GridRowStatus::Deleted;
        else
        {
            if (bPaintCursor)
                m_eStatus = (pCur->isAfterLast() || pCur->isBeforeFirst()) ? GridRowStatus::Invalid : GridRowStatus::Clean;
            else
            {
                Reference< XPropertySet > xSet = pCur->getPropertySet();
                if (xSet.is())
                {
                    m_bIsNew = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
                    if (!m_bIsNew && (pCur->isAfterLast() || pCur->isBeforeFirst()))
                        m_eStatus = GridRowStatus::Invalid;
                    else if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                        m_eStatus = GridRowStatus::Modified;
                    else
                        m_eStatus = GridRowStatus::Clean;
                }
                else
                    m_eStatus = GridRowStatus::Invalid;
            }
        }
        if (!m_bIsNew && IsValid())
            m_aBookmark = pCur->getBookmark();
        else
            m_aBookmark = Any();
    }
    else
        m_eStatus = GridRowStatus::Invalid;
}

DbGridRow::~DbGridRow()
{
}

void DbGridRow::SetState(CursorWrapper* pCur, bool bPaintCursor)
{
    if (pCur && pCur->Is())
    {
        if (pCur->rowDeleted())
        {
            m_eStatus = GridRowStatus::Deleted;
            m_bIsNew = false;
        }
        else
        {
            m_eStatus = GridRowStatus::Clean;
            if (!bPaintCursor)
            {
                Reference< XPropertySet > xSet = pCur->getPropertySet();
                DBG_ASSERT(xSet.is(), "DbGridRow::SetState : invalid cursor !");

                if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                    m_eStatus = GridRowStatus::Modified;
                m_bIsNew = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
            }
            else
                m_bIsNew = false;
        }

        try
        {
            if (!m_bIsNew && IsValid())
                m_aBookmark = pCur->getBookmark();
            else
                m_aBookmark = Any();
        }
        catch(SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION("svx");
            m_aBookmark = Any();
            m_eStatus = GridRowStatus::Invalid;
            m_bIsNew = false;
        }
    }
    else
    {
        m_aBookmark = Any();
        m_eStatus = GridRowStatus::Invalid;
        m_bIsNew = false;
    }
}

DbGridControl::DbGridControl(
                Reference< XComponentContext > const & _rxContext,
                vcl::Window* pParent,
                WinBits nBits)
            :EditBrowseBox(pParent, EditBrowseBoxFlags::NONE, nBits, DEFAULT_BROWSE_MODE )
            ,m_xContext(_rxContext)
            ,m_aBar(VclPtr<DbGridControl::NavigationBar>::Create(this))
            ,m_nAsynAdjustEvent(nullptr)
            ,m_pDataSourcePropListener(nullptr)
            ,m_pFieldListeners(nullptr)
            ,m_pCursorDisposeListener(nullptr)
            ,m_pGridListener(nullptr)
            ,m_pDataCursor(nullptr)
            ,m_pSeekCursor(nullptr)
            ,m_nSeekPos(-1)
            ,m_nTotalCount(-1)
            ,m_aNullDate(::dbtools::DBTypeConversion::getStandardDate())
            ,m_nMode(DEFAULT_BROWSE_MODE)
            ,m_nCurrentPos(-1)
            ,m_nDeleteEvent(nullptr)
            ,m_nOptions(DbGridControlOptions::Readonly)
            ,m_nOptionMask(DbGridControlOptions::Insert | DbGridControlOptions::Update | DbGridControlOptions::Delete)
            ,m_nLastColId(sal_uInt16(-1))
            ,m_nLastRowId(-1)
            ,m_bDesignMode(false)
            ,m_bRecordCountFinal(false)
            ,m_bNavigationBar(true)
            ,m_bSynchDisplay(true)
            ,m_bHandle(true)
            ,m_bFilterMode(false)
            ,m_bWantDestruction(false)
            ,m_bPendingAdjustRows(false)
            ,m_bHideScrollbars( false )
            ,m_bUpdating(false)
{

    OUString sName(SvxResId(RID_STR_NAVIGATIONBAR));
    m_aBar->SetAccessibleName(sName);
    m_aBar->Show();
    ImplInitWindow( InitWindowFacet::All );
}

void DbGridControl::InsertHandleColumn()
{
    // BrowseBox has problems when painting without a handleColumn (hide it here)
    if (HasHandle())
        BrowseBox::InsertHandleColumn(GetDefaultColumnWidth(OUString()));
    else
        BrowseBox::InsertHandleColumn(0);
}

void DbGridControl::Init()
{
    VclPtr<BrowserHeader> pNewHeader = CreateHeaderBar(this);
    pHeader->SetMouseTransparent(false);

    SetHeaderBar(pNewHeader);
    SetMode(m_nMode);
    SetCursorColor(Color(0xFF, 0, 0));

    InsertHandleColumn();
}

DbGridControl::~DbGridControl()
{
    disposeOnce();
}

void DbGridControl::dispose()
{
    if (!IsDisposed())
    {
        RemoveColumns();

        m_bWantDestruction = true;
        osl::MutexGuard aGuard(m_aDestructionSafety);
        if (m_pFieldListeners)
            DisconnectFromFields();
        m_pCursorDisposeListener.reset();
    }

    if (m_nDeleteEvent)
        Application::RemoveUserEvent(m_nDeleteEvent);

    if (m_pDataSourcePropMultiplexer.is())
    {
        m_pDataSourcePropMultiplexer->dispose();
        m_pDataSourcePropMultiplexer.clear();    // this should delete the multiplexer
        delete m_pDataSourcePropListener;
        m_pDataSourcePropListener = nullptr;
    }
    m_xRowSetListener.clear();

    m_pDataCursor.reset();
    m_pSeekCursor.reset();

    m_aBar.disposeAndClear();

    EditBrowseBox::dispose();
}

void DbGridControl::StateChanged( StateChangedType nType )
{
    EditBrowseBox::StateChanged( nType );

    switch (nType)
    {
        case StateChangedType::Mirroring:
            ImplInitWindow( InitWindowFacet::WritingMode );
            Invalidate();
            break;

        case StateChangedType::Zoom:
        {
            ImplInitWindow( InitWindowFacet::Font );

            // and give it a chance to rearrange
            Point aPoint = GetControlArea().TopLeft();
            sal_uInt16 nX = static_cast<sal_uInt16>(aPoint.X());
            ArrangeControls(nX, static_cast<sal_uInt16>(aPoint.Y()));
            ReserveControlArea(nX);
        }
        break;
        case StateChangedType::ControlFont:
            ImplInitWindow( InitWindowFacet::Font );
            Invalidate();
            break;
        case StateChangedType::ControlForeground:
            ImplInitWindow( InitWindowFacet::Foreground );
            Invalidate();
            break;
        case StateChangedType::ControlBackground:
            ImplInitWindow( InitWindowFacet::Background );
            Invalidate();
            break;
       default:;
    }
}

void DbGridControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    EditBrowseBox::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS ) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitWindow( InitWindowFacet::All );
        Invalidate();
    }
}

void DbGridControl::Select()
{
    EditBrowseBox::Select();

    // as the selected rows may have changed, update the according display in our navigation bar
    m_aBar->InvalidateState(DbGridControlNavigationBarState::Count);

    if (m_pGridListener)
        m_pGridListener->selectionChanged();
}

void DbGridControl::ImplInitWindow( const InitWindowFacet _eInitWhat )
{
    for (auto const & pCol : m_aColumns)
    {
        pCol->ImplInitWindow( GetDataWindow(), _eInitWhat );
    }

    if ( _eInitWhat & InitWindowFacet::WritingMode )
    {
        if ( m_bNavigationBar )
        {
            m_aBar->EnableRTL( IsRTLEnabled() );
        }
    }

    if ( _eInitWhat & InitWindowFacet::Font )
    {
        if ( m_bNavigationBar )
        {
            vcl::Font aFont = m_aBar->GetSettings().GetStyleSettings().GetFieldFont();
            if ( IsControlFont() )
                m_aBar->SetControlFont( GetControlFont() );
            else
                m_aBar->SetControlFont();

            m_aBar->SetZoom( GetZoom() );
        }
    }

    if ( _eInitWhat & InitWindowFacet::Background )
    {
        if (IsControlBackground())
        {
            GetDataWindow().SetBackground(GetControlBackground());
            GetDataWindow().SetControlBackground(GetControlBackground());
            GetDataWindow().SetFillColor(GetControlBackground());
        }
        else
        {
            GetDataWindow().SetControlBackground();
            GetDataWindow().SetFillColor(GetFillColor());
        }
    }
}

void DbGridControl::RemoveRows(bool bNewCursor)
{
    // Did the data cursor change?
    if (!bNewCursor)
    {
        m_pSeekCursor.reset();
        m_xPaintRow = m_xDataRow = m_xEmptyRow  = m_xCurrentRow = m_xSeekRow = nullptr;
        m_nCurrentPos = m_nSeekPos = -1;
        m_nOptions  = DbGridControlOptions::Readonly;

        RowRemoved(0, GetRowCount(), false);
        m_nTotalCount = -1;
    }
    else
    {
        RemoveRows();
    }
}

void DbGridControl::RemoveRows()
{
    // we're going to remove all columns and all row, so deactivate the current cell
    if (IsEditing())
        DeactivateCell();

    // de-initialize all columns
    // if there are columns, free all controllers
    for (auto const & pColumn : m_aColumns)
        pColumn->Clear();

    m_pSeekCursor.reset();
    m_pDataCursor.reset();

    m_xPaintRow = m_xDataRow = m_xEmptyRow  = m_xCurrentRow = m_xSeekRow = nullptr;
    m_nCurrentPos = m_nSeekPos = m_nTotalCount  = -1;
    m_nOptions  = DbGridControlOptions::Readonly;

    // reset number of sentences to zero in the browser
    EditBrowseBox::RemoveRows();
    m_aBar->InvalidateAll(m_nCurrentPos, true);
}

void DbGridControl::ArrangeControls(sal_uInt16& nX, sal_uInt16 nY)
{
    // positioning of the controls
    if (m_bNavigationBar)
    {
        tools::Rectangle aRect(GetControlArea());
        m_aBar->SetPosSizePixel(Point(0, nY + 1), Size(aRect.GetSize().Width(), aRect.GetSize().Height() - 1));
        nX = m_aBar->ArrangeControls();
    }
}

void DbGridControl::EnableHandle(bool bEnable)
{
    if (m_bHandle == bEnable)
        return;

    // HandleColumn is only hidden because there are a lot of problems while painting otherwise
    RemoveColumn( HandleColumnId );
    m_bHandle = bEnable;
    InsertHandleColumn();
}

namespace
{
    bool adjustModeForScrollbars( BrowserMode& _rMode, bool _bNavigationBar, bool _bHideScrollbars )
    {
        BrowserMode nOldMode = _rMode;

        if ( !_bNavigationBar )
        {
            _rMode &= ~BrowserMode::AUTO_HSCROLL;
        }

        if ( _bHideScrollbars )
        {
            _rMode |= ( BrowserMode::NO_HSCROLL | BrowserMode::NO_VSCROLL );
            _rMode &= ~BrowserMode( BrowserMode::AUTO_HSCROLL | BrowserMode::AUTO_VSCROLL );
        }
        else
        {
            _rMode |= ( BrowserMode::AUTO_HSCROLL | BrowserMode::AUTO_VSCROLL );
            _rMode &= ~BrowserMode( BrowserMode::NO_HSCROLL | BrowserMode::NO_VSCROLL );
        }

        // note: if we have a navigation bar, we always have a AUTO_HSCROLL. In particular,
        // _bHideScrollbars is ignored then
        if ( _bNavigationBar )
        {
            _rMode |= BrowserMode::AUTO_HSCROLL;
            _rMode &= ~BrowserMode::NO_HSCROLL;
        }

        return nOldMode != _rMode;
    }
}

void DbGridControl::EnableNavigationBar(bool bEnable)
{
    if (m_bNavigationBar == bEnable)
        return;

    m_bNavigationBar = bEnable;

    if (bEnable)
    {
        m_aBar->Show();
        m_aBar->Enable();
        m_aBar->InvalidateAll(m_nCurrentPos, true);

        if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
            SetMode( m_nMode );

        // get size of the reserved ControlArea
        Point aPoint = GetControlArea().TopLeft();
        sal_uInt16 nX = static_cast<sal_uInt16>(aPoint.X());

        ArrangeControls(nX, static_cast<sal_uInt16>(aPoint.Y()));
        ReserveControlArea(nX);
    }
    else
    {
        m_aBar->Hide();
        m_aBar->Disable();

        if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
            SetMode( m_nMode );

        ReserveControlArea();
    }
}

DbGridControlOptions DbGridControl::SetOptions(DbGridControlOptions nOpt)
{
    DBG_ASSERT(!m_xCurrentRow.is() || !m_xCurrentRow->IsModified(),
        "DbGridControl::SetOptions : please do not call when editing a record (things are much easier this way ;) !");

    // for the next setDataSource (which is triggered by a refresh, for instance)
    m_nOptionMask = nOpt;

    // normalize the new options
    Reference< XPropertySet > xDataSourceSet = m_pDataCursor->getPropertySet();
    if (xDataSourceSet.is())
    {
        // check what kind of options are available
        sal_Int32 nPrivileges = 0;
        xDataSourceSet->getPropertyValue(FM_PROP_PRIVILEGES) >>= nPrivileges;
        if ((nPrivileges & Privilege::INSERT) == 0)
            nOpt &= ~DbGridControlOptions::Insert;
        if ((nPrivileges & Privilege::UPDATE) == 0)
            nOpt &= ~DbGridControlOptions::Update;
        if ((nPrivileges & Privilege::DELETE) == 0)
            nOpt &= ~DbGridControlOptions::Delete;
    }
    else
        nOpt = DbGridControlOptions::Readonly;

    // need to do something after that ?
    if (nOpt == m_nOptions)
        return m_nOptions;

    // the 'update' option only affects our BrowserMode (with or w/o focus rect)
    BrowserMode nNewMode = m_nMode;
    if (!(m_nMode & BrowserMode::CURSOR_WO_FOCUS))
    {
        if (nOpt & DbGridControlOptions::Update)
            nNewMode |= BrowserMode::HIDECURSOR;
        else
            nNewMode &= ~BrowserMode::HIDECURSOR;
    }
    else
        nNewMode &= ~BrowserMode::HIDECURSOR;
        // should not be necessary if EnablePermanentCursor is used to change the cursor behaviour, but to be sure ...

    if (nNewMode != m_nMode)
    {
        SetMode(nNewMode);
        m_nMode = nNewMode;
    }

    // _after_ setting the mode because this results in an ActivateCell
    DeactivateCell();

    bool bInsertChanged = (nOpt & DbGridControlOptions::Insert) != (m_nOptions & DbGridControlOptions::Insert);
    m_nOptions = nOpt;
        // we need to set this before the code below because it indirectly uses m_nOptions

    // the 'insert' option affects our empty row
    if (bInsertChanged)
    {
        if (m_nOptions & DbGridControlOptions::Insert)
        {   // the insert option is to be set
            m_xEmptyRow = new DbGridRow();
            RowInserted(GetRowCount());
        }
        else
        {   // the insert option is to be reset
            m_xEmptyRow = nullptr;
            if ((GetCurRow() == GetRowCount() - 1) && (GetCurRow() > 0))
                GoToRowColumnId(GetCurRow() - 1, GetCurColumnId());
            RowRemoved(GetRowCount());
        }
    }

    // the 'delete' options has no immediate consequences

    ActivateCell();
    Invalidate();
    return m_nOptions;
}

void DbGridControl::ForceHideScrollbars()
{
    if ( m_bHideScrollbars )
        return;

    m_bHideScrollbars = true;

    if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
        SetMode( m_nMode );
}

void DbGridControl::EnablePermanentCursor(bool bEnable)
{
    if (IsPermanentCursorEnabled() == bEnable)
        return;

    if (bEnable)
    {
        m_nMode &= ~BrowserMode::HIDECURSOR;     // without this BrowserMode::CURSOR_WO_FOCUS won't have any affect
        m_nMode |= BrowserMode::CURSOR_WO_FOCUS;
    }
    else
    {
        if (m_nOptions & DbGridControlOptions::Update)
            m_nMode |= BrowserMode::HIDECURSOR;      // no cursor at all
        else
            m_nMode &= ~BrowserMode::HIDECURSOR;     // at least the "non-permanent" cursor

        m_nMode &= ~BrowserMode::CURSOR_WO_FOCUS;
    }
    SetMode(m_nMode);

    bool bWasEditing = IsEditing();
    DeactivateCell();
    if (bWasEditing)
        ActivateCell();
}

bool DbGridControl::IsPermanentCursorEnabled() const
{
    return (m_nMode & BrowserMode::CURSOR_WO_FOCUS) && !(m_nMode & BrowserMode::HIDECURSOR);
}

void DbGridControl::refreshController(sal_uInt16 _nColId, GrantControlAccess /*_aAccess*/)
{
    if ((GetCurColumnId() == _nColId) && IsEditing())
    {   // the controller which is currently active needs to be refreshed
        DeactivateCell();
        ActivateCell();
    }
}

void DbGridControl::setDataSource(const Reference< XRowSet >& _xCursor, DbGridControlOptions nOpts)
{
    if (!_xCursor.is() && !m_pDataCursor)
        return;

    if (m_pDataSourcePropMultiplexer.is())
    {
        m_pDataSourcePropMultiplexer->dispose();
        m_pDataSourcePropMultiplexer.clear();    // this should delete the multiplexer
        delete m_pDataSourcePropListener;
        m_pDataSourcePropListener = nullptr;
    }
    m_xRowSetListener.clear();

    // is the new cursor valid ?
    // the cursor is only valid if it contains some columns
    // if there is no cursor or the cursor is not valid we have to clean up an leave
    if (!_xCursor.is() || !Reference< XColumnsSupplier > (_xCursor, UNO_QUERY)->getColumns()->hasElements())
    {
        RemoveRows();
        return;
    }

    // did the data cursor change?
    sal_uInt16 nCurPos = GetColumnPos(GetCurColumnId());

    SetUpdateMode(false);
    RemoveRows();
    DisconnectFromFields();

    m_pCursorDisposeListener.reset();

    {
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        if (m_nAsynAdjustEvent)
        {
            // the adjust was thought to work with the old cursor which we don't have anymore
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = nullptr;
        }
    }

    // get a new formatter and data cursor
    m_xFormatter = nullptr;
    Reference< css::util::XNumberFormatsSupplier >  xSupplier = getNumberFormats(getConnection(_xCursor), true);
    if (xSupplier.is())
    {
        m_xFormatter.set( css::util::NumberFormatter::create(m_xContext),
                          UNO_QUERY);
        m_xFormatter->attachNumberFormatsSupplier(xSupplier);

        // retrieve the datebase of the Numberformatter
        try
        {
            xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= m_aNullDate;
        }
        catch(Exception&)
        {
        }
    }

    m_pDataCursor.reset(new CursorWrapper(_xCursor));

    // now create a cursor for painting rows
    // we need that cursor only if we are not in insert only mode
    Reference< XResultSet > xClone;
    Reference< XResultSetAccess > xAccess( _xCursor, UNO_QUERY );
    try
    {
        xClone = xAccess.is() ? xAccess->createResultSet() : Reference< XResultSet > ();
    }
    catch(Exception&)
    {
    }
    if (xClone.is())
        m_pSeekCursor.reset(new CursorWrapper(xClone));

    // property listening on the data source
    // (Normally one class would be sufficient : the multiplexer which could forward the property change to us.
    // But for that we would have been derived from ::comphelper::OPropertyChangeListener, which isn't exported.
    // So we introduce a second class, which is a ::comphelper::OPropertyChangeListener (in the implementation file we know this class)
    // and forwards the property changes to a our special method "DataSourcePropertyChanged".)
    if (m_pDataCursor)
    {
        m_pDataSourcePropListener = new FmXGridSourcePropListener(this);
        m_pDataSourcePropMultiplexer = new ::comphelper::OPropertyChangeMultiplexer(m_pDataSourcePropListener, m_pDataCursor->getPropertySet() );
        m_pDataSourcePropMultiplexer->addProperty(FM_PROP_ISMODIFIED);
        m_pDataSourcePropMultiplexer->addProperty(FM_PROP_ISNEW);
    }

    BrowserMode nOldMode = m_nMode;
    if (m_pSeekCursor)
    {
        try
        {
            Reference< XPropertySet >  xSet(_xCursor, UNO_QUERY);
            if (xSet.is())
            {
                // check what kind of options are available
                sal_Int32 nConcurrency = ResultSetConcurrency::READ_ONLY;
                xSet->getPropertyValue(FM_PROP_RESULTSET_CONCURRENCY) >>= nConcurrency;

                if ( ResultSetConcurrency::UPDATABLE == nConcurrency )
                {
                    sal_Int32 nPrivileges = 0;
                    xSet->getPropertyValue(FM_PROP_PRIVILEGES) >>= nPrivileges;

                    // Insert Option should be set if insert only otherwise you won't see any rows
                    // and no insertion is possible
                    if ((m_nOptionMask & DbGridControlOptions::Insert)
                        && ((nPrivileges & Privilege::INSERT) == Privilege::INSERT) && (nOpts & DbGridControlOptions::Insert))
                        m_nOptions |= DbGridControlOptions::Insert;
                    if ((m_nOptionMask & DbGridControlOptions::Update)
                        && ((nPrivileges & Privilege::UPDATE) == Privilege::UPDATE) && (nOpts & DbGridControlOptions::Update))
                        m_nOptions |= DbGridControlOptions::Update;
                    if ((m_nOptionMask & DbGridControlOptions::Delete)
                        && ((nPrivileges & Privilege::DELETE) == Privilege::DELETE) && (nOpts & DbGridControlOptions::Delete))
                        m_nOptions |= DbGridControlOptions::Delete;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

        bool bPermanentCursor = IsPermanentCursorEnabled();
        m_nMode = DEFAULT_BROWSE_MODE;

        if ( bPermanentCursor )
        {
            m_nMode |= BrowserMode::CURSOR_WO_FOCUS;
            m_nMode &= ~BrowserMode::HIDECURSOR;
        }
        else
        {
            // updates are allowed -> no focus rectangle
            if ( m_nOptions & DbGridControlOptions::Update )
                m_nMode |= BrowserMode::HIDECURSOR;
        }

        m_nMode |= BrowserMode::MULTISELECTION;

        adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars );

        Reference< XColumnsSupplier >  xSupplyColumns(_xCursor, UNO_QUERY);
        if (xSupplyColumns.is())
            InitColumnsByFields(Reference< XIndexAccess > (xSupplyColumns->getColumns(), UNO_QUERY));

        ConnectToFields();
    }

    sal_uInt32 nRecordCount(0);

    if (m_pSeekCursor)
    {
        Reference< XPropertySet > xSet = m_pDataCursor->getPropertySet();
        xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
        m_bRecordCountFinal = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ROWCOUNTFINAL));

        m_xRowSetListener = new RowSetEventListener(this);
        Reference< XRowsChangeBroadcaster> xChangeBroad(xSet,UNO_QUERY);
        if ( xChangeBroad.is( ) )
            xChangeBroad->addRowsChangeListener(m_xRowSetListener);


        // insert the currently known rows
        // and one row if we are able to insert rows
        if (m_nOptions & DbGridControlOptions::Insert)
        {
            // insert the empty row for insertion
            m_xEmptyRow = new DbGridRow();
            ++nRecordCount;
        }
        if (nRecordCount)
        {
            m_xPaintRow = m_xSeekRow = new DbGridRow(m_pSeekCursor.get(), true);
            m_xDataRow  = new DbGridRow(m_pDataCursor.get(), false);
            RowInserted(0, nRecordCount, false);

            if (m_xSeekRow->IsValid())
                try
                {
                    m_nSeekPos = m_pSeekCursor->getRow() - 1;
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("svx");
                    m_nSeekPos = -1;
                }
        }
        else
        {
            // no rows so we don't need a seekcursor
            m_pSeekCursor.reset();
        }
    }

    // go to the old column
    if (nCurPos == BROWSER_INVALIDID || nCurPos >= ColCount())
        nCurPos = 0;

    // Column zero is a valid choice and guaranteed to exist,
    // but invisible to the user; if we have at least one
    // user-visible column, go to that one.
    if (nCurPos == 0 && ColCount() > 1)
        nCurPos = 1;

    // there are rows so go to the selected current column
    if (nRecordCount)
        GoToRowColumnId(0, GetColumnId(nCurPos));
    // else stop the editing if necessary
    else if (IsEditing())
        DeactivateCell();

    // now reset the mode
    if (m_nMode != nOldMode)
        SetMode(m_nMode);

    // RecalcRows was already called while resizing
    if (!IsResizing() && GetRowCount())
        RecalcRows(GetTopRow(), GetVisibleRows(), true);

    m_aBar->InvalidateAll(m_nCurrentPos, true);
    SetUpdateMode(true);

    // start listening on the seek cursor
    if (m_pSeekCursor)
        m_pCursorDisposeListener.reset(new DisposeListenerGridBridge(*this, Reference< XComponent > (Reference< XInterface >(*m_pSeekCursor), UNO_QUERY)));
}

void DbGridControl::RemoveColumns()
{
    if ( IsEditing() )
        DeactivateCell();

    m_aColumns.clear();

    EditBrowseBox::RemoveColumns();
}

std::unique_ptr<DbGridColumn> DbGridControl::CreateColumn(sal_uInt16 nId) const
{
    return std::unique_ptr<DbGridColumn>(new DbGridColumn(nId, *const_cast<DbGridControl*>(this)));
}

sal_uInt16 DbGridControl::AppendColumn(const OUString& rName, sal_uInt16 nWidth, sal_uInt16 nModelPos, sal_uInt16 nId)
{
    DBG_ASSERT(nId == BROWSER_INVALIDID, "DbGridControl::AppendColumn : I want to set the ID myself ...");
    sal_uInt16 nRealPos = nModelPos;
    if (nModelPos != HEADERBAR_APPEND)
    {
        // calc the view pos. we can't use our converting functions because the new column
        // has no VCL-representation, yet.
        sal_Int16 nViewPos = nModelPos;
        while (nModelPos--)
        {
            if ( m_aColumns[ nModelPos ]->IsHidden() )
                --nViewPos;
        }
        // restore nModelPos, we need it later
        nModelPos = nRealPos;
        // the position the base class gets is the view pos + 1 (because of the handle column)
        nRealPos = nViewPos + 1;
    }

    // calculate the new id
    for (nId=1; (GetModelColumnPos(nId) != GRID_COLUMN_NOT_FOUND) && size_t(nId) <= m_aColumns.size(); ++nId)
        ;
    DBG_ASSERT(GetViewColumnPos(nId) == GRID_COLUMN_NOT_FOUND, "DbGridControl::AppendColumn : inconsistent internal state !");
        // my column's models say "there is no column with id nId", but the view (the base class) says "there is a column ..."

    EditBrowseBox::AppendColumn(rName, nWidth, nRealPos, nId);
    if (nModelPos == HEADERBAR_APPEND)
        m_aColumns.push_back( CreateColumn(nId) );
    else
        m_aColumns.insert( m_aColumns.begin() + nModelPos, CreateColumn(nId) );

    return nId;
}

void DbGridControl::RemoveColumn(sal_uInt16 nId)
{
    EditBrowseBox::RemoveColumn(nId);

    const sal_uInt16 nIndex = GetModelColumnPos(nId);
    if(nIndex != GRID_COLUMN_NOT_FOUND)
    {
        m_aColumns.erase( m_aColumns.begin()+nIndex );
    }
}

void DbGridControl::ColumnMoved(sal_uInt16 nId)
{
    EditBrowseBox::ColumnMoved(nId);

    // remove the col from the model
    sal_uInt16 nOldModelPos = GetModelColumnPos(nId);
#ifdef DBG_UTIL
    DbGridColumn* pCol = m_aColumns[ nOldModelPos ].get();
    DBG_ASSERT(!pCol->IsHidden(), "DbGridControl::ColumnMoved : moved a hidden col ? how this ?");
#endif

    // for the new model pos we can't use GetModelColumnPos because we are altering the model at the moment
    // so the method won't work (in fact it would return the old model pos)

    // the new view pos is calculated easily
    sal_uInt16 nNewViewPos = GetViewColumnPos(nId);

    // from that we can compute the new model pos
    size_t nNewModelPos;
    for (nNewModelPos = 0; nNewModelPos < m_aColumns.size(); ++nNewModelPos)
    {
        if (!m_aColumns[ nNewModelPos ]->IsHidden())
        {
            if (!nNewViewPos)
                break;
            else
                --nNewViewPos;
        }
    }
    DBG_ASSERT( nNewModelPos < m_aColumns.size(), "DbGridControl::ColumnMoved : could not find the new model position !");

    // this will work. of course the model isn't fully consistent with our view right now, but let's
    // look at the situation : a column has been moved with in the VIEW from pos m to n, say m<n (in the
    // other case we can use analogue arguments).
    // All cols k with m<k<=n have been shifted left on pos, the former col m now has pos n.
    // In the model this affects a range of cols x to y, where x<=m and y<=n. And the number of hidden cols
    // within this range is constant, so we may calculate the view pos from the model pos in the above way.

    // for instance, let's look at a grid with six columns where the third one is hidden. this will
    // initially look like this :

    //              +---+---+---+---+---+---+
    // model pos    | 0 | 1 |*2*| 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 2 | 3 | 4 | 5 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 1 | - | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+

    // if we move the column at (view) pos 1 to (view) pos 3 we have :

    //              +---+---+---+---+---+---+
    // model pos    | 0 | 3 |*2*| 4 | 1 | 5 |   // not reflecting the changes, yet
    //              +---+---+---+---+---+---+
    // ID           | 1 | 4 | 3 | 5 | 2 | 6 |   // already reflecting the changes
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 1 | - | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+

    // or, sorted by the out-of-date model positions :

    //              +---+---+---+---+---+---+
    // model pos    | 0 | 1 |*2*| 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 2 | 3 | 4 | 5 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 3 | - | 1 | 2 | 4 |
    //              +---+---+---+---+---+---+

    // We know the new view pos (3) of the moved column because our base class tells us. So we look at our
    // model for the 4th (the pos is zero-based) visible column, it is at (model) position 4. And this is
    // exactly the pos where we have to re-insert our column's model, so it looks ike this :

    //              +---+---+---+---+---+---+
    // model pos    | 0 |*1*| 2 | 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 3 | 4 | 5 | 2 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | - | 1 | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+

    // Now, all is consistent again.
    // (except of the hidden column : The cycling of the cols occurred on the model, not on the view. maybe
    // the user expected the latter but there really is no good argument against our method ;) ...)

    // And no, this large explanation isn't just because I wanted to play a board game or something like
    // that. It's because it took me a while to see it myself, and the whole theme (hidden cols, model col
    // positions, view col positions)  is really painful (at least for me) so the above pictures helped me a lot ;)

    auto temp = std::move(m_aColumns[ nOldModelPos ]);
    m_aColumns.erase( m_aColumns.begin() + nOldModelPos );
    m_aColumns.insert( m_aColumns.begin() + nNewModelPos, std::move(temp) );
}

bool DbGridControl::SeekRow(long nRow)
{
    // in filter mode or in insert only mode we don't have any cursor!
    if ( !SeekCursor( nRow ) )
        return false;

    if ( IsFilterMode() )
    {
        DBG_ASSERT( IsFilterRow( nRow ), "DbGridControl::SeekRow(): No filter row, wrong mode" );
        m_xPaintRow = m_xEmptyRow;
    }
    else
    {
        // on the current position we have to take the current row for display as we want
        // to have the most recent values for display
        if ( ( nRow == m_nCurrentPos ) && getDisplaySynchron() )
            m_xPaintRow = m_xCurrentRow;
        // seek to the empty insert row
        else if ( IsInsertionRow( nRow ) )
            m_xPaintRow = m_xEmptyRow;
        else
        {
            m_xSeekRow->SetState( m_pSeekCursor.get(), true );
            m_xPaintRow = m_xSeekRow;
        }
    }

    EditBrowseBox::SeekRow(nRow);

    return m_nSeekPos >= 0;
}

// Is called whenever the visible amount of data changes
void DbGridControl::VisibleRowsChanged( long nNewTopRow, sal_uInt16 nLinesOnScreen )
{
    RecalcRows(nNewTopRow, nLinesOnScreen, false);
}

void DbGridControl::RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, bool bUpdateCursor)
{
    // If no cursor -> no rows in the browser.
    if (!m_pSeekCursor)
    {
        DBG_ASSERT(GetRowCount() == 0,"DbGridControl: without cursor no rows are allowed to be there");
        return;
    }

    // ignore any implicitly made updates
    bool bDisablePaint = !bUpdateCursor && IsPaintEnabled();
    if (bDisablePaint)
        EnablePaint(false);

    // adjust cache to the visible area
    Reference< XPropertySet > xSet = m_pSeekCursor->getPropertySet();
    sal_Int32 nCacheSize = 0;
    xSet->getPropertyValue(FM_PROP_FETCHSIZE) >>= nCacheSize;
    bool bCacheAligned   = false;
    // no further cursor movements after initializing (m_nSeekPos < 0) because it is already
    // positioned on the first sentence
    long nDelta = nNewTopRow - GetTopRow();
    // limit for relative positioning
    long nLimit = nCacheSize ? nCacheSize / 2 : 0;

    // more lines on screen than in cache
    if (nLimit < nLinesOnScreen)
    {
        Any aCacheSize;
        aCacheSize <<= sal_Int32(nLinesOnScreen*2);
        xSet->setPropertyValue(FM_PROP_FETCHSIZE, aCacheSize);
        // here we need to update the cursor for sure
        bUpdateCursor = true;
        bCacheAligned = true;
        nLimit = nLinesOnScreen;
    }

    // In the following, all positionings are done as it is
    // ensured that there are enough lines in the data cache

    // window goes downwards with less than two windows difference or
    // the cache was updated and no rowcount yet
    if (nDelta < nLimit && (nDelta > 0
        || (bCacheAligned && m_nTotalCount < 0)) )
        SeekCursor(nNewTopRow + nLinesOnScreen - 1);
    else if (nDelta < 0 && std::abs(nDelta) < nLimit)
        SeekCursor(nNewTopRow);
    else if (nDelta != 0 || bUpdateCursor)
        SeekCursor(nNewTopRow, true);

    AdjustRows();

    // ignore any updates implicit made
    EnablePaint(true);
}

void DbGridControl::RowInserted(long nRow, long nNumRows, bool bDoPaint)
{
    if (nNumRows)
    {
        if (m_bRecordCountFinal && m_nTotalCount < 0)
        {
            // if we have an insert row we have to reduce to count by 1
            // as the total count reflects only the existing rows in database
            m_nTotalCount = GetRowCount() + nNumRows;
            if (m_xEmptyRow.is())
                --m_nTotalCount;
        }
        else if (m_nTotalCount >= 0)
            m_nTotalCount += nNumRows;

        EditBrowseBox::RowInserted(nRow, nNumRows, bDoPaint);
        m_aBar->InvalidateState(DbGridControlNavigationBarState::Count);
    }
}

void DbGridControl::RowRemoved(long nRow, long nNumRows, bool bDoPaint)
{
    if (nNumRows)
    {
        if (m_bRecordCountFinal && m_nTotalCount < 0)
        {
            m_nTotalCount = GetRowCount() - nNumRows;
            // if we have an insert row reduce by 1
            if (m_xEmptyRow.is())
                --m_nTotalCount;
        }
        else if (m_nTotalCount >= 0)
            m_nTotalCount -= nNumRows;

        EditBrowseBox::RowRemoved(nRow, nNumRows, bDoPaint);
        m_aBar->InvalidateState(DbGridControlNavigationBarState::Count);
    }
}

void DbGridControl::AdjustRows()
{
    if (!m_pSeekCursor)
        return;

    Reference< XPropertySet > xSet = m_pDataCursor->getPropertySet();

    // refresh RecordCount
    sal_Int32 nRecordCount = 0;
    xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
    if (!m_bRecordCountFinal)
        m_bRecordCountFinal = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ROWCOUNTFINAL));

    // Did the number of rows change?
    // Here we need to consider that there might be an additional row for adding new data sets

    // add additional AppendRow for insertion
    if (m_nOptions & DbGridControlOptions::Insert)
        ++nRecordCount;

    // If there is currently an insertion, so do not consider this added row in RecordCount or Appendrow
    if (!IsUpdating() && m_bRecordCountFinal && IsModified() && m_xCurrentRow != m_xEmptyRow &&
        m_xCurrentRow->IsNew())
        ++nRecordCount;
    // ensured with !m_bUpdating: otherwise the edited data set (that SaveRow added and why this
    // method was called) would be called twice (if m_bUpdating == sal_True): once in RecordCount
    // and a second time here (60787 - FS)

    if (nRecordCount != GetRowCount())
    {
        long nDelta = GetRowCount() - static_cast<long>(nRecordCount);
        if (nDelta > 0) // too many
        {
            RowRemoved(GetRowCount() - nDelta, nDelta, false);
            // some rows are gone, thus, repaint starting at the current position
            Invalidate();

            sal_Int32 nNewPos = AlignSeekCursor();
            if (m_bSynchDisplay)
                EditBrowseBox::GoToRow(nNewPos);

            SetCurrent(nNewPos);
            // there are rows so go to the selected current column
            if (nRecordCount)
                GoToRowColumnId(nNewPos, GetColumnId(GetCurColumnId()));
            if (!IsResizing() && GetRowCount())
                RecalcRows(GetTopRow(), GetVisibleRows(), true);
            m_aBar->InvalidateAll(m_nCurrentPos, true);
        }
        else  // too few
            RowInserted(GetRowCount(), -nDelta);
    }

    if (m_bRecordCountFinal && m_nTotalCount < 0)
    {
        if (m_nOptions & DbGridControlOptions::Insert)
            m_nTotalCount = GetRowCount() - 1;
        else
            m_nTotalCount = GetRowCount();
    }
    m_aBar->InvalidateState(DbGridControlNavigationBarState::Count);
}

svt::EditBrowseBox::RowStatus DbGridControl::GetRowStatus(long nRow) const
{
    if (IsFilterRow(nRow))
        return EditBrowseBox::FILTER;
    else if (m_nCurrentPos >= 0 && nRow == m_nCurrentPos)
    {
        // new row
        if (!IsValid(m_xCurrentRow))
            return EditBrowseBox::DELETED;
        else if (IsModified())
            return EditBrowseBox::MODIFIED;
        else if (m_xCurrentRow->IsNew())
            return EditBrowseBox::CURRENTNEW;
        else
            return EditBrowseBox::CURRENT;
    }
    else if (IsInsertionRow(nRow))
        return EditBrowseBox::NEW;
    else if (!IsValid(m_xSeekRow))
        return EditBrowseBox::DELETED;
    else
        return EditBrowseBox::CLEAN;
}

void DbGridControl::PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId) const
{
    if (!IsValid(m_xPaintRow))
        return;

    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = (Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    if (pColumn)
    {
        tools::Rectangle aArea(rRect);
        if ((GetMode() & BrowserMode::CURSOR_WO_FOCUS) == BrowserMode::CURSOR_WO_FOCUS)
        {
            aArea.AdjustTop(1 );
            aArea.AdjustBottom( -1 );
        }
        pColumn->Paint(rDev, aArea, m_xPaintRow.get(), getNumberFormatter());
    }
}

bool DbGridControl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{

    DeactivateCell( false );

    if  (   m_pDataCursor
        &&  ( m_nCurrentPos != nNewRow )
        && !SetCurrent( nNewRow )
        )
    {
        ActivateCell();
        return false;
    }

    return EditBrowseBox::CursorMoving( nNewRow, nNewCol );
}

bool DbGridControl::SetCurrent(long nNewRow)
{
    // Each movement of the datacursor must start with BeginCursorAction and end with
    // EndCursorAction to block all notifications during the movement
    BeginCursorAction();

    try
    {
        // compare positions
        if (SeekCursor(nNewRow))
        {
            if (IsFilterRow(nNewRow))   // special mode for filtering
            {
                m_xCurrentRow = m_xDataRow = m_xPaintRow = m_xEmptyRow;
                m_nCurrentPos = nNewRow;
            }
            else
            {
                bool bNewRowInserted = false;
                // Should we go to the insertrow ?
                if (IsInsertionRow(nNewRow))
                {
                    // to we need to move the cursor to the insert row?
                    // we need to insert the if the current row isn't the insert row or if the
                    // cursor triggered the move by itself and we need a reinitialization of the row
                    Reference< XPropertySet > xCursorProps = m_pDataCursor->getPropertySet();
                    if ( !::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_ISNEW)) )
                    {
                        Reference< XResultSetUpdate > xUpdateCursor(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);
                        xUpdateCursor->moveToInsertRow();
                    }
                    bNewRowInserted = true;
                }
                else
                {

                    if ( !m_pSeekCursor->isBeforeFirst() && !m_pSeekCursor->isAfterLast() )
                    {
                        Any aBookmark = m_pSeekCursor->getBookmark();
                        if (!m_xCurrentRow.is() || m_xCurrentRow->IsNew() || !CompareBookmark(aBookmark, m_pDataCursor->getBookmark()))
                        {
                            // adjust the cursor to the new desired row
                            if (!m_pDataCursor->moveToBookmark(aBookmark))
                            {
                                EndCursorAction();
                                return false;
                            }
                        }
                    }
                }
                m_xDataRow->SetState(m_pDataCursor.get(), false);
                m_xCurrentRow = m_xDataRow;

                long nPaintPos = -1;
                // do we have to repaint the last regular row in case of setting defaults or autovalues
                if (m_nCurrentPos >= 0 && m_nCurrentPos >= (GetRowCount() - 2))
                    nPaintPos = m_nCurrentPos;

                m_nCurrentPos = nNewRow;

                // repaint the new row to display all defaults
                if (bNewRowInserted)
                    RowModified(m_nCurrentPos);
                if (nPaintPos >= 0)
                    RowModified(nPaintPos);
            }
        }
        else
        {
            OSL_FAIL("DbGridControl::SetCurrent : SeekRow failed !");
            EndCursorAction();
            return false;
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        EndCursorAction();
        return false;
    }

    EndCursorAction();
    return true;
}

void DbGridControl::CursorMoved()
{

    // cursor movement due to deletion or insertion of rows
    if (m_pDataCursor && m_nCurrentPos != GetCurRow())
    {
        DeactivateCell();
        SetCurrent(GetCurRow());
    }

    EditBrowseBox::CursorMoved();
    m_aBar->InvalidateAll(m_nCurrentPos);

    // select the new column when they moved
    if ( IsDesignMode() && GetSelectedColumnCount() > 0 && GetCurColumnId() )
    {
        SelectColumnId( GetCurColumnId() );
    }

    if ( m_nLastColId != GetCurColumnId() )
        onColumnChange();
    m_nLastColId = GetCurColumnId();

    if ( m_nLastRowId != GetCurRow() )
        onRowChange();
    m_nLastRowId = GetCurRow();
}

void DbGridControl::onRowChange()
{
    // not interested in
}

void DbGridControl::onColumnChange()
{
    if ( m_pGridListener )
        m_pGridListener->columnChanged();
}

void DbGridControl::setDisplaySynchron(bool bSync)
{
    if (bSync != m_bSynchDisplay)
    {
        m_bSynchDisplay = bSync;
        if (m_bSynchDisplay)
            AdjustDataSource();
    }
}

void DbGridControl::AdjustDataSource(bool bFull)
{
    SAL_INFO("svx.fmcomp", "DbGridControl::AdjustDataSource");
    SolarMutexGuard aGuard;
    // If the current row is recalculated at the moment, do not adjust

    if (bFull)
        m_xCurrentRow = nullptr;
    // if we are on the same row only repaint
    // but this is only possible for rows which are not inserted, in that case the comparison result
    // may not be correct
    else
        if  (   m_xCurrentRow.is()
            &&  !m_xCurrentRow->IsNew()
            &&  !m_pDataCursor->isBeforeFirst()
            &&  !m_pDataCursor->isAfterLast()
            &&  !m_pDataCursor->rowDeleted()
            )
        {
            bool bEqualBookmarks = CompareBookmark( m_xCurrentRow->GetBookmark(), m_pDataCursor->getBookmark() );

            bool bDataCursorIsOnNew = false;
            m_pDataCursor->getPropertySet()->getPropertyValue( FM_PROP_ISNEW ) >>= bDataCursorIsOnNew;

            if ( bEqualBookmarks && !bDataCursorIsOnNew )
            {
                // position of my data cursor is the same as the position our current row points tpo
                // sync the status, repaint, done
                DBG_ASSERT(m_xDataRow == m_xCurrentRow, "Errors in the data row");
                SAL_INFO("svx.fmcomp", "same position, new state: " << ROWSTATUS(m_xCurrentRow));
                RowModified(m_nCurrentPos);
                return;
            }
        }

    // away from the data cursor's row
    if (m_xPaintRow == m_xCurrentRow)
        m_xPaintRow = m_xSeekRow;

    // not up-to-date row, thus, adjust completely
    if (!m_xCurrentRow.is())
        AdjustRows();

    sal_Int32 nNewPos = AlignSeekCursor();
    if (nNewPos < 0)// could not find any position
        return;

    if (nNewPos != m_nCurrentPos)
    {
        if (m_bSynchDisplay)
            EditBrowseBox::GoToRow(nNewPos);

        if (!m_xCurrentRow.is())
            // Happens e.g. when deleting the n last datasets (n>1) while the cursor was positioned
            // on the last one. In this case, AdjustRows deletes two rows from BrowseBox, by what
            // CurrentRow is corrected to point two rows down, so that GoToRow will point into
            // emptiness (since we are - purportedly - at the correct position)
            SetCurrent(nNewPos);
    }
    else
    {
        SetCurrent(nNewPos);
        RowModified(nNewPos);
    }

    // if the data cursor was moved from outside, this section is voided
    SetNoSelection();
    m_aBar->InvalidateAll(m_nCurrentPos, m_xCurrentRow.is());
}

sal_Int32 DbGridControl::AlignSeekCursor()
{
    // position SeekCursor onto the data cursor, no data transmission

    if (!m_pSeekCursor)
        return -1;

    Reference< XPropertySet > xSet = m_pDataCursor->getPropertySet();

    // now align the seek cursor and the data cursor
    if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW)))
        m_nSeekPos = GetRowCount() - 1;
    else
    {
        try
        {
            if ( m_pDataCursor->isBeforeFirst() )
            {
                // this is somewhat strange, but can nevertheless happen
                SAL_INFO( "svx.fmcomp", "DbGridControl::AlignSeekCursor: nobody should tamper with my cursor this way (before first)!" );
                m_pSeekCursor->first();
                m_pSeekCursor->previous();
                m_nSeekPos = -1;
            }
            else if ( m_pDataCursor->isAfterLast() )
            {
                SAL_INFO( "svx.fmcomp", "DbGridControl::AlignSeekCursor: nobody should tamper with my cursor this way (after last)!" );
                m_pSeekCursor->last();
                m_pSeekCursor->next();
                m_nSeekPos = -1;
            }
            else
            {
                m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                if (!CompareBookmark(m_pDataCursor->getBookmark(), m_pSeekCursor->getBookmark()))
                    // unfortunately, moveToBookmark might lead to a re-positioning of the seek
                    // cursor (if the complex moveToBookmark with all its events fires an update
                    // somewhere) -> retry
                    m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                    // Now there is still the chance of a failure but it is less likely.
                    // The alternative would be an loop until everything is fine - no good solution...
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            }
        }
        catch(Exception&)
        {
        }
    }
    return m_nSeekPos;
}

bool DbGridControl::SeekCursor(long nRow, bool bAbsolute)
{
    // position SeekCursor onto the data cursor, no data transmission

    // additions for the filtermode
    if (IsFilterRow(nRow))
    {
        m_nSeekPos = 0;
        return true;
    }

    if (!m_pSeekCursor)
        return false;

    // is this an insertion?
    if (IsValid(m_xCurrentRow) && m_xCurrentRow->IsNew() &&
        nRow >= m_nCurrentPos)
    {
        // if so, scrolling down must be prevented as this is already the last data set!
        if (nRow == m_nCurrentPos)
        {
            // no adjustment necessary
            m_nSeekPos = nRow;
        }
        else if (IsInsertionRow(nRow)) // blank row for data insertion
            m_nSeekPos = nRow;
    }
    else if (IsInsertionRow(nRow)) // blank row for data insertion
        m_nSeekPos = nRow;
    else if ((-1 == nRow) && (GetRowCount() == ((m_nOptions & DbGridControlOptions::Insert) ? 1 : 0)) && m_pSeekCursor->isAfterLast())
        m_nSeekPos = nRow;
    else
    {
        bool bSuccess = false;
        long nSteps = 0;
        try
        {
            if ( m_pSeekCursor->rowDeleted() )
            {
                // somebody deleted the current row of the seek cursor. Move it away from this row.
                m_pSeekCursor->next();
                if ( m_pSeekCursor->isAfterLast() || m_pSeekCursor->isBeforeFirst() )
                    bAbsolute = true;
            }

            if ( !bAbsolute )
            {
                DBG_ASSERT( !m_pSeekCursor->isAfterLast() && !m_pSeekCursor->isBeforeFirst(),
                    "DbGridControl::SeekCursor: how did the seek cursor get to this position?!" );
                nSteps = nRow - (m_pSeekCursor->getRow() - 1);
                bAbsolute = bAbsolute || (std::abs(nSteps) > 100);
            }

            if ( bAbsolute )
            {
                bSuccess = m_pSeekCursor->absolute(nRow + 1);
                if (bSuccess)
                    m_nSeekPos = nRow;
            }
            else
            {
                if (nSteps > 0) // position onto the last needed data set
                {
                    if (m_pSeekCursor->isAfterLast())
                        bSuccess = false;
                    else if (m_pSeekCursor->isBeforeFirst())
                        bSuccess = m_pSeekCursor->absolute(nSteps);
                    else
                        bSuccess = m_pSeekCursor->relative(nSteps);
                }
                else if (nSteps < 0)
                {
                    if (m_pSeekCursor->isBeforeFirst())
                        bSuccess = false;
                    else if (m_pSeekCursor->isAfterLast())
                        bSuccess = m_pSeekCursor->absolute(nSteps);
                    else
                        bSuccess = m_pSeekCursor->relative(nSteps);
                }
                else
                {
                    m_nSeekPos = nRow;
                    return true;
                }
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("DbGridControl::SeekCursor : failed ...");
        }

        try
        {
            if (!bSuccess)
            {
                if (bAbsolute || nSteps > 0)
                {
                    if (m_pSeekCursor->isLast())
                        bSuccess = true;
                    else
                        bSuccess = m_pSeekCursor->last();
                }
                else
                {
                    if (m_pSeekCursor->isFirst())
                        bSuccess = true;
                    else
                        bSuccess = m_pSeekCursor->first();
                }
            }

            if (bSuccess)
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            else
                m_nSeekPos = -1;
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("svx");
            OSL_FAIL("DbGridControl::SeekCursor : failed ...");
            m_nSeekPos = -1; // no further data set available
        }
    }
    return m_nSeekPos == nRow;
}

void DbGridControl::MoveToFirst()
{
    if (m_pSeekCursor && (GetCurRow() != 0))
        MoveToPosition(0);
}

void DbGridControl::MoveToLast()
{
    if (!m_pSeekCursor)
        return;

    if (m_nTotalCount < 0) // no RecordCount, yet
    {
        try
        {
            bool bRes = m_pSeekCursor->last();

            if (bRes)
            {
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
                AdjustRows();
            }
        }
        catch(Exception&)
        {
        }
    }

    // position onto the last data set not on a blank row
    if (m_nOptions & DbGridControlOptions::Insert)
    {
        if ((GetRowCount() - 1) > 0)
            MoveToPosition(GetRowCount() - 2);
    }
    else if (GetRowCount())
        MoveToPosition(GetRowCount() - 1);
}

void DbGridControl::MoveToPrev()
{
    long nNewRow = std::max(GetCurRow() - 1, 0L);
    if (GetCurRow() != nNewRow)
        MoveToPosition(nNewRow);
}

void DbGridControl::MoveToNext()
{
    if (!m_pSeekCursor)
        return;

    if (m_nTotalCount > 0)
    {
        // move the data cursor to the right position
        long nNewRow = std::min(GetRowCount() - 1, GetCurRow() + 1);
        if (GetCurRow() != nNewRow)
            MoveToPosition(nNewRow);
    }
    else
    {
        bool bOk = false;
        try
        {
            // try to move to next row
            // when not possible our paint cursor is already on the last row
            // then we must be sure that the data cursor is on the position
            // we call ourself again
            bOk = m_pSeekCursor->next();
            if (bOk)
            {
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
                MoveToPosition(GetCurRow() + 1);
            }
        }
        catch(SQLException &)
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

        if(!bOk)
        {
            AdjustRows();
            if (m_nTotalCount > 0) // only to avoid infinite recursion
                MoveToNext();
        }
    }
}

void DbGridControl::MoveToPosition(sal_uInt32 nPos)
{
    if (!m_pSeekCursor)
        return;

    if (m_nTotalCount < 0 && static_cast<long>(nPos) >= GetRowCount())
    {
        try
        {
            if (!m_pSeekCursor->absolute(nPos + 1))
            {
                AdjustRows();
                return;
            }
            else
            {
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
                AdjustRows();
            }
        }
        catch(Exception&)
        {
            return;
        }
    }
    EditBrowseBox::GoToRow(nPos);
    m_aBar->InvalidateAll(m_nCurrentPos);
}

void DbGridControl::AppendNew()
{
    if (!m_pSeekCursor || !(m_nOptions & DbGridControlOptions::Insert))
        return;

    if (m_nTotalCount < 0) // no RecordCount, yet
    {
        try
        {
            bool bRes = m_pSeekCursor->last();

            if (bRes)
            {
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
                AdjustRows();
            }
        }
        catch(Exception&)
        {
            return;
        }
    }

    long nNewRow = m_nTotalCount + 1;
    if (nNewRow > 0 && GetCurRow() != nNewRow)
        MoveToPosition(nNewRow - 1);
}

void DbGridControl::SetDesignMode(bool bMode)
{
    if (IsDesignMode() != bMode)
    {
        // adjust Enable/Disable for design mode so that the headerbar remains configurable
        if (bMode)
        {
            if (!IsEnabled())
            {
                Enable();
                GetDataWindow().Disable();
            }
        }
        else
        {
            // disable completely
            if (!GetDataWindow().IsEnabled())
                Disable();
        }

        m_bDesignMode = bMode;
        GetDataWindow().SetMouseTransparent(bMode);
        SetMouseTransparent(bMode);

        m_aBar->InvalidateAll(m_nCurrentPos, true);
    }
}

void DbGridControl::SetFilterMode(bool bMode)
{
    if (IsFilterMode() != bMode)
    {
        m_bFilterMode = bMode;

        if (bMode)
        {
            SetUpdateMode(false);

            // there is no cursor anymore
            if (IsEditing())
                DeactivateCell();
            RemoveRows(false);

            m_xEmptyRow = new DbGridRow();

            // setting the new filter controls
            for (auto const & pCurCol : m_aColumns)
            {
                if (!pCurCol->IsHidden())
                    pCurCol->UpdateControl();
            }

            // one row for filtering
            RowInserted(0);
            SetUpdateMode(true);
        }
        else
            setDataSource(Reference< XRowSet > ());
    }
}

OUString DbGridControl::GetCellText(long _nRow, sal_uInt16 _nColId) const
{
    size_t Location = GetModelColumnPos( _nColId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    OUString sRet;
    if ( const_cast<DbGridControl*>(this)->SeekRow(_nRow) )
        sRet = GetCurrentRowCellText(pColumn, m_xPaintRow);
    return sRet;
}

OUString DbGridControl::GetCurrentRowCellText(DbGridColumn const * pColumn,const DbGridRowRef& _rRow) const
{
    // text output for a single row
    OUString aText;
    if ( pColumn && IsValid(_rRow) )
        aText = pColumn->GetCellText(_rRow.get(), m_xFormatter);
    return aText;
}

sal_uInt32 DbGridControl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
{
    if (SeekRow(nRow))
    {
        size_t Location = GetModelColumnPos( nColId );
        DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
        return GetDataWindow().GetTextWidth(GetCurrentRowCellText(pColumn,m_xPaintRow));
    }
    else
        return 30;  // FIXME magic number for default cell width
}

void DbGridControl::PreExecuteRowContextMenu(sal_uInt16 /*nRow*/, PopupMenu& rMenu)
{
    bool bDelete = (m_nOptions & DbGridControlOptions::Delete) && GetSelectRowCount() && !IsCurrentAppending();
    // if only a blank row is selected than do not delete
    bDelete = bDelete && !((m_nOptions & DbGridControlOptions::Insert) && GetSelectRowCount() == 1 && IsRowSelected(GetRowCount() - 1));

    rMenu.EnableItem(rMenu.GetItemId("delete"), bDelete);
    rMenu.EnableItem(rMenu.GetItemId("save"), IsModified());

    // the undo is more difficult
    bool bCanUndo = IsModified();
    int nState = -1;
    if (m_aMasterStateProvider.IsSet())
        nState = m_aMasterStateProvider.Call(DbGridControlNavigationBarState::Undo);
    bCanUndo &= ( 0 != nState );

    rMenu.EnableItem(rMenu.GetItemId("undo"), bCanUndo);
}

void DbGridControl::PostExecuteRowContextMenu(sal_uInt16 /*nRow*/, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    if (nExecutionResult == rMenu.GetItemId("delete"))
    {
        // delete asynchronously
        if (m_nDeleteEvent)
            Application::RemoveUserEvent(m_nDeleteEvent);
        m_nDeleteEvent = Application::PostUserEvent(LINK(this,DbGridControl,OnDelete), nullptr, true);
    }
    else if (nExecutionResult == rMenu.GetItemId("undo"))
        Undo();
    else if (nExecutionResult == rMenu.GetItemId("save"))
        SaveRow();
}

void DbGridControl::DataSourcePropertyChanged(const PropertyChangeEvent& evt)
{
    SAL_INFO("svx.fmcomp", "DbGridControl::DataSourcePropertyChanged");
    SolarMutexGuard aGuard;
    // prop "IsModified" changed ?
    // during update don't care about the modified state
    if (!IsUpdating() && evt.PropertyName == FM_PROP_ISMODIFIED )
    {
        Reference< XPropertySet > xSource(evt.Source, UNO_QUERY);
        DBG_ASSERT( xSource.is(), "DbGridControl::DataSourcePropertyChanged: invalid event source!" );
        bool bIsNew = false;
        if (xSource.is())
            bIsNew = ::comphelper::getBOOL(xSource->getPropertyValue(FM_PROP_ISNEW));

        if (bIsNew && m_xCurrentRow.is())
        {
            DBG_ASSERT(::comphelper::getBOOL(xSource->getPropertyValue(FM_PROP_ROWCOUNTFINAL)), "DbGridControl::DataSourcePropertyChanged : somebody moved the form to a new record before the row count was final !");
            sal_Int32 nRecordCount = 0;
            xSource->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
            if (::comphelper::getBOOL(evt.NewValue))
            {   // modified state changed from sal_False to sal_True and we're on a insert row
                // -> we've to add a new grid row
                if ((nRecordCount == GetRowCount() - 1)  && m_xCurrentRow->IsNew())
                {
                    RowInserted(GetRowCount());
                    InvalidateStatusCell(m_nCurrentPos);
                    m_aBar->InvalidateAll(m_nCurrentPos);
                }
            }
            else
            {   // modified state changed from sal_True to sal_False and we're on a insert row
                // we have two "new row"s at the moment : the one we're editing currently (where the current
                // column is the only dirty element) and a "new new" row which is completely clean. As the first
                // one is about to be cleaned, too, the second one is obsolete now.
                if (m_xCurrentRow->IsNew() && nRecordCount == (GetRowCount() - 2))
                {
                    RowRemoved(GetRowCount() - 1);
                    InvalidateStatusCell(m_nCurrentPos);
                    m_aBar->InvalidateAll(m_nCurrentPos);
                }
            }
        }
        if (m_xCurrentRow.is())
        {
            m_xCurrentRow->SetStatus(::comphelper::getBOOL(evt.NewValue) ? GridRowStatus::Modified : GridRowStatus::Clean);
            m_xCurrentRow->SetNew( bIsNew );
            InvalidateStatusCell(m_nCurrentPos);
            SAL_INFO("svx.fmcomp", "modified flag changed, new state: " << ROWSTATUS(m_xCurrentRow));
        }
    }
}

void DbGridControl::StartDrag( sal_Int8 /*nAction*/, const Point& rPosPixel )
{
    if (!m_pSeekCursor || IsResizing())
        return;

    sal_uInt16 nColId = GetColumnAtXPosPixel(rPosPixel.X());
    long   nRow = GetRowAtYPosPixel(rPosPixel.Y());
    if (nColId != HandleColumnId && nRow >= 0)
    {
        if (GetDataWindow().IsMouseCaptured())
            GetDataWindow().ReleaseMouse();

        size_t Location = GetModelColumnPos( nColId );
        DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
        rtl::Reference<OStringTransferable> pTransferable = new OStringTransferable(GetCurrentRowCellText(pColumn,m_xPaintRow));
        pTransferable->StartDrag(this, DND_ACTION_COPY);
    }
}

bool DbGridControl::canCopyCellText(sal_Int32 _nRow, sal_uInt16 _nColId)
{
    return  (_nRow >= 0)
        &&  (_nRow < GetRowCount())
        &&  (_nColId != HandleColumnId)
        &&  (_nColId <= ColCount());
}

void DbGridControl::copyCellText(sal_Int32 _nRow, sal_uInt16 _nColId)
{
    DBG_ASSERT(canCopyCellText(_nRow, _nColId), "DbGridControl::copyCellText: invalid call!");
    DbGridColumn* pColumn = m_aColumns[ GetModelColumnPos(_nColId) ].get();
    SeekRow(_nRow);
    OStringTransfer::CopyString( GetCurrentRowCellText( pColumn,m_xPaintRow ), this );
}

void DbGridControl::executeRowContextMenu( long _nRow, const Point& _rPreferredPos )
{
    VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/rowsmenu.ui", "");
    VclPtr<PopupMenu> aContextMenu(aBuilder.get_menu("menu"));

    PreExecuteRowContextMenu( static_cast<sal_uInt16>(_nRow), *aContextMenu.get() );
    aContextMenu->RemoveDisabledEntries( true, true );
    PostExecuteRowContextMenu( static_cast<sal_uInt16>(_nRow), *aContextMenu.get(), aContextMenu->Execute( this, _rPreferredPos ) );
}

void DbGridControl::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            if ( !m_pSeekCursor )
            {
                EditBrowseBox::Command(rEvt);
                return;
            }

            if ( !rEvt.IsMouseEvent() )
            {   // context menu requested by keyboard
                if ( GetSelectRowCount() )
                {
                    long nRow = FirstSelectedRow( );

                    ::tools::Rectangle aRowRect( GetRowRectPixel( nRow ) );
                    executeRowContextMenu( nRow, aRowRect.LeftCenter() );

                    // handled
                    return;
                }
            }

            sal_uInt16 nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
            long   nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());

            if (nColId == HandleColumnId)
            {
                executeRowContextMenu( nRow, rEvt.GetMousePosPixel() );
            }
            else if (canCopyCellText(nRow, nColId))
            {
                VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/cellmenu.ui", "");
                VclPtr<PopupMenu> aContextMenu(aBuilder.get_menu("menu"));
                if (aContextMenu->Execute(this, rEvt.GetMousePosPixel()))
                    copyCellText(nRow, nColId);
            }
            else
            {
                EditBrowseBox::Command(rEvt);
                return;
            }

            SAL_FALLTHROUGH;
        }
        default:
            EditBrowseBox::Command(rEvt);
    }
}

IMPL_LINK_NOARG(DbGridControl, OnDelete, void*, void)
{
    m_nDeleteEvent = nullptr;
    DeleteSelectedRows();
}

void DbGridControl::DeleteSelectedRows()
{
    DBG_ASSERT(GetSelection(), "no selection!!!");

    if (!m_pSeekCursor)
        return;
}

CellController* DbGridControl::GetController(long /*nRow*/, sal_uInt16 nColumnId)
{
    if (!IsValid(m_xCurrentRow) || !IsEnabled())
        return nullptr;

    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    if (!pColumn)
        return nullptr;

    CellController* pReturn = nullptr;
    if (IsFilterMode())
        pReturn = pColumn->GetController().get();
    else
    {
        if (::comphelper::hasProperty(FM_PROP_ENABLED, pColumn->getModel()))
        {
            if (!::comphelper::getBOOL(pColumn->getModel()->getPropertyValue(FM_PROP_ENABLED)))
                return nullptr;
        }

        bool bInsert = (m_xCurrentRow->IsNew() && (m_nOptions & DbGridControlOptions::Insert));
        bool bUpdate = (!m_xCurrentRow->IsNew() && (m_nOptions & DbGridControlOptions::Update));

        if ((bInsert && !pColumn->IsAutoValue()) || bUpdate)
        {
            pReturn = pColumn->GetController().get();
        }
    }
    return pReturn;
}

void DbGridControl::CellModified()
{
    SAL_INFO("svx.fmcomp", "DbGridControl::CellModified");

    {
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        if (m_nAsynAdjustEvent)
        {
            SAL_INFO("svx.fmcomp", "forcing a synchron call to " << (m_bPendingAdjustRows ? "AdjustRows" : "AdustDataSource"));
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = nullptr;

            // force the call : this should be no problem as we're probably running in the solar thread here
            // (cell modified is triggered by user actions)
            if (m_bPendingAdjustRows)
                AdjustRows();
            else
                AdjustDataSource();
        }
    }

    if (!IsFilterMode() && IsValid(m_xCurrentRow) && !m_xCurrentRow->IsModified())
    {
        // enable edit mode
        // a data set should be inserted
        if (m_xCurrentRow->IsNew())
        {
            m_xCurrentRow->SetStatus(GridRowStatus::Modified);
            SAL_INFO("svx.fmcomp", "current row is new, new state: MODIFIED");
            // if no row was added yet, do it now
            if (m_nCurrentPos == GetRowCount() - 1)
            {
                // increment RowCount
                RowInserted(GetRowCount());
                InvalidateStatusCell(m_nCurrentPos);
                m_aBar->InvalidateAll(m_nCurrentPos);
            }
        }
        else if (m_xCurrentRow->GetStatus() != GridRowStatus::Modified)
        {
            m_xCurrentRow->SetState(m_pDataCursor.get(), false);
            SAL_INFO("svx.fmcomp", "current row is not new, after SetState, new state: " << ROWSTATUS(m_xCurrentRow));
            m_xCurrentRow->SetStatus(GridRowStatus::Modified);
            SAL_INFO("svx.fmcomp", "current row is not new, new state: MODIFIED");
            InvalidateStatusCell(m_nCurrentPos);
        }
    }
}

void DbGridControl::Dispatch(sal_uInt16 nId)
{
    if (nId == BROWSER_CURSORENDOFFILE)
    {
        if (m_nOptions & DbGridControlOptions::Insert)
            AppendNew();
        else
            MoveToLast();
    }
    else
        EditBrowseBox::Dispatch(nId);
}

void DbGridControl::Undo()
{
    if (IsFilterMode() || !IsValid(m_xCurrentRow) || !IsModified())
        return;

    // check if we have somebody doin' the UNDO for us
    int nState = -1;
    if (m_aMasterStateProvider.IsSet())
        nState = m_aMasterStateProvider.Call(DbGridControlNavigationBarState::Undo);
    if (nState>0)
    {   // yes, we have, and the slot is enabled
        DBG_ASSERT(m_aMasterSlotExecutor.IsSet(), "DbGridControl::Undo : a state, but no execute link ?");
        bool lResult = m_aMasterSlotExecutor.Call(DbGridControlNavigationBarState::Undo);
        if (lResult)
            // handled
            return;
    }
    else if (nState == 0)
        // yes, we have, and the slot is disabled
        return;

    BeginCursorAction();

    bool bAppending = m_xCurrentRow->IsNew();
    bool bDirty     = m_xCurrentRow->IsModified();

    try
    {
        // cancel editing
        Reference< XResultSetUpdate >  xUpdateCursor(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);
        // no effects if we're not updating currently
        if (bAppending)
            // just refresh the row
            xUpdateCursor->moveToInsertRow();
        else
            xUpdateCursor->cancelRowUpdates();

    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    EndCursorAction();

    m_xDataRow->SetState(m_pDataCursor.get(), false);
    if (m_xPaintRow == m_xCurrentRow)
        m_xPaintRow = m_xCurrentRow = m_xDataRow;
    else
        m_xCurrentRow = m_xDataRow;

    if (bAppending && (EditBrowseBox::IsModified() || bDirty))
        // remove the row
        if (m_nCurrentPos == GetRowCount() - 2)
        {   // maybe we already removed it (in resetCurrentRow, called if the above moveToInsertRow
            // caused our data source form to be reset - which should be the usual case ....)
            RowRemoved(GetRowCount() - 1);
            m_aBar->InvalidateAll(m_nCurrentPos);
        }

    RowModified(m_nCurrentPos);
}

void DbGridControl::resetCurrentRow()
{
    if (IsModified())
    {
        // scenario : we're on the insert row, the row is dirty, and thus there exists a "second" insert row (which
        // is clean). Normally in DataSourcePropertyChanged we would remove this second row if the modified state of
        // the insert row changes from sal_True to sal_False. But if our current cell is the only modified element (means the
        // data source isn't modified) and we're reset this DataSourcePropertyChanged would never be called, so we
        // would never delete the obsolete "second insert row". Thus in this special case this method here
        // is the only possibility to determine the redundance of the row (resetCurrentRow is called when the
        // "first insert row" is about to be cleaned, so of course the "second insert row" is redundant now)
        Reference< XPropertySet > xDataSource = getDataSource()->getPropertySet();
        if (xDataSource.is() && !::comphelper::getBOOL(xDataSource->getPropertyValue(FM_PROP_ISMODIFIED)))
        {
            // are we on a new row currently ?
            if (m_xCurrentRow->IsNew())
            {
                if (m_nCurrentPos == GetRowCount() - 2)
                {
                    RowRemoved(GetRowCount() - 1);
                    m_aBar->InvalidateAll(m_nCurrentPos);
                }
            }
        }

        // update the rows
        m_xDataRow->SetState(m_pDataCursor.get(), false);
        if (m_xPaintRow == m_xCurrentRow)
            m_xPaintRow = m_xCurrentRow = m_xDataRow;
        else
            m_xCurrentRow = m_xDataRow;
    }

    RowModified(GetCurRow()); // will update the current controller if affected
}

void DbGridControl::RowModified( long nRow )
{
    if (nRow == m_nCurrentPos && IsEditing())
    {
        CellControllerRef aTmpRef = Controller();
        aTmpRef->ClearModified();
        InitController(aTmpRef, m_nCurrentPos, GetCurColumnId());
    }
    EditBrowseBox::RowModified(nRow);
}

bool DbGridControl::IsModified() const
{
    return !IsFilterMode() && IsValid(m_xCurrentRow) && (m_xCurrentRow->IsModified() || EditBrowseBox::IsModified());
}

bool DbGridControl::IsCurrentAppending() const
{
    return m_xCurrentRow.is() && m_xCurrentRow->IsNew();
}

bool DbGridControl::IsInsertionRow(long nRow) const
{
    return (m_nOptions & DbGridControlOptions::Insert) && m_nTotalCount >= 0 && (nRow == GetRowCount() - 1);
}

bool DbGridControl::SaveModified()
{
    SAL_INFO("svx.fmcomp", "DbGridControl::SaveModified");
    DBG_ASSERT(IsValid(m_xCurrentRow), "GridControl:: Invalid row");
    if (!IsValid(m_xCurrentRow))
        return true;

    // accept input for this field
    // Where there changes at the current input field?
    if (!EditBrowseBox::IsModified())
        return true;

    size_t Location = GetModelColumnPos( GetCurColumnId() );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    bool bOK = pColumn && pColumn->Commit();
    DBG_ASSERT( Controller().is(), "DbGridControl::SaveModified: was modified, by have no controller?!" );
    if ( !Controller().is() )
        // this might happen if the callbacks implicitly triggered by Commit
        // fiddled with the form or the control ...
        // (Note that this here is a workaround, at most. We need a general concept how
        // to treat this, you can imagine an arbitrary number of scenarios where a callback
        // triggers something which leaves us in an expected state.)
        // #i67147# / 2006-07-17 / frank.schoenheit@sun.com
        return bOK;

    if (bOK)
    {
        Controller()->ClearModified();

        if ( IsValid(m_xCurrentRow) )
        {
            m_xCurrentRow->SetState(m_pDataCursor.get(), false);
            SAL_INFO("svx.fmcomp", "explicit SetState, new state: " << ROWSTATUS(m_xCurrentRow));
            InvalidateStatusCell( m_nCurrentPos );
        }
        else
        {
            SAL_INFO("svx.fmcomp", "no SetState, new state: " << ROWSTATUS(m_xCurrentRow));
        }
    }
    else
    {
        // reset the modified flag ....
        Controller()->SetModified();
    }

    return bOK;
}

bool DbGridControl::SaveRow()
{
    SAL_INFO("svx.fmcomp", "DbGridControl::SaveRow");
    // valid row
    if (!IsValid(m_xCurrentRow) || !IsModified())
        return true;
    // value of the controller was not saved, yet
    else if (Controller().is() && Controller()->IsModified())
    {
        if (!SaveModified())
            return false;
    }
    m_bUpdating = true;

    BeginCursorAction();
    bool bAppending = m_xCurrentRow->IsNew();
    bool bSuccess = false;
    try
    {
        Reference< XResultSetUpdate >  xUpdateCursor(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);
        if (bAppending)
            xUpdateCursor->insertRow();
        else
            xUpdateCursor->updateRow();
        bSuccess = true;
    }
    catch(SQLException&)
    {
        EndCursorAction();
        m_bUpdating = false;
        return false;
    }

    try
    {
        if (bSuccess)
        {
            // if we are appending we still sit on the insert row
            // we don't move just clear the flags not to move on the current row
            m_xCurrentRow->SetState(m_pDataCursor.get(), false);
            SAL_INFO("svx.fmcomp", "explicit SetState after a successful update, new state: " << ROWSTATUS(m_xCurrentRow));
            m_xCurrentRow->SetNew(false);

            // adjust the seekcursor if it is on the same position as the datacursor
            if (m_nSeekPos == m_nCurrentPos || bAppending)
            {
                // get the bookmark to refetch the data
                // in insert mode we take the new bookmark of the data cursor
                Any aBookmark = bAppending ? m_pDataCursor->getBookmark() : m_pSeekCursor->getBookmark();
                m_pSeekCursor->moveToBookmark(aBookmark);
                // get the data
                m_xSeekRow->SetState(m_pSeekCursor.get(), true);
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            }
        }
        // and repaint the row
        RowModified(m_nCurrentPos);
    }
    catch(Exception&)
    {
    }

    m_bUpdating = false;
    EndCursorAction();

    // The old code returned (nRecords != 0) here.
    // Me thinks this is wrong : If something goes wrong while update the record, an exception will be thrown,
    // which results in a "return sal_False" (see above). If no exception is thrown, everything is fine. If nRecords
    // is zero, this simply means all fields had their original values.
    // FS - 06.12.99 - 70502
    return true;
}

bool DbGridControl::PreNotify(NotifyEvent& rEvt)
{
    // do not handle events of the Navbar
    if (m_aBar->IsWindowOrChild(rEvt.GetWindow()))
        return BrowseBox::PreNotify(rEvt);

    switch (rEvt.GetType())
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rEvt.GetKeyEvent();

            sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();
            bool   bShift = pKeyEvent->GetKeyCode().IsShift();
            bool   bCtrl = pKeyEvent->GetKeyCode().IsMod1();
            bool   bAlt = pKeyEvent->GetKeyCode().IsMod2();
            if ( ( KEY_TAB == nCode ) && bCtrl && !bAlt )
            {
                // Ctrl-Tab is used to step out of the control, without traveling to the
                // remaining cells first
                // -> build a new key event without the Ctrl-key, and let the very base class handle it
                vcl::KeyCode aNewCode( KEY_TAB, bShift, false, false, false );
                KeyEvent aNewEvent( pKeyEvent->GetCharCode(), aNewCode );

                // call the Control - our direct base class will interpret this in a way we do not want (and do
                // a cell traveling)
                Control::KeyInput( aNewEvent );
                return true;
            }

            if ( !bShift && !bCtrl && ( KEY_ESCAPE == nCode ) )
            {
                if (IsModified())
                {
                    Undo();
                    return true;
                }
            }
            else if ( ( KEY_DELETE == nCode ) && !bShift && !bCtrl )    // delete rows
            {
                if ((m_nOptions & DbGridControlOptions::Delete) && GetSelectRowCount())
                {
                    // delete asynchronously
                    if (m_nDeleteEvent)
                        Application::RemoveUserEvent(m_nDeleteEvent);
                    m_nDeleteEvent = Application::PostUserEvent(LINK(this,DbGridControl,OnDelete), nullptr, true);
                    return true;
                }
            }

            SAL_FALLTHROUGH;
        }
        default:
            return EditBrowseBox::PreNotify(rEvt);
    }
}

bool DbGridControl::IsTabAllowed(bool bRight) const
{
    if (bRight)
        // Tab only if not on the _last_ row
        return GetCurRow() < (GetRowCount() - 1) || !m_bRecordCountFinal ||
               GetViewColumnPos(GetCurColumnId()) < (GetViewColCount() - 1);
    else
    {
        // Tab only if not on the _first_ row
        return GetCurRow() > 0 || (GetCurColumnId() && GetViewColumnPos(GetCurColumnId()) > 0);
    }
}

void DbGridControl::KeyInput( const KeyEvent& rEvt )
{
    if (rEvt.GetKeyCode().GetFunction() == KeyFuncType::COPY)
    {
        long nRow = GetCurRow();
        sal_uInt16 nColId = GetCurColumnId();
        if (nRow >= 0 && nRow < GetRowCount() && nColId < ColCount())
        {
            size_t Location = GetModelColumnPos( nColId );
            DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
            OStringTransfer::CopyString( GetCurrentRowCellText( pColumn, m_xCurrentRow ), this );
            return;
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

void DbGridControl::HideColumn(sal_uInt16 nId)
{
    DeactivateCell();

    // determine the col for the focus to set to after removal
    sal_uInt16 nPos = GetViewColumnPos(nId);
    sal_uInt16 nNewColId = nPos == (ColCount()-1)
        ? GetColumnIdFromViewPos(nPos-1)    // last col is to be removed -> take the previous
        : GetColumnIdFromViewPos(nPos+1);   // take the next

    long lCurrentWidth = GetColumnWidth(nId);
    EditBrowseBox::RemoveColumn(nId);
        // don't use my own RemoveColumn, this would remove it from m_aColumns, too

    // update my model
    size_t Location = GetModelColumnPos( nId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    DBG_ASSERT(pColumn, "DbGridControl::HideColumn : somebody did hide a nonexistent column !");
    if (pColumn)
    {
        pColumn->m_bHidden = true;
        pColumn->m_nLastVisibleWidth = CalcReverseZoom(lCurrentWidth);
    }

    // and reset the focus
    if ( nId == GetCurColumnId() )
        GoToColumnId( nNewColId );
}

void DbGridControl::ShowColumn(sal_uInt16 nId)
{
    sal_uInt16 nPos = GetModelColumnPos(nId);
    DBG_ASSERT(nPos != GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : invalid argument !");
    if (nPos == GRID_COLUMN_NOT_FOUND)
        return;

    DbGridColumn* pColumn = m_aColumns[ nPos ].get();
    if (!pColumn->IsHidden())
    {
        DBG_ASSERT(GetViewColumnPos(nId) != GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : inconsistent internal state !");
            // if the column isn't marked as hidden, it should be visible, shouldn't it ?
        return;
    }
    DBG_ASSERT(GetViewColumnPos(nId) == GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : inconsistent internal state !");
        // the opposite situation ...

    // to determine the new view position we need an adjacent non-hidden column
    sal_uInt16 nNextNonHidden = BROWSER_INVALIDID;
    // first search the cols to the right
    for ( size_t i = nPos + 1; i < m_aColumns.size(); ++i )
    {
        DbGridColumn* pCurCol = m_aColumns[ i ].get();
        if (!pCurCol->IsHidden())
        {
            nNextNonHidden = i;
            break;
        }
    }
    if ((nNextNonHidden == BROWSER_INVALIDID) && (nPos > 0))
    {
        // then to the left
        for ( size_t i = nPos; i > 0; --i )
        {
            DbGridColumn* pCurCol = m_aColumns[ i-1 ].get();
            if (!pCurCol->IsHidden())
            {
                nNextNonHidden = i-1;
                break;
            }
        }
    }
    sal_uInt16 nNewViewPos = (nNextNonHidden == BROWSER_INVALIDID)
        ? 1 // there is no visible column -> insert behind the handle col
        : GetViewColumnPos( m_aColumns[ nNextNonHidden ]->GetId() ) + 1;
            // the first non-handle col has "view pos" 0, but the pos arg for InsertDataColumn expects
            // a position 1 for the first non-handle col -> +1
    DBG_ASSERT(nNewViewPos != GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : inconsistent internal state !");
        // we found a col marked as visible but got no view pos for it ...

    if ((nNextNonHidden<nPos) && (nNextNonHidden != BROWSER_INVALIDID))
        // nNextNonHidden is a column to the left, so we want to insert the new col _right_ beside it's pos
        ++nNewViewPos;

    DeactivateCell();

    OUString aName;
    pColumn->getModel()->getPropertyValue(FM_PROP_LABEL) >>= aName;
    InsertDataColumn(nId, aName, CalcZoom(pColumn->m_nLastVisibleWidth), HeaderBarItemBits::CENTER | HeaderBarItemBits::VCENTER | HeaderBarItemBits::CLICKABLE, nNewViewPos);
    pColumn->m_bHidden = false;

    ActivateCell();
    Invalidate();
}

sal_uInt16 DbGridControl::GetColumnIdFromModelPos( sal_uInt16 nPos ) const
{
    if (nPos >= m_aColumns.size())
    {
        OSL_FAIL("DbGridControl::GetColumnIdFromModelPos : invalid argument !");
        return GRID_COLUMN_NOT_FOUND;
    }

    DbGridColumn* pCol = m_aColumns[ nPos ].get();
#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
    // in the debug version, we convert the ModelPos into a ViewPos and compare this with the
    // value we will return (nId at the corresponding Col in m_aColumns)

    if (!pCol->IsHidden())
    {   // makes sense only if the column is visible
        sal_uInt16 nViewPos = nPos;
        for ( size_t i = 0; i < m_aColumns.size() && i < nPos; ++i)
            if ( m_aColumns[ i ]->IsHidden())
                --nViewPos;

        DBG_ASSERT(pCol && GetColumnIdFromViewPos(nViewPos) == pCol->GetId(),
            "DbGridControl::GetColumnIdFromModelPos : this isn't consistent .... did I misunderstand something ?");
    }
#endif
    return pCol->GetId();
}

sal_uInt16 DbGridControl::GetModelColumnPos( sal_uInt16 nId ) const
{
    for ( size_t i = 0; i < m_aColumns.size(); ++i )
        if ( m_aColumns[ i ]->GetId() == nId )
            return i;

    return GRID_COLUMN_NOT_FOUND;
}

void DbGridControl::implAdjustInSolarThread(bool _bRows)
{
    SAL_INFO("svx.fmcomp", "DbGridControl::implAdjustInSolarThread");
    ::osl::MutexGuard aGuard(m_aAdjustSafety);
    if (::osl::Thread::getCurrentIdentifier() != Application::GetMainThreadIdentifier())
    {
        m_nAsynAdjustEvent = PostUserEvent(LINK(this, DbGridControl, OnAsyncAdjust), reinterpret_cast< void* >( _bRows ), true);
        m_bPendingAdjustRows = _bRows;
        if (_bRows)
            SAL_INFO("svx.fmcomp", "posting an AdjustRows");
        else
            SAL_INFO("svx.fmcomp", "posting an AdjustDataSource");
    }
    else
    {
        if (_bRows)
            SAL_INFO("svx.fmcomp", "doing an AdjustRows");
        else
            SAL_INFO("svx.fmcomp", "doing an AdjustDataSource");
        // always adjust the rows before adjusting the data source
        // If this is not necessary (because the row count did not change), nothing is done
        // The problem is that we can't rely on the order of which the calls come in: If the cursor was moved
        // to a position behind row count know 'til now, the cursorMoved notification may come before the
        // RowCountChanged notification
        // 94093 - 02.11.2001 - frank.schoenheit@sun.com
        AdjustRows();

        if ( !_bRows )
            AdjustDataSource();
    }
}

IMPL_LINK(DbGridControl, OnAsyncAdjust, void*, pAdjustWhat, void)
{
    m_nAsynAdjustEvent = nullptr;

    AdjustRows();
        // see implAdjustInSolarThread for a comment why we do this every time

    if ( !pAdjustWhat )
        AdjustDataSource();
}

void DbGridControl::BeginCursorAction()
{
    if (m_pFieldListeners)
    {
        ColumnFieldValueListeners* pListeners = static_cast<ColumnFieldValueListeners*>(m_pFieldListeners);
        ColumnFieldValueListeners::const_iterator aIter = pListeners->begin();
        while (aIter != pListeners->end())
        {
            GridFieldValueListener* pCurrent = (*aIter).second;
            if (pCurrent)
                pCurrent->suspend();
            ++aIter;
        }
    }

    if (m_pDataSourcePropListener)
        m_pDataSourcePropListener->suspend();
}

void DbGridControl::EndCursorAction()
{
    if (m_pFieldListeners)
    {
        ColumnFieldValueListeners* pListeners = static_cast<ColumnFieldValueListeners*>(m_pFieldListeners);
        ColumnFieldValueListeners::const_iterator aIter = pListeners->begin();
        while (aIter != pListeners->end())
        {
            GridFieldValueListener* pCurrent = (*aIter).second;
            if (pCurrent)
                pCurrent->resume();
            ++aIter;
        }
    }

    if (m_pDataSourcePropListener)
        m_pDataSourcePropListener->resume();
}

void DbGridControl::ConnectToFields()
{
    ColumnFieldValueListeners* pListeners = static_cast<ColumnFieldValueListeners*>(m_pFieldListeners);
    DBG_ASSERT(!pListeners || pListeners->empty(), "DbGridControl::ConnectToFields : please call DisconnectFromFields first !");

    if (!pListeners)
    {
        pListeners = new ColumnFieldValueListeners;
        m_pFieldListeners = pListeners;
    }

    for (auto const & pCurrent : m_aColumns)
    {
        sal_uInt16 nViewPos = pCurrent ? GetViewColumnPos(pCurrent->GetId()) : GRID_COLUMN_NOT_FOUND;
        if (GRID_COLUMN_NOT_FOUND == nViewPos)
            continue;

        Reference< XPropertySet >  xField = pCurrent->GetField();
        if (!xField.is())
            continue;

        // column is visible and bound here
        GridFieldValueListener*& rpListener = (*pListeners)[pCurrent->GetId()];
        DBG_ASSERT(!rpListener, "DbGridControl::ConnectToFields : already a listener for this column ?!");
        rpListener = new GridFieldValueListener(*this, xField, pCurrent->GetId());
    }
}

void DbGridControl::DisconnectFromFields()
{
    if (!m_pFieldListeners)
        return;

    ColumnFieldValueListeners* pListeners = static_cast<ColumnFieldValueListeners*>(m_pFieldListeners);
    while (!pListeners->empty())
    {
        sal_Int32 nOldSize = pListeners->size();
        pListeners->begin()->second->dispose();
        DBG_ASSERT(nOldSize > static_cast<sal_Int32>(pListeners->size()), "DbGridControl::DisconnectFromFields : dispose on a listener should result in a removal from my list !");
    }

    delete pListeners;
    m_pFieldListeners = nullptr;
}

void DbGridControl::FieldValueChanged(sal_uInt16 _nId)
{
    osl::MutexGuard aPreventDestruction(m_aDestructionSafety);
    // needed as this may run in a thread other than the main one
    if (GetRowStatus(GetCurRow()) != EditBrowseBox::MODIFIED)
        // all other cases are handled elsewhere
        return;

    size_t Location = GetModelColumnPos( _nId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    if (pColumn)
    {
        std::unique_ptr<vcl::SolarMutexTryAndBuyGuard> pGuard;
        while (!m_bWantDestruction && (!pGuard || !pGuard->isAcquired()))
            pGuard.reset(new vcl::SolarMutexTryAndBuyGuard);

        if (m_bWantDestruction)
        {   // at this moment, within another thread, our destructor tries to destroy the listener which called this method
            // => don't do anything
            // 73365 - 23.02.00 - FS
            return;
        }

        // and finally do the update ...
        pColumn->UpdateFromField(m_xCurrentRow.get(), m_xFormatter);
        RowModified(GetCurRow());
    }
}

void DbGridControl::FieldListenerDisposing(sal_uInt16 _nId)
{
    ColumnFieldValueListeners* pListeners = static_cast<ColumnFieldValueListeners*>(m_pFieldListeners);
    if (!pListeners)
    {
        OSL_FAIL("DbGridControl::FieldListenerDisposing : invalid call (have no listener array) !");
        return;
    }

    ColumnFieldValueListeners::const_iterator aPos = pListeners->find(_nId);
    if (aPos == pListeners->end())
    {
        OSL_FAIL("DbGridControl::FieldListenerDisposing : invalid call (did not find the listener) !");
        return;
    }

    delete aPos->second;

    pListeners->erase(aPos);
}

void DbGridControl::disposing(sal_uInt16 _nId)
{
    if (_nId == 0)
    {   // the seek cursor is being disposed
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        setDataSource(nullptr, DbGridControlOptions::Readonly); // our clone was disposed so we set our datasource to null to avoid later access to it
        if (m_nAsynAdjustEvent)
        {
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = nullptr;
        }
    }
}

sal_Int32 DbGridControl::GetAccessibleControlCount() const
{
    return EditBrowseBox::GetAccessibleControlCount() + 1; // the navigation control
}

Reference<XAccessible > DbGridControl::CreateAccessibleControl( sal_Int32 _nIndex )
{
    Reference<XAccessible > xRet;
    if ( _nIndex == EditBrowseBox::GetAccessibleControlCount() )
    {
        xRet = m_aBar->GetAccessible();
    }
    else
        xRet = EditBrowseBox::CreateAccessibleControl( _nIndex );
    return xRet;
}

Reference< XAccessible > DbGridControl::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    sal_uInt16 nColumnId = GetColumnId( _nColumnPos );
    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ].get() : nullptr;
    if ( pColumn )
    {
        Reference< css::awt::XControl> xInt(pColumn->GetCell());
        Reference< css::awt::XCheckBox> xBox(xInt,UNO_QUERY);
        if ( xBox.is() )
        {
            TriState eValue = TRISTATE_FALSE;
            switch( xBox->getState() )
            {
                case 0:
                    eValue = TRISTATE_FALSE;
                    break;
                case 1:
                    eValue = TRISTATE_TRUE;
                    break;
                case 2:
                    eValue = TRISTATE_INDET;
                    break;
            }
            return EditBrowseBox::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,eValue );
        }
    }
    return EditBrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
