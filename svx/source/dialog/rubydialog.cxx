/*************************************************************************
 *
 *  $RCSfile: rubydialog.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:01:05 $
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

#pragma hdrstop

#ifndef _SVX_RUBYDLG_HXX_
#include <rubydialog.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#include <dialmgr.hxx>
#include <dialogs.hrc>
#include <rubydialog.hrc>
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XRUBYSELECTION_HPP_
#include <com/sun/star/text/XRubySelection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RUBYADJUST_HPP_
#include <com/sun/star/text/RubyAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::text;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;
using namespace com::sun::star::text;
using namespace com::sun::star::view;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace rtl;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

SFX_IMPL_CHILDWINDOW( SvxRubyChildWindow, SID_RUBY_DIALOG );

static const sal_Char cRubyBaseText[] = "RubyBaseText";
static const sal_Char cRubyText[] = "RubyText";
static const sal_Char cCharacterStyles[] = "CharacterStyles";
static const sal_Char cRubyAdjust[] = "RubyAdjust";
static const sal_Char cRubyIsAbove[] = "RubyIsAbove";
static const sal_Char cDisplayName[] = "DisplayName";
static const sal_Char cRubyCharStyleName[] = "RubyCharStyleName";
static const sal_Char cRubies[] = "Rubies";
/* -----------------------------09.01.01 17:24--------------------------------

 ---------------------------------------------------------------------------*/
SvxRubyChildWindow::SvxRubyChildWindow( Window* pParent, USHORT nId,
    SfxBindings* pBindings, SfxChildWinInfo* pInfo) :
    SfxChildWindow(pParent, nId)
{
    pWindow = new SvxRubyDialog( pBindings, this, pParent, SVX_RES( RID_SVXDLG_RUBY ) );
    SvxRubyDialog* pDlg = (SvxRubyDialog*) pWindow;

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pDlg->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}
/* -----------------------------10.01.01 13:53--------------------------------

 ---------------------------------------------------------------------------*/
SfxChildWinInfo SvxRubyChildWindow::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}
/* -----------------------------09.01.01 17:17--------------------------------

 ---------------------------------------------------------------------------*/
class SvxRubyData_Impl : public cppu::WeakImplHelper1
                                <  ::com::sun::star::view::XSelectionChangeListener >
{
    Reference<XModel>               xModel;
    Reference<XRubySelection>       xSelection;
    Sequence<PropertyValues>        aRubyValues;
    Reference<XController>          xController;
    sal_Bool                        bHasSelectionChanged;
    public:
        SvxRubyData_Impl();
        ~SvxRubyData_Impl();

    void    SetController(Reference<XController> xCtrl);
    Reference<XModel>               GetModel()
                                    {
                                        if(!xController.is())
                                            xModel = 0;
                                        else
                                            xModel = xController->getModel();
                                        return xModel;
                                    }
    sal_Bool                        HasSelectionChanged() const{return bHasSelectionChanged;}
    Reference<XRubySelection>       GetRubySelection()
                                    {
                                        xSelection = Reference<XRubySelection>(xController, UNO_QUERY);
                                        return xSelection;
                                    }
    void                            UpdateRubyValues(sal_Bool bAutoUpdate)
                                    {
                                        if(!xSelection.is())
                                            aRubyValues.realloc(0);
                                        else
                                            aRubyValues = xSelection->getRubyList(bAutoUpdate);
                                        bHasSelectionChanged = sal_False;
                                    }
    Sequence<PropertyValues>&       GetRubyValues() {return aRubyValues;}
    void                            AssertOneEntry();

    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException);

};
//-----------------------------------------------------------------------------
SvxRubyData_Impl::SvxRubyData_Impl() :
    bHasSelectionChanged(sal_False)
{
}
//-----------------------------------------------------------------------------
SvxRubyData_Impl::~SvxRubyData_Impl()
{
}
//-----------------------------------------------------------------------------
void    SvxRubyData_Impl::SetController(Reference<XController> xCtrl)
{
    if(xCtrl.get() != xController.get())
    {
        try
        {
            Reference<XSelectionSupplier> xSelSupp(xController, UNO_QUERY);
            if(xSelSupp.is())
                xSelSupp->removeSelectionChangeListener(this);

            bHasSelectionChanged = sal_True;
            xController = xCtrl;
            xSelSupp = Reference<XSelectionSupplier>(xController, UNO_QUERY);
            if(xSelSupp.is())
                xSelSupp->addSelectionChangeListener(this);
        }
        catch(Exception&)
        {}
    }
}
//-----------------------------------------------------------------------------
void SvxRubyData_Impl::selectionChanged( const EventObject& aEvent ) throw (RuntimeException)
{
    bHasSelectionChanged = sal_True;
}
//-----------------------------------------------------------------------------
void SvxRubyData_Impl::disposing( const EventObject&) throw (RuntimeException)
{
    try
    {
        Reference<XSelectionSupplier> xSelSupp(xController, UNO_QUERY);
        if(xSelSupp.is())
            xSelSupp->removeSelectionChangeListener(this);
    }
    catch(Exception&)
    {}
    xController = 0;
}
//-----------------------------------------------------------------------------
void  SvxRubyData_Impl::AssertOneEntry()
{
    //create one entry
    if(!aRubyValues.getLength())
    {
        aRubyValues.realloc(1);
        Sequence<PropertyValue>& rValues = aRubyValues.getArray()[0];
        rValues.realloc(5);
        PropertyValue* pValues = rValues.getArray();
        pValues[0].Name = C2U(cRubyBaseText);
        pValues[1].Name = C2U(cRubyText);
        pValues[2].Name = C2U(cRubyAdjust);
        pValues[3].Name = C2U(cRubyIsAbove);
        pValues[4].Name = C2U(cRubyCharStyleName);
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SvxRubyDialog::SvxRubyDialog( SfxBindings *pBind, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId ) :
    SfxModelessDialog( pBind, pCW, pParent, rResId ),
    aLeftFT(this,               ResId(FT_LEFT )),
    aRightFT(this,              ResId(FT_RIGHT  )),
    aLeft1ED(this,              ResId(ED_LEFT_1  )),
    aRight1ED(this,             ResId(ED_RIGHT_1 )),
    aLeft2ED(this,              ResId(ED_LEFT_2  )),
    aRight2ED(this,             ResId(ED_RIGHT_2 )),
    aLeft3ED(this,              ResId(ED_LEFT_3  )),
    aRight3ED(this,             ResId(ED_RIGHT_3 )),
    aLeft4ED(this,              ResId(ED_LEFT_4  )),
    aRight4ED(this,             ResId(ED_RIGHT_4 )),
    aScrollSB(this,             ResId(SB_SCROLL  )),
    aAutoDetectionCB(this,      ResId(CB_AUTO_DETECT    )),
    aAdjustFT(this,             ResId(FT_ADJUST     )),
    aAdjustLB(this,             ResId(LB_ADJUST     )),
    aPositionFT(this,           ResId(FT_POSITION     )),
    aPositionLB(this,           ResId(LB_POSITION     )),
    aCharStyleFT(this,          ResId(FT_CHAR_STYLE     )),
    aCharStyleLB(this,          ResId(LB_CHAR_STYLE     )),
    aStylistPB(this,            ResId(PB_STYLIST        )),
    aPreviewFT(this,            ResId(FT_PREVIEW        )),
    aPreviewWin(*this,          ResId(WIN_PREVIEW       )),
    aApplyPB(this,              ResId(PB_APPLY          )),
    aClosePB(this,              ResId(PB_CLOSE          )),
    aHelpPB(this,               ResId(PB_HELP           )),
    pBindings(pBind),
    nLastPos(0),
    nCurrentEdit(0)
{
    xImpl = pImpl = new SvxRubyData_Impl;
    FreeResource();
    //#85638# automatic detection not yet available
    aAutoDetectionCB.Hide();
    aEditArr[0] = &aLeft1ED; aEditArr[1] = &aRight1ED;
    aEditArr[2] = &aLeft2ED; aEditArr[3] = &aRight2ED;
    aEditArr[4] = &aLeft3ED; aEditArr[5] = &aRight3ED;
    aEditArr[6] = &aLeft4ED; aEditArr[7] = &aRight4ED;

    aApplyPB.SetClickHdl(LINK(this, SvxRubyDialog, ApplyHdl_Impl));
    aClosePB.SetClickHdl(LINK(this, SvxRubyDialog, CloseHdl_Impl));
    aStylistPB.SetClickHdl(LINK(this, SvxRubyDialog, StylistHdl_Impl));
    aAutoDetectionCB.SetClickHdl(LINK(this, SvxRubyDialog, AutomaticHdl_Impl));
    aAdjustLB.SetSelectHdl(LINK(this, SvxRubyDialog, AdjustHdl_Impl));
    aPositionLB.SetSelectHdl(LINK(this, SvxRubyDialog, PositionHdl_Impl));
    aCharStyleLB.SetSelectHdl(LINK(this, SvxRubyDialog, CharStyleHdl_Impl));

    Link aScrLk(LINK(this, SvxRubyDialog, ScrollHdl_Impl));
    aScrollSB.SetScrollHdl( aScrLk );
    aScrollSB.SetEndScrollHdl( aScrLk );

    Link aEditLk(LINK(this, SvxRubyDialog, EditModifyHdl_Impl));
    Link aScrollLk(LINK(this, SvxRubyDialog, EditScrollHdl_Impl));
    Link aJumpLk(LINK(this, SvxRubyDialog, EditJumpHdl_Impl));
    for(USHORT i = 0; i < 8; i++)
    {
        aEditArr[i]->SetModifyHdl(aEditLk);
        aEditArr[i]->SetJumpHdl(aJumpLk);
        if(!i || 7 == i)
            aEditArr[i]->SetScrollHdl(aScrollLk);
    }

    UpdateColors();
}
/* -----------------------------09.01.01 17:17--------------------------------

 ---------------------------------------------------------------------------*/
SvxRubyDialog::~SvxRubyDialog()
{
    ClearCharStyleList();
    EventObject aEvent;
    xImpl->disposing(aEvent);
}
/* -----------------------------01.02.01 10:29--------------------------------

 ---------------------------------------------------------------------------*/
void SvxRubyDialog::ClearCharStyleList()
{
    for(USHORT i = 0; i < aCharStyleLB.GetEntryCount(); i++)
    {
        void* pData = aCharStyleLB.GetEntryData(i);
        delete (OUString*)pData;
    }
    aCharStyleLB.Clear();
}
/* -----------------------------09.01.01 17:17--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SvxRubyDialog::Close()
{
    pBindings->GetDispatcher()->Execute( SID_RUBY_DIALOG,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    return TRUE;
}
/* -----------------------------29.01.01 15:26--------------------------------

 ---------------------------------------------------------------------------*/
void SvxRubyDialog::Activate()
{
    SfxModelessDialog::Activate();
    SfxPoolItem* pState = 0;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    sal_Bool bEnable = (eState < SFX_ITEM_AVAILABLE) || !pState || !((SfxBoolItem*)pState)->GetValue();
    aStylistPB.Enable(bEnable);
    //get selection from current view frame
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    Reference< XController > xCtrl = pCurFrm->GetFrame()->GetController();
    pImpl->SetController(xCtrl);
    if(pImpl->HasSelectionChanged())
    {

        Reference< XRubySelection > xRubySel = pImpl->GetRubySelection();
        pImpl->UpdateRubyValues(aAutoDetectionCB.IsChecked());
        EnableControls(xRubySel.is());
        if(xRubySel.is())
        {
            Reference< XModel > xModel = pImpl->GetModel();
            const String sCharStyleSelect = aCharStyleLB.GetSelectEntry();
            ClearCharStyleList();
            Reference<XStyleFamiliesSupplier> xSupplier(xModel, UNO_QUERY);
            if(xSupplier.is())
            {
                try
                {
                    Reference<XNameAccess> xFam = xSupplier->getStyleFamilies();
                    Any aChar = xFam->getByName(C2U(cCharacterStyles));
                    Reference<XNameContainer> xChar;
                    aChar >>= xChar;
                    Reference<XIndexAccess> xCharIdx(xChar, UNO_QUERY);
                    if(xCharIdx.is())
                    {
                        OUString sUIName(C2U(cDisplayName));
                        for(sal_Int32 nStyle = 0; nStyle < xCharIdx->getCount(); nStyle++)
                        {
                            Any aStyle = xCharIdx->getByIndex(nStyle);
                            Reference<XStyle> xStyle;
                            aStyle >>= xStyle;
                            Reference<XPropertySet> xPrSet(xStyle, UNO_QUERY);
                            OUString sName, sCoreName;
                            if(xPrSet.is())
                            {
                                Reference<XPropertySetInfo> xInfo = xPrSet->getPropertySetInfo();
                                if(xInfo->hasPropertyByName(sUIName))
                                {
                                    Any aName = xPrSet->getPropertyValue(sUIName);
                                    aName >>= sName;
                                }
                            }
                            Reference<XNamed> xNamed(xStyle, UNO_QUERY);
                            if(xNamed.is())
                            {
                                sCoreName = xNamed->getName();
                                if(!sName.getLength())
                                    sName = sCoreName;
                            }
                            if(sName.getLength())
                            {
                                USHORT nPos = aCharStyleLB.InsertEntry(sName);
                                aCharStyleLB.SetEntryData( nPos, new OUString(sCoreName) );

                            }
                        }
                    }
                }
                catch(Exception&)
                {
                    DBG_ERROR("exception in style access")
                }
                if(sCharStyleSelect.Len())
                    aCharStyleLB.SelectEntry(sCharStyleSelect);
            }
            aCharStyleLB.Enable(xSupplier.is());
            aCharStyleFT.Enable(xSupplier.is());
        }
        Update();
        aPreviewWin.Invalidate();
    }
}
/* -----------------------------29.01.01 15:26--------------------------------

 ---------------------------------------------------------------------------*/
void    SvxRubyDialog::Deactivate()
{
    SfxModelessDialog::Deactivate();
}
/* -----------------------------30.01.01 15:35--------------------------------

 ---------------------------------------------------------------------------*/
void SvxRubyDialog::SetText(sal_Int32 nPos, Edit& rLeft, Edit& rRight)
{
    OUString sLeft, sRight;
    const Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    BOOL bEnable = aRubyValues.getLength() > nPos;
    if(bEnable)
    {
        const Sequence<PropertyValue> aProps = aRubyValues.getConstArray()[nPos];
        const PropertyValue* pProps = aProps.getConstArray();
        for(sal_Int32 nProp = 0; nProp < aProps.getLength(); nProp++)
        {
            if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyBaseText)))
                pProps[nProp].Value >>= sLeft;
            else if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyText)))
                pProps[nProp].Value >>= sRight;
        }
    }
    else if(!nPos)
        bEnable = TRUE;
    rLeft.Enable(bEnable);
    rRight.Enable(bEnable);
    rLeft.SetText(sLeft);
    rRight.SetText(sRight);
    rLeft.SaveValue();
    rRight.SaveValue();
}
//-----------------------------------------------------------------------------
void SvxRubyDialog::GetText()
{
    long nLastPos = GetLastPos();
    for(int i = 0; i < 8; i+=2)
    {
        if(aEditArr[i]->IsEnabled() &&
            (aEditArr[i]->GetText() != aEditArr[i]->GetSavedValue() ||
            aEditArr[i + 1]->GetText() != aEditArr[i + 1]->GetSavedValue()))
        {
            Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
            DBG_ASSERT(aRubyValues.getLength() > (i / 2 + nLastPos), "wrong index" )
            SetModified(TRUE);
            Sequence<PropertyValue> &rProps = aRubyValues.getArray()[i / 2 + nLastPos];
            PropertyValue* pProps = rProps.getArray();
            for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
            {
                if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyBaseText)))
                    pProps[nProp].Value <<= OUString(aEditArr[i]->GetText());
                else if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyText)))
                    pProps[nProp].Value <<= OUString(aEditArr[i + 1]->GetText());
            }
        }
    }
}
//-----------------------------------------------------------------------------
void SvxRubyDialog::Update()
{
    const Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
    sal_Int32 nLen = aRubyValues.getLength();
    aScrollSB.Enable(nLen > 4);
    aScrollSB.SetRange( Range(0, nLen > 4 ? nLen - 4 : 0));
    aScrollSB.SetThumbPos(0);
    SetLastPos(0);
    SetModified(FALSE);

    sal_Int16 nAdjust = -1;
    sal_Int16 nPosition = -1;
    OUString sCharStyleName, sTmp;
    sal_Bool bCharStyleEqual = sal_True;
    for(sal_Int32 nRuby = 0; nRuby < nLen; nRuby++)
    {
        const Sequence<PropertyValue> &rProps = aRubyValues.getConstArray()[nRuby];
        const PropertyValue* pProps = rProps.getConstArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            sal_Int16 nTmp;
            if(nAdjust > -2 &&
                pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyAdjust)))
            {
                pProps[nProp].Value >>= nTmp;
                if(!nRuby)
                    nAdjust = nTmp;
                else if(nAdjust != nTmp)
                    nAdjust = -2;
            }
            if(nPosition > -2 &&
                pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyIsAbove)))
            {
                sal_Bool bTmp = *(sal_Bool*)pProps[nProp].Value.getValue();
                if(!nRuby)
                    nPosition = bTmp ? 0 : 1;
                else  if(!nPosition && !bTmp || nPosition == 1 && bTmp  )
                    nPosition = -2;
            }
            if(bCharStyleEqual &&
                pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyCharStyleName)))
            {
                pProps[nProp].Value >>= sTmp;
                if(!nRuby)
                    sCharStyleName = sTmp;
                else if(sCharStyleName != sTmp)
                    bCharStyleEqual = sal_False;
            }
        }
    }
    if(!nLen)
    {
        //enable selection if the ruby list is empty
        nAdjust = 0;
        nPosition = 0;
    }
    if(nAdjust > -1)
        aAdjustLB.SelectEntryPos(nAdjust);
    else
        aAdjustLB.SetNoSelection();
    if(nPosition > -1)
        aPositionLB.SelectEntryPos(nPosition ? 1 : 0);
    if(!nLen || bCharStyleEqual && !sCharStyleName.getLength())
        sCharStyleName = C2U(cRubies);
    if(sCharStyleName.getLength())
    {
        for(USHORT i = 0; i < aCharStyleLB.GetEntryCount(); i++)
        {
            const OUString* pCoreName = (const OUString*)aCharStyleLB.GetEntryData(i);
            if(pCoreName && sCharStyleName == *pCoreName)
            {
                aCharStyleLB.SelectEntryPos(i);
                break;
            }
        }
    }
    else
        aCharStyleLB.SetNoSelection();

    ScrollHdl_Impl(&aScrollSB);
}
/* -----------------------------16.02.01 14:01--------------------------------

 ---------------------------------------------------------------------------*/
void    SvxRubyDialog::GetCurrentText(String& rBase, String& rRuby)
{
    rBase = aEditArr[nCurrentEdit * 2]->GetText();
    rRuby = aEditArr[nCurrentEdit * 2 + 1]->GetText();
}
/* -----------------------------31.01.01 14:09--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, ScrollHdl_Impl, ScrollBar*, pScroll)
{
    long nPos = pScroll->GetThumbPos();
    long nLastPos = GetLastPos();
    if(nLastPos != nPos)
    {
        GetText();
    }
    SetText(nPos++, aLeft1ED, aRight1ED);
    SetText(nPos++, aLeft2ED, aRight2ED);
    SetText(nPos++, aLeft3ED, aRight3ED);
    SetText(nPos, aLeft4ED, aRight4ED);
    SetLastPos(nPos - 3);
    aPreviewWin.Invalidate();
    return 0;
}
/* -----------------------------30.01.01 14:48--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, ApplyHdl_Impl, PushButton*, EMPTYARG)
{
    const Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    if(!aRubyValues.getLength())
    {
        AssertOneEntry();
        PositionHdl_Impl(&aPositionLB);
        AdjustHdl_Impl(&aAdjustLB);
        CharStyleHdl_Impl(&aCharStyleLB);
    }
    GetText();
    //reset all edit fields - SaveValue is called
    ScrollHdl_Impl(&aScrollSB);

    Reference<XRubySelection>  xSelection = pImpl->GetRubySelection();
    if(IsModified() && xSelection.is())
    {
        try
        {
            xSelection->setRubyList(aRubyValues, aAutoDetectionCB.IsChecked());
        }
        catch(Exception& )
        {
            DBG_ERROR("Exception caught")
        }
    }
    return 0;
}
/* -----------------------------29.01.01 09:38--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, CloseHdl_Impl, PushButton*, EMPTYARG)
{
    Close();
    return 0;
}
/* -----------------------------29.01.01 15:10--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, StylistHdl_Impl, PushButton*, EMPTYARG)
{
    SfxPoolItem* pState = 0;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    if(eState <= SFX_ITEM_SET || !pState || !((SfxBoolItem*)pState)->GetValue())
    {
        pBindings->GetDispatcher()->Execute( SID_STYLE_DESIGNER,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    }
    return 0;
}
/* -----------------------------30.01.01 15:32--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, AutomaticHdl_Impl, CheckBox*, pBox)
{
    pImpl->UpdateRubyValues(pBox->IsChecked());
    Update();
    return 0;
}
/* -----------------------------31.01.01 16:37--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, AdjustHdl_Impl, ListBox*, pBox)
{
    AssertOneEntry();
    sal_Int16 nAdjust = pBox->GetSelectEntryPos();
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for(sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyAdjust)))
                pProps[nProp].Value <<= nAdjust;
        }
        SetModified(TRUE);
    }
    aPreviewWin.Invalidate();
    return 0;
}
/* -----------------------------01.06.01 10:24--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, PositionHdl_Impl, ListBox*, pBox)
{
    AssertOneEntry();
    sal_Bool bAbove = !pBox->GetSelectEntryPos();
    const Type& rType = ::getBooleanCppuType();
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for(sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyIsAbove)))
                pProps[nProp].Value.setValue(&bAbove, rType);
        }
        SetModified(TRUE);
    }
    aPreviewWin.Invalidate();
    return 0;
}
/* -----------------------------01.02.01 10:06--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, CharStyleHdl_Impl, ListBox*, pBox)
{
    AssertOneEntry();
    OUString sStyleName;
    if(LISTBOX_ENTRY_NOTFOUND != aCharStyleLB.GetSelectEntryPos())
        sStyleName = *(OUString*) aCharStyleLB.GetEntryData(aCharStyleLB.GetSelectEntryPos());
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for(sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if(pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyCharStyleName)))
            {
                pProps[nProp].Value <<= sStyleName;
            }
        }
        SetModified(TRUE);
    }
    return 0;
}
/* -----------------------------16.02.01 08:35--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, EditModifyHdl_Impl, Edit*, pEdit)
{
    for(USHORT i = 0; i < 8; i++)
    {
        if(pEdit == aEditArr[i])
        {
            nCurrentEdit = i / 2;
            break;
        }
    }
    aPreviewWin.Invalidate();
    return 0;
}
/* -----------------------------17.07.01 09:11--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, EditScrollHdl_Impl, sal_Int32*, pParam)
{
    long nRet = 0;
    if(aScrollSB.IsEnabled())
    {
        //scroll forward
        if(*pParam > 0 && (aEditArr[7]->HasFocus() || aEditArr[6]->HasFocus() ))
        {
            if(aScrollSB.GetRangeMax() > aScrollSB.GetThumbPos())
            {
                aScrollSB.SetThumbPos(aScrollSB.GetThumbPos() + 1);
                aEditArr[6]->GrabFocus();
                nRet = 1;
            }
        }
        //scroll backward
        else if(aScrollSB.GetThumbPos() && (aEditArr[0]->HasFocus()||aEditArr[1]->HasFocus()) )
        {
            aScrollSB.SetThumbPos(aScrollSB.GetThumbPos() - 1);
            aEditArr[1]->GrabFocus();
            nRet = 1;
        }
        if(nRet)
            ScrollHdl_Impl(&aScrollSB);
    }
    return nRet;
}
/* -----------------------------20.07.2001 15:18------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxRubyDialog, EditJumpHdl_Impl, sal_Int32*, pParam)
{
    USHORT nIndex = USHRT_MAX;
    for(USHORT i = 0; i < 8; i++)
    {
        if(aEditArr[i]->HasFocus())
            nIndex = i;
    }
    if(nIndex < 8)
    {
        if(*pParam > 0)
        {
            if(nIndex < 6)
                aEditArr[nIndex + 2]->GrabFocus();
            else if( EditScrollHdl_Impl(pParam))
                aEditArr[nIndex]->GrabFocus();
        }
        else
        {
            if(nIndex > 1)
                aEditArr[nIndex - 2]->GrabFocus();
            else if( EditScrollHdl_Impl(pParam))
                aEditArr[nIndex]->GrabFocus();
        }
    }
    return 0;
};
/* -----------------------------19.06.01 11:33--------------------------------

 ---------------------------------------------------------------------------*/
void SvxRubyDialog::AssertOneEntry()
{
    pImpl->AssertOneEntry();
}

// ----------------------------------------------------------------------------

void SvxRubyDialog::UpdateColors( void )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    svtools::ColorConfig        aColorConfig;

    Font                    aFnt( aPreviewWin.GetFont() );

    Color                   aNewTextCol( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    Color                   aNewFillCol( rStyleSettings.GetWindowColor() );

    if( aNewFillCol != aFnt.GetFillColor() || aNewTextCol != aFnt.GetColor() )
    {
        aFnt.SetFillColor( aNewFillCol );
        aFnt.SetColor( aNewTextCol );
        aPreviewWin.SetFont( aFnt );

        aPreviewWin.Invalidate();
    }
}

// ----------------------------------------------------------------------------

void SvxRubyDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModelessDialog::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        UpdateColors();
}

/* -----------------------------29.01.01 15:44--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_MoveBox(long nOffset, Edit& rLeft, Edit& rRight)
{
    Size aLeftSz(rLeft.GetSizePixel());
    Point aRightPos(rRight.GetPosPixel());
    Size aRightSz(rRight.GetSizePixel());
    aLeftSz.Width() += nOffset;
    aRightSz.Width() -= nOffset;
    aRightPos.X() += nOffset;
    rLeft.SetSizePixel(aLeftSz);
    rRight.SetPosSizePixel(aRightPos, aRightSz);

}
/* -----------------------------16.02.01 08:09--------------------------------

 ---------------------------------------------------------------------------*/
RubyPreview::RubyPreview(SvxRubyDialog& rParent, const ResId& rResId) :
        Window(&rParent, rResId),
        rParentDlg(rParent)
{
    SetMapMode(MAP_TWIP);
    Size aWinSize = GetOutputSize();

    Font aFont = GetFont();
    aFont.SetHeight(aWinSize.Height() / 4);
    SetFont(aFont);

    SetBorderStyle( WINDOW_BORDER_MONO );
}
/* -----------------------------29.01.01 14:05--------------------------------

 ---------------------------------------------------------------------------*/
void RubyPreview::Paint( const Rectangle& rRect )
{
    Font aRubyFont = GetFont();
    Font aSaveFont(aRubyFont);
    aRubyFont.SetHeight(aRubyFont.GetHeight() * 70 / 100);

    Size aWinSize = GetOutputSize();
    Rectangle aRect(Point(0, 0), aWinSize);
    SetLineColor();
    SetFillColor( aSaveFont.GetFillColor() );
    DrawRect(aRect);

    String sBaseText, sRubyText;
    rParentDlg.GetCurrentText(sBaseText, sRubyText);

    long nTextHeight = GetTextHeight();
    long nBaseWidth = GetTextWidth(sBaseText);
    SetFont(aRubyFont);
    long nRubyWidth = GetTextWidth(sRubyText);
    SetFont(aSaveFont);

    USHORT nAdjust = rParentDlg.aAdjustLB.GetSelectEntryPos();
    //use center if no adjustment is available
    if(nAdjust > 4)
        nAdjust = 1;

    //which part is stretched ?
    sal_Bool bRubyStretch = nBaseWidth >= nRubyWidth;

    long nCenter = aWinSize.Width() / 2;
    long nLeftStart = nCenter - (bRubyStretch ? (nBaseWidth / 2) : (nRubyWidth / 2));
    long nRightEnd = nCenter + (bRubyStretch ? (nBaseWidth / 2) : (nRubyWidth / 2));

    long nYRuby = aWinSize.Height() / 4 - nTextHeight / 2;
    long nYBase = aWinSize.Height() * 3 / 4 - nTextHeight / 2;

    //use above also if no selection is set
    BOOL bAbove = rParentDlg.aPositionLB.GetSelectEntryPos() != 1;
    if(!bAbove)
    {
        long nTmp = nYRuby;
        nYRuby = nYBase;
        nYBase = nTmp;
    }
    long nYOutput, nOutTextWidth;
    String sOutputText;


    if(bRubyStretch)
    {
        DrawText( Point( nLeftStart , nYBase),  sBaseText );
        nYOutput = nYRuby;
        sOutputText = sRubyText;
        nOutTextWidth = nRubyWidth;
        SetFont(aRubyFont);
    }
    else
    {
        SetFont(aRubyFont);
        DrawText( Point( nLeftStart , nYRuby),  sRubyText );
        nYOutput = nYBase;
        sOutputText = sBaseText;
        nOutTextWidth = nBaseWidth;
        SetFont(aSaveFont);
    }

    switch(nAdjust)
    {
        case RubyAdjust_LEFT:
            DrawText( Point( nLeftStart , nYOutput),  sOutputText );
        break;
        case RubyAdjust_RIGHT:
            DrawText( Point( nRightEnd - nOutTextWidth, nYOutput),  sOutputText );
        break;
        case RubyAdjust_INDENT_BLOCK:
        {
            long nCharWidth = GetTextWidth(String::CreateFromAscii("X"));
            if(nOutTextWidth < (nRightEnd - nLeftStart - nCharWidth))
            {
                nCharWidth /= 2;
                nLeftStart += nCharWidth;
                nRightEnd -= nCharWidth;
            }
        }
        // no break!
        case RubyAdjust_BLOCK:
        if(sOutputText.Len() > 1)
        {
            xub_StrLen nCount = sOutputText.Len();
            long nSpace = ((nRightEnd - nLeftStart) - GetTextWidth(sOutputText)) / (nCount - 1);
            for(xub_StrLen i = 0; i < nCount; i++)
            {
                sal_Unicode cChar = sOutputText.GetChar(i);
                DrawText( Point( nLeftStart , nYOutput),  cChar);
                long nCharWidth = GetTextWidth(cChar);
                nLeftStart += nCharWidth + nSpace;
            }
            break;
        }
        //no break;
        case RubyAdjust_CENTER:
            DrawText( Point( nCenter - nOutTextWidth / 2 , nYOutput),  sOutputText );
        break;
    }
    SetFont(aSaveFont);
}
/* -----------------------------16.02.01 15:12--------------------------------

 ---------------------------------------------------------------------------*/
void RubyEdit::GetFocus()
{
    GetModifyHdl().Call(this);
    Edit::GetFocus();
}
/* -----------------------------17.07.01 09:00--------------------------------

 ---------------------------------------------------------------------------*/
long  RubyEdit::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode&  rKeyCode = pKEvt->GetKeyCode();
        USHORT nMod = rKeyCode.GetModifier();
        USHORT nCode = rKeyCode.GetCode();
        if( nCode == KEY_TAB && (!nMod || KEY_SHIFT == nMod))
        {
            sal_Int32 nParam = KEY_SHIFT == nMod ? -1 : 1;
            if(aScrollHdl.IsSet() && aScrollHdl.Call(&nParam))
                nHandled = 1;
        }
        else if(KEY_UP == nCode || KEY_DOWN == nCode)
        {
            sal_Int32 nParam = KEY_UP == nCode ? -1 : 1;
            aJumpHdl.Call(&nParam);
        }
    }
    if(!nHandled)
        nHandled = Edit::PreNotify(rNEvt);
    return nHandled;
}

