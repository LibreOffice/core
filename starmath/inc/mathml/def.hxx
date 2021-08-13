/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/color.hxx>

/* All possible data needed to do the job outside mathml limits */
// Ml prefix means it is part of mathml standard
// NMl means it is not part of mathml standard but needed info to work

/* For now empty, don't know yet what's needed besides default font size. */
struct SmGlobalData
{
};

/* Mhtml length tools */
/*************************************************************************************************/

enum class SmLengthUnit : uint_fast8_t
{
    MlEm,
    MlEx,
    MlPx,
    MlIn,
    MlCm,
    MlMm,
    MlPt,
    MlPc,
    MlP, // Percent
    MlM // Multiplier
};

struct SmLengthValue
{
    SmLengthUnit m_aLengthUnit;
    double m_aLengthValue;
    // Keeps original text value to avoid numerical error data loss
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
    NMlSmNode,
    // Mathml real elements
    MlMath,
    MlMi,
    MlMerror,
    MlMn,
    MlMo,
    MlMrow,
    MlMtext,
    MlMstyle
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
    MlForm,
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

enum class SmMlAttributeValueForm : uint_fast8_t
{
    MlPrefix = 0x01,
    MlInfix = 0x02,
    MlPosfix = 0x04
};

enum class SmMlAttributeValueHref : uint_fast8_t
{
    NMlEmpty = 0x00,
    NMlValid = 0x01
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
    sans_serif_italic = 0x002 | 0x20,
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

struct SmMlForm
{
    SmMlAttributeValueForm m_aForm;
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

/* order attributes */
/*************************************************************************************************/

struct SmMlAttributePos
{
    SmMlAttributeValueType m_aAttributeValueType;
    uint_fast8_t m_nPos;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
