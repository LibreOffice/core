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

#include <mathml/mathmldef.hxx>

void SmMlAttribute::clearPreviousAttributeValueType()
{
    switch (m_aSmAttributeValueType)
    {
        case SmAttributeValueType::MlHref:
            if (m_aAttributeValue.m_aHref.m_aLnk)
                delete m_aAttributeValue.m_aHref.m_aLnk;
        default:
            break;
    }
}

/* SmMlAttribute::get values */
/*************************************************************************************************/

const struct SmMlAccent* SmMlAttribute::getAccent() const()
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlAccent)
        return *m_aAttributeValue.m_aAccent;
    return nullptr;
}

const struct SmMlDir* SmMlAttribute::getDir() const()
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlDir)
        return *m_aAttributeValue.m_aDir;
    return nullptr;
}

const struct SmMlDisplaystyle* SmMlAttribute::getDisplaystyle() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlDisplaystyle)
        return *m_aAttributeValue.m_aDisplaystyle;
    return nullptr;
}

const struct SmMlFence* SmMlAttribute::getFence() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlFence)
        return *m_aAttributeValue.m_aFence;
    return nullptr;
}

const struct SmMlHref* SmMlAttribute::getHref() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlHref)
        return *m_aAttributeValue.m_aHref;
    return nullptr;
}

const struct SmMlLspace* SmMlAttribute::getLspace() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlLspace)
        return *m_aAttributeValue.m_aLspace;
    return nullptr;
}

const struct SmMlMathbackground* SmMlAttribute::getMathbackground() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMathbackground)
        return *m_aAttributeValue.m_aMathbackground;
    return nullptr;
}

const struct SmMlMathcolor* SmMlAttribute::getMathcolor() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMathcolor)
        return *m_aAttributeValue.m_aMathcolor;
    return nullptr;
}

const struct SmMlMathsize* SmMlAttribute::getMathsize() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlAccent)
        return *m_aAttributeValue.m_aAccent;
    return nullptr;
}

const struct SmMlMathvariant* SmMlAttribute::getMathvariant() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMathvariant)
        return *m_aAttributeValue.m_aMathvariant;
    return nullptr;
}

const struct SmMlMaxsize* SmMlAttribute::getMaxsize() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMaxsize)
        return *m_aAttributeValue.m_aMaxsize;
    return nullptr;
}

const struct SmMlMinsize* SmMlAttribute::getMinSize() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMinSize)
        return *m_aAttributeValue.m_aMinSize;
    return nullptr;
}

const struct SmMlMovablelimits* SmMlAttribute::getMovablelimits() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlMovablelimits)
        return *m_aAttributeValue.m_aMovablelimits;
    return nullptr;
}

const struct SmMlRspace* SmMlAttribute::getRspace() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlRspace)
        return *m_aAttributeValue.m_aRspace;
    return nullptr;
}

const struct SmMlSeparator* SmMlAttribute::getSeparator() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlSeparator)
        return *m_aAttributeValue.m_aSeparator;
    return nullptr;
}

const struct SmMlStretchy* SmMlAttribute::getStretchy() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlStretchy)
        return *m_aAttributeValue.m_aStretchy;
    return nullptr;
}

const struct SmMlSymmetric* SmMlAttribute::getSymmetric() const
{
    if (m_aSmAttributeValueType == SmAttributeValueType::MlSymmetric)
        return *m_aAttributeValue.m_aSymmetric;
    return nullptr;
}

/* set values */
/*************************************************************************************************/

void SmMlAttribute::setMlAccent(const SmMlAccent* aAccent)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aAccent.m_aAccent = aAccent->m_aAccent;
}

void SmMlAttribute::setMlDir(const SmMlDir* aDir)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aDir.m_aDir = aDir->m_aDir;
}

void SmMlAttribute::setMlDisplaystyle(const SmMlDisplaystyle* aDisplaystyle)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aDisplaystyle.m_aDisplaystyle = aDisplaystyle->m_aDisplaystyle;
}

void SmMlAttribute::setMlFence(const SmMlFence* aFence)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aFence.m_aFence = aFence->m_aFence;
}

void SmMlAttribute::setMlHref(const SmMlHref* aHref)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aHref.m_aHref = aHref->m_aHref;
    m_aAttributeValue.m_aLnk.m_aLnk = OUString(*aHref->m_aLnk);
}

void SmMlAttribute::setMlLspace(const SmMlLspace* aLspace)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aLspace.m_aLspace = aLspace->m_aLspace;
}

void SmMlAttribute::setMlMathbackground(const SmMlMathbackground* aMathbackground)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMathbackground.m_aMathbackground = aMathbackground->m_aMathbackground;
}

void SmMlAttribute::setMlMathcolor(const SmMlMathcolor* aMathcolor)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMathcolor.m_aMathcolor = aMathcolor->m_aMathcolor;
}

void SmMlAttribute::setMlMathsize(const SmMlMathsize* aMathsize)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMathsize.m_aMathsize = aMathsize->m_aMathsize;
}

void SmMlAttribute::setMlMathvariant(const SmMlMathvariant* aMathvariant)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMathvariant.m_aMathvariant = aMathvariant->m_aMathvariant;
}

void SmMlAttribute::setMaxsize(const SmMlMaxsize* aMaxsize)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMaxsize.m_aMaxsize = aMaxsize->m_aMaxsize;
}

void SmMlAttribute::setMinsize(const SmMlMinsize* aMinSize)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMinSize.m_aMinSize = aMinSize->m_aMinSize;
}

void SmMlAttribute::setMovablelimits(const SmMlMovablelimits* aMovablelimits)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aMovablelimits.m_aMovablelimits = aMovablelimits->m_aMovablelimits;
}

void SmMlAttribute::setMlRspace(const SmMlRspace* aRspace)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aRspace.m_aRspace = aRspace->m_aRspace;
}

void SmMlAttribute::setMlSeparator(const SmMlSeparator* aSeparator)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aSeparator.m_aSeparator = aSeparator->m_aSeparator;
}

void SmMlAttribute::setMlStretchy(const SmMlStretchy* aStretchy)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aStretchy.m_aStretchy = aStretchy->m_aStretchy;
}

void SmMlAttribute::setMlSymmetric(const SmMlSymmetric* aSymmetric)
{
    clearPreviousAttributeValueType();
    m_aAttributeValue.m_aSymmetric.m_aSymmetric = aSymmetric->m_aSymmetric;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
