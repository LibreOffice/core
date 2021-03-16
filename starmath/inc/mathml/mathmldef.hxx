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

#include <tools/color.hxx>

#pragma once

/* All possible data needed to do the job outside mathml limits */
/* For now empty, don't know yet what's needed besides default font size. */
struct SmGlobalData
{
};

/* Possible mathml attributes */
// Ml prefix means it is part of mathml standar
// NMl means it is not part of mathml standar but needed info to work
/*************************************************************************************************/

enum class SmAttributeValueType : uint_fast8_t
{
    Empty,
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
    MlMaxize,
    MlMinsize,
    MlMovablelimits,
    MlRspace,
    MlSeparator,
    MlStretchy,
    MlSymmetric
};

/* Possible values of mathml attributes */
/*************************************************************************************************/

enum class SmAttributeValueEmpty : uint_fast8_t
{
    MlEmpty = 0x00
};

enum class SmAttributeValueAccent : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueDir : uint_fast8_t
{
    MlLtr = 0x00,
    MlRtl = 0x01
};

enum class SmAttributeValueDisplaystyle : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueFence : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueHref : uint_fast8_t
{
    NMlEmpty = 0x00,
    NMlValie = 0x01
};

enum class SmAttributeValueLspace : uint_fast8_t
{
};

enum class SmAttributeValueMathbackground : uint_fast32_t
{
    MlTransparent = 0x00,
    MlRgb = 0x01
};

enum class SmAttributeValueMathcolor : uint_fast8_t
{
    MlDefault = 0x00,
    MlRgb = 0x01
};

enum class SmAttributeValueMathsize : uint_fast8_t
{
};

enum class SmAttributeValueMathvariant : uint_fast16_t
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

enum class SmAttributeValueMaxize : uint_fast8_t
{
};

enum class SmAttributeValueMinsize : uint_fast8_t
{
};

/*
 * Specifies whether attached under- and overscripts move to sub- and superscript positions when displaystyle is false.
 * Source: https://developer.mozilla.org/en-US/docs/Web/MathML/Element/mo
 */
enum class SmAttributeValueMovablelimits : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueRspace : uint_fast8_t
{
};

enum class SmAttributeValueSeparator : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueStretchy : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

enum class SmAttributeValueSymmetric : uint_fast8_t
{
    MlFalse = 0x00,
    MlTrue = 0x01
};

/* Structures for all possible attributes */
/*************************************************************************************************/

struct SmMlAccent
{
    SmAttributeValueAccent m_aAccent;
};

struct SmMlDir
{
    SmAttributeValueDir m_aDir;
};

struct SmMlDisplaystyle
{
    SmAttributeValueDisplaystyle m_aDisplaystyle;
};

struct SmMlFence
{
    SmAttributeValueFence m_aFence;
};

struct SmMlHref
{
    SmAttributeValueHref m_aHref;
    OUString* m_aLnk;
};

struct SmMlLspace
{
};

struct SmMlMathbackground
{
    SmAttributeValueMathbackground m_aMathbackground;
    Color m_aCol;
};

struct SmMlMathcolor
{
    SmAttributeValueMathcolor m_aMathcolor;
    Color m_aCol;
};

struct SmMlMathsize
{
};

struct SmMlMathvariant
{
    SmAttributeValueMathvariant m_aMathvariant;
};

struct SmMlMaxsize
{
};

struct SmMlMinsize
{
};

struct SmMlMovablelimits
{
    SmAttributeValueMovablelimits m_aMovablelimits;
};

struct SmMlRspace
{
};

struct SmMlSeparator
{
    SmAttributeValueSeparator m_aSeparator;
};

struct SmMlStretchy
{
    SmAttributeValueStretchy m_aStretchy;
};

struct SmMlSymmetric
{
    SmAttributeValueSymmetric m_aSymmetric;
};

/* Union for storing the mathml attribute value */
/*************************************************************************************************/

union SmMlAttributeValue {
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
    struct SmMlMinsize m_aMinSize;
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
    SmAttributeValueType m_aSmAttributeValueType;
    SmMlAttributeValue m_aAttributeValue;

private:
    void clearPreviousAttributeValueType();

public:
    /**
      * Returns the type of attribute we are dealing with.
      * Attribute Value Type
      */
    SmAttributeValueType getAttributeValueType() const { return m_aSmAttributeValueType; };

    /**
      * Set the type of attribute we are dealing with.
      * @param Attribute Value Type
      */
    void setAttributeValueType(SmAttributeValueType aAttributeValueType)
    {
        clearPreviousAttributeValueType();
        m_aSmAttributeValueType = aAttributeValueType;
    };

    // Get values
    const struct SmMlAccent* getAccent() const;
    const struct SmMlDir* getDir() const;
    const struct SmMlDisplaystyle* getDisplaystyle() const;
    const struct SmMlFence* getFence() const;
    const struct SmMlHref* getHref() const;
    const struct SmMlLspace* getLspace() const;
    const struct SmMlMathbackground* getMathbackground() const;
    const struct SmMlMathcolor* getMathcolor() const;
    const struct SmMlMathsize* getMathsize() const;
    const struct SmMlMathvariant* getMathvariant() const;
    const struct SmMlMaxsize* getMaxsize() const;
    const struct SmMlMinsize* getMinSize() const;
    const struct SmMlMovablelimits* getMovablelimits() const;
    const struct SmMlRspace* getRspace() const;
    const struct SmMlSeparator* getSeparator() const;
    const struct SmMlStretchy* getStretchy() const;
    const struct SmMlSymmetric* getSymmetric() const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
