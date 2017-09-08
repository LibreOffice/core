/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_INC_FILTERENTRIES_HXX
#define SC_INC_FILTERENTRIES_HXX

#include <sal/config.h>
#include "typedstrdata.hxx"
#include <vector>

struct ScFilterEntries
{
    std::vector<ScTypedStrData> maStrData;
    bool                        mbHasDates;
    bool                        mbHasEmpties;

    ScFilterEntries() : mbHasDates(false), mbHasEmpties(false) {}

    std::vector<ScTypedStrData>::iterator       begin()         { return maStrData.begin(); }
    std::vector<ScTypedStrData>::iterator       end()           { return maStrData.end(); }
    std::vector<ScTypedStrData>::const_iterator begin() const   { return maStrData.begin(); }
    std::vector<ScTypedStrData>::const_iterator end()   const   { return maStrData.end(); }
    std::vector<ScTypedStrData>::size_type      size()  const   { return maStrData.size(); }
          ScTypedStrData&                       front()         { return maStrData.front(); }
    const ScTypedStrData&                       front() const   { return maStrData.front(); }
    bool                                        empty() const   { return maStrData.empty(); }
    void                                        push_back( const ScTypedStrData& r ) { maStrData.push_back(r); }
    void                                        push_back( ScTypedStrData&& r )      { maStrData.push_back(r); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
