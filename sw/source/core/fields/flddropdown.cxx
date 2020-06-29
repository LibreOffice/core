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

#include <flddropdown.hxx>

#include <algorithm>

#include <svl/poolitem.hxx>
#include <comphelper/sequence.hxx>

#include <unofldmid.h>

using namespace com::sun::star;

using std::vector;

SwDropDownFieldType::SwDropDownFieldType()
    : SwFieldType(SwFieldIds::Dropdown)
{
}

SwDropDownFieldType::~SwDropDownFieldType()
{
}

std::unique_ptr<SwFieldType> SwDropDownFieldType::Copy() const
{
    return std::make_unique<SwDropDownFieldType>();
}

SwDropDownField::SwDropDownField(SwFieldType * pTyp)
    : SwField(pTyp, 0, LANGUAGE_SYSTEM)
{
}

SwDropDownField::SwDropDownField(const SwDropDownField & rSrc)
    : SwField(rSrc.GetTyp(), rSrc.GetFormat(), rSrc.GetLanguage()),
      m_aValues(rSrc.m_aValues), m_aSelectedItem(rSrc.m_aSelectedItem),
      m_aName(rSrc.m_aName), m_aHelp(rSrc.m_aHelp), m_aToolTip(rSrc.m_aToolTip)
{
}

SwDropDownField::~SwDropDownField()
{
}

OUString SwDropDownField::ExpandImpl(SwRootFrame const*const) const
{
    OUString sSelect = GetSelectedItem();
    if (sSelect.isEmpty())
    {
        vector<OUString>::const_iterator aIt = m_aValues.begin();
        if ( aIt != m_aValues.end())
            sSelect = *aIt;
    }
    // if still no list value is available a default text of 10 spaces is to be set
    if (sSelect.isEmpty())
        sSelect = "          ";
    return sSelect;
}

std::unique_ptr<SwField> SwDropDownField::Copy() const
{
    return std::make_unique<SwDropDownField>(*this);
}

OUString SwDropDownField::GetPar1() const
{
    return GetSelectedItem();
}

OUString SwDropDownField::GetPar2() const
{
    return GetName();
}

void SwDropDownField::SetPar1(const OUString & rStr)
{
    SetSelectedItem(rStr);
}

void SwDropDownField::SetPar2(const OUString & rName)
{
    SetName(rName);
}

void SwDropDownField::SetItems(const vector<OUString> & rItems)
{
    m_aValues = rItems;
    m_aSelectedItem.clear();
}

void SwDropDownField::SetItems(const uno::Sequence<OUString> & rItems)
{
    m_aValues.clear();

    comphelper::sequenceToContainer(m_aValues, rItems);

    m_aSelectedItem.clear();
}

uno::Sequence<OUString> SwDropDownField::GetItemSequence() const
{
    return comphelper::containerToSequence(m_aValues);
}


void SwDropDownField::SetSelectedItem(const OUString & rItem)
{
    vector<OUString>::const_iterator aIt =
        std::find(m_aValues.begin(), m_aValues.end(), rItem);

    if (aIt != m_aValues.end())
        m_aSelectedItem = *aIt;
    else
        m_aSelectedItem.clear();
}

void SwDropDownField::SetName(const OUString & rName)
{
    m_aName = rName;
}

void SwDropDownField::SetHelp(const OUString & rHelp)
{
    m_aHelp = rHelp;
}

void SwDropDownField::SetToolTip(const OUString & rToolTip)
{
    m_aToolTip = rToolTip;
}

bool SwDropDownField::QueryValue(::uno::Any &rVal, sal_uInt16 nWhich) const
{
    nWhich &= ~CONVERT_TWIPS;
    switch( nWhich )
    {
    case FIELD_PROP_PAR1:
        rVal <<= m_aSelectedItem;
        break;
    case FIELD_PROP_PAR2:
        rVal <<= m_aName;
        break;
    case FIELD_PROP_PAR3:
        rVal <<= m_aHelp;
        break;
    case FIELD_PROP_PAR4:
        rVal <<= m_aToolTip;
        break;
    case FIELD_PROP_STRINGS:
        rVal <<= GetItemSequence();
        break;

    default:
        assert(false);
    }
    return true;
}

bool SwDropDownField::PutValue(const uno::Any &rVal,
                               sal_uInt16 nWhich)
{
    switch( nWhich )
    {
    case FIELD_PROP_PAR1:
        {
            OUString aTmpStr;
            rVal >>= aTmpStr;

            SetSelectedItem(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR2:
        rVal >>= m_aName;
        break;

    case FIELD_PROP_PAR3:
        rVal >>= m_aHelp;
        break;

    case FIELD_PROP_PAR4:
        rVal >>= m_aToolTip;
        break;

    case FIELD_PROP_STRINGS:
        {
            uno::Sequence<OUString> aSeq;
            rVal >>= aSeq;
            SetItems(aSeq);
        }
        break;

    default:
        assert(false);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
