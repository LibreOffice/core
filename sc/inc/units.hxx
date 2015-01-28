/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_INC_UNITS_HXX
#define INCLUDED_SC_INC_UNITS_HXX

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <osl/mutex.hxx>

struct ut_system;

class ScAddress;
class ScDocument;
class ScTokenArray;

namespace sc {

/*
 * We implement this as a singleton which automatically
 * cleans itself up thanks to the use of shared and weak
 * pointers.
 */
class Units {
private:
    // A scoped_ptr would be more appropriate, however
    // we require a custom deleter which scoped_ptr doesn't
    // offer.
    ::boost::shared_ptr< ut_system > mpUnitSystem;

    Units();

    static ::osl::Mutex ourSingletonMutex;
    static ::boost::weak_ptr< Units > ourUnits;

public:
    static ::boost::shared_ptr< Units > GetUnits();

    ~Units();

    bool verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc);

};

} // namespace sc

#endif // INCLUDED_SC_INC_UNITS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

