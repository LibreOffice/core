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


#ifndef _TOOLS_RESARY_HXX
#define _TOOLS_RESARY_HXX

#include "tools/toolsdllapi.h"
#include <tools/resid.hxx>
#include <tools/rc.hxx>

// ---------------------
// - ImplResStringItem -
// ---------------------

struct ImplResStringItem
{
    XubString   maStr;
    long        mnValue;

//#if 0 // _SOLAR__PRIVATE
#ifdef _TOOLS_RESARY_CXX
                ImplResStringItem( const XubString& rStr ) :
                    maStr( rStr ) {}
#endif
//#endif
};

// ------------------
// - ResStringArray -
// ------------------

#define RESARRAY_INDEX_NOTFOUND (0xffffffff)

class TOOLS_DLLPUBLIC ResStringArray
{
    private:
    // ---------------------
    // - ImplResStringItem -
    // ---------------------
    struct ImplResStringItem
    {
        XubString   m_aStr;
        long        m_nValue;

        ImplResStringItem( const XubString& rStr, long nValue = 0 ) :
        m_aStr( rStr ),
        m_nValue( nValue )
        {}
    };

    std::vector< ImplResStringItem >    m_aStrings;

    public:
    ResStringArray( const ResId& rResId );
    ~ResStringArray();

    const XubString&    GetString( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_aStr : String::EmptyString(); }
    long                GetValue( sal_uInt32 nIndex ) const
    { return (nIndex < m_aStrings.size()) ? m_aStrings[nIndex].m_nValue : -1; }
    sal_uInt32          Count() const { return sal_uInt32(m_aStrings.size()); }

    sal_uInt32          FindIndex( long nValue ) const;

    private:
    ResStringArray( const ResStringArray& );
    ResStringArray&     operator=( const ResStringArray& );
};

#endif  // _TOOLS_RESARY_HXX
