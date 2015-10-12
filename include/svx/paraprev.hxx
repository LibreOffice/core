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
#ifndef INCLUDED_SVX_PARAPREV_HXX
#define INCLUDED_SVX_PARAPREV_HXX

#include <vcl/window.hxx>
#include <editeng/svxenum.hxx>
#include <svx/svxdllapi.h>

enum SvxPrevLineSpace
{
    SVX_PREV_LINESPACE_1 = 0,
    SVX_PREV_LINESPACE_15,
    SVX_PREV_LINESPACE_2,
    SVX_PREV_LINESPACE_PROP,
    SVX_PREV_LINESPACE_MIN,
    SVX_PREV_LINESPACE_DURCH
};

class SVX_DLLPUBLIC SvxParaPrevWindow : public vcl::Window
{
    using Window::Draw;
private:
    Size                aSize;

    // indentation
    long                nLeftMargin;
    long                nRightMargin;
    short               nFirstLineOfst;
    // distances
    sal_uInt16          nUpper;
    sal_uInt16          nLower;
    // adjustment
    SvxAdjust           eAdjust;
    // last line in justification
    SvxAdjust           eLastLine;
    // line distance
    SvxPrevLineSpace    eLine;
    sal_uInt16          nLineVal;

    OUString            aText;
    Rectangle           Lines[9];

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual Size GetOptimalSize() const override;

    void DrawParagraph(vcl::RenderContext& rRenderContext, bool bAll);

public:
    SvxParaPrevWindow( vcl::Window* pParent, WinBits nBits);

    void SetFirstLineOfst( short nNew )
    {
        nFirstLineOfst = nNew;
    }
    void SetLeftMargin( long nNew )
    {
        nLeftMargin = nNew;
    }
    void SetRightMargin( long nNew )
    {
        nRightMargin = nNew;
    }
    void SetUpper( sal_uInt16 nNew )
    {
        nUpper = nNew;
    }
    void SetLower( sal_uInt16 nNew )
    {
        nLower = nNew;
    }
    void SetAdjust( SvxAdjust eNew )
    {
        eAdjust = eNew;
    }
    void SetLastLine( SvxAdjust eNew )
    {
        eLastLine = eNew;
    }
    void SetLineSpace( SvxPrevLineSpace eNew, sal_uInt16 nNew = 0 )
    {
        eLine = eNew; nLineVal = nNew;
    }
    void SetText( const OUString& rStr ) override
    {
        aText = rStr;
    }
    void SetSize( Size aNew )
    {
        aSize = aNew;
    }
    OUString GetText() const override
    {
        return aText;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
