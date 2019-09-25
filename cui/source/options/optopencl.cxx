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

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/openclwrapper.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/restartdialog.hxx>

#include "optopencl.hxx"

SvxOpenCLTabPage::SvxOpenCLTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optopenclpage.ui", "OptOpenCLPage", &rSet)
    , maConfig(OpenCLConfig::get())
    , mxUseOpenCL(m_xBuilder->weld_check_button("useopencl"))
    , mxOclUsed(m_xBuilder->weld_label("openclused"))
    , mxOclNotUsed(m_xBuilder->weld_label("openclnotused"))
{
    mxUseOpenCL->set_active(maConfig.mbUseOpenCL);
    mxUseOpenCL->set_sensitive(!officecfg::Office::Common::Misc::UseOpenCL::isReadOnly());

    bool bCLUsed = openclwrapper::GPUEnv::isOpenCLEnabled();
    mxOclUsed->set_visible(bCLUsed);
    mxOclNotUsed->set_visible(!bCLUsed);
}

SvxOpenCLTabPage::~SvxOpenCLTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxOpenCLTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxOpenCLTabPage>(pPage, pController, *rAttrSet);
}

bool SvxOpenCLTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if (mxUseOpenCL->get_state_changed_from_saved())
        maConfig.mbUseOpenCL = mxUseOpenCL->get_active();

    if (maConfig != OpenCLConfig::get())
    {
        maConfig.set();
        bModified = true;
    }

    if (bModified)
    {
        batch->commit();
        SolarMutexGuard aGuard;
        if (svtools::executeRestartDialog(comphelper::getProcessComponentContext(), nullptr,
                                      svtools::RESTART_REASON_OPENCL))
            GetDialogController()->response(RET_OK);
    }

    return bModified;
}

void SvxOpenCLTabPage::Reset( const SfxItemSet* )
{
    maConfig = OpenCLConfig::get();

    mxUseOpenCL->set_active(maConfig.mbUseOpenCL);
    mxUseOpenCL->save_state();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
