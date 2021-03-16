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

#include <tools/color.hxx>

/* All possible data needed to do the job outside mathml limits */
// Ml prefix means it is part of mathml standar
// NMl means it is not part of mathml standar but needed info to work

/* For now empty, don't know yet what's needed besides default font size. */
struct SmGlobalData
{
};

/* Mthml length tools */
/*************************************************************************************************/

enum class SmLengthUnit : uint_fast8_t
{
    MlEm,
    MlEx,
    MlPx,
    MlIn,
    MlCm,
    MlMM,
    MlPt,
    MlPc,
    MlP // Percent
};

struct SmLengthValue
{
    SmLengthUnit m_aLengthUnit;
    double m_aLengthValue;
    // Keeps original text value to avoid numerial error data loss
    OUString* m_aOriginalText;
};

/* Possible mathml elements */
/*************************************************************************************************/

enum class SmMlElementType : uint_fast8_t
{
    // Used for base element. Means no information contained.
    NMlEmpty,
    // Used for structural dependencies. Means no information contained.
    NMlStructural,
    // Mathml real elements
    MlMath,
    MlMi,
    MlMerror,
    MlMn,
    MlMo,
    MlMrow,
    MlMtext
};

/* Possible mathml attributes */
/*************************************************************************************************/

enum class SmMlAttributeValueType : uint_fast8_t
{
    NMlEmpty,
    MlAccent,
    MlDir,
    MlDisplaystyle,
    MlFence,
    MlHref,
    MlLspace,
    MlMathbackground,
    MlMathcolor,
    MlMathsize,
    MlMathvariant,
    MlMaxsize,
    MlMinsize,
    MlMovablelimits,
    MlRspace,
    MlSeparator,
    MlStretchy,
    MlSymmetric
};

/* Possible values of mathml attributes */
/*************************************************************************************************/

enum class SmMlAttributeValueEmpty : uint_fast8_t
{
    MlEmpty = 0x00
};

enum class SmMlAttributeValueAccent : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueDir : uint_fast8_t
{
    MlLtr = 0x00,
    MlRtl = 0x01
};

enum class SmMlAttributeValueDisplaystyle : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueFence : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueHref : uint_fast8_t
{
    NMlEmpty = 0x00,
    NMlValie = 0x01
};

enum class SmMlAttributeValueLspace : uint_fast8_t
{
    NMlEmpty = 0x00
};

enum class SmMlAttributeValueMathbackground : uint_fast32_t
{
    MlTransparent = 0x00,
    MlRgb = 0x01
};

enum class SmMlAttributeValueMathcolor : uint_fast8_t
{
    MlDefault = 0x00,
    MlRgb = 0x01
};

enum class SmMlAttributeValueMathsize : uint_fast8_t
{
    NMlEmpty = 0x00,
};

enum class SmMlAttributeValueMathvariant : uint_fast16_t
{
    normal = 0x000,
    bold = 0x001,
    italic = 0x002,
    double_struck = 0x004,
    script = 0x008,
    fraktur = 0x010,
    sans_serif = 0x020,
    monospace = 0x040,
    bold_italic = 0x001 | 0x002,
    bold_fraktur = 0x001 | 0x010,
    bold_script = 0x001 | 0x008,
    bold_sans_serif = 0x001 | 0x020,
    sans_serif_italic = 0x001 | 0x002 | 0x20,
    sans_serif_bold_italic = 0x001 | 0x002 | 0x020,
    // Non english
    initial = 0x080,
    tailed = 0x100,
    looped = 0x200,
    stretched = 0x400
};

enum class SmMlAttributeValueMaxsize : uint_fast8_t
{
    MlInfinity = 0x00,
    MlFinite = 0x01
};

enum class SmMlAttributeValueMinsize : uint_fast8_t
{
    MlInfinity = 0x00,
    MlFinite = 0x01
};

/*
 * Specifies whether attached under- and overscripts move to sub- and superscript positions when displaystyle is false.
 * Source: https://developer.mozilla.org/en-US/docs/Web/MathML/Element/mo
 */
enum class SmMlAttributeValueMovablelimits : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueRspace : uint_fast8_t
{
    NMlEmpty = 0x00
};

enum class SmMlAttributeValueSeparator : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueStretchy : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmMlAttributeValueSymmetric : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

/* Structures for all possible attributes */
/*************************************************************************************************/

struct SmMlAccent
{
    SmMlAttributeValueAccent m_aAccent;
};

struct SmMlDir
{
    SmMlAttributeValueDir m_aDir;
};

struct SmMlDisplaystyle
{
    SmMlAttributeValueDisplaystyle m_aDisplaystyle;
};

struct SmMlFence
{
    SmMlAttributeValueFence m_aFence;
};

struct SmMlHref
{
    SmMlAttributeValueHref m_aHref;
    OUString* m_aLnk;
};

struct SmMlLspace
{
    SmLengthValue m_aLengthValue;
};

struct SmMlMathbackground
{
    SmMlAttributeValueMathbackground m_aMathbackground;
    Color m_aCol;
};

struct SmMlMathcolor
{
    SmMlAttributeValueMathcolor m_aMathcolor;
    Color m_aCol;
};

struct SmMlMathsize
{
    SmLengthValue m_aLengthValue;
};

struct SmMlMathvariant
{
    SmMlAttributeValueMathvariant m_aMathvariant;
};

struct SmMlMaxsize
{
    SmMlAttributeValueMaxsize m_aMaxsize;
    SmLengthValue m_aLengthValue;
};

struct SmMlMinsize
{
    SmMlAttributeValueMinsize m_aMinsize;
    SmLengthValue m_aLengthValue;
};

struct SmMlMovablelimits
{
    SmMlAttributeValueMovablelimits m_aMovablelimits;
};

struct SmMlRspace
{
    SmLengthValue m_aLengthValue;
};

struct SmMlSeparator
{
    SmMlAttributeValueSeparator m_aSeparator;
};

struct SmMlStretchy
{
    SmMlAttributeValueStretchy m_aStretchy;
};

struct SmMlSymmetric
{
    SmMlAttributeValueSymmetric m_aSymmetric;
};

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
    void clearPreviousAttributeValueType();
    void setDefaultAttributeValueType();
    void setAttributeValueType(const SmMlAttribute& aMlAttribute);

public:
    SmMlAttribute() { m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty; };

    virtual ~SmMlAttribute() { clearPreviousAttributeValueType(); };

    SmMlAttribute(SmMlAttributeValueType)
    {
        m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
        setDefaultAttributeValueType();
    };

    SmMlAttribute(const SmMlAttribute& aMlAttribute);

public:
    /**
      * Returns the type of attribute we are dealing with.
      * Attribute Value Type
      */
    SmMlAttributeValueType getMlAttributeValueType() const { return m_aSmMlAttributeValueType; };

    /**
      * Set the type of attribute we are dealing with.
      * @param Attribute Value Type
      */
    void setMlAttributeValueType(SmMlAttributeValueType aAttributeValueType);

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
    const struct SmMlMinsize* getMlMinSize() const;
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
    void setMaxsize(const SmMlMaxsize* getMaxsize);
    void setMinsize(const SmMlMinsize* getMinSize);
    void setMovablelimits(const SmMlMovablelimits* getMovablelimits);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
