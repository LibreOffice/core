/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_RANDOM_HXX
#define INCLUDED_COMPHELPER_RANDOM_HXX

#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

namespace rng
{

/// set initial seed (equivalent of libc srand())
COMPHELPER_DLLPUBLIC void seed(int i);

/// uniform distribution in [0,1)
COMPHELPER_DLLPUBLIC double uniform();

} // namespace

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
