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

#include <config_features.h>

#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/fmobjfac.hxx>
#include <svx/objfac3d.hxx>
#include <vcl/svapp.hxx>

#include <registerinterfaces.hxx>
#include <sddll.hxx>
#include <app.hrc>
#include <AnimationChildWindow.hxx>
#include <BezierObjectBar.hxx>
#include <diactrl.hxx>
#include <DrawDocShell.hxx>
#include <FactoryIds.hxx>
#include <gluectrl.hxx>
#include <GraphicDocShell.hxx>
#include <GraphicObjectBar.hxx>
#include <GraphicViewShell.hxx>
#include <GraphicViewShellBase.hxx>
#include <ImpressViewShellBase.hxx>
#include <PresentationViewShell.hxx>
#include <PresentationViewShellBase.hxx>
#include <MediaObjectBar.hxx>
#include <NavigatorChildWindow.hxx>
#include <OutlineViewShell.hxx>
#include <OutlineViewShellBase.hxx>
#include <PaneChildWindows.hxx>
#include <SpellDialogChildWindow.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorterViewShellBase.hxx>
#include <SdShapeTypes.hxx>
#include <TextObjectBar.hxx>
#include <tmplctrl.hxx>

#include <svx/svxids.hrc>
#include <svx/bmpmask.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/f3dchild.hxx>
#include <svx/fillctrl.hxx>
#include <svx/fontwork.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include <svx/ParaLineSpacingPopup.hxx>
#include <svx/TextCharacterSpacingPopup.hxx>
#include <svx/TextUnderlinePopup.hxx>
#include <svx/grafctrl.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/imapdlg.hxx>
#include <svx/layctrl.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/linectrl.hxx>
#include <svx/modctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/srchdlg.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/xmlsecctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <svx/tbxctl.hxx>
#include <sfx2/emojipopup.hxx>
#include <sfx2/charmappopup.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sdabstdlg.hxx>
#include <sdfilter.hxx>
#include <sdmod.hxx>

using namespace ::com::sun::star;

// Register all Factories
void SdDLL::RegisterFactorys()
{
    if (utl::ConfigManager::IsFuzzing() || SvtModuleOptions().IsImpress())
    {
        ::sd::ImpressViewShellBase::RegisterFactory (
            ::sd::IMPRESS_FACTORY_ID);
        ::sd::SlideSorterViewShellBase::RegisterFactory (
            ::sd::SLIDE_SORTER_FACTORY_ID);
        ::sd::OutlineViewShellBase::RegisterFactory (
            ::sd::OUTLINE_FACTORY_ID);
        ::sd::PresentationViewShellBase::RegisterFactory (
            ::sd::PRESENTATION_FACTORY_ID);
    }
    if (!utl::ConfigManager::IsFuzzing() && SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicViewShellBase::RegisterFactory (::sd::DRAW_FACTORY_ID);
    }
}

// Register all Interfaces

void SdDLL::RegisterInterfaces(SdModule* pMod)
{
    // Module
    SdModule::RegisterInterface(pMod);

    // View shell base.
    ::sd::ViewShellBase::RegisterInterface(pMod);

    // DocShells
    ::sd::DrawDocShell::RegisterInterface(pMod);
    ::sd::GraphicDocShell::RegisterInterface(pMod);

    // Impress ViewShells
    ::sd::DrawViewShell::RegisterInterface(pMod);
    ::sd::OutlineViewShell::RegisterInterface(pMod);
    ::sd::PresentationViewShell::RegisterInterface(pMod);

    // Draw ViewShell
    ::sd::GraphicViewShell::RegisterInterface(pMod);

    // Impress ObjectShells
    ::sd::BezierObjectBar::RegisterInterface(pMod);
    ::sd::TextObjectBar::RegisterInterface(pMod);
    ::sd::GraphicObjectBar::RegisterInterface(pMod);

    // Media ObjectShell
    ::sd::MediaObjectBar::RegisterInterface(pMod);

    // Table ObjectShell
    ::sd::ui::table::RegisterInterfaces(pMod);

    // View shells for the side panes.
    ::sd::slidesorter::SlideSorterViewShell::RegisterInterface (pMod);
}

// Register all Controllers

void SdDLL::RegisterControllers(SdModule* pMod)
{
    SdTbxCtlDiaPages::RegisterControl( SID_PAGES_PER_ROW, pMod );
    SdTbxCtlGlueEscDir::RegisterControl( SID_GLUE_ESCDIR, pMod );

    ::sd::AnimationChildWindow::RegisterChildWindow(false, pMod);
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( static_cast<sal_uInt16>(::sd::DrawViewShell::GetInterfaceId()), pMod );
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( static_cast<sal_uInt16>(::sd::GraphicViewShell::GetInterfaceId()), pMod );

    Svx3DChildWindow::RegisterChildWindow(false, pMod);
    SvxFontWorkChildWindow::RegisterChildWindow(false, pMod);
    SvxColorChildWindow::RegisterChildWindow(false, pMod, SfxChildWindowFlags::TASK);
    SvxSearchDialogWrapper::RegisterChildWindow(false, pMod);
    SvxBmpMaskChildWindow::RegisterChildWindow(false, pMod);
    SvxIMapDlgChildWindow::RegisterChildWindow(false, pMod);
    SvxHlinkDlgWrapper::RegisterChildWindow(false, pMod);
    ::sd::SpellDialogChildWindow::RegisterChildWindow(false, pMod);
#if HAVE_FEATURE_AVMEDIA
    ::avmedia::MediaPlayer::RegisterChildWindow(false, pMod);
#endif
    ::sd::LeftPaneImpressChildWindow::RegisterChildWindow(false, pMod);
    ::sd::LeftPaneDrawChildWindow::RegisterChildWindow(false, pMod);
    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindow(false, pMod);

    SvxFillToolBoxControl::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl::RegisterControl(0, pMod);

    SvxStyleToolBoxControl::RegisterControl(0, pMod);

    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );
    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_TOP_TO_BOTTOM, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXTDIRECTION_LEFT_TO_RIGHT, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_CAPTION_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_FONTWORK_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_DRAW_TEXT_VERTICAL, pMod);
    SvxVertTextTbxCtrl::RegisterControl(SID_TEXT_FITTOSIZE_VERTICAL, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_LEFT_TO_RIGHT, pMod);
    SvxCTLTextTbxCtrl::RegisterControl(SID_ATTR_PARA_RIGHT_TO_LEFT, pMod);

    // register StatusBarControls
    SvxZoomPageStatusBarControl::RegisterControl( SID_ZOOM_ENTIRE_PAGE, pMod );
    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pMod );
    SvxPosSizeStatusBarControl::RegisterControl( SID_ATTR_SIZE, pMod );
    SvxModifyControl::RegisterControl( SID_DOC_MODIFIED, pMod );
    SvxZoomSliderControl::RegisterControl( SID_ATTR_ZOOMSLIDER, pMod );

    // #UndoRedo#
    SvxUndoRedoControl::RegisterControl( SID_UNDO , pMod );
    SvxUndoRedoControl::RegisterControl( SID_REDO , pMod );

    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH, pMod );

    SvxClipBoardControl::RegisterControl( SID_PASTE, pMod );
    SvxClipBoardControl::RegisterControl( SID_PASTE_UNFORMATTED, pMod );

    svx::ParaLineSpacingPopup::RegisterControl(SID_ATTR_PARA_LINESPACE, pMod);
    svx::TextCharacterSpacingPopup::RegisterControl(SID_ATTR_CHAR_KERNING, pMod);
    svx::TextUnderlinePopup::RegisterControl(SID_ATTR_CHAR_UNDERLINE, pMod);

#if HAVE_FEATURE_AVMEDIA
    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );
#endif
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pMod );
    SdTemplateControl::RegisterControl( SID_STATUS_LAYOUT, pMod );
    SvxTableToolBoxControl::RegisterControl(SID_INSERT_TABLE, pMod );
    SvxTbxCtlDraw::RegisterControl(SID_INSERT_DRAW, pMod );

    EmojiPopup::RegisterControl(SID_EMOJI_CONTROL, pMod );
    CharmapPopup::RegisterControl(SID_CHARMAP_CONTROL, pMod );
}

void SdDLL::Init()
{
    if ( SfxApplication::GetModule(SfxToolsModule::Draw) )    // Module already active
        return;

    SfxObjectFactory* pDrawFact = nullptr;
    SfxObjectFactory* pImpressFact = nullptr;

    if (utl::ConfigManager::IsFuzzing() || SvtModuleOptions().IsImpress())
        pImpressFact = &::sd::DrawDocShell::Factory();

    if (!utl::ConfigManager::IsFuzzing() && SvtModuleOptions().IsDraw())
        pDrawFact = &::sd::GraphicDocShell::Factory();

    auto pUniqueModule = std::make_unique<SdModule>(pImpressFact, pDrawFact);
    SdModule* pModule = pUniqueModule.get();
    SfxApplication::SetModule(SfxToolsModule::Draw, std::move(pUniqueModule));

    if (!utl::ConfigManager::IsFuzzing() && SvtModuleOptions().IsImpress())
    {
        // Register the Impress shape types in order to make the shapes accessible.
        ::accessibility::RegisterImpressShapeTypes ();
        ::sd::DrawDocShell::Factory().SetDocumentServiceName( "com.sun.star.presentation.PresentationDocument" );
    }

    if (!utl::ConfigManager::IsFuzzing() && SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicDocShell::Factory().SetDocumentServiceName( "com.sun.star.drawing.DrawingDocument" );
    }

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces(pModule);

    // register your controllers here
    RegisterControllers(pModule);

    // register 3D-object-factory
    E3dObjFactory();

    // register css::form::component::Form-Object-Factory
    FmFormObjFactory();

    // register your exotic remote controls here
#ifdef ENABLE_SDREMOTE
    if (!utl::ConfigManager::IsFuzzing() && !Application::IsHeadlessModeEnabled())
        RegisterRemotes();
#endif
}

#ifndef DISABLE_DYNLOADING

extern "C" SAL_DLLPUBLIC_EXPORT
void lok_preload_hook()
{
    SdFilter::Preload();
    SdAbstractDialogFactory::Create();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
