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



#ifndef CONNECTIVITY_QUOTED_STRING_HXX
#define CONNECTIVITY_QUOTED_STRING_HXX

#include <tools/string.hxx>
#include "file/filedllapi.hxx"

namespace connectivity
{
    //==================================================================
    // Ableitung von String mit ueberladenen GetToken/GetTokenCount-Methoden
    // Speziell fuer FLAT FILE-Format: Strings koennen gequotet sein
    //==================================================================
    class OOO_DLLPUBLIC_FILE QuotedTokenizedString
    {
        String m_sString;
    public:
        QuotedTokenizedString() {}
        QuotedTokenizedString(const String& _sString) : m_sString(_sString){}

        xub_StrLen  GetTokenCount( sal_Unicode cTok , sal_Unicode cStrDel ) const;
        void        GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok = ';', sal_Unicode cStrDel = '\0' ) const;
        inline String& GetString() { return m_sString; }
        inline xub_StrLen Len() const { return m_sString.Len(); }
        inline operator String&() { return m_sString; }
    };
}

#endif // CONNECTIVITY_QUOTED_STRING_HXX
