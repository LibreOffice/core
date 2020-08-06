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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include "datman.hxx"
#include "toolbar.hxx"
#include <o3tl/any.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/event.hxx>
#include <bitmaps.hlst>

#include "bibtools.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


// Constants --------------------------------------------------------------


BibToolBarListener::BibToolBarListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
        nIndex(nId),
        aCommand(aStr),
        pToolBar(pTB)
{
}

BibToolBarListener::~BibToolBarListener()
{
}

void BibToolBarListener::statusChanged(const css::frame::FeatureStateEvent& rEvt)
{
    if(rEvt.FeatureURL.Complete == aCommand)
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableItem(nIndex,rEvt.IsEnabled);

        css::uno::Any aState=rEvt.State;
        if(auto bChecked = o3tl::tryAccess<bool>(aState))
        {
            pToolBar->CheckItem(nIndex, *bChecked);
        }

    }
};


BibTBListBoxListener::BibTBListBoxListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBListBoxListener::~BibTBListBoxListener()
{
}

void BibTBListBoxListener::statusChanged(const css::frame::FeatureStateEvent& rEvt)
{
    if(rEvt.FeatureURL.Complete != GetCommand())
        return;

    SolarMutexGuard aGuard;
    pToolBar->EnableSourceList(rEvt.IsEnabled);

    Any aState = rEvt.State;
    if(auto pStringSeq = o3tl::tryAccess<Sequence<OUString>>(aState))
    {
        pToolBar->UpdateSourceList(false);
        pToolBar->ClearSourceList();

        const OUString* pStringArray = pStringSeq->getConstArray();

        sal_uInt32 nCount = pStringSeq->getLength();
        OUString aEntry;
        for( sal_uInt32 i=0; i<nCount; i++ )
        {
            aEntry = pStringArray[i];
            pToolBar->InsertSourceEntry(aEntry);
        }
        pToolBar->UpdateSourceList(true);
    }

    pToolBar->SelectSourceEntry(rEvt.FeatureDescriptor);
};

BibTBQueryMenuListener::BibTBQueryMenuListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBQueryMenuListener::~BibTBQueryMenuListener()
{
}

void BibTBQueryMenuListener::statusChanged(const frame::FeatureStateEvent& rEvt)
{
    if(rEvt.FeatureURL.Complete != GetCommand())
        return;

    SolarMutexGuard aGuard;
    pToolBar->EnableSourceList(rEvt.IsEnabled);

    uno::Any aState=rEvt.State;
    auto pStringSeq = o3tl::tryAccess<Sequence<OUString>>(aState);
    if(!pStringSeq)
        return;

    pToolBar->ClearFilterMenu();

    const OUString* pStringArray = pStringSeq->getConstArray();

    sal_uInt32 nCount = pStringSeq->getLength();
    for( sal_uInt32 i=0; i<nCount; i++ )
    {
        sal_uInt16 nID = pToolBar->InsertFilterItem(pStringArray[i]);
        if(pStringArray[i]==rEvt.FeatureDescriptor)
        {
            pToolBar->SelectFilterItem(nID);
        }
    }
};

BibTBEditListener::BibTBEditListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBEditListener::~BibTBEditListener()
{
}

void BibTBEditListener::statusChanged(const frame::FeatureStateEvent& rEvt)
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableQuery(rEvt.IsEnabled);

        uno::Any aState=rEvt.State;
        if(auto aStr = o3tl::tryAccess<OUString>(aState))
        {
            pToolBar->SetQueryString(*aStr);
        }
    }
}

ComboBoxControl::ComboBoxControl(vcl::Window* pParent)
    : InterimItemWindow(pParent, "modules/sbibliography/ui/combobox.ui", "ComboBox")
    , m_xFtSource(m_xBuilder->weld_label("label"))
    , m_xLBSource(m_xBuilder->weld_combo_box("combobox"))
{
    m_xFtSource->set_toolbar_background();
    m_xLBSource->set_toolbar_background();
    m_xLBSource->set_size_request(100, -1);
    SetSizePixel(get_preferred_size());
}

void ComboBoxControl::dispose()
{
    m_xLBSource.reset();
    m_xFtSource.reset();
    InterimItemWindow::dispose();
}

ComboBoxControl::~ComboBoxControl()
{
    disposeOnce();
}

EditControl::EditControl(vcl::Window* pParent)
    : InterimItemWindow(pParent, "modules/sbibliography/ui/editbox.ui", "EditBox")
    , m_xFtQuery(m_xBuilder->weld_label("label"))
    , m_xEdQuery(m_xBuilder->weld_entry("entry"))
{
    m_xFtQuery->set_toolbar_background();
    m_xEdQuery->set_toolbar_background();
    m_xEdQuery->set_size_request(100, -1);
    SetSizePixel(get_preferred_size());
}

void EditControl::dispose()
{
    m_xEdQuery.reset();
    m_xFtQuery.reset();
    InterimItemWindow::dispose();
}

EditControl::~EditControl()
{
    disposeOnce();
}

BibToolBar::BibToolBar(vcl::Window* pParent, Link<void*,void> aLink)
    : ToolBox(pParent, "toolbar", "modules/sbibliography/ui/toolbar.ui")
    , xSource(VclPtr<ComboBoxControl>::Create(this))
    , pLbSource(xSource->get_widget())
    , xQuery(VclPtr<EditControl>::Create(this))
    , pEdQuery(xQuery->get_widget())
    , pPopupMenu(VclPtr<PopupMenu>::Create())
    , nMenuId(0)
    , nSelMenuItem(0)
    , aLayoutManager(aLink)
    , nSymbolsSize(SFX_SYMBOLS_SIZE_SMALL)
    , nOutStyle(0)
{
    SvtMiscOptions aSvtMiscOptions;
    nSymbolsSize = aSvtMiscOptions.GetCurrentSymbolsSize();
    nOutStyle  = aSvtMiscOptions.GetToolboxStyle();

    SetOutStyle(TOOLBOX_STYLE_FLAT);
    xSource->Show();
    pLbSource->connect_changed(LINK( this, BibToolBar, SelHdl));

    SvtMiscOptions().AddListenerLink( LINK( this, BibToolBar, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, BibToolBar, SettingsChanged_Impl ) );

    aIdle.SetInvokeHandler(LINK( this, BibToolBar, SendSelHdl));
    aIdle.SetPriority(TaskPriority::LOWEST);

    SetDropdownClickHdl( LINK( this, BibToolBar, MenuHdl));

    xQuery->Show();

    nTBC_SOURCE = GetItemId(".uno:Bib/source");
    nTBC_QUERY = GetItemId(".uno:Bib/query");
    nTBC_BT_AUTOFILTER = GetItemId(".uno:Bib/autoFilter");
    nTBC_BT_COL_ASSIGN = GetItemId("TBC_BT_COL_ASSIGN");
    nTBC_BT_CHANGESOURCE = GetItemId(".uno:Bib/sdbsource");
    nTBC_BT_FILTERCRIT = GetItemId(".uno:Bib/standardFilter");
    nTBC_BT_REMOVEFILTER = GetItemId(".uno:Bib/removeFilter");

    SetItemWindow(nTBC_SOURCE, xSource.get());
    SetItemWindow(nTBC_QUERY , xQuery.get());

    ApplyImageList();

    ::bib::AddToTaskPaneList( this );
}

BibToolBar::~BibToolBar()
{
    disposeOnce();
}

void BibToolBar::dispose()
{
    SvtMiscOptions().RemoveListenerLink( LINK( this, BibToolBar, OptionsChanged_Impl ) );
    Application::RemoveEventListener( LINK( this, BibToolBar, SettingsChanged_Impl ) );
    ::bib::RemoveFromTaskPaneList( this );
    pEdQuery = nullptr;
    xQuery.disposeAndClear();
    pLbSource = nullptr;
    xSource.disposeAndClear();
    ToolBox::dispose();
}

void BibToolBar::InitListener()
{
    ToolBox::ImplToolItems::size_type nCount=GetItemCount();

    uno::Reference< frame::XDispatch >  xDisp(xController,UNO_QUERY);
    uno::Reference< util::XURLTransformer > xTrans( util::URLTransformer::create(comphelper::getProcessComponentContext()) );
    if( !xTrans.is() )
        return;

    util::URL aQueryURL;
    aQueryURL.Complete = ".uno:Bib/MenuFilter";
    xTrans->parseStrict( aQueryURL);
    BibToolBarListener* pQuery=new BibTBQueryMenuListener(this, aQueryURL.Complete, nTBC_BT_AUTOFILTER);
    xDisp->addStatusListener(uno::Reference< frame::XStatusListener > (pQuery),aQueryURL);

    for(ToolBox::ImplToolItems::size_type nPos=0;nPos<nCount;nPos++)
    {
        sal_uInt16 nId=GetItemId(nPos);
        if (!nId)
            continue;

        util::URL aURL;
        aURL.Complete = GetItemCommand(nId);
        if(aURL.Complete.isEmpty())
            continue;

        xTrans->parseStrict( aURL );

        css::uno::Reference< css::frame::XStatusListener> xListener;
        if (nId == nTBC_SOURCE)
        {
            xListener=new BibTBListBoxListener(this,aURL.Complete,nId);
        }
        else if (nId == nTBC_QUERY)
        {
            xListener=new BibTBEditListener(this,aURL.Complete,nId);
        }
        else
        {
            xListener=new BibToolBarListener(this,aURL.Complete,nId);
        }

        aListenerArr.push_back( xListener );
        xDisp->addStatusListener(xListener,aURL);
    }
}

void BibToolBar::SetXController(const uno::Reference< frame::XController > & xCtr)
{
    xController=xCtr;
    InitListener();

}

void BibToolBar::Select()
{
    sal_uInt16  nId=GetCurItemId();

    if (nId != nTBC_BT_AUTOFILTER)
    {
        SendDispatch(nId,Sequence<PropertyValue>() );
    }
    else
    {
        Sequence<PropertyValue> aPropVal(2);
        PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
        pPropertyVal[0].Name="QueryText";
        OUString aSelection = pEdQuery->get_text();
        pPropertyVal[0].Value <<= aSelection;

        pPropertyVal[1].Name="QueryField";
        pPropertyVal[1].Value <<= aQueryField;
        SendDispatch(nId,aPropVal);
    }
}

void BibToolBar::SendDispatch(sal_uInt16 nId, const Sequence< PropertyValue >& rArgs)
{
    OUString aCommand = GetItemCommand(nId);

    uno::Reference< frame::XDispatchProvider >  xDSP( xController, UNO_QUERY );

    if( !xDSP.is() || aCommand.isEmpty() )
        return;

    uno::Reference< util::XURLTransformer >  xTrans( util::URLTransformer::create(comphelper::getProcessComponentContext()) );
    if( !xTrans.is() )
        return;

    // load the file
    util::URL aURL;
    aURL.Complete = aCommand;

    xTrans->parseStrict( aURL );

    uno::Reference< frame::XDispatch >  xDisp = xDSP->queryDispatch( aURL, OUString(), frame::FrameSearchFlag::SELF );

    if ( xDisp.is() )
            xDisp->dispatch( aURL, rArgs);

}

void BibToolBar::Click()
{
    sal_uInt16 nId = GetCurItemId();

    vcl::Window* pWin = GetParent();

    if (nId == nTBC_BT_COL_ASSIGN )
    {
        if (pDatMan)
            pDatMan->CreateMappingDialog(pWin ? pWin->GetFrameWeld() : nullptr);
        CheckItem( nId, false );
    }
    else if (nId == nTBC_BT_CHANGESOURCE)
    {
        if (pDatMan)
        {
            OUString sNew = pDatMan->CreateDBChangeDialog(pWin ? pWin->GetFrameWeld() : nullptr);
            if (!sNew.isEmpty())
                pDatMan->setActiveDataSource(sNew);
        }
        CheckItem( nId, false );
    }
}

void BibToolBar::ClearFilterMenu()
{
    pPopupMenu->Clear();
    nMenuId=0;
}
sal_uInt16 BibToolBar::InsertFilterItem(const OUString& aMenuEntry)
{
    nMenuId++;
    pPopupMenu->InsertItem(nMenuId,aMenuEntry);

    return nMenuId;
}
void BibToolBar::SelectFilterItem(sal_uInt16    nId)
{
    pPopupMenu->CheckItem(nId);
    nSelMenuItem=nId;
    aQueryField = MnemonicGenerator::EraseAllMnemonicChars( pPopupMenu->GetItemText(nId) );
}

void BibToolBar::EnableSourceList(bool bFlag)
{
    xSource->set_sensitive(bFlag);
}

void BibToolBar::ClearSourceList()
{
    pLbSource->clear();
}

void BibToolBar::UpdateSourceList(bool bFlag)
{
    if (bFlag)
        pLbSource->thaw();
    else
        pLbSource->freeze();
}

void BibToolBar::InsertSourceEntry(const OUString& aEntry)
{
    pLbSource->append_text(aEntry);
}

void BibToolBar::SelectSourceEntry(const OUString& aStr)
{
    pLbSource->set_active_text(aStr);
}

void BibToolBar::EnableQuery(bool bFlag)
{
    xQuery->set_sensitive(bFlag);
}

void BibToolBar::SetQueryString(const OUString& aStr)
{
    pEdQuery->set_text(aStr);
}

bool BibToolBar::PreNotify( NotifyEvent& rNEvt )
{
    bool bResult = true;

    MouseNotifyEvent nSwitch=rNEvt.GetType();
    if (pEdQuery && pEdQuery->has_focus() && nSwitch == MouseNotifyEvent::KEYINPUT)
    {
        const vcl::KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if(nKey == KEY_RETURN)
        {
            Sequence<PropertyValue> aPropVal(2);
            PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
            pPropertyVal[0].Name = "QueryText";
            OUString aSelection = pEdQuery->get_text();
            pPropertyVal[0].Value <<= aSelection;
            pPropertyVal[1].Name="QueryField";
            pPropertyVal[1].Value <<= aQueryField;
            SendDispatch(nTBC_BT_AUTOFILTER, aPropVal);
            return bResult;
        }

    }

    bResult=ToolBox::PreNotify(rNEvt);

    return bResult;
}

IMPL_LINK_NOARG( BibToolBar, SelHdl, weld::ComboBox&, void )
{
    aIdle.Start();
}

IMPL_LINK_NOARG( BibToolBar, SendSelHdl, Timer*, void )
{
    Sequence<PropertyValue> aPropVal(1);
    PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
    pPropertyVal[0].Name = "DataSourceName";
    OUString aEntry( MnemonicGenerator::EraseAllMnemonicChars( pLbSource->get_active_text() ) );
    pPropertyVal[0].Value <<= aEntry;
    SendDispatch(nTBC_SOURCE, aPropVal);
}

IMPL_LINK_NOARG( BibToolBar, MenuHdl, ToolBox*, void)
{
    sal_uInt16  nId=GetCurItemId();
    if (nId != nTBC_BT_AUTOFILTER)
        return;

    EndSelection();     // before SetDropMode (SetDropMode calls SetItemImage)

    SetItemDown(nTBC_BT_AUTOFILTER, true);
    nId = pPopupMenu->Execute(this, GetItemRect(nTBC_BT_AUTOFILTER));


    if(nId>0)
    {
        pPopupMenu->CheckItem(nSelMenuItem,false);
        pPopupMenu->CheckItem(nId);
        nSelMenuItem=nId;
        aQueryField = MnemonicGenerator::EraseAllMnemonicChars( pPopupMenu->GetItemText(nId) );
        Sequence<PropertyValue> aPropVal(2);
        PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
        pPropertyVal[0].Name = "QueryText";
        OUString aSelection = pEdQuery->get_text();
        pPropertyVal[0].Value <<= aSelection;
        pPropertyVal[1].Name="QueryField";
        pPropertyVal[1].Value <<= aQueryField;
        SendDispatch(nTBC_BT_AUTOFILTER, aPropVal);
    }

    MouseEvent aLeave( Point(), 0, MouseEventModifiers::LEAVEWINDOW | MouseEventModifiers::SYNTHETIC );
    MouseMove( aLeave );
    SetItemDown(nTBC_BT_AUTOFILTER, false);
}

void    BibToolBar::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    for(uno::Reference<frame::XStatusListener> & rListener : aListenerArr)
    {
        rListener->statusChanged(rEvent);
    }
}

void BibToolBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            ApplyImageList();
    ToolBox::DataChanged( rDCEvt );
}

IMPL_LINK_NOARG( BibToolBar, OptionsChanged_Impl, LinkParamNone*, void )
{
    bool bRebuildToolBar = false;
    sal_Int16 eSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
    if ( nSymbolsSize != eSymbolsSize )
    {
        nSymbolsSize = eSymbolsSize;
        bRebuildToolBar = true;
    }
    else if ( nOutStyle != SvtMiscOptions().GetToolboxStyle() )
    {
        nOutStyle = SvtMiscOptions().GetToolboxStyle();
        SetOutStyle( nOutStyle );
        bRebuildToolBar = true;
    }

    if ( bRebuildToolBar )
        RebuildToolbar();
}

IMPL_LINK_NOARG( BibToolBar, SettingsChanged_Impl, VclSimpleEvent&, void )
{
    // Check if toolbar button size have changed and we have to use system settings
    sal_Int16 eSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
    if ( eSymbolsSize != nSymbolsSize )
    {
        nSymbolsSize = eSymbolsSize;
        RebuildToolbar();
    }
}

void BibToolBar::RebuildToolbar()
{
    ApplyImageList();
    // We have to call parent asynchronously as SetSize works also asynchronously!
    Application::PostUserEvent( aLayoutManager );
}

void BibToolBar::ApplyImageList()
{
    SetItemImage(nTBC_BT_AUTOFILTER, Image(StockImage::Yes, nSymbolsSize == SFX_SYMBOLS_SIZE_SMALL ? OUStringLiteral(RID_EXTBMP_AUTOFILTER_SC) : OUStringLiteral(RID_EXTBMP_AUTOFILTER_LC)));
    SetItemImage(nTBC_BT_FILTERCRIT, Image(StockImage::Yes, nSymbolsSize == SFX_SYMBOLS_SIZE_SMALL ? OUStringLiteral(RID_EXTBMP_FILTERCRIT_SC) : OUStringLiteral(RID_EXTBMP_FILTERCRIT_LC)));
    SetItemImage(nTBC_BT_REMOVEFILTER, Image(StockImage::Yes, nSymbolsSize == SFX_SYMBOLS_SIZE_SMALL ? OUStringLiteral(RID_EXTBMP_REMOVE_FILTER_SORT_SC) : OUStringLiteral(RID_EXTBMP_REMOVE_FILTER_SORT_LC)));
    AdjustToolBox();
}

void BibToolBar::AdjustToolBox()
{
    Size aOldSize = GetSizePixel();
    Size aSize = CalcWindowSizePixel();
    if ( !aSize.Width() )
        aSize.setWidth( aOldSize.Width() );
    else if ( !aSize.Height() )
        aSize.setHeight( aOldSize.Height() );

    Size aTbSize = GetSizePixel();
    if (
        (aSize.Width() && aSize.Width() != aTbSize.Width()) ||
        (aSize.Height() && aSize.Height() != aTbSize.Height())
       )
    {
        SetPosSizePixel( GetPosPixel(), aSize );
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
