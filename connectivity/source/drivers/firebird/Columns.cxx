/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Columns.hxx"

using namespace ::connectivity;
using namespace ::connectivity::firebird;

using namespace ::cppu;
using namespace ::osl;

Columns::Columns(OWeakObject& rTable,
                 Mutex& rMutex,
                 const TStringVector& rVector):
    OColumnsHelper(rTable,
                   sal_True, // TODO: is this case sensitivity?
                   rMutex,
                   rVector)
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */