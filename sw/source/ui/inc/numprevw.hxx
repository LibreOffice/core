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

#ifndef _NUMPREVW_HXX
#define _NUMPREVW_HXX


#include <vcl/window.hxx>

class SwNumRule;
namespace rtl { class OUString; }

class NumberingPreview : public Window
{
    const SwNumRule*    pActNum;
    Font                aStdFont;
    long                nPageWidth;
    const OUString*     pOutlineNames;
    sal_Bool                bPosition;
    sal_uInt16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        NumberingPreview(Window* pParent, const ResId& rResId)
        : Window(pParent, rResId),
            pActNum(0),nPageWidth(0), pOutlineNames(0),
            bPosition(sal_False), nActLevel(USHRT_MAX)
        {
        }

        NumberingPreview(Window* pParent)
            : Window(pParent)
            , pActNum(0),nPageWidth(0), pOutlineNames(0),
            bPosition(sal_False), nActLevel(USHRT_MAX)
        {
        }

        ~NumberingPreview();

        void    SetNumRule(const SwNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const OUString* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = sal_True;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
