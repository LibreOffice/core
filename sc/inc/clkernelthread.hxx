/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CLKERNELTHREAD_HXX
#define INCLUDED_SC_INC_CLKERNELTHREAD_HXX

#include <queue>

#include <osl/conditn.hxx>
#include <salhelper/thread.hxx>

#include <boost/noncopyable.hpp>

#include "scdllapi.h"
#include "formulacell.hxx"

namespace sc {

struct CLBuildKernelWorkItem
{
    enum { COMPILE, FINISH } meWhatToDo;
    ScFormulaCellGroupRef mxGroup;
};

class SC_DLLPUBLIC CLBuildKernelThread : public salhelper::Thread, boost::noncopyable
{
public:
    CLBuildKernelThread();
    virtual ~CLBuildKernelThread();

    void finish();

    void push(CLBuildKernelWorkItem item);

    osl::Condition maCompilationDoneCondition;

protected:
    virtual void execute() SAL_OVERRIDE;

private:
    osl::Mutex maQueueMutex;
    osl::Condition maQueueCondition;
    std::queue<CLBuildKernelWorkItem> maQueue;
    void produce();
    void consume();
};

}

#endif // INCLUDED_SC_INC_CLKERNELTHREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
