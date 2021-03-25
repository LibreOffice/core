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

#include <gridcols.hxx>
#include <tools/debug.hxx>
#include <fmservs.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star::uno;


static const css::uno::Sequence<OUString>& getColumnTypes()
{
    static css::uno::Sequence<OUString> aColumnTypes = [&]()
    {
        css::uno::Sequence<OUString> tmp(10);
        OUString* pNames = tmp.getArray();
        pNames[TYPE_CHECKBOX] = FM_COL_CHECKBOX;
        pNames[TYPE_COMBOBOX] = FM_COL_COMBOBOX;
        pNames[TYPE_CURRENCYFIELD] = FM_COL_CURRENCYFIELD;
        pNames[TYPE_DATEFIELD] = FM_COL_DATEFIELD;
        pNames[TYPE_FORMATTEDFIELD] = FM_COL_FORMATTEDFIELD;
        pNames[TYPE_LISTBOX] = FM_COL_LISTBOX;
        pNames[TYPE_NUMERICFIELD] = FM_COL_NUMERICFIELD;
        pNames[TYPE_PATTERNFIELD] = FM_COL_PATTERNFIELD;
        pNames[TYPE_TEXTFIELD] = FM_COL_TEXTFIELD;
        pNames[TYPE_TIMEFIELD] = FM_COL_TIMEFIELD;
        return tmp;
    }();
    return aColumnTypes;
}


extern "C" {

// comparison of PropertyInfo
static int NameCompare(const void* pFirst, const void* pSecond)
{
    return static_cast<OUString const *>(pFirst)->compareTo(*static_cast<OUString const *>(pSecond));
}

}

namespace
{

    sal_Int32 lcl_findPos(const OUString& aStr, const Sequence< OUString>& rList)
    {
        const OUString* pStrList = rList.getConstArray();
        OUString* pResult = static_cast<OUString*>(bsearch(&aStr, static_cast<void const *>(pStrList), rList.getLength(), sizeof(OUString),
            &NameCompare));

        if (pResult)
            return (pResult - pStrList);
        else
            return -1;
    }
}


sal_Int32 getColumnTypeByModelName(const OUString& aModelName)
{
    static const OUStringLiteral aModelPrefix(u"com.sun.star.form.component.");
    static const OUStringLiteral aCompatibleModelPrefix(u"stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == FM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.indexOf(aModelPrefix);
#ifdef DBG_UTIL
        sal_Int32 nCompatiblePrefixPos = aModelName.indexOf(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1), "::getColumnTypeByModelName() : wrong service!");
#endif

        OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.getLength())
            : aModelName.copy(aCompatibleModelPrefix.getLength());

        const css::uno::Sequence<OUString>& rColumnTypes = getColumnTypes();
        nTypeId = lcl_findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
