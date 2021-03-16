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

#include "def.hxx"

/* All possible data needed to do the job outside mathml limits */
// Ml prefix means it is part of mathml standar
// NMl means it is not part of mathml standar but needed info to work

/* Union for storing the mathml attribute value */
/*************************************************************************************************/

union SmMlAttributeValue {
    SmMlAttributeValue(){};

    struct SmMlAccent m_aAccent;
    struct SmMlDir m_aDir;
    struct SmMlDisplaystyle m_aDisplaystyle;
    struct SmMlFence m_aFence;
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

private:
    void clearPreviousAttributeValue();
    void setDefaultAttributeValue();
    void setAttributeValue(const SmMlAttribute* aMlAttribute);

public:
    SmMlAttribute() { m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty; };

    virtual ~SmMlAttribute() { clearPreviousAttributeValue(); };

    SmMlAttribute(SmMlAttributeValueType)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setDefaultAttributeValue();
    };

    SmMlAttribute(const SmMlAttribute& aMlAttribute)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setAttributeValue(&aMlAttribute);
    }

    SmMlAttribute(const SmMlAttribute* aMlAttribute)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setAttributeValue(aMlAttribute);
    }

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
        m_aSmMlAttributeValueType = aAttributeValueType;
        setDefaultAttributeValue();
    }

    void setMlAttributeValue(const SmMlAttribute& aMlAttribute)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setAttributeValue(&aMlAttribute);
    }

    void setMlAttributeValue(const SmMlAttribute* aMlAttribute)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setAttributeValue(aMlAttribute);
    }

public:
    // Get values
    const struct SmMlAccent* getMlAccent() const;
    const struct SmMlDir* getMlDir() const;
    const struct SmMlDisplaystyle* getMlDisplaystyle() const;
    const struct SmMlFence* getMlFence() const;
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
    void setMlAccent(const SmMlAccent* getAccent);
    void setMlDir(const SmMlDir* getDir);
    void setMlDisplaystyle(const SmMlDisplaystyle* getDisplaystyle);
    void setMlFence(const SmMlFence* getFence);
    void setMlHref(const SmMlHref* getHref);
    void setMlLspace(const SmMlLspace* getLspace);
    void setMlMathbackground(const SmMlMathbackground* getMathbackground);
    void setMlMathcolor(const SmMlMathcolor* getMathcolor);
    void setMlMathsize(const SmMlMathsize* getMathsize);
    void setMlMathvariant(const SmMlMathvariant* getMathvariant);
    void setMlMaxsize(const SmMlMaxsize* getMaxsize);
    void setMlMinsize(const SmMlMinsize* getMinSize);
    void setMlMovablelimits(const SmMlMovablelimits* getMovablelimits);
    void setMlRspace(const SmMlRspace* getRspace);
    void setMlSeparator(const SmMlSeparator* getSeparator);
    void setMlStretchy(const SmMlStretchy* getStretchy);
    void setMlSymmetric(const SmMlSymmetric* getSymmetric);
};

/* order attributes */
/*************************************************************************************************/

struct SmMlAttributePos
{
    SmMlAttributeValueType m_aAttributeValueType;
    uint_fast8_t m_nPos;
};

namespace starmathdatabase
{
extern SmMlAttributePos MlAttributeListEmpty[];
extern SmMlAttributePos MlAttributeListMath[];
extern SmMlAttributePos MlAttributeListMi[];
extern SmMlAttributePos MlAttributeListMerror[];
extern SmMlAttributePos MlAttributeListMn[];
extern SmMlAttributePos MlAttributeListMo[];
extern SmMlAttributePos MlAttributeListMrow[];
extern SmMlAttributePos MlAttributeListMtext[];
extern SmMlAttributePos MlAttributeListMstyle[];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
