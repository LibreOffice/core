/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbar.cxx,v $
 * $Revision: 1.22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <datman.hxx>
#include <tools/debug.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include "bibbeam.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"

#ifndef BIBTOOLS_HXX
#include "bibtools.hxx"
#endif
#include <vos/mutex.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
#define C2U(cChar) OUString::createFromAscii(cChar)


// Konstanten -------------------------------------------------------------


BibToolBarListener::BibToolBarListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId):
        nIndex(nId),
        aCommand(aStr),
        pToolBar(pTB)
{
}

BibToolBarListener::~BibToolBarListener()
{
}

void BibToolBarListener::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& rEvt)throw( ::com::sun::star::uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == aCommand)
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        pToolBar->EnableItem(nIndex,rEvt.IsEnabled);

        ::com::sun::star::uno::Any aState=rEvt.State;
        if(aState.getValueType()==::getBooleanCppuType())
        {
            sal_Bool bChecked= *(sal_Bool*)aState.getValue();
            pToolBar->CheckItem(nIndex, bChecked);
        }

        /*
        rtl::OUString FeatureDescriptor;
        sal_Bool Requery;
        ::com::sun::star::uno::Any State;*/
    }
};

sal_uInt16 BibToolBarListener::GetIndex()
{
    return nIndex;
}
void BibToolBarListener::SetIndex(sal_uInt16 nId)
{
    nIndex=nId;
}

rtl::OUString   BibToolBarListener::GetCommand()
{
    return aCommand;
}
void BibToolBarListener::SetCommand(const rtl::OUString& aStr)
{
    aCommand=aStr;
}

BibTBListBoxListener::BibTBListBoxListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBListBoxListener::~BibTBListBoxListener()
{
}

void BibTBListBoxListener::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& rEvt)throw( ::com::sun::star::uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        pToolBar->EnableSourceList(rEvt.IsEnabled);

        Any aState = rEvt.State;
        if(aState.getValueType() == ::getCppuType((Sequence<rtl::OUString>*)0))
        {
            pToolBar->UpdateSourceList(sal_False);
            pToolBar->ClearSourceList();

            Sequence<rtl::OUString>* pStringSeq = (Sequence<rtl::OUString>*)aState.getValue();
            const rtl::OUString* pStringArray = (const rtl::OUString*)pStringSeq->getConstArray();

            sal_uInt32 nCount = pStringSeq->getLength();
            XubString aEntry;
            for( sal_uInt32 i=0; i<nCount; i++ )
            {
                aEntry = String(pStringArray[i]);
                pToolBar->InsertSourceEntry(aEntry);
            }
            pToolBar->UpdateSourceList(sal_True);
        }

        XubString aStr = String(rEvt.FeatureDescriptor);
        pToolBar->SelectSourceEntry(aStr);
    }
};

BibTBQueryMenuListener::BibTBQueryMenuListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBQueryMenuListener::~BibTBQueryMenuListener()
{
}

void BibTBQueryMenuListener::statusChanged(const frame::FeatureStateEvent& rEvt)throw( uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        pToolBar->EnableSourceList(rEvt.IsEnabled);

        uno::Any aState=rEvt.State;
        if(aState.getValueType()==::getCppuType((Sequence<rtl::OUString>*)0))
        {
            pToolBar->ClearFilterMenu();

            Sequence<rtl::OUString>* pStringSeq = (Sequence<rtl::OUString>*) aState.getValue();
            const rtl::OUString* pStringArray = (const rtl::OUString*)pStringSeq->getConstArray();

            sal_uInt32 nCount = pStringSeq->getLength();
            for( sal_uInt32 i=0; i<nCount; i++ )
            {
                sal_uInt16 nID=pToolBar->InsertFilterItem(String(pStringArray[i]));
                if(pStringArray[i]==rEvt.FeatureDescriptor)
                {
//                  XubString aStr = rEvt.FeatureDescriptor;
                    pToolBar->SelectFilterItem(nID);
                }
            }
        }
    }
};

BibTBEditListener::BibTBEditListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId):
    BibToolBarListener(pTB,aStr,nId)
{
}

BibTBEditListener::~BibTBEditListener()
{
}

void BibTBEditListener::statusChanged(const frame::FeatureStateEvent& rEvt)throw( uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == GetCommand())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        pToolBar->EnableQuery(rEvt.IsEnabled);

        uno::Any aState=rEvt.State;
        if(aState.getValueType()==::getCppuType((const OUString*)0))
        {
            String aStr = String(*(OUString*) aState.getValue());
            pToolBar->SetQueryString(aStr);
        }
    }
}

SV_IMPL_PTRARR( BibToolBarListenerArr, BibToolBarListenerPtr);

BibToolBar::BibToolBar(Window* pParent, Link aLink, WinBits nStyle):
    ToolBox(pParent,BibResId(RID_BIB_TOOLBAR)),
    aImgLst(BibResId(  RID_TOOLBAR_IMGLIST     )),
    aImgLstHC(BibResId(RID_TOOLBAR_IMGLIST_HC  )),
    aBigImgLst(BibResId( RID_TOOLBAR_BIGIMGLIST )),
    aBigImgLstHC(BibResId( RID_TOOLBAR_BIGIMGLIST_HC )),
    aFtSource(this,WB_VCENTER),
    aLBSource(this,WB_DROPDOWN),
    aFtQuery(this,WB_VCENTER),
    aEdQuery(this),
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
    Size aSize=GetSizePixel();
    Size a2Size(GetOutputSizePixel());
    a2Size.Width()=100;
    aLBSource.SetSizePixel(a2Size);
    aLBSource.SetDropDownLineCount(9);
    aLBSource.Show();
    aLBSource.SetSelectHdl(LINK( this, BibToolBar, SelHdl));

    SvtMiscOptions().AddListener( LINK( this, BibToolBar, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, BibToolBar, SettingsChanged_Impl ) );

    aTimer.SetTimeoutHdl(LINK( this, BibToolBar, SendSelHdl));
    aTimer.SetTimeout(400);

    SetDropdownClickHdl( LINK( this, BibToolBar, MenuHdl));

    aEdQuery.SetSizePixel(aLBSource.GetSizePixel());
    aEdQuery.Show();

    XubString aStr=GetItemText(TBC_FT_SOURCE);
    Rectangle aRect=GetItemRect(TBC_FT_SOURCE);
    aFtSource.SetText(aStr);
    aFtSource.SetSizePixel(aRect.GetSize());
    aFtSource.SetBackground(Wallpaper( COL_TRANSPARENT ));

    aStr=GetItemText(TBC_FT_QUERY);
    aRect=GetItemRect(TBC_FT_QUERY);
    aFtQuery.SetText(aStr);
    aFtQuery.SetSizePixel(aRect.GetSize());
    aFtQuery.SetBackground(Wallpaper( COL_TRANSPARENT ));

    SetItemWindow(TBC_FT_SOURCE,&aFtSource);
    SetItemWindow(TBC_LB_SOURCE,&aLBSource);
    SetItemWindow(TBC_FT_QUERY ,&aFtQuery);
    SetItemWindow(TBC_ED_QUERY ,&aEdQuery);

    ::bib::AddToTaskPaneList( this );
}

BibToolBar::~BibToolBar()
{
    SvtMiscOptions().RemoveListener( LINK( this, BibToolBar, OptionsChanged_Impl ) );
    Application::RemoveEventListener( LINK( this, BibToolBar, SettingsChanged_Impl ) );
    ::bib::RemoveFromTaskPaneList( this );
}

void BibToolBar::InitListener()
{
    sal_uInt16  nCount=GetItemCount();

    uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< frame::XDispatch >  xDisp(xController,UNO_QUERY);

    uno::Reference< util::XURLTransformer >  xTrans ( xMgr->createInstance( C2U("com.sun.star.util.URLTransformer") ), UNO_QUERY );
    if( xTrans.is() )
    {
        util::URL aQueryURL;
        aQueryURL.Complete = C2U(".uno:Bib/MenuFilter");
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
            if(!aURL.Complete.getLength())
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

            BibToolBarListenerPtr pxInsert = new Reference<frame::XStatusListener>;
            (*pxInsert) = pListener;
            aListenerArr.Insert( pxInsert, aListenerArr.Count() );
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
        PropertyValue* pPropertyVal = (PropertyValue*)aPropVal.getConstArray();
        pPropertyVal[0].Name=C2U("QueryText");
        rtl::OUString aSelection = aEdQuery.GetText();
        pPropertyVal[0].Value <<= aSelection;

        pPropertyVal[1].Name=C2U("QueryField");
        pPropertyVal[1].Value <<= aQueryField;
        SendDispatch(nId,aPropVal);
    }
}

void BibToolBar::SendDispatch(sal_uInt16 nId, const Sequence< PropertyValue >& rArgs)
{
    rtl::OUString aCommand = GetItemCommand(nId);

    uno::Reference< frame::XDispatchProvider >  xDSP( xController, UNO_QUERY );

    if( xDSP.is() && aCommand.getLength())
    {
        uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();

        uno::Reference< util::XURLTransformer >  xTrans ( xMgr->createInstance( C2U("com.sun.star.util.URLTransformer") ), UNO_QUERY );
        if( xTrans.is() )
        {
            // Datei laden
            util::URL aURL;
            aURL.Complete = aCommand;

            xTrans->parseStrict( aURL );

            uno::Reference< frame::XDispatch >  xDisp = xDSP->queryDispatch( aURL, rtl::OUString(), frame::FrameSearchFlag::SELF );

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
        CheckItem( nId, FALSE );
    }
    else if(nId == TBC_BT_CHANGESOURCE)
    {
        if(pDatMan)
        {
            OUString sNew = pDatMan->CreateDBChangeDialog(GetParent());
            if(sNew.getLength())
                pDatMan->setActiveDataSource(sNew);
        }
        CheckItem( nId, FALSE );
    }
}

void BibToolBar::ClearFilterMenu()
{
    aPopupMenu.Clear();
    nMenuId=0;
}
sal_uInt16 BibToolBar::InsertFilterItem(const XubString& aMenuEntry)
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

void BibToolBar::EnableSourceList(sal_Bool bFlag)
{
    aFtSource.Enable(bFlag);
    aLBSource.Enable(bFlag);
}

void BibToolBar::ClearSourceList()
{
    aLBSource.Clear();
}

void BibToolBar::UpdateSourceList(sal_Bool bFlag)
{
    aLBSource.SetUpdateMode(bFlag);
}

void BibToolBar::InsertSourceEntry(const XubString& aEntry, sal_uInt16 nPos)
{
    aLBSource.InsertEntry(aEntry, nPos);
}

void BibToolBar::SelectSourceEntry(const XubString& aStr)
{
    aLBSource.SelectEntry(aStr);
}

void BibToolBar::EnableQuery(sal_Bool bFlag)
{
    aFtQuery.Enable(bFlag);
    aEdQuery.Enable(bFlag);
}

void BibToolBar::SetQueryString(const XubString& aStr)
{
    aEdQuery.SetText(aStr);
}


long BibToolBar::PreNotify( NotifyEvent& rNEvt )
{
    long nResult=sal_True;

    sal_uInt16 nSwitch=rNEvt.GetType();
    if(aEdQuery.HasFocus() && nSwitch==EVENT_KEYINPUT)// || nSwitch==EVENT_KEYUP)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if(nKey == KEY_RETURN)
        {
            Sequence<PropertyValue> aPropVal(2);
            PropertyValue* pPropertyVal = (PropertyValue*)aPropVal.getConstArray();
            pPropertyVal[0].Name = C2U("QueryText");
            rtl::OUString aSelection = aEdQuery.GetText();
            pPropertyVal[0].Value <<= aSelection;
            pPropertyVal[1].Name=C2U("QueryField");
            pPropertyVal[1].Value <<= aQueryField;
            SendDispatch(TBC_BT_AUTOFILTER,aPropVal);
            return nResult;
        }

    }

    nResult=ToolBox::PreNotify(rNEvt);

    return nResult;
}

IMPL_LINK( BibToolBar, SelHdl, ListBox*, /*pLb*/ )
{
    aTimer.Start();
    return 0;
}

IMPL_LINK( BibToolBar, SendSelHdl, Timer*,/*pT*/)
{
    Sequence<PropertyValue> aPropVal(1);
    PropertyValue* pPropertyVal = (PropertyValue*)aPropVal.getConstArray();
    pPropertyVal[0].Name = C2U("DataSourceName");
    String aEntry( MnemonicGenerator::EraseAllMnemonicChars( aLBSource.GetSelectEntry() ) );
    rtl::OUString aSelection = aEntry;
    pPropertyVal[0].Value <<= aSelection;
    SendDispatch(TBC_LB_SOURCE,aPropVal);

    return 0;
}
//-----------------------------------------------------------------------------
IMPL_LINK( BibToolBar, MenuHdl, ToolBox*, /*pToolbox*/)
{
    sal_uInt16  nId=GetCurItemId();
    if(nId==TBC_BT_AUTOFILTER)
    {
        EndSelection();     // vor SetDropMode (SetDropMode ruft SetItemImage)

        SetItemDown(TBC_BT_AUTOFILTER,sal_True);
        nId = aPopupMenu.Execute(this, GetItemRect(TBC_BT_AUTOFILTER));


        if(nId>0)
        {
            aPopupMenu.CheckItem(nSelMenuItem,sal_False);
            aPopupMenu.CheckItem(nId);
            nSelMenuItem=nId;
            aQueryField = MnemonicGenerator::EraseAllMnemonicChars( aPopupMenu.GetItemText(nId) );
            Sequence<PropertyValue> aPropVal(2);
            PropertyValue* pPropertyVal = (PropertyValue*)aPropVal.getConstArray();
            pPropertyVal[0].Name = C2U("QueryText");
            rtl::OUString aSelection = aEdQuery.GetText();
            pPropertyVal[0].Value <<= aSelection;
            pPropertyVal[1].Name=C2U("QueryField");
            pPropertyVal[1].Value <<= aQueryField;
            SendDispatch(TBC_BT_AUTOFILTER,aPropVal);
        }

        Point aPoint;
        MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
        MouseMove( aLeave );
        SetItemDown(TBC_BT_AUTOFILTER,sal_False);


    }
    return 0;
}
//-----------------------------------------------------------------------------
void    BibToolBar::statusChanged(const frame::FeatureStateEvent& rEvent)
                                            throw( uno::RuntimeException )
{
    for(sal_uInt16 i = 0; i < aListenerArr.Count(); i++)
    {
        BibToolBarListenerPtr pListener = aListenerArr.GetObject(i);
        (*pListener)->statusChanged(rEvent);
    }
}
/* -----------------------------07.05.2002 15:08------------------------------

 ---------------------------------------------------------------------------*/
void BibToolBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();
    ToolBox::DataChanged( rDCEvt );
}
/* -----------------------------07.05.2002 15:09------------------------------
 ---------------------------------------------------------------------------*/

IMPL_LINK( BibToolBar, OptionsChanged_Impl, void*, /*pVoid*/ )
{
    sal_Bool bRebuildToolBar = sal_False;
    sal_Int16 eSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
    if ( nSymbolsSize != eSymbolsSize )
    {
        nSymbolsSize = eSymbolsSize;
        bRebuildToolBar = sal_True;
    }
    else if ( nOutStyle != SvtMiscOptions().GetToolboxStyle() )
    {
        nOutStyle = SvtMiscOptions().GetToolboxStyle();
        SetOutStyle( nOutStyle );
        bRebuildToolBar = sal_True;
    }

    if ( bRebuildToolBar )
        RebuildToolbar();

    return 0L;
}

//-----------------------------------------------------------------------------

IMPL_LINK( BibToolBar, SettingsChanged_Impl, void*, /*pVoid*/ )
{
    // Check if toolbar button size have changed and we have to use system settings
    sal_Int16 eSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
    if ( eSymbolsSize != nSymbolsSize )
    {
        nSymbolsSize = eSymbolsSize;
        RebuildToolbar();
    }

    return 0L;
}

//-----------------------------------------------------------------------------
void BibToolBar::RebuildToolbar()
{
    ApplyImageList();
    // We have to call parent asynchronously as SetSize works also asynchronously!
    Application::PostUserEvent( aLayoutManager, 0 );
}

//-----------------------------------------------------------------------------

void BibToolBar::ApplyImageList()
{
    ImageList& rList = ( nSymbolsSize == SFX_SYMBOLS_SIZE_SMALL ) ?
                       ( GetDisplayBackground().GetColor().IsDark() ? aImgLstHC : aImgLst ) :
                       ( GetDisplayBackground().GetColor().IsDark() ? aBigImgLstHC : aBigImgLst );

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
    if ( aSize.Width() && aSize.Width() != aTbSize.Width() ||
         aSize.Height() && aSize.Height() != aTbSize.Height() )
    {
        SetPosSizePixel( GetPosPixel(), aSize );
        Invalidate();
    }
}
