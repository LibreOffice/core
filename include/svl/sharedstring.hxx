/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_SHAREDSTRING_HXX
#define INCLUDED_SVL_SHAREDSTRING_HXX

#include <svl/svldllapi.h>
#include <rtl/ustring.hxx>

namespace svl {

class SVL_DLLPUBLIC SharedString
{
    rtl_uString* mpData;
    rtl_uString* mpDataIgnoreCase;
public:

    static SharedString getEmptyString();

    SharedString();
    SharedString( rtl_uString* pData, rtl_uString* pDataIgnoreCase );
    SharedString( const OUString& rStr );
    SharedString( const SharedString& r );
    ~SharedString();

    SharedString& operator= ( const SharedString& r );

    bool operator== ( const SharedString& r ) const;
    bool operator!= ( const SharedString& r ) const;

    OUString getString() const { return mpData ? OUString(mpData) : OUString();}

    rtl_uString* getData() { return mpData;}
    const rtl_uString* getData() const { return mpData;}

    rtl_uString* getDataIgnoreCase() { return mpDataIgnoreCase;}
    const rtl_uString* getDataIgnoreCase() const { return mpDataIgnoreCase;}

    bool isValid() const { return mpData != NULL;}
    bool isEmpty() const { return mpData == NULL || mpData->length == 0;}

    sal_Int32 getLength() const { return mpData ? mpData->length : 0;}
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
