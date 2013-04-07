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

#include <unofldmid.h>
#include <unoprnms.hxx>

using namespace com::sun::star;

using std::vector;

static String aEmptyString;

SwDropDownFieldType::SwDropDownFieldType()
    : SwFieldType(RES_DROPDOWN)
{
}

SwDropDownFieldType::~SwDropDownFieldType()
{
}

SwFieldType * SwDropDownFieldType::Copy() const
{
    return new SwDropDownFieldType;
}

SwDropDownField::SwDropDownField(SwFieldType * pTyp)
    : SwField(pTyp, 0, LANGUAGE_SYSTEM)
{
}

SwDropDownField::SwDropDownField(const SwDropDownField & rSrc)
    : SwField(rSrc.GetTyp(), rSrc.GetFormat(), rSrc.GetLanguage()),
      aValues(rSrc.aValues), aSelectedItem(rSrc.aSelectedItem),
      aName(rSrc.aName), aHelp(rSrc.aHelp), aToolTip(rSrc.aToolTip)
{
}

SwDropDownField::~SwDropDownField()
{
}

String SwDropDownField::Expand() const
{
    String sSelect = GetSelectedItem();
    if(!sSelect.Len())
    {
        vector<OUString>::const_iterator aIt = aValues.begin();
        if ( aIt != aValues.end())
            sSelect = *aIt;
    }
    //if still no list value is available a default text of 10 spaces is to be set
    if(!sSelect.Len())
        sSelect.AppendAscii ( RTL_CONSTASCII_STRINGPARAM ("          "));
    return sSelect;
}

SwField * SwDropDownField::Copy() const
{
    return new SwDropDownField(*this);
}

const OUString & SwDropDownField::GetPar1() const
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
    aValues = rItems;
    aSelectedItem = aEmptyString;
}

void SwDropDownField::SetItems(const uno::Sequence<OUString> & rItems)
{
    aValues.clear();

    sal_Int32 aCount = rItems.getLength();
    for (int i = 0; i < aCount; i++)
        aValues.push_back(rItems[i]);

    aSelectedItem = aEmptyString;
}

uno::Sequence<OUString> SwDropDownField::GetItemSequence() const
{
    uno::Sequence<OUString> aSeq( aValues.size() );
    OUString* pSeq = aSeq.getArray();
    int i = 0;
    vector<OUString>::const_iterator aIt;

    for (aIt = aValues.begin(); aIt != aValues.end(); ++aIt)
    {
        pSeq[i] = *aIt;
        i++;
    }

    return aSeq;
}

const OUString & SwDropDownField::GetSelectedItem() const
{
    return aSelectedItem;
}

const OUString & SwDropDownField::GetName() const
{
    return aName;
}

const OUString & SwDropDownField::GetHelp() const
{
    return aHelp;
}

const OUString & SwDropDownField::GetToolTip() const
{
    return aToolTip;
}

sal_Bool SwDropDownField::SetSelectedItem(const OUString & rItem)
{
    vector<OUString>::const_iterator aIt =
        std::find(aValues.begin(), aValues.end(), rItem);

    if (aIt != aValues.end())
        aSelectedItem = *aIt;
    else
        aSelectedItem = OUString();

    return (aIt != aValues.end());
}

void SwDropDownField::SetName(const OUString & rName)
{
    aName = rName;
}

void SwDropDownField::SetHelp(const OUString & rHelp)
{
    aHelp = rHelp;
}

void SwDropDownField::SetToolTip(const OUString & rToolTip)
{
    aToolTip = rToolTip;
}

bool SwDropDownField::QueryValue(::uno::Any &rVal, sal_uInt16 nWhich) const
{
    nWhich &= ~CONVERT_TWIPS;
    switch( nWhich )
    {
    case FIELD_PROP_PAR1:
        rVal <<= GetSelectedItem();
        break;
    case FIELD_PROP_PAR2:
        rVal <<= GetName();
        break;
    case FIELD_PROP_PAR3:
        rVal <<= GetHelp();
        break;
    case FIELD_PROP_PAR4:
        rVal <<= GetToolTip();
        break;
    case FIELD_PROP_STRINGS:
        rVal <<= GetItemSequence();

        break;

    default:
        OSL_FAIL("illegal property");
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
        {
            OUString aTmpStr;
            rVal >>= aTmpStr;

            SetName(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR3:
        {
            OUString aTmpStr;
            rVal >>= aTmpStr;

            SetHelp(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR4:
        {
            OUString aTmpStr;
            rVal >>= aTmpStr;

            SetToolTip(aTmpStr);
        }
        break;

    case FIELD_PROP_STRINGS:
        {
            uno::Sequence<OUString> aSeq;
            rVal >>= aSeq;
            SetItems(aSeq);
        }
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
