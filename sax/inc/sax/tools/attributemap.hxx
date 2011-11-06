/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
