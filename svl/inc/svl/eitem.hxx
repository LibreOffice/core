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

#ifndef _SFXENUMITEM_HXX
#define _SFXENUMITEM_HXX

#include "svl/svldllapi.h"
#include <svl/cenumitm.hxx>

//============================================================================
class SVL_DLLPUBLIC SfxEnumItem: public CntEnumItem
{
protected:
    explicit SfxEnumItem(sal_uInt16 which = 0, sal_uInt16 nValue = 0):
        CntEnumItem(which, nValue) {}

    SfxEnumItem(sal_uInt16 which, SvStream & rStream):
        CntEnumItem(which, rStream) {}

public:
    TYPEINFO();

};

//============================================================================
class SVL_DLLPUBLIC SfxBoolItem: public CntBoolItem
{
public:
    TYPEINFO();

    explicit SfxBoolItem(sal_uInt16 which = 0, sal_Bool bValue = sal_False):
        CntBoolItem(which, bValue) {}

    SfxBoolItem(sal_uInt16 which, SvStream & rStream):
        CntBoolItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
    { return new SfxBoolItem(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxBoolItem(*this); }
};

#endif //  _SFXENUMITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
