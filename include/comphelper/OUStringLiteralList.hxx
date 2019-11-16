/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_OUSTRINGLITERALLIST_HXX
#define INCLUDED_COMPHELPER_OUSTRINGLITERALLIST_HXX

#include <sal/config.h>

#include <initializer_list>
#include <rtl/ustring.hxx>

namespace comphelper
{
// Constructs std::initializer_list<OUStringLiteral> without explicit functional cast
// to avoid [loplugin:redundantfcast] warnings
inline const std::initializer_list<OUStringLiteral>&
OUStringLiteralList(const std::initializer_list<OUStringLiteral>& list)
{
    return list;
}
}

#endif // INCLUDED_COMPHELPER_OUSTRINGLITERALLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
