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

#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediatoolbox.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdobj.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>
#include <vcl/svapp.hxx>

#include "registerinterfaces.hxx"
#include "sddll.hxx"
#include "app.hrc"
#include "AnimationChildWindow.hxx"
#include "BezierObjectBar.hxx"
#include "diactrl.hxx"
#include "DrawDocShell.hxx"
#include "FactoryIds.hxx"
#include "gluectrl.hxx"
#include "GraphicDocShell.hxx"
#include "GraphicObjectBar.hxx"
#include "GraphicViewShell.hxx"
#include "GraphicViewShellBase.hxx"
#include "ImpressViewShellBase.hxx"
#include "PresentationViewShell.hxx"
#include "PresentationViewShellBase.hxx"
#include "MediaObjectBar.hxx"
#include "NavigatorChildWindow.hxx"
#include "OutlineViewShell.hxx"
#include "OutlineViewShellBase.hxx"
#include "PaneChildWindows.hxx"
#include "sdresid.hxx"
#include "sdobjfac.hxx"
#include "cfgids.hxx"
#include "SpellDialogChildWindow.hxx"
#include "SlideSorterViewShell.hxx"
#include "SlideSorterViewShellBase.hxx"
#include "strmname.h"
#include "SdShapeTypes.hxx"
#include "TextObjectBar.hxx"
#include "tmplctrl.hxx"

#include <svx/svxids.hrc>
#include <svx/bmpmask.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/f3dchild.hxx>
#include <svx/fillctrl.hxx>
#include <svx/fntctl.hxx>
#include <svx/fntszctl.hxx>
#include <svx/fontwork.hxx>
#include <svx/formatpaintbrushctrl.hxx>
#include <svx/ParaLineSpacingPopup.hxx>
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
#include <svx/SvxShapeTypes.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svx/xmlsecctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <svx/tbxctl.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

// Register all Factories
void SdDLL::RegisterFactorys()
{
    if (utl::ConfigManager::IsAvoidConfig() || SvtModuleOptions().IsImpress())
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
    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicViewShellBase::RegisterFactory (::sd::DRAW_FACTORY_ID);
    }
}

// Register all Interfaces

void SdDLL::RegisterInterfaces()
{
    // Module
    SfxModule* pMod = SD_MOD();
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

void SdDLL::RegisterControllers()
{
    SfxModule* pMod = SD_MOD();

    SdTbxCtlDiaPages::RegisterControl( SID_PAGES_PER_ROW, pMod );
    SdTbxCtlGlueEscDir::RegisterControl( SID_GLUE_ESCDIR, pMod );

    ::sd::AnimationChildWindow::RegisterChildWindow(false, pMod);
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::DrawViewShell::GetInterfaceId(), pMod );
    ::sd::NavigatorChildWindow::RegisterChildWindowContext( (sal_uInt16) ::sd::GraphicViewShell::GetInterfaceId(), pMod );

    Svx3DChildWindow::RegisterChildWindow(false, pMod);
    SvxFontWorkChildWindow::RegisterChildWindow(false, pMod);
    SvxColorChildWindow::RegisterChildWindow(false, pMod, SfxChildWindowFlags::TASK);
    SvxSearchDialogWrapper::RegisterChildWindow(false, pMod);
    SvxBmpMaskChildWindow::RegisterChildWindow(false, pMod);
    SvxIMapDlgChildWindow::RegisterChildWindow(false, pMod);
    SvxHlinkDlgWrapper::RegisterChildWindow(false, pMod);
    ::sd::SpellDialogChildWindow::RegisterChildWindow(false, pMod);
    ::avmedia::MediaPlayer::RegisterChildWindow(false, pMod);
    ::sd::LeftPaneImpressChildWindow::RegisterChildWindow(false, pMod);
    ::sd::LeftPaneDrawChildWindow::RegisterChildWindow(false, pMod);
    ::sfx2::sidebar::SidebarChildWindow::RegisterChildWindow(false, pMod);

    SvxFillToolBoxControl::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl::RegisterControl(0, pMod);
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_LINE_COLOR, pMod);
    SvxColorToolBoxControl::RegisterControl(SID_ATTR_FILL_COLOR, pMod);

    SvxLineEndToolBoxControl::RegisterControl( SID_ATTR_LINEEND_STYLE, pMod );

    SvxStyleToolBoxControl::RegisterControl(0, pMod);
    SvxFontNameToolBoxControl::RegisterControl(0, pMod);
    SvxColorToolBoxControl::RegisterControl( SID_ATTR_CHAR_COLOR, pMod );
    SvxColorToolBoxControl::RegisterControl( SID_ATTR_CHAR_BACK_COLOR, pMod );

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

    // MenuControls for PopupMenu
    SvxFontMenuControl::RegisterControl( SID_ATTR_CHAR_FONT, pMod );
    SvxFontSizeMenuControl::RegisterControl( SID_ATTR_CHAR_FONTHEIGHT, pMod );

    SfxMenuControl::RegisterControl( SID_SET_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_DELETE_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_BEZIER_CLOSE, pMod );

    // #UndoRedo#
    SvxUndoRedoControl::RegisterControl( SID_UNDO , pMod );
    SvxUndoRedoControl::RegisterControl( SID_REDO , pMod );

    svx::FormatPaintBrushToolBoxControl::RegisterControl(SID_FORMATPAINTBRUSH, pMod );

    SvxClipBoardControl::RegisterControl( SID_PASTE, pMod );
    SvxClipBoardControl::RegisterControl( SID_PASTE_UNFORMATTED, pMod );

    SvxColorToolBoxControl::RegisterControl( SID_EXTRUSION_3D_COLOR, pMod );
    svx::ParaLineSpacingPopup::RegisterControl(SID_ATTR_PARA_LINESPACE, pMod);

    ::avmedia::MediaToolBoxControl::RegisterControl( SID_AVMEDIA_TOOLBOX, pMod );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pMod );
    SdTemplateControl::RegisterControl( SID_STATUS_LAYOUT, pMod );
    SvxTableToolBoxControl::RegisterControl(SID_INSERT_TABLE, pMod );
    SvxFrameLineStyleToolBoxControl::RegisterControl(SID_FRAME_LINESTYLE, pMod );
    SvxColorToolBoxControl::RegisterControl(SID_FRAME_LINECOLOR, pMod );
    SvxFrameToolBoxControl::RegisterControl(SID_ATTR_BORDER, pMod );
    SvxTbxCtlDraw::RegisterControl(SID_INSERT_DRAW, pMod );
}

void SdDLL::Init()
{
    if ( SD_MOD() )
        return;

    SfxObjectFactory* pDrawFact = nullptr;
    SfxObjectFactory* pImpressFact = nullptr;

    if (utl::ConfigManager::IsAvoidConfig() || SvtModuleOptions().IsImpress())
        pImpressFact = &::sd::DrawDocShell::Factory();

    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsDraw())
        pDrawFact = &::sd::GraphicDocShell::Factory();

    // the SdModule must be created
     SdModule** ppShlPtr = reinterpret_cast<SdModule**>(GetAppData(SHL_DRAW));

     // #i46427#
     // The SfxModule::SfxModule stops when the first given factory
     // is 0, so we must not give a 0 as first factory
     if( pImpressFact )
     {
        (*ppShlPtr) = new SdModule( pImpressFact, pDrawFact );
     }
     else
     {
        (*ppShlPtr) = new SdModule( pDrawFact, pImpressFact );
     }

    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsImpress())
    {
        // Register the Impress shape types in order to make the shapes accessible.
        ::accessibility::RegisterImpressShapeTypes ();
        ::sd::DrawDocShell::Factory().SetDocumentServiceName( "com.sun.star.presentation.PresentationDocument" );
    }

    if (!utl::ConfigManager::IsAvoidConfig() && SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicDocShell::Factory().SetDocumentServiceName( "com.sun.star.drawing.DrawingDocument" );
    }

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControllers();

    // register SvDraw-Fields
    SdrRegisterFieldClasses();

    // register 3D-Objekt-Factory
    E3dObjFactory();

    // register css::form::component::Form-Object-Factory
    FmFormObjFactory();

    // register Object-Factory
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // register your exotic remote controls here
#ifdef ENABLE_SDREMOTE
    if (!utl::ConfigManager::IsAvoidConfig() && !Application::IsHeadlessModeEnabled())
        RegisterRemotes();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
