/*************************************************************************
 *
 *  $RCSfile: changedb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:18:03 $
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

#define _CHANGEDB_CXX

#include <svtools/stdctrl.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifdef REPLACE_OFADBMGR
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEACCESS_HPP_
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#else

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SBA_SBAOBJ_HXX //autogen
#include <offmgr/sbaobj.hxx>
#endif

#endif

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _CHANGEDB_HXX
#include <changedb.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _CHANGEDB_HRC
#include <changedb.hrc>
#endif

#ifdef REPLACE_OFADBMGR
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
//using namespace com::sun::star::sdb;
//using namespace com::sun::star::sdbc;
//using namespace com::sun::star::sdbcx;
//using namespace com::sun::star::beans;
#define C2U(cChar) OUString::createFromAscii(cChar)
#endif

/*--------------------------------------------------------------------
    Beschreibung: Feldeinfuegen bearbeiten
 --------------------------------------------------------------------*/


SwChangeDBDlg::SwChangeDBDlg(SwView& rVw) :
    SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), SW_RES(DLG_CHANGE_DB)),

    aDescFT     (this, SW_RES(FT_DESC       )),
    aUsedDBFT   (this, SW_RES(FT_USEDDB     )),
    aAvailDBFT  (this, SW_RES(FT_AVAILDB    )),
    aDBListGB   (this, SW_RES(GB_DBLIST     )),
    aUsedDBTLB  (this, SW_RES(TLB_USEDDB    )),
    aAvailDBTLB (this, SW_RES(TLB_AVAILDB   )),
    aDocDBTextFT(this, SW_RES(FT_DOCDBTEXT  )),
    aDocDBNameFT(this, SW_RES(FT_DOCDBNAME  )),
    aOKBT       (this, SW_RES(BT_OK         )),
    aCancelBT   (this, SW_RES(BT_CANCEL     )),
    aHelpBT     (this, SW_RES(BT_HELP       )),
//  aChangeBT   (this, SW_RES(BT_CHANGEDB    )),

    aRootOpened     (SW_RES(BMP_ROOT_CLOSED)),
    aRootClosed     (SW_RES(BMP_ROOT_OPENED)),
    aDBBMP          (SW_RES(BMP_DB)),
    aTableBMP       (SW_RES(BMP_TABLE)),

    pMgr( new SwFldMgr() ),
    pSh(rVw.GetWrtShellPtr())
{
    FillDBPopup();

    FreeResource();

    ShowDBName(pSh->GetDBName());
    aOKBT.SetClickHdl(LINK(this, SwChangeDBDlg, ButtonHdl));

    aUsedDBTLB.SetSelectionMode(MULTIPLE_SELECTION);
    aUsedDBTLB.SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    aUsedDBTLB.SetFont(GetSettings().GetStyleSettings().GetAppFont());
    aUsedDBTLB.SetSpaceBetweenEntries(0);
    aUsedDBTLB.SetNodeBitmaps( aRootOpened, aRootClosed );

    Link aLink = LINK(this, SwChangeDBDlg, TreeSelectHdl);

    aUsedDBTLB.SetSelectHdl(aLink);
    aUsedDBTLB.SetDeselectHdl(aLink);
    aAvailDBTLB.SetSelectHdl(aLink);
    aAvailDBTLB.SetDeselectHdl(aLink);
    TreeSelectHdl();
}

/*--------------------------------------------------------------------
    Beschreibung: Datenbank-Listboxen initialisieren
 --------------------------------------------------------------------*/


void SwChangeDBDlg::FillDBPopup()
{
#ifdef REPLACE_OFADBMGR
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
//  SwNewDBMgr* pNewDBMgr = pSh->GetNewDBMgr();

//  String sDBNames = pNewDBMgr->GetSbaObject()->GetDatabaseNames();
//  SfxApplication* pSfxApp = SFX_APP();
//  sDBNames = pSfxApp->LocalizeDBName(INI2NATIONAL, sDBNames);

//  USHORT nDBNamesCount = sDBNames.GetTokenCount();

    String sDataBaseName(pSh->GetDBName());
    String sDBName(sDataBaseName.GetToken(0, DB_DELIM));
    String sTableName(sDataBaseName.GetToken(1, DB_DELIM));
    aAvailDBTLB.Select(sDBName, sTableName, aEmptyStr);

//  sDBNames = pNewDBMgr->GetSbaObject()->GetDatabaseNames();
    SvStringsDtor aAllDBNames(5, 5);

    Sequence<OUString> aDBNames = xDBContext->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nDBCount = aDBNames.getLength();

    for(long i = 0; i < nDBCount; i++)
    {
        String sDBName(pDBNames[i]);
        aAllDBNames.Insert(new String(sDBName), aAllDBNames.Count());
    }

    SvStringsDtor aDBNameList(5, 1);
    pSh->GetAllUsedDB( aDBNameList, &aAllDBNames );

    USHORT nCount = aDBNameList.Count();
    aUsedDBTLB.Clear();
    SvLBoxEntry *pFirst = 0;
    SvLBoxEntry *pLast = 0;

    for (i = 0; i < nCount; i++)
    {
        sDBName = *aDBNameList.GetObject(i);
        sDBName = sDBName.GetToken(0);
        pLast = Insert(sDBName);
        if (!pFirst)
            pFirst = pLast;
    }

    if (pFirst)
    {
        aUsedDBTLB.MakeVisible(pFirst);
        aUsedDBTLB.Select(pFirst);
    }

#else

    SwNewDBMgr* pNewDBMgr = pSh->GetNewDBMgr();

    String sDBNames = pNewDBMgr->GetSbaObject()->GetDatabaseNames();
    SfxApplication* pSfxApp = SFX_APP();
    sDBNames = pSfxApp->LocalizeDBName(INI2NATIONAL, sDBNames);

    USHORT nDBNamesCount = sDBNames.GetTokenCount();



//  if (rDBName.Len())
//      sDBName = pSfxApp->LocalizeDBName(INI2NATIONAL, rDBName);
//  else
    String sDBName = pSfxApp->LocalizeDBName(INI2NATIONAL, pSh->GetDBName());
    aAvailDBTLB.Select(sDBName);

    SvStringsDtor aDBNameList(5, 1);

    sDBNames = pNewDBMgr->GetSbaObject()->GetDatabaseNames();
    SvStringsDtor aAllDBNames(5, 5);
    for (USHORT i = 0; i < nDBNamesCount; i++)
#ifdef UNX
        aAllDBNames.Insert(new String(sDBNames.GetToken(i)), aAllDBNames.Count());
#else
        aAllDBNames.Insert(new String(sDBNames.GetToken(i).ToUpperAscii()), aAllDBNames.Count());
#endif


    pSh->GetAllUsedDB( aDBNameList, &aAllDBNames );

    USHORT nCount = aDBNameList.Count();
    aUsedDBTLB.Clear();
    SvLBoxEntry *pFirst = 0;
    SvLBoxEntry *pLast = 0;

    for (i = 0; i < nCount; i++)
    {
        sDBName = pNewDBMgr->ExtractDBName(*aDBNameList.GetObject(i));

#ifndef UNX
        // Richtige Schreibweise ermitteln (upper/lower-case)
        for (USHORT i2 = 0; i2 < nDBNamesCount; i2++)
        {
            if (sDBName.GetToken(0, DB_DELIM).ToUpperAscii() == sDBNames.GetToken(i2).ToUpperAscii())
            {
                sDBName = sDBNames.GetToken(i2);
                break;
            }
        }
#endif
        pLast = Insert(sDBName);
        if (!pFirst)
            pFirst = pLast;
    }

    if (pFirst)
    {
        aUsedDBTLB.MakeVisible(pFirst);
        aUsedDBTLB.Select(pFirst);
    }
#endif //REPLACE_OFADBMGR
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SvLBoxEntry* SwChangeDBDlg::Insert(const String& rDBName)
{
    String sDBName(rDBName.GetToken(0, DB_DELIM));
    String sTableName(rDBName.GetToken(1, DB_DELIM));
    SvLBoxEntry* pParent;
    SvLBoxEntry* pChild;

#ifdef REPLACE_OFADBMGR
#else
    sDBName = SFX_APP()->LocalizeDBName(INI2NATIONAL, sDBName);
#endif //REPLACE_OFADBMGR

    USHORT nParent = 0;
    USHORT nChild = 0;

#ifdef REPLACE_OFADBMGR
    while ((pParent = aUsedDBTLB.GetEntry(nParent++)) != NULL)
    {
        if (sDBName == aUsedDBTLB.GetEntryText(pParent))
        {
            while ((pChild = aUsedDBTLB.GetEntry(pParent, nChild++)) != NULL)
            {
                if (sTableName == aUsedDBTLB.GetEntryText(pChild))
                    return pChild;
            }
            return aUsedDBTLB.InsertEntry(sTableName, aTableBMP, aTableBMP, pParent);
        }
    }
#else
    while ((pParent = aUsedDBTLB.GetEntry(nParent++)) != NULL)
    {
#ifdef UNX
        if (sDBName == aUsedDBTLB.GetEntryText(pParent))
#else
        if (sDBName.ToUpperAscii() == aUsedDBTLB.GetEntryText(pParent).ToUpperAscii())
#endif
        {
            while ((pChild = aUsedDBTLB.GetEntry(pParent, nChild++)) != NULL)
            {
#ifdef UNX
                if (sTableName == aUsedDBTLB.GetEntryText(pChild))
#else
                if (sTableName.ToUpperAscii() == aUsedDBTLB.GetEntryText(pChild).ToUpperAscii())
#endif
                    return pChild;
            }
            return aUsedDBTLB.InsertEntry(sTableName, aTableBMP, aTableBMP, pParent);
        }
    }
#endif
    pParent = aUsedDBTLB.InsertEntry(sDBName, aDBBMP, aDBBMP);
    return aUsedDBTLB.InsertEntry(sTableName, aTableBMP, aTableBMP, pParent);
}

/*--------------------------------------------------------------------
    Beschreibung: Dialog zerstoeren
 --------------------------------------------------------------------*/


__EXPORT SwChangeDBDlg::~SwChangeDBDlg()
{
    delete pMgr;
}

/*--------------------------------------------------------------------
     Beschreibung:  Schliessen
 --------------------------------------------------------------------*/


void __EXPORT SwChangeDBDlg::Apply()
{
    UpdateFlds();
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


void SwChangeDBDlg::UpdateFlds()
{
    SvStringsDtor aDBNames( (BYTE)aUsedDBTLB.GetSelectionCount(), 1 );
    SvLBoxEntry* pEntry = aUsedDBTLB.FirstSelected();

    while( pEntry )
    {
        if( aUsedDBTLB.GetParent( pEntry ))
        {
            String* pTmp = new String( aUsedDBTLB.GetEntryText(
                                            aUsedDBTLB.GetParent( pEntry )));
#ifdef REPLACE_OFADBMGR
#else
            *pTmp = SFX_APP()->LocalizeDBName( NATIONAL2INI, *pTmp);
#endif
            *pTmp += DB_DELIM;
            *pTmp += aUsedDBTLB.GetEntryText( pEntry );
            aDBNames.Insert(pTmp, aDBNames.Count() );
        }
        pEntry = aUsedDBTLB.NextSelected(pEntry);
    }

    pSh->StartAllAction();
#ifdef REPLACE_OFADBMGR
    String sTableName, sColumnName;
    String sTemp(aAvailDBTLB.GetDBName(sTableName, sColumnName));
    sTemp += DB_DELIM;
    sTemp += sTableName;
    pSh->ChangeDBFields( aDBNames, sTemp);
#else
    pSh->ChangeDBFields( aDBNames, aAvailDBTLB.GetDBName());
#endif
    pSh->EndAllAction();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwChangeDBDlg, ButtonHdl, Button *, pBtn )
{
#ifdef REPLACE_OFADBMGR
    String sTableName, sColumnName;
    String sTemp(aAvailDBTLB.GetDBName(sTableName, sColumnName));
    sTemp += DB_DELIM;
    sTemp += sTableName;
    sTemp += DB_DELIM;
    sTemp += sColumnName;
    pSh->ChgDBName(sTemp);
#else
    String sNewDBName(aAvailDBTLB.GetDBName());
    pSh->ChgDBName(sNewDBName);
#endif
    ShowDBName(pSh->GetDBName());
    EndDialog(RET_OK);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwChangeDBDlg, TreeSelectHdl, SvTreeListBox *, pBox )
{
    BOOL bEnable = FALSE;

    SvLBoxEntry* pEntry = aAvailDBTLB.GetCurEntry();

    if (pEntry)
    {
        if (aAvailDBTLB.GetParent(pEntry))
            bEnable = TRUE;
        aOKBT.Enable( bEnable );
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Datenbankname fuer Anzeige wandeln
 --------------------------------------------------------------------*/

void SwChangeDBDlg::ShowDBName(const String& rDBName)
{
    String sTmp(rDBName.GetToken(0, DB_DELIM));
    String sName;

#ifdef REPLACE_OFADBMGR
#else
    sTmp = SFX_APP()->LocalizeDBName(INI2NATIONAL, sTmp);
#endif
    sTmp += '.';
    sTmp += rDBName.GetToken(1, DB_DELIM);

    for (USHORT i = 0; i < sTmp.Len(); i++)
    {
        sName += sTmp.GetChar(i);
        if (sTmp.GetChar(i) == '~')
            sName += '~';
    }

    aDocDBNameFT.SetText(sName);
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:36  hr
      initial import

      Revision 1.43  2000/09/18 16:05:27  willem.vandorp
      OpenOffice header added.

      Revision 1.42  2000/07/07 15:25:43  os
      replace ofadbmgr

      Revision 1.41  2000/06/26 13:35:59  os
      new DataBase API

      Revision 1.40  2000/04/18 15:17:31  os
      UNICODE

      Revision 1.39  2000/03/03 15:17:00  os
      StarView remainders removed

      Revision 1.38  2000/02/11 14:46:04  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.37  2000/01/31 12:53:09  os
      #72339# dialog needs one apply button only

      Revision 1.36  1999/09/29 06:49:52  mh
      chg: header

      Revision 1.35  1999/06/09 17:34:28  JP
      have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp


      Rev 1.34   09 Jun 1999 19:34:28   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.33   04 Feb 1999 21:18:30   JP
   Bug #61404#/#61014#: Optimierung der Schnittstellen

      Rev 1.32   19 Nov 1998 13:18:44   OM
   #59660# Eintrag vorselektieren

      Rev 1.31   06 Nov 1997 17:03:22   OM
   Geaenderte DB-Selektionsbox

      Rev 1.30   03 Nov 1997 13:18:10   MA
   precomp entfernt

      Rev 1.29   30 Oct 1997 14:30:48   OM
   Feldbefehl-Umstellung

      Rev 1.28   18 Sep 1997 14:34:04   OM
   Feldumstellung

      Rev 1.27   02 Sep 1997 09:55:54   OM
   SDB-Headeranpassung

      Rev 1.26   15 Aug 1997 12:11:20   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.25   06 Aug 1997 13:39:54   TRI
   VCL: GetpApp() statt pApp

      Rev 1.24   23 Jul 1997 20:25:52   HJS
   includes

      Rev 1.23   14 May 1997 19:01:46   OM
   Datenbank auch in ExpressionFields austauschen

      Rev 1.22   13 May 1997 13:11:06   OM
   #39586# Datenbank festlegen beendet Dialog

      Rev 1.21   08 Apr 1997 12:44:38   MA
   includes

      Rev 1.20   07 Apr 1997 18:23:06   MH
   chg: header

      Rev 1.19   18 Mar 1997 15:24:14   OM
   Sortierte Treelistbox

      Rev 1.18   11 Nov 1996 09:56:04   MA
   ResMgr

      Rev 1.17   28 Oct 1996 11:29:52   OM
   #32799#Namen der aktuellen Datenbank korrekt anzeigen

      Rev 1.16   02 Oct 1996 16:50:26   MA
   Umstellung Enable/Disable

      Rev 1.15   25 Sep 1996 14:11:42   OM
   Neue Datenbanktrenner

      Rev 1.14   28 Aug 1996 12:13:32   OS
   includes

      Rev 1.13   14 Aug 1996 11:45:54   OM
   Neue Segs

      Rev 1.12   14 Aug 1996 11:44:34   OM
   Datenbankumstellung

      Rev 1.11   23 Jul 1996 16:08:30   OM
   Neue Segs

      Rev 1.10   23 Jul 1996 16:07:28   OM
   Datenbank am Dok umsetzen

      Rev 1.9   17 Jul 1996 13:47:30   OM
   Datenbankumstellung 327

      Rev 1.8   23 May 1996 16:34:40   OM
   DB-Namen in Expressionfields austauschen

      Rev 1.7   23 May 1996 15:29:02   OM
   Schreibweise der DB-Namen restaurieren

      Rev 1.6   22 May 1996 12:04:38   OM
   ChangeDBFields an Doc verlagert

      Rev 1.5   20 May 1996 16:33:28   OM
   Nur verwendete Datenbanken zu Auswahl stellen

      Rev 1.4   20 May 1996 11:06:44   OM
   ChangeDBFields in die EditShell verlagert

      Rev 1.3   15 May 1996 17:05:28   OM
   Neue Segs

      Rev 1.2   15 May 1996 17:05:02   OM
   Datenbanken austauschen

      Rev 1.1   15 May 1996 14:02:22   OM
   Neue Segs

      Rev 1.0   15 May 1996 14:01:12   OM
   Initial revision.

*************************************************************************/


