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
