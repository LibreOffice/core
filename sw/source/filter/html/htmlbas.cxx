/*************************************************************************
 *
 *  $RCSfile: htmlbas.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX //autogen
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif



#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif

#ifndef _SFX_OBJUNO_HXX //autogen
#include <sfx2/objuno.hxx>
#endif

#include "doc.hxx"
#include "docsh.hxx"
#include "docufld.hxx"
#include "wrthtml.hxx"
#include "swhtml.hxx"


static HTMLOutEvent __FAR_DATA aBodyEventTable[] =
{
    { sHTML_O_SDonload,     sHTML_O_onload,     SFX_EVENT_OPENDOC   },
    { sHTML_O_SDonunload,   sHTML_O_onunload,   SFX_EVENT_PREPARECLOSEDOC   },
    { sHTML_O_SDonfocus,    sHTML_O_onfocus,    SFX_EVENT_ACTIVATEDOC   },
    { sHTML_O_SDonblur,     sHTML_O_onblur,     SFX_EVENT_DEACTIVATEDOC },
    { 0,                    0,                  0                   }
};


void SwHTMLParser::NewScript()
{
    ParseScriptOptions( aScriptType, eScriptLang, aScriptURL,
                        aBasicLib, aBasicModule );

    BOOL bDownload = aScriptURL.Len();
    if( bDownload )
    {
        // Den Inhalt des Script-Tags ignorieren
        bIgnoreRawData = TRUE;
    }
}

void SwHTMLParser::EndScript()
{
    BOOL bInsIntoBasic = FALSE,
         bInsSrcIntoFld = FALSE;

    switch( eScriptLang )
    {
    case HTML_SL_STARBASIC:
        bInsIntoBasic = TRUE;
        break;
    default:
        bInsSrcIntoFld = TRUE;
        break;
    }


    bIgnoreRawData = FALSE;
    aScriptSource.ConvertLineEnd();

//  MIB 23.5.97: SGML-Kommentare brauchen nicht mehr entfernt zu werden,
//  weil JS das jetzt selber kann.
//  RemoveSGMLComment( aScriptSource, TRUE );

    // Ausser StarBasic und unbenutzem JavaScript jedes Script oder den
    // Modulnamen in einem Feld merken merken
    if( bInsSrcIntoFld )
    {
        SwScriptFieldType *pType =
            (SwScriptFieldType*)pDoc->GetSysFldType( RES_SCRIPTFLD );

        SwScriptField aFld( pType, aScriptType,
                            aScriptURL.Len() ? aScriptURL : aScriptSource,
                            aScriptURL.Len()!=0 );
        InsertAttr( SwFmtFld( aFld ) );
    }

    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( aScriptSource.Len() && pDocSh &&
        bInsIntoBasic && IsNewDoc() )
    {
    // Fuer JavaScript und StarBasic noch ein Basic-Modul anlegen
        // Das Basic entfernt natuerlich weiterhin keine SGML-Kommentare
        RemoveSGMLComment( aScriptSource, TRUE );

        SFX_APP()->EnterBasicCall();

        BasicManager *pBasicMan = pDocSh->GetBasicManager();
        ASSERT( pBasicMan, "Wo ist der BasicManager?" );
        if( pBasicMan )
        {
            StarBASIC *pBasic;
            if( aBasicLib.Len() )
            {
                pBasic = pBasicMan->GetLib( aBasicLib );
                if( !pBasic )
                    pBasic = pBasicMan->CreateLib( aBasicLib );
            }
            else
                pBasic = pBasicMan->GetStdLib();

            if( pBasic )
            {
                if( !aBasicModule.Len() )
                {
                    BOOL bFound = TRUE;
                    while( bFound )
                    {
                        aBasicModule.AssignAscii( "Modul" );
                        aBasicModule += String::CreateFromInt32(
                                            (sal_Int32)(++nSBModuleCnt) );
                        bFound = (pBasic->FindModule( aBasicModule ) != 0);
                    }
                }

                SbModule *pModule =
                    pBasic->MakeModule( aBasicModule, aScriptSource );
                if( pModule )
                    pModule->Compile();
            }
        }

        SFX_APP()->LeaveBasicCall();
    }

    aScriptSource.Erase();
    aScriptType.Erase();
    aScriptURL.Erase();

    aBasicLib.Erase();
    aBasicModule.Erase();
}

void SwHTMLParser::AddScriptSource()
{
    // Hier merken wir und nur ein par Strings
    if( aToken.Len() > 2 &&
        (HTML_SL_STARBASIC==eScriptLang && aToken.GetChar( 0 ) == '\'') )
    {
        xub_StrLen nPos = STRING_NOTFOUND;
        if( !aBasicLib.Len() )
        {
            nPos = aToken.SearchAscii( sHTML_SB_library );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicLib =
                    aToken.Copy( nPos + sizeof(sHTML_SB_library) - 1 );
                aBasicLib.EraseLeadingChars().EraseTrailingChars();
            }
        }

        if( !aBasicModule.Len() && nPos==STRING_NOTFOUND )
        {
            nPos = aToken.SearchAscii( sHTML_SB_module );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicModule =
                    aToken.Copy( nPos + sizeof(sHTML_SB_module) - 1 );
                aBasicModule.EraseLeadingChars().EraseTrailingChars();
            }
        }

        if( nPos==STRING_NOTFOUND )
        {
            if( aScriptSource.Len() )
                aScriptSource += '\n';
            (aScriptSource += aToken);
        }
    }
    else if( aScriptSource.Len() || aToken.Len() )
    {
        // Leerzeilen am Anfang werden ignoriert
        if( aScriptSource.Len() )
        {
            aScriptSource += '\n';
        }
        else
        {
            // Wir stehen hinter dem CR/LF der Zeile davor
            nScriptStartLineNr = GetLineNr() - 1;
        }
        aScriptSource += aToken;
    }
}

void SwHTMLParser::InsertBasicDocEvent( USHORT nEvent, const String& rName,
                                        ScriptType eScrType,
                                        const String& rScrType )
{
    ASSERT( rName.Len(), "InsertBasicDocEvent() ohne Macro gerufen" );
    if( !rName.Len() )
        return;

    SwDocShell *pDocSh = pDoc->GetDocShell();
    ASSERT( pDocSh, "Wo ist die DocShell?" );
    if( !pDocSh )
        return;

    String sEvent( rName );
    sEvent.ConvertLineEnd();
    String sScriptType;
    if( EXTENDED_STYPE == eScrType )
        sScriptType = rScrType;

    SfxEventConfiguration* pECfg = SFX_APP()->GetEventConfig();
    pECfg->ConfigureEvent( nEvent, SvxMacro( sEvent, sScriptType, eScrType ),
                           pDocSh );
}

void SwHTMLWriter::OutBasic()
{
    if( !bCfgStarBasic )
        return;

    SFX_APP()->EnterBasicCall();

    BasicManager *pBasicMan = pDoc->GetDocShell()->GetBasicManager();
    ASSERT( pBasicMan, "Wo ist der Basic-Manager?" );
    //JP 17.07.96: Bug 29538 - nur das DocumentBasic schreiben
    if( !pBasicMan || pBasicMan == SFX_APP()->GetBasicManager() )
    {
        SFX_APP()->LeaveBasicCall();
        return;
    }

    // und jetzt alle StarBasic-Module und alle unbenutzen JavaSrript-Module
    // ausgeben
    for( USHORT i=0; i<pBasicMan->GetLibCount(); i++ )
    {
        StarBASIC *pBasic = pBasicMan->GetLib( i  );
        const String& rLibName = pBasic->GetName();

        SbxArray *pModules = pBasic->GetModules();
        for( USHORT j=0; j<pModules->Count(); j++ )
        {
            const SbModule *pModule = PTR_CAST( SbModule, pModules->Get(j) );
            ASSERT( pModule, "Wo ist das Modul?" );

            String sLang(
                    String::CreateFromAscii( SVX_MACRO_LANGUAGE_STARBASIC ) );
            ScriptType eType = STARBASIC;

            if( 0==i && 0==j )
            {
                OutNewLine();
                ByteString sOut( '<' );
                sOut.Append( sHTML_meta );
                sOut.Append( ' ' );
                sOut.Append( sHTML_O_httpequiv );
                  sOut.Append( "=\"" );
                sOut.Append( sHTML_META_content_script_type );
                sOut.Append( "\" " );
                sOut.Append( sHTML_O_content );
                sOut.Append( "=\"text/x-" );
                Strm() << sOut.GetBuffer();
                // Entities aren't welcome here
                ByteString sLang8( sLang, eDestEnc );
                Strm() << sLang8.GetBuffer() << "\">";
            }

            const String& rModName = pModule->GetName();
            Strm() << SwHTMLWriter::sNewLine;   // nicht einruecken!
            HTMLOutFuncs::OutScript( Strm(), pModule->GetSource(),
                                     sLang, eType, aEmptyStr,
                                     &rLibName, &rModName,
                                     eDestEnc );
        }
    }

    SFX_APP()->LeaveBasicCall();
}


void SwHTMLWriter::OutBasicBodyEvents()
{
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !pDocSh )
        return;

    SfxEventConfiguration* pECfg = SFX_APP()->GetEventConfig();
    ASSERT( pECfg, "Wo ist die Event-Konfiguration?" );
    if( !pECfg )
        return;

    SvxMacroTableDtor *pMacTable = pECfg->GetDocEventTable( pDocSh );
    if( pMacTable && pMacTable->Count() )
        HTMLOutFuncs::Out_Events( Strm(), *pMacTable, aBodyEventTable,
                                  bCfgStarBasic, eDestEnc );
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlbas.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.92  2000/09/18 16:04:43  willem.vandorp
      OpenOffice header added.

      Revision 1.91  2000/08/24 20:17:16  jp
      Bug #78047#: EndScript - no assert

      Revision 1.90  2000/05/15 10:07:15  os
      Basic removed

      Revision 1.89  2000/04/28 14:29:11  mib
      unicode

      Revision 1.88  2000/04/10 12:20:55  mib
      unicode

      Revision 1.87  2000/03/30 07:31:01  os
      UNO III

      Revision 1.86  2000/03/16 16:40:32  kz
      remove include sbjsmod.hxx

      Revision 1.85  2000/03/03 12:44:32  mib
      Removed JavaScript

      Revision 1.84  1999/11/19 16:40:19  os
      modules renamed

      Revision 1.83  1999/09/21 09:48:47  mib
      multiple text encodings

      Revision 1.82  1999/09/17 12:12:45  mib
      support of multiple and non system text encodings

      Revision 1.81  1999/04/27 14:47:32  JP
      Bug #65314#: neu: IsJavaScriptEnabled


      Rev 1.80   27 Apr 1999 16:47:32   JP
   Bug #65314#: neu: IsJavaScriptEnabled

      Rev 1.79   27 Jan 1999 09:43:32   OS
   #56371# TF_ONE51

      Rev 1.78   22 Sep 1998 13:46:50   MIB
   #56943#: Bei String-Ueberlauf im JavaScript Fehlermeldung ausgeben

      Rev 1.77   11 Sep 1998 15:04:50   MIB
   generische Events wieder fuer alle Script-Typen erlauben

      Rev 1.76   09 Sep 1998 17:58:10   MIB
   #55407#: Stat onclosedoc inprepareclodedoc fuer unload-Event benutzen

      Rev 1.75   28 Jul 1998 12:07:12   MIB
   #52559#: JavaScript-Zeilennummern

      Rev 1.74   27 Jul 1998 10:55:28   MIB
   #50320#: <SCRIPT SRC=...> wieder aktiviert

      Rev 1.73   08 Jul 1998 17:20:24   MIB
   StarScript

      Rev 1.72   04 Jul 1998 17:43:48   MIB
   StarScript in IDE, StarScript-Events

      Rev 1.71   05 May 1998 13:09:28   MIB
   Scripte aus ANSI-Zeichensetz konvertieren

      Rev 1.70   21 Apr 1998 11:21:46   RG
   String -> UString

      Rev 1.69   24 Mar 1998 12:36:18   MIB
   fix #47774#: Nur Dokument-Events exportieren

      Rev 1.68   20 Feb 1998 19:04:28   MA
   header

      Rev 1.67   20 Feb 1998 15:11:16   MIB
   include

      Rev 1.66   19 Feb 1998 16:15:02   MIB
   StarOne-TF: StarOne-Scripte

      Rev 1.65   26 Nov 1997 19:09:18   MA
   includes

      Rev 1.64   24 Nov 1997 13:49:26   TJ
   include svhtml.hxx

      Rev 1.63   21 Nov 1997 09:42:40   MIB
   fix #45422#: Dokument-Basic beim Basic-Import immer anlegen

      Rev 1.62   03 Nov 1997 14:08:42   MA
   precomp entfernt

      Rev 1.61   09 Oct 1997 14:33:50   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.60   16 Sep 1997 11:11:56   MIB
   unbenutzte Funktionen entfernt

      Rev 1.59   12 Sep 1997 11:51:58   MIB
   fix #41136#: &(xxx);-Makros

      Rev 1.58   01 Sep 1997 13:52:46   MIB
   Zeichensetz-/Zeilenende-Konvertierung fuer Event-Optionen

      Rev 1.57   29 Aug 1997 16:50:28   OS
   DLL-Umstellung

      Rev 1.56   15 Aug 1997 12:47:48   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.55   11 Aug 1997 14:05:04   OM
   Headerfile-Umstellung

      Rev 1.54   07 Aug 1997 15:08:56   OM
   Headerfile-Umstellung

      Rev 1.53   15 Jul 1997 14:07:20   MIB
   fix: Scripte in System-Zeichensatz wandeln

      Rev 1.52   02 Jul 1997 10:34:40   MIB
   Assert weg

      Rev 1.51   23 May 1997 16:04:34   MIB
   Nur noch SGML-Kommentare fuer StarBASIC entfernen

      Rev 1.50   22 May 1997 11:01:30   MIB
   URL-Flag am Script-Feld

      Rev 1.49   21 May 1997 11:15:04   MIB
   fix #39768#: <SCRIPT SRC=...> zum Eibindenen externer Scripte

      Rev 1.48   02 May 1997 20:53:00   NF
   includes weil SfxApplication unbekannt!

      Rev 1.47   11 Apr 1997 14:45:14   MIB
   fix: Kein Basic/JsvaScript beim einfuegen, JAVA-Basic-IDE-Stuff entfernt

      Rev 1.46   24 Mar 1997 17:41:36   MIB
   fix #37936#: SCRIPT in SELECT erlauben, opt: OutBasic aus Goodies nutzen

      Rev 1.45   19 Mar 1997 14:43:48   MIB
   fix #37904#: Vor --> ein // exportieren

      Rev 1.44   23 Feb 1997 14:03:04   MIB
   LiveScript wie JavaScript behandeln

      Rev 1.43   20 Feb 1997 11:04:52   MIB
   fix: Script-Sourcen fuer Nicht-JS immer loeschen

      Rev 1.42   11 Feb 1997 16:14:06   HJS
   fuer segprag

      Rev 1.41   11 Feb 1997 15:53:38   MIB
   fix #35907# Basic-Source-String loeschen

      Rev 1.40   10 Feb 1997 16:50:20   JP
   Umstellung JavaScript aufrufen und FrameSet Erkennung

      Rev 1.39   30 Jan 1997 17:31:38   MIB
   kein JavaScript mehr in Basic-Module

      Rev 1.38   23 Jan 1997 15:02:52   MIB
   falsches Assert weg

      Rev 1.37   22 Jan 1997 09:27:30   MIB
   Hack fuer Erkennung von unbenutztem JavaScript richtig gemacht

      Rev 1.36   21 Jan 1997 17:48:44   MIB
   Events ueber HTMLParser ausgeben

      Rev 1.35   17 Jan 1997 16:44:08   MIB
   ONFOCUS und ONBLUR am Body-Tag exportieren

      Rev 1.34   16 Jan 1997 19:23:26   MIB
   JavaScript in Basic-Modulen, unbenutztes JavaScript erkennen

      Rev 1.33   15 Jan 1997 15:34:22   MIB
   bug fix: auch JavaScript1.1 erkennen, Script-Typ erkennung des HTMLParser nutzen

      Rev 1.32   15 Jan 1997 14:22:40   MIB
   bug fix: Ohne Medium keine JavaScript laden

      Rev 1.31   13 Jan 1997 12:17:06   MIB
   JavaScript in Basic Modulen speichern (noch nicht aktiviert)

      Rev 1.30   08 Jan 1997 21:23:00   HJS
   includes

      Rev 1.29   19 Dec 1996 11:33:56   MIB
   JavaScript nicht ausfuehren, wenn die Filter-Detection das schon getan hat

      Rev 1.28   18 Dec 1996 14:49:52   MIB
   Macro-Unstellung am SFX/Script-Unterstuetung am HTMLParser

      Rev 1.27   16 Dec 1996 16:23:36   MIB
   Bug fix: Script-Felder und Kommentare vor Tabellen

      Rev 1.26   05 Dec 1996 14:44:24   JP
   Unterscheidung von Basic/JavaScript an den SvxMacros

      Rev 1.25   04 Dec 1996 15:01:16   JP
   SW_EVENT -> SVX_EVENT/SFX_EVENT

      Rev 1.24   01 Dec 1996 07:51:02   JP
   JavaScript bis zur 347 ausgebaut (MM)

      Rev 1.23   25 Nov 1996 15:19:14   JP
   InsertBasicModul: Kommentar an Anfang bis zum ZeilenUmbruch entfernen

      Rev 1.22   23 Nov 1996 13:37:54   JP
   InsertBasicINetFmtEvent entfernt; wurde ueberfluessig

      Rev 1.21   21 Nov 1996 16:29:10   JP
   Handler an den Anchor/Images immer speichern

      Rev 1.20   20 Nov 1996 15:09:52   JP
   Java ueber Eintrag in der Ini freischalten

      Rev 1.19   19 Nov 1996 14:06:18   JP
   ImageEvents schreiben

      Rev 1.18   15 Nov 1996 18:44:20   JP
   InsertAndCreate erstmal nur zum Testen

      Rev 1.17   15 Nov 1996 18:25:06   JP
   JavaScript einfuegen und laufen lassen

      Rev 1.16   13 Nov 1996 14:50:04   JP
   neu: ScriptFeld am Doc setzen

      Rev 1.15   23 Oct 1996 19:49:00   JP
   String Umstellung: [] -> GetChar()

      Rev 1.14   09 Oct 1996 13:40:12   MIB
   Pretty-Printing: Einrueckung auf Absatz-Ebene

      Rev 1.13   19 Sep 1996 12:16:00   MIB
   StarBasic-Export-Option beachten

      Rev 1.12   02 Sep 1996 18:40:18   JP
   INetFeld entfernt

      Rev 1.11   21 Aug 1996 16:55:26   MIB
   Probleme mit dem URLATR-define behoben

      Rev 1.10   21 Aug 1996 14:59:30   MIB
   Verwendung des URLATTR-defines vervollstaenigt

      Rev 1.9   15 Aug 1996 10:10:46   MIB
   URL-Attribut exportieren

      Rev 1.8   14 Aug 1996 16:22:22   MIB
   gcc und msvc: statt *aStr nicht aStr[0] sondern aStr[(USHORT)0]

      Rev 1.7   13 Aug 1996 20:33:14   sdo
   GCC

      Rev 1.6   06 Aug 1996 12:54:44   MIB
   Umstellung INet-Felder in INet-Attribut verbereitet

      Rev 1.5   02 Aug 1996 18:41:30   MIB
   Verwendung von pFlyFmt typsicherer gemacht = bug fix fuer Draw-Formate

      Rev 1.4   19 Jul 1996 15:08:12   JP
   Bug #29538#: nur das DocBasis schreiben

      Rev 1.3   01 Jul 1996 16:45:12   JP
   statt SFX_APP pApp benutzen, DocShell muss nicht vorhanden sein (Clipboard)

      Rev 1.2   27 Jun 1996 16:41:38   MIB
   Kommentare um Basic-Source verbessert entfernen, Includes aufgeraeumt

      Rev 1.1   07 Jun 1996 12:43:10   MIB
   Macros an INetField/NoTxtNode lesen und schreiben

      Rev 1.0   30 May 1996 15:02:46   MIB
   Initial revision.



*************************************************************************/


