/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <outdevstatestack.hxx>

bool OutDevStateStack::empty() const
{
    return maData.empty();
}

size_t OutDevStateStack::size() const
{
    return maData.size();
}

void OutDevStateStack::push_back( OutDevState* p )
{
    maData.push_back(std::unique_ptr<OutDevState>(p));
}

void OutDevStateStack::pop_back()
{
    maData.pop_back();
}

OutDevState& OutDevStateStack::back()
{
    return *maData.back();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
