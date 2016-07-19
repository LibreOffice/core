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

#include "hintwin.hxx"
#include "global.hxx"

#define HINT_LINESPACE  2
#define HINT_INDENT     3
#define HINT_MARGIN     4

ScHintWindow::ScHintWindow( vcl::Window* pParent, const OUString& rTit, const OUString& rMsg ) :
    Window( pParent, WinBits( WB_BORDER ) ),
    aTitle( rTit ),
    aMessage( convertLineEnd(rMsg, LINEEND_CR) )
{
    // pale yellow, the same as for notes in detfunc.cxx
    Color aYellow( 255,255,192 );           // pale yellow
    SetBackground( aYellow );

    aTextFont = GetFont();
    aTextFont.SetTransparent( true );
    aTextFont.SetWeight( WEIGHT_NORMAL );
    aHeadFont = aTextFont;
    aHeadFont.SetWeight( WEIGHT_BOLD );

    SetFont( aHeadFont );
    Size aHeadSize( GetTextWidth( aTitle ), GetTextHeight() );
    SetFont( aTextFont );

    Size aTextSize;
    sal_Int32 nIndex = 0;
    while ( nIndex != -1 )
    {
        OUString aLine = aMessage.getToken( 0, '\r', nIndex );
        Size aLineSize( GetTextWidth( aLine ), GetTextHeight() );
        nTextHeight = aLineSize.Height();
        aTextSize.Height() += nTextHeight;
        if ( aLineSize.Width() > aTextSize.Width() )
            aTextSize.Width() = aLineSize.Width();
    }
    aTextSize.Width() += HINT_INDENT;

    aTextStart = Point( HINT_MARGIN + HINT_INDENT,
                        aHeadSize.Height() + HINT_MARGIN + HINT_LINESPACE );

    Size aWinSize( std::max( aHeadSize.Width(), aTextSize.Width() ) + 2 * HINT_MARGIN + 1,
                    aHeadSize.Height() + aTextSize.Height() + HINT_LINESPACE + 2 * HINT_MARGIN + 1 );
    SetOutputSizePixel( aWinSize );
}

ScHintWindow::~ScHintWindow()
{
}

void ScHintWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& /* rRect */ )
{
    SetFont( aHeadFont );
    DrawText( Point(HINT_MARGIN,HINT_MARGIN), aTitle );

    SetFont( aTextFont );
    sal_Int32 nIndex = 0;
    Point aLineStart = aTextStart;
    while ( nIndex != -1 )
    {
        OUString aLine = aMessage.getToken( 0, '\r', nIndex );
        DrawText( aLineStart, aLine );
        aLineStart.Y() += nTextHeight;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
