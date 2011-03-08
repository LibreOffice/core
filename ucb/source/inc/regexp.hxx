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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
