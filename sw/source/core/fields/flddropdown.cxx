/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <flddropdown.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include <svl/poolitem.hxx>

#include <unofldmid.h>
#include <unoprnms.hxx>

using namespace com::sun::star;

using rtl::OUString;
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
        vector<String>::const_iterator aIt = aValues.begin();
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

const String & SwDropDownField::GetPar1() const
{
    return GetSelectedItem();
}

String SwDropDownField::GetPar2() const
{
    return GetName();
}

void SwDropDownField::SetPar1(const String & rStr)
{
    SetSelectedItem(rStr);
}

void SwDropDownField::SetPar2(const String & rName)
{
    SetName(rName);
}

void SwDropDownField::SetItems(const vector<String> & rItems)
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
    vector<String>::const_iterator aIt;

    for (aIt = aValues.begin(); aIt != aValues.end(); ++aIt)
    {
        pSeq[i] = rtl::OUString(*aIt);

        i++;
    }

    return aSeq;
}

const String & SwDropDownField::GetSelectedItem() const
{
    return aSelectedItem;
}

const String & SwDropDownField::GetName() const
{
    return aName;
}

const String & SwDropDownField::GetHelp() const
{
    return aHelp;
}

const String & SwDropDownField::GetToolTip() const
{
    return aToolTip;
}

sal_Bool SwDropDownField::SetSelectedItem(const String & rItem)
{
    vector<String>::const_iterator aIt =
        std::find(aValues.begin(), aValues.end(), rItem);

    if (aIt != aValues.end())
        aSelectedItem = *aIt;
    else
        aSelectedItem = String();

    return (aIt != aValues.end());
}

void SwDropDownField::SetName(const String & rName)
{
    aName = rName;
}

void SwDropDownField::SetHelp(const String & rHelp)
{
    aHelp = rHelp;
}

void SwDropDownField::SetToolTip(const String & rToolTip)
{
    aToolTip = rToolTip;
}

bool SwDropDownField::QueryValue(::uno::Any &rVal, sal_uInt16 nWhich) const
{
    nWhich &= ~CONVERT_TWIPS;
    switch( nWhich )
    {
    case FIELD_PROP_PAR1:
        rVal <<= rtl::OUString(GetSelectedItem());
        break;
    case FIELD_PROP_PAR2:
        rVal <<= rtl::OUString(GetName());
        break;
    case FIELD_PROP_PAR3:
        rVal <<= rtl::OUString(GetHelp());
        break;
    case FIELD_PROP_PAR4:
        rVal <<= rtl::OUString(GetToolTip());
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
            String aTmpStr;
            ::GetString( rVal, aTmpStr );

            SetSelectedItem(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR2:
        {
            String aTmpStr;
            ::GetString( rVal, aTmpStr );

            SetName(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR3:
        {
            String aTmpStr;
            ::GetString( rVal, aTmpStr );

            SetHelp(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR4:
        {
            String aTmpStr;
            ::GetString( rVal, aTmpStr );

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
