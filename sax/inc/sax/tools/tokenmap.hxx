/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SAX_TOKENMAP_HXX_
#define _SAX_TOKENMAP_HXX_

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include "sax/dllapi.h"
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
