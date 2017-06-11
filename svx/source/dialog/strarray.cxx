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

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strarray.hxx>
#include <tools/resary.hxx>
#include <svx/svxitems.hrc>
#include "fieldunit.hrc"
#include "numberingtype.hrc"

SvxStringArray::SvxStringArray(const char **pResId, size_t nLength)
{
    for (size_t i = 0; i < nLength; ++i)
        m_aTranslations.push_back(SvxResId(pResId[i]));
}

const OUString SvxStringArray::GetString(sal_uInt32 nPos) const
{
    return m_aTranslations[nPos];
}

sal_uInt32 SvxFieldUnitTable::Count()
{
    return SAL_N_ELEMENTS(RID_SVXSTR_FIELDUNIT_TABLE);
}

OUString SvxFieldUnitTable::GetString(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return SvxResId(RID_SVXSTR_FIELDUNIT_TABLE[nPos].first);
    return OUString();
}

FieldUnit SvxFieldUnitTable::GetValue(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return RID_SVXSTR_FIELDUNIT_TABLE[nPos].second;
    return FUNIT_NONE;
}

OUString SvxAttrNameTable::GetString(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return SvxResId(RID_ATTR_NAMES[nPos].first);
    return OUString();
}

sal_uInt32 SvxAttrNameTable::Count()
{
    return SAL_N_ELEMENTS(RID_ATTR_NAMES);
}

sal_uInt16 SvxAttrNameTable::GetValue(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return RID_ATTR_NAMES[nPos].second;
    return 0;
}

sal_uInt32 SvxAttrNameTable::FindIndex(int nValue)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_ATTR_NAMES); ++i)
    {
        if (nValue == RID_ATTR_NAMES[i].second)
            return i;
    }
    return RESARRAY_INDEX_NOTFOUND;
}

OUString SvxNumberingTypeTable::GetString(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return SvxResId(RID_SVXSTRARY_NUMBERINGTYPE[nPos].first);
    return OUString();
}

sal_uInt32 SvxNumberingTypeTable::Count()
{
    return SAL_N_ELEMENTS(RID_SVXSTRARY_NUMBERINGTYPE);
}

int SvxNumberingTypeTable::GetValue(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return RID_SVXSTRARY_NUMBERINGTYPE[nPos].second;
    return 0;
}

sal_uInt32 SvxNumberingTypeTable::FindIndex(int nValue)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_SVXSTRARY_NUMBERINGTYPE); ++i)
    {
        if (nValue == RID_SVXSTRARY_NUMBERINGTYPE[i].second)
            return i;
    }
    return RESARRAY_INDEX_NOTFOUND;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
