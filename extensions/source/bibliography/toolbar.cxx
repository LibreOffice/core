/*************************************************************************
 *
 *  $RCSfile: toolbar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 11:41:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif


#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include "bibbeam.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
#define C2U(cChar) OUString::createFromAscii(cChar)


// Konstanten -------------------------------------------------------------


BibToolBarListener::BibToolBarListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId):
        pToolBar(pTB),
        nIndex(nId),
        aCommand(aStr)
{
}

BibToolBarListener::~BibToolBarListener()
{
}

void BibToolBarListener::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& rEvt)throw( ::com::sun::star::uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == aCommand)
    {
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

BibToolBar::BibToolBar(Window* pParent, WinBits nStyle):
    ToolBox(pParent,BibResId(RID_BIB_TOOLBAR)),
    aFtSource(this,WB_VCENTER),
    aLBSource(this,WB_DROPDOWN),
    aFtQuery(this,WB_VCENTER),
    aEdQuery(this),
    nSelMenuItem(0),
    nMenuId(0)
{
    SetStyle(GetStyle()|nStyle);
    SetOutStyle(TOOLBOX_STYLE_FLAT);
    Size aSize=GetSizePixel();
    Size a2Size(GetOutputSizePixel());
    a2Size.Width()=100;
    aLBSource.SetSizePixel(a2Size);
    aLBSource.SetDropDownLineCount(9);
    aLBSource.Show();
    aLBSource.SetSelectHdl(LINK( this, BibToolBar, SelHdl));

    aTimer.SetTimeoutHdl(LINK( this, BibToolBar, SendSelHdl));
    aTimer.SetTimeout(400);

    aMenuTimer.SetTimeoutHdl(LINK( this, BibToolBar, MenuHdl));
    aMenuTimer.SetTimeout(300);

    aEdQuery.SetSizePixel(aLBSource.GetSizePixel());
    aEdQuery.Show();

    XubString aStr=GetItemText(TBC_FT_SOURCE);
    Rectangle aRect=GetItemRect(TBC_FT_SOURCE);
    aFtSource.SetText(aStr);
    aFtSource.SetSizePixel(aRect.GetSize());

    aStr=GetItemText(TBC_FT_QUERY);
    aRect=GetItemRect(TBC_FT_QUERY);
    aFtQuery.SetText(aStr);
    aFtQuery.SetSizePixel(aRect.GetSize());

    SetItemWindow(TBC_FT_SOURCE,&aFtSource);
    SetItemWindow(TBC_LB_SOURCE,&aLBSource);
    SetItemWindow(TBC_FT_QUERY ,&aFtQuery);
    SetItemWindow(TBC_ED_QUERY ,&aEdQuery);


}

BibToolBar::~BibToolBar()
{
}
void BibToolBar::InitListener()
{
    sal_uInt16  nCount=GetItemCount();

    uno::Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
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
            if(nId==TBC_FT_SOURCE || nId==TBC_FT_QUERY) continue;

            util::URL aURL;
            aURL.Complete = GetItemCommand(nId);

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

    if( xDSP.is() && aCommand.len())
    {
        uno::Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();

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

    if(nId==TBC_BT_AUTOFILTER)
    {
        aMenuTimer.Start();
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
    aQueryField = aPopupMenu.GetItemText(nId).EraseAllChars( '~' );
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

IMPL_LINK( BibToolBar, SelHdl, ListBox*, pLb )
{
    aTimer.Start();
    return 0;
}

IMPL_LINK( BibToolBar, SendSelHdl, Timer*,pT)
{
    Sequence<PropertyValue> aPropVal(1);
    PropertyValue* pPropertyVal = (PropertyValue*)aPropVal.getConstArray();
    pPropertyVal[0].Name = C2U("DataSourceName");
    String aEntry( aLBSource.GetSelectEntry().EraseAllChars( '~' ) );
    rtl::OUString aSelection = aEntry;
    pPropertyVal[0].Value <<= aSelection;
    SendDispatch(TBC_LB_SOURCE,aPropVal);

    return 0;
}
//-----------------------------------------------------------------------------
IMPL_LINK( BibToolBar, MenuHdl, Timer*,pT)
{
    if(IsItemDown(TBC_BT_AUTOFILTER))
    {
        EndSelection();     // vor SetDropMode (SetDropMode ruft SetItemImage)

        SetItemDown(TBC_BT_AUTOFILTER,sal_True);
        sal_uInt16 nId = aPopupMenu.Execute(this, GetItemRect(TBC_BT_AUTOFILTER));


        if(nId>0)
        {
            aPopupMenu.CheckItem(nSelMenuItem,sal_False);
            aPopupMenu.CheckItem(nId);
            nSelMenuItem=nId;
            aQueryField = aPopupMenu.GetItemText(nId).EraseAllChars( '~' );
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
