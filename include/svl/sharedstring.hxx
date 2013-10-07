/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SVL_SHAREDSTRING_HXX
#define SVL_SHAREDSTRING_HXX

#include "svl/svldllapi.h"
#include "rtl/ustring.hxx"

namespace svl {

class SVL_DLLPUBLIC SharedString
{
    rtl_uString* mpData;
    rtl_uString* mpDataIgnoreCase;
public:
    SharedString();
    SharedString( rtl_uString* pData, rtl_uString* pDataIgnoreCase );
    SharedString( const SharedString& r );
    ~SharedString();

    SharedString& operator= ( const SharedString& r );

    bool operator== ( const SharedString& r ) const;

    rtl_uString* getData();
    const rtl_uString* getData() const;

    rtl_uString* getDataIgnoreCase();
    const rtl_uString* getDataIgnoreCase() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
