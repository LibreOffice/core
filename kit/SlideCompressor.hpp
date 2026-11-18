/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/ThreadPool.hpp>

#include <functional>
#include <memory>
#include <vector>

/// Do slide compression work in a thread pool
class SlideCompressor {
    ThreadPool &_pool;

    struct SlideItem {
        std::vector<char> _output;
    };
    std::vector<std::shared_ptr<SlideItem>> _items;
public:
    SlideCompressor(ThreadPool &pool) :
        _pool(pool)
    {
    }

    /// workFn generates is passed a buffer to generate its output into
    void pushWork(std::function<void(std::vector<char>&)> workFn)
    {
        auto item = std::make_shared<SlideItem>();
        _items.push_back(item);
        _pool.pushWork([item = std::move(item), workFn = std::move(workFn)]
                       { workFn(item->_output); });
    }

    /// sendFunc is called in the same order after all items are compressed
    void compress(std::function<void(std::vector<char>&)> sendFunc)
    {
        LOG_TRC("Compressing " << _items.size() << " layers with " << _pool.getThreadCount() << " threads");

        if (_items.size() > 0)
            _pool.run();

        for (auto &it : _items)
        {
            if (it->_output.size() > 0)
                sendFunc(it->_output);
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
