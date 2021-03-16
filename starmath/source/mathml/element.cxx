/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
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
}

SmMlAttribute SmMlElement::getAttribute(SmMlAttributeValueType aElementType) const
{
    for (uint_fast8_t i = 0; i < m_aAttributePosList.size(); ++i)
    {
        if (m_aAttributePosList[i].m_aAttributeValueType == aElementType)
            return m_aAttributeList[m_aAttributePosList[i].m_nPos];
    }
    return SmMlAttribute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
