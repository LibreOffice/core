/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mathml/element.hxx>

void SmMlElement::SmImplAttributeType()
{
    switch (m_aElementType)
    {
        case SmMlElementType::NMlEmpty:
            m_aAttributePosList = std::vector<SmMlAttributePos>(0);
            break;
        case SmMlElementType::NMlStructural:
            m_aAttributePosList = std::vector<SmMlAttributePos>(0);
            break;
        case SmMlElementType::NMlSmNode:
            m_aAttributePosList = std::vector<SmMlAttributePos>(0);
            break;
        case SmMlElementType::MlMath:
            m_aAttributePosList = std::vector<SmMlAttributePos>(0);
            //m_aAttributePosList = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMath), std::end(starmathdatabase::MlAttributeListMath));
            break;
        case SmMlElementType::MlMi:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMi),
                                                std::end(starmathdatabase::MlAttributeListMi));
            break;
        case SmMlElementType::MlMerror:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMerror),
                                                std::end(starmathdatabase::MlAttributeListMerror));
            break;
        case SmMlElementType::MlMn:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMn),
                                                std::end(starmathdatabase::MlAttributeListMn));
            break;
        case SmMlElementType::MlMo:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMo),
                                                std::end(starmathdatabase::MlAttributeListMo));
            break;
        case SmMlElementType::MlMrow:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMrow),
                                                std::end(starmathdatabase::MlAttributeListMrow));
            break;
        case SmMlElementType::MlMtext:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMtext),
                                                std::end(starmathdatabase::MlAttributeListMtext));
            break;
        case SmMlElementType::MlMstyle:
            m_aAttributePosList
                = std::vector<SmMlAttributePos>(std::begin(starmathdatabase::MlAttributeListMstyle),
                                                std::end(starmathdatabase::MlAttributeListMstyle));
            break;
        default:
            break;
    }
    // Create attribute vector with given pattern
    m_aAttributeList = starmathdatabase::makeMlAttributeList(m_aAttributePosList);
}

SmMlAttribute SmMlElement::getAttribute(SmMlAttributeValueType aAttributeType) const
{
    // Look for the attribute position and return if exists
    for (size_t i = 0; i < m_aAttributePosList.size(); ++i)
    {
        if (m_aAttributePosList[i].m_aAttributeValueType == aAttributeType)
            return m_aAttributeList[m_aAttributePosList[i].m_nPos];
    }
    return SmMlAttribute();
}

bool SmMlElement::isAttributeSet(SmMlAttributeValueType aAttributeType) const
{
    // Look for the attribute position and return if exists
    for (size_t i = 0; i < m_aAttributePosList.size(); ++i)
    {
        if (m_aAttributePosList[i].m_aAttributeValueType == aAttributeType)
            return m_aAttributeList[m_aAttributePosList[i].m_nPos].isSet();
    }
    return false;
}

void SmMlElement::setAttribute(const SmMlAttribute* aAttribute)
{
    // Look for the attribute position and assign if exists
    for (size_t i = 0; i < m_aAttributePosList.size(); ++i)
    {
        if (m_aAttributePosList[i].m_aAttributeValueType == aAttribute->getMlAttributeValueType())
        {
            m_aAttributeList[m_aAttributePosList[i].m_nPos].setMlAttributeValue(aAttribute);
            break;
        }
    }
}

void SmMlElement::setSubElement(size_t nPos, SmMlElement* aElement)
{
    // This is the new parent element
    aElement->setParentElement(this);
    aElement->setSubElementId(nPos);
    // Check if the vector is long enough
    // Careful nOldSize can be 0 and -1 will underflow
    // We must put something on the empty locations
    size_t nOldSize = m_aSubElements.size();
    if (nPos + 1 > nOldSize)
    {
        m_aSubElements.resize(nPos + 1);
        for (; nOldSize < nPos; ++nOldSize)
            m_aSubElements[nOldSize] = nullptr;
    }
    // Assign value
    m_aSubElements[nPos] = aElement;
}

std::vector<SmMlAttribute>
starmathdatabase::makeMlAttributeList(std::vector<SmMlAttributePos> aAttributePosList)
{
    std::vector<SmMlAttribute> aAttributeList(aAttributePosList.size());
    for (size_t i = 0; i < aAttributePosList.size(); ++i)
    {
        aAttributeList[i].setMlAttributeValueType(aAttributePosList[i].m_aAttributeValueType);
    }
    return aAttributeList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
