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

#define HTML_O_EDIT     "EDIT"

static HTMLOptionEnum<ScrollingMode> const aScrollingTable[] =
{
    { sHTML_SC_yes,     ScrollingMode::Yes    },
    { sHTML_SC_no,      ScrollingMode::No     },
    { sHTML_SC_auto,    ScrollingMode::Auto   },
    { nullptr,          (ScrollingMode)0 }
};

void SfxFrameHTMLParser::ParseFrameOptions(
    SfxFrameDescriptor *pFrame, const HTMLOptions& rOptions, const OUString& rBaseURL )
{
    // Get and set the options
    Size aMargin( pFrame->GetMargin() );

    // Netscape seems to set marginwidth to 0 as soon as
    // marginheight is set, and vice versa.
    // Netscape does however not allow for a direct
    // setting to 0, while IE4.0 does
    // We will not mimic that bug !
    bool bMarginWidth = false, bMarginHeight = false;

    for (const auto & rOption : rOptions)
    {
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::BORDERCOLOR:
            {
                Color aColor;
                rOption.GetColor( aColor );
                pFrame->SetWallpaper( Wallpaper( aColor ) );
                break;
            }
        case HtmlOptionId::SRC:
            pFrame->SetURL(
                    INetURLObject::GetAbsURL(
                        rBaseURL, rOption.GetString()) );
            break;
        case HtmlOptionId::NAME:
            pFrame->SetName( rOption.GetString() );
            break;
        case HtmlOptionId::MARGINWIDTH:
            aMargin.Width() = rOption.GetNumber();

            if( !bMarginHeight )
                aMargin.Height() = 0;
            bMarginWidth = true;
            break;
        case HtmlOptionId::MARGINHEIGHT:
            aMargin.Height() = rOption.GetNumber();

            if( !bMarginWidth )
                aMargin.Width() = 0;
            bMarginHeight = true;
            break;
        case HtmlOptionId::SCROLLING:
            pFrame->SetScrollingMode( rOption.GetEnum( aScrollingTable, ScrollingMode::Auto ) );
            break;
        case HtmlOptionId::FRAMEBORDER:
        {
            const OUString& aStr = rOption.GetString();
            bool bBorder = true;
            if ( aStr.equalsIgnoreAsciiCase("NO") ||
                 aStr.equalsIgnoreAsciiCase("0") )
                bBorder = false;
            pFrame->SetFrameBorder( bBorder );
            break;
        }
        case HtmlOptionId::NORESIZE:
            pFrame->SetResizable( false );
            break;
        default:
            if (rOption.GetTokenString().equalsIgnoreAsciiCase(HTML_O_EDIT))
            {
                const OUString& aStr = rOption.GetString();
                bool bEdit = true;
                if ( aStr.equalsIgnoreAsciiCase("FALSE") )
                    bEdit = false;
                pFrame->SetEditable( bEdit );
            }

            break;
        }
    }

    pFrame->SetMargin( aMargin );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
