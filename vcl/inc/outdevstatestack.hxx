/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OUTDEVSTATESTACK_HXX
#define INCLUDED_VCL_OUTDEVSTATESTACK_HXX

#include <vcl/outdevstate.hxx>
#include <memory>
#include <deque>

class OutDevStateStack
{
    std::deque<std::unique_ptr<OutDevState>> maData;

public:
    bool empty() const;
    size_t size() const;
    void push_back( OutDevState* p );
    void pop_back();
    OutDevState& back();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
