/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clkernelthread.hxx"
#include "formulagroup.hxx"

using namespace std;

namespace sc {

CLBuildKernelThread::CLBuildKernelThread() : salhelper::Thread("opencl-build-kernel-thread") {}

CLBuildKernelThread::~CLBuildKernelThread() {}

void CLBuildKernelThread::execute()
{
    sc::FormulaGroupInterpreter::compileKernels();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
