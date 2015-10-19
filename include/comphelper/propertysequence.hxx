/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_PROPERTYSEQUENCE_HXX
#define INCLUDED_COMPHELPER_PROPERTYSEQUENCE_HXX

#include <utility>
#include <initializer_list>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace comphelper
{
    inline css::uno::Sequence< css::beans::PropertyValue > InitPropertySequence(
        ::std::initializer_list< ::std::pair< OUString, css::uno::Any > > vInit)
    {
        css::uno::Sequence< css::beans::PropertyValue> vResult{static_cast<sal_Int32>(vInit.size())};
        size_t nCount{0};
        for(const auto& aEntry : vInit)
        {
            vResult[nCount].Name = aEntry.first;
            vResult[nCount].Handle = -1;
            vResult[nCount].Value = aEntry.second;
            // State is default-initialized to DIRECT_VALUE
            ++nCount;
        }
        return vResult;
    }
}   // namespace comphelper



#endif // INCLUDED_COMPHELPER_PROPERTYSEQUENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
