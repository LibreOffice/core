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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <rubydialog.hxx>
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <rubydialog.hrc>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/XRubySelection.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svtools/colorcfg.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::text;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;
using namespace com::sun::star::text;
using namespace com::sun::star::view;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using rtl::OUString;

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

SvxRubyChildWindow::SvxRubyChildWindow( Window* _pParent, USHORT nId,
    SfxBindings* pBindings, SfxChildWinInfo* pInfo) :
    SfxChildWindow(_pParent, nId)
{
    pWindow = new SvxRubyDialog( pBindings, this, _pParent, SVX_RES( RID_SVXDLG_RUBY ) );
    SvxRubyDialog* pDlg = (SvxRubyDialog*) pWindow;

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pDlg->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}

SfxChildWinInfo SvxRubyChildWindow::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

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
void SvxRubyData_Impl::selectionChanged( const EventObject& ) throw (RuntimeException)
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
                                    Window* _pParent, const ResId& rResId ) :
    SfxModelessDialog( pBind, pCW, _pParent, rResId ),
    aLeftFT(this,               ResId(FT_LEFT,*rResId.GetResMgr() )),
    aLeft1ED(this,              ResId(ED_LEFT_1,*rResId.GetResMgr()  )),
    aRightFT(this,              ResId(FT_RIGHT,*rResId.GetResMgr()  )),
    aRight1ED(this,             ResId(ED_RIGHT_1,*rResId.GetResMgr() )),
    aLeft2ED(this,              ResId(ED_LEFT_2,*rResId.GetResMgr()  )),
    aRight2ED(this,             ResId(ED_RIGHT_2,*rResId.GetResMgr() )),
    aLeft3ED(this,              ResId(ED_LEFT_3,*rResId.GetResMgr()  )),
    aRight3ED(this,             ResId(ED_RIGHT_3,*rResId.GetResMgr() )),
    aLeft4ED(this,              ResId(ED_LEFT_4,*rResId.GetResMgr()  )),
    aRight4ED(this,             ResId(ED_RIGHT_4,*rResId.GetResMgr() )),
    aScrollSB(this,             ResId(SB_SCROLL,*rResId.GetResMgr()  )),
    aAutoDetectionCB(this,      ResId(CB_AUTO_DETECT,*rResId.GetResMgr()    )),
    aAdjustFT(this,             ResId(FT_ADJUST,*rResId.GetResMgr()     )),
    aAdjustLB(this,             ResId(LB_ADJUST,*rResId.GetResMgr()     )),
    aPositionFT(this,           ResId(FT_POSITION,*rResId.GetResMgr()     )),
    aPositionLB(this,           ResId(LB_POSITION,*rResId.GetResMgr()     )),
    aCharStyleFT(this,          ResId(FT_CHAR_STYLE,*rResId.GetResMgr()     )),
    aCharStyleLB(this,          ResId(LB_CHAR_STYLE,*rResId.GetResMgr()     )),
    aStylistPB(this,            ResId(PB_STYLIST,*rResId.GetResMgr()        )),
    aPreviewFT(this,            ResId(FT_PREVIEW,*rResId.GetResMgr()        )),
    aPreviewWin(*this,          ResId(WIN_PREVIEW,*rResId.GetResMgr()       )),
    aApplyPB(this,              ResId(PB_APPLY,*rResId.GetResMgr()          )),
    aClosePB(this,              ResId(PB_CLOSE,*rResId.GetResMgr()          )),
    aHelpPB(this,               ResId(PB_HELP,*rResId.GetResMgr()           )),
    nLastPos(0),
    nCurrentEdit(0),
    bModified(FALSE),
    pBindings(pBind)
{
    xImpl = pImpl = new SvxRubyData_Impl;
    FreeResource();
    // automatic detection not yet available
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

SvxRubyDialog::~SvxRubyDialog()
{
    ClearCharStyleList();
    EventObject aEvent;
    xImpl->disposing(aEvent);
}

void SvxRubyDialog::ClearCharStyleList()
{
    for(USHORT i = 0; i < aCharStyleLB.GetEntryCount(); i++)
    {
        void* pData = aCharStyleLB.GetEntryData(i);
        delete (OUString*)pData;
    }
    aCharStyleLB.Clear();
}

BOOL    SvxRubyDialog::Close()
{
    pBindings->GetDispatcher()->Execute( SID_RUBY_DIALOG,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    return TRUE;
}

void SvxRubyDialog::Activate()
{
    SfxModelessDialog::Activate();
    SfxPoolItem* pState = 0;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    sal_Bool bEnable = (eState < SFX_ITEM_AVAILABLE) || !pState || !((SfxBoolItem*)pState)->GetValue();
    aStylistPB.Enable(bEnable);
    //get selection from current view frame
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    Reference< XController > xCtrl = pCurFrm->GetFrame().GetController();
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
                    OSL_FAIL("exception in style access");
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

void    SvxRubyDialog::Deactivate()
{
    SfxModelessDialog::Deactivate();
}

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
    long nTempLastPos = GetLastPos();
    for(int i = 0; i < 8; i+=2)
    {
        if(aEditArr[i]->IsEnabled() &&
            (aEditArr[i]->GetText() != aEditArr[i]->GetSavedValue() ||
            aEditArr[i + 1]->GetText() != aEditArr[i + 1]->GetSavedValue()))
        {
            Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
            DBG_ASSERT(aRubyValues.getLength() > (i / 2 + nTempLastPos), "wrong index" );
            SetModified(TRUE);
            Sequence<PropertyValue> &rProps = aRubyValues.getArray()[i / 2 + nTempLastPos];
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
            if(nAdjust > -2 &&
                pProps[nProp].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cRubyAdjust)))
            {
                sal_Int16 nTmp = sal_Int16();
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
                else if( (!nPosition && !bTmp) || (nPosition == 1 && bTmp)  )
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
    if(!nLen || (bCharStyleEqual && !sCharStyleName.getLength()))
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

void    SvxRubyDialog::GetCurrentText(String& rBase, String& rRuby)
{
    rBase = aEditArr[nCurrentEdit * 2]->GetText();
    rRuby = aEditArr[nCurrentEdit * 2 + 1]->GetText();
}

IMPL_LINK(SvxRubyDialog, ScrollHdl_Impl, ScrollBar*, pScroll)
{
    long nPos = pScroll->GetThumbPos();
    if(GetLastPos() != nPos)
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
            OSL_FAIL("Exception caught");
        }
    }
    return 0;
}

IMPL_LINK(SvxRubyDialog, CloseHdl_Impl, PushButton*, EMPTYARG)
{
    Close();
    return 0;
}

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

IMPL_LINK(SvxRubyDialog, AutomaticHdl_Impl, CheckBox*, pBox)
{
    pImpl->UpdateRubyValues(pBox->IsChecked());
    Update();
    return 0;
}

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

IMPL_LINK(SvxRubyDialog, CharStyleHdl_Impl, ListBox*, EMPTYARG )
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

void RubyPreview::Paint( const Rectangle& /* rRect */ )
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

void RubyEdit::GetFocus()
{
    GetModifyHdl().Call(this);
    Edit::GetFocus();
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
