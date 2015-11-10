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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMPREVW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMPREVW_HXX

#include <vcl/window.hxx>

class SwNumRule;
namespace rtl { class OUString; }

class NumberingPreview : public vcl::Window
{
    const SwNumRule*    pActNum;
    vcl::Font           aStdFont;
    long                nPageWidth;
    const OUString*     pOutlineNames;
    bool                bPosition;
    sal_uInt16          nActLevel;

    protected:
        virtual void        Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;

    public:
        NumberingPreview(vcl::Window* pParent, const ResId& rResId)
        : Window(pParent, rResId),
            pActNum(nullptr),nPageWidth(0), pOutlineNames(nullptr),
            bPosition(false), nActLevel(USHRT_MAX)
        {
        }

        NumberingPreview(vcl::Window* pParent)
            : Window(pParent)
            , pActNum(nullptr),nPageWidth(0), pOutlineNames(nullptr),
            bPosition(false), nActLevel(USHRT_MAX)
        {
        }

        virtual ~NumberingPreview();

        void    SetNumRule(const SwNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const OUString* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = true;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
