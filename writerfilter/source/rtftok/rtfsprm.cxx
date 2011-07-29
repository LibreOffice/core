/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <rtfsprm.hxx>
#include <rtl/strbuf.hxx>

#include <resourcemodel/QNameToString.hxx>

using rtl::OStringBuffer;

namespace writerfilter {
namespace rtftok {

RTFSprm::RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue)
    : m_nKeyword(nKeyword),
    m_pValue(pValue)
{
}

sal_uInt32 RTFSprm::getId() const
{
    return m_nKeyword;
}

Value::Pointer_t RTFSprm::getValue()
{
    return Value::Pointer_t(m_pValue->Clone());
}

writerfilter::Reference<BinaryObj>::Pointer_t RTFSprm::getBinary()
{
    return m_pValue->getBinary();
}

writerfilter::Reference<Stream>::Pointer_t RTFSprm::getStream()
{
    return m_pValue->getStream();
}

writerfilter::Reference<Properties>::Pointer_t RTFSprm::getProps()
{
    return m_pValue->getProperties();
}

Sprm::Kind RTFSprm::getKind()
{
    return Sprm::UNKNOWN;
}

std::string RTFSprm::getName() const
{
    return "RTFSprm";
}

std::string RTFSprm::toString() const
{
    OStringBuffer aBuf("RTFSprm");

    std::string sResult = (*QNameToString::Instance())(m_nKeyword);
    if (sResult.length() == 0)
        sResult = (*SprmIdToString::Instance())(m_nKeyword);

    aBuf.append(" ('");
    if (sResult.length() == 0)
        aBuf.append(sal_Int32(m_nKeyword));
    else
        aBuf.append(sResult.c_str());
    aBuf.append("', '");
    aBuf.append(m_pValue->toString().c_str());
    aBuf.append("')");

    return aBuf.makeStringAndClear().getStr();
}

RTFValue::Pointer_t RTFSprms::find(Id nKeyword)
{
    for (RTFSprms::Iterator_t i = m_aSprms.begin(); i != m_aSprms.end(); ++i)
        if (i->first == nKeyword)
            return i->second;
    RTFValue::Pointer_t pValue;
    return pValue;
}

bool RTFSprms::erase(Id nKeyword)
{
    for (RTFSprms::Iterator_t i = m_aSprms.begin(); i != m_aSprms.end(); ++i)
        if (i->first == nKeyword)
        {
            m_aSprms.erase(i);
            return true;
        }
    return false;
}

RTFSprms::RTFSprms()
    : m_aSprms()
{
}

RTFSprms::RTFSprms(const RTFSprms& rSprms)
{
    for (std::vector< std::pair<Id, RTFValue::Pointer_t> >::const_iterator i = rSprms.m_aSprms.begin(); i != rSprms.m_aSprms.end(); ++i)
        m_aSprms.push_back(std::make_pair(i->first, RTFValue::Pointer_t(i->second->Clone())));
}

std::vector< std::pair<Id, RTFValue::Pointer_t> >* RTFSprms::operator->()
{
    return &m_aSprms;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
