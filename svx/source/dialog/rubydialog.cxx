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

#include <memory>
#include <sal/config.h>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <svx/rubydialog.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
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
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

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

const char cRubyBaseText[] = "RubyBaseText";
const char cRubyText[] = "RubyText";
const char cRubyAdjust[] = "RubyAdjust";
const char cRubyPosition[] = "RubyPosition";
const char cRubyCharStyleName[] = "RubyCharStyleName";

} // end anonymous namespace

SvxRubyChildWindow::SvxRubyChildWindow(vcl::Window* _pParent, sal_uInt16 nId,SfxBindings* pBindings, SfxChildWinInfo const * pInfo)
    : SfxChildWindow(_pParent, nId)
{
    auto xDlg = std::make_shared<SvxRubyDialog>(pBindings, this, _pParent->GetFrameWeld());
    SetController(xDlg);
    xDlg->Initialize(pInfo);
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
    virtual ~SvxRubyData_Impl() override;

    void SetController(const Reference<XController>& xCtrl);
    Reference<XModel> const & GetModel()
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
    Reference<XRubySelection> const & GetRubySelection()
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

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source) override;

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
    if (xCtrl.get() == xController.get())
        return;

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

void SvxRubyData_Impl::selectionChanged(const EventObject& )
{
    bHasSelectionChanged = true;
}

void SvxRubyData_Impl::disposing(const EventObject&)
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
    if (!aRubyValues.hasElements())
    {
        aRubyValues.realloc(1);
        Sequence<PropertyValue>& rValues = aRubyValues.getArray()[0];
        rValues.realloc(5);
        PropertyValue* pValues = rValues.getArray();
        pValues[0].Name = cRubyBaseText;
        pValues[1].Name = cRubyText;
        pValues[2].Name = cRubyAdjust;
        pValues[3].Name = cRubyPosition;
        pValues[4].Name = cRubyCharStyleName;
    }
}

SvxRubyDialog::SvxRubyDialog(SfxBindings* pBind, SfxChildWindow* pCW, weld::Window* pParent)
    : SfxModelessDialogController(pBind, pCW, pParent,
                        "svx/ui/asianphoneticguidedialog.ui", "AsianPhoneticGuideDialog")
    , nLastPos(0)
    , nCurrentEdit(0)
    , bModified(false)
    , pBindings(pBind)
    , m_pImpl( new SvxRubyData_Impl )
    , m_xLeftFT(m_xBuilder->weld_label("basetextft"))
    , m_xRightFT(m_xBuilder->weld_label("rubytextft"))
    , m_xLeft1ED(m_xBuilder->weld_entry("Left1ED"))
    , m_xRight1ED(m_xBuilder->weld_entry("Right1ED"))
    , m_xLeft2ED(m_xBuilder->weld_entry("Left2ED"))
    , m_xRight2ED(m_xBuilder->weld_entry("Right2ED"))
    , m_xLeft3ED(m_xBuilder->weld_entry("Left3ED"))
    , m_xRight3ED(m_xBuilder->weld_entry("Right3ED"))
    , m_xLeft4ED(m_xBuilder->weld_entry("Left4ED"))
    , m_xRight4ED(m_xBuilder->weld_entry("Right4ED"))
    , m_xScrolledWindow(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , m_xAdjustLB(m_xBuilder->weld_combo_box("adjustlb"))
    , m_xPositionLB(m_xBuilder->weld_combo_box("positionlb"))
    , m_xCharStyleFT(m_xBuilder->weld_label("styleft"))
    , m_xCharStyleLB(m_xBuilder->weld_combo_box("stylelb"))
    , m_xStylistPB(m_xBuilder->weld_button("styles"))
    , m_xApplyPB(m_xBuilder->weld_button("ok"))
    , m_xClosePB(m_xBuilder->weld_button("cancel"))
    , m_xContentArea(m_xDialog->weld_content_area())
    , m_xGrid(m_xBuilder->weld_widget("grid"))
    , m_xPreviewWin(new RubyPreview)
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "preview", *m_xPreviewWin))
{
    m_xCharStyleLB->make_sorted();
    m_xPreviewWin->setRubyDialog(this);
    m_xScrolledWindow->set_user_managed_scrolling();
    m_xScrolledWindow->set_size_request(-1, m_xGrid->get_preferred_size().Height());
    m_xScrolledWindow->set_vpolicy(VclPolicyType::NEVER);

    aEditArr[0] = m_xLeft1ED.get(); aEditArr[1] = m_xRight1ED.get();
    aEditArr[2] = m_xLeft2ED.get(); aEditArr[3] = m_xRight2ED.get();
    aEditArr[4] = m_xLeft3ED.get(); aEditArr[5] = m_xRight3ED.get();
    aEditArr[6] = m_xLeft4ED.get(); aEditArr[7] = m_xRight4ED.get();

    m_xApplyPB->connect_clicked(LINK(this, SvxRubyDialog, ApplyHdl_Impl));
    m_xClosePB->connect_clicked(LINK(this, SvxRubyDialog, CloseHdl_Impl));
    m_xStylistPB->connect_clicked(LINK(this, SvxRubyDialog, StylistHdl_Impl));
    m_xAdjustLB->connect_changed(LINK(this, SvxRubyDialog, AdjustHdl_Impl));
    m_xPositionLB->connect_changed(LINK(this, SvxRubyDialog, PositionHdl_Impl));
    m_xCharStyleLB->connect_changed(LINK(this, SvxRubyDialog, CharStyleHdl_Impl));

    Link<weld::ScrolledWindow&, void> aScrLk(LINK(this, SvxRubyDialog, ScrollHdl_Impl));
    m_xScrolledWindow->connect_vadjustment_changed(aScrLk);

    Link<weld::Entry&,void> aEditLk(LINK(this, SvxRubyDialog, EditModifyHdl_Impl));
    Link<weld::Widget&,void> aFocusLk(LINK(this, SvxRubyDialog, EditFocusHdl_Impl));
    Link<const KeyEvent&,bool> aKeyUpDownLk(LINK(this, SvxRubyDialog, KeyUpDownHdl_Impl));
    Link<const KeyEvent&,bool> aKeyTabUpDownLk(LINK(this, SvxRubyDialog, KeyUpDownTabHdl_Impl));
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        aEditArr[i]->connect_changed(aEditLk);
        aEditArr[i]->connect_focus_in(aFocusLk);
        if (!i || 7 == i)
            aEditArr[i]->connect_key_press(aKeyTabUpDownLk);
        else
            aEditArr[i]->connect_key_press(aKeyUpDownLk);
    }
}

SvxRubyDialog::~SvxRubyDialog()
{
    ClearCharStyleList();
    EventObject aEvent;
    m_pImpl->disposing(aEvent);
}

void SvxRubyDialog::ClearCharStyleList()
{
    m_xCharStyleLB->clear();
}

void SvxRubyDialog::Close()
{
    if (IsClosing())
        return;
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (pViewFrame)
        pViewFrame->ToggleChildWindow(SID_RUBY_DIALOG);
}

void SvxRubyDialog::Activate()
{
    SfxModelessDialogController::Activate();
    //get selection from current view frame
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    Reference< XController > xCtrl = pCurFrm->GetFrame().GetController();
    m_pImpl->SetController(xCtrl);
    if (!m_pImpl->HasSelectionChanged())
        return;

    Reference< XRubySelection > xRubySel = m_pImpl->GetRubySelection();
    m_pImpl->UpdateRubyValues();
    EnableControls(xRubySel.is());
    if (xRubySel.is())
    {
        Reference< XModel > xModel = m_pImpl->GetModel();
        const OUString sCharStyleSelect = m_xCharStyleLB->get_active_text();
        ClearCharStyleList();
        Reference<XStyleFamiliesSupplier> xSupplier(xModel, UNO_QUERY);
        if (xSupplier.is())
        {
            try
            {
                Reference<XNameAccess> xFam = xSupplier->getStyleFamilies();
                Any aChar = xFam->getByName("CharacterStyles");
                Reference<XNameContainer> xChar;
                aChar >>= xChar;
                Reference<XIndexAccess> xCharIdx(xChar, UNO_QUERY);
                if (xCharIdx.is())
                {
                    OUString sUIName("DisplayName");
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
                            m_xCharStyleLB->append(sCoreName, sName);

                        }
                    }
                }
            }
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION("svx.dialog", "exception in style access");
            }
            if (!sCharStyleSelect.isEmpty())
                m_xCharStyleLB->set_active_text(sCharStyleSelect);
        }
        m_xCharStyleLB->set_sensitive(xSupplier.is());
        m_xCharStyleFT->set_sensitive(xSupplier.is());
    }
    Update();
    m_xPreviewWin->Invalidate();
}

void SvxRubyDialog::SetRubyText(sal_Int32 nPos, weld::Entry& rLeft, weld::Entry& rRight)
{
    OUString sLeft, sRight;
    const Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    bool bEnable = aRubyValues.getLength() > nPos;
    if (bEnable)
    {
        const Sequence<PropertyValue> aProps = aRubyValues.getConstArray()[nPos];
        for (const PropertyValue& rProp : aProps)
        {
            if (rProp.Name == cRubyBaseText)
                rProp.Value >>= sLeft;
            else if (rProp.Name == cRubyText)
                rProp.Value >>= sRight;
        }
    }
    else if (!nPos)
    {
        bEnable = true;
    }
    rLeft.set_sensitive(bEnable);
    rRight.set_sensitive(bEnable);
    rLeft.set_text(sLeft);
    rRight.set_text(sRight);
    rLeft.save_value();
    rRight.save_value();
}

void SvxRubyDialog::GetRubyText()
{
    long nTempLastPos = GetLastPos();
    for (int i = 0; i < 8; i+=2)
    {
        if (aEditArr[i]->get_sensitive() &&
            (aEditArr[i]->get_value_changed_from_saved() ||
             aEditArr[i + 1]->get_value_changed_from_saved()))
        {
            Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
            DBG_ASSERT(aRubyValues.getLength() > (i / 2 + nTempLastPos), "wrong index" );
            SetModified(true);
            Sequence<PropertyValue>& rProps = aRubyValues.getArray()[i / 2 + nTempLastPos];
            for (PropertyValue & propVal : rProps)
            {
                if (propVal.Name == cRubyBaseText)
                    propVal.Value <<= aEditArr[i]->get_text();
                else if (propVal.Name == cRubyText)
                    propVal.Value <<= aEditArr[i + 1]->get_text();
            }
        }
    }
}

void SvxRubyDialog::Update()
{
    const Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    sal_Int32 nLen = aRubyValues.getLength();
    m_xScrolledWindow->vadjustment_configure(0, 0, !nLen ? 1 : nLen, 1, 4, 4);
    if (nLen > 4)
        m_xScrolledWindow->set_vpolicy(VclPolicyType::ALWAYS);
    else
        m_xScrolledWindow->set_vpolicy(VclPolicyType::NEVER);
    SetLastPos(0);
    SetModified(false);

    sal_Int16 nAdjust = -1;
    sal_Int16 nPosition = -1;
    OUString sCharStyleName, sTmp;
    bool bCharStyleEqual = true;
    for (sal_Int32 nRuby = 0; nRuby < nLen; nRuby++)
    {
        const Sequence<PropertyValue> &rProps = aRubyValues.getConstArray()[nRuby];
        for (const PropertyValue& rProp : rProps)
        {
            if (nAdjust > -2 && rProp.Name == cRubyAdjust)
            {
                sal_Int16 nTmp = sal_Int16();
                rProp.Value >>= nTmp;
                if (!nRuby)
                    nAdjust = nTmp;
                else if(nAdjust != nTmp)
                    nAdjust = -2;
            }
            if (nPosition > -2 && rProp.Name == cRubyPosition )
            {
                sal_Int16 nTmp = sal_Int16();
                rProp.Value >>= nTmp;
                if (!nRuby)
                    nPosition = nTmp;
                else if(nPosition != nTmp)
                    nPosition = -2;
            }
            if (bCharStyleEqual && rProp.Name == cRubyCharStyleName)
            {
                rProp.Value >>= sTmp;
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
        m_xAdjustLB->set_active(nAdjust);
    else
        m_xAdjustLB->set_active(-1);
    if (nPosition > -1)
        m_xPositionLB->set_active(nPosition);
    if (!nLen || (bCharStyleEqual && sCharStyleName.isEmpty()))
        sCharStyleName = "Rubies";
    if (!sCharStyleName.isEmpty())
    {
        for (int i = 0, nEntryCount = m_xCharStyleLB->get_count(); i < nEntryCount; i++)
        {
            OUString sCoreName = m_xCharStyleLB->get_id(i);
            if (sCharStyleName == sCoreName)
            {
                m_xCharStyleLB->set_active(i);
                break;
            }
        }
    }
    else
        m_xCharStyleLB->set_active(-1);

    ScrollHdl_Impl(*m_xScrolledWindow);
}

void SvxRubyDialog::GetCurrentText(OUString& rBase, OUString& rRuby)
{
    rBase = aEditArr[nCurrentEdit * 2]->get_text();
    rRuby = aEditArr[nCurrentEdit * 2 + 1]->get_text();
}

IMPL_LINK(SvxRubyDialog, ScrollHdl_Impl, weld::ScrolledWindow&, rScroll, void)
{
    int nPos = rScroll.vadjustment_get_value();
    if (GetLastPos() != nPos)
    {
        GetRubyText();
    }
    SetRubyText(nPos++, *m_xLeft1ED, *m_xRight1ED);
    SetRubyText(nPos++, *m_xLeft2ED, *m_xRight2ED);
    SetRubyText(nPos++, *m_xLeft3ED, *m_xRight3ED);
    SetRubyText(nPos, *m_xLeft4ED, *m_xRight4ED);
    SetLastPos(nPos - 3);
    m_xPreviewWin->Invalidate();
}

IMPL_LINK_NOARG(SvxRubyDialog, ApplyHdl_Impl, weld::Button&, void)
{
    const Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    if (!aRubyValues.hasElements())
    {
        AssertOneEntry();
        PositionHdl_Impl(*m_xPositionLB);
        AdjustHdl_Impl(*m_xAdjustLB);
        CharStyleHdl_Impl(*m_xCharStyleLB);
    }
    GetRubyText();
    //reset all edit fields - SaveValue is called
    ScrollHdl_Impl(*m_xScrolledWindow);

    Reference<XRubySelection> xSelection = m_pImpl->GetRubySelection();
    if (IsModified() && xSelection.is())
    {
        try
        {
            xSelection->setRubyList(aRubyValues, false);
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION("svx.dialog", "");
        }
    }
}

IMPL_LINK_NOARG(SvxRubyDialog, CloseHdl_Impl, weld::Button&, void)
{
    Close();
}

IMPL_LINK_NOARG(SvxRubyDialog, StylistHdl_Impl, weld::Button&, void)
{
    std::unique_ptr<SfxPoolItem> pState;
    SfxItemState eState = pBindings->QueryState(SID_STYLE_DESIGNER, pState);
    if (eState <= SfxItemState::SET || !pState || !static_cast<SfxBoolItem*>(pState.get())->GetValue())
    {
        pBindings->GetDispatcher()->Execute(SID_STYLE_DESIGNER,
                                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
    }
}

IMPL_LINK(SvxRubyDialog, AdjustHdl_Impl, weld::ComboBox&, rBox, void)
{
    AssertOneEntry();
    sal_Int16 nAdjust = rBox.get_active();
    Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    for (PropertyValues & rProps : aRubyValues)
    {
        for (PropertyValue & propVal : rProps)
        {
            if (propVal.Name == cRubyAdjust)
                propVal.Value <<= nAdjust;
        }
        SetModified(true);
    }
    m_xPreviewWin->Invalidate();
}

IMPL_LINK(SvxRubyDialog, PositionHdl_Impl, weld::ComboBox&, rBox, void)
{
    AssertOneEntry();
    sal_Int16 nPosition = rBox.get_active();
    Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    for (PropertyValues & rProps : aRubyValues)
    {
        for (PropertyValue & propVal : rProps)
        {
            if (propVal.Name == cRubyPosition)
                propVal.Value <<= nPosition;
        }
        SetModified(true);
    }
    m_xPreviewWin->Invalidate();
}

IMPL_LINK_NOARG(SvxRubyDialog, CharStyleHdl_Impl, weld::ComboBox&, void)
{
    AssertOneEntry();
    OUString sStyleName;
    if (m_xCharStyleLB->get_active() != -1)
        sStyleName = m_xCharStyleLB->get_active_id();
    Sequence<PropertyValues>& aRubyValues = m_pImpl->GetRubyValues();
    for (PropertyValues & rProps : aRubyValues)
    {
        for (PropertyValue & propVal : rProps)
        {
            if (propVal.Name == cRubyCharStyleName)
            {
                propVal.Value <<= sStyleName;
            }
        }
        SetModified(true);
    }
}

IMPL_LINK(SvxRubyDialog, EditFocusHdl_Impl, weld::Widget&, rEdit, void)
{
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        if (&rEdit == aEditArr[i])
        {
            nCurrentEdit = i / 2;
            break;
        }
    }
    m_xPreviewWin->Invalidate();
}

IMPL_LINK(SvxRubyDialog, EditModifyHdl_Impl, weld::Entry&, rEdit, void)
{
    EditFocusHdl_Impl(rEdit);
}

bool SvxRubyDialog::EditScrollHdl_Impl(sal_Int32 nParam)
{
    bool bRet = false;
    //scroll forward
    if (nParam > 0 && (aEditArr[7]->has_focus() || aEditArr[6]->has_focus() ))
    {
        if (m_xScrolledWindow->vadjustment_get_upper() >
            m_xScrolledWindow->vadjustment_get_value() + m_xScrolledWindow->vadjustment_get_page_size())
        {
            m_xScrolledWindow->vadjustment_set_value(m_xScrolledWindow->vadjustment_get_value() + 1);
            aEditArr[6]->grab_focus();
            bRet = true;
        }
    }
    //scroll backward
    else if (m_xScrolledWindow->vadjustment_get_value() && (aEditArr[0]->has_focus()||aEditArr[1]->has_focus()) )
    {
        m_xScrolledWindow->vadjustment_set_value(m_xScrolledWindow->vadjustment_get_value() - 1);
        aEditArr[1]->grab_focus();
        bRet = true;
    }
    if (bRet)
        ScrollHdl_Impl(*m_xScrolledWindow);
    return bRet;
}

bool SvxRubyDialog::EditJumpHdl_Impl(sal_Int32 nParam)
{
    bool bHandled = false;
    sal_uInt16 nIndex = USHRT_MAX;
    for (sal_uInt16 i = 0; i < 8; i++)
    {
        if(aEditArr[i]->has_focus())
            nIndex = i;
    }
    if (nIndex < 8)
    {
        if (nParam > 0)
        {
            if (nIndex < 6)
                aEditArr[nIndex + 2]->grab_focus();
            else if( EditScrollHdl_Impl(nParam))
                aEditArr[nIndex]->grab_focus();
        }
        else
        {
            if (nIndex > 1)
                aEditArr[nIndex - 2]->grab_focus();
            else if( EditScrollHdl_Impl(nParam))
                aEditArr[nIndex]->grab_focus();
        }
        bHandled = true;
    }
    return bHandled;
}

void SvxRubyDialog::AssertOneEntry()
{
    m_pImpl->AssertOneEntry();
}

void SvxRubyDialog::EnableControls(bool bEnable)
{
    m_xContentArea->set_sensitive(bEnable);
    m_xApplyPB->set_sensitive(bEnable);
}

RubyPreview::RubyPreview()
    : m_pParentDlg(nullptr)
{
}

RubyPreview::~RubyPreview()
{
}

void RubyPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    rRenderContext.Push(PushFlags::ALL);

    rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));

    Size aWinSize = rRenderContext.GetOutputSize();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    Color aNewTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);
    Color aNewFillColor(rStyleSettings.GetWindowColor());

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetFontHeight(aWinSize.Height() / 4);
    aFont.SetFillColor(aNewFillColor);
    aFont.SetColor(aNewTextColor);
    rRenderContext.SetFont(aFont);

    tools::Rectangle aRect(Point(0, 0), aWinSize);
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(aFont.GetFillColor());
    rRenderContext.DrawRect(aRect);

    OUString sBaseText, sRubyText;
    m_pParentDlg->GetCurrentText(sBaseText, sRubyText);

    long nTextHeight = rRenderContext.GetTextHeight();
    long nBaseWidth = rRenderContext.GetTextWidth(sBaseText);

    vcl::Font aRubyFont(aFont);
    aRubyFont.SetFontHeight(aRubyFont.GetFontHeight() * 70 / 100);
    rRenderContext.SetFont(aRubyFont);
    long nRubyWidth = rRenderContext.GetTextWidth(sRubyText);
    rRenderContext.SetFont(aFont);

    RubyAdjust nAdjust = static_cast<RubyAdjust>(m_pParentDlg->m_xAdjustLB->get_active());
    //use center if no adjustment is available
    if (nAdjust > RubyAdjust_INDENT_BLOCK)
        nAdjust = RubyAdjust_CENTER;

    //which part is stretched ?
    bool bRubyStretch = nBaseWidth >= nRubyWidth;

    long nCenter = aWinSize.Width() / 2;
    long nHalfWidth = std::max( nBaseWidth, nRubyWidth ) /2;
    long nLeftStart = nCenter - nHalfWidth;
    long nRightEnd = nCenter + nHalfWidth;

    // Default values for TOP or no selection
    long nYRuby = aWinSize.Height() / 4 - nTextHeight / 2;
    long nYBase = aWinSize.Height() * 3 / 4 - nTextHeight / 2;

    sal_Int16 nRubyPos = m_pParentDlg->m_xPositionLB->get_active();
    if ( nRubyPos == 1 )    // BOTTOM
    {
        long nTmp = nYRuby;
        nYRuby = nYBase;
        nYBase = nTmp;
    }
    else if ( nRubyPos == 2 ) // RIGHT ( vertically )
    {
        // Align the ruby text and base text to the vertical center.
        nYBase =  ( aWinSize.Height() - nTextHeight ) / 2;
        nYRuby = ( aWinSize.Height() - nRubyWidth ) / 2;

        // Align the ruby text at the right side of the base text
        nAdjust = RubyAdjust_RIGHT;
        nHalfWidth = nBaseWidth / 2;
        nLeftStart = nCenter - nHalfWidth;
        nRightEnd = nCenter + nHalfWidth + nRubyWidth + nTextHeight;
        // Render base text first, then render ruby text on the right.
        bRubyStretch = true;

        aRubyFont.SetVertical(true);
        aRubyFont.SetOrientation(2700);
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
        rRenderContext.SetFont(aFont);
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
            long nCharWidth = rRenderContext.GetTextWidth("X");
            if (nOutTextWidth < (nRightEnd - nLeftStart - nCharWidth))
            {
                nCharWidth /= 2;
                nLeftStart += nCharWidth;
                nRightEnd -= nCharWidth;
            }
            [[fallthrough]];
        }
        case RubyAdjust_BLOCK:
        {
            if (sOutputText.getLength() > 1)
            {
                sal_Int32 nCount = sOutputText.getLength();
                long nSpace = ((nRightEnd - nLeftStart) - rRenderContext.GetTextWidth(sOutputText)) / (nCount - 1);
                for (sal_Int32 i = 0; i < nCount; i++)
                {
                    OUString sChar(sOutputText[i]);
                    rRenderContext.DrawText(Point(nLeftStart , nYOutput),  sChar);
                    long nCharWidth = rRenderContext.GetTextWidth(sChar);
                    nLeftStart += nCharWidth + nSpace;
                }
                break;
            }
            [[fallthrough]];
        }
        case RubyAdjust_CENTER:
            rRenderContext.DrawText(Point(nCenter - nOutTextWidth / 2 , nYOutput),  sOutputText);
        break;
        default: break;
    }
    rRenderContext.Pop();
}

void RubyPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 40,
                                   pDrawingArea->get_text_height() * 7);
    CustomWidgetController::SetDrawingArea(pDrawingArea);
}

IMPL_LINK(SvxRubyDialog, KeyUpDownHdl_Impl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();
    if (KEY_UP == nCode || KEY_DOWN == nCode)
    {
        sal_Int32 nParam = KEY_UP == nCode ? -1 : 1;
        bHandled = EditJumpHdl_Impl(nParam);
    }
    return bHandled;
}

IMPL_LINK(SvxRubyDialog, KeyUpDownTabHdl_Impl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nMod = rKeyCode.GetModifier();
    sal_uInt16 nCode = rKeyCode.GetCode();
    if (nCode == KEY_TAB && (!nMod || KEY_SHIFT == nMod))
    {
        sal_Int32 nParam = KEY_SHIFT == nMod ? -1 : 1;
        if (EditScrollHdl_Impl(nParam))
            bHandled = true;
    }
    if (!bHandled)
        bHandled = KeyUpDownHdl_Impl(rKEvt);
    return bHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
