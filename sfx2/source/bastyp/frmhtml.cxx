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


#include <svtools/htmltokn.h>
#include <svtools/asynclink.hxx>

#include <sfx2/sfx.hrc>

#include <sfx2/frmhtml.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include "sfxtypes.hxx"

static sal_Char const sHTML_SC_yes[] =  "YES";
static sal_Char const sHTML_SC_no[] =       "NO";
static sal_Char const sHTML_SC_auto[] = "AUTO";

#define HTML_O_READONLY "READONLY"
#define HTML_O_EDIT     "EDIT"

static HTMLOptionEnum const aScollingTable[] =
{
    { sHTML_SC_yes,     ScrollingYes    },
    { sHTML_SC_no,      ScrollingNo     },
    { sHTML_SC_auto,    ScrollingAuto   },
    { 0,                0               }
};

void SfxFrameHTMLParser::ParseFrameOptions(
    SfxFrameDescriptor *pFrame, const HTMLOptions& rOptions, const String& rBaseURL )
{
    // Get and set the options
    Size aMargin( pFrame->GetMargin() );

    // Netscape seems to set marginwidth to 0 as soon as
    // marginheight is set, and vice versa.
    // Netscape does however not allow for a direct
    // seting to 0, while IE4.0 does
    // We will not mimic that bug !
    sal_Bool bMarginWidth = sal_False, bMarginHeight = sal_False;

    for (size_t i = 0, n = rOptions.size(); i < n; ++i)
    {
        const HTMLOption& aOption = rOptions[i];
        switch( aOption.GetToken() )
        {
        case HTML_O_BORDERCOLOR:
            {
                Color aColor;
                aOption.GetColor( aColor );
                pFrame->SetWallpaper( Wallpaper( aColor ) );
                break;
            }
        case HTML_O_SRC:
            pFrame->SetURL(
                String(
                    INetURLObject::GetAbsURL(
                        rBaseURL, aOption.GetString())) );
            break;
        case HTML_O_NAME:
            pFrame->SetName( aOption.GetString() );
            break;
        case HTML_O_MARGINWIDTH:
            aMargin.Width() = aOption.GetNumber();

            if( !bMarginHeight )
                aMargin.Height() = 0;
            bMarginWidth = sal_True;
            break;
        case HTML_O_MARGINHEIGHT:
            aMargin.Height() = aOption.GetNumber();

            if( !bMarginWidth )
                aMargin.Width() = 0;
            bMarginHeight = sal_True;
            break;
        case HTML_O_SCROLLING:
            pFrame->SetScrollingMode(
                (ScrollingMode)aOption.GetEnum( aScollingTable,
                                                 ScrollingAuto ) );
            break;
        case HTML_O_FRAMEBORDER:
        {
            String aStr = aOption.GetString();
            sal_Bool bBorder = sal_True;
            if ( aStr.EqualsIgnoreCaseAscii("NO") ||
                 aStr.EqualsIgnoreCaseAscii("0") )
                bBorder = sal_False;
            pFrame->SetFrameBorder( bBorder );
            break;
        }
        case HTML_O_NORESIZE:
            pFrame->SetResizable( sal_False );
            break;
        default:
            if (aOption.GetTokenString().equalsIgnoreAsciiCase(HTML_O_READONLY))
            {
                String aStr = aOption.GetString();
                sal_Bool bReadonly = sal_True;
                if ( aStr.EqualsIgnoreCaseAscii("FALSE") )
                    bReadonly = sal_False;
                pFrame->SetReadOnly( bReadonly );
            }
            else if (aOption.GetTokenString().equalsIgnoreAsciiCase(HTML_O_EDIT))
            {
                String aStr = aOption.GetString();
                sal_Bool bEdit = sal_True;
                if ( aStr.EqualsIgnoreCaseAscii("FALSE") )
                    bEdit = sal_False;
                pFrame->SetEditable( bEdit );
            }

            break;
        }
    }

    pFrame->SetMargin( aMargin );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
