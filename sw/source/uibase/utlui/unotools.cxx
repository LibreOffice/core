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

#include <sal/config.h>

#include <string_view>

#include <swtypes.hxx>

#include <strings.hrc>
#include <unotools.hxx>
#include <unoprnms.hxx>
#include <unotextcursor.hxx>
#include <i18nutil/unicode.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/string.h>
#include <svtools/colorcfg.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/jobset.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <TextCursorHelper.hxx>
#include <doc.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral cFactory = u"private:factory/swriter";

static void disableScrollBars(uno::Reference< beans::XPropertySet > const & xViewProps,
    bool bEnableOnlineMode)
{
    //the scrollbar logic is kind of busted looking in writer, when the hori scrollbar
    //property is changed then the hori scrollbar is enabled if the property is
    //true or browse (online) mode is enabled. So...
    //disable online mode
    //turn off scrollbars
    //turn back on online mode if that's what we want
    //which subverts the (dodgy/buggy) scrollbar setting

    //To reproduce this problem, in edit->autotext and click through
    //the examples and see if the preview gets a horizontal scrollbar
    uno::Any aFalseSet(uno::Any(false));
    xViewProps->setPropertyValue(UNO_NAME_SHOW_ONLINE_LAYOUT, aFalseSet);

    xViewProps->setPropertyValue(UNO_NAME_SHOW_HORI_SCROLL_BAR, aFalseSet);
    xViewProps->setPropertyValue(UNO_NAME_SHOW_VERT_SCROLL_BAR, aFalseSet);

    if (bEnableOnlineMode)
    {
        xViewProps->setPropertyValue(UNO_NAME_SHOW_ONLINE_LAYOUT, uno::Any(true));
    }
}

SwOneExampleFrame::SwOneExampleFrame(sal_uInt32 nFlags,
                                 const Link<SwOneExampleFrame&,void>* pInitializedLink,
                                 const OUString* pURL)
    : m_aLoadedIdle("sw uibase SwOneExampleFrame Loaded")
    , m_pModuleView(SW_MOD()->GetView())
    , m_nStyleFlags(nFlags)
    , m_bIsInitialized(false)
{
    if (pURL && !pURL->isEmpty())
        m_sArgumentURL = *pURL;

    if( pInitializedLink )
        m_aInitializedLink = *pInitializedLink;

    // the controller is asynchronously set
    m_aLoadedIdle.SetInvokeHandler(LINK(this, SwOneExampleFrame, TimeoutHdl));
    m_aLoadedIdle.SetPriority(TaskPriority::HIGH_IDLE);
}

void SwOneExampleFrame::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    m_xVirDev = VclPtr<VirtualDevice>::Create();
    Size aSize(m_xVirDev->LogicToPixel(Size(150, 188), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);
    CreateControl();
}

bool SwOneExampleFrame::Command(const CommandEvent& rCEvt)
{
    switch (rCEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            //#125881# quickly clicking crashes because the control is not fully initialized
            if (m_xController.is())
                return CreatePopup(rCEvt.GetMousePosPixel());
        }
        break;
        default:;
        break;
    }
    return CustomWidgetController::Command(rCEvt);
}

void SwOneExampleFrame::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    Size aSize(GetOutputSizePixel());
    // m_xVirDev instead of rRenderContext just to avoid overlays in writer re-triggering
    // invalidate on rRenderContext if it is a vcl::Window, which is the "classic" gen mode
    m_xVirDev->SetOutputSizePixel(aSize);

    Color aBgColor = SW_MOD()->GetColorConfig().GetColorValue(::svtools::DOCCOLOR).nColor;
    m_xVirDev->DrawWallpaper(tools::Rectangle(Point(), aSize), aBgColor);

    if (m_xCursor)
    {
        uno::Reference<view::XViewSettingsSupplier> xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet>  xViewProps = xSettings->getViewSettings();
        uno::Any aZoom = xViewProps->getPropertyValue(UNO_NAME_ZOOM_VALUE);
        sal_Int16 nZoom = 100;
        aZoom >>= nZoom;

        double fZoom = 100.0 / nZoom;

        m_xVirDev->Push(vcl::PushFlags::ALL);
        m_xVirDev->SetMapMode(MapMode(MapUnit::MapTwip));
        SwDoc *pDoc = m_xCursor->GetDoc();
        SwDocShell* pShell = pDoc->GetDocShell();
        tools::Rectangle aRect(Point(), m_xVirDev->PixelToLogic(aSize));
        pShell->SetVisArea(tools::Rectangle(Point(), Size(aRect.GetWidth() * fZoom,
                                                          aRect.GetHeight() * fZoom)));
        pShell->DoDraw(m_xVirDev.get(), aRect.TopLeft(), aRect.GetSize(), JobSetup(), ASPECT_CONTENT, true);
        m_xVirDev->Pop();
    }

    rRenderContext.DrawOutDev(Point(), aSize, Point(), aSize, *m_xVirDev);
}

SwOneExampleFrame::~SwOneExampleFrame()
{
    DisposeControl();
}

void SwOneExampleFrame::CreateControl()
{
    // create new doc
    OUString sTempURL(cFactory);
    if(!m_sArgumentURL.isEmpty())
        sTempURL = m_sArgumentURL;

    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    uno::Sequence<beans::PropertyValue> args( comphelper::InitPropertySequence({
            { "DocumentService", uno::Any(OUString("com.sun.star.text.TextDocument")) },
            { "OpenFlags", uno::Any(OUString("-RB")) },
            { "Referer", uno::Any(OUString("private:user")) },
            { "ReadOnly", uno::Any(true) },
            { "Hidden", uno::Any(true) }
        }));

    m_xModel.set(xDesktop->loadComponentFromURL(sTempURL, "_blank", 0, args), uno::UNO_QUERY);

    m_aLoadedIdle.Start();
}

void SwOneExampleFrame::DisposeControl()
{
    m_aLoadedIdle.Stop();
    m_xCursor = nullptr;
    if (m_xModel)
    {
        m_xModel->dispose();
        m_xModel = nullptr;
    }
    m_xController = nullptr;
}

IMPL_LINK( SwOneExampleFrame, TimeoutHdl, Timer*, pTimer, void )
{
    if (!m_xModel.is())
        return;

    m_xController = m_xModel->getCurrentController();

    if (m_xController.is())
    {
        uno::Reference<frame::XFrame> xFrame = m_xController->getFrame();
        uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                uno::Reference< frame::XLayoutManager > xLayoutManager;
                uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                aValue >>= xLayoutManager;
                if ( xLayoutManager.is() )
                    xLayoutManager->setVisible( false );
            }
            catch (const uno::Exception&)
            {
            }
        }

        //now the ViewOptions should be set properly
        uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        const uno::Any aTrueSet( true );
        const uno::Any aFalseSet( false );

        if( !m_bIsInitialized )
        {
            xViewProps->setPropertyValue(UNO_NAME_SHOW_BREAKS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_DRAWINGS, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_FIELD_COMMANDS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_GRAPHICS, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_HIDE_WHITESPACE, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HIDDEN_PARAGRAPHS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HIDDEN_TEXT, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HORI_RULER, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_PARA_BREAKS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_PROTECTED_SPACES, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_SOFT_HYPHENS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_SPACES, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_TABLES, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_TABSTOPS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_VERT_RULER, aFalseSet);

            if(0 ==(m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT))
            {
                uno::Any aZoom;
                aZoom <<= sal_Int16(view::DocumentZoomType::PAGE_WIDTH_EXACT);
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
            }
            else
            {
                uno::Any aZoom;
                aZoom <<= sal_Int16(view::DocumentZoomType::BY_VALUE);
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);

                sal_Int16 nZoomValue = 75;
                if(EX_SHOW_BUSINESS_CARDS == m_nStyleFlags)
                {
                    nZoomValue = 80;
                }
                aZoom <<= nZoomValue;
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
            }

            // set onlinelayout property after setting the zoom
            disableScrollBars(xViewProps, (m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);
            m_bIsInitialized = true;
        }

        uno::Reference< text::XTextDocument >  xDoc(m_xModel, uno::UNO_QUERY);
        uno::Reference< text::XText >  xText = xDoc->getText();
        uno::Reference< text::XTextCursor > xTextCursor = xText->createTextCursor();
        m_xCursor = dynamic_cast<SwXTextCursor*>(xTextCursor.get());
        assert(bool(xTextCursor) == bool(m_xCursor) && "expect to get SwXTextCursor type here");

        //From here, a cursor is defined, which goes through the template,
        //and overwrites the template words where it is necessary.

        SwDoc *pDoc = m_xCursor ? m_xCursor->GetDoc() : nullptr;
        if (pDoc && (m_nStyleFlags & EX_LOCALIZE_TOC_STRINGS))
        {
            SwEditShell* pSh = pDoc->GetEditShell();

            do
            {
              if (pSh->GetCurWord() == "HEADING1")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING1));
              }
              else if (pSh->GetCurWord() == "ENTRY1")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY1));
              }
              else if (pSh->GetCurWord() == "HEADING11")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING11));
              }
              else if (pSh->GetCurWord() == "ENTRY11")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY11));
              }
              else if (pSh->GetCurWord() == "HEADING12")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING12));
              }
              else if (pSh->GetCurWord() == "ENTRY12")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY12));
              }
              else if (pSh->GetCurWord() == "TABLE1")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_TABLE1));
              }
              else if (pSh->GetCurWord() == "IMAGE1")
              {
                pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_IMAGE1));
              }
            }
            while(pSh->Right(sal_uInt16(1), SwCursorSkipMode::Cells, true));

            TOXTypes eTypes[] = { TOX_INDEX, TOX_USER, TOX_CONTENT };
            for (auto eType : eTypes)
            {
                const SwTOXType* pTOXType = pDoc->GetTOXType(eType, 0);
                SwTOXMarks aMarks;
                pTOXType->CollectTextMarks(aMarks);
                for (auto pMark : aMarks)
                {
                    if (pMark->GetAlternativeText() == "Chapter")
                        pMark->SetAlternativeText(SwResId(STR_IDXEXAMPLE_IDXMARK_CHAPTER));
                    else if (pMark->GetAlternativeText() == "Keyword")
                        pMark->SetAlternativeText(SwResId(STR_IDXEXAMPLE_IDXMARK_KEYWORD));
                    else if (pMark->GetAlternativeText() == "this")
                        pMark->SetAlternativeText(SwResId(STR_IDXEXAMPLE_IDXMARK_THIS));
                    else if (pMark->GetAlternativeText() == "User Directory Entry")
                        pMark->SetAlternativeText(SwResId(STR_IDXEXAMPLE_IDXMARK_USER_DIR_ENTRY));
                    else if (pMark->GetAlternativeText() == "Entry")
                        pMark->SetAlternativeText(SwResId(STR_IDXEXAMPLE_IDXMARK_ENTRY));

                    if (pMark->GetPrimaryKey() == "Primary key")
                        pMark->SetPrimaryKey(SwResId(STR_IDXEXAMPLE_IDXMARK_PRIMARY_KEY));

                    if (pMark->GetSecondaryKey() == "Secondary key")
                        pMark->SetSecondaryKey(SwResId(STR_IDXEXAMPLE_IDXMARK_SECONDARY_KEY));
                }
            }
        }

        uno::Any aPageStyle = m_xCursor->getPropertyValue(UNO_NAME_PAGE_STYLE_NAME);
        OUString sPageStyle;
        aPageStyle >>= sPageStyle;

        uno::Reference< style::XStyleFamiliesSupplier >  xSSupp( xDoc, uno::UNO_QUERY);
        uno::Reference< container::XNameAccess >  xStyles = xSSupp->getStyleFamilies();
        uno::Any aPFamily = xStyles->getByName( "PageStyles" );
        uno::Reference< container::XNameContainer >  xPFamily;

        if( EX_SHOW_DEFAULT_PAGE != m_nStyleFlags
                && (aPFamily >>= xPFamily) && !sPageStyle.isEmpty() )
        {
            uno::Any aPStyle = xPFamily->getByName( sPageStyle );
            uno::Reference< style::XStyle >  xPStyle;
            aPStyle >>= xPStyle;
            uno::Reference< beans::XPropertySet >  xPProp(xPStyle, uno::UNO_QUERY);
            uno::Any aSize = xPProp->getPropertyValue(UNO_NAME_SIZE);
            awt::Size aPSize;
            aSize >>= aPSize;
            //TODO: set page width to card width
            aPSize.Width = 10000;
            aSize <<= aPSize;
            xPProp->setPropertyValue(UNO_NAME_SIZE, aSize);

            uno::Any aZero; aZero <<= sal_Int32(0);
            xPProp->setPropertyValue(UNO_NAME_LEFT_MARGIN, aZero);
            xPProp->setPropertyValue(UNO_NAME_RIGHT_MARGIN, aZero);
        }

        uno::Reference<awt::XWindow> xWin = xFrame->getContainerWindow();
        Size aWinSize(GetOutputSizePixel());
        xWin->setPosSize(0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE);

        // can only be done here - the SFX changes the ScrollBar values
        disableScrollBars(xViewProps, (m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);

        m_aInitializedLink.Call(*this);

        uno::Reference< text::XTextViewCursorSupplier >  xCursorSupp(m_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCursor(xCursorSupp->getViewCursor(), uno::UNO_QUERY);
        if(xScrCursor.is())
            xScrCursor->screenUp();

        if (pDoc)
        {
            SwEditShell* pSh = pDoc->GetEditShell();
            if( pSh && pSh->ActionCount() )
            {
                pSh->EndAllAction();
                pSh->UnlockPaint();
            }
        }

        SW_MOD()->SetView(m_pModuleView);

        Invalidate();
    }
    else
        pTimer->Start();
}

void SwOneExampleFrame::ClearDocument()
{
    if( !m_xCursor )
        return;

    SwDoc* pDoc = m_xCursor->GetDoc();
    SwEditShell* pSh = pDoc->GetEditShell();
    pSh->LockPaint(LockPaintReason::ExampleFrame);
    pSh->StartAllAction();
    pSh->KillPams();
    pSh->ClearMark();
    pDoc->ClearDoc();
    pSh->ClearUpCursors();

    if( m_aLoadedIdle.IsActive())
    {
        pSh->EndAllAction();
        pSh->UnlockPaint();
    }
    m_aLoadedIdle.Start();
}

bool SwOneExampleFrame::CreatePopup(const Point& rPt)
{
    if (EX_SHOW_ONLINE_LAYOUT != m_nStyleFlags)
        return false;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/previewmenu.ui"));
    std::unique_ptr<weld::Menu> xPop(xBuilder->weld_menu("previewmenu"));

    uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

    uno::Any aZoom = xViewProps->getPropertyValue(UNO_NAME_ZOOM_VALUE);
    sal_Int16 nZoom = 0;
    aZoom >>= nZoom;

    for (auto const nZoomPreset : { 20, 40, 50, 75, 100 })
    {
        OUString sTemp = unicode::formatPercent(nZoomPreset,
            Application::GetSettings().GetUILanguageTag());
        OUString sIdent = "zoom" + OUString::number(nZoomPreset);
        xPop->set_label(sIdent, sTemp);
        if (nZoom == nZoomPreset)
            xPop->set_active(sIdent, true);
    }

    PopupHdl(xPop->popup_at_rect(GetDrawingArea(), tools::Rectangle(rPt, Size(1, 1))));

    return true;
}

void SwOneExampleFrame::PopupHdl(std::u16string_view rId)
{
    std::u16string_view sZoomValue;
    if (o3tl::starts_with(rId, u"zoom", &sZoomValue))
    {
        sal_Int16 nZoom = o3tl::toInt32(sZoomValue);
        uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom;
        aZoom <<= nZoom;
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
        aZoom <<= sal_Int16(view::DocumentZoomType::BY_VALUE);
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
    }
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
