/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfsprm.hxx>
#include <rtl/strbuf.hxx>

#include <resourcemodel/QNameToString.hxx>


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

RTFValue::Pointer_t RTFSprms::find(Id nKeyword, bool bFirst)
{
    RTFValue::Pointer_t pValue;
    for (RTFSprms::Iterator_t i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
        if (i->first == nKeyword)
        {
            if (bFirst)
                return i->second;
            else
                pValue = i->second;
        }
    return pValue;
}

void RTFSprms::set(Id nKeyword, RTFValue::Pointer_t pValue, bool bOverwrite, bool bAppend)
{
    ensureCopyBeforeWrite();
    bool bFound = false;
    if (bOverwrite || !bAppend)
    {
        for (RTFSprms::Iterator_t i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
            if (i->first == nKeyword)
            {
                if (bOverwrite)
                {
                    i->second = pValue;
                    return;
                }
                else
                {
                    bFound = true;
                    break;
                }
            }
    }
    if (bAppend || !bFound)
        m_pSprms->push_back(std::make_pair(nKeyword, pValue));
}

bool RTFSprms::erase(Id nKeyword)
{
    ensureCopyBeforeWrite();
    for (RTFSprms::Iterator_t i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
    {
        if (i->first == nKeyword)
        {
            m_pSprms->erase(i);
            return true;
        }
    }
    return false;
}

void RTFSprms::deduplicate(RTFSprms& rReference)
{
    ensureCopyBeforeWrite();

    RTFSprms::Iterator_t i = m_pSprms->begin();
    while (i != m_pSprms->end())
    {
        bool bIgnore = false;
        RTFValue::Pointer_t pValue(rReference.find(i->first));
        if (pValue.get() && i->second->equals(*pValue))
            bIgnore = true;
        if (bIgnore)
            i = m_pSprms->erase(i);
        else
            ++i;
    }
}

void RTFSprms::ensureCopyBeforeWrite()
{
    if (m_pSprms->m_nRefCount > 1) {
        boost::intrusive_ptr<RTFSprmsImpl> pClone(new RTFSprmsImpl());
        for (std::vector< std::pair<Id, RTFValue::Pointer_t> >::const_iterator i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
            pClone->push_back(std::make_pair(i->first, RTFValue::Pointer_t(i->second->Clone())));
        m_pSprms = pClone;
    }
}

RTFSprms::RTFSprms()
    : m_pSprms(new RTFSprmsImpl())
{
}

RTFSprms::~RTFSprms()
{
}

RTFSprms::RTFSprms(const RTFSprms& rSprms)
{
    *this = rSprms;
}

RTFSprms& RTFSprms::operator=(const RTFSprms& rOther)
{
    m_pSprms = rOther.m_pSprms;
    return *this;
}

void RTFSprms::clear()
{
    if (m_pSprms->m_nRefCount == 1)
        return m_pSprms->clear();
    else
        m_pSprms.reset(new RTFSprmsImpl());
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
