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

#ifndef _SAX_ATTRIBUTEMAP_HXX_
#define _SAX_ATTRIBUTEMAP_HXX_

#include "sax/dllapi.h"

#include <map>
#include "sax/tools/tokenmap.hxx"

namespace sax
{
    class SaxTokenMap;

    typedef std::map< sal_uInt32, rtl::OString > AttributeMapBase;

    /** a map for a set of xml attributes, identified with integer tokens.
        Attribute values are stored in utf-8 encoding. */
    class SAX_DLLPUBLIC AttributeMap : public AttributeMapBase
    {
    public:
        AttributeMap( const SaxTokenMap& rTokenMap );
        ~AttributeMap();

        /** returns a unicode string, if the token does not exists the string is empty */
        ::rtl::OUString getString( SaxToken nToken ) const;

        /** returns true if the attribute with the token nToken is part of this map */
        bool has( SaxToken nToken ) const;

        /** converts the attribute with the token nToken to sal_Int32 or returns
            nDefault if this attribute does not exists */
        sal_Int32 getInt32( SaxToken nToken, sal_Int32 nDefault = 0 ) const;

        /** converts the attribute with the token nToken to a token or returns
            nDefault if this attribute does not exists */
        sal_uInt32 getToken( SaxToken nToken, SaxToken nDefault ) const;

    private:
        const SaxTokenMap& mrTokenMap;
    };

}

#endif // _SAX_ATTRIBUTEMAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
