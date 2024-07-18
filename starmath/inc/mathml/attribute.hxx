/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "def.hxx"

/* All possible data needed to do the job outside mathml limits */
// Ml prefix means it is part of mathml standard
// NMl means it is not part of mathml standard but needed info to work

/* Union for storing the mathml attribute value */
/*************************************************************************************************/

union SmMlAttributeValue {
    SmMlAttributeValue(){};

    struct SmMlAccent m_aAccent;
    struct SmMlDir m_aDir;
    struct SmMlDisplaystyle m_aDisplaystyle;
    struct SmMlFence m_aFence;
    struct SmMlForm m_aForm;
    struct SmMlHref m_aHref;
    struct SmMlLspace m_aLspace;
    struct SmMlMathbackground m_aMathbackground;
    struct SmMlMathcolor m_aMathcolor;
    struct SmMlMathsize m_aMathsize;
    struct SmMlMathvariant m_aMathvariant;
    struct SmMlMaxsize m_aMaxsize;
    struct SmMlMinsize m_aMinsize;
    struct SmMlMovablelimits m_aMovablelimits;
    struct SmMlRspace m_aRspace;
    struct SmMlSeparator m_aSeparator;
    struct SmMlStretchy m_aStretchy;
    struct SmMlSymmetric m_aSymmetric;
};

/* Class managing the attribute value */
/*************************************************************************************************/

class SmMlAttribute
{
private:
    SmMlAttributeValueType m_aSmMlAttributeValueType;
    SmMlAttributeValue m_aAttributeValue;
    bool m_bSet;

private:
    void clearPreviousAttributeValue();
    void setDefaultAttributeValue();
    void setAttributeValue(const SmMlAttribute* aMlAttribute);

public:
    SmMlAttribute()
        : m_aSmMlAttributeValueType(SmMlAttributeValueType::NMlEmpty)
        , m_bSet(false){};

    ~SmMlAttribute() { clearPreviousAttributeValue(); };

    explicit SmMlAttribute(SmMlAttributeValueType)
        : m_aSmMlAttributeValueType(SmMlAttributeValueType::NMlEmpty)
        , m_bSet(false)
    {
        setDefaultAttributeValue();
    };

    SmMlAttribute(const SmMlAttribute& aMlAttribute)
        : m_aSmMlAttributeValueType(SmMlAttributeValueType::NMlEmpty)
        , m_bSet(aMlAttribute.isSet())
    {
        setAttributeValue(&aMlAttribute);
    }

    explicit SmMlAttribute(const SmMlAttribute* aMlAttribute)
        : m_aSmMlAttributeValueType(SmMlAttributeValueType::NMlEmpty)
        , m_bSet(aMlAttribute->isSet())
    {
        setAttributeValue(aMlAttribute);
    }

public:
    /** Check if the attribute has been set
    */
    bool isSet() const { return m_bSet; }

    /** Set if the attribute has been set
    */
    void setSet(bool bSet) { m_bSet = bSet; }

public:
    /**
      * Returns the type of attribute we are dealing with.
      * Attribute Value Type
      */
    SmMlAttributeValueType getMlAttributeValueType() const { return m_aSmMlAttributeValueType; };

    /**
      * Checks if the attribute contains information.
      * Attribute Value Type
      */
    bool isNullAttribute() const
    {
        return m_aSmMlAttributeValueType == SmMlAttributeValueType::NMlEmpty;
    };

    /**
      * Compares the type of attribute with a given one.
      * Attribute Value Type
      */
    bool isMlAttributeValueType(SmMlAttributeValueType aAttributeValueType) const
    {
        return m_aSmMlAttributeValueType == aAttributeValueType;
    };

    /**
      * Set the type of attribute we are dealing with.
      * @param Attribute Value Type
      */
    void setMlAttributeValueType(SmMlAttributeValueType aAttributeValueType)
    {
        clearPreviousAttributeValue();
        m_aSmMlAttributeValueType = aAttributeValueType;
        setDefaultAttributeValue();
    }

    void setMlAttributeValue(const SmMlAttribute& aMlAttribute)
    {
        m_bSet = true;
        setAttributeValue(&aMlAttribute);
    }

    void setMlAttributeValue(const SmMlAttribute* aMlAttribute)
    {
        m_bSet = true;
        setAttributeValue(aMlAttribute);
    }

public:
    // Get values
    const struct SmMlAccent* getMlAccent() const;
    const struct SmMlDir* getMlDir() const;
    const struct SmMlDisplaystyle* getMlDisplaystyle() const;
    const struct SmMlFence* getMlFence() const;
    const struct SmMlForm* getMlForm() const;
    const struct SmMlHref* getMlHref() const;
    const struct SmMlLspace* getMlLspace() const;
    const struct SmMlMathbackground* getMlMathbackground() const;
    const struct SmMlMathcolor* getMlMathcolor() const;
    const struct SmMlMathsize* getMlMathsize() const;
    const struct SmMlMathvariant* getMlMathvariant() const;
    const struct SmMlMaxsize* getMlMaxsize() const;
    const struct SmMlMinsize* getMlMinsize() const;
    const struct SmMlMovablelimits* getMlMovablelimits() const;
    const struct SmMlRspace* getMlRspace() const;
    const struct SmMlSeparator* getMlSeparator() const;
    const struct SmMlStretchy* getMlStretchy() const;
    const struct SmMlSymmetric* getMlSymmetric() const;

    // Set values
    // Note that content is copied.
    void setMlAccent(const SmMlAccent* aAccent);
    void setMlDir(const SmMlDir* aDir);
    void setMlDisplaystyle(const SmMlDisplaystyle* aDisplaystyle);
    void setMlFence(const SmMlFence* aFence);
    void setMlForm(const SmMlForm* aForm);
    void setMlHref(const SmMlHref* aHref);
    void setMlLspace(const SmMlLspace* aLspace);
    void setMlMathbackground(const SmMlMathbackground* aMathbackground);
    void setMlMathcolor(const SmMlMathcolor* aMathcolor);
    void setMlMathsize(const SmMlMathsize* aMathsize);
    void setMlMathvariant(const SmMlMathvariant* aMathvariant);
    void setMlMaxsize(const SmMlMaxsize* aMaxsize);
    void setMlMinsize(const SmMlMinsize* aMinSize);
    void setMlMovablelimits(const SmMlMovablelimits* aMovablelimits);
    void setMlRspace(const SmMlRspace* aRspace);
    void setMlSeparator(const SmMlSeparator* aSeparator);
    void setMlStretchy(const SmMlStretchy* aStretchy);
    void setMlSymmetric(const SmMlSymmetric* aSymmetric);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
