/*************************************************************************
 *
 *  $RCSfile: sddll1.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:14:44 $
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

#include <svtools/moduleoptions.hxx>
#include "sddll.hxx"
#include "diactrl.hxx"
#include "tbx_ww.hxx"
#ifndef SD_TEXT_OBJECT_BAR_HXX
#include "TextObjectBar.hxx"
#endif
#ifndef SD_BEZIER_OBJECT_BAR_HXX
#include "BezierObjectBar.hxx"
#endif
#ifndef SD_GLUE_POINTS_OBJECT_BAR_HXX
#include "GluePointsObjectBar.hxx"
#endif
#ifndef SD_GRAPHIC_OBJECT_BAR_HXX
#include "GraphicObjectBar.hxx"
#endif
#ifndef SD_MEDIA_OBJECT_BAR_HXX
#include "MediaObjectBar.hxx"
#endif
#ifndef SD_DRAW_OBJECT_BAR_HXX
#include "DrawObjectBar.hxx"
#endif
#ifndef SD_IMPRESS_OBJECT_BAR_HXX
#include "ImpressObjectBar.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_BASE_HXX
#include "PresentationViewShellBase.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_BASE_HXX
#include "OutlineViewShellBase.hxx"
#endif
#ifndef SD_SLIDE_SORTER_VIEW_SHELL_BASE_HXX
#include "SlideSorterViewShellBase.hxx"
#endif
#ifndef SD_PREVIEW_VIEW_SHELL_HXX
#include "PreviewViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_BASE_HXX
#include "GraphicViewShellBase.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#ifndef SD_SLIDEBROWSER_SLIDE_SORTER_VIEW_SHELL_HXX
#include "SlideSorterViewShell.hxx"
#endif
#ifndef SD_SLIDEBROWSER_TASK_PANE_VIEW_SHELL_HXX
#include "TaskPaneViewShell.hxx"
#endif
#ifndef SD_FACTORY_IDS_HXX
#include "FactoryIds.hxx"
#endif
#include "sdmod.hxx"
#include "app.hrc"



/*************************************************************************
|*
|* Register all Factorys
|*
\************************************************************************/


void SdDLL::RegisterFactorys()
{
    if (SvtModuleOptions().IsImpress())
    {
        ::sd::ViewShellBase::RegisterFactory (
            ::sd::IMPRESS_FACTORY_ID);
        ::sd::SlideSorterViewShellBase::RegisterFactory (
            ::sd::SLIDE_SORTER_FACTORY_ID);
        ::sd::OutlineViewShellBase::RegisterFactory (
            ::sd::OUTLINE_FACTORY_ID);
        ::sd::PresentationViewShellBase::RegisterFactory (
            ::sd::PRESENTATION_FACTORY_ID);
    }
    if (SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicViewShellBase::RegisterFactory (::sd::DRAW_FACTORY_ID);
    }

    /* af
    if (SvtModuleOptions().IsImpress())
    {
        // Impress
        ::sd::DrawViewShell::RegisterFactory(1);
        ::sd::SlideViewShell::RegisterFactory(2);
        ::sd::OutlineViewShell::RegisterFactory(3);
        ::sd::PresentationViewShell::RegisterFactory(4);
        ::sd::PreviewViewShell::RegisterFactory(5);
    }

    if (SvtModuleOptions().IsDraw()) {
        // Draw
        ::sd::GraphicViewShell::RegisterFactory(1);

        // #93468# Need preview view also for draw documents
        ::sd::PreviewViewShell::RegisterFactory(5);
    }
    */
}



/*************************************************************************
|*
|* Register all Interfaces
|*
\************************************************************************/


void SdDLL::RegisterInterfaces()
{
    // Modul
    SfxModule* pMod = SD_MOD();
    SdModule::RegisterInterface(pMod);

    // View shell base.
    ::sd::ViewShellBase::RegisterInterface(pMod);

    // DocShells
    ::sd::DrawDocShell::RegisterInterface(pMod);
    ::sd::GraphicDocShell::RegisterInterface(pMod);

    // Impress ViewShells
    ::sd::DrawViewShell::RegisterInterface(pMod);
    ::sd::SlideViewShell::RegisterInterface(pMod);
    ::sd::OutlineViewShell::RegisterInterface(pMod);
    ::sd::PresentationViewShell::RegisterInterface(pMod);
    ::sd::PreviewViewShell::RegisterInterface(pMod);

    // Draw ViewShell
    ::sd::GraphicViewShell::RegisterInterface(pMod);

    // Impress ObjectShells
    ::sd::ImpressObjectBar::RegisterInterface(pMod);
    ::sd::BezierObjectBar::RegisterInterface(pMod);
    ::sd::GluePointsObjectBar::RegisterInterface(pMod);
    ::sd::TextObjectBar::RegisterInterface(pMod);
    ::sd::GraphicObjectBar::RegisterInterface(pMod);

    // Draw ObjectShell
    ::sd::DrawObjectBar::RegisterInterface(pMod);

    // Media ObjectShell
    ::sd::MediaObjectBar::RegisterInterface(pMod);

    // View shells for the side panes.
    ::sd::slidesorter::SlideSorterViewShell::RegisterInterface (pMod);
    ::sd::toolpanel::TaskPaneViewShell::RegisterInterface(pMod);
    // Tell the task pane view shell to register the interfaces of its
    // controls.
    ::sd::toolpanel::TaskPaneViewShell::RegisterControls();
}
