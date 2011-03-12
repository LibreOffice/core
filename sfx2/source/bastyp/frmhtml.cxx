/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <tools/table.hxx>
#include <svtools/htmltokn.h>
#include <svtools/asynclink.hxx>

#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>

#include <sfx2/sfx.hrc>

#include <sfx2/frmhtml.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include "sfxtypes.hxx"

#define SFX_HTMLFRMSIZE_REL 0x0001
#define SFX_HTMLFRMSIZE_PERCENT 0x0002

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

void SfxFrameHTMLParser::ParseFrameOptions( SfxFrameDescriptor *pFrame, const HTMLOptions *pOptions, const String& rBaseURL )
{
    // Get and set the options
    Size aMargin( pFrame->GetMargin() );

    // MIB 07/15/1997: Netscape seems to set marginwidth to 0 as soon as
    // marginheight is set, and vice versa. Due to bug #41665# let us also
    // do like that for now. Netscape does however not aallow for a direct
    // seting to 0, while IE4.0 does
    // We will not mimic that bug !
    sal_Bool bMarginWidth = sal_False, bMarginHeight = sal_False;

    sal_uInt16 nArrLen = pOptions->Count();
    for ( sal_uInt16 i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_BORDERCOLOR:
            {
                Color aColor;
                pOption->GetColor( aColor );
                pFrame->SetWallpaper( Wallpaper( aColor ) );
                break;
            }
        case HTML_O_SRC:
            pFrame->SetURL(
                String(
                    INetURLObject::GetAbsURL(
                        rBaseURL, pOption->GetString())) );
            break;
        case HTML_O_NAME:
            pFrame->SetName( pOption->GetString() );
            break;
        case HTML_O_MARGINWIDTH:
            aMargin.Width() = pOption->GetNumber();

            if( !bMarginHeight )
                aMargin.Height() = 0;
            bMarginWidth = sal_True;
            break;
        case HTML_O_MARGINHEIGHT:
            aMargin.Height() = pOption->GetNumber();

            if( !bMarginWidth )
                aMargin.Width() = 0;
            bMarginHeight = sal_True;
            break;
        case HTML_O_SCROLLING:
            pFrame->SetScrollingMode(
                (ScrollingMode)pOption->GetEnum( aScollingTable,
                                                 ScrollingAuto ) );
            break;
        case HTML_O_FRAMEBORDER:
        {
            String aStr = pOption->GetString();
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
            if ( pOption->GetTokenString().EqualsIgnoreCaseAscii(
                                                        HTML_O_READONLY ) )
            {
                String aStr = pOption->GetString();
                sal_Bool bReadonly = sal_True;
                if ( aStr.EqualsIgnoreCaseAscii("FALSE") )
                    bReadonly = sal_False;
                pFrame->SetReadOnly( bReadonly );
            }
            else if ( pOption->GetTokenString().EqualsIgnoreCaseAscii(
                                                        HTML_O_EDIT ) )
            {
                String aStr = pOption->GetString();
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
