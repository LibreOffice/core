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
    case NS_ooxml::LN_CT_Ind_left:
    case NS_ooxml::LN_CT_Ind_right:
        return std::make_shared<RTFValue>(0);

    default:
        return RTFValue::Pointer_t();
    }
}

/// Is it problematic to deduplicate this SPRM?
static bool isSPRMDeduplicateBlacklist(Id nId)
{
    switch (nId)
    {
    // See the NS_ooxml::LN_CT_PPrBase_tabs handler in DomainMapper,
    // deduplication is explicitly not wanted for these tokens.
    case NS_ooxml::LN_CT_TabStop_val:
    case NS_ooxml::LN_CT_TabStop_leader:
    case NS_ooxml::LN_CT_TabStop_pos:
    // Erasing these just because they equal to the style one is
    // problematic, as then the used value won't be from the style, but
    // possibly from the numbering.
    case NS_ooxml::LN_CT_Ind_left:
    case NS_ooxml::LN_CT_Ind_right:
    // \htmautsp arrives after the style table, so only the non-style value is
    // correct, keep these.
    case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
    case NS_ooxml::LN_CT_Spacing_afterAutospacing:
        return true;

    default:
        return false;
    }
}

/// Should this SPRM be removed if all its children is removed?
static bool isSPRMChildrenExpected(Id nId)
{
    switch (nId)
    {
    case NS_ooxml::LN_CT_PBdr_top:
    case NS_ooxml::LN_CT_PBdr_left:
    case NS_ooxml::LN_CT_PBdr_bottom:
    case NS_ooxml::LN_CT_PBdr_right:
        // Expected children are NS_ooxml::LN_CT_Border_*.
        SAL_FALLTHROUGH;
    case NS_ooxml::LN_CT_PrBase_shd:
        // Expected children are NS_ooxml::LN_CT_Shd_*.
        return true;

    default:
        return false;
    }
}

/// Does the clone / deduplication of a single sprm.
static void cloneAndDeduplicateSprm(std::pair<Id, RTFValue::Pointer_t>& rSprm, RTFSprms& ret)
{
    RTFValue::Pointer_t const pValue(ret.find(rSprm.first));
    if (pValue)
    {
        if (rSprm.second->equals(*pValue))
        {
            if (!isSPRMDeduplicateBlacklist(rSprm.first))
                ret.erase(rSprm.first); // duplicate to style
        }
        else if (!rSprm.second->getSprms().empty() || !rSprm.second->getAttributes().empty())
        {
            RTFSprms const sprms(pValue->getSprms().cloneAndDeduplicate(rSprm.second->getSprms()));
            RTFSprms const attributes(pValue->getAttributes().cloneAndDeduplicate(rSprm.second->getAttributes()));
            // Don't copy the sprm in case we expect it to have children but it doesn't have some.
            if (!isSPRMChildrenExpected(rSprm.first) || !sprms.empty() || !attributes.empty())
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

RTFSprms RTFSprms::cloneAndDeduplicate(RTFSprms& rReference) const
{
    RTFSprms ret(*this);
    ret.ensureCopyBeforeWrite();

    // Note: apparently some attributes are set with OVERWRITE_NO_APPEND;
    // it is probably a bad idea to mess with those in any way here?
    for (auto& rSprm : rReference)
    {
        // Paragraph formatting sprms are directly contained in case of
        // paragraphs, but they are below NS_ooxml::LN_CT_Style_pPr in case of
        // styles. So handle those children directly, to avoid unexpected
        // addition of direct formatting sprms at the paragraph level.
        if (rSprm.first == NS_ooxml::LN_CT_Style_pPr)
        {
            for (auto& i : rSprm.second->getSprms())
                cloneAndDeduplicateSprm(i, ret);
        }
        else
            cloneAndDeduplicateSprm(rSprm, ret);
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
        boost::intrusive_ptr<RTFSprmsImpl> pClone(new RTFSprmsImpl);
        for (auto& rSprm : *m_pSprms)
            pClone->push_back(std::make_pair(rSprm.first, RTFValue::Pointer_t(rSprm.second->Clone())));
        m_pSprms = pClone;
    }
}

RTFSprms::RTFSprms()
    : m_pSprms(new RTFSprmsImpl)
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
        m_pSprms.reset(new RTFSprmsImpl);
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
