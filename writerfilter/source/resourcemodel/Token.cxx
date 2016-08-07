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
