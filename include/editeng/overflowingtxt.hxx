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

#ifndef INCLUDED_EDITENG_OVERFLOWINGTXT_HXX
#define INCLUDED_EDITENG_OVERFLOWINGTXT_HXX

#include <editeng/macros.hxx>
#include <editeng/editengdllapi.h>

class OUString;


class OutlinerParaObject;


class EDITENG_DLLPUBLIC OverflowingText
{

public:
        OUString mHeadTxt;
        const OutlinerParaObject *mpMidParas;
        OUString mTailTxt;
        // NOTE: mpMidParas and mTailTxt might be empty

        // Constructor
        OverflowingText(
            const OUString &headTxt,
            const OutlinerParaObject *pMidParas,
            const OUString &tailTxt)
                : mHeadTxt(headTxt),
                  mpMidParas(pMidParas),
                  mTailTxt(tailTxt)
                { }

        OUString GetHeadingLines() const;
        OUString GetEndingLines() const;
};

class EDITENG_DLLPUBLIC NonOverflowingText {
    public:
        const OutlinerParaObject *mpHeadParas;
        OUString mPreOverflowingTxt;
        // NOTE: mPreOverflowingTxt might be empty

        // Constructor
        NonOverflowingText(
            const OutlinerParaObject *pHeadParas,
            const OUString &preOverflowingTxt)
                : mpHeadParas(pHeadParas),
                  mPreOverflowingTxt(preOverflowingTxt)
                {
                    if (pHeadParas == NULL) // Redundant line for debugging
                        DBG_ASSERT( pHeadParas != NULL, "pHeadParas is null?! All text is overflowing then" );
                }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
