/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include <tokenarray.hxx>
#include <memory>
#include <map>
class ScTokenArray;

namespace sc
{
class SharedFormulaGroupEntry
{
private:
    std::unique_ptr<ScTokenArray> mpArray;
    ScAddress maOrigin;

public:
    SharedFormulaGroupEntry(std::unique_ptr<ScTokenArray> pArray, const ScAddress& rOrigin)
        : mpArray(std::move(pArray))
        , maOrigin(rOrigin)
    {
    }

    const ScTokenArray* getTokenArray() const { return mpArray.get(); }
    const ScAddress& getOrigin() const { return maOrigin; }
};

class SharedFormulaGroups
{
private:
    typedef std::map<size_t, SharedFormulaGroupEntry> StoreType;
    StoreType m_Store;

public:
    void set(size_t nSharedId, std::unique_ptr<ScTokenArray> pArray);
    void set(size_t nSharedId, std::unique_ptr<ScTokenArray> pArray, const ScAddress& rOrigin);
    const ScTokenArray* get(size_t nSharedId) const;
    const SharedFormulaGroupEntry* getEntry(size_t nSharedId) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
