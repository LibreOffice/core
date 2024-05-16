/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfsprm.hxx"
#include <ooxml/resourceids.hxx>
#include <ooxml/QNameToString.hxx>
#include <rtl/strbuf.hxx>
#include "rtfdocumentimpl.hxx"
#include <algorithm>

namespace writerfilter::rtftok
{
RTFSprm::RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue)
    : m_nKeyword(nKeyword)
    , m_pValue(pValue)
{
}

sal_uInt32 RTFSprm::getId() const { return m_nKeyword; }

Value::Pointer_t RTFSprm::getValue() { return Value::Pointer_t(m_pValue->Clone()); }

writerfilter::Reference<Properties>::Pointer_t RTFSprm::getProps()
{
    return m_pValue->getProperties();
}

#ifdef DBG_UTIL
std::string RTFSprm::getName() const { return "RTFSprm"; }
#endif

#ifdef DBG_UTIL
std::string RTFSprm::toString() const
{
    OStringBuffer aBuf("RTFSprm");

    std::string sResult = QNameToString(m_nKeyword);

    aBuf.append(" ('");
    if (sResult.length() == 0)
        aBuf.append(sal_Int32(m_nKeyword));
    else
        aBuf.append(sResult.c_str());
    aBuf.append("', '" + m_pValue->toString() + "')");

    return std::string(aBuf);
}
#endif

namespace
{
class RTFSprms_compare
{
    Id m_keyword;

public:
    RTFSprms_compare(Id keyword)
        : m_keyword{ keyword }
    {
    }
    bool operator()(const std::pair<Id, RTFValue::Pointer_t>& raPair) const
    {
        return raPair.first == m_keyword;
    }
};
}

RTFValue::Pointer_t RTFSprms::find(Id nKeyword, bool bFirst, bool bForWrite)
{
    if (bForWrite)
        ensureCopyBeforeWrite();

    RTFSprms_compare cmp{ nKeyword };

    if (bFirst)
    {
        auto it = std::find_if(m_pSprms->begin(), m_pSprms->end(), cmp);
        if (it != m_pSprms->end())
            return it->second;
    }
    else
    // find last
    {
        auto rit = std::find_if(m_pSprms->rbegin(), m_pSprms->rend(), cmp);
        if (rit != m_pSprms->rend())
            return rit->second;
    }

    return RTFValue::Pointer_t{};
}

void RTFSprms::set(Id nKeyword, const RTFValue::Pointer_t& pValue, RTFOverwrite eOverwrite)
{
    ensureCopyBeforeWrite();

    switch (eOverwrite)
    {
        case RTFOverwrite::YES_PREPEND:
        {
            std::erase_if(*m_pSprms, RTFSprms_compare{ nKeyword });
            m_pSprms->emplace(m_pSprms->cbegin(), nKeyword, pValue);
            break;
        }
        case RTFOverwrite::YES:
        {
            auto it
                = std::find_if(m_pSprms->begin(), m_pSprms->end(), RTFSprms_compare{ nKeyword });
            if (it != m_pSprms->end())
                it->second = pValue;
            else
                m_pSprms->emplace_back(nKeyword, pValue);
            break;
        }
        case RTFOverwrite::NO_IGNORE:
        {
            if (std::none_of(m_pSprms->cbegin(), m_pSprms->cend(), RTFSprms_compare{ nKeyword }))
                m_pSprms->emplace_back(nKeyword, pValue);
            break;
        }
        case RTFOverwrite::NO_APPEND:
        {
            m_pSprms->emplace_back(nKeyword, pValue);
            break;
        }
    }
}

bool RTFSprms::erase(Id nKeyword)
{
    ensureCopyBeforeWrite();

    auto i = std::find_if(m_pSprms->begin(), m_pSprms->end(), RTFSprms_compare{ nKeyword });
    if (i != m_pSprms->end())
    {
        m_pSprms->erase(i);
        return true;
    }
    return false;
}

void RTFSprms::eraseLast(Id nKeyword)
{
    ensureCopyBeforeWrite();

    auto i = std::find_if(m_pSprms->rbegin(), m_pSprms->rend(), RTFSprms_compare{ nKeyword });
    if (i != m_pSprms->rend())
        m_pSprms->erase(std::next(i).base());
}

static RTFValue::Pointer_t getDefaultSPRM(Id const id, Id nStyleType)
{
    if (nStyleType == NS_ooxml::LN_Value_ST_StyleType_character)
    {
        switch (id)
        {
            case NS_ooxml::LN_EG_RPrBase_szCs:
            case NS_ooxml::LN_EG_RPrBase_sz:
                return new RTFValue(24);
            case NS_ooxml::LN_CT_Color_val:
                return new RTFValue(0);
            case NS_ooxml::LN_EG_RPrBase_b:
            case NS_ooxml::LN_EG_RPrBase_i:
                return new RTFValue(0);
            case NS_ooxml::LN_CT_Underline_val:
                return new RTFValue(NS_ooxml::LN_Value_ST_Underline_none);
            case NS_ooxml::LN_CT_Fonts_ascii:
            case NS_ooxml::LN_CT_Fonts_eastAsia:
            case NS_ooxml::LN_CT_Fonts_cs:
                return new RTFValue(u"Times New Roman"_ustr);
            default:
                break;
        }
    }

    if (!nStyleType || nStyleType == NS_ooxml::LN_Value_ST_StyleType_paragraph)
    {
        switch (id)
        {
            case NS_ooxml::LN_CT_Spacing_before:
            case NS_ooxml::LN_CT_Spacing_after:
            case NS_ooxml::LN_CT_Ind_left:
            case NS_ooxml::LN_CT_Ind_right:
            case NS_ooxml::LN_CT_Ind_firstLine:
                return new RTFValue(0);
            case NS_ooxml::LN_CT_Spacing_lineRule:
                return new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_auto);
            case NS_ooxml::LN_CT_Spacing_line:
                // presumably this means 100%, cf. static const int nSingleLineSpacing = 240;
                return new RTFValue(240);
            case NS_ooxml::LN_CT_NumPr_numId:
                return new RTFValue(0);
            case NS_ooxml::LN_CT_PrBase_pBdr:
            { // tdf#150382 default all paragraph borders to none
                RTFSprms attributes;
                RTFSprms sprms;
                for (int i = 0; i < 4; ++i)
                {
                    auto const nBorder = getParagraphBorder(i);
                    RTFSprms aAttributes;
                    RTFSprms aSprms;
                    aAttributes.set(NS_ooxml::LN_CT_Border_val,
                                    new RTFValue(NS_ooxml::LN_Value_ST_Border_none));
                    sprms.set(nBorder, new RTFValue(aAttributes, aSprms));
                }
                return new RTFValue(attributes, sprms);
            }

            default:
                break;
        }
    }

    return RTFValue::Pointer_t();
}

/// Is it problematic to deduplicate this SPRM?
static bool isSPRMDeduplicateDenylist(Id nId, RTFSprms* pDirect)
{
    switch (nId)
    {
        // See the NS_ooxml::LN_CT_PPrBase_tabs handler in DomainMapper,
        // deduplication is explicitly not wanted for these tokens.
        case NS_ooxml::LN_CT_TabStop_val:
        case NS_ooxml::LN_CT_TabStop_leader:
        case NS_ooxml::LN_CT_TabStop_pos:
        // \htmautsp arrives after the style table, so only the non-style value is
        // correct, keep these.
        case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
        case NS_ooxml::LN_CT_Spacing_afterAutospacing:
        // \chbrdr requires *all* of the border settings to be present,
        // otherwise a default (NONE) border is created from the removed
        // attributes which then overrides the style-defined border.
        // See BorderHandler.cxx and NS_ooxml::LN_EG_RPrBase_bdr in DomainMapper.
        // This also is needed for NS_ooxml::LN_CT_PBdr_top etc.
        case NS_ooxml::LN_CT_Border_sz:
        case NS_ooxml::LN_CT_Border_val:
        case NS_ooxml::LN_CT_Border_color:
        case NS_ooxml::LN_CT_Border_space:
        case NS_ooxml::LN_CT_Border_shadow:
        case NS_ooxml::LN_CT_Border_frame:
        case NS_ooxml::LN_CT_Border_themeTint:
        case NS_ooxml::LN_CT_Border_themeColor:
            return true;
        // Removing \fi and \li if the style has the same value would mean taking these values from
        // \ls, while deduplication would be done to take the values from the style.
        case NS_ooxml::LN_CT_Ind_firstLine:
        case NS_ooxml::LN_CT_Ind_left:
            return pDirect && pDirect->find(NS_ooxml::LN_CT_PPrBase_numPr);

        default:
            return false;
    }
}

/// Should this SPRM be removed if all its children are removed?
static bool isSPRMChildrenExpected(Id nId)
{
    switch (nId)
    {
        case NS_ooxml::LN_CT_PBdr_top:
        case NS_ooxml::LN_CT_PBdr_left:
        case NS_ooxml::LN_CT_PBdr_bottom:
        case NS_ooxml::LN_CT_PBdr_right:
            // Expected children are NS_ooxml::LN_CT_Border_*.
        case NS_ooxml::LN_CT_PrBase_shd:
            // Expected children are NS_ooxml::LN_CT_Shd_*.
        case NS_ooxml::LN_CT_PPrBase_ind:
            // Expected children are NS_ooxml::LN_CT_Ind_*.
            return true;

        default:
            return false;
    }
}

/// Does the clone / deduplication of a single sprm.
static void cloneAndDeduplicateSprm(std::pair<Id, RTFValue::Pointer_t> const& rSprm, RTFSprms& ret,
                                    Id nStyleType, RTFSprms* pDirect = nullptr)
{
    RTFValue::Pointer_t const pValue(ret.find(rSprm.first));
    if (pValue)
    {
        if (rSprm.second->equals(*pValue))
        {
            //this removes properties that are equal at the style and at the sprm
            //don't do that for paragraph styles
            if (nStyleType == NS_ooxml::LN_Value_ST_StyleType_character)
            {
                if (!isSPRMDeduplicateDenylist(rSprm.first, pDirect))
                {
                    ret.erase(rSprm.first); // duplicate to style
                }
            }
        }
        else if (!rSprm.second->getSprms().empty() || !rSprm.second->getAttributes().empty())
        {
            RTFSprms const sprms(pValue->getSprms().cloneAndDeduplicate(
                rSprm.second->getSprms(), nStyleType, /*bImplicitPPr =*/false, pDirect));
            RTFSprms const attributes(pValue->getAttributes().cloneAndDeduplicate(
                rSprm.second->getAttributes(), nStyleType, /*bImplicitPPr =*/false, pDirect));
            // Don't copy the sprm in case we expect it to have children but it doesn't have some.
            if (!isSPRMChildrenExpected(rSprm.first) || !sprms.empty() || !attributes.empty())
                ret.set(rSprm.first,
                        RTFValue::Pointer_t(pValue->CloneWithSprms(attributes, sprms)));
        }
    }
    else
    {
        // not found - try to override style with default
        RTFValue::Pointer_t const pDefault(getDefaultSPRM(rSprm.first, nStyleType));
        if (pDefault)
        {
            ret.set(rSprm.first, pDefault);
        }
        else if (!rSprm.second->getSprms().empty() || !rSprm.second->getAttributes().empty())
        {
            RTFSprms const sprms(
                RTFSprms().cloneAndDeduplicate(rSprm.second->getSprms(), nStyleType));
            RTFSprms const attributes(
                RTFSprms().cloneAndDeduplicate(rSprm.second->getAttributes(), nStyleType));
            if (!sprms.empty() || !attributes.empty())
            {
                ret.set(rSprm.first, new RTFValue(attributes, sprms));
            }
        }
    }
}

/// Extracts the list level matching nLevel from pAbstract.
static RTFValue::Pointer_t getListLevel(const RTFValue::Pointer_t& pAbstract, int nLevel)
{
    for (const auto& rPair : pAbstract->getSprms())
    {
        if (rPair.first != NS_ooxml::LN_CT_AbstractNum_lvl)
            continue;

        RTFValue::Pointer_t pLevel = rPair.second->getAttributes().find(NS_ooxml::LN_CT_Lvl_ilvl);
        if (!pLevel)
            continue;

        if (pLevel->getInt() != nLevel)
            continue;

        return rPair.second;
    }

    return RTFValue::Pointer_t();
}

void RTFSprms::deduplicateList(const std::map<int, int>& rInvalidListLevelFirstIndents)
{
    int nLevel = 0;
    RTFValue::Pointer_t pLevelId
        = getNestedSprm(*this, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_ilvl);
    if (pLevelId)
        nLevel = pLevelId->getInt();

    auto it = rInvalidListLevelFirstIndents.find(nLevel);
    if (it == rInvalidListLevelFirstIndents.end())
        return;

    int nListValue = it->second;

    RTFValue::Pointer_t pParagraphValue
        = getNestedAttribute(*this, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_firstLine);
    if (!pParagraphValue)
        return;

    int nParagraphValue = pParagraphValue->getInt();

    if (nParagraphValue == nListValue)
        eraseNestedAttribute(*this, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_firstLine);
}

void RTFSprms::duplicateList(const RTFValue::Pointer_t& pAbstract)
{
    int nLevel = 0;
    RTFValue::Pointer_t pLevelId
        = getNestedSprm(*this, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_ilvl);
    if (pLevelId)
        nLevel = pLevelId->getInt();

    RTFValue::Pointer_t pLevel = getListLevel(pAbstract, nLevel);
    if (!pLevel)
        return;

    RTFValue::Pointer_t pLevelInd = pLevel->getSprms().find(NS_ooxml::LN_CT_PPrBase_ind);
    if (!pLevelInd)
        return;

    for (const auto& rListLevelPair : pLevelInd->getAttributes())
    {
        switch (rListLevelPair.first)
        {
            case NS_ooxml::LN_CT_Ind_left:
            case NS_ooxml::LN_CT_Ind_right:
            case NS_ooxml::LN_CT_Ind_firstLine:
                RTFValue::Pointer_t pParagraphValue
                    = getNestedAttribute(*this, NS_ooxml::LN_CT_PPrBase_ind, rListLevelPair.first);
                if (!pParagraphValue)
                    putNestedAttribute(*this, NS_ooxml::LN_CT_PPrBase_ind, rListLevelPair.first,
                                       getDefaultSPRM(rListLevelPair.first, 0));

                break;
        }
    }
}

RTFSprms RTFSprms::cloneAndDeduplicate(RTFSprms& rReference, Id const nStyleType,
                                       bool const bImplicitPPr, RTFSprms* pDirect) const
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
        if (bImplicitPPr && rSprm.first == NS_ooxml::LN_CT_Style_pPr)
        {
            for (const auto& i : rSprm.second->getSprms())
                cloneAndDeduplicateSprm(i, ret, nStyleType, pDirect);
        }
        else
            cloneAndDeduplicateSprm(rSprm, ret, nStyleType, pDirect);
    }
    return ret;
}

bool RTFSprms::equals(const RTFSprms& rOther) const
{
    auto it1 = m_pSprms->cbegin();
    auto it1End = m_pSprms->cend();
    auto it2 = rOther.m_pSprms->cbegin();
    auto it2End = rOther.m_pSprms->cend();
    while (it1 != it1End && it2 != it2End)
    {
        if (it1->first != it2->first)
            return false;
        if (!it1->second->equals(*it2->second))
            return false;
        ++it1;
        ++it2;
    }
    return it1 == it1End && it2 == it2End;
}

void RTFSprms::ensureCopyBeforeWrite()
{
    if (m_pSprms->GetRefCount() > 1)
    {
        tools::SvRef<RTFSprmsImpl> pClone(new RTFSprmsImpl);
        for (auto& rSprm : *m_pSprms)
            pClone->push_back(
                std::make_pair(rSprm.first, RTFValue::Pointer_t(rSprm.second->Clone())));
        m_pSprms = pClone;
    }
}

RTFSprms::RTFSprms()
    : m_pSprms(new RTFSprmsImpl)
{
}

RTFSprms::~RTFSprms() = default;

void RTFSprms::clear()
{
    if (m_pSprms->GetRefCount() == 1)
        return m_pSprms->clear();

    m_pSprms = tools::SvRef<RTFSprmsImpl>(new RTFSprmsImpl);
}

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
