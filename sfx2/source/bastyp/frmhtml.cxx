/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <tools/list.hxx>
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

static sal_Char __READONLY_DATA sHTML_SC_yes[] =    "YES";
static sal_Char __READONLY_DATA sHTML_SC_no[] =     "NO";
static sal_Char __READONLY_DATA sHTML_SC_auto[] =   "AUTO";

#define HTML_O_READONLY "READONLY"
#define HTML_O_EDIT     "EDIT"

static HTMLOptionEnum __READONLY_DATA aScollingTable[] =
{
    { sHTML_SC_yes,     ScrollingYes    },
    { sHTML_SC_no,      ScrollingNo     },
    { sHTML_SC_auto,    ScrollingAuto   },
    { 0,                0               }
};

void SfxFrameHTMLParser::ParseFrameOptions( SfxFrameDescriptor *pFrame, const HTMLOptions *pOptions, const String& rBaseURL )
{
    // die Optionen holen und setzen
    Size aMargin( pFrame->GetMargin() );

    // MIB 15.7.97: Netscape scheint marginwidth auf 0 zu setzen, sobald
    // marginheight gesetzt wird und umgekehrt. Machen wir jetzt wegen
    // bug #41665# auch so.
    // Netscape l"a\st aber ein direktes Setzen auf 0 nicht zu, IE4.0 schon.
    // Den Bug machen wir nicht mit!
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

//          if( aMargin.Width() < 1 )
//              aMargin.Width() = 1;
            if( !bMarginHeight )
                aMargin.Height() = 0;
            bMarginWidth = sal_True;
            break;
        case HTML_O_MARGINHEIGHT:
            aMargin.Height() = pOption->GetNumber();

//          if( aMargin.Height() < 1 )
//              aMargin.Height() = 1;
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
