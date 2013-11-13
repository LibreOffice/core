/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <queue>

#include <osl/conditn.hxx>
#include <salhelper/thread.hxx>

#include "formulacell.hxx"

namespace sc {

struct CLBuildKernelWorkItem
{
    enum { COMPILE, FINISH } meWhatToDo;
    ScFormulaCellGroupRef mxGroup;
};

class CLBuildKernelThread : public salhelper::Thread
{
public:
    CLBuildKernelThread();
    virtual ~CLBuildKernelThread();

    void finish();

    void push(CLBuildKernelWorkItem item);

protected:
    virtual void execute();

private:
    osl::Mutex maMutex;
    osl::Condition maCondition;
    std::queue<CLBuildKernelWorkItem> maQueue;
    void produce();
    void consume();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
