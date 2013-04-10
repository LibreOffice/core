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

#include <unotools/moduleoptions.hxx>
#include "sddll.hxx"
#include "diactrl.hxx"
#include "tbx_ww.hxx"
#include "TextObjectBar.hxx"
#include "BezierObjectBar.hxx"
#include "GraphicObjectBar.hxx"
#include "MediaObjectBar.hxx"
#include "ImpressViewShellBase.hxx"
#include "PresentationViewShellBase.hxx"
#include "OutlineViewShell.hxx"
#include "PresentationViewShell.hxx"
#include "OutlineViewShellBase.hxx"
#include "SlideSorterViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "GraphicViewShellBase.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "FactoryIds.hxx"
#include "sdmod.hxx"
#include "app.hrc"

namespace sd { namespace ui { namespace table {
    extern void RegisterInterfaces( SfxModule* pMod );
} } }


/*************************************************************************
|*
|* Register all Factorys
|*
\************************************************************************/


void SdDLL::RegisterFactorys()
{
    if (SvtModuleOptions().IsImpress())
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
    if (SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicViewShellBase::RegisterFactory (::sd::DRAW_FACTORY_ID);
    }
}



/*************************************************************************
|*
|* Register all Interfaces
|*
\************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
