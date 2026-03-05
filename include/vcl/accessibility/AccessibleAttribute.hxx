/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

namespace AccessibleAttribute
{
// row index text as specified in ARIA,
// maps to attribute of the same name for AT-SPI2, IAccessible2, UIA
// https://www.w3.org/TR/core-aam-1.2/#ariaRowIndexText
constexpr OUString RowIndexText = u"rowindextext"_ustr;

// column index text as specified in ARIA,
// maps to attribute of the same name for AT-SPI2, IAccessible2, UIA
// https://www.w3.org/TR/core-aam-1.2/#ariaColIndexText
constexpr OUString ColIndexText = u"colindextext"_ustr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
