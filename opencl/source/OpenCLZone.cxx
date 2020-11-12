/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <opencl/openclwrapper.hxx>
#include <opencl/OpenCLZone.hxx>
#include <opencl_device.hxx>

#include <memory>

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

/**
 * Called from a signal handler if we get
 * a crash or hang in some CL code.
 */
void OpenCLZone::hardDisable()
{
    // protect ourselves from double calling etc.
    static bool bDisabled = false;
    if (bDisabled)
        return;

    bDisabled = true;

    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseOpenCL::set(false, xChanges);
    xChanges->commit();

    // Force synchronous config write
    auto xConfProvider
        = css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext());
    css::uno::Reference<css::util::XFlushable> xFlushable(xConfProvider, css::uno::UNO_QUERY_THROW);
    xFlushable->flush();

    releaseOpenCLEnv(&openclwrapper::gpuEnv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
