/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svl/sharedstring.hxx"

namespace svl {

SharedString::SharedString() : mpData(NULL), mpDataIgnoreCase(NULL) {}

SharedString::SharedString( rtl_uString* pData, rtl_uString* pDataIgnoreCase ) :
    mpData(pData), mpDataIgnoreCase(pDataIgnoreCase)
{
    if (mpData)
        rtl_uString_acquire(mpData);
    if (mpDataIgnoreCase)
        rtl_uString_acquire(mpDataIgnoreCase);
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

rtl_uString* SharedString::getData()
{
    return mpData;
}

const rtl_uString* SharedString::getData() const
{
    return mpData;
}

rtl_uString* SharedString::getDataIgnoreCase()
{
    return mpDataIgnoreCase;
}

const rtl_uString* SharedString::getDataIgnoreCase() const
{
    return mpDataIgnoreCase;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
