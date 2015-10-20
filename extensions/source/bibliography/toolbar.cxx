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
#include <datman.hxx>
#include <svx/svxids.hrc>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/mnemonic.hxx>
#include "bibbeam.hxx"
#include "bibview.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"

#include "bibtools.hxx"
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


// Konstanten -------------------------------------------------------------


BibToolBarListener::BibToolBarListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
        nIndex(nId),
        aCommand(aStr),
        pToolBar(pTB)
{
}

BibToolBarListener::~BibToolBarListener()
{
}

void BibToolBarListener::statusChanged(const css::frame::FeatureStateEvent& rEvt)throw( css::uno::RuntimeException, std::exception )
{
    if(rEvt.FeatureURL.Complete == aCommand)
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableItem(nIndex,rEvt.IsEnabled);

        css::uno::Any aState=rEvt.State;
        if(aState.getValueType()==cppu::UnoType<bool>::get())
        {
            bool bChecked= *static_cast<sal_Bool const *>(aState.getValue());
            pToolBar->CheckItem(nIndex, bChecked);
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

void BibTBListBoxListener::statusChanged(const css::frame::FeatureStateEvent& rEvt)throw( css::uno::RuntimeException, std::exception )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableSourceList(rEvt.IsEnabled);

        Any aState = rEvt.State;
        if(aState.getValueType() == cppu::UnoType<Sequence<OUString>>::get())
        {
            pToolBar->UpdateSourceList(false);
            pToolBar->ClearSourceList();

            Sequence<OUString> const * pStringSeq = static_cast<Sequence<OUString> const *>(aState.getValue());
            const OUString* pStringArray = pStringSeq->getConstArray();

            sal_uInt32 nCount = pStringSeq->getLength();
            OUString aEntry;
            for( sal_uInt32 i=0; i<nCount; i++ )
            {
                aEntry = pStringArray[i];
                pToolBar->InsertSourceEntry(aEntry);
            }
            pToolBar->UpdateSourceList();
        }

        pToolBar->SelectSourceEntry(rEvt.FeatureDescriptor);
    }
};

BibTBQueryMenuListener::BibTBQueryMenuListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBQueryMenuListener::~BibTBQueryMenuListener()
{
}

void BibTBQueryMenuListener::statusChanged(const frame::FeatureStateEvent& rEvt)throw( uno::RuntimeException, std::exception )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableSourceList(rEvt.IsEnabled);

        uno::Any aState=rEvt.State;
        if(aState.getValueType()==cppu::UnoType<Sequence<OUString>>::get())
        {
            pToolBar->ClearFilterMenu();

            Sequence<OUString> const * pStringSeq = static_cast<Sequence<OUString> const *>(aState.getValue());
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

void BibTBEditListener::statusChanged(const frame::FeatureStateEvent& rEvt)throw( uno::RuntimeException, std::exception )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        SolarMutexGuard aGuard;
        pToolBar->EnableQuery(rEvt.IsEnabled);

        uno::Any aState=rEvt.State;
        if(aState.getValueType()== ::cppu::UnoType<OUString>::get())
        {
            OUString aStr = *static_cast<OUString const *>(aState.getValue());
            pToolBar->SetQueryString(aStr);
        }
    }
}

BibToolBar::BibToolBar(vcl::Window* pParent, Link<void*,void> aLink, WinBits nStyle):
    ToolBox(pParent,BibResId(RID_BIB_TOOLBAR)),
    aImgLst(BibResId(  RID_TOOLBAR_IMGLIST     )),
    aBigImgLst(BibResId( RID_TOOLBAR_BIGIMGLIST )),
    aFtSource(VclPtr<FixedText>::Create(this,WB_VCENTER)),
    aLBSource(VclPtr<ListBox>::Create(this,WB_DROPDOWN)),
    aFtQuery(VclPtr<FixedText>::Create(this,WB_VCENTER)),
    aEdQuery(VclPtr<Edit>::Create(this)),
    nMenuId(0),
    nSelMenuItem(0),
    aLayoutManager( aLink ),
    nSymbolsSize( SFX_SYMBOLS_SIZE_SMALL ),
    nOutStyle( 0 )
{
    SvtMiscOptions aSvtMiscOptions;
    nSymbolsSize = aSvtMiscOptions.GetCurrentSymbolsSize();
    nOutStyle  = aSvtMiscOptions.GetToolboxStyle();

    ApplyImageList();
    SetStyle(GetStyle()|nStyle);
    SetOutStyle(TOOLBOX_STYLE_FLAT);
    Size a2Size(GetOutputSizePixel());
    a2Size.Width()=100;
    aLBSource->SetSizePixel(a2Size);
    aLBSource->SetDropDownLineCount(9);
    aLBSource->Show();
    aLBSource->SetSelectHdl(LINK( this, BibToolBar, SelHdl));

    SvtMiscOptions().AddListenerLink( LINK( this, BibToolBar, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, BibToolBar, SettingsChanged_Impl ) );

    aIdle.SetIdleHdl(LINK( this, BibToolBar, SendSelHdl));
    aIdle.SetPriority(SchedulerPriority::LOWEST);

    SetDropdownClickHdl( LINK( this, BibToolBar, MenuHdl));

    aEdQuery->SetSizePixel(aLBSource->GetSizePixel());
    aEdQuery->Show();

    OUString aStr=GetItemText(TBC_FT_SOURCE);
    aFtSource->SetText(aStr);
    aFtSource->SetSizePixel(aFtSource->get_preferred_size());
    aFtSource->SetBackground(Wallpaper( COL_TRANSPARENT ));

    aStr=GetItemText(TBC_FT_QUERY);
    aFtQuery->SetText(aStr);
    aFtQuery->SetSizePixel(aFtQuery->get_preferred_size());
    aFtQuery->SetBackground(Wallpaper( COL_TRANSPARENT ));

    SetItemWindow(TBC_FT_SOURCE, aFtSource.get());
    SetItemWindow(TBC_LB_SOURCE, aLBSource.get());
    SetItemWindow(TBC_FT_QUERY , aFtQuery.get());
    SetItemWindow(TBC_ED_QUERY , aEdQuery.get());

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
    aFtSource.disposeAndClear();
    aFtQuery.disposeAndClear();
    aEdQuery.disposeAndClear();
    aLBSource.disposeAndClear();
    ToolBox::dispose();
}

void BibToolBar::InitListener()
{
    sal_uInt16  nCount=GetItemCount();

    uno::Reference< frame::XDispatch >  xDisp(xController,UNO_QUERY);
    uno::Reference< util::XURLTransformer > xTrans( util::URLTransformer::create(comphelper::getProcessComponentContext()) );
    if( xTrans.is() )
    {
        util::URL aQueryURL;
        aQueryURL.Complete = ".uno:Bib/MenuFilter";
        xTrans->parseStrict( aQueryURL);
        BibToolBarListener* pQuery=new BibTBQueryMenuListener(this,aQueryURL.Complete,TBC_BT_AUTOFILTER);
        xDisp->addStatusListener(uno::Reference< frame::XStatusListener > (pQuery),aQueryURL);

        for(sal_uInt16 nPos=0;nPos<nCount;nPos++)
        {
            sal_uInt16 nId=GetItemId(nPos);
            if(!nId || nId==TBC_FT_SOURCE || nId==TBC_FT_QUERY)
                continue;

            util::URL aURL;
            aURL.Complete = GetItemCommand(nId);
            if(aURL.Complete.isEmpty())
                continue;

            xTrans->parseStrict( aURL );

            BibToolBarListener* pListener=NULL;
            if(nId==TBC_LB_SOURCE)
            {
                pListener=new BibTBListBoxListener(this,aURL.Complete,nId);
            }
            else if(nId==TBC_ED_QUERY)
            {
                pListener=new BibTBEditListener(this,aURL.Complete,nId);
            }
            else
            {
                pListener=new BibToolBarListener(this,aURL.Complete,nId);
            }

            BibToolBarListenerRef* pxInsert = new uno::Reference<frame::XStatusListener>;
            (*pxInsert) = pListener;
            aListenerArr.push_back( pxInsert );
            xDisp->addStatusListener(uno::Reference< frame::XStatusListener > (pListener),aURL);
        }
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

    if(nId!=TBC_BT_AUTOFILTER)
    {
        SendDispatch(nId,Sequence<PropertyValue>() );
    }
    else
    {
        Sequence<PropertyValue> aPropVal(2);
        PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
        pPropertyVal[0].Name="QueryText";
        OUString aSelection = aEdQuery->GetText();
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

    if( xDSP.is() && !aCommand.isEmpty())
    {
        uno::Reference< util::XURLTransformer >  xTrans( util::URLTransformer::create(comphelper::getProcessComponentContext()) );
        if( xTrans.is() )
        {
            // Datei laden
            util::URL aURL;
            aURL.Complete = aCommand;

            xTrans->parseStrict( aURL );

            uno::Reference< frame::XDispatch >  xDisp = xDSP->queryDispatch( aURL, OUString(), frame::FrameSearchFlag::SELF );

            if ( xDisp.is() )
                    xDisp->dispatch( aURL, rArgs);
        }
    }

}

void BibToolBar::Click()
{
    sal_uInt16  nId=GetCurItemId();

    if(nId == TBC_BT_COL_ASSIGN )
    {
        if(pDatMan)
            pDatMan->CreateMappingDialog(GetParent());
        CheckItem( nId, false );
    }
    else if(nId == TBC_BT_CHANGESOURCE)
    {
        if(pDatMan)
        {
            OUString sNew = pDatMan->CreateDBChangeDialog(GetParent());
            if(!sNew.isEmpty())
                pDatMan->setActiveDataSource(sNew);
        }
        CheckItem( nId, false );
    }
}

void BibToolBar::ClearFilterMenu()
{
    aPopupMenu.Clear();
    nMenuId=0;
}
sal_uInt16 BibToolBar::InsertFilterItem(const OUString& aMenuEntry)
{
    nMenuId++;
    aPopupMenu.InsertItem(nMenuId,aMenuEntry);

    return nMenuId;
}
void BibToolBar::SelectFilterItem(sal_uInt16    nId)
{
    aPopupMenu.CheckItem(nId);
    nSelMenuItem=nId;
    aQueryField = MnemonicGenerator::EraseAllMnemonicChars( aPopupMenu.GetItemText(nId) );
}

void BibToolBar::EnableSourceList(bool bFlag)
{
    aFtSource->Enable(bFlag);
    aLBSource->Enable(bFlag);
}

void BibToolBar::ClearSourceList()
{
    aLBSource->Clear();
}

void BibToolBar::UpdateSourceList(bool bFlag)
{
    aLBSource->SetUpdateMode(bFlag);
}

void BibToolBar::InsertSourceEntry(const OUString& aEntry, sal_Int32 nPos)
{
    aLBSource->InsertEntry(aEntry, nPos);
}

void BibToolBar::SelectSourceEntry(const OUString& aStr)
{
    aLBSource->SelectEntry(aStr);
}

void BibToolBar::EnableQuery(bool bFlag)
{
    aFtQuery->Enable(bFlag);
    aEdQuery->Enable(bFlag);
}

void BibToolBar::SetQueryString(const OUString& aStr)
{
    aEdQuery->SetText(aStr);
}


bool BibToolBar::PreNotify( NotifyEvent& rNEvt )
{
    bool bResult = true;

    MouseNotifyEvent nSwitch=rNEvt.GetType();
    if(aEdQuery->HasFocus() && nSwitch==MouseNotifyEvent::KEYINPUT)
    {
        const vcl::KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if(nKey == KEY_RETURN)
        {
            Sequence<PropertyValue> aPropVal(2);
            PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
            pPropertyVal[0].Name = "QueryText";
            OUString aSelection = aEdQuery->GetText();
            pPropertyVal[0].Value <<= aSelection;
            pPropertyVal[1].Name="QueryField";
            pPropertyVal[1].Value <<= aQueryField;
            SendDispatch(TBC_BT_AUTOFILTER,aPropVal);
            return bResult;
        }

    }

    bResult=ToolBox::PreNotify(rNEvt);

    return bResult;
}

IMPL_LINK_NOARG_TYPED( BibToolBar, SelHdl, ListBox&, void )
{
    aIdle.Start();
}

IMPL_LINK_NOARG_TYPED( BibToolBar, SendSelHdl, Idle*, void )
{
    Sequence<PropertyValue> aPropVal(1);
    PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
    pPropertyVal[0].Name = "DataSourceName";
    OUString aEntry( MnemonicGenerator::EraseAllMnemonicChars( aLBSource->GetSelectEntry() ) );
    OUString aSelection = aEntry;
    pPropertyVal[0].Value <<= aSelection;
    SendDispatch(TBC_LB_SOURCE,aPropVal);
}

IMPL_LINK_NOARG_TYPED( BibToolBar, MenuHdl, ToolBox*, void)
{
    sal_uInt16  nId=GetCurItemId();
    if(nId==TBC_BT_AUTOFILTER)
    {
        EndSelection();     // vor SetDropMode (SetDropMode ruft SetItemImage)

        SetItemDown(TBC_BT_AUTOFILTER,true);
        nId = aPopupMenu.Execute(this, GetItemRect(TBC_BT_AUTOFILTER));


        if(nId>0)
        {
            aPopupMenu.CheckItem(nSelMenuItem,false);
            aPopupMenu.CheckItem(nId);
            nSelMenuItem=nId;
            aQueryField = MnemonicGenerator::EraseAllMnemonicChars( aPopupMenu.GetItemText(nId) );
            Sequence<PropertyValue> aPropVal(2);
            PropertyValue* pPropertyVal = const_cast<PropertyValue*>(aPropVal.getConstArray());
            pPropertyVal[0].Name = "QueryText";
            OUString aSelection = aEdQuery->GetText();
            pPropertyVal[0].Value <<= aSelection;
            pPropertyVal[1].Name="QueryField";
            pPropertyVal[1].Value <<= aQueryField;
            SendDispatch(TBC_BT_AUTOFILTER,aPropVal);
        }

        Point aPoint;
        MouseEvent aLeave( aPoint, 0, MouseEventModifiers::LEAVEWINDOW | MouseEventModifiers::SYNTHETIC );
        MouseMove( aLeave );
        SetItemDown(TBC_BT_AUTOFILTER,false);


    }
}

void    BibToolBar::statusChanged(const frame::FeatureStateEvent& rEvent)
                                            throw( uno::RuntimeException )
{
    for(size_t i = 0; i < aListenerArr.size(); i++)
    {
        BibToolBarListenerRef* pListener = &aListenerArr[i];
        (*pListener)->statusChanged(rEvent);
    }
}

void BibToolBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            ApplyImageList();
    ToolBox::DataChanged( rDCEvt );
}

IMPL_LINK_NOARG_TYPED( BibToolBar, OptionsChanged_Impl, LinkParamNone*, void )
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



IMPL_LINK_NOARG_TYPED( BibToolBar, SettingsChanged_Impl, VclSimpleEvent&, void )
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
    ImageList& rList = ( nSymbolsSize == SFX_SYMBOLS_SIZE_SMALL ) ?
                       ( aImgLst ) :
                       ( aBigImgLst );

    SetItemImage(TBC_BT_AUTOFILTER  , rList.GetImage(SID_FM_AUTOFILTER));
    SetItemImage(TBC_BT_FILTERCRIT  , rList.GetImage(SID_FM_FILTERCRIT));
    SetItemImage(TBC_BT_REMOVEFILTER, rList.GetImage(SID_FM_REMOVE_FILTER_SORT ));
    AdjustToolBox();
}

void BibToolBar::AdjustToolBox()
{
    Size aOldSize = GetSizePixel();
    Size aSize = CalcWindowSizePixel();
    if ( !aSize.Width() )
        aSize.Width() = aOldSize.Width();
    else if ( !aSize.Height() )
        aSize.Height() = aOldSize.Height();

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
