/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_SOURCE_CORE_UNITS_RAUSTACK_HXX
#define INCLUDED_SC_SOURCE_CORE_UNITS_RAUSTACK_HXX

#include <boost/variant.hpp>

#include <address.hxx>
#include <dociter.hxx>
#include <rangelst.hxx>


#include "utunit.hxx"

namespace sc {
namespace units {

enum class RAUSItemType {
    UNITS,
    RANGE
};

struct RAUSItem {
    RAUSItemType type;
    boost::variant< ScRange, UtUnit > item;
};

class RangeListIterator {
private:
    const ScRangeList mRangeList;
    ScDocument* mpDoc;

    ScCellIterator mIt;
    size_t nCurrentIndex;

public:
    RangeListIterator(ScDocument* pDoc, const ScRangeList& rRangeList);

    bool first();

    const ScAddress& GetPos() const;

    bool next();
};

}} // sc::units

// class RangeAndUnitStack {



// };

// class RATSIterator {
// public:
//     // TODO: need to be able to return non-initialisation
//     static RATSIterator getIterator(RangeAndTokenStack& rStack, ScDoc* pDoc, int nItems);

// }

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_RAUSTACK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
