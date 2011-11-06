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



#ifndef OOX_TOKEN_TOKENMAP_HXX
#define OOX_TOKEN_TOKENMAP_HXX

#include <vector>
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace oox {

// ============================================================================

class TokenMap
{
public:
    explicit            TokenMap();
                        ~TokenMap();

    /** Returns the Unicode name of the passed token identifier. */
    ::rtl::OUString     getUnicodeTokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed Unicode token name. */
    sal_Int32           getTokenFromUnicode( const ::rtl::OUString& rUnicodeName ) const;

    /** Returns the UTF8 name of the passed token identifier as byte sequence. */
    ::com::sun::star::uno::Sequence< sal_Int8 >
                        getUtf8TokenName( sal_Int32 nToken ) const;

    /** Returns the token identifier for the passed UTF8 token name. */
    sal_Int32           getTokenFromUtf8(
                            const ::com::sun::star::uno::Sequence< sal_Int8 >& rUtf8Name ) const;

private:
    struct TokenName
    {
        ::rtl::OUString maUniName;
        ::com::sun::star::uno::Sequence< sal_Int8 > maUtf8Name;
    };
    typedef ::std::vector< TokenName > TokenNameVector;

    TokenNameVector     maTokenNames;
};

// ============================================================================

struct StaticTokenMap : public ::rtl::Static< TokenMap, StaticTokenMap > {};

// ============================================================================

} // namespace oox

#endif

