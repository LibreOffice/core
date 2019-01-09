/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_SHAREDFORMULAGROUPS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_SHAREDFORMULAGROUPS_HXX

#include <memory>
#include <map>
class ScTokenArray;

namespace sc {

class SharedFormulaGroups
{
private:
    typedef std::map<size_t, std::unique_ptr<ScTokenArray>> StoreType;
    StoreType m_Store;

public:
    void set( size_t nSharedId, std::unique_ptr<ScTokenArray> pArray );
    const ScTokenArray* get( size_t nSharedId ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
