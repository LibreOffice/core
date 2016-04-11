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
#include <cppuhelper/implbase.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

using namespace css::uno;
using namespace css::frame;
using namespace css::text;
using namespace css::beans;
using namespace css::style;
using namespace css::view;
using namespace css::lang;
using namespace css::container;

SFX_IMPL_CHILDWINDOW(SvxRubyChildWindow, SID_RUBY_DIALOG);

namespace
{

static const sal_Char cRubyBaseText[] = "RubyBaseText";
static const sal_Char cRubyText[] = "RubyText";
static const sal_Char cCharacterStyles[] = "CharacterStyles";
static const sal_Char cRubyAdjust[] = "RubyAdjust";
static const sal_Char cRubyIsAbove[] = "RubyIsAbove";
static const sal_Char cDisplayName[] = "DisplayName";
static const sal_Char cRubyCharStyleName[] = "RubyCharStyleName";
static const sal_Char cRubies[] = "Rubies";

} // end anonymous namespace

SvxRubyChildWindow::SvxRubyChildWindow(vcl::Window* _pParent, sal_uInt16 nId,SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxChildWindow(_pParent, nId)
{
    VclPtr<SvxRubyDialog> pDlg = VclPtr<SvxRubyDialog>::Create(pBindings, this, _pParent);
    SetWindow(pDlg);

    if (pInfo->nFlags & SfxChildWindowFlags::ZOOMIN)
        pDlg->RollUp();

    pDlg->Initialize(pInfo);
}

SfxChildWinInfo SvxRubyChildWindow::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

class SvxRubyData_Impl : public cppu::WeakImplHelper<css::view::XSelectionChangeListener>
{
    Reference<XModel> xModel;
    Reference<XRubySelection> xSelection;
    Sequence<PropertyValues> aRubyValues;
    Reference<XController> xController;
    bool bHasSelectionChanged;

public:
    SvxRubyData_Impl();
    virtual ~SvxRubyData_Impl();

    void SetController(const Reference<XController>& xCtrl);
    Reference<XModel> GetModel()
    {
        if (!xController.is())
            xModel = nullptr;
        else
            xModel = xController->getModel();
        return xModel;
    }
    bool HasSelectionChanged() const
    {
        return bHasSelectionChanged;
    }
    Reference<XRubySelection> GetRubySelection()
    {
        xSelection.set(xController, UNO_QUERY);
        return xSelection;
    }
    void UpdateRubyValues()
    {
        if (!xSelection.is())
            aRubyValues.realloc(0);
        else
            aRubyValues = xSelection->getRubyList(false);
        bHasSelectionChanged = false;
    }
    Sequence<PropertyValues>& GetRubyValues()
    {
        return aRubyValues;
    }
    void AssertOneEntry();

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& aEvent)
                            throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source)
                            throw (RuntimeException, std::exception) override;

};

SvxRubyData_Impl::SvxRubyData_Impl()
    : bHasSelectionChanged(false)
{
}

SvxRubyData_Impl::~SvxRubyData_Impl()
{
}

void SvxRubyData_Impl::SetController(const Reference<XController>& xCtrl)
{
    if (xCtrl.get() != xController.get())
    {
        try
        {
            Reference<XSelectionSupplier> xSelSupp(xController, UNO_QUERY);
            if (xSelSupp.is())
                xSelSupp->removeSelectionChangeListener(this);

            bHasSelectionChanged = true;
            xController = xCtrl;
            xSelSupp.set(xController, UNO_QUERY);
            if (xSelSupp.is())
                xSelSupp->addSelectionChangeListener(this);
        }
        catch (const Exception&)
        {
        }
    }
}

void SvxRubyData_Impl::selectionChanged(const EventObject& ) throw (RuntimeException, std::exception)
{
    bHasSelectionChanged = true;
}

void SvxRubyData_Impl::disposing(const EventObject&) throw (RuntimeException, std::exception)
{
    try
    {
        Reference<XSelectionSupplier> xSelSupp(xController, UNO_QUERY);
        if (xSelSupp.is())
            xSelSupp->removeSelectionChangeListener(this);
    }
    catch (const Exception&)
    {
    }
    xController = nullptr;
}

void SvxRubyData_Impl::AssertOneEntry()
{
    //create one entry
    if (!aRubyValues.getLength())
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

SvxRubyDialog::SvxRubyDialog(SfxBindings* pBind, SfxChildWindow* pCW, vcl::Window* _pParent)
    : SfxModelessDialog(pBind, pCW, _pParent, "AsianPhoneticGuideDialog", "svx/ui/asianphoneticguidedialog.ui")
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
    m_pScrolledWindow->setUserManagedScrolling(true);
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

    Link<ScrollBar*,void> aScrLk(LINK(this, SvxRubyDialog, ScrollHdl_Impl));
    m_pScrollSB->SetScrollHdl(aScrLk);
    m_pScrollSB->SetEndScrollHdl(aScrLk);

    Link<Edit&,void> aEditLk(LINK(this, SvxRubyDialog, EditModifyHdl_Impl));
    Link<sal_Int32,bool> aScrollLk(LINK(this, SvxRubyDialog, EditScrollHdl_Impl));
    Link<sal_Int32,void> aJumpLk(LINK(this, SvxRubyDialog, EditJumpHdl_Impl));
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        aEditArr[i]->SetModifyHdl(aEditLk);
        aEditArr[i]->SetJumpHdl(aJumpLk);
        if (!i || 7 == i)
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
    disposeOnce();
}

void SvxRubyDialog::dispose()
{
    ClearCharStyleList();
    EventObject aEvent;
    xImpl->disposing(aEvent);
    m_pLeftFT.clear();
    m_pRightFT.clear();
    m_pLeft1ED.clear();
    m_pRight1ED.clear();
    m_pLeft2ED.clear();
    m_pRight2ED.clear();
    m_pLeft3ED.clear();
    m_pRight3ED.clear();
    m_pLeft4ED.clear();
    m_pRight4ED.clear();
    for (int i = 0; i < 7; i++)
        aEditArr[i].clear();
    m_pScrolledWindow.clear();
    m_pScrollSB.clear();
    m_pAdjustLB.clear();
    m_pPositionLB.clear();
    m_pCharStyleFT.clear();
    m_pCharStyleLB.clear();
    m_pStylistPB.clear();
    m_pPreviewWin.clear();
    m_pApplyPB.clear();
    m_pClosePB.clear();
    SfxModelessDialog::dispose();
}

void SvxRubyDialog::ClearCharStyleList()
{
    for (sal_Int32 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
    {
        void* pData = m_pCharStyleLB->GetEntryData(i);
        delete static_cast<OUString*>(pData);
    }
    m_pCharStyleLB->Clear();
}

bool SvxRubyDialog::Close()
{
    pBindings->GetDispatcher()->Execute( SID_RUBY_DIALOG,
                              SfxCallMode::ASYNCHRON |
                              SfxCallMode::RECORD);
    return true;
}

void SvxRubyDialog::Activate()
{
    SfxModelessDialog::Activate();
    SfxPoolItem* pState = nullptr;
    SfxItemState    eState = pBindings->QueryState( SID_STYLE_DESIGNER, pState );
    bool bEnable = (eState < SfxItemState::DEFAULT) || !pState || !static_cast<SfxBoolItem*>(pState)->GetValue();
    delete pState;
    m_pStylistPB->Enable(bEnable);
    //get selection from current view frame
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    Reference< XController > xCtrl = pCurFrm->GetFrame().GetController();
    pImpl->SetController(xCtrl);
    if (pImpl->HasSelectionChanged())
    {

        Reference< XRubySelection > xRubySel = pImpl->GetRubySelection();
        pImpl->UpdateRubyValues();
        EnableControls(xRubySel.is());
        if (xRubySel.is())
        {
            Reference< XModel > xModel = pImpl->GetModel();
            const OUString sCharStyleSelect = m_pCharStyleLB->GetSelectEntry();
            ClearCharStyleList();
            Reference<XStyleFamiliesSupplier> xSupplier(xModel, UNO_QUERY);
            if (xSupplier.is())
            {
                try
                {
                    Reference<XNameAccess> xFam = xSupplier->getStyleFamilies();
                    Any aChar = xFam->getByName(cCharacterStyles);
                    Reference<XNameContainer> xChar;
                    aChar >>= xChar;
                    Reference<XIndexAccess> xCharIdx(xChar, UNO_QUERY);
                    if (xCharIdx.is())
                    {
                        OUString sUIName(cDisplayName);
                        for (sal_Int32 nStyle = 0; nStyle < xCharIdx->getCount(); nStyle++)
                        {
                            Any aStyle = xCharIdx->getByIndex(nStyle);
                            Reference<XStyle> xStyle;
                            aStyle >>= xStyle;
                            Reference<XPropertySet> xPrSet(xStyle, UNO_QUERY);
                            OUString sName, sCoreName;
                            if (xPrSet.is())
                            {
                                Reference<XPropertySetInfo> xInfo = xPrSet->getPropertySetInfo();
                                if (xInfo->hasPropertyByName(sUIName))
                                {
                                    Any aName = xPrSet->getPropertyValue(sUIName);
                                    aName >>= sName;
                                }
                            }
                            if (xStyle.is())
                            {
                                sCoreName = xStyle->getName();
                                if (sName.isEmpty())
                                    sName = sCoreName;
                            }
                            if (!sName.isEmpty())
                            {
                                const sal_Int32 nPos = m_pCharStyleLB->InsertEntry(sName);
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

void SvxRubyDialog::Deactivate()
{
    SfxModelessDialog::Deactivate();
}

void SvxRubyDialog::SetRubyText(sal_Int32 nPos, Edit& rLeft, Edit& rRight)
{
    OUString sLeft, sRight;
    const Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    bool bEnable = aRubyValues.getLength() > nPos;
    if (bEnable)
    {
        const Sequence<PropertyValue> aProps = aRubyValues.getConstArray()[nPos];
        const PropertyValue* pProps = aProps.getConstArray();
        for (sal_Int32 nProp = 0; nProp < aProps.getLength(); nProp++)
        {
            if (pProps[nProp].Name == cRubyBaseText)
                pProps[nProp].Value >>= sLeft;
            else if (pProps[nProp].Name == cRubyText)
                pProps[nProp].Value >>= sRight;
        }
    }
    else if (!nPos)
    {
        bEnable = true;
    }
    rLeft.Enable(bEnable);
    rRight.Enable(bEnable);
    rLeft.SetText(sLeft);
    rRight.SetText(sRight);
    rLeft.SaveValue();
    rRight.SaveValue();
}

void SvxRubyDialog::GetRubyText()
{
    long nTempLastPos = GetLastPos();
    for (int i = 0; i < 8; i+=2)
    {
        if (aEditArr[i]->IsEnabled() &&
            (aEditArr[i]->IsValueChangedFromSaved() || aEditArr[i + 1]->IsValueChangedFromSaved()))
        {
            Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
            DBG_ASSERT(aRubyValues.getLength() > (i / 2 + nTempLastPos), "wrong index" );
            SetModified(true);
            Sequence<PropertyValue>& rProps = aRubyValues.getArray()[i / 2 + nTempLastPos];
            PropertyValue* pProps = rProps.getArray();
            for (sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
            {
                if (pProps[nProp].Name == cRubyBaseText)
                    pProps[nProp].Value <<= OUString(aEditArr[i]->GetText());
                else if (pProps[nProp].Name == cRubyText)
                    pProps[nProp].Value <<= OUString(aEditArr[i + 1]->GetText());
            }
        }
    }
}

void SvxRubyDialog::Update()
{
    const Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
    sal_Int32 nLen = aRubyValues.getLength();
    m_pScrollSB->Enable(nLen > 4);
    m_pScrollSB->SetRange( Range(0, nLen > 4 ? nLen - 4 : 0));
    m_pScrollSB->SetThumbPos(0);
    SetLastPos(0);
    SetModified(false);

    sal_Int16 nAdjust = -1;
    sal_Int16 nPosition = -1;
    OUString sCharStyleName, sTmp;
    bool bCharStyleEqual = true;
    for (sal_Int32 nRuby = 0; nRuby < nLen; nRuby++)
    {
        const Sequence<PropertyValue> &rProps = aRubyValues.getConstArray()[nRuby];
        const PropertyValue* pProps = rProps.getConstArray();
        for (sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if (nAdjust > -2 && pProps[nProp].Name == cRubyAdjust)
            {
                sal_Int16 nTmp = sal_Int16();
                pProps[nProp].Value >>= nTmp;
                if (!nRuby)
                    nAdjust = nTmp;
                else if(nAdjust != nTmp)
                    nAdjust = -2;
            }
            if (nPosition > -2 && pProps[nProp].Name == cRubyIsAbove)
            {
                bool bTmp = *static_cast<sal_Bool const *>(pProps[nProp].Value.getValue());
                if (!nRuby)
                    nPosition = bTmp ? 0 : 1;
                else if ((!nPosition && !bTmp) || (nPosition == 1 && bTmp))
                    nPosition = -2;
            }
            if (bCharStyleEqual && pProps[nProp].Name == cRubyCharStyleName)
            {
                pProps[nProp].Value >>= sTmp;
                if (!nRuby)
                    sCharStyleName = sTmp;
                else if (sCharStyleName != sTmp)
                    bCharStyleEqual = false;
            }
        }
    }
    if (!nLen)
    {
        //enable selection if the ruby list is empty
        nAdjust = 0;
        nPosition = 0;
    }
    if (nAdjust > -1)
        m_pAdjustLB->SelectEntryPos(nAdjust);
    else
        m_pAdjustLB->SetNoSelection();
    if (nPosition > -1)
        m_pPositionLB->SelectEntryPos(nPosition ? 1 : 0);
    if (!nLen || (bCharStyleEqual && sCharStyleName.isEmpty()))
        sCharStyleName = cRubies;
    if (!sCharStyleName.isEmpty())
    {
        for (sal_Int32 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
        {
            const OUString* pCoreName = static_cast<const OUString*>(m_pCharStyleLB->GetEntryData(i));
            if (pCoreName && sCharStyleName == *pCoreName)
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

void SvxRubyDialog::GetCurrentText(OUString& rBase, OUString& rRuby)
{
    rBase = aEditArr[nCurrentEdit * 2]->GetText();
    rRuby = aEditArr[nCurrentEdit * 2 + 1]->GetText();
}

IMPL_LINK_TYPED(SvxRubyDialog, ScrollHdl_Impl, ScrollBar*, pScroll, void)
{
    long nPos = pScroll->GetThumbPos();
    if (GetLastPos() != nPos)
    {
        GetRubyText();
    }
    SetRubyText(nPos++, *m_pLeft1ED, *m_pRight1ED);
    SetRubyText(nPos++, *m_pLeft2ED, *m_pRight2ED);
    SetRubyText(nPos++, *m_pLeft3ED, *m_pRight3ED);
    SetRubyText(nPos, *m_pLeft4ED, *m_pRight4ED);
    SetLastPos(nPos - 3);
    m_pPreviewWin->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxRubyDialog, ApplyHdl_Impl, Button*, void)
{
    const Sequence<PropertyValues>& aRubyValues = pImpl->GetRubyValues();
    if (!aRubyValues.getLength())
    {
        AssertOneEntry();
        PositionHdl_Impl(*m_pPositionLB);
        AdjustHdl_Impl(*m_pAdjustLB);
        CharStyleHdl_Impl(*m_pCharStyleLB);
    }
    GetRubyText();
    //reset all edit fields - SaveValue is called
    ScrollHdl_Impl(m_pScrollSB);

    Reference<XRubySelection> xSelection = pImpl->GetRubySelection();
    if (IsModified() && xSelection.is())
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
}

IMPL_LINK_NOARG_TYPED(SvxRubyDialog, CloseHdl_Impl, Button*, void)
{
    Close();
}

IMPL_LINK_NOARG_TYPED(SvxRubyDialog, StylistHdl_Impl, Button*, void)
{
    SfxPoolItem* pState = nullptr;
    SfxItemState eState = pBindings->QueryState(SID_STYLE_DESIGNER, pState);
    if (eState <= SfxItemState::SET || !pState || !static_cast<SfxBoolItem*>(pState)->GetValue())
    {
        pBindings->GetDispatcher()->Execute(SID_STYLE_DESIGNER,
                                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
    }
    delete pState;
}

IMPL_LINK_TYPED(SvxRubyDialog, AdjustHdl_Impl, ListBox&, rBox, void)
{
    AssertOneEntry();
    sal_Int16 nAdjust = rBox.GetSelectEntryPos();
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for (sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for (sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if (pProps[nProp].Name == cRubyAdjust)
                pProps[nProp].Value <<= nAdjust;
        }
        SetModified(true);
    }
    m_pPreviewWin->Invalidate();
}

IMPL_LINK_TYPED(SvxRubyDialog, PositionHdl_Impl, ListBox&, rBox, void)
{
    AssertOneEntry();
    sal_Bool bAbove = !rBox.GetSelectEntryPos();
    const Type& rType = cppu::UnoType<bool>::get();
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for (sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for (sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if (pProps[nProp].Name == cRubyIsAbove)
                pProps[nProp].Value.setValue(&bAbove, rType);
        }
        SetModified(true);
    }
    m_pPreviewWin->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxRubyDialog, CharStyleHdl_Impl, ListBox&, void)
{
    AssertOneEntry();
    OUString sStyleName;
    if (LISTBOX_ENTRY_NOTFOUND != m_pCharStyleLB->GetSelectEntryPos())
        sStyleName = *static_cast<OUString*>(m_pCharStyleLB->GetSelectEntryData());
    Sequence<PropertyValues>&  aRubyValues = pImpl->GetRubyValues();
    for (sal_Int32 nRuby = 0; nRuby < aRubyValues.getLength(); nRuby++)
    {
        Sequence<PropertyValue> &rProps = aRubyValues.getArray()[nRuby];
        PropertyValue* pProps = rProps.getArray();
        for (sal_Int32 nProp = 0; nProp < rProps.getLength(); nProp++)
        {
            if (pProps[nProp].Name == cRubyCharStyleName)
            {
                pProps[nProp].Value <<= sStyleName;
            }
        }
        SetModified(true);
    }
}

IMPL_LINK_TYPED(SvxRubyDialog, EditModifyHdl_Impl, Edit&, rEdit, void)
{
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        if (&rEdit == aEditArr[i])
        {
            nCurrentEdit = i / 2;
            break;
        }
    }
    m_pPreviewWin->Invalidate();
}

IMPL_LINK_TYPED(SvxRubyDialog, EditScrollHdl_Impl, sal_Int32, nParam, bool)
{
    bool bRet = false;
    if (m_pScrollSB->IsEnabled())
    {
        //scroll forward
        if (nParam > 0 && (aEditArr[7]->HasFocus() || aEditArr[6]->HasFocus() ))
        {
            if (m_pScrollSB->GetRangeMax() > m_pScrollSB->GetThumbPos())
            {
                m_pScrollSB->SetThumbPos(m_pScrollSB->GetThumbPos() + 1);
                aEditArr[6]->GrabFocus();
                bRet = true;
            }
        }
        //scroll backward
        else if (m_pScrollSB->GetThumbPos() && (aEditArr[0]->HasFocus()||aEditArr[1]->HasFocus()) )
        {
            m_pScrollSB->SetThumbPos(m_pScrollSB->GetThumbPos() - 1);
            aEditArr[1]->GrabFocus();
            bRet = true;
        }
        if (bRet)
            ScrollHdl_Impl(m_pScrollSB);
    }
    return bRet;
}

IMPL_LINK_TYPED(SvxRubyDialog, EditJumpHdl_Impl, sal_Int32, nParam, void)
{
    sal_uInt16 nIndex = USHRT_MAX;
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        if(aEditArr[i]->HasFocus())
            nIndex = i;
    }
    if (nIndex < 8)
    {
        if (nParam > 0)
        {
            if (nIndex < 6)
                aEditArr[nIndex + 2]->GrabFocus();
            else if( EditScrollHdl_Impl(nParam))
                aEditArr[nIndex]->GrabFocus();
        }
        else
        {
            if (nIndex > 1)
                aEditArr[nIndex - 2]->GrabFocus();
            else if( EditScrollHdl_Impl(nParam))
                aEditArr[nIndex]->GrabFocus();
        }
    }
};

void SvxRubyDialog::AssertOneEntry()
{
    pImpl->AssertOneEntry();
}

void SvxRubyDialog::UpdateColors()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    vcl::Font aFont(m_pPreviewWin->GetFont());

    Color aNewTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);
    Color aNewFillColor(rStyleSettings.GetWindowColor());

    if (aNewFillColor != aFont.GetFillColor() || aNewTextColor != aFont.GetColor())
    {
        aFont.SetFillColor(aNewFillColor);
        aFont.SetColor(aNewTextColor);
        m_pPreviewWin->SetFont(aFont);
    }
}

void SvxRubyDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModelessDialog::DataChanged( rDCEvt );

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        UpdateColors();
    }
}

void SvxRubyDialog::EnableControls(bool bEnable)
{
    get_content_area()->Enable(bEnable);
    m_pApplyPB->Enable(bEnable);
}

RubyPreview::RubyPreview(vcl::Window *pParent)
    : Window(pParent, WB_BORDER)
    , m_pParentDlg(nullptr)
{
    SetBorderStyle(WindowBorderStyle::MONO);
}

RubyPreview::~RubyPreview()
{
    disposeOnce();
}

void RubyPreview::dispose()
{
    m_pParentDlg.clear();
    vcl::Window::dispose();
}

VCL_BUILDER_FACTORY(RubyPreview)

void RubyPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    rRenderContext.Push(PushFlags::MAPMODE);

    rRenderContext.SetMapMode(MAP_TWIP);

    Size aWinSize = rRenderContext.GetOutputSize();

    vcl::Font aSaveFont = rRenderContext.GetFont();
    aSaveFont.SetHeight(aWinSize.Height() / 4);
    rRenderContext.SetFont(aSaveFont);

    Rectangle aRect(Point(0, 0), aWinSize);
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(aSaveFont.GetFillColor());
    rRenderContext.DrawRect(aRect);

    OUString sBaseText, sRubyText;
    m_pParentDlg->GetCurrentText(sBaseText, sRubyText);

    long nTextHeight = rRenderContext.GetTextHeight();
    long nBaseWidth = rRenderContext.GetTextWidth(sBaseText);

    vcl::Font aRubyFont(aSaveFont);
    aRubyFont.SetHeight(aRubyFont.GetHeight() * 70 / 100);
    rRenderContext.SetFont(aRubyFont);
    long nRubyWidth = rRenderContext.GetTextWidth(sRubyText);
    rRenderContext.SetFont(aSaveFont);

    sal_Int32 nAdjust = m_pParentDlg->m_pAdjustLB->GetSelectEntryPos();
    //use center if no adjustment is available
    if (nAdjust > 4)
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
    if (!bAbove)
    {
        long nTmp = nYRuby;
        nYRuby = nYBase;
        nYBase = nTmp;
    }

    long nYOutput;
    long nOutTextWidth;
    OUString sOutputText;

    if (bRubyStretch)
    {
        rRenderContext.DrawText(Point(nLeftStart , nYBase),  sBaseText);
        nYOutput = nYRuby;
        sOutputText = sRubyText;
        nOutTextWidth = nRubyWidth;
        rRenderContext.SetFont(aRubyFont);
    }
    else
    {
        rRenderContext.SetFont(aRubyFont);
        rRenderContext.DrawText(Point(nLeftStart , nYRuby),  sRubyText);
        nYOutput = nYBase;
        sOutputText = sBaseText;
        nOutTextWidth = nBaseWidth;
        rRenderContext.SetFont(aSaveFont);
    }

    switch (nAdjust)
    {
        case RubyAdjust_LEFT:
            rRenderContext.DrawText(Point(nLeftStart , nYOutput),  sOutputText);
        break;
        case RubyAdjust_RIGHT:
            rRenderContext.DrawText(Point(nRightEnd - nOutTextWidth, nYOutput), sOutputText);
        break;
        case RubyAdjust_INDENT_BLOCK:
        {
            long nCharWidth = GetTextWidth("X");
            if (nOutTextWidth < (nRightEnd - nLeftStart - nCharWidth))
            {
                nCharWidth /= 2;
                nLeftStart += nCharWidth;
                nRightEnd -= nCharWidth;
            }
        }
        // no break!
        case RubyAdjust_BLOCK:
        {
            if (sOutputText.getLength() > 1)
            {
                sal_Int32 nCount = sOutputText.getLength();
                long nSpace = ((nRightEnd - nLeftStart) - GetTextWidth(sOutputText)) / (nCount - 1);
                for (sal_Int32 i = 0; i < nCount; i++)
                {
                    OUString sChar(sOutputText[i]);
                    rRenderContext.DrawText(Point(nLeftStart , nYOutput),  sChar);
                    long nCharWidth = GetTextWidth(sChar);
                    nLeftStart += nCharWidth + nSpace;
                }
                break;
            }
        }
        //no break;
        case RubyAdjust_CENTER:
            rRenderContext.DrawText(Point(nCenter - nOutTextWidth / 2 , nYOutput),  sOutputText);
        break;
    }
    rRenderContext.SetFont(aSaveFont);
    rRenderContext.Pop();
}

Size RubyPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(215, 50), MapMode(MAP_APPFONT));
}

void RubyEdit::GetFocus()
{
    GetModifyHdl().Call(*this);
    Edit::GetFocus();
}

bool RubyEdit::PreNotify(NotifyEvent& rNEvt)
{
    bool bHandled = false;
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode = pKEvt->GetKeyCode();
        sal_uInt16 nMod = rKeyCode.GetModifier();
        sal_uInt16 nCode = rKeyCode.GetCode();
        if (nCode == KEY_TAB && (!nMod || KEY_SHIFT == nMod))
        {
            sal_Int32 nParam = KEY_SHIFT == nMod ? -1 : 1;
            if(aScrollHdl.IsSet() && aScrollHdl.Call(nParam))
                bHandled = true;
        }
        else if (KEY_UP == nCode || KEY_DOWN == nCode)
        {
            sal_Int32 nParam = KEY_UP == nCode ? -1 : 1;
            aJumpHdl.Call(nParam);
        }
    }
    if (!bHandled)
        bHandled = Edit::PreNotify(rNEvt);
    return bHandled;
}

VCL_BUILDER_FACTORY(RubyEdit)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
