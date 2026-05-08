/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/BufferedDecompositionGroupPrimitive2D.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <osl/thread.hxx>
#include <unotools/weakref.hxx>
#include <chrono>
#include <condition_variable>
#include <unordered_map>
#include <vector>

namespace drawinglayer::primitive2d
{
class BufferedDecompositionFlusher final : public osl::Thread
{
public:
    static void update(const BufferedDecompositionPrimitive2D*);
    static void update(const BufferedDecompositionGroupPrimitive2D*);
    static void remove(const BufferedDecompositionPrimitive2D*);
    static void remove(const BufferedDecompositionGroupPrimitive2D*);

    BufferedDecompositionFlusher();

    /// Flush all pending decompositions, join the background thread, and destroy
    /// the singleton. The next update() call will spawn a fresh instance.
    /// Safe to call when no instance exists.
    static DRAWINGLAYERCORE_DLLPUBLIC void shutdown();

    void onTeardown();

private:
    struct FlushBatch
    {
        std::vector<rtl::Reference<BufferedDecompositionPrimitive2D>> mRemoved1;
        std::vector<rtl::Reference<BufferedDecompositionGroupPrimitive2D>> mRemoved2;
    };

    virtual void SAL_CALL run() override;
    void updateImpl(const BufferedDecompositionPrimitive2D*);
    void updateImpl(const BufferedDecompositionGroupPrimitive2D*);
    void removeImpl(const BufferedDecompositionPrimitive2D*);
    void removeImpl(const BufferedDecompositionGroupPrimitive2D*);
    /// Drain entries idle for at least @p aMinIdleAge into a FlushBatch.
    FlushBatch collectRemoved(std::chrono::steady_clock::duration aMinIdleAge);
    /// Drop the buffered decompositions of the collected entries (under SolarMutex).
    static void flushRemoved(FlushBatch& rBatch);
    void flushPending();

    // Explicitly not using rtl::Reference because they are removed from here when they destruct.
    std::unordered_map<const BufferedDecompositionPrimitive2D*,
                       unotools::WeakReference<BufferedDecompositionPrimitive2D>>
        maRegistered1;
    std::unordered_map<const BufferedDecompositionGroupPrimitive2D*,
                       unotools::WeakReference<BufferedDecompositionGroupPrimitive2D>>
        maRegistered2;
    std::mutex maMutex;
    bool mbShutdown{ false };
    std::condition_variable maDelayOrTerminate;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
