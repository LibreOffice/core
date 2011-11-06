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



#ifndef _UCB_REGEXP_HXX_
#define _UCB_REGEXP_HXX_

#include <rtl/ustring.hxx>

//============================================================================
namespace ucb_impl {

class Regexp
{
public:
    enum Kind
    {
        KIND_PREFIX,
        KIND_AUTHORITY,
        KIND_DOMAIN
    };

    inline bool operator ==(Regexp const & rOther) const;

    inline bool isDefault() const
    { return m_eKind == KIND_PREFIX && m_aPrefix.getLength() == 0; }

    inline Kind getKind() const { return m_eKind; }

    bool matches(rtl::OUString const & rString, rtl::OUString * pTranslation,
                 bool * pTranslated) const;

    rtl::OUString getRegexp(bool bReverse) const;

    static Regexp parse(rtl::OUString const & rRegexp);

private:
    Kind m_eKind;
    rtl::OUString m_aPrefix;
    rtl::OUString m_aInfix;
    rtl::OUString m_aReversePrefix;
    bool m_bEmptyDomain;
    bool m_bTranslation;

    inline Regexp(Kind eTheKind, rtl::OUString const & rThePrefix,
                  bool bTheEmptyDomain, rtl::OUString const & rTheInfix,
                  bool bTheTranslation,
                  rtl::OUString const & rTheReversePrefix);
};

inline bool Regexp::operator ==(Regexp const & rOther) const
{
    return m_eKind == rOther.m_eKind
           && m_aPrefix == rOther.m_aPrefix
           && m_aInfix == rOther.m_aInfix;
}

}

#endif // _UCB_REGEXP_HXX_

