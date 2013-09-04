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

#include <svx/rubydialog.hxx>
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
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
#include <vcl/layout.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::text;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;
using namespace com::sun::star::view;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

SFX_IMPL_CHILDWINDOW( SvxRubyChildWindow, SID_RUBY_DIALOG );

static const sal_Char cRubyBaseText[] = "RubyBaseText";
static const sal_Char cRubyText[] = "RubyText";
static const sal_Char cCharacterStyles[] = "CharacterStyles";
static const sal_Char cRubyAdjust[] = "RubyAdjust";
static const sal_Char cRubyIsAbove[] = "RubyIsAbove";
static const sal_Char cDisplayName[] = "DisplayName";
static const sal_Char cRubyCharStyleName[] = "RubyCharStyleName";
static const sal_Char cRubies[] = "Rubies";

SvxRubyChildWindow::SvxRubyChildWindow( Window* _pParent, sal_uInt16 nId,
    SfxBindings* pBindings, SfxChildWinInfo* pInfo) :
    SfxChildWindow(_pParent, nId)
{
    SvxRubyDialog* pDlg = new SvxRubyDialog(pBindings, this, _pParent);
    pWindow = pDlg;

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
    bool                            bHasSelectionChanged;
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
    bool                            HasSelectionChanged() const{return bHasSelectionChanged;}
    Reference<XRubySelection>       GetRubySelection()
                                    {
                                        xSelection = Reference<XRubySelection>(xController, UNO_QUERY);
                                        return xSelection;
                                    }
    void                            UpdateRubyValues()
                                    {
                                        if(!xSelection.is())
                                            aRubyValues.realloc(0);
                                        else
                                            aRubyValues = xSelection->getRubyList(false);
                                        bHasSelectionChanged = false;
                                    }
    Sequence<PropertyValues>&       GetRubyValues() {return aRubyValues;}
    void                            AssertOneEntry();

    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException);

};
//-----------------------------------------------------------------------------
SvxRubyData_Impl::SvxRubyData_Impl() :
    bHasSelectionChanged(false)
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

            bHasSelectionChanged = true;
            xController = xCtrl;
            xSelSupp = Reference<XSelectionSupplier>(xController, UNO_QUERY);
            if(xSelSupp.is())
                xSelSupp->addSelectionChangeListener(this);
        }
        catch (const Exception&)
        {
        }
    }
}
//-----------------------------------------------------------------------------
void SvxRubyData_Impl::selectionChanged( const EventObject& ) throw (RuntimeException)
{
    bHasSelectionChanged = true;
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
    catch (const Exception&)
    {
    }
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
        pValues[0].Name = cRubyBaseText;
        pValues[1].Name = cRubyText;
        pValues[2].Name = cRubyAdjust;
        pValues[3].Name = cRubyIsAbove;
        pValues[4].Name = cRubyCharStyleName;
    }
}

SvxRubyDialog::SvxRubyDialog(SfxBindings *pBind, SfxChildWindow *pCW,
                                    Window* _pParent)
    : SfxModelessDialog(pBind, pCW, _pParent, "AsianPhoneticGuideDialog",
        "svx/ui/asianphoneticguidedialog.ui")
    , nLastPos(0)
    , nCurrentEdit(0)
    , bModified(false)
    , pBindings(pBind)
{
    xImpl = pImpl = new SvxRubyData_Impl;
    get(m_pLeftFT, "basetextft");
    get(m_pRightFT, "rubytextft");
    get(m_pAdjustLB, "adjustlb");
    get(m_pPositionLB, "positionlb");
    get(m_pCharStyleFT, "styleft");
    get(m_pCharStyleLB, "stylelb");
    m_pCharStyleLB->SetStyle(m_pCharStyleLB->GetStyle() | WB_SORT);
    get(m_pStylistPB, "styles");
    get(m_pApplyPB, "apply");
    get(m_pClosePB, "close");
    get(m_pPreviewWin, "preview");
    m_pPreviewWin->setRubyDialog(this);
    get(m_pScrolledWindow, "scrolledwindow");
    m_pScrollSB = &m_pScrolledWindow->getVertScrollBar();
    get(m_pLeft1ED, "Left1ED");
    get(m_pRight1ED, "Right1ED");
    get(m_pLeft2ED, "Left2ED");
    get(m_pRight2ED, "Right2ED");
    get(m_pLeft3ED, "Left3ED");
    get(m_pRight3ED, "Right3ED");
    get(m_pLeft4ED, "Left4ED");
    get(m_pRight4ED, "Right4ED");
    aEditArr[0] = m_pLeft1ED; aEditArr[1] = m_pRight1ED;
    aEditArr[2] = m_pLeft2ED; aEditArr[3] = m_pRight2ED;
    aEditArr[4] = m_pLeft3ED; aEditArr[5] = m_pRight3ED;
    aEditArr[6] = m_pLeft4ED; aEditArr[7] = m_pRight4ED;

    m_pApplyPB->SetClickHdl(LINK(this, SvxRubyDialog, ApplyHdl_Impl));
    m_pClosePB->SetClickHdl(LINK(this, SvxRubyDialog, CloseHdl_Impl));
    m_pStylistPB->SetClickHdl(LINK(this, SvxRubyDialog, StylistHdl_Impl));
    m_pAdjustLB->SetSelectHdl(LINK(this, SvxRubyDialog, AdjustHdl_Impl));
    m_pPositionLB->SetSelectHdl(LINK(this, SvxRubyDialog, PositionHdl_Impl));
    m_pCharStyleLB->SetSelectHdl(LINK(this, SvxRubyDialog, CharStyleHdl_Impl));

    Link aScrLk(LINK(this, SvxRubyDialog, ScrollHdl_Impl));
    m_pScrollSB->SetScrollHdl( aScrLk );
    m_pScrollSB->SetEndScrollHdl( aScrLk );

    Link aEditLk(LINK(this, SvxRubyDialog, EditModifyHdl_Impl));
    Link aScrollLk(LINK(this, SvxRubyDialog, EditScrollHdl_Impl));
    Link aJumpLk(LINK(this, SvxRubyDialog, EditJumpHdl_Impl));
    for(sal_uInt16 i = 0; i < 8; i++)
    {
        aEditArr[i]->SetModifyHdl(aEditLk);
        aEditArr[i]->SetJumpHdl(aJumpLk);
        if(!i || 7 == i)
            aEditArr[i]->SetScrollHdl(aScrollLk);
    }

    UpdateColors();

    OUString leftLabelName = m_pLeftFT->GetText(), rightLabelName = m_pRightFT->GetText();
    m_pLeft2ED->SetAccessibleName(leftLabelName);
    m_pLeft3ED->SetAccessibleName(leftLabelName);
    m_pLeft4ED->SetAccessibleName(leftLabelName);
    m_pRight2ED->SetAccessibleName(rightLabelName);
    m_pRight3ED->SetAccessibleName(rightLabelName);
    m_pRight4ED->SetAccessibleName(rightLabelName);
}

SvxRubyDialog::~SvxRubyDialog()
{
    ClearCharStyleList();
    EventObject aEvent;
    xImpl->disposing(aEvent);
}

void SvxRubyDialog::ClearCharStyleList()
{
    for(sal_uInt16 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
    {
        void* pData = m_pCharStyleLB->GetEntryData(i);
        delete (OUString*)pData;
    }
    m_pCharStyleLB->Clear();
}

sal_Bool    SvxRubyDialog::Close()
{
    pBindings->GetDispatcher()->Execute( SID_RUBY_DIALOG,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    return sal_True;
}

void SvxRubyDialog::Activate()
{
    SfxModelessDialog::Activate();
    SfxPoolItem* pState = 0;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    sal_Bool bEnable = (eState < SFX_ITEM_AVAILABLE) || !pState || !((SfxBoolItem*)pState)->GetValue();
    delete pState;
    m_pStylistPB->Enable(bEnable);
    //get selection from current view frame
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    Reference< XController > xCtrl = pCurFrm->GetFrame().GetController();
    pImpl->SetController(xCtrl);
    if(pImpl->HasSelectionChanged())
    {

        Reference< XRubySelection > xRubySel = pImpl->GetRubySelection();
        pImpl->UpdateRubyValues();
        EnableControls(xRubySel.is());
        if(xRubySel.is())
        {
            Reference< XModel > xModel = pImpl->GetModel();
            const OUString sCharStyleSelect = m_pCharStyleLB->GetSelectEntry();
            ClearCharStyleList();
            Reference<XStyleFamiliesSupplier> xSupplier(xModel, UNO_QUERY);
            if(xSupplier.is())
            {
                try
                {
                    Reference<XNameAccess> xFam = xSupplier->getStyleFamilies();
                    Any aChar = xFam->getByName(cCharacterStyles);
                    Reference<XNameContainer> xChar;
                    aChar >>= xChar;
                    Reference<XIndexAccess> xCharIdx(xChar, UNO_QUERY);
                    if(xCharIdx.is())
                    {
                        OUString sUIName(cDisplayName);
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
                            if(xStyle.is())
                            {
                                sCoreName = xStyle->getName();
                                if(sName.isEmpty())
                                    sName = sCoreName;
                            }
                            if(!sName.isEmpty())
                            {
                                sal_uInt16 nPos = m_pCharStyleLB->InsertEntry(sName);
                                m_pCharStyleLB->SetEntryData( nPos, new OUString(sCoreName) );

                            }
                        }
                    }
                }
                catch (const Exception&)
                {
                    OSL_FAIL("exception in style access");
                }
                if(!sCharStyleSelect.isEmpty())
                    m_pCharStyleLB->SelectEntry(sCharStyleSelect);
            }
            m_pCharStyleLB->Enable(xSupplier.is());
            m_pCharStyleFT->Enable(xSupplier.is());
        }
        Update();
        m_pPreviewWin->Invalidate();
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
    sal_Bool bEnable = aRubyValues.getLength() > nPos;
    if(bEnable)
    {
        const Sequence<PropertyValue> aProps = aRubyValues.getConstArray()[nPos];
        const PropertyValue* pProps = aProps.getConstArray();
        for(sal_Int32 nProp = 0; nProp < aProps.getLength(); nProp++)
        {
            if ( pProps[nProp].Name == cRubyBaseText )
                pProps[nProp].Value >>= sLeft;
            else if ( pProps[nProp].Name == cRubyText )
                pProps[nProp].Value >>= sRight;
        }
    }
    else if(!nPos)
        bEnable = sal_True;
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
            SetModified(sal_True);
            Sequence<PropertyValue> &rProps = aRubyValues.getArray()[i / 2 + nTempLastPos];
            PropertyValue* pProps = rProps.getArray();
            for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
            {
                if ( pProps[nProp].Name == cRubyBaseText )
                    pProps[nProp].Value <<= OUString(aEditArr[i]->GetText());
                else if ( pProps[nProp].Name == cRubyText )
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
    m_pScrollSB->Enable(nLen > 4);
    m_pScrollSB->SetRange( Range(0, nLen > 4 ? nLen - 4 : 0));
    m_pScrollSB->SetThumbPos(0);
    SetLastPos(0);
    SetModified(sal_False);

    sal_Int16 nAdjust = -1;
    sal_Int16 nPosition = -1;
    OUString sCharStyleName, sTmp;
    bool bCharStyleEqual = true;
    for(sal_Int32 nRuby = 0; nRuby < nLen; nRuby++)
    {
        const Sequence<PropertyValue> &rProps = aRubyValues.getConstArray()[nRuby];
        const PropertyValue* pProps = rProps.getConstArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if(nAdjust > -2 && pProps[nProp].Name == cRubyAdjust )
            {
                sal_Int16 nTmp = sal_Int16();
                pProps[nProp].Value >>= nTmp;
                if(!nRuby)
                    nAdjust = nTmp;
                else if(nAdjust != nTmp)
                    nAdjust = -2;
            }
            if(nPosition > -2 && pProps[nProp].Name == cRubyIsAbove )
            {
                sal_Bool bTmp = *(sal_Bool*)pProps[nProp].Value.getValue();
                if(!nRuby)
                    nPosition = bTmp ? 0 : 1;
                else if( (!nPosition && !bTmp) || (nPosition == 1 && bTmp)  )
                    nPosition = -2;
            }
            if(bCharStyleEqual && pProps[nProp].Name == cRubyCharStyleName )
            {
                pProps[nProp].Value >>= sTmp;
                if(!nRuby)
                    sCharStyleName = sTmp;
                else if(sCharStyleName != sTmp)
                    bCharStyleEqual = false;
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
        m_pAdjustLB->SelectEntryPos(nAdjust);
    else
        m_pAdjustLB->SetNoSelection();
    if(nPosition > -1)
        m_pPositionLB->SelectEntryPos(nPosition ? 1 : 0);
    if(!nLen || (bCharStyleEqual && sCharStyleName.isEmpty()))
        sCharStyleName = cRubies;
    if(!sCharStyleName.isEmpty())
    {
        for(sal_uInt16 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
        {
            const OUString* pCoreName = (const OUString*)m_pCharStyleLB->GetEntryData(i);
            if(pCoreName && sCharStyleName == *pCoreName)
            {
                m_pCharStyleLB->SelectEntryPos(i);
                break;
            }
        }
    }
    else
        m_pCharStyleLB->SetNoSelection();

    ScrollHdl_Impl(m_pScrollSB);
}

void    SvxRubyDialog::GetCurrentText(OUString& rBase, OUString& rRuby)
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
    SetText(nPos++, *m_pLeft1ED, *m_pRight1ED);
    SetText(nPos++, *m_pLeft2ED, *m_pRight2ED);
    SetText(nPos++, *m_pLeft3ED, *m_pRight3ED);
    SetText(nPos, *m_pLeft4ED, *m_pRight4ED);
    SetLastPos(nPos - 3);
    m_pPreviewWin->Invalidate();
    return 0;
}

IMPL_LINK_NOARG(SvxRubyDialog, ApplyHdl_Impl)
{
    const Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    if(!aRubyValues.getLength())
    {
        AssertOneEntry();
        PositionHdl_Impl(m_pPositionLB);
        AdjustHdl_Impl(m_pAdjustLB);
        CharStyleHdl_Impl(m_pCharStyleLB);
    }
    GetText();
    //reset all edit fields - SaveValue is called
    ScrollHdl_Impl(m_pScrollSB);

    Reference<XRubySelection>  xSelection = pImpl->GetRubySelection();
    if(IsModified() && xSelection.is())
    {
        try
        {
            xSelection->setRubyList(aRubyValues, false);
        }
        catch (const Exception&)
        {
            OSL_FAIL("Exception caught");
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SvxRubyDialog, CloseHdl_Impl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(SvxRubyDialog, StylistHdl_Impl)
{
    SfxPoolItem* pState = 0;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    if(eState <= SFX_ITEM_SET || !pState || !((SfxBoolItem*)pState)->GetValue())
    {
        pBindings->GetDispatcher()->Execute( SID_STYLE_DESIGNER,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    }
    delete pState;
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
            if ( pProps[nProp].Name == cRubyAdjust )
                pProps[nProp].Value <<= nAdjust;
        }
        SetModified(sal_True);
    }
    m_pPreviewWin->Invalidate();
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
            if ( pProps[nProp].Name == cRubyIsAbove )
                pProps[nProp].Value.setValue(&bAbove, rType);
        }
        SetModified(sal_True);
    }
    m_pPreviewWin->Invalidate();
    return 0;
}

IMPL_LINK_NOARG(SvxRubyDialog, CharStyleHdl_Impl)
{
    AssertOneEntry();
    OUString sStyleName;
    if(LISTBOX_ENTRY_NOTFOUND != m_pCharStyleLB->GetSelectEntryPos())
        sStyleName = *(OUString*) m_pCharStyleLB->GetEntryData(m_pCharStyleLB->GetSelectEntryPos());
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for(sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for(sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if ( pProps[nProp].Name == cRubyCharStyleName )
            {
                pProps[nProp].Value <<= sStyleName;
            }
        }
        SetModified(sal_True);
    }
    return 0;
}

IMPL_LINK(SvxRubyDialog, EditModifyHdl_Impl, Edit*, pEdit)
{
    for(sal_uInt16 i = 0; i < 8; i++)
    {
        if(pEdit == aEditArr[i])
        {
            nCurrentEdit = i / 2;
            break;
        }
    }
    m_pPreviewWin->Invalidate();
    return 0;
}

IMPL_LINK(SvxRubyDialog, EditScrollHdl_Impl, sal_Int32*, pParam)
{
    long nRet = 0;
    if(m_pScrollSB->IsEnabled())
    {
        //scroll forward
        if(*pParam > 0 && (aEditArr[7]->HasFocus() || aEditArr[6]->HasFocus() ))
        {
            if(m_pScrollSB->GetRangeMax() > m_pScrollSB->GetThumbPos())
            {
                m_pScrollSB->SetThumbPos(m_pScrollSB->GetThumbPos() + 1);
                aEditArr[6]->GrabFocus();
                nRet = 1;
            }
        }
        //scroll backward
        else if(m_pScrollSB->GetThumbPos() && (aEditArr[0]->HasFocus()||aEditArr[1]->HasFocus()) )
        {
            m_pScrollSB->SetThumbPos(m_pScrollSB->GetThumbPos() - 1);
            aEditArr[1]->GrabFocus();
            nRet = 1;
        }
        if(nRet)
            ScrollHdl_Impl(m_pScrollSB);
    }
    return nRet;
}

IMPL_LINK(SvxRubyDialog, EditJumpHdl_Impl, sal_Int32*, pParam)
{
    sal_uInt16 nIndex = USHRT_MAX;
    for(sal_uInt16 i = 0; i < 8; i++)
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

    Font                    aFnt( m_pPreviewWin->GetFont() );

    Color                   aNewTextCol( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    Color                   aNewFillCol( rStyleSettings.GetWindowColor() );

    if( aNewFillCol != aFnt.GetFillColor() || aNewTextCol != aFnt.GetColor() )
    {
        aFnt.SetFillColor( aNewFillCol );
        aFnt.SetColor( aNewTextCol );
        m_pPreviewWin->SetFont( aFnt );

        m_pPreviewWin->Invalidate();
    }
}

// ----------------------------------------------------------------------------

void SvxRubyDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModelessDialog::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        UpdateColors();
}

void SvxRubyDialog::EnableControls(bool bEnable)
{
    get_content_area()->Enable(bEnable);
    m_pApplyPB->Enable(bEnable);
}

RubyPreview::RubyPreview(Window *pParent)
    : Window(pParent, WB_BORDER)
    , m_pParentDlg(NULL)
{
    SetMapMode(MAP_TWIP);
    SetBorderStyle( WINDOW_BORDER_MONO );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRubyPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new RubyPreview(pParent);
}

void RubyPreview::Paint( const Rectangle& /* rRect */ )
{
    Size aWinSize = GetOutputSize();

    Font aSaveFont = GetFont();
    aSaveFont.SetHeight(aWinSize.Height() / 4);
    SetFont(aSaveFont);

    Rectangle aRect(Point(0, 0), aWinSize);
    SetLineColor();
    SetFillColor( aSaveFont.GetFillColor() );
    DrawRect(aRect);

    OUString sBaseText, sRubyText;
    m_pParentDlg->GetCurrentText(sBaseText, sRubyText);

    long nTextHeight = GetTextHeight();
    long nBaseWidth = GetTextWidth(sBaseText);

    Font aRubyFont(aSaveFont);
    aRubyFont.SetHeight(aRubyFont.GetHeight() * 70 / 100);
    SetFont(aRubyFont);
    long nRubyWidth = GetTextWidth(sRubyText);
    SetFont(aSaveFont);

    sal_uInt16 nAdjust = m_pParentDlg->m_pAdjustLB->GetSelectEntryPos();
    //use center if no adjustment is available
    if(nAdjust > 4)
        nAdjust = 1;

    //which part is stretched ?
    bool bRubyStretch = nBaseWidth >= nRubyWidth;

    long nCenter = aWinSize.Width() / 2;
    long nLeftStart = nCenter - (bRubyStretch ? (nBaseWidth / 2) : (nRubyWidth / 2));
    long nRightEnd = nCenter + (bRubyStretch ? (nBaseWidth / 2) : (nRubyWidth / 2));

    long nYRuby = aWinSize.Height() / 4 - nTextHeight / 2;
    long nYBase = aWinSize.Height() * 3 / 4 - nTextHeight / 2;

    //use above also if no selection is set
    bool bAbove = m_pParentDlg->m_pPositionLB->GetSelectEntryPos() != 1;
    if(!bAbove)
    {
        long nTmp = nYRuby;
        nYRuby = nYBase;
        nYBase = nTmp;
    }
    long nYOutput, nOutTextWidth;
    OUString sOutputText;


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
            long nCharWidth = GetTextWidth(OUString("X"));
            if(nOutTextWidth < (nRightEnd - nLeftStart - nCharWidth))
            {
                nCharWidth /= 2;
                nLeftStart += nCharWidth;
                nRightEnd -= nCharWidth;
            }
        }
        // no break!
        case RubyAdjust_BLOCK:
        if(sOutputText.getLength() > 1)
        {
            sal_Int32 nCount = sOutputText.getLength();
            long nSpace = ((nRightEnd - nLeftStart) - GetTextWidth(sOutputText)) / (nCount - 1);
            for(sal_Int32 i = 0; i < nCount; i++)
            {
                OUString sChar(sOutputText[i]);
                DrawText( Point( nLeftStart , nYOutput),  sChar);
                long nCharWidth = GetTextWidth(sChar);
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

Size RubyPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(215, 50), MapMode(MAP_APPFONT));
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
        sal_uInt16 nMod = rKeyCode.GetModifier();
        sal_uInt16 nCode = rKeyCode.GetCode();
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

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRubyEdit(Window *pParent, VclBuilder::stringmap &)
{
    return new RubyEdit(pParent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
