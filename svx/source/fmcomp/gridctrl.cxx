/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <sal/macros.h>
#include "fmhelp.hrc"
#include <svx/gridctrl.hxx>
#include "gridcell.hxx"
#include "svx/dbtoolsclient.hxx"
#include "svx/fmtools.hxx"
#include <svtools/stringtransfer.hxx>

#include "fmprop.hrc"
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/XRowsChangeBroadcaster.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <comphelper/extract.hxx>
#include <tools/resid.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/menu.hxx>

#include "svx/fmresids.hrc"

#include <svx/svxids.hrc>
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>
#include "fmservs.hxx"
#include "sdbdatacolumn.hxx"

#include <comphelper/stl_types.hxx>
#include <comphelper/property.hxx>
#include "trace.hxx"

#include <algorithm>

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

#define ROWSTATUS(row)  !row.Is() ? "NULL" : row->GetStatus() == GRS_CLEAN ? "CLEAN" : row->GetStatus() == GRS_MODIFIED ? "MODIFIED" : row->GetStatus() == GRS_DELETED ? "DELETED" : "INVALID"


#define DEFAULT_BROWSE_MODE             \
              BROWSER_COLUMNSELECTION   \
            | BROWSER_MULTISELECTION    \
            | BROWSER_KEEPSELECTION     \
            | BROWSER_TRACKING_TIPS     \
            | BROWSER_HLINESFULL        \
            | BROWSER_VLINESFULL        \
            | BROWSER_HEADERBAR_NEW     \

class RowSetEventListener : public ::cppu::WeakImplHelper1<XRowsChangeListener>
{
    DbGridControl* m_pControl;
public:
    RowSetEventListener(DbGridControl* i_pControl) : m_pControl(i_pControl)
    {
    }
private:
    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& /*i_aEvt*/) throw ( RuntimeException )
    {
    }
    virtual void SAL_CALL rowsChanged(const ::com::sun::star::sdb::RowsChangeEvent& i_aEvt) throw ( RuntimeException )
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
                rSeekRow->SetState(pSeek, sal_True);
                sal_Int32 nSeekPos = pSeek->getRow() - 1;
                m_pControl->SetSeekPos(nSeekPos,aAccess);
                m_pControl->RowModified(nSeekPos);
            }
        }
    }
};
//==============================================================================

class GridFieldValueListener;
DECLARE_STL_MAP(sal_uInt16, GridFieldValueListener*, ::std::less<sal_uInt16>, ColumnFieldValueListeners);

//==============================================================================

DBG_NAME(GridFieldValueListener)
class GridFieldValueListener : protected ::comphelper::OPropertyChangeListener
{
    osl::Mutex                          m_aMutex;
    DbGridControl&                      m_rParent;
    ::comphelper::OPropertyChangeMultiplexer*   m_pRealListener;
    sal_uInt16                          m_nId;
    sal_Int16                           m_nSuspended;
    sal_Bool                            m_bDisposed : 1;

public:
    GridFieldValueListener(DbGridControl& _rParent, const Reference< XPropertySet >& xField, sal_uInt16 _nId);
    virtual ~GridFieldValueListener();

    virtual void _propertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException );

    void suspend() { ++m_nSuspended; }
    void resume() { --m_nSuspended; }

    void dispose();
};
//------------------------------------------------------------------------------
GridFieldValueListener::GridFieldValueListener(DbGridControl& _rParent, const Reference< XPropertySet >& _rField, sal_uInt16 _nId)
    :OPropertyChangeListener(m_aMutex)
    ,m_rParent(_rParent)
    ,m_pRealListener(NULL)
    ,m_nId(_nId)
    ,m_nSuspended(0)
    ,m_bDisposed(sal_False)
{
    DBG_CTOR(GridFieldValueListener, NULL);
    if (_rField.is())
    {
        m_pRealListener = new ::comphelper::OPropertyChangeMultiplexer(this, _rField);
        m_pRealListener->addProperty(FM_PROP_VALUE);
        m_pRealListener->acquire();
    }
}

//------------------------------------------------------------------------------
GridFieldValueListener::~GridFieldValueListener()
{
    DBG_DTOR(GridFieldValueListener, NULL);
    dispose();
}

//------------------------------------------------------------------------------
void GridFieldValueListener::_propertyChanged(const PropertyChangeEvent& _evt) throw( RuntimeException )
{
    DBG_ASSERT(m_nSuspended>=0, "GridFieldValueListener::_propertyChanged : resume > suspend !");
    if (m_nSuspended <= 0)
        m_rParent.FieldValueChanged(m_nId, _evt);
}

//------------------------------------------------------------------------------
void GridFieldValueListener::dispose()
{
    if (m_bDisposed)
    {
        DBG_ASSERT(m_pRealListener == NULL, "GridFieldValueListener::dispose : inconsistent !");
        return;
    }

    if (m_pRealListener)
    {
        m_pRealListener->dispose();
        m_pRealListener->release();
        m_pRealListener = NULL;
    }

    m_bDisposed = sal_True;
    m_rParent.FieldListenerDisposing(m_nId);
}

//==============================================================================

class DisposeListenerGridBridge : public FmXDisposeListener
{
    osl::Mutex              m_aMutex;
    DbGridControl&          m_rParent;
    FmXDisposeMultiplexer*  m_pRealListener;

public:
    DisposeListenerGridBridge(  DbGridControl& _rParent, const Reference< XComponent >& _rxObject, sal_Int16 _rId = -1);
    virtual ~DisposeListenerGridBridge();

    virtual void disposing(const EventObject& _rEvent, sal_Int16 _nId) throw( RuntimeException ) { m_rParent.disposing(_nId, _rEvent); }
};

//==============================================================================


DBG_NAME(DisposeListenerGridBridge)
//------------------------------------------------------------------------------
DisposeListenerGridBridge::DisposeListenerGridBridge(DbGridControl& _rParent, const Reference< XComponent >& _rxObject, sal_Int16 _rId)
    :FmXDisposeListener(m_aMutex)
    ,m_rParent(_rParent)
    ,m_pRealListener(NULL)
{
    DBG_CTOR(DisposeListenerGridBridge,NULL);

    if (_rxObject.is())
    {
        m_pRealListener = new FmXDisposeMultiplexer(this, _rxObject, _rId);
        m_pRealListener->acquire();
    }
}

//------------------------------------------------------------------------------
DisposeListenerGridBridge::~DisposeListenerGridBridge()
{
    if (m_pRealListener)
    {
        m_pRealListener->dispose();
        m_pRealListener->release();
        m_pRealListener = NULL;
    }

    DBG_DTOR(DisposeListenerGridBridge,NULL);
}

//==============================================================================

static sal_uInt16 ControlMap[] =
    {
        DbGridControl::NavigationBar::RECORD_TEXT,
        DbGridControl::NavigationBar::RECORD_ABSOLUTE,
        DbGridControl::NavigationBar::RECORD_OF,
        DbGridControl::NavigationBar::RECORD_COUNT,
        DbGridControl::NavigationBar::RECORD_FIRST,
        DbGridControl::NavigationBar::RECORD_NEXT,
        DbGridControl::NavigationBar::RECORD_PREV,
        DbGridControl::NavigationBar::RECORD_LAST,
        DbGridControl::NavigationBar::RECORD_NEW,
        0
    };

//------------------------------------------------------------------------------
sal_Bool CompareBookmark(const Any& aLeft, const Any& aRight)
{
    return ::comphelper::compare(aLeft, aRight);
}

//==============================================================================
class FmXGridSourcePropListener : public ::comphelper::OPropertyChangeListener
{
    DbGridControl* m_pParent;

    // a DbGridControl has no mutex, so we use our own as the base class expects one
    osl::Mutex      m_aMutex;
    sal_Int16           m_nSuspended;

public:
    FmXGridSourcePropListener(DbGridControl* _pParent);

    void suspend() { ++m_nSuspended; }
    void resume() { --m_nSuspended; }

    virtual void _propertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException );
};

//------------------------------------------------------------------------------
FmXGridSourcePropListener::FmXGridSourcePropListener(DbGridControl* _pParent)
    :OPropertyChangeListener(m_aMutex)
    ,m_pParent(_pParent)
    ,m_nSuspended(0)
{
    DBG_ASSERT(m_pParent, "FmXGridSourcePropListener::FmXGridSourcePropListener : invalid parent !");
}

//------------------------------------------------------------------------------
void FmXGridSourcePropListener::_propertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException )
{
    DBG_ASSERT(m_nSuspended>=0, "FmXGridSourcePropListener::_propertyChanged : resume > suspend !");
    if (m_nSuspended <= 0)
        m_pParent->DataSourcePropertyChanged(evt);
}

//==============================================================================
//------------------------------------------------------------------------------
DbGridControl::NavigationBar::AbsolutePos::AbsolutePos(Window* pParent, WinBits nStyle)
                   :NumericField(pParent, nStyle)
{
    SetMin(1);
    SetFirst(1);
    SetSpinSize(1);

    SetDecimalDigits(0);
    SetStrictFormat(sal_True);
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::AbsolutePos::KeyInput(const KeyEvent& rEvt)
{
    if (rEvt.GetKeyCode() == KEY_RETURN && GetText().Len())
    {
        sal_Int64 nRecord = GetValue();
        if (nRecord < GetMin() || nRecord > GetMax())
            return;
        else
            ((NavigationBar*)GetParent())->PositionDataSource(static_cast<sal_Int32>(nRecord));
    }
    else if (rEvt.GetKeyCode() == KEY_TAB)
        GetParent()->GetParent()->GrabFocus();
    else
        NumericField::KeyInput(rEvt);
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::AbsolutePos::LoseFocus()
{
    NumericField::LoseFocus();
    sal_Int64 nRecord = GetValue();
    if (nRecord < GetMin() || nRecord > GetMax())
        return;
    else
    {
        ((NavigationBar*)GetParent())->PositionDataSource(static_cast<sal_Int32>(nRecord));
        ((NavigationBar*)GetParent())->InvalidateState(NavigationBar::RECORD_ABSOLUTE);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::PositionDataSource(sal_Int32 nRecord)
{
    if (m_bPositioning)
        return;
    // the MoveToPosition may cause a LoseFocus which would lead to a second MoveToPosition, so protect agains this
    // recursion
    // 68167 - 13.08.99 - FS
    m_bPositioning = sal_True;
    ((DbGridControl*)GetParent())->MoveToPosition(nRecord - 1);
    m_bPositioning = sal_False;
}

//------------------------------------------------------------------------------
DbGridControl::NavigationBar::NavigationBar(Window* pParent, WinBits nStyle)
          :Control(pParent, nStyle)
          ,m_aRecordText(this, WB_VCENTER)
          ,m_aAbsolute(this, WB_VCENTER)
          ,m_aRecordOf(this, WB_VCENTER)
          ,m_aRecordCount(this, WB_CENTER | WB_VCENTER)
          ,m_aFirstBtn(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS)
          ,m_aPrevBtn(this, WB_REPEAT|WB_RECTSTYLE|WB_NOPOINTERFOCUS)
          ,m_aNextBtn(this, WB_REPEAT|WB_RECTSTYLE|WB_NOPOINTERFOCUS)
          ,m_aLastBtn(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS)
          ,m_aNewBtn(this, WB_RECTSTYLE|WB_NOPOINTERFOCUS)
          ,m_nDefaultWidth(0)
          ,m_nCurrentPos(-1)
          ,m_bPositioning(sal_False)
{
    m_aFirstBtn.SetSymbol(SYMBOL_FIRST);
    m_aPrevBtn.SetSymbol(SYMBOL_PREV);
    m_aNextBtn.SetSymbol(SYMBOL_NEXT);
    m_aLastBtn.SetSymbol(SYMBOL_LAST);
    m_aNewBtn.SetModeImage(((DbGridControl*)pParent)->GetImage(DbGridControl_Base::NEW));

    m_aFirstBtn.SetHelpId(HID_GRID_TRAVEL_FIRST);
    m_aPrevBtn.SetHelpId(HID_GRID_TRAVEL_PREV);
    m_aNextBtn.SetHelpId(HID_GRID_TRAVEL_NEXT);
    m_aLastBtn.SetHelpId(HID_GRID_TRAVEL_LAST);
    m_aNewBtn.SetHelpId(HID_GRID_TRAVEL_NEW);
    m_aAbsolute.SetHelpId(HID_GRID_TRAVEL_ABSOLUTE);
    m_aRecordCount.SetHelpId(HID_GRID_NUMBEROFRECORDS);

    // Handler fuer Buttons einrichten
    m_aFirstBtn.SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aPrevBtn.SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aNextBtn.SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aLastBtn.SetClickHdl(LINK(this,NavigationBar,OnClick));
    m_aNewBtn.SetClickHdl(LINK(this,NavigationBar,OnClick));

    m_aRecordText.SetText(XubString(SVX_RES(RID_STR_REC_TEXT)));
    m_aRecordOf.SetText(XubString(SVX_RES(RID_STR_REC_FROM_TEXT)));
    m_aRecordCount.SetText(rtl::OUString('?'));

    m_nDefaultWidth = ArrangeControls();

    m_aFirstBtn.Disable();
    m_aPrevBtn.Disable();
    m_aNextBtn.Disable();
    m_aLastBtn.Disable();
    m_aNewBtn.Disable();
    m_aRecordText.Disable();
    m_aRecordOf.Disable();
    m_aRecordCount.Disable();
    m_aAbsolute.Disable();

    AllSettings aSettings = m_aNextBtn.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 4);
    aSettings.SetMouseSettings(aMouseSettings);
    m_aNextBtn.SetSettings(aSettings, sal_True);
    m_aPrevBtn.SetSettings(aSettings, sal_True);

    m_aFirstBtn.Show();
    m_aPrevBtn.Show();
    m_aNextBtn.Show();
    m_aLastBtn.Show();
    m_aNewBtn.Show();
    m_aRecordText.Show();
    m_aRecordOf.Show();
    m_aRecordCount.Show();
    m_aAbsolute.Show();
}

namespace
{
    void SetPosAndSize(Button& _rButton,Point& _rPos,const Size& _rSize)
    {
        _rButton.SetPosPixel( _rPos );
        _rButton.SetSizePixel( _rSize );
        _rPos.X() += (sal_uInt16)_rSize.Width();
    }
}
//------------------------------------------------------------------------------
sal_uInt16 DbGridControl::NavigationBar::ArrangeControls()
{
    // Positionierung der Controls
    // Basisgroessen ermitteln
    sal_uInt16      nX = 0;
    sal_uInt16      nY = 0;
    Rectangle   aRect(((DbGridControl*)GetParent())->GetControlArea());
    const long  nH      = aRect.GetSize().Height();
    Size        aBorder = LogicToPixel(Size(3, 3),MAP_APPFONT);
                aBorder = Size(CalcZoom(aBorder.Width()), CalcZoom(aBorder.Height()));

    // Controls Groessen und Positionen setzen
    //
    XubString aText    = m_aRecordText.GetText();
    long nTextWidth = m_aRecordText.GetTextWidth(aText);
    m_aRecordText.SetPosPixel(Point(nX,nY) );
    m_aRecordText.SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    m_aAbsolute.SetPosPixel( Point(nX,nY));
    m_aAbsolute.SetSizePixel( Size(3*nH,aRect.GetSize().Height()) ); // Heuristik XXXXXXX
    nX = sal::static_int_cast< sal_uInt16 >(nX + (3*nH) + aBorder.Width());

    aText      = m_aRecordOf.GetText();
    nTextWidth = m_aRecordOf.GetTextWidth(aText);
    m_aRecordOf.SetPosPixel(Point(nX,nY) );
    m_aRecordOf.SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    nTextWidth = m_aRecordCount.GetTextWidth( rtl::OUString("0000000 (00000) *") );
    m_aRecordCount.SetPosPixel(Point(nX,nY) );
    m_aRecordCount.SetSizePixel(Size(nTextWidth,nH));
    nX = sal::static_int_cast< sal_uInt16 >(nX + nTextWidth + aBorder.Width());

    Point aButtonPos(nX,nY);
    Size  aButtonSize(nH,nH);
    SetPosAndSize(m_aFirstBtn, aButtonPos, aButtonSize);
    SetPosAndSize(m_aPrevBtn, aButtonPos, aButtonSize);
    SetPosAndSize(m_aNextBtn, aButtonPos, aButtonSize);
    SetPosAndSize(m_aLastBtn, aButtonPos, aButtonSize);
    SetPosAndSize(m_aNewBtn, aButtonPos, aButtonSize);

    nX = sal::static_int_cast< sal_uInt16 >(
        aButtonPos.X() + (sal_uInt16)(nH + aBorder.Width()));

    // Ist der Font des Edits groesser als das Feld?
    Font aOutputFont = m_aAbsolute.GetFont();
    if (aOutputFont.GetSize().Height() > nH)
    {
        Font aApplFont = OutputDevice::GetDefaultFont(
            DEFAULTFONT_SANS_UNICODE,
            Application::GetSettings().GetUILanguage(),
            DEFAULTFONT_FLAGS_ONLYONE,
            this
        );
        aApplFont.SetSize( Size( 0, nH - 2 ) );
        m_aAbsolute.SetControlFont( aApplFont );

        aApplFont.SetTransparent( sal_True );
        m_aRecordText.SetControlFont( aApplFont );
        m_aRecordOf.SetControlFont( aApplFont );
        m_aRecordCount.SetControlFont( aApplFont );
    }
    return nX;
}

//------------------------------------------------------------------------------
IMPL_LINK(DbGridControl::NavigationBar, OnClick, Button *, pButton )
{
    DbGridControl* pParent = (DbGridControl*)GetParent();

    if (pParent->m_aMasterSlotExecutor.IsSet())
    {
        long lResult = 0;
        if (pButton == &m_aFirstBtn)
            lResult = pParent->m_aMasterSlotExecutor.Call((void*)RECORD_FIRST);
        else if( pButton == &m_aPrevBtn )
            lResult = pParent->m_aMasterSlotExecutor.Call((void*)RECORD_PREV);
        else if( pButton == &m_aNextBtn )
            lResult = pParent->m_aMasterSlotExecutor.Call((void*)RECORD_NEXT);
        else if( pButton == &m_aLastBtn )
            lResult = pParent->m_aMasterSlotExecutor.Call((void*)RECORD_LAST);
        else if( pButton == &m_aNewBtn )
            lResult = pParent->m_aMasterSlotExecutor.Call((void*)RECORD_NEW);

        if (lResult)
            // the link already handled it
            return 0;
    }

    if (pButton == &m_aFirstBtn)
        pParent->MoveToFirst();
    else if( pButton == &m_aPrevBtn )
        pParent->MoveToPrev();
    else if( pButton == &m_aNextBtn )
        pParent->MoveToNext();
    else if( pButton == &m_aLastBtn )
        pParent->MoveToLast();
    else if( pButton == &m_aNewBtn )
        pParent->AppendNew();
    return 0;
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::InvalidateAll(sal_Int32 nCurrentPos, sal_Bool bAll)
{
    if (m_nCurrentPos != nCurrentPos || nCurrentPos < 0 || bAll)
    {
        DbGridControl* pParent = (DbGridControl*)GetParent();

        sal_Int32 nAdjustedRowCount = pParent->GetRowCount() - ((pParent->GetOptions() & DbGridControl::OPT_INSERT) ? 2 : 1);

        // Wann mu� alles invalidiert werden
        bAll = bAll || m_nCurrentPos <= 0;
        bAll = bAll || nCurrentPos <= 0;
        bAll = bAll || m_nCurrentPos >= nAdjustedRowCount;
        bAll = bAll || nCurrentPos >= nAdjustedRowCount;

        if ( bAll )
        {
            m_nCurrentPos = nCurrentPos;
            int i = 0;
            while (ControlMap[i])
                SetState(ControlMap[i++]);
        }
        else    // befindet sich in der Mitte
        {
            m_nCurrentPos = nCurrentPos;
            SetState(NavigationBar::RECORD_COUNT);
            SetState(NavigationBar::RECORD_ABSOLUTE);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::NavigationBar::GetState(sal_uInt16 nWhich) const
{
    DbGridControl* pParent = (DbGridControl*)GetParent();

    if (!pParent->IsOpen() || pParent->IsDesignMode() || !pParent->IsEnabled()
        || pParent->IsFilterMode() )
        return sal_False;
    else
    {
        // check if we have a master state provider
        if (pParent->m_aMasterStateProvider.IsSet())
        {
            long nState = pParent->m_aMasterStateProvider.Call(reinterpret_cast< void* >( nWhich ) );
            if (nState>=0)
                return (nState>0);
        }

        sal_Bool bAvailable = sal_True;

        switch (nWhich)
        {
            case NavigationBar::RECORD_FIRST:
            case NavigationBar::RECORD_PREV:
                bAvailable = m_nCurrentPos > 0;
                break;
            case NavigationBar::RECORD_NEXT:
                if(pParent->m_bRecordCountFinal)
                {
                    bAvailable = m_nCurrentPos < pParent->GetRowCount() - 1;
                    if (!bAvailable && pParent->GetOptions() & DbGridControl::OPT_INSERT)
                        bAvailable = (m_nCurrentPos == pParent->GetRowCount() - 2) && pParent->IsModified();
                }
                break;
            case NavigationBar::RECORD_LAST:
                if(pParent->m_bRecordCountFinal)
                {
                    if (pParent->GetOptions() & DbGridControl::OPT_INSERT)
                        bAvailable = pParent->IsCurrentAppending() ? pParent->GetRowCount() > 1 :
                                     m_nCurrentPos != pParent->GetRowCount() - 2;
                    else
                        bAvailable = m_nCurrentPos != pParent->GetRowCount() - 1;
                }
                break;
            case NavigationBar::RECORD_NEW:
                bAvailable = (pParent->GetOptions() & DbGridControl::OPT_INSERT) && pParent->GetRowCount() && m_nCurrentPos < pParent->GetRowCount() - 1;
                break;
            case NavigationBar::RECORD_ABSOLUTE:
                bAvailable = pParent->GetRowCount() > 0;
                break;
        }
        return bAvailable;
    }
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::SetState(sal_uInt16 nWhich)
{
    sal_Bool bAvailable = GetState(nWhich);
    DbGridControl* pParent = (DbGridControl*)GetParent();
    Window* pWnd = NULL;
    switch (nWhich)
    {
        case NavigationBar::RECORD_FIRST:
            pWnd = &m_aFirstBtn;
            break;
        case NavigationBar::RECORD_PREV:
            pWnd = &m_aPrevBtn;
            break;
        case NavigationBar::RECORD_NEXT:
            pWnd = &m_aNextBtn;
            break;
        case NavigationBar::RECORD_LAST:
            pWnd = &m_aLastBtn;
            break;
        case NavigationBar::RECORD_NEW:
            pWnd = &m_aNewBtn;
            break;
        case NavigationBar::RECORD_ABSOLUTE:
            pWnd = &m_aAbsolute;
            if (bAvailable)
            {
                if (pParent->m_nTotalCount >= 0)
                {
                    if (pParent->IsCurrentAppending())
                        m_aAbsolute.SetMax(pParent->m_nTotalCount + 1);
                    else
                        m_aAbsolute.SetMax(pParent->m_nTotalCount);
                }
                else
                    m_aAbsolute.SetMax(LONG_MAX);

                m_aAbsolute.SetValue(m_nCurrentPos + 1);
            }
            else
                m_aAbsolute.SetText(String());
            break;
        case NavigationBar::RECORD_TEXT:
            pWnd = &m_aRecordText;
            break;
        case NavigationBar::RECORD_OF:
            pWnd = &m_aRecordOf;
            break;
        case NavigationBar::RECORD_COUNT:
        {
            pWnd = &m_aRecordCount;
            String aText;
            if (bAvailable)
            {
                if (pParent->GetOptions() & DbGridControl::OPT_INSERT)
                {
                    if (pParent->IsCurrentAppending() && !pParent->IsModified())
                        aText = String::CreateFromInt32(pParent->GetRowCount());
                    else
                        aText = String::CreateFromInt32(pParent->GetRowCount() - 1);
                }
                else
                    aText = String::CreateFromInt32(pParent->GetRowCount());
                if(!pParent->m_bRecordCountFinal)
                    aText += rtl::OUString(" *");
            }
            else
                aText = String();

            // add the number of selected rows, if applicable
            if (pParent->GetSelectRowCount())
            {
                String aExtendedInfo(aText);
                aExtendedInfo.AppendAscii(" (");
                aExtendedInfo += String::CreateFromInt32(pParent->GetSelectRowCount());
                aExtendedInfo += ')';
                pWnd->SetText(aExtendedInfo);
            }
            else
                pWnd->SetText(aText);

            pParent->SetRealRowCount(aText);
        }   break;
    }
    DBG_ASSERT(pWnd, "kein Fenster");
    if (pWnd && (pWnd->IsEnabled() != bAvailable))
        // this "pWnd->IsEnabled() != bAvailable" is a little hack : Window::Enable always generates a user
        // event (ImplGenerateMouseMove) even if nothing happened. This may lead to some unwanted effects, so we
        // do this check.
        // For further explanation see Bug 69900.
        pWnd->Enable(bAvailable);
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::Resize()
{
    Control::Resize();
    ArrangeControls();
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::Paint(const Rectangle& rRect)
{
    Control::Paint(rRect);
    Point aAbsolutePos = m_aAbsolute.GetPosPixel();
    Size  aAbsoluteSize = m_aAbsolute.GetSizePixel();

    DrawLine(Point(aAbsolutePos.X() - 1, 0 ),
             Point(aAbsolutePos.X() - 1, aAbsolutePos.Y() + aAbsoluteSize.Height()));

    DrawLine(Point(aAbsolutePos.X() + aAbsoluteSize.Width() + 1, 0 ),
             Point(aAbsolutePos.X() + aAbsoluteSize.Width() + 1, aAbsolutePos.Y() + aAbsoluteSize.Height()));
}

//------------------------------------------------------------------------------
void DbGridControl::NavigationBar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    Window* pWindows[] = {  &m_aRecordText,
                            &m_aAbsolute,
                            &m_aRecordOf,
                            &m_aRecordCount,
                            &m_aFirstBtn,
                            &m_aPrevBtn,
                            &m_aNextBtn,
                            &m_aLastBtn,
                            &m_aNewBtn
                        };

    switch ( nType )
    {
        case STATE_CHANGE_MIRRORING:
        {
            sal_Bool bIsRTLEnabled = IsRTLEnabled();
            for ( size_t i=0; i < (sizeof (pWindows) / sizeof(pWindows[0])); ++i )
                pWindows[i]->EnableRTL( bIsRTLEnabled );
        }
        break;

        case STATE_CHANGE_ZOOM:
        {
            Fraction aZoom = GetZoom();

            // not all of these controls need to know the new zoom, but to be sure ...
            Font aFont( GetSettings().GetStyleSettings().GetFieldFont() );
            if ( IsControlFont() )
                aFont.Merge( GetControlFont() );

            for (size_t i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
            {
                pWindows[i]->SetZoom(aZoom);
                pWindows[i]->SetZoomedPointFont(aFont);
            }

            SetZoomedPointFont( aFont );

            // rearrange the controls
            m_nDefaultWidth = ArrangeControls();
        }
        break;
    }
}

//------------------------------------------------------------------------------
DbGridRow::DbGridRow(CursorWrapper* pCur, sal_Bool bPaintCursor)
          :m_bIsNew(sal_False)
{

    if (pCur && pCur->Is())
    {
        Reference< XIndexAccess >  xColumns(pCur->getColumns(), UNO_QUERY);
        DataColumn* pColumn;
        for (sal_Int32 i = 0; i < xColumns->getCount(); ++i)
        {
            Reference< XPropertySet > xColSet;
            ::cppu::extractInterface(xColSet, xColumns->getByIndex(i));
            pColumn = new DataColumn(xColSet);
            m_aVariants.push_back( pColumn );
        }

        if (pCur->rowDeleted())
            m_eStatus = GRS_DELETED;
        else
        {
            if (bPaintCursor)
                m_eStatus = (pCur->isAfterLast() || pCur->isBeforeFirst()) ? GRS_INVALID : GRS_CLEAN;
            else
            {
                Reference< XPropertySet > xSet = pCur->getPropertySet();
                if (xSet.is())
                {
                    m_bIsNew = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
                    if (!m_bIsNew && (pCur->isAfterLast() || pCur->isBeforeFirst()))
                        m_eStatus = GRS_INVALID;
                    else if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                        m_eStatus = GRS_MODIFIED;
                    else
                        m_eStatus = GRS_CLEAN;
                }
                else
                    m_eStatus = GRS_INVALID;
            }
        }
        if (!m_bIsNew && IsValid())
            m_aBookmark = pCur->getBookmark();
        else
            m_aBookmark = Any();
    }
    else
        m_eStatus = GRS_INVALID;
}

//------------------------------------------------------------------------------
DbGridRow::~DbGridRow()
{
    for ( size_t i = 0, n = m_aVariants.size(); i < n; ++i )
        delete m_aVariants[ i ];
    m_aVariants.clear();
}

//------------------------------------------------------------------------------
void DbGridRow::SetState(CursorWrapper* pCur, sal_Bool bPaintCursor)
{
    if (pCur && pCur->Is())
    {
        if (pCur->rowDeleted())
        {
            m_eStatus = GRS_DELETED;
            m_bIsNew = sal_False;
        }
        else
        {
            m_eStatus = GRS_CLEAN;
            if (!bPaintCursor)
            {
                Reference< XPropertySet > xSet = pCur->getPropertySet();
                DBG_ASSERT(xSet.is(), "DbGridRow::SetState : invalid cursor !");

                if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                    m_eStatus = GRS_MODIFIED;
                m_bIsNew = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
            }
            else
                m_bIsNew = sal_False;
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
            DBG_UNHANDLED_EXCEPTION();
            m_aBookmark = Any();
            m_eStatus = GRS_INVALID;
            m_bIsNew = sal_False;
        }
    }
    else
    {
        m_aBookmark = Any();
        m_eStatus = GRS_INVALID;
        m_bIsNew = sal_False;
    }
}

DBG_NAME(DbGridControl);
//------------------------------------------------------------------------------
DbGridControl::DbGridControl(
                Reference< XMultiServiceFactory > _rxFactory,
                Window* pParent,
                WinBits nBits)
            :DbGridControl_Base(pParent, EBBF_NONE, nBits, DEFAULT_BROWSE_MODE )
            ,m_xServiceFactory(_rxFactory)
            ,m_aBar(this)
            ,m_nAsynAdjustEvent(0)
            ,m_pDataSourcePropMultiplexer(NULL)
            ,m_pDataSourcePropListener(NULL)
            ,m_pFieldListeners(NULL)
            ,m_pCursorDisposeListener(NULL)
            ,m_pGridListener(NULL)
            ,m_pDataCursor(NULL)
            ,m_pSeekCursor(NULL)
            ,m_nSeekPos(-1)
            ,m_nTotalCount(-1)
            ,m_aNullDate(OTypeConversionClient().getStandardDate())
            ,m_nMode(DEFAULT_BROWSE_MODE)
            ,m_nCurrentPos(-1)
            ,m_nDeleteEvent(0)
            ,m_nOptions(OPT_READONLY)
            ,m_nOptionMask(OPT_INSERT | OPT_UPDATE | OPT_DELETE)
            ,m_nLastColId((sal_uInt16)-1)
            ,m_nLastRowId(-1)
            ,m_bDesignMode(sal_False)
            ,m_bRecordCountFinal(sal_False)
            ,m_bMultiSelection(sal_True)
            ,m_bNavigationBar(sal_True)
            ,m_bSynchDisplay(sal_True)
            ,m_bForceROController(sal_False)
            ,m_bHandle(sal_True)
            ,m_bFilterMode(sal_False)
            ,m_bWantDestruction(sal_False)
            ,m_bInAdjustDataSource(sal_False)
            ,m_bPendingAdjustRows(sal_False)
            ,m_bHideScrollbars( sal_False )
            ,m_bUpdating(sal_False)
{
    DBG_CTOR(DbGridControl,NULL);

    String sName(SVX_RES(RID_STR_NAVIGATIONBAR));
    m_aBar.SetAccessibleName(sName);
    m_aBar.Show();
    ImplInitWindow( InitAll );
}

//------------------------------------------------------------------------------
void DbGridControl::InsertHandleColumn()
{
    // Handle Column einfuegen
    // Da die BrowseBox ohne handleColums Paintprobleme hat
    // wird diese versteckt
    if (HasHandle())
        BrowseBox::InsertHandleColumn(GetDefaultColumnWidth(String()));
    else
        BrowseBox::InsertHandleColumn(0);
}

//------------------------------------------------------------------------------
void DbGridControl::Init()
{
    BrowserHeader* pNewHeader = CreateHeaderBar(this);
    pHeader->SetMouseTransparent(sal_False);

    SetHeaderBar(pNewHeader);
    SetMode(m_nMode);
    SetCursorColor(Color(0xFF, 0, 0));

    InsertHandleColumn();
}

//------------------------------------------------------------------------------
DbGridControl::~DbGridControl()
{
    RemoveColumns();

    {
        m_bWantDestruction = sal_True;
        osl::MutexGuard aGuard(m_aDestructionSafety);
        if (m_pFieldListeners)
            DisconnectFromFields();
        if (m_pCursorDisposeListener)
        {
            delete m_pCursorDisposeListener;
            m_pCursorDisposeListener = NULL;
        }
    }

    if (m_nDeleteEvent)
        Application::RemoveUserEvent(m_nDeleteEvent);

    if (m_pDataSourcePropMultiplexer)
    {
        m_pDataSourcePropMultiplexer->dispose();
        m_pDataSourcePropMultiplexer->release();    // this should delete the multiplexer
        delete m_pDataSourcePropListener;
        m_pDataSourcePropMultiplexer = NULL;
        m_pDataSourcePropListener = NULL;
    }
    m_xRowSetListener.clear();

    delete m_pDataCursor;
    delete m_pSeekCursor;

    DBG_DTOR(DbGridControl,NULL);
}

//------------------------------------------------------------------------------
void DbGridControl::StateChanged( StateChangedType nType )
{
    DbGridControl_Base::StateChanged( nType );

    switch (nType)
    {
        case STATE_CHANGE_MIRRORING:
            ImplInitWindow( InitWritingMode );
            Invalidate();
            break;

        case STATE_CHANGE_ZOOM:
        {
            ImplInitWindow( InitFont );

            // and give it a chance to rearrange
            Point aPoint = GetControlArea().TopLeft();
            sal_uInt16 nX = (sal_uInt16)aPoint.X();
            ArrangeControls(nX, (sal_uInt16)aPoint.Y());
            ReserveControlArea((sal_uInt16)nX);
        }
        break;
        case STATE_CHANGE_CONTROLFONT:
            ImplInitWindow( InitFont );
            Invalidate();
            break;
        case STATE_CHANGE_CONTROLFOREGROUND:
            ImplInitWindow( InitForeground );
            Invalidate();
            break;
        case STATE_CHANGE_CONTROLBACKGROUND:
            ImplInitWindow( InitBackground );
            Invalidate();
            break;
    }
}

//------------------------------------------------------------------------------
void DbGridControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    DbGridControl_Base::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS ) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitWindow( InitAll );
        Invalidate();
    }
}

//------------------------------------------------------------------------------
void DbGridControl::Select()
{
    DbGridControl_Base::Select();

    // as the selected rows may have changed, udate the according display in our navigation bar
    m_aBar.InvalidateState(NavigationBar::RECORD_COUNT);

    if (m_pGridListener)
        m_pGridListener->selectionChanged();
}

//------------------------------------------------------------------------------
void DbGridControl::ImplInitWindow( const InitWindowFacet _eInitWhat )
{
    for ( size_t i = 0; i < m_aColumns.size(); ++i )
    {
        DbGridColumn* pCol = m_aColumns[ i ];
        if (pCol)
            pCol->ImplInitWindow( GetDataWindow(), _eInitWhat );
    }

    if ( ( _eInitWhat & InitWritingMode ) != 0 )
    {
        if ( m_bNavigationBar )
        {
            m_aBar.EnableRTL( IsRTLEnabled() );
        }
    }

    if ( ( _eInitWhat & InitFont ) != 0 )
    {
        if ( m_bNavigationBar )
        {
            Font aFont = m_aBar.GetSettings().GetStyleSettings().GetFieldFont();
            if ( IsControlFont() )
                m_aBar.SetControlFont( GetControlFont() );
            else
                m_aBar.SetControlFont();

            m_aBar.SetZoom( GetZoom() );
        }
    }

    if ( ( _eInitWhat & InitBackground ) != 0 )
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

//------------------------------------------------------------------------------
void DbGridControl::RemoveRows(sal_Bool bNewCursor)
{
    // Hat sich der DatenCursor verandert ?
    if (!bNewCursor)
    {
        DELETEZ(m_pSeekCursor);
        m_xPaintRow = m_xDataRow = m_xEmptyRow  = m_xCurrentRow = m_xSeekRow = NULL;
        m_nCurrentPos = m_nSeekPos = -1;
        m_nOptions  = OPT_READONLY;

        RowRemoved(0, GetRowCount(), sal_False);
        m_nTotalCount = -1;
    }
    else
    {
        RemoveRows();
    }
}

//------------------------------------------------------------------------------
void DbGridControl::RemoveRows()
{
    // we're going to remove all columns and all row, so deactivate the current cell
    if (IsEditing())
        DeactivateCell();

    // alle Columns deinitialisieren
    // existieren Spalten, dann alle Controller freigeben
    for (size_t i = 0; i < m_aColumns.size(); i++)
        m_aColumns[ i ]->Clear();

    DELETEZ(m_pSeekCursor);
    DELETEZ(m_pDataCursor);

    m_xPaintRow = m_xDataRow = m_xEmptyRow  = m_xCurrentRow = m_xSeekRow = NULL;
    m_nCurrentPos = m_nSeekPos = m_nTotalCount  = -1;
    m_nOptions  = OPT_READONLY;

    // Anzahl Saetze im Browser auf 0 zuruecksetzen
    DbGridControl_Base::RemoveRows();
    m_aBar.InvalidateAll(m_nCurrentPos, sal_True);
}

//------------------------------------------------------------------------------
void DbGridControl::ArrangeControls(sal_uInt16& nX, sal_uInt16 nY)
{
    // Positionierung der Controls
    if (m_bNavigationBar)
    {
        nX = m_aBar.GetDefaultWidth();
        Rectangle   aRect(GetControlArea());
        m_aBar.SetPosSizePixel(Point(0,nY + 1), Size(nX, aRect.GetSize().Height() - 1));
    }
}

//------------------------------------------------------------------------------
void DbGridControl::EnableHandle(sal_Bool bEnable)
{
    if (m_bHandle == bEnable)
        return;

    // HandleColumn wird nur ausgeblendet,
    // da es sonst etliche Probleme mit dem Zeichnen gibt
    RemoveColumn( HandleColumnId );
    m_bHandle = bEnable;
    InsertHandleColumn();
}

//------------------------------------------------------------------------------
namespace
{
    bool adjustModeForScrollbars( BrowserMode& _rMode, sal_Bool _bNavigationBar, sal_Bool _bHideScrollbars )
    {
        BrowserMode nOldMode = _rMode;

        if ( !_bNavigationBar )
        {
            _rMode &= ~BROWSER_AUTO_HSCROLL;
        }

        if ( _bHideScrollbars )
        {
            _rMode |= ( BROWSER_NO_HSCROLL | BROWSER_NO_VSCROLL );
            _rMode &= ~( BROWSER_AUTO_HSCROLL | BROWSER_AUTO_VSCROLL );
        }
        else
        {
            _rMode |= ( BROWSER_AUTO_HSCROLL | BROWSER_AUTO_VSCROLL );
            _rMode &= ~( BROWSER_NO_HSCROLL | BROWSER_NO_VSCROLL );
        }

        // note: if we have a navigation bar, we always have a AUTO_HSCROLL. In particular,
        // _bHideScrollbars is ignored then
        if ( _bNavigationBar )
        {
            _rMode |= BROWSER_AUTO_HSCROLL;
            _rMode &= ~BROWSER_NO_HSCROLL;
        }

        return nOldMode != _rMode;
    }
}

//------------------------------------------------------------------------------
void DbGridControl::EnableNavigationBar(sal_Bool bEnable)
{
    if (m_bNavigationBar == bEnable)
        return;

    m_bNavigationBar = bEnable;

    if (bEnable)
    {
        m_aBar.Show();
        m_aBar.Enable();
        m_aBar.InvalidateAll(m_nCurrentPos, sal_True);

        if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
            SetMode( m_nMode );

        // liefert die Groe�e der Reserved ControlArea
        Point aPoint = GetControlArea().TopLeft();
        sal_uInt16 nX = (sal_uInt16)aPoint.X();

        ArrangeControls(nX, (sal_uInt16)aPoint.Y());
        ReserveControlArea((sal_uInt16)nX);
    }
    else
    {
        m_aBar.Hide();
        m_aBar.Disable();

        if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
            SetMode( m_nMode );

        ReserveControlArea();
    }
}

//------------------------------------------------------------------------------
sal_uInt16 DbGridControl::SetOptions(sal_uInt16 nOpt)
{
    DBG_ASSERT(!m_xCurrentRow || !m_xCurrentRow->IsModified(),
        "DbGridControl::SetOptions : please do not call when editing a record (things are much easier this way ;) !");

    // for the next setDataSource (which is triggered by a refresh, for instance)
    m_nOptionMask = nOpt;

    // normalize the new options
    Reference< XPropertySet > xDataSourceSet = m_pDataCursor->getPropertySet();
    if (xDataSourceSet.is())
    {
        // feststellen welche Updatem�glichkeiten bestehen
        sal_Int32 nPrivileges = 0;
        xDataSourceSet->getPropertyValue(FM_PROP_PRIVILEGES) >>= nPrivileges;
        if ((nPrivileges & Privilege::INSERT) == 0)
            nOpt &= ~OPT_INSERT;
        if ((nPrivileges & Privilege::UPDATE) == 0)
            nOpt &= ~OPT_UPDATE;
        if ((nPrivileges & Privilege::DELETE) == 0)
            nOpt &= ~OPT_DELETE;
    }
    else
        nOpt = OPT_READONLY;

    // need to do something after that ?
    if (nOpt == m_nOptions)
        return m_nOptions;

    // the 'update' option only affects our BrowserMode (with or w/o focus rect)
    BrowserMode nNewMode = m_nMode;
    if ((m_nMode & BROWSER_CURSOR_WO_FOCUS) == 0)
    {
        if (nOpt & OPT_UPDATE)
            nNewMode |= BROWSER_HIDECURSOR;
        else
            nNewMode &= ~BROWSER_HIDECURSOR;
    }
    else
        nNewMode &= ~BROWSER_HIDECURSOR;
        // should not be neccessary if EnablePermanentCursor is used to change the cursor behaviour, but to be sure ...

    if (nNewMode != m_nMode)
    {
        SetMode(nNewMode);
        m_nMode = nNewMode;
    }

    // _after_ setting the mode because this results in an ActivateCell
    DeactivateCell();

    sal_Bool bInsertChanged = (nOpt & OPT_INSERT) != (m_nOptions & OPT_INSERT);
    m_nOptions = nOpt;
        // we need to set this before the code below because it indirectly uses m_nOptions

    // the 'insert' option affects our empty row
    if (bInsertChanged)
    {
        if (m_nOptions & OPT_INSERT)
        {   // the insert option is to be set
            m_xEmptyRow = new DbGridRow();
            RowInserted(GetRowCount(), 1, sal_True);
        }
        else
        {   // the insert option is to be reset
            m_xEmptyRow = NULL;
            if ((GetCurRow() == GetRowCount() - 1) && (GetCurRow() > 0))
                GoToRowColumnId(GetCurRow() - 1, GetCurColumnId());
            RowRemoved(GetRowCount(), 1, sal_True);
        }
    }

    // the 'delete' options has no immediate consequences

    ActivateCell();
    Invalidate();
    return m_nOptions;
}

//------------------------------------------------------------------------------
void DbGridControl::ForceHideScrollbars( sal_Bool _bForce )
{
    if ( m_bHideScrollbars == _bForce )
        return;

    m_bHideScrollbars = _bForce;

    if ( adjustModeForScrollbars( m_nMode, m_bNavigationBar, m_bHideScrollbars ) )
        SetMode( m_nMode );
}

//------------------------------------------------------------------------------
void DbGridControl::EnablePermanentCursor(sal_Bool bEnable)
{
    if (IsPermanentCursorEnabled() == bEnable)
        return;

    if (bEnable)
    {
        m_nMode &= ~BROWSER_HIDECURSOR;     // without this BROWSER_CURSOR_WO_FOCUS won't have any affect
        m_nMode |= BROWSER_CURSOR_WO_FOCUS;
    }
    else
    {
        if (m_nOptions & OPT_UPDATE)
            m_nMode |= BROWSER_HIDECURSOR;      // no cursor at all
        else
            m_nMode &= ~BROWSER_HIDECURSOR;     // at least the "non-permanent" cursor

        m_nMode &= ~BROWSER_CURSOR_WO_FOCUS;
    }
    SetMode(m_nMode);

    sal_Bool bWasEditing = IsEditing();
    DeactivateCell();
    if (bWasEditing)
        ActivateCell();
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::IsPermanentCursorEnabled() const
{
    return ((m_nMode & BROWSER_CURSOR_WO_FOCUS) != 0) && ((m_nMode & BROWSER_HIDECURSOR) == 0);
}

//------------------------------------------------------------------------------
void DbGridControl::refreshController(sal_uInt16 _nColId, GrantControlAccess /*_aAccess*/)
{
    if ((GetCurColumnId() == _nColId) && IsEditing())
    {   // the controller which is currently active needs to be refreshed
        DeactivateCell();
        ActivateCell();
    }
}

//------------------------------------------------------------------------------
void DbGridControl::setDataSource(const Reference< XRowSet >& _xCursor, sal_uInt16 nOpts)
{
    if (!_xCursor.is() && !m_pDataCursor)
        return;

    if (m_pDataSourcePropMultiplexer)
    {
        m_pDataSourcePropMultiplexer->dispose();
        m_pDataSourcePropMultiplexer->release();    // this should delete the multiplexer
        delete m_pDataSourcePropListener;
        m_pDataSourcePropMultiplexer = NULL;
        m_pDataSourcePropListener = NULL;
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

    // Hat sich der DatenCursor verandert ?
    sal_uInt16 nCurPos = GetColumnPos(GetCurColumnId());

    SetUpdateMode(sal_False);
    RemoveRows();
    DisconnectFromFields();

    DELETEZ(m_pCursorDisposeListener);

    {
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        if (m_nAsynAdjustEvent)
        {
            // the adjust was thought to work with the old cursor which we don't have anymore
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = 0;
        }
    }

    // get a new formatter and data cursor
    m_xFormatter = NULL;
    OStaticDataAccessTools aStaticTools;
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = aStaticTools.getNumberFormats(aStaticTools.getRowSetConnection(_xCursor), sal_True);
    if (xSupplier.is() && m_xServiceFactory.is())
    {
        m_xFormatter =  Reference< ::com::sun::star::util::XNumberFormatter >(
            m_xServiceFactory->createInstance(FM_NUMBER_FORMATTER),
            UNO_QUERY);
        if (m_xFormatter.is())
        {
            m_xFormatter->attachNumberFormatsSupplier(xSupplier);

            // retrieve the datebase of the Numberformatter
            try
            {
                xSupplier->getNumberFormatSettings()->getPropertyValue(rtl::OUString("NullDate")) >>= m_aNullDate;
            }
            catch(Exception&)
            {
            }
        }
    }

    m_pDataCursor = new CursorWrapper(_xCursor);

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
        m_pSeekCursor = new CursorWrapper(xClone);

    // property listening on the data source
    // (Normally one class would be sufficient : the multiplexer which could forward the property change to us.
    // But for that we would have been derived from ::comphelper::OPropertyChangeListener, which isn't exported.
    // So we introduce a second class, which is a ::comphelper::OPropertyChangeListener (in the implementation file we know this class)
    // and forwards the property changes to a our special method "DataSourcePropertyChanged".)
    if (m_pDataCursor)
    {
        m_pDataSourcePropListener = new FmXGridSourcePropListener(this);
        m_pDataSourcePropMultiplexer = new ::comphelper::OPropertyChangeMultiplexer(m_pDataSourcePropListener, m_pDataCursor->getPropertySet() );
        m_pDataSourcePropMultiplexer->acquire();
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
                // feststellen welche Updatemoeglichkeiten bestehen
                sal_Int32 nConcurrency = ResultSetConcurrency::READ_ONLY;
                xSet->getPropertyValue(FM_PROP_RESULTSET_CONCURRENCY) >>= nConcurrency;

                if ( ResultSetConcurrency::UPDATABLE == nConcurrency )
                {
                    sal_Int32 nPrivileges = 0;
                    xSet->getPropertyValue(FM_PROP_PRIVILEGES) >>= nPrivileges;

                    // Insert Option should be set if insert only otherwise you won't see any rows
                    // and no insertion is possible
                    if ((m_nOptionMask & OPT_INSERT) && ((nPrivileges & Privilege::INSERT) == Privilege::INSERT) && (nOpts & OPT_INSERT))
                        m_nOptions |= OPT_INSERT;
                    if ((m_nOptionMask & OPT_UPDATE) && ((nPrivileges & Privilege::UPDATE) == Privilege::UPDATE) && (nOpts & OPT_UPDATE))
                        m_nOptions |= OPT_UPDATE;
                    if ((m_nOptionMask & OPT_DELETE) && ((nPrivileges & Privilege::DELETE) == Privilege::DELETE) && (nOpts & OPT_DELETE))
                        m_nOptions |= OPT_DELETE;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        sal_Bool bPermanentCursor = IsPermanentCursorEnabled();
        m_nMode = DEFAULT_BROWSE_MODE;

        if ( bPermanentCursor )
        {
            m_nMode |= BROWSER_CURSOR_WO_FOCUS;
            m_nMode &= ~BROWSER_HIDECURSOR;
        }
        else
        {
            // Duerfen Updates gemacht werden, kein Focus-RechtEck
            if ( m_nOptions & OPT_UPDATE )
                m_nMode |= BROWSER_HIDECURSOR;
        }

        if ( m_bMultiSelection )
            m_nMode |= BROWSER_MULTISELECTION;
        else
            m_nMode &= ~BROWSER_MULTISELECTION;

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
        if (m_nOptions & OPT_INSERT)
        {
            // insert the empty row for insertion
            m_xEmptyRow = new DbGridRow();
            ++nRecordCount;
        }
        if (nRecordCount)
        {
            m_xPaintRow = m_xSeekRow = new DbGridRow(m_pSeekCursor, sal_True);
            m_xDataRow  = new DbGridRow(m_pDataCursor, sal_False);
            RowInserted(0, nRecordCount, sal_False);

            if (m_xSeekRow->IsValid())
                try
                {
                    m_nSeekPos = m_pSeekCursor->getRow() - 1;
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                    m_nSeekPos = -1;
                }
        }
        else
        {
            // no rows so we don't need a seekcursor
            DELETEZ(m_pSeekCursor);
        }
    }

    // Zur alten Spalte gehen
    if (nCurPos == BROWSER_INVALIDID || nCurPos >= ColCount())
        nCurPos = 0;

    // there are rows so go to the selected current column
    if (nRecordCount)
        GoToRowColumnId(0, GetColumnId(nCurPos));
    // else stop the editing if neccessary
    else if (IsEditing())
        DeactivateCell();

    // now reset the mode
    if (m_nMode != nOldMode)
        SetMode(m_nMode);

    // beim Resizen wird RecalcRows gerufen
    if (!IsResizing() && GetRowCount())
        RecalcRows(GetTopRow(), GetVisibleRows(), sal_True);

    m_aBar.InvalidateAll(m_nCurrentPos, sal_True);
    SetUpdateMode(sal_True);

    // start listening on the seek cursor
    if (m_pSeekCursor)
        m_pCursorDisposeListener = new DisposeListenerGridBridge(*this, Reference< XComponent > ((Reference< XInterface >)*m_pSeekCursor, UNO_QUERY), 0);
}

//------------------------------------------------------------------------------
void DbGridControl::RemoveColumns()
{
    if ( IsEditing() )
        DeactivateCell();

    for (size_t i = 0, n = m_aColumns.size(); i < n; i++)
        delete m_aColumns[ i ];
    m_aColumns.clear();

    DbGridControl_Base::RemoveColumns();
}

//------------------------------------------------------------------------------
DbGridColumn* DbGridControl::CreateColumn(sal_uInt16 nId) const
{
    return new DbGridColumn(nId, *(DbGridControl*)this);
}

//------------------------------------------------------------------------------
sal_uInt16 DbGridControl::AppendColumn(const XubString& rName, sal_uInt16 nWidth, sal_uInt16 nModelPos, sal_uInt16 nId)
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
    for (nId=1; (GetModelColumnPos(nId) != GRID_COLUMN_NOT_FOUND) && (nId <= m_aColumns.size()); ++nId)
        ;
    DBG_ASSERT(GetViewColumnPos(nId) == GRID_COLUMN_NOT_FOUND, "DbGridControl::AppendColumn : inconsistent internal state !");
        // my column's models say "there is no column with id nId", but the view (the base class) says "there is a column ..."

    DbGridControl_Base::AppendColumn(rName, nWidth, nRealPos, nId);
    if (nModelPos == HEADERBAR_APPEND)
        m_aColumns.push_back( CreateColumn(nId) );
    else
    {
        DbGridColumns::iterator it = m_aColumns.begin();
        ::std::advance( it, nModelPos );
        m_aColumns.insert( it, CreateColumn(nId) );
    }

    return nId;
}

//------------------------------------------------------------------------------
void DbGridControl::RemoveColumn(sal_uInt16 nId)
{
    DbGridControl_Base::RemoveColumn(nId);

    const sal_uInt16 nIndex = GetModelColumnPos(nId);
    if(nIndex != GRID_COLUMN_NOT_FOUND)
    {
        delete m_aColumns[nIndex];
        m_aColumns.erase( m_aColumns.begin()+nIndex );
    }
}

//------------------------------------------------------------------------------
void DbGridControl::ColumnMoved(sal_uInt16 nId)
{
    DbGridControl_Base::ColumnMoved(nId);

    // remove the col from the model
    sal_uInt16 nOldModelPos = GetModelColumnPos(nId);
#ifdef DBG_UTIL
    DbGridColumn* pCol = m_aColumns[ (sal_uInt32)nOldModelPos ];
    DBG_ASSERT(!pCol->IsHidden(), "DbGridControl::ColumnMoved : moved a hidden col ? how this ?");
#endif

    // for the new model pos we can't use GetModelColumnPos because we are altering the model at the moment
    // so the method won't work (in fact it would return the old model pos)

    // the new view pos is calculated easily
    sal_uInt16 nNewViewPos = GetViewColumnPos(nId);

    // from that we can compute the new model pos
    sal_uInt16 nNewModelPos;
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
    //
    // for instance, let's look at a grid with six columns where the third one is hidden. this will
    // initially look like this :
    //
    //              +---+---+---+---+---+---+
    // model pos    | 0 | 1 |*2*| 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 2 | 3 | 4 | 5 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 1 | - | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+
    //
    // if we move the column at (view) pos 1 to (view) pos 3 we have :
    //
    //              +---+---+---+---+---+---+
    // model pos    | 0 | 3 |*2*| 4 | 1 | 5 |   // not reflecting the changes, yet
    //              +---+---+---+---+---+---+
    // ID           | 1 | 4 | 3 | 5 | 2 | 6 |   // already reflecting the changes
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 1 | - | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+
    //
    // or, sorted by the out-of-date model positions :
    //
    //              +---+---+---+---+---+---+
    // model pos    | 0 | 1 |*2*| 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 2 | 3 | 4 | 5 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | 3 | - | 1 | 2 | 4 |
    //              +---+---+---+---+---+---+
    //
    // We know the new view pos (3) of the moved column because our base class tells us. So we look at our
    // model for the 4th (the pos is zero-based) visible column, it is at (model) position 4. And this is
    // exactly the pos where we have to re-insert our column's model, so it looks ike this :
    //
    //              +---+---+---+---+---+---+
    // model pos    | 0 |*1*| 2 | 3 | 4 | 5 |
    //              +---+---+---+---+---+---+
    // ID           | 1 | 3 | 4 | 5 | 2 | 6 |
    //              +---+---+---+---+---+---+
    // view pos     | 0 | - | 1 | 2 | 3 | 4 |
    //              +---+---+---+---+---+---+
    //
    // Now, all is consistent again.
    // (except of the hidden column : The cycling of the cols occurred on the model, not on the view. maybe
    // the user expected the latter but there really is no good argument against our method ;) ...)
    //
    // And no, this large explanation isn't just because I wanted to play a board game or something like
    // that. It's because it took me a while to see it myself, and the whole theme (hidden cols, model col
    // positions, view col positions)  is really painful (at least for me) so the above pictures helped me a lot ;)


    DbGridColumn* temp = m_aColumns[ nOldModelPos ];

    DbGridColumns::iterator it = m_aColumns.begin();
    ::std::advance( it, nOldModelPos );
    m_aColumns.erase( it );

    it = m_aColumns.begin();
    ::std::advance( it, nNewModelPos );
    m_aColumns.insert( it, temp );
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::SeekRow(long nRow)
{
    // in filter mode or in insert only mode we don't have any cursor!
    if ( !SeekCursor( nRow ) )
        return sal_False;

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
            m_xSeekRow->SetState( m_pSeekCursor, sal_True );
            m_xPaintRow = m_xSeekRow;
        }
    }

    DbGridControl_Base::SeekRow(nRow);

    return m_nSeekPos >= 0;
}
//------------------------------------------------------------------------------
// Wird aufgerufen, wenn die dargestellte Datenmenge sich aendert
//------------------------------------------------------------------------------
void DbGridControl::VisibleRowsChanged( long nNewTopRow, sal_uInt16 nLinesOnScreen )
{
    RecalcRows(nNewTopRow, nLinesOnScreen , sal_False);
}

//------------------------------------------------------------------------------
void DbGridControl::RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, sal_Bool bUpdateCursor)
{
    DBG_CHKTHIS( DbGridControl, NULL );
    // Wenn kein Cursor -> keine Rows im Browser.
    if (!m_pSeekCursor)
    {
        DBG_ASSERT(GetRowCount() == 0,"DbGridControl: ohne Cursor darf es keine Rows geben");
        return;
    }

    // ignore any updates implicit made
    sal_Bool bDisablePaint = !bUpdateCursor && IsPaintEnabled();
    if (bDisablePaint)
        EnablePaint(sal_False);

    // Cache an den sichtbaren Bereich anpassen
    Reference< XPropertySet > xSet = m_pSeekCursor->getPropertySet();
    sal_Int32 nCacheSize = 0;
    xSet->getPropertyValue(FM_PROP_FETCHSIZE) >>= nCacheSize;
    sal_Bool bCacheAligned   = sal_False;
    // Nach der Initialisierung (m_nSeekPos < 0) keine Cursorbewegung, da bereits auf den ersten
    // Satz positioniert
    long nDelta = nNewTopRow - GetTopRow();
    // Limit fuer relative Positionierung
    long nLimit = (nCacheSize) ? nCacheSize / 2 : 0;

    // mehr Zeilen auf dem Bildschirm als im Cache
    if (nLimit < nLinesOnScreen)
    {
        Any aCacheSize;
        aCacheSize <<= sal_Int32(nLinesOnScreen*2);
        xSet->setPropertyValue(FM_PROP_FETCHSIZE, aCacheSize);
        // jetzt auf alle Faelle den Cursor anpassen
        bUpdateCursor = sal_True;
        bCacheAligned = sal_True;
        nLimit = nLinesOnScreen;
    }

    // Im folgenden werden die Positionierungen so vorgenommen, da� sichergestellt ist
    // da� ausreichend Zeilen im DatenCache vorhanden sind

    // Fenster geht nach unten, weniger als zwei Fenster Differenz
    // oder Cache angepasst und noch kein Rowcount
    if (nDelta < nLimit && (nDelta > 0
        || (bCacheAligned && m_nTotalCount < 0)) )
        SeekCursor(nNewTopRow + nLinesOnScreen - 1, sal_False);
    else if (nDelta < 0 && Abs(nDelta) < nLimit)
        SeekCursor(nNewTopRow, sal_False);
    else if (nDelta != 0 || bUpdateCursor)
        SeekCursor(nNewTopRow, sal_True);

    AdjustRows();

    // ignore any updates implicit made
    EnablePaint(sal_True);
}

//------------------------------------------------------------------------------
void DbGridControl::RowInserted(long nRow, long nNumRows, sal_Bool bDoPaint, sal_Bool bKeepSelection)
{
    if (nNumRows)
    {
        if (m_bRecordCountFinal && m_nTotalCount < 0)
        {
            // if we have an insert row we have to reduce to count by 1
            // as the total count reflects only the existing rows in database
            m_nTotalCount = GetRowCount() + nNumRows;
            if (m_xEmptyRow.Is())
                --m_nTotalCount;
        }
        else if (m_nTotalCount >= 0)
            m_nTotalCount += nNumRows;

        DbGridControl_Base::RowInserted(nRow, nNumRows, bDoPaint, bKeepSelection);
        m_aBar.InvalidateState(NavigationBar::RECORD_COUNT);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::RowRemoved(long nRow, long nNumRows, sal_Bool bDoPaint)
{
    if (nNumRows)
    {
        if (m_bRecordCountFinal && m_nTotalCount < 0)
        {
            m_nTotalCount = GetRowCount() - nNumRows;
            // if we have an insert row reduce by 1
            if (m_xEmptyRow.Is())
                --m_nTotalCount;
        }
        else if (m_nTotalCount >= 0)
            m_nTotalCount -= nNumRows;

        DbGridControl_Base::RowRemoved(nRow, nNumRows, bDoPaint);
        m_aBar.InvalidateState(NavigationBar::RECORD_COUNT);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::AdjustRows()
{
    if (!m_pSeekCursor)
        return;

    Reference< XPropertySet > xSet = m_pDataCursor->getPropertySet();

    // Aktualisieren des RecordCounts
    sal_Int32 nRecordCount = 0;
    xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
    if (!m_bRecordCountFinal)
        m_bRecordCountFinal = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ROWCOUNTFINAL));

    // hat sich die aktuelle Anzahl Rows veraendert
    // hierbei muss auch beruecksichtigt werden,
    // das eine zusaetzliche Zeile zum einfuegen von Datensaetzen vorhanden sein kann

    // zusaetzliche AppendRow fuers einfuegen
    if (m_nOptions & OPT_INSERT)
        ++nRecordCount;

    // wird gerade eingefuegt, dann gehoert die gerade hinzuzufuegende
    // Zeile nicht zum RecordCount und die Appendrow ebenfalls nicht
    if (!IsUpdating() && m_bRecordCountFinal && IsModified() && m_xCurrentRow != m_xEmptyRow &&
        m_xCurrentRow->IsNew())
        ++nRecordCount;
    // das ist mit !m_bUpdating abgesichert : innerhalb von SaveRow (m_bUpdating == sal_True) wuerde sonst der Datensatz, den ich editiere
    // (und den SaveRow gerade angefuegt hat, wodurch diese Methode hier getriggert wurde), doppelt zaehlen : einmal ist er schon
    // in dem normalen RecordCount drin, zum zweiten wuerde er hier gezaehlt werden (60787 - FS)

    if (nRecordCount != GetRowCount())
    {
        long nDelta = GetRowCount() - (long)nRecordCount;
        if (nDelta > 0)                                         // zuviele
        {
            RowRemoved(GetRowCount() - nDelta, nDelta, sal_False);
            // es sind Zeilen weggefallen, dann ab der aktuellen Position neu zeichen
            Invalidate();

            sal_Int32 nNewPos = AlignSeekCursor();
            if (m_bSynchDisplay)
                DbGridControl_Base::GoToRow(nNewPos);

            SetCurrent(nNewPos);
            // there are rows so go to the selected current column
            if (nRecordCount)
                GoToRowColumnId(nNewPos, GetColumnId(GetCurColumnId()));
            if (!IsResizing() && GetRowCount())
                RecalcRows(GetTopRow(), GetVisibleRows(), sal_True);
            m_aBar.InvalidateAll(m_nCurrentPos, sal_True);
        }
        else                                                    // zuwenig
            RowInserted(GetRowCount(), -nDelta, sal_True);
    }

    if (m_bRecordCountFinal && m_nTotalCount < 0)
    {
        if (m_nOptions & OPT_INSERT)
            m_nTotalCount = GetRowCount() - 1;
        else
            m_nTotalCount = GetRowCount();
    }
    m_aBar.InvalidateState(NavigationBar::RECORD_COUNT);
}

//------------------------------------------------------------------------------
DbGridControl_Base::RowStatus DbGridControl::GetRowStatus(long nRow) const
{
    if (IsFilterRow(nRow))
        return DbGridControl_Base::FILTER;
    else if (m_nCurrentPos >= 0 && nRow == m_nCurrentPos)
    {
        // neue Zeile
        if (!IsValid(m_xCurrentRow))
            return DbGridControl_Base::DELETED;
        else if (IsModified())
            return DbGridControl_Base::MODIFIED;
        else if (m_xCurrentRow->IsNew())
            return DbGridControl_Base::CURRENTNEW;
        else
            return DbGridControl_Base::CURRENT;
    }
    else if (IsInsertionRow(nRow))
        return DbGridControl_Base::NEW;
    else if (!IsValid(m_xSeekRow))
        return DbGridControl_Base::DELETED;
    else
        return DbGridControl_Base::CLEAN;
}

//------------------------------------------------------------------------------
void DbGridControl::PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const
{
    DbGridControl_Base::PaintStatusCell(rDev, rRect);
}

//------------------------------------------------------------------------------
void DbGridControl::PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    if (!IsValid(m_xPaintRow))
        return;

    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = (Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    if (pColumn)
    {
        Rectangle aArea(rRect);
        if ((GetMode() & BROWSER_CURSOR_WO_FOCUS) == BROWSER_CURSOR_WO_FOCUS)
        {
            aArea.Top() += 1;
            aArea.Bottom() -= 1;
        }
        pColumn->Paint(rDev, aArea, m_xPaintRow, getNumberFormatter());
    }
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
{
    DBG_CHKTHIS( DbGridControl, NULL );

    DeactivateCell( sal_False );

    if  (   m_pDataCursor
        &&  ( m_nCurrentPos != nNewRow )
        && !SetCurrent( nNewRow )
        )
    {
        ActivateCell();
        return sal_False;
    }

    if ( !DbGridControl_Base::CursorMoving( nNewRow, nNewCol ) )
        return sal_False;

    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::SetCurrent(long nNewRow)
{
    // Each movement of the datacursor must start with BeginCursorAction and end with
    // EndCursorAction to block all notifications during the movement
    BeginCursorAction();

    try
    {
        // Abgleichen der Positionen
        if (SeekCursor(nNewRow))
        {
            if (IsFilterRow(nNewRow))   // special mode for filtering
            {
                m_xCurrentRow = m_xDataRow = m_xPaintRow = m_xEmptyRow;
                m_nCurrentPos = nNewRow;
            }
            else
            {
                sal_Bool bNewRowInserted = sal_False;
                // Should we go to the insertrow ?
                if (IsInsertionRow(nNewRow))
                {
                    // to we need to move the cursor to the insert row?
                    // we need to insert the if the current row isn't the insert row or if the
                    // cursor triggered the move by itselt and we need a reinitialization of the row
                    Reference< XPropertySet > xCursorProps = m_pDataCursor->getPropertySet();
                    if ( !::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_ISNEW)) )
                    {
                        Reference< XResultSetUpdate > xUpdateCursor((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
                        xUpdateCursor->moveToInsertRow();
                    }
                    bNewRowInserted = sal_True;
                }
                else
                {

                    if ( !m_pSeekCursor->isBeforeFirst() && !m_pSeekCursor->isAfterLast() )
                    {
                        Any aBookmark = m_pSeekCursor->getBookmark();
                        if (!m_xCurrentRow || m_xCurrentRow->IsNew() || !CompareBookmark(aBookmark, m_pDataCursor->getBookmark()))
                        {
                            // adjust the cursor to the new desired row
                            if (!m_pDataCursor->moveToBookmark(aBookmark))
                            {
                                EndCursorAction();
                                return sal_False;
                            }
                        }
                    }
                }
                m_xDataRow->SetState(m_pDataCursor, sal_False);
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
            return sal_False;
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        EndCursorAction();
        return sal_False;
    }

    EndCursorAction();
    return sal_True;
}

//------------------------------------------------------------------------------
void DbGridControl::CursorMoved()
{
    DBG_CHKTHIS( DbGridControl, NULL );

    // CursorBewegung durch loeschen oder einfuegen von Zeilen
    if (m_pDataCursor && m_nCurrentPos != GetCurRow())
    {
        DeactivateCell(sal_True);
        SetCurrent(GetCurRow());
    }

    DbGridControl_Base::CursorMoved();
    m_aBar.InvalidateAll(m_nCurrentPos);

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

//------------------------------------------------------------------------------
void DbGridControl::onRowChange()
{
    // not interested in
}

//------------------------------------------------------------------------------
void DbGridControl::onColumnChange()
{
    if ( m_pGridListener )
        m_pGridListener->columnChanged();
}

//------------------------------------------------------------------------------
void DbGridControl::setDisplaySynchron(sal_Bool bSync)
{
    if (bSync != m_bSynchDisplay)
    {
        m_bSynchDisplay = bSync;
        if (m_bSynchDisplay)
            AdjustDataSource(sal_False);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::AdjustDataSource(sal_Bool bFull)
{
    TRACE_RANGE("DbGridControl::AdjustDataSource");
    SolarMutexGuard aGuard;
    // wird die aktuelle Zeile gerade neu bestimmt,
    // wird kein abgleich vorgenommen

    if (bFull)
        m_xCurrentRow = NULL;
    // if we are on the same row only repaint
    // but this is only possible for rows which are not inserted, in that case the comparision result
    // may not be correct
    else
        if  (   m_xCurrentRow.Is()
            &&  !m_xCurrentRow->IsNew()
            &&  !m_pDataCursor->isBeforeFirst()
            &&  !m_pDataCursor->isAfterLast()
            &&  !m_pDataCursor->rowDeleted()
            )
        {
            sal_Bool bEqualBookmarks = CompareBookmark( m_xCurrentRow->GetBookmark(), m_pDataCursor->getBookmark() );

            sal_Bool bDataCursorIsOnNew = sal_False;
            m_pDataCursor->getPropertySet()->getPropertyValue( FM_PROP_ISNEW ) >>= bDataCursorIsOnNew;

            if ( bEqualBookmarks && !bDataCursorIsOnNew )
            {
                // position of my data cursor is the same as the position our current row points tpo
                // sync the status, repaint, done
                DBG_ASSERT(m_xDataRow == m_xCurrentRow, "Fehler in den Datenzeilen");
                TRACE_RANGE_MESSAGE1("same position, new state : %s", ROWSTATUS(m_xCurrentRow));
                RowModified(m_nCurrentPos);
                return;
            }
        }

    // weg von der Row des DatenCursors
    if (m_xPaintRow == m_xCurrentRow)
        m_xPaintRow = m_xSeekRow;

    // keine aktuelle Zeile dann komplett anpassen
    if (!m_xCurrentRow)
        AdjustRows();

    sal_Int32 nNewPos = AlignSeekCursor();
    if (nNewPos < 0)    // keine Position gefunden
        return;

    m_bInAdjustDataSource = sal_True;
    if (nNewPos != m_nCurrentPos)
    {
        if (m_bSynchDisplay)
            DbGridControl_Base::GoToRow(nNewPos);

        if (!m_xCurrentRow.Is())
            // das tritt zum Beispiel auf, wenn man die n (n>1) letzten Datensaetze geloescht hat, waehrend der Cursor auf dem letzten
            // steht : AdjustRows entfernt dann zwei Zeilen aus der BrowseBox, wodurch diese ihre CurrentRow um zwei nach unten
            // korrigiert, so dass dann das GoToRow in's Leere laeuft (da wir uns ja angeblich schon an der richtigen Position
            // befinden)
            SetCurrent(nNewPos);
    }
    else
    {
        SetCurrent(nNewPos);
        RowModified(nNewPos);
    }
    m_bInAdjustDataSource = sal_False;

    // Wird der DatenCursor von aussen bewegt, wird die selektion aufgehoben
    SetNoSelection();
    m_aBar.InvalidateAll(m_nCurrentPos, m_xCurrentRow.Is());
}

//------------------------------------------------------------------------------
sal_Int32 DbGridControl::AlignSeekCursor()
{
    DBG_CHKTHIS( DbGridControl, NULL );
    // Positioniert den SeekCursor auf den DatenCursor, Daten werden nicht uebertragen

    if (!m_pSeekCursor)
        return -1;

    Reference< XPropertySet > xSet = m_pDataCursor->getPropertySet();

    // jetzt den seekcursor an den DatenCursor angleichen
    if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW)))
        m_nSeekPos = GetRowCount() - 1;
    else
    {
        try
        {
            if ( m_pDataCursor->isBeforeFirst() )
            {
                // this is somewhat strange, but can nevertheless happen
                DBG_WARNING( "DbGridControl::AlignSeekCursor: nobody should tamper with my cursor this way (before first)!" );
                m_pSeekCursor->first();
                m_pSeekCursor->previous();
                m_nSeekPos = -1;
            }
            else if ( m_pDataCursor->isAfterLast() )
            {
                DBG_WARNING( "DbGridControl::AlignSeekCursor: nobody should tamper with my cursor this way (after last)!" );
                m_pSeekCursor->last();
                m_pSeekCursor->next();
                m_nSeekPos = -1;
            }
            else
            {
                m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                if (!CompareBookmark(m_pDataCursor->getBookmark(), m_pSeekCursor->getBookmark()))
                    // dummerweise kann das moveToBookmark indirekt dazu fuehren, dass der Seek-Cursor wieder neu positoniert wird (wenn
                    // naemlich das mit all seinen zu feuernden Events relativ komplexe moveToBookmark irgendwo ein Update ausloest),
                    // also muss ich es nochmal versuchen
                    m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                    // Nicht dass das jetzt nicht auch schief gegangen sein koennte, aber es ist zumindest unwahrscheinlicher geworden.
                    // Und die Alternative waere eine Schleife so lange bis es stimmt, und das kann auch nicht die Loesung sein
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            }
        }
        catch(Exception&)
        {
        }
    }
    return m_nSeekPos;
}
//------------------------------------------------------------------------------
sal_Bool DbGridControl::SeekCursor(long nRow, sal_Bool bAbsolute)
{
    DBG_CHKTHIS( DbGridControl, NULL );
    // Positioniert den SeekCursor, Daten werden nicht uebertragen

    // additions for the filtermode
    if (IsFilterRow(nRow))
    {
        m_nSeekPos = 0;
        return sal_True;
    }

    if (!m_pSeekCursor)
        return sal_False;

    // Befinden wir uns gerade beim Einfuegen
    if (IsValid(m_xCurrentRow) && m_xCurrentRow->IsNew() &&
        nRow >= m_nCurrentPos)
    {
        // dann darf auf alle Faelle nicht weiter nach unten gescrollt werden
        // da der letzte Datensatz bereits erreicht wurde!
        if (nRow == m_nCurrentPos)
        {
            // auf die aktuelle Zeile bewegt, dann muß kein abgleich gemacht werden, wenn
            // gerade ein Datensatz eingefuegt wird
            m_nSeekPos = nRow;
        }
        else if (IsInsertionRow(nRow))  // Leerzeile zum Einfuegen von Datensaetzen
            m_nSeekPos = nRow;
    }
    else if (IsInsertionRow(nRow))  // Leerzeile zum Einfuegen von Datensaetzen
        m_nSeekPos = nRow;
    else if ((-1 == nRow) && (GetRowCount() == ((m_nOptions & OPT_INSERT) ? 1 : 0)) && m_pSeekCursor->isAfterLast())
        m_nSeekPos = nRow;
    else
    {

        sal_Bool bSuccess=sal_False;
        long nSteps = 0;
        try
        {
            if ( m_pSeekCursor->rowDeleted() )
            {
                // somebody deleted the current row of the seek cursor. Move it away from this row.
                m_pSeekCursor->next();
                if ( m_pSeekCursor->isAfterLast() || m_pSeekCursor->isBeforeFirst() )
                    bAbsolute = sal_True;
            }

            if ( !bAbsolute )
            {
                DBG_ASSERT( !m_pSeekCursor->isAfterLast() && !m_pSeekCursor->isBeforeFirst(),
                    "DbGridControl::SeekCursor: how did the seek cursor get to this position?!" );
                nSteps = nRow - (m_pSeekCursor->getRow() - 1);
                bAbsolute = bAbsolute || (abs(nSteps) > 100);
            }

            if ( bAbsolute )
            {
                bSuccess = m_pSeekCursor->absolute(nRow + 1);
                if (bSuccess)
                    m_nSeekPos = nRow;
            }
            else
            {
                if (nSteps > 0)                                 // auf den letzten benoetigten Datensatz positionieren
                {
                    if (m_pSeekCursor->isAfterLast())
                        bSuccess = sal_False;
                    else if (m_pSeekCursor->isBeforeFirst())
                        bSuccess = m_pSeekCursor->absolute(nSteps);
                    else
                        bSuccess = m_pSeekCursor->relative(nSteps);
                }
                else if (nSteps < 0)
                {
                    if (m_pSeekCursor->isBeforeFirst())
                        bSuccess = sal_False;
                    else if (m_pSeekCursor->isAfterLast())
                        bSuccess = m_pSeekCursor->absolute(nSteps);
                    else
                        bSuccess = m_pSeekCursor->relative(nSteps);
                }
                else
                {
                    m_nSeekPos = nRow;
                    return sal_True;
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
                    bSuccess = m_pSeekCursor->last();
                else
                    bSuccess = m_pSeekCursor->first();
            }

            if (bSuccess)
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            else
                m_nSeekPos = -1;
        }
        catch(Exception&)
        {
            OSL_FAIL("DbGridControl::SeekCursor : failed ...");
            DBG_UNHANDLED_EXCEPTION();
            m_nSeekPos = -1;                        // kein Datensatz mehr vorhanden
        }
    }
    return m_nSeekPos == nRow;
}
//------------------------------------------------------------------------------
void DbGridControl::MoveToFirst()
{
    if (m_pSeekCursor && (GetCurRow() != 0))
        MoveToPosition(0);
}

//------------------------------------------------------------------------------
void DbGridControl::MoveToLast()
{
    if (!m_pSeekCursor)
        return;

    if (m_nTotalCount < 0)          // RecordCount steht noch nicht fest
    {
        try
        {
            sal_Bool bRes = m_pSeekCursor->last();

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

    // auf den letzen Datensatz positionieren, nicht auf die Leerzeile
    if (m_nOptions & OPT_INSERT)
    {
        if ((GetRowCount() - 1) > 0)
            MoveToPosition(GetRowCount() - 2);
    }
    else if (GetRowCount())
        MoveToPosition(GetRowCount() - 1);
}

//------------------------------------------------------------------------------
void DbGridControl::MoveToPrev()
{
    long nNewRow = std::max(GetCurRow() - 1L, 0L);
    if (GetCurRow() != nNewRow)
        MoveToPosition(nNewRow);
}

//------------------------------------------------------------------------------
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
        sal_Bool bOk = sal_False;
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
            DBG_UNHANDLED_EXCEPTION();
        }

        if(!bOk)
        {
            AdjustRows();
            if (m_nTotalCount > 0) // only to avoid infinte recursion
                MoveToNext();
        }
    }
}

//------------------------------------------------------------------------------
void DbGridControl::MoveToPosition(sal_uInt32 nPos)
{
    if (!m_pSeekCursor)
        return;

    if (m_nTotalCount < 0 && (long)nPos >= GetRowCount())
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
    DbGridControl_Base::GoToRow(nPos);
    m_aBar.InvalidateAll(m_nCurrentPos);
}

//------------------------------------------------------------------------------
void DbGridControl::AppendNew()
{
    if (!m_pSeekCursor || !(m_nOptions & OPT_INSERT))
        return;

    if (m_nTotalCount < 0)          // RecordCount steht noch nicht fest
    {
        try
        {
            sal_Bool bRes = m_pSeekCursor->last();

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

//------------------------------------------------------------------------------
void DbGridControl::SetDesignMode(sal_Bool bMode)
{
    if (IsDesignMode() != bMode)
    {
        // Enable/Disable f�r den Designmode anpassen damit die Headerbar konfigurierbar bleibt
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
            // komplett disablen
            if (!GetDataWindow().IsEnabled())
                Disable();
        }

        m_bDesignMode = bMode;
        GetDataWindow().SetMouseTransparent(bMode);
        SetMouseTransparent(bMode);

        m_aBar.InvalidateAll(m_nCurrentPos, sal_True);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::SetFilterMode(sal_Bool bMode)
{
    if (IsFilterMode() != bMode)
    {
        m_bFilterMode = bMode;

        if (bMode)
        {
            SetUpdateMode(sal_False);

            // es gibt kein Cursor mehr
            if (IsEditing())
                DeactivateCell();
            RemoveRows(sal_False);

            m_xEmptyRow = new DbGridRow();

            // setting the new filter controls
            for ( size_t i = 0; i < m_aColumns.size(); ++i )
            {
                DbGridColumn* pCurCol = m_aColumns[ i ];
                if (!pCurCol->IsHidden())
                    pCurCol->UpdateControl();
            }

            // one row for filtering
            RowInserted(0, 1, sal_True);
            SetUpdateMode(sal_True);
        }
        else
            setDataSource(Reference< XRowSet > ());
    }
}
// -----------------------------------------------------------------------------
String DbGridControl::GetCellText(long _nRow, sal_uInt16 _nColId) const
{
    size_t Location = GetModelColumnPos( _nColId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    String sRet;
    if ( const_cast<DbGridControl*>(this)->SeekRow(_nRow) )
        sRet = GetCurrentRowCellText(pColumn, m_xPaintRow);
    return sRet;
}
//------------------------------------------------------------------------------
XubString DbGridControl::GetCurrentRowCellText(DbGridColumn* pColumn,const DbGridRowRef& _rRow) const
{
    // Ausgabe des Textes fuer eine Zelle
    XubString aText;
    if ( pColumn && IsValid(m_xPaintRow) )
        aText = pColumn->GetCellText(_rRow, m_xFormatter);
    return aText;
}

//------------------------------------------------------------------------------
sal_uInt32 DbGridControl::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
{
    if (SeekRow(nRow))
    {
        size_t Location = GetModelColumnPos( nColId );
        DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
        return GetDataWindow().GetTextWidth(GetCurrentRowCellText(pColumn,m_xPaintRow));
    }
    else
        return 30;  //xxxx
}

//------------------------------------------------------------------------------
void DbGridControl::PreExecuteRowContextMenu(sal_uInt16 /*nRow*/, PopupMenu& rMenu)
{
    sal_Bool bDelete = (m_nOptions & OPT_DELETE) && GetSelectRowCount() && !IsCurrentAppending();
    // ist nur die Leerzeile selektiert, dann nicht loeschen
    bDelete = bDelete && !((m_nOptions & OPT_INSERT) && GetSelectRowCount() == 1 && IsRowSelected(GetRowCount() - 1));

    rMenu.EnableItem(SID_FM_DELETEROWS, bDelete);
    rMenu.EnableItem(SID_FM_RECORD_SAVE, IsModified());

    // the undo is more difficult
    sal_Bool bCanUndo = IsModified();
    long nState = -1;
    if (m_aMasterStateProvider.IsSet())
        nState = m_aMasterStateProvider.Call((void*)SID_FM_RECORD_UNDO);
    bCanUndo &= ( 0 != nState );

    rMenu.EnableItem(SID_FM_RECORD_UNDO, bCanUndo);
}

//------------------------------------------------------------------------------
void DbGridControl::PostExecuteRowContextMenu(sal_uInt16 /*nRow*/, const PopupMenu& /*rMenu*/, sal_uInt16 nExecutionResult)
{
    switch (nExecutionResult)
    {
        case SID_FM_DELETEROWS:
            // delete asynchron
            if (m_nDeleteEvent)
                Application::RemoveUserEvent(m_nDeleteEvent);
            m_nDeleteEvent = Application::PostUserEvent(LINK(this,DbGridControl,OnDelete));
            break;
        case SID_FM_RECORD_UNDO:
            Undo();
            break;
        case SID_FM_RECORD_SAVE:
            SaveRow();
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
void DbGridControl::DataSourcePropertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException )
{
    TRACE_RANGE("DbGridControl::DataSourcePropertyChanged");
    SolarMutexGuard aGuard;
    // prop "IsModified" changed ?
    // during update don't care about the modified state
    if (!IsUpdating() && evt.PropertyName.compareTo(FM_PROP_ISMODIFIED) == COMPARE_EQUAL)
    {
        Reference< XPropertySet > xSource(evt.Source, UNO_QUERY);
        DBG_ASSERT( xSource.is(), "DbGridControl::DataSourcePropertyChanged: invalid event source!" );
        sal_Bool bIsNew = sal_False;
        if (xSource.is())
            bIsNew = ::comphelper::getBOOL(xSource->getPropertyValue(FM_PROP_ISNEW));

        if (bIsNew && m_xCurrentRow.Is())
        {
            DBG_ASSERT(::comphelper::getBOOL(xSource->getPropertyValue(FM_PROP_ROWCOUNTFINAL)), "DbGridControl::DataSourcePropertyChanged : somebody moved the form to a new record before the row count was final !");
            sal_Int32 nRecordCount = 0;
            xSource->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
            if (::comphelper::getBOOL(evt.NewValue))
            {   // modified state changed from sal_False to sal_True and we're on a insert row
                // -> we've to add a new grid row
                if ((nRecordCount == GetRowCount() - 1)  && m_xCurrentRow->IsNew())
                {
                    RowInserted(GetRowCount(), 1, sal_True);
                    InvalidateStatusCell(m_nCurrentPos);
                    m_aBar.InvalidateAll(m_nCurrentPos);
                }
            }
            else
            {   // modified state changed from sal_True to sal_False and we're on a insert row
                // we have two "new row"s at the moment : the one we're editing currently (where the current
                // column is the only dirty element) and a "new new" row which is completely clean. As the first
                // one is about to be cleaned, too, the second one is obsolet now.
                if (m_xCurrentRow->IsNew() && nRecordCount == (GetRowCount() - 2))
                {
                    RowRemoved(GetRowCount() - 1, 1, sal_True);
                    InvalidateStatusCell(m_nCurrentPos);
                    m_aBar.InvalidateAll(m_nCurrentPos);
                }
            }
        }
        if (m_xCurrentRow.Is())
        {
            m_xCurrentRow->SetStatus(::comphelper::getBOOL(evt.NewValue) ? GRS_MODIFIED : GRS_CLEAN);
            m_xCurrentRow->SetNew( bIsNew );
            InvalidateStatusCell(m_nCurrentPos);
            TRACE_RANGE_MESSAGE1("modified flag changed, new state : %s", ROWSTATUS(m_xCurrentRow));
        }
    }
}

//------------------------------------------------------------------------------
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
        DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
        OStringTransferable* pTransferable = new OStringTransferable(GetCurrentRowCellText(pColumn,m_xPaintRow));
        Reference< XTransferable > xEnsureDelete(pTransferable);
        pTransferable->StartDrag(this, DND_ACTION_COPY);
    }
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::canCopyCellText(sal_Int32 _nRow, sal_Int16 _nColId)
{
    return  (_nRow >= 0)
        &&  (_nRow < GetRowCount())
        &&  (_nColId != HandleColumnId)
        &&  (_nColId <= ColCount());
}

//------------------------------------------------------------------------------
void DbGridControl::copyCellText(sal_Int32 _nRow, sal_Int16 _nColId)
{
    DBG_ASSERT(canCopyCellText(_nRow, _nColId), "DbGridControl::copyCellText: invalid call!");
    DbGridColumn* pColumn = m_aColumns[ GetModelColumnPos(_nColId) ];
    SeekRow(_nRow);
    OStringTransfer::CopyString( GetCurrentRowCellText( pColumn,m_xPaintRow ), this );
}

//------------------------------------------------------------------------------
void DbGridControl::executeRowContextMenu( long _nRow, const Point& _rPreferredPos )
{
    PopupMenu aContextMenu( SVX_RES( RID_SVXMNU_ROWS ) );

    PreExecuteRowContextMenu( (sal_uInt16)_nRow, aContextMenu );
    aContextMenu.RemoveDisabledEntries( sal_True, sal_True );
    PostExecuteRowContextMenu( (sal_uInt16)_nRow, aContextMenu, aContextMenu.Execute( this, _rPreferredPos ) );

    // TODO: why this weird cast to sal_uInt16? What if we really have more than 65535 lines?
    // -> change this to sal_uInt32
}

//------------------------------------------------------------------------------
void DbGridControl::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if ( !m_pSeekCursor )
            {
                DbGridControl_Base::Command(rEvt);
                return;
            }

            if ( !rEvt.IsMouseEvent() )
            {   // context menu requested by keyboard
                if ( GetSelectRowCount() )
                {
                    long nRow = FirstSelectedRow( );

                    ::Rectangle aRowRect( GetRowRectPixel( nRow, sal_True ) );
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
                PopupMenu aContextMenu(SVX_RES(RID_SVXMNU_CELL));
                aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
                switch (aContextMenu.Execute(this, rEvt.GetMousePosPixel()))
                {
                    case SID_COPY:
                        copyCellText(nRow, nColId);
                        break;
                }
            }
            else
            {
                DbGridControl_Base::Command(rEvt);
                return;
            }
        }
        default:
            DbGridControl_Base::Command(rEvt);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(DbGridControl, OnDelete, void*, /*EMPTYTAG*/ )
{
    DBG_CHKTHIS(DbGridControl, NULL );
    m_nDeleteEvent = 0;
    DeleteSelectedRows();
    return 0;
}

//------------------------------------------------------------------------------
void DbGridControl::DeleteSelectedRows()
{
    DBG_ASSERT(GetSelection(), "keine selection!!!");

    if (!m_pSeekCursor)
        return;
}

//------------------------------------------------------------------------------
CellController* DbGridControl::GetController(long /*nRow*/, sal_uInt16 nColumnId)
{
    if (!IsValid(m_xCurrentRow) || !IsEnabled())
        return NULL;

    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    if (!pColumn)
        return NULL;

    CellController* pReturn = NULL;
    if (IsFilterMode())
        pReturn = &pColumn->GetController();
    else
    {
        if (::comphelper::hasProperty(FM_PROP_ENABLED, pColumn->getModel()))
        {
            if (!::comphelper::getBOOL(pColumn->getModel()->getPropertyValue(FM_PROP_ENABLED)))
                return NULL;
        }

        sal_Bool bInsert = (m_xCurrentRow->IsNew() && (m_nOptions & OPT_INSERT));
        sal_Bool bUpdate = (!m_xCurrentRow->IsNew() && (m_nOptions & OPT_UPDATE));

        if ((bInsert && !pColumn->IsAutoValue()) || bUpdate || m_bForceROController)
        {
            pReturn = &pColumn->GetController();
            if (pReturn)
            {
                // wenn es eine Edit-Zeile ist, kann ich ihr das forced read-only mitgeben
                if (!pReturn->ISA(EditCellController) && !pReturn->ISA(SpinCellController))
                    // ich konnte den Controller in forceROController nicht auf ReadOnly setzen
                    if (!bInsert && !bUpdate)
                        // ich bin nur hier, da m_bForceROController gesetzt war
                        // -> lieber kein Controller als einer ohne RO
                        pReturn = NULL;
            }
        }
    }
    return pReturn;
}

//------------------------------------------------------------------------------
void DbGridControl::InitController(CellControllerRef& /*rController*/, long /*nRow*/, sal_uInt16 nColumnId)
{
    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    if (pColumn)
        pColumn->UpdateFromField(m_xCurrentRow, m_xFormatter);
}

//------------------------------------------------------------------------------
void DbGridControl::CellModified()
{
    TRACE_RANGE("DbGridControl::CellModified");

    {
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        if (m_nAsynAdjustEvent)
        {
            TRACE_RANGE_MESSAGE1("forcing a synchron call to ", m_bPendingAdjustRows ? "AdjustRows" : "AdustDataSource");
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = 0;

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
        // Einschalten des Editiermodus
        // Datensatz soll eingefuegt werden
        if (m_xCurrentRow->IsNew())
        {
            m_xCurrentRow->SetStatus(GRS_MODIFIED);
            TRACE_RANGE_MESSAGE("current row is new, new state : MODIFIED");
            // wenn noch keine Zeile hinzugefuegt wurde, dann neue hinzunehmen
            if (m_nCurrentPos == GetRowCount() - 1)
            {
                // RowCount um einen erhoehen
                RowInserted(GetRowCount(), 1, sal_True);
                InvalidateStatusCell(m_nCurrentPos);
                m_aBar.InvalidateAll(m_nCurrentPos);
            }
        }
        else if (m_xCurrentRow->GetStatus() != GRS_MODIFIED)
        {
            m_xCurrentRow->SetState(m_pDataCursor, sal_False);
            TRACE_RANGE_MESSAGE1("current row is not new, after SetState, new state : %s", ROWSTATUS(m_xCurrentRow));
            m_xCurrentRow->SetStatus(GRS_MODIFIED);
            TRACE_RANGE_MESSAGE("current row is not new, new state : MODIFIED");
            InvalidateStatusCell(m_nCurrentPos);
        }
    }
}

//------------------------------------------------------------------------------
void DbGridControl::Dispatch(sal_uInt16 nId)
{
    if (nId == BROWSER_CURSORENDOFFILE)
    {
        if (m_nOptions & OPT_INSERT)
            AppendNew();
        else
            MoveToLast();
    }
    else
        DbGridControl_Base::Dispatch(nId);
}

//------------------------------------------------------------------------------
void DbGridControl::Undo()
{
    if (!IsFilterMode() && IsValid(m_xCurrentRow) && IsModified())
    {
        // check if we have somebody doin' the UNDO for us
        long nState = -1;
        if (m_aMasterStateProvider.IsSet())
            nState = m_aMasterStateProvider.Call((void*)SID_FM_RECORD_UNDO);
        if (nState>0)
        {   // yes, we have, and the slot is enabled
            DBG_ASSERT(m_aMasterSlotExecutor.IsSet(), "DbGridControl::Undo : a state, but no execute link ?");
            long lResult = m_aMasterSlotExecutor.Call((void*)SID_FM_RECORD_UNDO);
            if (lResult)
                // handled
                return;
        }
        else if (nState == 0)
            // yes, we have, and the slot is disabled
            return;

        BeginCursorAction();

        sal_Bool bAppending = m_xCurrentRow->IsNew();
        sal_Bool bDirty     = m_xCurrentRow->IsModified();

        try
        {
            // Editieren abbrechen
            Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
            // no effects if we're not updating currently
            if (bAppending)
                // just refresh the row
                xUpdateCursor->moveToInsertRow();
            else
                xUpdateCursor->cancelRowUpdates();

        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        EndCursorAction();

        m_xDataRow->SetState(m_pDataCursor, sal_False);
        if (&m_xPaintRow == &m_xCurrentRow)
            m_xPaintRow = m_xCurrentRow = m_xDataRow;
        else
            m_xCurrentRow = m_xDataRow;

        if (bAppending && (DbGridControl_Base::IsModified() || bDirty))
        // remove the row
            if (m_nCurrentPos == GetRowCount() - 2)
            {   // maybe we already removed it (in resetCurrentRow, called if the above moveToInsertRow
                // caused our data source form to be reset - which should be the usual case ....)
                RowRemoved(GetRowCount() - 1, 1, sal_True);
                m_aBar.InvalidateAll(m_nCurrentPos);
            }

        RowModified(m_nCurrentPos);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::resetCurrentRow()
{
    if (IsModified())
    {
        // scenario : we're on the insert row, the row is dirty, and thus there exists a "second" insert row (which
        // is clean). Normally in DataSourcePropertyChanged we would remove this second row if the modified state of
        // the insert row changes from sal_True to sal_False. But if our current cell is the only modified element (means the
        // data source isn't modified) and we're reset this DataSourcePropertyChanged would never be called, so we
        // would never delete the obsolet "second insert row". Thus in this special case this method here
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
                    RowRemoved(GetRowCount() - 1, 1, sal_True);
                    m_aBar.InvalidateAll(m_nCurrentPos);
                }
            }
        }

        // update the rows
        m_xDataRow->SetState(m_pDataCursor, sal_False);
        if (&m_xPaintRow == &m_xCurrentRow)
            m_xPaintRow = m_xCurrentRow = m_xDataRow;
        else
            m_xCurrentRow = m_xDataRow;
    }

    RowModified(GetCurRow());       // will update the current controller if affected
}

//------------------------------------------------------------------------------
void DbGridControl::RowModified( long nRow, sal_uInt16 /*nColId*/ )
{
    if (nRow == m_nCurrentPos && IsEditing())
    {
        CellControllerRef aTmpRef = Controller();
        aTmpRef->ClearModified();
        InitController(aTmpRef, m_nCurrentPos, GetCurColumnId());
    }
    DbGridControl_Base::RowModified(nRow);
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::IsModified() const
{
    return !IsFilterMode() && IsValid(m_xCurrentRow) && (m_xCurrentRow->IsModified() || DbGridControl_Base::IsModified());
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::IsCurrentAppending() const
{
    return m_xCurrentRow.Is() && m_xCurrentRow->IsNew();
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::IsInsertionRow(long nRow) const
{
    return (m_nOptions & OPT_INSERT) && m_nTotalCount >= 0 && (nRow == GetRowCount() - 1);
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::SaveModified()
{
    TRACE_RANGE("DbGridControl::SaveModified");
    DBG_ASSERT(IsValid(m_xCurrentRow), "GridControl:: Invalid row");
    if (!IsValid(m_xCurrentRow))
        return sal_True;

    // Uebernimmt die Dateneingabe fuer das Feld
    // Hat es aenderungen im aktuellen Eingabefeld gegeben ?
    if (!DbGridControl_Base::IsModified())
        return sal_True;

    size_t Location = GetModelColumnPos( GetCurColumnId() );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    sal_Bool bOK = pColumn->Commit();
    DBG_ASSERT( Controller().Is(), "DbGridControl::SaveModified: was modified, by have no controller?!" );
    if ( !Controller().Is() )
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
            m_xCurrentRow->SetState(m_pDataCursor, sal_False);
            TRACE_RANGE_MESSAGE1("explicit SetState, new state : %s", ROWSTATUS(m_xCurrentRow));
            InvalidateStatusCell( m_nCurrentPos );
        }
#ifdef DBG_UTIL
        else
        {
            TRACE_RANGE_MESSAGE1("no SetState, new state : %s", ROWSTATUS(m_xCurrentRow));
        }
#endif
    }
    else
    {
        // reset the modified flag ....
        Controller()->SetModified();
    }

    return bOK;
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::SaveRow()
{
    TRACE_RANGE("DbGridControl::SaveRow");
    // gueltige Zeile
    if (!IsValid(m_xCurrentRow) || !IsModified())
        return sal_True;
    // Wert des Controllers noch nicht gespeichert
    else if (Controller().Is() && Controller()->IsModified())
    {
        if (!SaveModified())
            return sal_False;
    }
    m_bUpdating = sal_True;

    BeginCursorAction();
    sal_Bool bAppending = m_xCurrentRow->IsNew();
    sal_Bool bSuccess = sal_False;
    try
    {
        Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
        if (bAppending)
            xUpdateCursor->insertRow();
        else
            xUpdateCursor->updateRow();
        bSuccess = sal_True;
    }
    catch(SQLException&)
    {
        EndCursorAction();
        m_bUpdating = sal_False;
        return sal_False;
    }

    try
    {
        if (bSuccess)
        {
            // if we are appending we still sit on the insert row
            // we don't move just clear the flags not to move on the current row
            m_xCurrentRow->SetState(m_pDataCursor, sal_False);
            TRACE_RANGE_MESSAGE1("explicit SetState after a successfull update, new state : %s", ROWSTATUS(m_xCurrentRow));
            m_xCurrentRow->SetNew(sal_False);

            // adjust the seekcursor if it is on the same position as the datacursor
            if (m_nSeekPos == m_nCurrentPos || bAppending)
            {
                // get the bookmark to refetch the data
                // in insert mode we take the new bookmark of the data cursor
                Any aBookmark = bAppending ? m_pDataCursor->getBookmark() : m_pSeekCursor->getBookmark();
                m_pSeekCursor->moveToBookmark(aBookmark);
                // get the data
                m_xSeekRow->SetState(m_pSeekCursor, sal_True);
                m_nSeekPos = m_pSeekCursor->getRow() - 1;
            }
        }
        // and repaint the row
        RowModified(m_nCurrentPos);
    }
    catch(Exception&)
    {
    }

    m_bUpdating = sal_False;
    EndCursorAction();

    // The old code returned (nRecords != 0) here.
    // Me thinks this is wrong : If something goes wrong while update the record, an exception will be thrown,
    // which results in a "return sal_False" (see above). If no exception is thrown, everything is fine. If nRecords
    // is zero, this simply means all fields had their original values.
    // FS - 06.12.99 - 70502
    return sal_True;
}

//------------------------------------------------------------------------------
long DbGridControl::PreNotify(NotifyEvent& rEvt)
{
    // keine Events der Navbar behandeln
    if (m_aBar.IsWindowOrChild(rEvt.GetWindow()))
        return BrowseBox::PreNotify(rEvt);

    switch (rEvt.GetType())
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rEvt.GetKeyEvent();

            sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();
            sal_Bool   bShift = pKeyEvent->GetKeyCode().IsShift();
            sal_Bool   bCtrl = pKeyEvent->GetKeyCode().IsMod1();
            sal_Bool   bAlt = pKeyEvent->GetKeyCode().IsMod2();
            if ( ( KEY_TAB == nCode ) && bCtrl && !bAlt )
            {
                // Ctrl-Tab is used to step out of the control, without traveling to the
                // remaining cells first
                // -> build a new key event without the Ctrl-key, and let the very base class handle it
                KeyCode aNewCode( KEY_TAB, bShift, sal_False, sal_False, sal_False );
                KeyEvent aNewEvent( pKeyEvent->GetCharCode(), aNewCode );

                // call the Control - our direct base class will interpret this in a way we do not want (and do
                // a cell traveling)
                Control::KeyInput( aNewEvent );
                return 1;
            }

            if ( !bShift && !bCtrl && ( KEY_ESCAPE == nCode ) )
            {
                if (IsModified())
                {
                    Undo();
                    return 1;
                }
            }
            else if ( ( KEY_DELETE == nCode ) && !bShift && !bCtrl )    // delete rows
            {
                if ((m_nOptions & OPT_DELETE) && GetSelectRowCount())
                {
                    // delete asynchron
                    if (m_nDeleteEvent)
                        Application::RemoveUserEvent(m_nDeleteEvent);
                    m_nDeleteEvent = Application::PostUserEvent(LINK(this,DbGridControl,OnDelete));
                    return 1;
                }
            }
        }   // kein break!
        default:
            return DbGridControl_Base::PreNotify(rEvt);
    }
}

//------------------------------------------------------------------------------
sal_Bool DbGridControl::IsTabAllowed(sal_Bool bRight) const
{
    if (bRight)
        // Tab nur wenn nicht auf der letzten Zelle
        return GetCurRow() < (GetRowCount() - 1) || !m_bRecordCountFinal ||
               GetViewColumnPos(GetCurColumnId()) < (GetViewColCount() - 1);
    else
    {
        // Tab nur wenn nicht auf der ersten Zelle
        return GetCurRow() > 0 || (GetCurColumnId() && GetViewColumnPos(GetCurColumnId()) > 0);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::KeyInput( const KeyEvent& rEvt )
{
    if (rEvt.GetKeyCode().GetFunction() == KEYFUNC_COPY)
    {
        long nRow = GetCurRow();
        sal_uInt16 nColId = GetCurColumnId();
        if (nRow >= 0 && nRow < GetRowCount() && nColId < ColCount())
        {
            size_t Location = GetModelColumnPos( nColId );
            DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
            OStringTransfer::CopyString( GetCurrentRowCellText( pColumn,m_xPaintRow ), this );
            return;
        }
    }
    DbGridControl_Base::KeyInput(rEvt);
}

//------------------------------------------------------------------------------
void DbGridControl::HideColumn(sal_uInt16 nId)
{
    DeactivateCell();

    // determine the col for the focus to set to after removal
    sal_uInt16 nPos = GetViewColumnPos(nId);
    sal_uInt16 nNewColId = nPos == (ColCount()-1)
        ? GetColumnIdFromViewPos(nPos-1)    // last col is to be removed -> take the previous
        : GetColumnIdFromViewPos(nPos+1);   // take the next

    long lCurrentWidth = GetColumnWidth(nId);
    DbGridControl_Base::RemoveColumn(nId);
        // don't use my own RemoveColumn, this would remove it from m_aColumns, too

    // update my model
    size_t Location = GetModelColumnPos( nId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    DBG_ASSERT(pColumn, "DbGridControl::HideColumn : somebody did hide a nonexistent column !");
    if (pColumn)
    {
        pColumn->m_bHidden = sal_True;
        pColumn->m_nLastVisibleWidth = CalcReverseZoom(lCurrentWidth);
    }

    // and reset the focus
    if ( nId == GetCurColumnId() )
        GoToColumnId( nNewColId );
}

//------------------------------------------------------------------------------
void DbGridControl::ShowColumn(sal_uInt16 nId)
{
    sal_uInt16 nPos = GetModelColumnPos(nId);
    DBG_ASSERT(nPos != GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : invalid argument !");
    if (nPos == GRID_COLUMN_NOT_FOUND)
        return;

    DbGridColumn* pColumn = m_aColumns[ nPos ];
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
        DbGridColumn* pCurCol = m_aColumns[ i ];
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
            DbGridColumn* pCurCol = m_aColumns[ i-1 ];
            if (!pCurCol->IsHidden())
            {
                nNextNonHidden = i-1;
                break;
            }
        }
    }
    sal_uInt16 nNewViewPos = (nNextNonHidden == BROWSER_INVALIDID)
        ? 1 // there is no visible column -> insert behinde the handle col
        : GetViewColumnPos( m_aColumns[ nNextNonHidden ]->GetId() ) + 1;
            // the first non-handle col has "view pos" 0, but the pos arg for InsertDataColumn expects
            // a position 1 for the first non-handle col -> +1
    DBG_ASSERT(nNewViewPos != GRID_COLUMN_NOT_FOUND, "DbGridControl::ShowColumn : inconsistent internal state !");
        // we found a col marked as visible but got no view pos for it ...

    if ((nNextNonHidden<nPos) && (nNextNonHidden != BROWSER_INVALIDID))
        // nNextNonHidden is a column to the left, so we want to insert the new col _right_ beside it's pos
        ++nNewViewPos;

    DeactivateCell();

    ::rtl::OUString aName;
    pColumn->getModel()->getPropertyValue(FM_PROP_LABEL) >>= aName;
    InsertDataColumn(nId, aName, CalcZoom(pColumn->m_nLastVisibleWidth), HIB_CENTER | HIB_VCENTER | HIB_CLICKABLE, nNewViewPos);
    pColumn->m_bHidden = sal_False;

    ActivateCell();
    Invalidate();
}

//------------------------------------------------------------------------------
sal_uInt16 DbGridControl::GetColumnIdFromModelPos( sal_uInt16 nPos ) const
{
    if (nPos >= m_aColumns.size())
    {
        OSL_FAIL("DbGridControl::GetColumnIdFromModelPos : invalid argument !");
        return GRID_COLUMN_NOT_FOUND;
    }

    DbGridColumn* pCol = m_aColumns[ nPos ];
#if (OSL_DEBUG_LEVEL > 0) || defined DBG_UTIL
    // in der Debug-Version rechnen wir die ModelPos in eine ViewPos um und vergleichen das mit dem Wert,
    // den wir zurueckliefern werden (nId an der entsprechenden Col in m_aColumns)

    if (!pCol->IsHidden())
    {   // macht nur Sinn, wenn die Spalte sichtbar ist
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

//------------------------------------------------------------------------------
sal_uInt16 DbGridControl::GetModelColumnPos( sal_uInt16 nId ) const
{
    for ( size_t i = 0; i < m_aColumns.size(); ++i )
        if ( m_aColumns[ i ]->GetId() == nId )
            return i;

    return GRID_COLUMN_NOT_FOUND;
}

//------------------------------------------------------------------------------
void DbGridControl::implAdjustInSolarThread(sal_Bool _bRows)
{
    TRACE_RANGE("DbGridControl::implAdjustInSolarThread");
    ::osl::MutexGuard aGuard(m_aAdjustSafety);
    if (::osl::Thread::getCurrentIdentifier() != Application::GetMainThreadIdentifier())
    {
        m_nAsynAdjustEvent = PostUserEvent(LINK(this, DbGridControl, OnAsyncAdjust), reinterpret_cast< void* >( _bRows ));
        m_bPendingAdjustRows = _bRows;
#ifdef DBG_UTIL
        if (_bRows)
            TRACE_RANGE_MESSAGE("posting an AdjustRows")
        else
            TRACE_RANGE_MESSAGE("posting an AdjustDataSource")
#endif
    }
    else
    {
#ifdef DBG_UTIL
        if (_bRows)
            TRACE_RANGE_MESSAGE("doing an AdjustRows")
        else
            TRACE_RANGE_MESSAGE("doing an AdjustDataSource")
#endif
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

//------------------------------------------------------------------------------
IMPL_LINK(DbGridControl, OnAsyncAdjust, void*, pAdjustWhat)
{
    m_nAsynAdjustEvent = 0;

    AdjustRows();
        // see implAdjustInSolarThread for a comment why we do this every time

    if ( !pAdjustWhat )
        AdjustDataSource();

    return 0L;
}

//------------------------------------------------------------------------------
void DbGridControl::BeginCursorAction()
{
    if (m_pFieldListeners)
    {
        ColumnFieldValueListeners* pListeners = (ColumnFieldValueListeners*)m_pFieldListeners;
        ConstColumnFieldValueListenersIterator aIter = pListeners->begin();
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

//------------------------------------------------------------------------------
void DbGridControl::EndCursorAction()
{
    if (m_pFieldListeners)
    {
        ColumnFieldValueListeners* pListeners = (ColumnFieldValueListeners*)m_pFieldListeners;
        ConstColumnFieldValueListenersIterator aIter = pListeners->begin();
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

//------------------------------------------------------------------------------
void DbGridControl::ConnectToFields()
{
    ColumnFieldValueListeners* pListeners = (ColumnFieldValueListeners*)m_pFieldListeners;
    DBG_ASSERT(!pListeners || pListeners->empty(), "DbGridControl::ConnectToFields : please call DisconnectFromFields first !");

    if (!pListeners)
    {
        pListeners = new ColumnFieldValueListeners;
        m_pFieldListeners = pListeners;
    }

    for ( size_t i = 0; i < m_aColumns.size(); ++i )
    {
        DbGridColumn* pCurrent = m_aColumns[ i ];
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

//------------------------------------------------------------------------------
void DbGridControl::DisconnectFromFields()
{
    if (!m_pFieldListeners)
        return;

    ColumnFieldValueListeners* pListeners = (ColumnFieldValueListeners*)m_pFieldListeners;
    while (pListeners->size())
    {
#ifdef DBG_UTIL
        sal_Int32 nOldSize = pListeners->size();
#endif
        pListeners->begin()->second->dispose();
        DBG_ASSERT(nOldSize > (sal_Int32)pListeners->size(), "DbGridControl::DisconnectFromFields : dispose on a listener should result in a removal from my list !");
    }

    delete pListeners;
    m_pFieldListeners = NULL;
}

//------------------------------------------------------------------------------
void DbGridControl::FieldValueChanged(sal_uInt16 _nId, const PropertyChangeEvent& /*_evt*/)
{
    osl::MutexGuard aPreventDestruction(m_aDestructionSafety);
    // needed as this may run in a thread other than the main one
    if (GetRowStatus(GetCurRow()) != DbGridControl_Base::MODIFIED)
        // all other cases are handled elsewhere
        return;

    size_t Location = GetModelColumnPos( _nId );
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    if (pColumn)
    {
        sal_Bool bAcquiredPaintSafety = sal_False;
        while (!m_bWantDestruction && !bAcquiredPaintSafety)
            bAcquiredPaintSafety  = Application::GetSolarMutex().tryToAcquire();

        if (m_bWantDestruction)
        {   // at this moment, within another thread, our destructor tries to destroy the listener which called this method
            // => don't do anything
            // 73365 - 23.02.00 - FS
            if (bAcquiredPaintSafety)
                // though the above while-loop suggests that (m_bWantDestruction && bAcquiredPaintSafety) is impossible,
                // it isnt't, as m_bWantDestruction isn't protected with any mutex
                Application::GetSolarMutex().release();
            return;
        }
        // here we got the solar mutex, transfer it to a guard for safety reasons
        SolarMutexGuard aPaintSafety;
        Application::GetSolarMutex().release();

        // and finally do the update ...
        pColumn->UpdateFromField(m_xCurrentRow, m_xFormatter);
        RowModified(GetCurRow(), _nId);
    }
}

//------------------------------------------------------------------------------
void DbGridControl::FieldListenerDisposing(sal_uInt16 _nId)
{
    ColumnFieldValueListeners* pListeners = (ColumnFieldValueListeners*)m_pFieldListeners;
    if (!pListeners)
    {
        OSL_FAIL("DbGridControl::FieldListenerDisposing : invalid call (have no listener array) !");
        return;
    }

    ColumnFieldValueListenersIterator aPos = pListeners->find(_nId);
    if (aPos == pListeners->end())
    {
        OSL_FAIL("DbGridControl::FieldListenerDisposing : invalid call (did not find the listener) !");
        return;
    }

    delete aPos->second;

    pListeners->erase(aPos);
}

//------------------------------------------------------------------------------
void DbGridControl::disposing(sal_uInt16 _nId, const EventObject& /*_rEvt*/)
{
    if (_nId == 0)
    {   // the seek cursor is beeing disposed
        ::osl::MutexGuard aGuard(m_aAdjustSafety);
        setDataSource(NULL,0); // our clone was disposed so we set our datasource to null to avoid later acces to it
        if (m_nAsynAdjustEvent)
        {
            RemoveUserEvent(m_nAsynAdjustEvent);
            m_nAsynAdjustEvent = 0;
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int32 DbGridControl::GetAccessibleControlCount() const
{
    return DbGridControl_Base::GetAccessibleControlCount() + 1; // the navigation control
}
// -----------------------------------------------------------------------------
Reference<XAccessible > DbGridControl::CreateAccessibleControl( sal_Int32 _nIndex )
{
    Reference<XAccessible > xRet;
    if ( _nIndex == DbGridControl_Base::GetAccessibleControlCount() )
    {
        xRet = m_aBar.GetAccessible();
    }
    else
        xRet = DbGridControl_Base::CreateAccessibleControl( _nIndex );
    return xRet;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > DbGridControl::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    sal_uInt16 nColumnId = GetColumnId( _nColumnPos );
    size_t Location = GetModelColumnPos(nColumnId);
    DbGridColumn* pColumn = ( Location < m_aColumns.size() ) ? m_aColumns[ Location ] : NULL;
    if ( pColumn )
    {
        Reference< ::com::sun::star::awt::XControl> xInt(pColumn->GetCell());
        Reference< ::com::sun::star::awt::XCheckBox> xBox(xInt,UNO_QUERY);
        if ( xBox.is() )
        {
            TriState eValue = STATE_NOCHECK;
            switch( xBox->getState() )
            {
                case 0:
                    eValue = STATE_NOCHECK;
                    break;
                case 1:
                    eValue = STATE_CHECK;
                    break;
                case 2:
                    eValue = STATE_DONTKNOW;
                    break;
            }
            return DbGridControl_Base::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,eValue );
        }
    }
    return DbGridControl_Base::CreateAccessibleCell( _nRow, _nColumnPos );
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
