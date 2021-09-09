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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoedhlp.hxx>
#include <editeng/unoedsrc.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <osl/diagnose.h>
#include <svl/itemset.hxx>
#include <sal/log.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/weldeditview.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/cursor.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/uitest/uiobject.hxx>

WeldEditView::WeldEditView() {}

// tdf#127033 want to use UI font so override makeEditEngine to enable that
void WeldEditView::makeEditEngine()
{
    SfxItemPool* pItemPool = EditEngine::CreatePool();

    vcl::Font aAppFont(Application::GetSettings().GetStyleSettings().GetAppFont());

    pItemPool->SetPoolDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              "", PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO));
    pItemPool->SetPoolDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              "", PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO_CJK));
    pItemPool->SetPoolDefaultItem(SvxFontItem(aAppFont.GetFamilyType(), aAppFont.GetFamilyName(),
                                              "", PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW,
                                              EE_CHAR_FONTINFO_CTL));

    pItemPool->SetPoolDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT));
    pItemPool->SetPoolDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT_CJK));
    pItemPool->SetPoolDefaultItem(
        SvxFontHeightItem(aAppFont.GetFontHeight() * 20, 100, EE_CHAR_FONTHEIGHT_CTL));

    m_xEditEngine.reset(new EditEngine(pItemPool));
}

void WeldEditView::Resize()
{
    if (m_xEditView)
    {
        OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
        Size aOutputSize(rDevice.PixelToLogic(GetOutputSizePixel()));
        Size aSize(aOutputSize);
        m_xEditEngine->SetPaperSize(aSize);
        m_xEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));
    }
    weld::CustomWidgetController::Resize();
}

void WeldEditView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    rRenderContext.Push(PushFlags::ALL);
    rRenderContext.SetClipRegion();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    rRenderContext.SetBackground(aBgColor);

    std::vector<tools::Rectangle> aLogicRects;

    if (m_xEditView)
    {
        m_xEditView->SetBackgroundColor(aBgColor);

        m_xEditView->Paint(rRect, &rRenderContext);

        if (HasFocus())
        {
            m_xEditView->ShowCursor(false);
            vcl::Cursor* pCursor = m_xEditView->GetCursor();
            pCursor->DrawToDevice(rRenderContext);
        }

        // get logic selection
        m_xEditView->GetSelectionRectangles(aLogicRects);
    }

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(COL_BLACK);
    rRenderContext.SetRasterOp(RasterOp::Invert);

    for (const auto& rSelectionRect : aLogicRects)
        rRenderContext.DrawRect(rSelectionRect);

    rRenderContext.Pop();
}

bool WeldEditView::MouseMove(const MouseEvent& rMEvt)
{
    return m_xEditView && m_xEditView->MouseMove(rMEvt);
}

bool WeldEditView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (!IsMouseCaptured())
        CaptureMouse();

    if (!HasFocus() && CanFocus())
    {
        GrabFocus();
        GetFocus();
    }

    return m_xEditView && m_xEditView->MouseButtonDown(rMEvt);
}

bool WeldEditView::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (IsMouseCaptured())
        ReleaseMouse();
    return m_xEditView && m_xEditView->MouseButtonUp(rMEvt);
}

bool WeldEditView::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();

    if (nKey == KEY_TAB)
    {
        return false;
    }
    else if (m_xEditView && !m_xEditView->PostKeyEvent(rKEvt))
    {
        if (rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2())
        {
            if (nKey == KEY_A)
            {
                sal_Int32 nPar = m_xEditEngine->GetParagraphCount();
                if (nPar)
                {
                    sal_Int32 nLen = m_xEditEngine->GetTextLen(nPar - 1);
                    m_xEditView->SetSelection(ESelection(0, 0, nPar - 1, nLen));
                }
                return true;
            }
        }

        return false;
    }

    return true;
}

bool WeldEditView::Command(const CommandEvent& rCEvt)
{
    if (!m_xEditView)
        return false;
    m_xEditView->Command(rCEvt);
    return true;
}

class WeldEditAccessible;

namespace
{
class WeldViewForwarder : public SvxViewForwarder
{
    WeldEditAccessible& m_rEditAcc;

    WeldViewForwarder(const WeldViewForwarder&) = delete;
    WeldViewForwarder& operator=(const WeldViewForwarder&) = delete;

public:
    explicit WeldViewForwarder(WeldEditAccessible& rAcc)
        : m_rEditAcc(rAcc)
    {
    }

    virtual bool IsValid() const override;
    virtual Point LogicToPixel(const Point& rPoint, const MapMode& rMapMode) const override;
    virtual Point PixelToLogic(const Point& rPoint, const MapMode& rMapMode) const override;
};
}

class WeldEditAccessible;

namespace
{
class WeldEditSource;

/* analog to SvxEditEngineForwarder */
class WeldTextForwarder : public SvxTextForwarder
{
    WeldEditAccessible& m_rEditAcc;
    WeldEditSource& m_rEditSource;

    DECL_LINK(NotifyHdl, EENotify&, void);

    WeldTextForwarder(const WeldTextForwarder&) = delete;
    WeldTextForwarder& operator=(const WeldTextForwarder&) = delete;

public:
    WeldTextForwarder(WeldEditAccessible& rAcc, WeldEditSource& rSource);
    virtual ~WeldTextForwarder() override;

    virtual sal_Int32 GetParagraphCount() const override;
    virtual sal_Int32 GetTextLen(sal_Int32 nParagraph) const override;
    virtual OUString GetText(const ESelection& rSel) const override;
    virtual SfxItemSet GetAttribs(const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib
                                                          = EditEngineAttribs::All) const override;
    virtual SfxItemSet GetParaAttribs(sal_Int32 nPara) const override;
    virtual void SetParaAttribs(sal_Int32 nPara, const SfxItemSet& rSet) override;
    virtual void RemoveAttribs(const ESelection& rSelection) override;
    virtual void GetPortions(sal_Int32 nPara, std::vector<sal_Int32>& rList) const override;

    virtual SfxItemState GetItemState(const ESelection& rSel, sal_uInt16 nWhich) const override;
    virtual SfxItemState GetItemState(sal_Int32 nPara, sal_uInt16 nWhich) const override;

    virtual void QuickInsertText(const OUString& rText, const ESelection& rSel) override;
    virtual void QuickInsertField(const SvxFieldItem& rFld, const ESelection& rSel) override;
    virtual void QuickSetAttribs(const SfxItemSet& rSet, const ESelection& rSel) override;
    virtual void QuickInsertLineBreak(const ESelection& rSel) override;

    virtual SfxItemPool* GetPool() const override;

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos,
                                    std::optional<Color>& rpTxtColor,
                                    std::optional<Color>& rpFldColor) override;
    virtual void FieldClicked(const SvxFieldItem&) override;
    virtual bool IsValid() const override;

    virtual LanguageType GetLanguage(sal_Int32, sal_Int32) const override;
    virtual sal_Int32 GetFieldCount(sal_Int32 nPara) const override;
    virtual EFieldInfo GetFieldInfo(sal_Int32 nPara, sal_uInt16 nField) const override;
    virtual EBulletInfo GetBulletInfo(sal_Int32 nPara) const override;
    virtual tools::Rectangle GetCharBounds(sal_Int32 nPara, sal_Int32 nIndex) const override;
    virtual tools::Rectangle GetParaBounds(sal_Int32 nPara) const override;
    virtual MapMode GetMapMode() const override;
    virtual OutputDevice* GetRefDevice() const override;
    virtual bool GetIndexAtPoint(const Point&, sal_Int32& nPara, sal_Int32& nIndex) const override;
    virtual bool GetWordIndices(sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart,
                                sal_Int32& nEnd) const override;
    virtual bool GetAttributeRun(sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara,
                                 sal_Int32 nIndex, bool bInCell = false) const override;
    virtual sal_Int32 GetLineCount(sal_Int32 nPara) const override;
    virtual sal_Int32 GetLineLen(sal_Int32 nPara, sal_Int32 nLine) const override;
    virtual void GetLineBoundaries(/*out*/ sal_Int32& rStart, /*out*/ sal_Int32& rEnd,
                                   sal_Int32 nParagraph, sal_Int32 nLine) const override;
    virtual sal_Int32 GetLineNumberAtIndex(sal_Int32 nPara, sal_Int32 nLine) const override;
    virtual bool Delete(const ESelection&) override;
    virtual bool InsertText(const OUString&, const ESelection&) override;
    virtual bool QuickFormatDoc(bool bFull = false) override;

    virtual sal_Int16 GetDepth(sal_Int32 nPara) const override;
    virtual bool SetDepth(sal_Int32 nPara, sal_Int16 nNewDepth) override;

    virtual const SfxItemSet* GetEmptyItemSetPtr() override;
    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void AppendParagraph() override;
    virtual sal_Int32 AppendTextPortion(sal_Int32 nPara, const OUString& rText,
                                        const SfxItemSet& rSet) override;

    virtual void CopyText(const SvxTextForwarder& rSource) override;
};

/* analog to SvxEditEngineViewForwarder */
class WeldEditViewForwarder : public SvxEditViewForwarder
{
    WeldEditAccessible& m_rEditAcc;

    WeldEditViewForwarder(const WeldEditViewForwarder&) = delete;
    WeldEditViewForwarder& operator=(const WeldEditViewForwarder&) = delete;

public:
    explicit WeldEditViewForwarder(WeldEditAccessible& rAcc);

    virtual bool IsValid() const override;

    virtual Point LogicToPixel(const Point& rPoint, const MapMode& rMapMode) const override;
    virtual Point PixelToLogic(const Point& rPoint, const MapMode& rMapMode) const override;

    virtual bool GetSelection(ESelection& rSelection) const override;
    virtual bool SetSelection(const ESelection& rSelection) override;
    virtual bool Copy() override;
    virtual bool Cut() override;
    virtual bool Paste() override;
};

class WeldEditSource : public SvxEditSource
{
    SfxBroadcaster m_aBroadCaster;
    WeldViewForwarder m_aViewFwd;
    WeldTextForwarder m_aTextFwd;
    WeldEditViewForwarder m_aEditViewFwd;
    WeldEditAccessible& m_rEditAcc;

    WeldEditSource(const WeldEditSource& rSrc)
        : SvxEditSource()
        , m_aViewFwd(rSrc.m_rEditAcc)
        , m_aTextFwd(rSrc.m_rEditAcc, *this)
        , m_aEditViewFwd(rSrc.m_rEditAcc)
        , m_rEditAcc(rSrc.m_rEditAcc)
    {
    }

    WeldEditSource& operator=(const WeldEditSource&) = delete;

public:
    WeldEditSource(WeldEditAccessible& rAcc)
        : m_aViewFwd(rAcc)
        , m_aTextFwd(rAcc, *this)
        , m_aEditViewFwd(rAcc)
        , m_rEditAcc(rAcc)
    {
    }

    virtual std::unique_ptr<SvxEditSource> Clone() const override
    {
        return std::unique_ptr<SvxEditSource>(new WeldEditSource(*this));
    }

    virtual SvxTextForwarder* GetTextForwarder() override { return &m_aTextFwd; }

    virtual SvxViewForwarder* GetViewForwarder() override { return &m_aViewFwd; }

    virtual SvxEditViewForwarder* GetEditViewForwarder(bool /*bCreate*/) override
    {
        return &m_aEditViewFwd;
    }

    virtual void UpdateData() override
    {
        // would possibly only by needed if the XText interface is implemented
        // and its text needs to be updated.
    }
    virtual SfxBroadcaster& GetBroadcaster() const override
    {
        return const_cast<WeldEditSource*>(this)->m_aBroadCaster;
    }
};
}

typedef cppu::WeakImplHelper<
    css::accessibility::XAccessible, css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleContext, css::accessibility::XAccessibleEventBroadcaster>
    WeldEditAccessibleBaseClass;

class WeldEditAccessible : public WeldEditAccessibleBaseClass
{
    weld::CustomWidgetController* m_pController;
    EditEngine* m_pEditEngine;
    EditView* m_pEditView;
    std::unique_ptr<::accessibility::AccessibleTextHelper> m_xTextHelper;

public:
    WeldEditAccessible(weld::CustomWidgetController* pController)
        : m_pController(pController)
        , m_pEditEngine(nullptr)
        , m_pEditView(nullptr)
    {
    }

    ::accessibility::AccessibleTextHelper* GetTextHelper() { return m_xTextHelper.get(); }

    void Init(EditEngine* pEditEngine, EditView* pEditView)
    {
        m_pEditEngine = pEditEngine;
        m_pEditView = pEditView;
        m_xTextHelper.reset(
            new ::accessibility::AccessibleTextHelper(std::make_unique<WeldEditSource>(*this)));
        m_xTextHelper->SetEventSource(this);
    }

    EditEngine* GetEditEngine() { return m_pEditEngine; }
    EditView* GetEditView() { return m_pEditView; }

    void ClearWin()
    {
        // remove handler before current object gets destroyed
        // (avoid handler being called for already dead object)
        m_pEditEngine->SetNotifyHdl(Link<EENotify&, void>());

        m_pEditEngine = nullptr;
        m_pEditView = nullptr;
        m_pController = nullptr; // implicitly results in AccessibleStateType::DEFUNC set

        //! make TextHelper implicitly release C++ references to some core objects
        m_xTextHelper->SetEditSource(::std::unique_ptr<SvxEditSource>());

        //! make TextHelper release references
        //! (e.g. the one set by the 'SetEventSource' call)
        m_xTextHelper->Dispose();
        m_xTextHelper.reset();
    }

    // XAccessible
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override
    {
        return this;
    }

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(const css::awt::Point& rPoint) override
    {
        //! the arguments coordinates are relative to the current window !
        //! Thus the top left-point is (0, 0)
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        Size aSz(m_pController->GetOutputSizePixel());
        return rPoint.X >= 0 && rPoint.Y >= 0 && rPoint.X < aSz.Width() && rPoint.Y < aSz.Height();
    }

    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleAtPoint(const css::awt::Point& rPoint) override
    {
        SolarMutexGuard aGuard;
        if (!m_xTextHelper)
            throw css::uno::RuntimeException();

        return m_xTextHelper->GetAt(rPoint);
    }

    virtual css::awt::Rectangle SAL_CALL getBounds() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        const Point aOutPos;
        const Size aOutSize(m_pController->GetOutputSizePixel());
        css::awt::Rectangle aRet;

        aRet.X = aOutPos.X();
        aRet.Y = aOutPos.Y();
        aRet.Width = aOutSize.Width();
        aRet.Height = aOutSize.Height();

        return aRet;
    }

    virtual css::awt::Point SAL_CALL getLocation() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        const css::awt::Rectangle aRect(getBounds());
        css::awt::Point aRet;

        aRet.X = aRect.X;
        aRet.Y = aRect.Y;

        return aRet;
    }

    virtual css::awt::Point SAL_CALL getLocationOnScreen() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        css::awt::Point aScreenLoc(0, 0);

        css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
        if (xParent)
        {
            css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(
                xParent->getAccessibleContext());
            css::uno::Reference<css::accessibility::XAccessibleComponent> xParentComponent(
                xParentContext, css::uno::UNO_QUERY);
            OSL_ENSURE(xParentComponent.is(),
                       "WeldEditAccessible::getLocationOnScreen: no parent component!");
            if (xParentComponent.is())
            {
                css::awt::Point aParentScreenLoc(xParentComponent->getLocationOnScreen());
                css::awt::Point aOwnRelativeLoc(getLocation());
                aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
                aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
            }
        }

        return aScreenLoc;
    }

    virtual css::awt::Size SAL_CALL getSize() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        Size aSz(m_pController->GetOutputSizePixel());
        return css::awt::Size(aSz.Width(), aSz.Height());
    }

    virtual void SAL_CALL grabFocus() override { m_pController->GrabFocus(); }

    virtual sal_Int32 SAL_CALL getForeground() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        Color nCol = m_pEditEngine->GetAutoColor();
        return static_cast<sal_Int32>(nCol);
    }

    virtual sal_Int32 SAL_CALL getBackground() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        Color nCol = m_pEditEngine->GetBackgroundColor();
        return static_cast<sal_Int32>(nCol);
    }

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override
    {
        if (m_xTextHelper)
            return m_xTextHelper->GetChildCount();
        return 0;
    }

    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleChild(sal_Int32 i) override
    {
        if (m_xTextHelper)
            return m_xTextHelper->GetChild(i);
        throw css::lang::IndexOutOfBoundsException(); // there is no child...
    }

    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleParent() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        return m_pController->GetDrawingArea()->get_accessible_parent();
    }

    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        // -1 for child not found/no parent (according to specification)
        sal_Int32 nRet = -1;

        css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
        if (!xParent)
            return nRet;

        try
        {
            css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(
                xParent->getAccessibleContext());

            //  iterate over parent's children and search for this object
            if (xParentContext.is())
            {
                sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
                for (sal_Int32 nChild = 0; (nChild < nChildCount) && (-1 == nRet); ++nChild)
                {
                    css::uno::Reference<css::accessibility::XAccessible> xChild(
                        xParentContext->getAccessibleChild(nChild));
                    if (xChild.get() == this)
                        nRet = nChild;
                }
            }
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("svx", "WeldEditAccessible::getAccessibleIndexInParent");
        }

        return nRet;
    }

    virtual sal_Int16 SAL_CALL getAccessibleRole() override
    {
        return css::accessibility::AccessibleRole::TEXT_FRAME;
    }

    virtual OUString SAL_CALL getAccessibleDescription() override
    {
        SolarMutexGuard aGuard;

        OUString aRet;

        if (m_pController)
        {
            aRet = m_pController->GetAccessibleDescription();
        }

        return aRet;
    }

    virtual OUString SAL_CALL getAccessibleName() override
    {
        SolarMutexGuard aGuard;

        OUString aRet;

        if (m_pController)
        {
            aRet = m_pController->GetAccessibleName();
        }

        return aRet;
    }

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet>
        SAL_CALL getAccessibleRelationSet() override
    {
        SolarMutexGuard aGuard;
        if (!m_pController)
            throw css::uno::RuntimeException();

        return m_pController->GetDrawingArea()->get_accessible_relation_set();
    }

    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet>
        SAL_CALL getAccessibleStateSet() override
    {
        SolarMutexGuard aGuard;
        ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper;

        css::uno::Reference<css::accessibility::XAccessibleStateSet> xStateSet(pStateSet);

        if (!m_pController || !m_xTextHelper)
            pStateSet->AddState(css::accessibility::AccessibleStateType::DEFUNC);
        else
        {
            pStateSet->AddState(css::accessibility::AccessibleStateType::MULTI_LINE);
            pStateSet->AddState(css::accessibility::AccessibleStateType::ENABLED);
            pStateSet->AddState(css::accessibility::AccessibleStateType::EDITABLE);
            pStateSet->AddState(css::accessibility::AccessibleStateType::FOCUSABLE);
            pStateSet->AddState(css::accessibility::AccessibleStateType::SELECTABLE);
            if (m_pController->HasFocus())
                pStateSet->AddState(css::accessibility::AccessibleStateType::FOCUSED);
            if (m_pController->IsActive())
                pStateSet->AddState(css::accessibility::AccessibleStateType::ACTIVE);
            if (m_pController->IsVisible())
                pStateSet->AddState(css::accessibility::AccessibleStateType::SHOWING);
            if (m_pController->IsReallyVisible())
                pStateSet->AddState(css::accessibility::AccessibleStateType::VISIBLE);
            if (COL_TRANSPARENT != m_pEditEngine->GetBackgroundColor())
                pStateSet->AddState(css::accessibility::AccessibleStateType::OPAQUE);
        }

        return xStateSet;
    }

    virtual css::lang::Locale SAL_CALL getLocale() override
    {
        SolarMutexGuard aGuard;
        return LanguageTag(m_pEditEngine->GetDefaultLanguage()).getLocale();
    }

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& rListener) override
    {
        if (!m_xTextHelper) // not disposing (about to destroy view shell)
            return;
        m_xTextHelper->AddEventListener(rListener);
    }

    virtual void SAL_CALL removeAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& rListener) override
    {
        if (!m_xTextHelper) // not disposing (about to destroy view shell)
            return;
        m_xTextHelper->RemoveEventListener(rListener);
    }
};

css::uno::Reference<css::accessibility::XAccessible> WeldEditView::CreateAccessible()
{
    if (!m_xAccessible.is())
        m_xAccessible.set(new WeldEditAccessible(this));
    return css::uno::Reference<css::accessibility::XAccessible>(m_xAccessible.get());
}

WeldEditView::~WeldEditView()
{
    if (m_xAccessible.is())
    {
        m_xAccessible->ClearWin(); // make Accessible nonfunctional
        m_xAccessible.clear();
    }
}

bool WeldViewForwarder::IsValid() const { return m_rEditAcc.GetEditView() != nullptr; }

Point WeldViewForwarder::LogicToPixel(const Point& rPoint, const MapMode& rMapMode) const
{
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (!pEditView)
        return Point();
    OutputDevice& rOutDev = pEditView->GetOutputDevice();
    MapMode aMapMode(rOutDev.GetMapMode());
    Point aPoint(OutputDevice::LogicToLogic(rPoint, rMapMode, MapMode(aMapMode.GetMapUnit())));
    aMapMode.SetOrigin(Point());
    return rOutDev.LogicToPixel(aPoint, aMapMode);
}

Point WeldViewForwarder::PixelToLogic(const Point& rPoint, const MapMode& rMapMode) const
{
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (!pEditView)
        return Point();
    OutputDevice& rOutDev = pEditView->GetOutputDevice();
    MapMode aMapMode(rOutDev.GetMapMode());
    aMapMode.SetOrigin(Point());
    Point aPoint(rOutDev.PixelToLogic(rPoint, aMapMode));
    return OutputDevice::LogicToLogic(aPoint, MapMode(aMapMode.GetMapUnit()), rMapMode);
}

WeldTextForwarder::WeldTextForwarder(WeldEditAccessible& rAcc, WeldEditSource& rSource)
    : m_rEditAcc(rAcc)
    , m_rEditSource(rSource)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(LINK(this, WeldTextForwarder, NotifyHdl));
}

WeldTextForwarder::~WeldTextForwarder()
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link<EENotify&, void>());
}

IMPL_LINK(WeldTextForwarder, NotifyHdl, EENotify&, rNotify, void)
{
    if (EditEngine* pEditEngine = m_rEditAcc.GetEditEngine())
    {
        if (rNotify.eNotificationType == EE_NOTIFY_PROCESSNOTIFICATIONS
            && !pEditEngine->GetUpdateMode())
        {
            // tdf#143088 an UpdateMode of false will just to on to cause
            // AccessibleTextHelper_Impl::GetTextForwarder to throw an
            // exception as a Frozen EditEngine is considered Invalid so return
            // early instead
            return;
        }
    }

    ::std::unique_ptr<SfxHint> aHint = SvxEditSourceHelper::EENotification2Hint(&rNotify);
    if (aHint)
        m_rEditSource.GetBroadcaster().Broadcast(*aHint);
}

sal_Int32 WeldTextForwarder::GetParagraphCount() const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetParagraphCount() : 0;
}

sal_Int32 WeldTextForwarder::GetTextLen(sal_Int32 nParagraph) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetTextLen(nParagraph) : 0;
}

OUString WeldTextForwarder::GetText(const ESelection& rSel) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    OUString aRet;
    if (pEditEngine)
        aRet = pEditEngine->GetText(rSel);
    return convertLineEnd(aRet, GetSystemLineEnd());
}

SfxItemSet WeldTextForwarder::GetAttribs(const ESelection& rSel,
                                         EditEngineAttribs nOnlyHardAttrib) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    assert(pEditEngine && "EditEngine missing");
    if (rSel.nStartPara == rSel.nEndPara)
    {
        GetAttribsFlags nFlags = GetAttribsFlags::NONE;
        switch (nOnlyHardAttrib)
        {
            case EditEngineAttribs::All:
                nFlags = GetAttribsFlags::ALL;
                break;
            case EditEngineAttribs::OnlyHard:
                nFlags = GetAttribsFlags::CHARATTRIBS;
                break;
            default:
                SAL_WARN("svx", "unknown flags for WeldTextForwarder::GetAttribs");
        }

        return pEditEngine->GetAttribs(rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags);
    }
    else
    {
        return pEditEngine->GetAttribs(rSel, nOnlyHardAttrib);
    }
}

SfxItemSet WeldTextForwarder::GetParaAttribs(sal_Int32 nPara) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    assert(pEditEngine && "EditEngine missing");

    SfxItemSet aSet(pEditEngine->GetParaAttribs(nPara));

    sal_uInt16 nWhich = EE_PARA_START;
    while (nWhich <= EE_PARA_END)
    {
        if (aSet.GetItemState(nWhich) != SfxItemState::SET)
        {
            if (pEditEngine->HasParaAttrib(nPara, nWhich))
                aSet.Put(pEditEngine->GetParaAttrib(nPara, nWhich));
        }
        nWhich++;
    }

    return aSet;
}

void WeldTextForwarder::SetParaAttribs(sal_Int32 nPara, const SfxItemSet& rSet)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetParaAttribs(nPara, rSet);
}

SfxItemPool* WeldTextForwarder::GetPool() const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetEmptyItemSet().GetPool() : nullptr;
}

void WeldTextForwarder::RemoveAttribs(const ESelection& rSelection)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->RemoveAttribs(rSelection, false /*bRemoveParaAttribs*/, 0);
}

void WeldTextForwarder::GetPortions(sal_Int32 nPara, std::vector<sal_Int32>& rList) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->GetPortions(nPara, rList);
}

void WeldTextForwarder::QuickInsertText(const OUString& rText, const ESelection& rSel)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertText(rText, rSel);
}

void WeldTextForwarder::QuickInsertLineBreak(const ESelection& rSel)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertLineBreak(rSel);
}

void WeldTextForwarder::QuickInsertField(const SvxFieldItem& rFld, const ESelection& rSel)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertField(rFld, rSel);
}

void WeldTextForwarder::QuickSetAttribs(const SfxItemSet& rSet, const ESelection& rSel)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickSetAttribs(rSet, rSel);
}

bool WeldTextForwarder::IsValid() const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    // cannot reliably query EditEngine state
    // while in the middle of an update
    return pEditEngine && pEditEngine->GetUpdateMode();
}

OUString WeldTextForwarder::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara,
                                           sal_Int32 nPos, std::optional<Color>& rpTxtColor,
                                           std::optional<Color>& rpFldColor)
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor)
                       : OUString();
}

void WeldTextForwarder::FieldClicked(const SvxFieldItem&) {}

static SfxItemState GetSvxEditEngineItemState(EditEngine const& rEditEngine, const ESelection& rSel,
                                              sal_uInt16 nWhich)
{
    std::vector<EECharAttrib> aAttribs;

    const SfxPoolItem* pLastItem = nullptr;

    SfxItemState eState = SfxItemState::DEFAULT;

    // check all paragraphs inside the selection
    for (sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++)
    {
        SfxItemState eParaState = SfxItemState::DEFAULT;

        // calculate start and endpos for this paragraph
        sal_Int32 nPos = 0;
        if (rSel.nStartPara == nPara)
            nPos = rSel.nStartPos;

        sal_Int32 nEndPos = rSel.nEndPos;
        if (rSel.nEndPara != nPara)
            nEndPos = rEditEngine.GetTextLen(nPara);

        // get list of char attribs
        rEditEngine.GetCharAttribs(nPara, aAttribs);

        bool bEmpty = true; // we found no item inside the selection of this paragraph
        bool bGaps = false; // we found items but there are gaps between them
        sal_Int32 nLastEnd = nPos;

        const SfxPoolItem* pParaItem = nullptr;

        for (const auto& rAttrib : aAttribs)
        {
            OSL_ENSURE(rAttrib.pAttr, "GetCharAttribs gives corrupt data");

            const bool bEmptyPortion = (rAttrib.nStart == rAttrib.nEnd);
            if ((!bEmptyPortion && (rAttrib.nStart >= nEndPos))
                || (bEmptyPortion && (rAttrib.nStart > nEndPos)))
                break; // break if we are already behind our selection

            if ((!bEmptyPortion && (rAttrib.nEnd <= nPos))
                || (bEmptyPortion && (rAttrib.nEnd < nPos)))
                continue; // or if the attribute ends before our selection

            if (rAttrib.pAttr->Which() != nWhich)
                continue; // skip if is not the searched item

            // if we already found an item
            if (pParaItem)
            {
                // ... and its different to this one than the state is don't care
                if (*pParaItem != *(rAttrib.pAttr))
                    return SfxItemState::DONTCARE;
            }
            else
            {
                pParaItem = rAttrib.pAttr;
            }

            if (bEmpty)
                bEmpty = false;

            if (!bGaps && rAttrib.nStart > nLastEnd)
                bGaps = true;

            nLastEnd = rAttrib.nEnd;
        }

        if (!bEmpty && !bGaps && nLastEnd < (nEndPos - 1))
            bGaps = true;
        if (bEmpty)
            eParaState = SfxItemState::DEFAULT;
        else if (bGaps)
            eParaState = SfxItemState::DONTCARE;
        else
            eParaState = SfxItemState::SET;

        // if we already found an item check if we found the same
        if (pLastItem)
        {
            if ((pParaItem == nullptr) || (*pLastItem != *pParaItem))
                return SfxItemState::DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

SfxItemState WeldTextForwarder::GetItemState(const ESelection& rSel, sal_uInt16 nWhich) const
{
    SfxItemState nState = SfxItemState::DISABLED;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        nState = GetSvxEditEngineItemState(*pEditEngine, rSel, nWhich);
    return nState;
}

SfxItemState WeldTextForwarder::GetItemState(sal_Int32 nPara, sal_uInt16 nWhich) const
{
    SfxItemState nState = SfxItemState::DISABLED;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        const SfxItemSet& rSet = pEditEngine->GetParaAttribs(nPara);
        nState = rSet.GetItemState(nWhich);
    }
    return nState;
}

LanguageType WeldTextForwarder::GetLanguage(sal_Int32 nPara, sal_Int32 nIndex) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLanguage(nPara, nIndex) : LANGUAGE_NONE;
}

sal_Int32 WeldTextForwarder::GetFieldCount(sal_Int32 nPara) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldCount(nPara) : 0;
}

EFieldInfo WeldTextForwarder::GetFieldInfo(sal_Int32 nPara, sal_uInt16 nField) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldInfo(nPara, nField) : EFieldInfo();
}

EBulletInfo WeldTextForwarder::GetBulletInfo(sal_Int32 /*nPara*/) const { return EBulletInfo(); }

tools::Rectangle WeldTextForwarder::GetCharBounds(sal_Int32 nPara, sal_Int32 nIndex) const
{
    tools::Rectangle aRect(0, 0, 0, 0);
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();

    if (pEditEngine)
    {
        // Handle virtual position one-past-the end of the string
        if (nIndex >= pEditEngine->GetTextLen(nPara))
        {
            if (nIndex)
                aRect = pEditEngine->GetCharacterBounds(EPosition(nPara, nIndex - 1));

            aRect.Move(aRect.Right() - aRect.Left(), 0);
            aRect.SetSize(Size(1, pEditEngine->GetTextHeight()));
        }
        else
        {
            aRect = pEditEngine->GetCharacterBounds(EPosition(nPara, nIndex));
        }
    }
    return aRect;
}

tools::Rectangle WeldTextForwarder::GetParaBounds(sal_Int32 nPara) const
{
    tools::Rectangle aRect(0, 0, 0, 0);
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();

    if (pEditEngine)
    {
        const Point aPnt = pEditEngine->GetDocPosTopLeft(nPara);
        const sal_Int32 nWidth = pEditEngine->CalcTextWidth();
        const sal_Int32 nHeight = pEditEngine->GetTextHeight(nPara);
        aRect = tools::Rectangle(aPnt.X(), aPnt.Y(), aPnt.X() + nWidth, aPnt.Y() + nHeight);
    }

    return aRect;
}

MapMode WeldTextForwarder::GetMapMode() const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetRefMapMode() : MapMode(MapUnit::Map100thMM);
}

OutputDevice* WeldTextForwarder::GetRefDevice() const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetRefDevice() : nullptr;
}

bool WeldTextForwarder::GetIndexAtPoint(const Point& rPos, sal_Int32& nPara,
                                        sal_Int32& nIndex) const
{
    bool bRes = false;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        EPosition aDocPos = pEditEngine->FindDocPosition(rPos);
        nPara = aDocPos.nPara;
        nIndex = aDocPos.nIndex;
        bRes = true;
    }
    return bRes;
}

bool WeldTextForwarder::GetWordIndices(sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart,
                                       sal_Int32& nEnd) const
{
    bool bRes = false;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        ESelection aRes = pEditEngine->GetWord(ESelection(nPara, nIndex, nPara, nIndex),
                                               css::i18n::WordType::DICTIONARY_WORD);

        if (aRes.nStartPara == nPara && aRes.nStartPara == aRes.nEndPara)
        {
            nStart = aRes.nStartPos;
            nEnd = aRes.nEndPos;

            bRes = true;
        }
    }

    return bRes;
}

bool WeldTextForwarder::GetAttributeRun(sal_Int32& nStartIndex, sal_Int32& nEndIndex,
                                        sal_Int32 nPara, sal_Int32 nIndex, bool bInCell) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (!pEditEngine)
        return false;
    SvxEditSourceHelper::GetAttributeRun(nStartIndex, nEndIndex, *pEditEngine, nPara, nIndex,
                                         bInCell);
    return true;
}

sal_Int32 WeldTextForwarder::GetLineCount(sal_Int32 nPara) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineCount(nPara) : 0;
}

sal_Int32 WeldTextForwarder::GetLineLen(sal_Int32 nPara, sal_Int32 nLine) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineLen(nPara, nLine) : 0;
}

void WeldTextForwarder::GetLineBoundaries(/*out*/ sal_Int32& rStart, /*out*/ sal_Int32& rEnd,
                                          sal_Int32 nPara, sal_Int32 nLine) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->GetLineBoundaries(rStart, rEnd, nPara, nLine);
    else
        rStart = rEnd = 0;
}

sal_Int32 WeldTextForwarder::GetLineNumberAtIndex(sal_Int32 nPara, sal_Int32 nIndex) const
{
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineNumberAtIndex(nPara, nIndex) : 0;
}

bool WeldTextForwarder::QuickFormatDoc(bool /*bFull*/)
{
    bool bRes = false;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

sal_Int16 WeldTextForwarder::GetDepth(sal_Int32 /*nPara*/) const
{
    // math has no outliner...
    return -1;
}

bool WeldTextForwarder::SetDepth(sal_Int32 /*nPara*/, sal_Int16 nNewDepth)
{
    // math has no outliner...
    return -1 == nNewDepth; // is it the value from 'GetDepth' ?
}

bool WeldTextForwarder::Delete(const ESelection& rSelection)
{
    bool bRes = false;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickDelete(rSelection);
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

bool WeldTextForwarder::InsertText(const OUString& rStr, const ESelection& rSelection)
{
    bool bRes = false;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickInsertText(rStr, rSelection);
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

const SfxItemSet* WeldTextForwarder::GetEmptyItemSetPtr()
{
    const SfxItemSet* pItemSet = nullptr;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pItemSet = &pEditEngine->GetEmptyItemSet();
    }
    return pItemSet;
}

void WeldTextForwarder::AppendParagraph()
{
    // append an empty paragraph
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        sal_Int32 nParaCount = pEditEngine->GetParagraphCount();
        pEditEngine->InsertParagraph(nParaCount, OUString());
    }
}

sal_Int32 WeldTextForwarder::AppendTextPortion(sal_Int32 nPara, const OUString& rText,
                                               const SfxItemSet& rSet)
{
    sal_uInt16 nRes = 0;
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine && nPara < pEditEngine->GetParagraphCount())
    {
        // append text
        ESelection aSel(nPara, pEditEngine->GetTextLen(nPara));
        pEditEngine->QuickInsertText(rText, aSel);

        // set attributes for new appended text
        nRes = aSel.nEndPos = pEditEngine->GetTextLen(nPara);
        pEditEngine->QuickSetAttribs(rSet, aSel);
    }
    return nRes;
}

void WeldTextForwarder::CopyText(const SvxTextForwarder& rSource)
{
    const WeldTextForwarder* pSourceForwarder = dynamic_cast<const WeldTextForwarder*>(&rSource);
    if (!pSourceForwarder)
        return;
    EditEngine* pSourceEditEngine = pSourceForwarder->m_rEditAcc.GetEditEngine();
    EditEngine* pEditEngine = m_rEditAcc.GetEditEngine();
    if (pEditEngine && pSourceEditEngine)
    {
        std::unique_ptr<EditTextObject> pNewTextObject = pSourceEditEngine->CreateTextObject();
        pEditEngine->SetText(*pNewTextObject);
    }
}

WeldEditViewForwarder::WeldEditViewForwarder(WeldEditAccessible& rAcc)
    : m_rEditAcc(rAcc)
{
}

bool WeldEditViewForwarder::IsValid() const { return m_rEditAcc.GetEditView() != nullptr; }

Point WeldEditViewForwarder::LogicToPixel(const Point& rPoint, const MapMode& rMapMode) const
{
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (!pEditView)
        return Point();
    OutputDevice& rOutDev = pEditView->GetOutputDevice();
    MapMode aMapMode(rOutDev.GetMapMode());
    Point aPoint(OutputDevice::LogicToLogic(rPoint, rMapMode, MapMode(aMapMode.GetMapUnit())));
    aMapMode.SetOrigin(Point());
    return rOutDev.LogicToPixel(aPoint, aMapMode);
}

Point WeldEditViewForwarder::PixelToLogic(const Point& rPoint, const MapMode& rMapMode) const
{
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (!pEditView)
        return Point();
    OutputDevice& rOutDev = pEditView->GetOutputDevice();
    MapMode aMapMode(rOutDev.GetMapMode());
    aMapMode.SetOrigin(Point());
    Point aPoint(rOutDev.PixelToLogic(rPoint, aMapMode));
    return OutputDevice::LogicToLogic(aPoint, MapMode(aMapMode.GetMapUnit()), rMapMode);
}

bool WeldEditViewForwarder::GetSelection(ESelection& rSelection) const
{
    bool bRes = false;
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (pEditView)
    {
        rSelection = pEditView->GetSelection();
        bRes = true;
    }
    return bRes;
}

bool WeldEditViewForwarder::SetSelection(const ESelection& rSelection)
{
    bool bRes = false;
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->SetSelection(rSelection);
        bRes = true;
    }
    return bRes;
}

bool WeldEditViewForwarder::Copy()
{
    bool bRes = false;
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Copy();
        bRes = true;
    }
    return bRes;
}

bool WeldEditViewForwarder::Cut()
{
    bool bRes = false;
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Cut();
        bRes = true;
    }
    return bRes;
}

bool WeldEditViewForwarder::Paste()
{
    bool bRes = false;
    EditView* pEditView = m_rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Paste();
        bRes = true;
    }
    return bRes;
}

void WeldEditView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_size_request());
    if (aSize.Width() == -1)
        aSize.setWidth(500);
    if (aSize.Height() == -1)
        aSize.setHeight(100);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));
    aSize = aOutputSize;
    aSize.setHeight(aSize.Height());

    makeEditEngine();
    m_xEditEngine->SetPaperSize(aSize);
    m_xEditEngine->SetRefDevice(&rDevice);

    m_xEditEngine->SetControlWord(m_xEditEngine->GetControlWord() | EEControlBits::MARKFIELDS);

    m_xEditView.reset(new EditView(m_xEditEngine.get(), nullptr));
    m_xEditView->setEditViewCallbacks(this);
    m_xEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));

    m_xEditView->SetBackgroundColor(aBgColor);
    m_xEditEngine->InsertView(m_xEditView.get());

    pDrawingArea->set_cursor(PointerStyle::Text);

    if (m_xAccessible.is())
        m_xAccessible->Init(m_xEditEngine.get(), m_xEditView.get());
}

int WeldEditView::GetSurroundingText(OUString& rSurrounding)
{
    if (!m_xEditView)
        return -1;
    rSurrounding = m_xEditView->GetSurroundingText();
    return m_xEditView->GetSurroundingTextSelection().Min();
}

bool WeldEditView::DeleteSurroundingText(const Selection& rRange)
{
    if (!m_xEditView)
        return false;
    return m_xEditView->DeleteSurroundingText(rRange);
}

void WeldEditView::GetFocus()
{
    if (m_xEditView)
        m_xEditView->ShowCursor(false);

    weld::CustomWidgetController::GetFocus();

    if (m_xAccessible.is())
    {
        // Note: will implicitly send the AccessibleStateType::FOCUSED event
        ::accessibility::AccessibleTextHelper* pHelper = m_xAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus();
    }
}

void WeldEditView::LoseFocus()
{
    weld::CustomWidgetController::LoseFocus();
    Invalidate(); // redraw without cursor

    if (m_xAccessible.is())
    {
        // Note: will implicitly send the AccessibleStateType::FOCUSED event
        ::accessibility::AccessibleTextHelper* pHelper = m_xAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus(false);
    }
}

bool WeldEditView::CanFocus() const { return true; }

namespace
{
class WeldEditViewUIObject final : public DrawingAreaUIObject
{
private:
    WeldEditView* mpEditView;

public:
    WeldEditViewUIObject(const VclPtr<vcl::Window>& rDrawingArea)
        : DrawingAreaUIObject(rDrawingArea)
        , mpEditView(static_cast<WeldEditView*>(mpController))
    {
    }

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow)
    {
        return std::unique_ptr<UIObject>(new WeldEditViewUIObject(pWindow));
    }

    virtual StringMap get_state() override
    {
        StringMap aMap = WindowUIObject::get_state();
        aMap["Text"] = mpEditView->GetText();
        return aMap;
    }

private:
    virtual OUString get_name() const override { return "WeldEditViewUIObject"; }
};
}

FactoryFunction WeldEditView::GetUITestFactory() const { return WeldEditViewUIObject::create; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
