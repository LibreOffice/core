/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "attribute.hxx"
#include <rect.hxx>

#include <editeng/editdata.hxx>

class SmMlElement : public SmRect
{
    /* Technical stuff */

public:
    SmMlElement()
        : m_aElementType(SmMlElementType::NMlEmpty)
        , m_aText(u"")
        , m_aESelection(0, 0, 0, 0)
        , m_aAttributeList(0)
        , m_aAttributePosList(0)
        , m_aSubElements(0)
        , m_aParentElement(nullptr)
    {
        SmImplAttributeType();
    };
    /* Mathml stuff */

protected:
    SmMlElement(SmMlElementType aElementType)
        : m_aElementType(aElementType)
        , m_aText(u"\u00B6")
        , m_aESelection(0, 0, 0, 0)
        , m_aSubElements(0)
        , m_aParentElement(nullptr)
    {
        SmImplAttributeType();
    };

private:
    // Type of element
    SmMlElementType m_aElementType;

    // Element text
    OUString m_aText;

    // Location in source code
    ESelection m_aESelection;

    // Attribute list
    std::vector<SmMlAttribute> m_aAttributeList;

    // Attribute position list
    std::vector<SmMlAttributePos> m_aAttributePosList;

    // Sub elements
    std::vector<SmMlElement*> m_aSubElements;

    // Parent element
    SmMlElement* m_aParentElement;

private:
    void SmImplAttributeType();

public: // Element type
    /**
      * Returns the mathml element type
      * @return mathml element type
      */
    SmMlElementType getMlElementType() const { return m_aElementType; };

    /**
      * Check if the mathml element is of a given type
      * @param aElementType
      * @return is mathml element type
      */
    bool isMlElementType(SmMlElementType aElementType) const
    {
        return m_aElementType == aElementType;
    };

public: // location in the source
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

    /**
      * Gets the line in the text where the node is located.
      * It is used to do the visual <-> text correspondence.
      * @return line
      */
    sal_Int32 GetSourceCodeRow() const { return m_aESelection.nStartPara; }

    /**
      * Gets the column of the line in the text where the node is located.
      * It is used to do the visual <-> text correspondence.
      * @return column
      */
    sal_Int32 GetSourceCodeColumn() const { return m_aESelection.nStartPos; }

public: // attributes
    /**
      * Returns the amount of available attributes
      * @return attribute count
      */
    size_t getAttributeCount() const { return m_aAttributeList.size(); };

    /**
      * Get's a given attribute.
      * If no available returns empty attribute.
      * @param nAttributePos
      * @return given attribute.
      */
    SmMlAttribute getAttribute(size_t nAttributePos) const
    {
        return nAttributePos < m_aAttributeList.size() ? m_aAttributeList[nAttributePos]
                                                       : SmMlAttribute();
    }

    /**
      * Get's a given attribute.
      * If no available returns empty attribute.
      * @param nAttributePos
      * @return given attribute.
      */
    SmMlAttribute getAttribute(SmMlAttributeValueType aElementType) const;

    /**
      * Set's a given attribute.
      * If no available does nothing.
      * @param nAttributePos
      * @return given attribute.
      */
    void setAttribute(const SmMlAttribute* aAttribute);

public: // sub elements
    /**
      * Returns the sub elements count
      * @return sub elements count
      */
    size_t getSubElementsCount() const { return m_aSubElements.size(); };

    /**
      * Returns a given sub element
      * @param nPos
      * @return sub elements
      */
    SmMlElement* getSubElement(size_t nPos)
    {
        return nPos < m_aSubElements.size() ? m_aSubElements[nPos] : nullptr;
    };

    /**
      * Returns a given sub element
      * @param nPos
      * @return sub elements
      */
    const SmMlElement* getSubElement(size_t nPos) const
    {
        return nPos < m_aSubElements.size() ? m_aSubElements[nPos] : nullptr;
    };

    /**
      * Sets a given sub element
      * @param nPos
      * @param aElement
      */
    void setSubElement(size_t nPos, SmMlElement* aElement);

public: // parent elements
    /**
      * Returns the parent element
      * @return parent element
      */
    SmMlElement* getParentElement() { return m_aParentElement; };

    /**
      * Returns the parent element
      * @return parent element
      */
    const SmMlElement* getParentElement() const { return m_aParentElement; };

    /**
      * Sets the parent element
      * No allocation / free is done.
      * @param aParentElement
      */
    void setParentElement(SmMlElement* aParentElement) { m_aParentElement = aParentElement; };

public: // text elements
    /**
      * Returns the element text
      */
    OUString getText() const { return m_aText; };

    /**
      * Returns the element text
      */
    void setText(OUString aText) { m_aText = aText; };
};

namespace starmathdatabase
{
/**
  * Generates an attribute vector of default values from an attribute position list.
  * @param aAttributePosList
  * @return attribute vector
  */
std::vector<SmMlAttribute> makeMlAttributeList(std::vector<SmMlAttributePos> aAttributePosList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
