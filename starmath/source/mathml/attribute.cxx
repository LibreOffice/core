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

#include <mathml/attribute.hxx>

void SmMlAttribute::clearPreviousAttributeValue()
{
    switch (m_aSmMlAttributeValueType)
    {
        case SmMlAttributeValueType::NMlEmpty:
            break;
        case SmMlAttributeValueType::MlHref:
            if (m_aAttributeValue.m_aHref.m_aLnk)
                delete m_aAttributeValue.m_aHref.m_aLnk;
            break;
        case SmMlAttributeValueType::MlLspace:
            if (m_aAttributeValue.m_aLspace.m_aLengthValue.m_aOriginalText)
                delete m_aAttributeValue.m_aLspace.m_aLengthValue.m_aOriginalText;
            break;
        case SmMlAttributeValueType::MlMathsize:
            if (m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aOriginalText)
                delete m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aOriginalText;
            break;
        case SmMlAttributeValueType::MlMaxsize:
            if (m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aOriginalText)
                delete m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aOriginalText;
            break;
        case SmMlAttributeValueType::MlMinsize:
            if (m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aOriginalText)
                delete m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aOriginalText;
            break;
        case SmMlAttributeValueType::MlRspace:
            if (m_aAttributeValue.m_aRspace.m_aLengthValue.m_aOriginalText)
                delete m_aAttributeValue.m_aRspace.m_aLengthValue.m_aOriginalText;
            break;
        default:
            break;
    }
}

void SmMlAttribute::setDefaultAttributeValue()
{
    switch (m_aSmMlAttributeValueType)
    {
        case SmMlAttributeValueType::NMlEmpty:
            break;
        case SmMlAttributeValueType::MlAccent:
            m_aAttributeValue.m_aAccent.m_aAccent = SmMlAttributeValueAccent::MlFalse;
            break;
        case SmMlAttributeValueType::MlDir:
            m_aAttributeValue.m_aDir.m_aDir = SmMlAttributeValueDir::MlLtr;
            break;
        case SmMlAttributeValueType::MlDisplaystyle:
            m_aAttributeValue.m_aDisplaystyle.m_aDisplaystyle
                = SmMlAttributeValueDisplaystyle::MlFalse;
            break;
        case SmMlAttributeValueType::MlFence:
            m_aAttributeValue.m_aFence.m_aFence = SmMlAttributeValueFence::MlFalse;
            break;
        case SmMlAttributeValueType::MlHref:
            m_aAttributeValue.m_aHref.m_aHref = SmMlAttributeValueHref::NMlEmpty;
            m_aAttributeValue.m_aHref.m_aLnk = new OUString(u"");
            break;
        case SmMlAttributeValueType::MlLspace:
            m_aAttributeValue.m_aLspace.m_aLengthValue.m_aLengthUnit = SmLengthUnit::MlEm;
            m_aAttributeValue.m_aLspace.m_aLengthValue.m_aLengthValue = 5.0 / 18;
            m_aAttributeValue.m_aLspace.m_aLengthValue.m_aOriginalText = new OUString(u"5/18em");
            break;
        case SmMlAttributeValueType::MlMathbackground:
            m_aAttributeValue.m_aMathbackground.m_aMathbackground
                = SmMlAttributeValueMathbackground::MlTransparent;
            break;
        case SmMlAttributeValueType::MlMathcolor:
            m_aAttributeValue.m_aMathcolor.m_aMathcolor = SmMlAttributeValueMathcolor::MlDefault;
            break;
        case SmMlAttributeValueType::MlMathsize:
            m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aLengthUnit = SmLengthUnit::MlP;
            m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aLengthValue = 100;
            m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aOriginalText = new OUString(u"100%");
            break;
        case SmMlAttributeValueType::MlMathvariant:
            m_aAttributeValue.m_aMathvariant.m_aMathvariant = SmMlAttributeValueMathvariant::normal;
            break;
        case SmMlAttributeValueType::MlMaxsize:
            m_aAttributeValue.m_aMaxsize.m_aMaxsize = SmMlAttributeValueMaxsize::MlInfinity;
            m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aLengthUnit = SmLengthUnit::MlP;
            m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aLengthValue = 10000;
            m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aOriginalText = new OUString(u"10000%");
            break;
        case SmMlAttributeValueType::MlMinsize:
            m_aAttributeValue.m_aMinsize.m_aMinsize = SmMlAttributeValueMinsize::MlInfinity;
            m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aLengthUnit = SmLengthUnit::MlP;
            m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aLengthValue = 1;
            m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aOriginalText = new OUString(u"1%");
            break;
        case SmMlAttributeValueType::MlMovablelimits:
            m_aAttributeValue.m_aMovablelimits.m_aMovablelimits
                = SmMlAttributeValueMovablelimits::MlFalse;
            break;
        case SmMlAttributeValueType::MlRspace:
            m_aAttributeValue.m_aRspace.m_aLengthValue.m_aLengthUnit = SmLengthUnit::MlEm;
            m_aAttributeValue.m_aRspace.m_aLengthValue.m_aLengthValue = 5.0 / 18;
            m_aAttributeValue.m_aRspace.m_aLengthValue.m_aOriginalText = new OUString(u"5/18em");
            break;
        case SmMlAttributeValueType::MlSeparator:
            m_aAttributeValue.m_aSeparator.m_aSeparator = SmMlAttributeValueSeparator::MlFalse;
            break;
        case SmMlAttributeValueType::MlStretchy:
            m_aAttributeValue.m_aStretchy.m_aStretchy = SmMlAttributeValueStretchy::MlFalse;
            break;
        case SmMlAttributeValueType::MlSymmetric:
            m_aAttributeValue.m_aSymmetric.m_aSymmetric = SmMlAttributeValueSymmetric::MlFalse;
            break;
    }
}

void SmMlAttribute::setAttributeValue(const SmMlAttribute* aAttribute)
{
    switch (aAttribute->getMlAttributeValueType())
    {
        case SmMlAttributeValueType::NMlEmpty:
            clearPreviousAttributeValue();
            m_aSmMlAttributeValueType = SmMlAttributeValueType::NMlEmpty;
            break;
        case SmMlAttributeValueType::MlAccent:
            setMlAccent(aAttribute->getMlAccent());
            break;
        case SmMlAttributeValueType::MlDir:
            setMlDir(aAttribute->getMlDir());
            break;
        case SmMlAttributeValueType::MlDisplaystyle:
            setMlDisplaystyle(aAttribute->getMlDisplaystyle());
            break;
        case SmMlAttributeValueType::MlFence:
            setMlFence(aAttribute->getMlFence());
            break;
        case SmMlAttributeValueType::MlHref:
            setMlHref(aAttribute->getMlHref());
            break;
        case SmMlAttributeValueType::MlLspace:
            setMlLspace(aAttribute->getMlLspace());
            break;
        case SmMlAttributeValueType::MlMathbackground:
            setMlMathbackground(aAttribute->getMlMathbackground());
            break;
        case SmMlAttributeValueType::MlMathcolor:
            setMlMathcolor(aAttribute->getMlMathcolor());
            break;
        case SmMlAttributeValueType::MlMathsize:
            setMlMathsize(aAttribute->getMlMathsize());
            break;
        case SmMlAttributeValueType::MlMathvariant:
            setMlMathvariant(aAttribute->getMlMathvariant());
            break;
        case SmMlAttributeValueType::MlMaxsize:
            setMlMaxsize(aAttribute->getMlMaxsize());
            break;
        case SmMlAttributeValueType::MlMinsize:
            setMlMinsize(aAttribute->getMlMinsize());
            break;
        case SmMlAttributeValueType::MlMovablelimits:
            setMlMovablelimits(aAttribute->getMlMovablelimits());
            break;
        case SmMlAttributeValueType::MlRspace:
            setMlRspace(aAttribute->getMlRspace());
            break;
        case SmMlAttributeValueType::MlSeparator:
            setMlSeparator(aAttribute->getMlSeparator());
            break;
        case SmMlAttributeValueType::MlStretchy:
            setMlStretchy(aAttribute->getMlStretchy());
            break;
        case SmMlAttributeValueType::MlSymmetric:
            setMlSymmetric(aAttribute->getMlSymmetric());
            break;
    }
}

/* get values */
/*************************************************************************************************/

const struct SmMlAccent* SmMlAttribute::getMlAccent() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlAccent)
        return &m_aAttributeValue.m_aAccent;
    return nullptr;
}

const struct SmMlDir* SmMlAttribute::getMlDir() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlDir)
        return &m_aAttributeValue.m_aDir;
    return nullptr;
}

const struct SmMlDisplaystyle* SmMlAttribute::getMlDisplaystyle() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlDisplaystyle)
        return &m_aAttributeValue.m_aDisplaystyle;
    return nullptr;
}

const struct SmMlFence* SmMlAttribute::getMlFence() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlFence)
        return &m_aAttributeValue.m_aFence;
    return nullptr;
}

const struct SmMlHref* SmMlAttribute::getMlHref() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlHref)
        return &m_aAttributeValue.m_aHref;
    return nullptr;
}

const struct SmMlLspace* SmMlAttribute::getMlLspace() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlLspace)
        return &m_aAttributeValue.m_aLspace;
    return nullptr;
}

const struct SmMlMathbackground* SmMlAttribute::getMlMathbackground() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMathbackground)
        return &m_aAttributeValue.m_aMathbackground;
    return nullptr;
}

const struct SmMlMathcolor* SmMlAttribute::getMlMathcolor() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMathcolor)
        return &m_aAttributeValue.m_aMathcolor;
    return nullptr;
}

const struct SmMlMathsize* SmMlAttribute::getMlMathsize() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlAccent)
        return &m_aAttributeValue.m_aMathsize;
    return nullptr;
}

const struct SmMlMathvariant* SmMlAttribute::getMlMathvariant() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMathvariant)
        return &m_aAttributeValue.m_aMathvariant;
    return nullptr;
}

const struct SmMlMaxsize* SmMlAttribute::getMlMaxsize() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMaxsize)
        return &m_aAttributeValue.m_aMaxsize;
    return nullptr;
}

const struct SmMlMinsize* SmMlAttribute::getMlMinsize() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMinsize)
        return &m_aAttributeValue.m_aMinsize;
    return nullptr;
}

const struct SmMlMovablelimits* SmMlAttribute::getMlMovablelimits() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlMovablelimits)
        return &m_aAttributeValue.m_aMovablelimits;
    return nullptr;
}

const struct SmMlRspace* SmMlAttribute::getMlRspace() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlRspace)
        return &m_aAttributeValue.m_aRspace;
    return nullptr;
}

const struct SmMlSeparator* SmMlAttribute::getMlSeparator() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlSeparator)
        return &m_aAttributeValue.m_aSeparator;
    return nullptr;
}

const struct SmMlStretchy* SmMlAttribute::getMlStretchy() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlStretchy)
        return &m_aAttributeValue.m_aStretchy;
    return nullptr;
}

const struct SmMlSymmetric* SmMlAttribute::getMlSymmetric() const
{
    if (m_aSmMlAttributeValueType == SmMlAttributeValueType::MlSymmetric)
        return &m_aAttributeValue.m_aSymmetric;
    return nullptr;
}

/* set values */
/*************************************************************************************************/

void SmMlAttribute::setMlAccent(const SmMlAccent* aAccent)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aAccent.m_aAccent = aAccent->m_aAccent;
}

void SmMlAttribute::setMlDir(const SmMlDir* aDir)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aDir.m_aDir = aDir->m_aDir;
}

void SmMlAttribute::setMlDisplaystyle(const SmMlDisplaystyle* aDisplaystyle)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aDisplaystyle.m_aDisplaystyle = aDisplaystyle->m_aDisplaystyle;
}

void SmMlAttribute::setMlFence(const SmMlFence* aFence)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aFence.m_aFence = aFence->m_aFence;
}

void SmMlAttribute::setMlHref(const SmMlHref* aHref)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aHref.m_aHref = aHref->m_aHref;
    m_aAttributeValue.m_aHref.m_aLnk = new OUString(*aHref->m_aLnk);
}

void SmMlAttribute::setMlLspace(const SmMlLspace* aLspace)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aLspace.m_aLengthValue.m_aLengthUnit
        = aLspace->m_aLengthValue.m_aLengthUnit;
    m_aAttributeValue.m_aLspace.m_aLengthValue.m_aLengthValue
        = aLspace->m_aLengthValue.m_aLengthValue;
    m_aAttributeValue.m_aLspace.m_aLengthValue.m_aOriginalText
        = new OUString(*aLspace->m_aLengthValue.m_aOriginalText);
}

void SmMlAttribute::setMlMathbackground(const SmMlMathbackground* aMathbackground)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMathbackground.m_aMathbackground = aMathbackground->m_aMathbackground;
}

void SmMlAttribute::setMlMathcolor(const SmMlMathcolor* aMathcolor)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMathcolor.m_aMathcolor = aMathcolor->m_aMathcolor;
}

void SmMlAttribute::setMlMathsize(const SmMlMathsize* aMathsize)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aLengthUnit
        = aMathsize->m_aLengthValue.m_aLengthUnit;
    m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aLengthValue
        = aMathsize->m_aLengthValue.m_aLengthValue;
    m_aAttributeValue.m_aMathsize.m_aLengthValue.m_aOriginalText
        = new OUString(*aMathsize->m_aLengthValue.m_aOriginalText);
}

void SmMlAttribute::setMlMathvariant(const SmMlMathvariant* aMathvariant)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMathvariant.m_aMathvariant = aMathvariant->m_aMathvariant;
}

void SmMlAttribute::setMlMaxsize(const SmMlMaxsize* aMaxsize)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMaxsize.m_aMaxsize = aMaxsize->m_aMaxsize;
    m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aLengthUnit
        = aMaxsize->m_aLengthValue.m_aLengthUnit;
    m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aLengthValue
        = aMaxsize->m_aLengthValue.m_aLengthValue;
    m_aAttributeValue.m_aMaxsize.m_aLengthValue.m_aOriginalText
        = new OUString(*aMaxsize->m_aLengthValue.m_aOriginalText);
}

void SmMlAttribute::setMlMinsize(const SmMlMinsize* aMinsize)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMinsize.m_aMinsize = aMinsize->m_aMinsize;
    m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aLengthUnit
        = aMinsize->m_aLengthValue.m_aLengthUnit;
    m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aLengthValue
        = aMinsize->m_aLengthValue.m_aLengthValue;
    m_aAttributeValue.m_aMinsize.m_aLengthValue.m_aOriginalText
        = new OUString(*aMinsize->m_aLengthValue.m_aOriginalText);
}

void SmMlAttribute::setMlMovablelimits(const SmMlMovablelimits* aMovablelimits)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aMovablelimits.m_aMovablelimits = aMovablelimits->m_aMovablelimits;
}

void SmMlAttribute::setMlRspace(const SmMlRspace* aRspace)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aRspace.m_aLengthValue.m_aLengthUnit
        = aRspace->m_aLengthValue.m_aLengthUnit;
    m_aAttributeValue.m_aRspace.m_aLengthValue.m_aLengthValue
        = aRspace->m_aLengthValue.m_aLengthValue;
    m_aAttributeValue.m_aRspace.m_aLengthValue.m_aOriginalText
        = new OUString(*aRspace->m_aLengthValue.m_aOriginalText);
}

void SmMlAttribute::setMlSeparator(const SmMlSeparator* aSeparator)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aSeparator.m_aSeparator = aSeparator->m_aSeparator;
}

void SmMlAttribute::setMlStretchy(const SmMlStretchy* aStretchy)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aStretchy.m_aStretchy = aStretchy->m_aStretchy;
}

void SmMlAttribute::setMlSymmetric(const SmMlSymmetric* aSymmetric)
{
    clearPreviousAttributeValue();
    m_aAttributeValue.m_aSymmetric.m_aSymmetric = aSymmetric->m_aSymmetric;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
