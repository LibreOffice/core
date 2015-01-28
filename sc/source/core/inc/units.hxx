/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_SOURCE_CORE_INC_UNITS_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_UNITS_HXX

#include <boost/shared_ptr.hpp>

struct ut_system;

namespace sc { namespace units {

class Units {
private:
    ::boost::shared_ptr< ut_system > mpUnitSystem;

public:
    Units();
};

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_INC_UNITS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

