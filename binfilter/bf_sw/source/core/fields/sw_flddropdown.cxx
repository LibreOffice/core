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

#include <algorithm>
#include <bf_svtools/poolitem.hxx>
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif
#include <flddropdown.hxx>
namespace binfilter {
extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008
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
      aName(rSrc.aName)
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

void SwDropDownField::SetItems(const Sequence<OUString> & rItems)
{
    aValues.clear();

    sal_Int32 aCount = rItems.getLength();
    for (int i = 0; i < aCount; i++)
        aValues.push_back(rItems[i]);

    aSelectedItem = aEmptyString;
}

Sequence<OUString> SwDropDownField::GetItemSequence() const
{
    Sequence<OUString> aSeq( aValues.size() );
    OUString* pSeq = aSeq.getArray();
    int i = 0;
    vector<String>::const_iterator aIt;
    
    for (aIt = aValues.begin(); aIt != aValues.end(); aIt++)
    {
        pSeq[i] = ::rtl::OUString(*aIt);
        
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

BOOL SwDropDownField::SetSelectedItem(const String & rItem)
{
    vector<String>::const_iterator aIt = 
        find(aValues.begin(), aValues.end(), rItem);

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

BOOL SwDropDownField::QueryValue(Any &rVal, BYTE nMId) 
    const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rVal <<= ::rtl::OUString(GetSelectedItem());
        break;
    case FIELD_PROP_PAR2:
        rVal <<= ::rtl::OUString(GetName());
        break;
    case FIELD_PROP_STRINGS:
        rVal <<= GetItemSequence();

        break;

    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

BOOL SwDropDownField::PutValue(const Any &rVal, 
                               BYTE nMId)
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        {
            String aTmpStr;
            ::binfilter::GetString( rVal, aTmpStr );
            
            SetSelectedItem(aTmpStr);
        }
        break;

    case FIELD_PROP_PAR2:
        {
            String aTmpStr;
            ::binfilter::GetString( rVal, aTmpStr );
            
            SetName(aTmpStr);
        }
        break;

    case FIELD_PROP_STRINGS:
        {
            Sequence<OUString> aSeq;
            rVal >>= aSeq;
            SetItems(aSeq);
        }
        break;
        
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
}
