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
#include <rtl/strbuf.hxx>

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

    for (auto& rSprm : *m_pSprms)
        if (rSprm.first == nKeyword)
        {
            if (bFirst)
                return rSprm.second;
            else
                pValue = rSprm.second;
        }
    return pValue;
}

void RTFSprms::set(Id nKeyword, RTFValue::Pointer_t pValue, RTFOverwrite eOverwrite)
{
    ensureCopyBeforeWrite();
    bool bFound = false;
    if (eOverwrite == RTFOverwrite::YES || eOverwrite == RTFOverwrite::NO_IGNORE)
    {
        for (auto& rSprm : *m_pSprms)
            if (rSprm.first == nKeyword)
            {
                if (eOverwrite == RTFOverwrite::YES)
                {
                    rSprm.second = pValue;
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
    for (auto i = m_pSprms->begin(); i != m_pSprms->end(); ++i)
    {
        if (i->first == nKeyword)
        {
            m_pSprms->erase(i);
            return true;
        }
    }
    return false;
}

void RTFSprms::eraseLast(Id nKeyword)
{
    ensureCopyBeforeWrite();
    for (auto i = m_pSprms->rbegin(); i != m_pSprms->rend(); ++i)
    {
        if (i->first == nKeyword)
        {
            m_pSprms->erase(std::next(i).base());
            return;
        }
    }
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
    for (auto& rSprm : rReference)
    {
        RTFValue::Pointer_t const pValue(ret.find(rSprm.first));
        if (pValue)
        {
            if (rSprm.second->equals(*pValue))
            {
                ret.erase(rSprm.first); // duplicate to style
            }
            else if (!rSprm.second->getSprms().empty() || !rSprm.second->getAttributes().empty())
            {
                RTFSprms const sprms(pValue->getSprms().cloneAndDeduplicate(rSprm.second->getSprms()));
                RTFSprms const attributes(pValue->getAttributes().cloneAndDeduplicate(rSprm.second->getAttributes()));
                ret.set(rSprm.first, RTFValue::Pointer_t(pValue->CloneWithSprms(attributes, sprms)));
            }
        }
        else
        {
            // not found - try to override style with default
            RTFValue::Pointer_t const pDefault(getDefaultSPRM(rSprm.first));
            if (pDefault)
            {
                ret.set(rSprm.first, pDefault);
            }
            else if (!rSprm.second->getSprms().empty() || !rSprm.second->getAttributes().empty())
            {
                RTFSprms const sprms(RTFSprms().cloneAndDeduplicate(rSprm.second->getSprms()));
                RTFSprms const attributes(RTFSprms().cloneAndDeduplicate(rSprm.second->getAttributes()));
                if (!sprms.empty() || !attributes.empty())
                {
                    ret.set(rSprm.first, std::make_shared<RTFValue>(attributes, sprms));
                }
            }
        }
    }
    return ret;
}

bool RTFSprms::equals(RTFValue& rOther)
{
    for (auto& rSprm : *m_pSprms)
        if (!rSprm.second->equals(rOther))
            return false;
    return true;
}

void RTFSprms::ensureCopyBeforeWrite()
{
    if (m_pSprms->m_nRefCount > 1)
    {
        boost::intrusive_ptr<RTFSprmsImpl> pClone(new RTFSprmsImpl());
        for (auto& rSprm : *m_pSprms)
            pClone->push_back(std::make_pair(rSprm.first, RTFValue::Pointer_t(rSprm.second->Clone())));
        m_pSprms = pClone;
    }
}

RTFSprms::RTFSprms()
    : m_pSprms(new RTFSprmsImpl())
{
}

RTFSprms::~RTFSprms() = default;

RTFSprms::RTFSprms(const RTFSprms& rSprms)
{
    *this = rSprms;
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
