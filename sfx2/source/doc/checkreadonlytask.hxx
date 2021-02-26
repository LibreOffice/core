/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/threadpool.hxx>
#include <memory>
#include <mutex>
#include <condition_variable>

class SfxMedium;

class CheckReadOnlyTask : public comphelper::ThreadTask
{
public:
    CheckReadOnlyTask(SfxMedium* pMed, const std::shared_ptr<std::recursive_mutex>& pMutex,
                      const std::shared_ptr<std::condition_variable_any>& pCond,
                      const std::shared_ptr<bool>& pIsDestructed,
                      const std::shared_ptr<comphelper::ThreadTaskTag>& pTag);

    virtual void doWork() override;

    SfxMedium* _pMed;
    std::shared_ptr<std::recursive_mutex> _pMutex;
    std::shared_ptr<std::condition_variable_any> _pMyCond;
    std::shared_ptr<bool> _pIsDestructed;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
