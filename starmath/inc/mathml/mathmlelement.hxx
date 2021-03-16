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

#pragma once

#include "mathmldef.hxx"
#include <rect.hxx>

#include <editeng/editdata.hxx>

class SmMlElement : public SmRect
{
    /* Technical stuff */

public:
    SmMlElement()
        : m_aElementType(SmMlElementType::NMlEmpty){};

    /* Mathml stuff */

protected:
    SmMlElement(SmMlElementType aElementType)
        : m_aElementType(aElementType){};

private:
    // Type of element
    SmMlElementType m_aElementType;

    // Location in source code
    ESelection m_aESelection;

    // Attribute list
    std::vector<SmMlAttribute> aAttributeList;

public:
    /*
     * Returns the mathml element type
     * @return mathml element type
     */
    SmMlElementType getMlElementType() const { return m_aElementType; };

    /*
     * Check if the mathml element is of a given type
     * @param aElementType
     * @return is mathml element type
     */
    bool isMlElementType(SmMlElementType aElementType) const
    {
        return m_aElementType == aElementType;
    };

    /**
      * Returns the location in the source code of the node type
      * @return selection
      */
    ESelection getESelection() const { return m_aESelection; };

    /**
      * Sets the location in the source code of the node type
      * @param aESelection
      */
    void setESelection(ESelection aESelection) { m_aESelection = aESelection; };

    /*
     * Returns the amount of available attributes
     * @return attribute count
     */
    size_t getAttributeCount() const { return aAttributeList.size(); };

    /*
     * Get's a given attribute.
     * If no available returns empty attribute.
     * @param nAttributePos
     * @return given attribute.
     */
    SmMlAttribute getAttribute(size_t nAttributePos) const
    {
        return nAttributePos > aAttributeList.size() ? SmMlAttribute(aAttributeList[nAttributePos])
                                                     : SmMlAttribute();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
