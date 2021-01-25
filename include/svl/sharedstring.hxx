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

    static const SharedString & getEmptyString();

    SharedString();
    SharedString( rtl_uString* pData, rtl_uString* pDataIgnoreCase );
    explicit SharedString( const OUString& rStr );
    SharedString( const SharedString& r );
    SharedString(SharedString&& r) noexcept;
    ~SharedString();

    SharedString& operator= ( const SharedString& r );
    SharedString& operator=(SharedString&& r) noexcept;

    bool operator== ( const SharedString& r ) const;
    bool operator!= ( const SharedString& r ) const;

    OUString getString() const;

    rtl_uString* getData();
    const rtl_uString* getData() const;

    rtl_uString* getDataIgnoreCase();
    const rtl_uString* getDataIgnoreCase() const;

    bool isValid() const;
    bool isEmpty() const;

    sal_Int32 getLength() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
