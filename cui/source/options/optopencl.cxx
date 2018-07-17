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

#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <svl/zforlist.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/openclwrapper.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/simptabl.hxx>
#include "optHeaderTabListbox.hxx"

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <strings.hrc>
#include <dialmgr.hxx>
#include "optopencl.hxx"
#include <svtools/treelistentry.hxx>

SvxOpenCLTabPage::SvxOpenCLTabPage(vcl::Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, "OptOpenCLPage", "cui/ui/optopenclpage.ui", &rSet),
    maConfig(OpenCLConfig::get())
{
    get(mpUseSwInterpreter, "useswinterpreter");
    get(mpUseOpenCL, "useopencl");
    get(mpOclUsed,"openclused");
    get(mpOclNotUsed,"openclnotused");

    mpUseSwInterpreter->Check(officecfg::Office::Common::Misc::UseSwInterpreter::get());
    mpUseSwInterpreter->Enable(!officecfg::Office::Common::Misc::UseSwInterpreter::isReadOnly());

    mpUseOpenCL->Check(maConfig.mbUseOpenCL);
    mpUseOpenCL->Enable(!officecfg::Office::Common::Misc::UseOpenCL::isReadOnly());

    bool bCLUsed = openclwrapper::GPUEnv::isOpenCLEnabled();
    mpOclUsed->Show(bCLUsed);
    mpOclNotUsed->Show(!bCLUsed);
}

SvxOpenCLTabPage::~SvxOpenCLTabPage()
{
    disposeOnce();
}

void SvxOpenCLTabPage::dispose()
{
    mpUseSwInterpreter.clear();
    mpUseOpenCL.clear();
    mpOclUsed.clear();
    mpOclNotUsed.clear();

    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxOpenCLTabPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxOpenCLTabPage>::Create(pParent.pParent, *rAttrSet);
}

bool SvxOpenCLTabPage::FillItemSet( SfxItemSet* )
{
 bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if (mpUseSwInterpreter->IsValueChangedFromSaved())
    {
        officecfg::Office::Common::Misc::UseSwInterpreter::set(mpUseSwInterpreter->IsChecked(), batch);
        bModified = true;
    }

    if (mpUseOpenCL->IsValueChangedFromSaved())
        maConfig.mbUseOpenCL = mpUseOpenCL->IsChecked();

    if (maConfig != OpenCLConfig::get())
    {
        maConfig.set();
        bModified = true;
    }

    if (bModified)
    {
        std::unique_ptr<weld::MessageDialog> xWarnBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Question, VclButtonsType::NONE,
                                                      CuiResId(RID_SVXSTR_OPENCL_RESTART)));
        xWarnBox->add_button("Restart Now",RET_YES);
        xWarnBox->add_button("Restart Later",RET_NO);
        sal_uInt16 nRet = xWarnBox->run();
        batch->commit();
        if (nRet == RET_YES)
        {
            css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
        }

    }

    return bModified;
}

void SvxOpenCLTabPage::Reset( const SfxItemSet* )
{
    maConfig = OpenCLConfig::get();

    mpUseSwInterpreter->Check(officecfg::Office::Common::Misc::UseSwInterpreter::get());
    mpUseSwInterpreter->SaveValue();

    mpUseOpenCL->Check(maConfig.mbUseOpenCL);
    mpUseOpenCL->SaveValue();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
