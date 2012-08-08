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

#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// namespace

namespace comphelper{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

SequenceAsHashMap::SequenceAsHashMap()
    : SequenceAsHashMapBase()
{
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Any& aSource)
{
    (*this) << aSource;
}

//-----------------------------------------------
SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::uno::Any >& lSource)
{
    (*this) << lSource;
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    (*this) << lSource;
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    (*this) << lSource;
}

SequenceAsHashMap::~SequenceAsHashMap()
{
}

void SequenceAsHashMap::operator<<(const css::uno::Any& aSource)
{
    // An empty Any reset this instance!
    if (!aSource.hasValue())
    {
        clear();
        return;
    }

    css::uno::Sequence< css::beans::NamedValue > lN;
    if (aSource >>= lN)
    {
        (*this) << lN;
        return;
    }

    css::uno::Sequence< css::beans::PropertyValue > lP;
    if (aSource >>= lP)
    {
        (*this) << lP;
        return;
    }

    throw css::beans::IllegalTypeException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Any contains wrong type." )),
            css::uno::Reference< css::uno::XInterface >());
}

//-----------------------------------------------
void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::uno::Any >& lSource)
{
    sal_Int32 c = lSource.getLength();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        css::beans::PropertyValue lP;
        if (lSource[i] >>= lP)
        {
            if (
                (lP.Name.isEmpty()) ||
                (!lP.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PropertyValue struct contains no usefull informations." )),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lP.Name] = lP.Value;
            continue;
        }

        css::beans::NamedValue lN;
        if (lSource[i] >>= lN)
        {
            if (
                (lN.Name.isEmpty()) ||
                (!lN.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NamedValue struct contains no usefull informations." )),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lN.Name] = lN.Value;
            continue;
        }

        // ignore VOID Any ... but reject wrong filled ones!
        if (lSource[i].hasValue())
            throw css::beans::IllegalTypeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Any contains wrong type." )),
                    css::uno::Reference< css::uno::XInterface >());
    }
}

void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    clear();

          sal_Int32                  c       = lSource.getLength();
    const css::beans::PropertyValue* pSource = lSource.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
        (*this)[pSource[i].Name] = pSource[i].Value;
}

void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    clear();

          sal_Int32               c       = lSource.getLength();
    const css::beans::NamedValue* pSource = lSource.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
        (*this)[pSource[i].Name] = pSource[i].Value;
}

void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::PropertyValue >& lDestination) const
{
    sal_Int32 c = (sal_Int32)size();
    lDestination.realloc(c);
    css::beans::PropertyValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first ;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::NamedValue >& lDestination) const
{
    sal_Int32 c = (sal_Int32)size();
    lDestination.realloc(c);
    css::beans::NamedValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first ;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

const css::uno::Any SequenceAsHashMap::getAsConstAny(::sal_Bool bAsPropertyValueList) const
{
    css::uno::Any aDestination;
    if (bAsPropertyValueList)
        aDestination = css::uno::makeAny(getAsConstPropertyValueList());
    else
        aDestination = css::uno::makeAny(getAsConstNamedValueList());
    return aDestination;
}

const css::uno::Sequence< css::beans::NamedValue > SequenceAsHashMap::getAsConstNamedValueList() const
{
    css::uno::Sequence< css::beans::NamedValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

const css::uno::Sequence< css::beans::PropertyValue > SequenceAsHashMap::getAsConstPropertyValueList() const
{
    css::uno::Sequence< css::beans::PropertyValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

sal_Bool SequenceAsHashMap::match(const SequenceAsHashMap& rCheck) const
{
    const_iterator pCheck;
    for (  pCheck  = rCheck.begin();
           pCheck != rCheck.end()  ;
         ++pCheck                  )
    {
        const ::rtl::OUString& sCheckName  = pCheck->first;
        const css::uno::Any&   aCheckValue = pCheck->second;
              const_iterator   pFound      = find(sCheckName);

        if (pFound == end())
            return sal_False;

        const css::uno::Any& aFoundValue = pFound->second;
        if (aFoundValue != aCheckValue)
            return sal_False;
    }

    return sal_True;
}

void SequenceAsHashMap::update(const SequenceAsHashMap& rUpdate)
{
    const_iterator pUpdate;
    for (  pUpdate  = rUpdate.begin();
           pUpdate != rUpdate.end()  ;
         ++pUpdate                   )
    {
        const ::rtl::OUString& sName  = pUpdate->first;
        const css::uno::Any&   aValue = pUpdate->second;

        (*this)[sName] = aValue;
    }
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
