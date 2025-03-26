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

#pragma once

#include <drawinglayer/primitive2d/BufferedDecompositionGroupPrimitive2D.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <osl/thread.hxx>
#include <unordered_set>

namespace drawinglayer::primitive2d
{
class BufferedDecompositionFlusher : public osl::Thread
{
public:
    static void update(const BufferedDecompositionPrimitive2D*);
    static void update(const BufferedDecompositionGroupPrimitive2D*);

    BufferedDecompositionFlusher();

    static DRAWINGLAYERCORE_DLLPUBLIC void shutdown();

    /// Only called by FlusherDeinit
    void onTeardown();

private:
    virtual void SAL_CALL run() override;
    void updateImpl(const BufferedDecompositionPrimitive2D*);
    void updateImpl(const BufferedDecompositionGroupPrimitive2D*);

    std::unordered_set<rtl::Reference<BufferedDecompositionPrimitive2D>> maRegistered1;
    std::unordered_set<rtl::Reference<BufferedDecompositionGroupPrimitive2D>> maRegistered2;
    std::mutex maMutex;
    bool mbShutdown{ false };
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
