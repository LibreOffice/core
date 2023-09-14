/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#pragma once

#include <editeng/editengdllapi.h>
#include <rtl/ustring.hxx>

/*************************************************************************
 *                      class SvxDoCapitals
 * The virtual Method Do is called by SvxFont::DoOnCapitals alternately
 * the uppercase and lowercase parts. The derivate of SvxDoCapitals fills
 * this method with life.
 *************************************************************************/

class EDITENG_DLLPUBLIC SvxDoCapitals
{
protected:
    const OUString& rTxt;
    const sal_Int32 nIdx;
    const sal_Int32 nLen;

public:
    SvxDoCapitals(const OUString& _rTxt, const sal_Int32 _nIdx, const sal_Int32 _nLen)
        : rTxt(_rTxt)
        , nIdx(_nIdx)
        , nLen(_nLen)
    {
    }

    virtual ~SvxDoCapitals() {}

    virtual void DoSpace(const bool bDraw);
    virtual void SetSpace();
    virtual void Do(const OUString& rTxt, const sal_Int32 nIdx, const sal_Int32 nLen,
                    const bool bUpper)
        = 0;

    const OUString& GetTxt() const { return rTxt; }
    sal_Int32 GetIdx() const { return nIdx; }
    sal_Int32 GetLen() const { return nLen; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
