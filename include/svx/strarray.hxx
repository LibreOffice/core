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
#ifndef INCLUDED_SVX_STRARRAY_HXX
#define INCLUDED_SVX_STRARRAY_HXX

#include <tools/fldunit.hxx>
#include <svx/svxdllapi.h>
#include <vector>

//  class SvxStringArray -------------------------------------------------

class SVX_DLLPUBLIC SvxStringArray
{
    std::vector<OUString> m_aTranslations;
public:
    SvxStringArray(const char** pResId, size_t nLength);
    const OUString GetString(sal_uInt32 nPos) const;
};

class SVX_DLLPUBLIC SvxFieldUnitTable
{
public:
    static OUString GetString(sal_uInt32 i);
    static sal_uInt32 Count();
    static FieldUnit GetValue(sal_uInt32 i);
};

class SVX_DLLPUBLIC SvxNumberingTypeTable
{
public:
    static OUString GetString(sal_uInt32 i);
    static sal_uInt32 Count();
    static int GetValue(sal_uInt32 i);
    static sal_uInt32 FindIndex(int nValue);
};

class SVX_DLLPUBLIC SvxAttrNameTable
{
public:
    static OUString GetString(sal_uInt32 i);
    static sal_uInt32 Count();
    static sal_uInt16 GetValue(sal_uInt32 i);
    static sal_uInt32 FindIndex(int nValue);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
