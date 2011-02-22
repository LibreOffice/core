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

#include <iostream>
#include "resourcemodel/WW8ResourceModel.hxx"
#include "ooxml/OOXMLFastTokens.hxx"

namespace writerfilter
{

size_t TokenHash::operator()(const Token_t & rToken) const
{
    return rToken.getId();
}

Token_t::Token_t()
{
    assign(ooxml::OOXML_FAST_TOKENS_END);
}

Token_t::Token_t(sal_Int32 nId)
{
    assign(nId);
}

void Token_t::assign(sal_Int32 nId)
{
    m_nId = nId;

#ifdef DEBUG
    m_string = ooxml::fastTokenToId(nId);
    ::std::clog << m_string << ::std::endl;
#endif
}

Token_t::~Token_t()
{
}

sal_Int32 Token_t::getId() const
{
    return m_nId;
}

Token_t::operator sal_Int32() const
{
    return getId();
}

Token_t & Token_t::operator = (sal_Int32 nId)
{
    assign(nId);

    return *this;
}

#ifdef DEBUG
::std::string Token_t::toString() const
{
    return m_string;
}
#endif
}
