/*************************************************************************
 *
 *  $RCSfile: initui.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:02:23 $
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

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _INITUI_HRC
#include <initui.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung:   globale Pointer
 --------------------------------------------------------------------*/

SwGlossaries*       pGlossaries = 0;

// Liefert alle benoetigten Pfade. Wird durch UI initialisiert
SwGlossaryList*     pGlossaryList = 0;

String* pOldGrfCat = 0;
String* pOldTabCat = 0;
String* pOldFrmCat = 0;
String* pOldDrwCat = 0;
String* pCurrGlosGroup = 0;

SvStringsDtor* pDBNameList = 0;

SvStringsDtor*  pAuthFieldNameList = 0;
SvStringsDtor*  pAuthFieldTypeList = 0;

/*--------------------------------------------------------------------
    Beschreibung:   UI beenden
 --------------------------------------------------------------------*/

void _FinitUI()
{
    delete ViewShell::GetShellRes();
    ViewShell::SetShellRes( 0 );

    SwEditWin::_FinitStaticData();

    DELETEZ(pGlossaries);

    delete SwFieldType::pFldNames;

    delete pOldGrfCat;
    delete pOldTabCat;
    delete pOldFrmCat;
    delete pOldDrwCat;
    delete pCurrGlosGroup;
    delete pDBNameList;
    delete pGlossaryList;
    delete pAuthFieldNameList;
    delete pAuthFieldTypeList;


}
/*--------------------------------------------------------------------
    Beschreibung:   Initialisierung
 --------------------------------------------------------------------*/


void _InitUI()
{
    // ShellResource gibt der CORE die Moeglichkeit mit Resourcen zu arbeiten
    ViewShell::SetShellRes( new ShellResource );
    pDBNameList = new SvStringsDtor( 5, 5 );
    SwEditWin::_InitStaticData();
}


ShellResource::ShellResource()
    : Resource( SW_RES(RID_SW_SHELLRES) ),
    aPostItPage( SW_RES( STR_POSTIT_PAGE ) ),
    aPostItAuthor( SW_RES( STR_POSTIT_AUTHOR ) ),
    aPostItLine( SW_RES( STR_POSTIT_LINE ) ),
    aCalc_Syntax( SW_RES( STR_CALC_SYNTAX ) ),
    aCalc_ZeroDiv( SW_RES( STR_CALC_ZERODIV ) ),
    aCalc_Brack( SW_RES( STR_CALC_BRACK ) ),
    aCalc_Pow( SW_RES( STR_CALC_POW ) ),
    aCalc_VarNFnd( SW_RES( STR_CALC_VARNFND ) ),
    aCalc_Overflow( SW_RES( STR_CALC_OVERFLOW ) ),
    aCalc_WrongTime( SW_RES( STR_CALC_WRONGTIME ) ),
    aCalc_Default( SW_RES( STR_CALC_DEFAULT ) ),
    aCalc_Error( SW_RES( STR_CALC_ERROR ) ),
    aGetRefFld_Up( SW_RES( STR_GETREFFLD_UP ) ),
    aGetRefFld_Down( SW_RES( STR_GETREFFLD_DOWN ) ),
    aStrAllPageHeadFoot( SW_RES( STR_ALLPAGE_HEADFOOT ) ),
    aStrNone( SW_RES( STR_TEMPLATE_NONE )),
    aFixedStr( SW_RES( STR_FIELD_FIXED )),
    aTOXIndexName(          SW_RES(STR_TOI)),
    aTOXUserName(           SW_RES(STR_TOU)),
    aTOXContentName(        SW_RES(STR_TOC)),
    aTOXIllustrationsName(  SW_RES(STR_TOX_ILL)),
    aTOXObjectsName(        SW_RES(STR_TOX_OBJ)),
    aTOXTablesName(         SW_RES(STR_TOX_TBL)),
    aTOXAuthoritiesName(    SW_RES(STR_TOX_AUTH)),
    pAutoFmtNameLst( 0 )
{
    const USHORT nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;

    for(USHORT i = 0; i < nCount; ++i)
    {
        String* pNew = new SW_RESSTR(FLD_DOCINFO_BEGIN + i);
        aDocInfoLst.Insert(pNew, aDocInfoLst.Count());
    }

    FreeResource();
}

ShellResource::~ShellResource()
{
    if( pAutoFmtNameLst )
        delete pAutoFmtNameLst, pAutoFmtNameLst = 0;
}

SwGlossaries* GetGlossaries()
{
    if (!pGlossaries)
        pGlossaries = new SwGlossaries;
    return (pGlossaries);
}

BOOL HasGlossaryList()
{
    return pGlossaryList != 0;
}

SwGlossaryList* GetGlossaryList()
{
    if(!pGlossaryList)
        pGlossaryList = new SwGlossaryList();

    return pGlossaryList;
}

struct ImpAutoFmtNameListLoader : public Resource
{
    ImpAutoFmtNameListLoader( SvStringsDtor& rLst );
};

void ShellResource::_GetAutoFmtNameLst() const
{
    SvStringsDtor** ppLst = (SvStringsDtor**)&pAutoFmtNameLst;
    *ppLst = new SvStringsDtor( STR_AUTOFMTREDL_END );
    ImpAutoFmtNameListLoader aTmp( **ppLst );
}

ImpAutoFmtNameListLoader::ImpAutoFmtNameListLoader( SvStringsDtor& rLst )
    : Resource( ResId(RID_SHELLRES_AUTOFMTSTRS, pSwResMgr) )
{
    for( USHORT n = 0; n < STR_AUTOFMTREDL_END; ++n )
    {
        String* p = new String( ResId( n + 1, pSwResMgr) );
        if(STR_AUTOFMTREDL_TYPO == n)
        {
            LocaleDataWrapper& rLclD = GetAppLocaleData();
#ifdef WNT
            //fuer Windows Sonderbehandlung, da MS hier ein paar Zeichen im Dialogfont vergessen hat
            p->SearchAndReplace(C2S("%1"), C2S(",,"));
            p->SearchAndReplace(C2S("%2"), C2S("''"));
#else
            //unter richtigen Betriebssystemen funktioniert es auch so
            p->SearchAndReplace(C2S("%1"), rLclD.getDoubleQuotationMarkStartChar());
            p->SearchAndReplace(C2S("%2"), rLclD.getDoubleQuotationMarkEndChar());
#endif
        }
        rLst.Insert( p, n );
    }
    FreeResource();
}
/* -----------------16.09.99 12:28-------------------

 --------------------------------------------------*/
const String&   SwAuthorityFieldType::GetAuthFieldName(ToxAuthorityField eType)
{
    if(!pAuthFieldNameList)
    {
        pAuthFieldNameList = new SvStringsDtor(AUTH_FIELD_END, 1);
        for(USHORT i = 0; i < AUTH_FIELD_END; i++)
        {
            String*  pTmp = new String(SW_RES(STR_AUTH_FIELD_START + i));
            //remove short cuts
            pTmp->EraseAllChars('~');
            pAuthFieldNameList->Insert(pTmp, pAuthFieldNameList->Count());
        }
    }
    return *pAuthFieldNameList->GetObject(eType);
}
/* -----------------16.09.99 12:29-------------------

 --------------------------------------------------*/
const String&   SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
{
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new SvStringsDtor(AUTH_TYPE_END, 1);
        for(USHORT i = 0; i < AUTH_TYPE_END; i++)
            pAuthFieldTypeList->Insert(
                new String(SW_RES(STR_AUTH_TYPE_START + i)),
                                    pAuthFieldTypeList->Count());
    }
    return *pAuthFieldTypeList->GetObject(eType);
}

/*************************************************************************

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/06 13:39:19  jp
    should changes: don't use IniManager

    Revision 1.1.1.1  2000/09/18 17:14:50  hr
    initial import

    Revision 1.112  2000/09/18 16:06:17  willem.vandorp
    OpenOffice header added.

    Revision 1.111  2000/04/18 15:14:08  os
    UNICODE

    Revision 1.110  2000/02/14 14:23:29  os
    #70473# Unicode

    Revision 1.109  2000/02/10 10:35:22  os
    #70359# titles added to AutoText groups

    Revision 1.108  1999/10/06 12:24:13  os
    index corrected

    Revision 1.107  1999/10/05 10:40:03  os
    index corrected

    Revision 1.106  1999/09/22 12:48:10  aw
    changes for IAO-Handle handling in SW (AW)

    Revision 1.105  1999/09/20 09:58:34  os
    local resources separated; Authority defines

    Revision 1.104  1999/09/15 14:10:32  os
    language improvements

    Revision 1.103  1999/07/07 06:15:48  OS
    extended indexes: Index names in ShellRes


      Rev 1.102   07 Jul 1999 08:15:48   OS
   extended indexes: Index names in ShellRes

      Rev 1.101   10 Jun 1999 13:14:52   JP
   have to change: no AppWin from SfxApp

      Rev 1.100   28 May 1999 14:09:04   OS
   #54839# Plattformstrings raus

      Rev 1.99   09 Mar 1999 18:27:42   JP
   Task #61405#: AutoCompletion von Woertern

      Rev 1.98   05 Mar 1999 14:33:02   JP
   Bug #62868#: beim Anzeigen der Feldnamen opt. das Fix mit ausgeben

      Rev 1.97   12 Jan 1999 21:14:32   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen

      Rev 1.96   10 Dec 1998 09:47:56   MIB
   #60060#: Beschriftungen fuer Zeichen-Objekte

      Rev 1.95   06 Oct 1998 09:47:22   JP
   Task #57586#: ShellRes um die AutoFmt-RedlineNameListe erweitert

      Rev 1.94   16 Jul 1998 16:40:46   JP
   STR_NONE ->STR_TEMPLATE_NONE fuer den MacCompiler

      Rev 1.93   26 Jun 1998 17:12:52   AMA
   Chg: Notizen werden mit Zeilenangabe gedruckt

      Rev 1.92   29 May 1998 18:56:32   JP
   neuer String in den ShellResourcen

      Rev 1.91   26 May 1998 15:31:36   JP
   neuen String aufgenommen: Alle

      Rev 1.90   21 Nov 1997 12:10:12   MA
   includes

      Rev 1.89   03 Nov 1997 13:59:24   MA
   precomp entfernt

      Rev 1.88   20 Jan 1997 17:04:30   JP
   ShowTip Umbau

      Rev 1.87   11 Nov 1996 10:54:12   MA
   ResMgr

      Rev 1.86   06 Nov 1996 16:55:10   OS
   include

      Rev 1.85   06 Nov 1996 16:39:06   OS
   SpellCheck in der OFA

      Rev 1.84   31 Oct 1996 14:51:50   JP
   neue Resource fuer GetRefFelder

      Rev 1.83   26 Sep 1996 16:57:24   OS
   neu: SwGlossaryList

      Rev 1.82   19 Sep 1996 10:34:56   JP
   neu: pDBNameList fuers Basic

      Rev 1.81   28 Aug 1996 15:42:32   OS
   includes

      Rev 1.80   16 Aug 1996 11:28:26   OM
   Dokinfo aufgegliedert

      Rev 1.79   12 Aug 1996 16:33:34   OM
   Namen der Def Datenbank in SwTypes verschoben

      Rev 1.78   25 Jul 1996 16:42:52   OM
   Datenbankname am Dok auch mit Tabelle initialisieren

      Rev 1.77   20 Dec 1995 17:21:22   OM
   Address->FAR_DATA

      Rev 1.76   05 Dec 1995 19:02:00   JP
   static aActGroup nur noch ein Pointer (wird von InitUi gepflegt!)

      Rev 1.75   28 Nov 1995 21:18:20   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.74   28 Nov 1995 13:39:54   JP
   Aufgeraeumt - alte externs entfernt, SwShellRes ueber Resource laden

      Rev 1.73   27 Nov 1995 21:16:56   JP
   static Strings aus caption.cxx hierher als Pointer verschoben

      Rev 1.72   24 Nov 1995 16:57:22   OM
   PCH->PRECOMPILED

      Rev 1.71   13 Nov 1995 12:21:32   JP
   static Namen von Grafik/OLE/Tabelle setzen fuer Verzeichnisse entfernt

*************************************************************************/


