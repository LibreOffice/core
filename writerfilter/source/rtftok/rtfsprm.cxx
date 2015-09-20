/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfsprm.hxx>
#include <ooxml/resourceids.hxx>
#include <ooxml/QNameToString.hxx>


namespace writerfilter
{
namespace rtftok
{

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

writerfilter::Reference<Properties>::Pointer_t RTFSprm::getProps()
{
    return m_pValue->getProperties();
}

#ifdef DEBUG_WRITERFILTER
std::string RTFSprm::getName() const
{
    return "RTFSprm";
}
#endif

#ifdef DEBUG_WRITERFILTER
std::string RTFSprm::toString() const
{
    OStringBuffer aBuf("RTFSprm");

    std::string sResult;

    sResult = (*QNameToString::Instance())(m_nKeyword);

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
#endif

RTFValue::Pointer_t RTFSprms::find(Id nKeyword, bool bFirst, bool bForWrite)
{
    RTFValue::Pointer_t pValue;

    if (bForWrite)
        ensureCopyBeforeWrite();

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

void RTFSprms::set(Id nKeyword, RTFValue::Pointer_t pValue, RTFOverwrite eOverwrite)
{
    ensureCopyBeforeWrite();
    bool bFound = false;
    if (eOverwrite == RTFOverwrite::YES || eOverwrite == RTFOverwrite::NO_IGNORE)
    {
        for (RTFSprms::Iterator_t i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
            if (i->first == nKeyword)
            {
                if (eOverwrite == RTFOverwrite::YES)
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
    if (eOverwrite == RTFOverwrite::NO_APPEND || !bFound)
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

static RTFValue::Pointer_t getDefaultSPRM(Id const id)
{
    switch (id)
    {
    case NS_ooxml::LN_CT_Spacing_before:
    case NS_ooxml::LN_CT_Spacing_after:
    case NS_ooxml::LN_EG_RPrBase_b:
        return std::make_shared<RTFValue>(0);

    default:
        return RTFValue::Pointer_t();
    }
}

RTFSprms RTFSprms::cloneAndDeduplicate(RTFSprms& rReference) const
{
    RTFSprms ret(*this);
    ret.ensureCopyBeforeWrite();

    // Note: apparently some attributes are set with OVERWRITE_NO_APPEND;
    // it is probably a bad idea to mess with those in any way here?
    for (RTFSprms::Iterator_t i = rReference.begin(); i != rReference.end(); ++i)
    {
        RTFValue::Pointer_t const pValue(ret.find(i->first));
        if (pValue)
        {
            if (i->second->equals(*pValue))
            {
                ret.erase(i->first); // duplicate to style
            }
            else if (!i->second->getSprms().empty() || !i->second->getAttributes().empty())
            {
                RTFSprms const sprms(
                    pValue->getSprms().cloneAndDeduplicate(i->second->getSprms()));
                RTFSprms const attributes(
                    pValue->getAttributes().cloneAndDeduplicate(i->second->getAttributes()));
                ret.set(i->first, RTFValue::Pointer_t(
                            pValue->CloneWithSprms(attributes, sprms)));
            }
        }
        else
        {
            // not found - try to override style with default
            RTFValue::Pointer_t const pDefault(getDefaultSPRM(i->first));
            if (pDefault)
            {
                ret.set(i->first, pDefault);
            }
            else if (!i->second->getSprms().empty() || !i->second->getAttributes().empty())
            {
                RTFSprms const sprms(
                    RTFSprms().cloneAndDeduplicate(i->second->getSprms()));
                RTFSprms const attributes(
                    RTFSprms().cloneAndDeduplicate(i->second->getAttributes()));
                if (!sprms.empty() || !attributes.empty())
                {
                    ret.set(i->first, std::make_shared<RTFValue>(attributes, sprms));
                }
            }
        }
    }
    return ret;
}

bool RTFSprms::equals(RTFValue& rOther)
{
    RTFSprms::Iterator_t i = m_pSprms->begin();
    while (i != m_pSprms->end())
        if (!i->second->equals(rOther))
            return false;
    return true;
}

void RTFSprms::ensureCopyBeforeWrite()
{
    if (m_pSprms->m_nRefCount > 1)
    {
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
