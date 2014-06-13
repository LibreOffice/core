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

SharedString SharedString::getEmptyString()
{
    // unicode string array for empty string is globally shared in OUString.
    // Let's take advantage of that.
    rtl_uString* pData = NULL;
    rtl_uString_new(&pData);
    return SharedString(pData, pData);
}

SharedString::SharedString() : mpData(NULL), mpDataIgnoreCase(NULL) {}

SharedString::SharedString( rtl_uString* pData, rtl_uString* pDataIgnoreCase ) :
    mpData(pData), mpDataIgnoreCase(pDataIgnoreCase)
{
    if (mpData)
        rtl_uString_acquire(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_acquire(mpDataIgnoreCase);
}

SharedString::SharedString( const OUString& rStr ) : mpData(rStr.pData), mpDataIgnoreCase(NULL)
{
    rtl_uString_acquire(mpData);
}

SharedString::SharedString( const SharedString& r ) : mpData(r.mpData), mpDataIgnoreCase(r.mpDataIgnoreCase)
{
    if (mpData)
        rtl_uString_acquire(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_acquire(mpDataIgnoreCase);
}

SharedString::~SharedString()
{
    if (mpData)
        rtl_uString_release(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_release(mpDataIgnoreCase);
}

SharedString& SharedString::operator= ( const SharedString& r )
{
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
    // Only compare case sensitive strings.
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

bool SharedString::operator!= ( const SharedString& r ) const
{
    return !operator== (r);
}









}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
