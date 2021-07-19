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
#ifndef INCLUDED_SFX2_STYFITEM_HXX
#define INCLUDED_SFX2_STYFITEM_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/style.hxx>
#include <unotools/resmgr.hxx>
#include <vector>

struct SFX2_DLLPUBLIC SfxFilterTuple
{
    OUString aName;
    SfxStyleSearchBits nFlags;
    SfxFilterTuple(const OUString& rName, SfxStyleSearchBits nArg)
        : aName(rName)
        , nFlags(nArg)
    {
    }
};

typedef std::vector<SfxFilterTuple> SfxStyleFilter;

class SFX2_DLLPUBLIC SfxStyleFamilyItem
{
    SfxStyleFamily  nFamily;
    OUString        aText;
    OUString        aImage;
    SfxStyleFilter  aFilterList;

public:
    SfxStyleFamilyItem(SfxStyleFamily nFamily, const OUString &rName, const OUString& rImage, const std::pair<TranslateId, SfxStyleSearchBits>* pStringArray, const std::locale& rLocale);

    const OUString& GetText() const { return aText; }
    SfxStyleFamily  GetFamily() const { return nFamily; }
    const SfxStyleFilter& GetFilterList() const { return aFilterList; }
    const OUString&    GetImage() const { return aImage; }
};

using SfxStyleFamilies = std::vector<SfxStyleFamilyItem>;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
