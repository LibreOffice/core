/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SAX_TOKENMAP_HXX_
#define _SAX_TOKENMAP_HXX_

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include "sax/tools/saxobject.hxx"

namespace sax
{
    /** type for a token identifier */
    typedef sal_uInt32 SaxToken;

    /** this class maps a set of ascii/utf-8 strings to token identifier */
    class SAX_DLLPUBLIC SaxTokenMap : public SaxObject
    {
    public:
        /** constant do indicate an unknown token */
        const static SaxToken InvalidToken = (SaxToken)-1;

        /** returns the token identifier for the given ascii string or SaxTokenMap::InvalidToken */
        virtual SaxToken GetToken( const sal_Char* pChar, sal_uInt32 nLength  = 0 ) const = 0;

        /** returns the token identifier for the given unicode string or SaxTokenMap::InvalidToken */
        virtual SaxToken GetToken( const ::rtl::OUString& rToken ) const = 0;

        /** returns the unicode string for the given token identifier */
        virtual const ::rtl::OUString& GetToken( SaxToken nToken ) const = 0;

        /** returns if the given unicode string equals the given token identifier */
        bool IsToken( const ::rtl::OUString& rToken, SaxToken nToken ) const { return GetToken( rToken ) == nToken; }
    };

    /** reference type to a SaxTokenMap */
    typedef rtl::Reference< SaxTokenMap > SaxTokenMapRef;
}

#endif // _SAX_TOKENMAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
