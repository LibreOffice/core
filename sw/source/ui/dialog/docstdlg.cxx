/*************************************************************************
 *
 *  $RCSfile: docstdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 14:44:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <stdio.h>
#include <ctype.h>

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif

#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSTDLG_HXX
#include <docstdlg.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

// fuer Statistikfelder
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif

#ifndef _DOCSTDLG_HRC
#include <docstdlg.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung: Create
 --------------------------------------------------------------------*/


SfxTabPage *  SwDocStatPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwDocStatPage(pParent, rSet);
}

/*--------------------------------------------------------------------
    Beschreibung:   Ctor
 --------------------------------------------------------------------*/


SwDocStatPage::SwDocStatPage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage  (pParent, SW_RES(TP_DOC_STAT), rSet),

    aTableLbl   (this, SW_RES( FT_TABLE      )),
    aGrfLbl     (this, SW_RES( FT_GRF        )),
    aOLELbl     (this, SW_RES( FT_OLE        )),
    aPageLbl    (this, SW_RES( FT_PAGE       )),
    aParaLbl    (this, SW_RES( FT_PARA       )),
    aWordLbl    (this, SW_RES( FT_WORD       )),
    aWordDelimFT(this, SW_RES( FT_WORD_DELIM )),
    aWordDelim  (this, SW_RES( ED_WORD_DELIM )),
    aCharLbl    (this, SW_RES( FT_CHAR       )),
    aTableNo    (this, SW_RES( FT_TABLE_COUNT)),
    aGrfNo      (this, SW_RES( FT_GRF_COUNT  )),
    aOLENo      (this, SW_RES( FT_OLE_COUNT  )),
    aPageNo     (this, SW_RES( FT_PAGE_COUNT )),
    aParaNo     (this, SW_RES( FT_PARA_COUNT )),
    aWordNo     (this, SW_RES( FT_WORD_COUNT )),
    aCharNo     (this, SW_RES( FT_CHAR_COUNT )),
    aLineLbl    (this, SW_RES( FT_LINE       )),
    aLineNo     (this, SW_RES( FT_LINE_COUNT )),
    aUpdatePB   (this, SW_RES( PB_PDATE      ))
{
    Update();
    FreeResource();
    aUpdatePB.SetClickHdl(LINK(this, SwDocStatPage, UpdateHdl));
}


 SwDocStatPage::~SwDocStatPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   ItemSet fuellen bei Aenderung
 --------------------------------------------------------------------*/


BOOL  SwDocStatPage::FillItemSet(SfxItemSet &rSet)
{
    // evtl UserData setzen

    String sEd(aWordDelim.GetText());
    String sDelim;
    String sChar;

    xub_StrLen i = 0;
    sal_Unicode c;

    while (i < sEd.Len())
    {
        c = sEd.GetChar(i++);

        if (c == '\\')
        {
            c = sEd.GetChar(i++);

            switch (c)
            {
                case 'n':   sDelim += '\n'; break;
                case 't':   sDelim += '\t'; break;
                case '\\':  sDelim += '\\'; break;

                case 'x':
                {
                    sal_Unicode nVal, nChar;
                    BOOL bValidData = TRUE;
                    xub_StrLen n;
                    for( n = 0, nChar = 0; n < 2 && i < sEd.Len(); ++n, ++i )
                    {
                        if( ((nVal = sEd.GetChar( i )) >= '0') && ( nVal <= '9') )
                            nVal -= '0';
                        else if( (nVal >= 'A') && (nVal <= 'F') )
                            nVal -= 'A' - 10;
                        else if( (nVal >= 'a') && (nVal <= 'f') )
                            nVal -= 'a' - 10;
                        else
                        {
                            DBG_ASSERT( !this, "ungueltiger Hex-Wert" );
                            bValidData = FALSE;
                            break;
                        }

                        (nChar <<= 4 ) += nVal;
                    }
                    if( bValidData )
                        sDelim += nChar;
                    break;
                }

                default:    // Unbekannt, daher nur Backslash einfuegen
                    sDelim += '\\';
                    i--;
                    break;
            }
        }
        else
            sDelim += c;
    }


    SW_MOD()->GetModuleConfig()->SetWordDelimiter(sDelim);

    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  SwDocStatPage::Reset(const SfxItemSet &rSet)
{
    // Im Set befindet sich die DocInfo
    // bei Bedarf UserData auswerten
    String sDelim(SW_MOD()->GetDocStatWordDelim());
    String sEd;
    for (xub_StrLen i = 0; i < sDelim.Len(); i++)
    {
        sal_Unicode c = sDelim.GetChar(i);

        switch (c)
        {
            case '\n':  sEd.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\n")); break;
            case '\t':  sEd.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\t")); break;
            case '\\':  sEd.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\\\"));    break;

            default:
                if( c <= 0x1f || c >= 0x7f )
                {
                    sEd.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\\x" ))
                        += String::CreateFromInt32( c, 16 );
                }
                else
                    sEd += c;
        }
    }
    aWordDelim.SetText(sEd);
    aWordDelim.ClearModifyFlag();
}


/*------------------------------------------------------------------------
 Beschreibung:  Aktualisieren / Setzen der Daten
------------------------------------------------------------------------*/


void SwDocStatPage::SetData(const SwDocStat &rStat)
{
    aTableNo.SetText(String::CreateFromInt32( rStat.nTbl ));
    aGrfNo.SetText(String::CreateFromInt32( rStat.nGrf ));
    aOLENo.SetText(String::CreateFromInt32( rStat.nOLE ));
    aPageNo.SetText(String::CreateFromInt32( rStat.nPage ));
    aParaNo.SetText(String::CreateFromInt32( rStat.nPara ));
    aWordNo.SetText(String::CreateFromInt32( rStat.nWord ));
    aCharNo.SetText(String::CreateFromInt32( rStat.nChar ));
}

/*------------------------------------------------------------------------
 Beschreibung:  Aktualisieren der Statistik
------------------------------------------------------------------------*/


void SwDocStatPage::Update()
{
    SfxViewShell *pVSh = SfxViewShell::Current();
    ViewShell *pSh = 0;
    if ( pVSh->ISA(SwView) )
        pSh = ((SwView*)pVSh)->GetWrtShellPtr();
    else if ( pVSh->ISA(SwPagePreView) )
        pSh = &((SwPagePreView*)pVSh)->GetViewShell();

    ASSERT( pSh, "Shell not found" );

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), TRUE );
    pSh->StartAction();
    pSh->GetDoc()->UpdateDocStat( aDocStat );
    pSh->EndAction();

    SetData(aDocStat);
}

/*-----------------19.06.97 16.37-------------------
    Zeilennummer aktualisieren
--------------------------------------------------*/
IMPL_LINK( SwDocStatPage, UpdateHdl, PushButton*, pButton)
{
    FillItemSet(*(SfxItemSet *)0);  // Worttrenner setzen
    aDocStat.bModified |= aWordDelim.IsModified();
    aWordDelim.ClearModifyFlag();
    Update();
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    aLineNo.SetText( String::CreateFromInt32( pFEShell->GetLineCount(FALSE)));
    //pButton->Disable();
    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:34  hr
    initial import

    Revision 1.43  2000/09/18 16:05:20  willem.vandorp
    OpenOffice header added.

    Revision 1.42  2000/05/22 16:26:16  jp
    Changes for Unicode

    Revision 1.41  2000/04/13 08:01:20  os
    UNICODE

    Revision 1.40  1999/07/08 17:16:10  MA
    Use internal object to toggle wait cursor


      Rev 1.39   08 Jul 1999 19:16:10   MA
   Use internal object to toggle wait cursor

      Rev 1.38   26 May 1998 12:04:42   OM
   #50480 Worttrenner aktualisieren

      Rev 1.37   15 May 1998 12:48:22   OM
   Worttrenner

      Rev 1.36   14 May 1998 16:46:54   OM
   Worttrenner konfigurierbar

      Rev 1.35   01 Sep 1997 13:15:08   OS
   DLL-Umstellung

      Rev 1.34   27 Aug 1997 09:53:10   MH
   chg: header

      Rev 1.33   26 Aug 1997 14:53:38   TRI
   VCL includes

      Rev 1.32   23 Jun 1997 10:57:00   OS
   LineCount: nicht die akt. Psition

      Rev 1.31   20 Jun 1997 16:46:58   HJS
   sexport -> __export

      Rev 1.30   20 Jun 1997 16:41:40   HJS
   sexport -> __export

      Rev 1.29   19 Jun 1997 17:29:00   OS
   Zeilen zaehlen

      Rev 1.28   11 Nov 1996 09:31:16   MA
   ResMgr

      Rev 1.27   01 Nov 1996 10:32:20   OM
   Statistik-TP auf Standardgroesse gebracht

      Rev 1.26   28 Aug 1996 10:10:00   OS
   includes

      Rev 1.25   02 Jul 1996 18:43:38   MA
   Wait-Umstellung 325

      Rev 1.24   19 Jan 1996 20:07:42   MA
   fix#24463# UpdateDocStat in Seitenansicht

      Rev 1.23   24 Nov 1995 16:58:16   OM
   PCH->PRECOMPILED

      Rev 1.22   26 Feb 1995 15:37:28   MA
   fix: SEXPORT nachgeruestet.

      Rev 1.21   24 Jan 1995 12:49:46   SWG
   Map entfernt

      Rev 1.20   18 Jan 1995 19:12:48   ER
   fld.hxx -> *fld*.hxx

      Rev 1.19   09 Dec 1994 18:26:00   MS
   DocStat geaendert

      Rev 1.18   07 Dec 1994 17:00:20   MS
   DocStat -> TabPage

      Rev 1.17   25 Oct 1994 16:31:12   ER
   add: PCH

      Rev 1.16   17 Oct 1994 16:26:38   PK
   ausgeboxtes wieder reingeboxt


------------------------------------------------------------------------*/


