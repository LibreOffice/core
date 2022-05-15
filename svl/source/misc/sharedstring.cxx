/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/sharedstring.hxx>

namespace svl {

/** ref-counting traffic associated with SharedString temporaries can be significant, so use a singleton here, so we can return a const& from getEmptyString */
static OUString EMPTY(u"");
const SharedString EMPTY_SHARED_STRING(EMPTY.pData, EMPTY.pData);

const SharedString & SharedString::getEmptyString()
{
    // unicode string array for empty string is globally shared in OUString.
    // Let's take advantage of that.
    return EMPTY_SHARED_STRING;
}

SharedString& SharedString::operator= ( const SharedString& r )
{
    if(this == &r)
        return *this;

    if (mpData)
        rtl_uString_release(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_release(mpDataIgnoreCase);

    mpData = r.mpData;
    mpDataIgnoreCase = r.mpDataIgnoreCase;

    if (mpData)
        rtl_uString_acquire(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_acquire(mpDataIgnoreCase);

    return *this;
}

bool SharedString::operator== ( const SharedString& r ) const
{
    // Compare only the original (not case-folded) string.

    if (mpData == r.mpData)
        return true;

    if (mpData)
    {
        if (!r.mpData)
            return false;

        if (mpData->length != r.mpData->length)
            return false;

        return rtl_ustr_reverseCompare_WithLength(mpData->buffer, mpData->length, r.mpData->buffer, r.mpData->length) == 0;
    }

    return !r.mpData;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
