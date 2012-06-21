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

#ifndef _SAX_ATTRIBUTEMAP_HXX_
#define _SAX_ATTRIBUTEMAP_HXX_

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
