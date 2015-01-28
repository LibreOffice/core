/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "units.hxx"

#include <udunits2.h>

using namespace sc::units;

static void freeUtSystem( ut_system* pSystem ) {
    ut_free_system( pSystem );
}

Units::Units() {
    // TODO: we need to get the path for our unit database, but only
    // if using bundled udunits.
    mpUnitSystem = boost::shared_ptr< ut_system >( ut_read_xml( 0 ),
                                                   &freeUtSystem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
