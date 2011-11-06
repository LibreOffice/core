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



#ifndef _ADRPARSE_HXX
#define _ADRPARSE_HXX

#include "svl/svldllapi.h"
#include <tools/list.hxx>
#include <tools/string.hxx>

//============================================================================
struct SvAddressEntry_Impl
{
    UniString m_aAddrSpec;
    UniString m_aRealName;

    SvAddressEntry_Impl() {};
    SvAddressEntry_Impl(UniString const & rTheAddrSpec,
                        UniString const & rTheRealName):
        m_aAddrSpec(rTheAddrSpec), m_aRealName(rTheRealName) {}
};

//============================================================================
DECLARE_LIST(SvAddressList_Impl, SvAddressEntry_Impl *)

//============================================================================
class SVL_DLLPUBLIC SvAddressParser
{
    friend class SvAddressParser_Impl;

    SvAddressEntry_Impl m_aFirst;
    SvAddressList_Impl m_aRest;
    bool m_bHasFirst;

public:
    SvAddressParser(UniString const & rInput);

    ~SvAddressParser();

    sal_Int32 Count() const { return m_bHasFirst ? m_aRest.Count() + 1 : 0; }

    inline UniString const & GetEmailAddress(sal_Int32 nIndex) const;

    inline UniString const &GetRealName(sal_Int32 nIndex) const;

    /** Create an RFC 822 <mailbox> (i.e., 'e-mail address').

        @param rPhrase  Either an empty string (the <mailbox> will have no
        <phrase> an will be of the form <addr-spec>), or some text that will
        become the <phrase> part of a <phrase route-addr> form <mailbox>.  Non
        US-ASCII characters within the text are put into a <qouted-string>
        verbatim, so the result may actually not be a valid RFC 822 <mailbox>,
        but a more human-readable representation.

        @param rAddrSpec  A valid RFC 822 <addr-spec>.  (An RFC 822 <mailbox>
        including a <route> cannot be created by this method.)

        @param rMailbox  If this method returns true, this parameter returns
        the created RFC 822 <mailbox> (rather, a more human-readable
        representation thereof).  Otherwise, this parameter is not modified.

        @return  True, if rAddrSpec is a valid RFC 822 <addr-spec>.
     */
    static bool createRFC822Mailbox(String const & rPhrase,
                                    String const & rAddrSpec,
                                    String & rMailbox);
};

inline UniString const & SvAddressParser::GetEmailAddress(sal_Int32 nIndex)
    const
{
    return nIndex == 0 ? m_aFirst.m_aAddrSpec :
                         m_aRest.GetObject(nIndex - 1)->m_aAddrSpec;
}

inline UniString const & SvAddressParser::GetRealName(sal_Int32 nIndex) const
{
    return nIndex == 0 ? m_aFirst.m_aRealName :
                         m_aRest.GetObject(nIndex - 1)->m_aRealName;
}

#endif // _ADRPARSE_HXX

