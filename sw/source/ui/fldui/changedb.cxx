/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: changedb.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:45:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#define _CHANGEDB_CXX

#include <svtools/stdctrl.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

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
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _CHANGEDB_HRC
#include <changedb.hrc>
#endif

#include <unomid.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


/*--------------------------------------------------------------------
    Beschreibung: Feldeinfuegen bearbeiten
 --------------------------------------------------------------------*/


SwChangeDBDlg::SwChangeDBDlg(SwView& rVw) :
    SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), SW_RES(DLG_CHANGE_DB)),

    aDBListFL   (this, SW_RES(FL_DBLIST     )),
    aUsedDBFT   (this, SW_RES(FT_USEDDB     )),
    aAvailDBFT  (this, SW_RES(FT_AVAILDB    )),
    aUsedDBTLB  (this, SW_RES(TLB_USEDDB    )),
    aAvailDBTLB (this, SW_RES(TLB_AVAILDB   ), 0),
    aAddDBPB    (this, SW_RES(PB_ADDDB)),
    aDescFT     (this, SW_RES(FT_DESC       )),
    aDocDBTextFT(this, SW_RES(FT_DOCDBTEXT  )),
    aDocDBNameFT(this, SW_RES(FT_DOCDBNAME  )),
    aOKBT       (this, SW_RES(BT_OK         )),
    aCancelBT   (this, SW_RES(BT_CANCEL     )),
    aHelpBT     (this, SW_RES(BT_HELP       )),
//  aChangeBT   (this, SW_RES(BT_CHANGEDB    )),
    aImageList      (SW_RES(ILIST_DB_DLG    )),
    aImageListHC    (SW_RES(ILIST_DB_DLG_HC )),

    pSh(rVw.GetWrtShellPtr()),
    pMgr( new SwFldMgr() )
{
    aAvailDBTLB.SetWrtShell(*pSh);
    FillDBPopup();

    FreeResource();

    ShowDBName(pSh->GetDBData());
    aOKBT.SetClickHdl(LINK(this, SwChangeDBDlg, ButtonHdl));
    aAddDBPB.SetClickHdl(LINK(this, SwChangeDBDlg, AddDBHdl));

    aUsedDBTLB.SetSelectionMode(MULTIPLE_SELECTION);
    aUsedDBTLB.SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    aUsedDBTLB.SetSpaceBetweenEntries(0);

    aUsedDBTLB.SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE),
                    aImageList.GetImage(IMG_EXPAND  ), BMP_COLOR_NORMAL );
    aUsedDBTLB.SetNodeBitmaps( aImageListHC.GetImage(IMG_COLLAPSE),
                    aImageListHC.GetImage(IMG_EXPAND  ), BMP_COLOR_HIGHCONTRAST );

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
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")

    const SwDBData& rDBData = pSh->GetDBData();
    String sDBName(rDBData.sDataSource);
    String sTableName(rDBData.sCommand);
    aAvailDBTLB.Select(sDBName, sTableName, aEmptyStr);

    SvStringsDtor aAllDBNames(5, 5);

    Sequence<OUString> aDBNames = xDBContext->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    sal_Int32 nDBCount = aDBNames.getLength();
    for(sal_Int32 i = 0; i < nDBCount; i++)
    {
        aAllDBNames.Insert(new String(pDBNames[i]), aAllDBNames.Count());
    }

    SvStringsDtor aDBNameList(5, 1);
    pSh->GetAllUsedDB( aDBNameList, &aAllDBNames );

    USHORT nCount = aDBNameList.Count();
    aUsedDBTLB.Clear();
    SvLBoxEntry *pFirst = 0;
    SvLBoxEntry *pLast = 0;

    for (USHORT k = 0; k < nCount; k++)
    {
        sDBName = *aDBNameList.GetObject(k);
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

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SvLBoxEntry* SwChangeDBDlg::Insert(const String& rDBName)
{
    String sDBName(rDBName.GetToken(0, DB_DELIM));
    String sTableName(rDBName.GetToken(1, DB_DELIM));
    int nCommandType = rDBName.GetToken(2, DB_DELIM).ToInt32();
    SvLBoxEntry* pParent;
    SvLBoxEntry* pChild;

    USHORT nParent = 0;
    USHORT nChild = 0;

    Image aTableImg = aImageList.GetImage(IMG_DBTABLE);
    Image aDBImg = aImageList.GetImage(IMG_DB);
    Image aQueryImg = aImageList.GetImage(IMG_DBQUERY);
    Image aHCTableImg = aImageListHC.GetImage(IMG_DBTABLE);
    Image aHCDBImg = aImageListHC.GetImage(IMG_DB);
    Image aHCQueryImg = aImageListHC.GetImage(IMG_DBQUERY);
    Image& rToInsert = nCommandType ? aQueryImg : aTableImg;
    Image& rHCToInsert = nCommandType ? aHCQueryImg : aHCTableImg;
    while ((pParent = aUsedDBTLB.GetEntry(nParent++)) != NULL)
    {
        if (sDBName == aUsedDBTLB.GetEntryText(pParent))
        {
            while ((pChild = aUsedDBTLB.GetEntry(pParent, nChild++)) != NULL)
            {
                if (sTableName == aUsedDBTLB.GetEntryText(pChild))
                    return pChild;
            }
            SvLBoxEntry* pRet = aUsedDBTLB.InsertEntry(sTableName, rToInsert, rToInsert, pParent);
            aUsedDBTLB.SetExpandedEntryBmp(pRet, rHCToInsert, BMP_COLOR_HIGHCONTRAST);
            aUsedDBTLB.SetCollapsedEntryBmp(pRet, rHCToInsert, BMP_COLOR_HIGHCONTRAST);
            pRet->SetUserData((void*)nCommandType);
            return pRet;
        }
    }
    pParent = aUsedDBTLB.InsertEntry(sDBName, aDBImg, aDBImg);
    aUsedDBTLB.SetExpandedEntryBmp(pParent, aHCDBImg, BMP_COLOR_HIGHCONTRAST);
    aUsedDBTLB.SetCollapsedEntryBmp(pParent, aHCDBImg, BMP_COLOR_HIGHCONTRAST);

    SvLBoxEntry* pRet = aUsedDBTLB.InsertEntry(sTableName, rToInsert, rToInsert, pParent);
    aUsedDBTLB.SetExpandedEntryBmp(pRet, rHCToInsert, BMP_COLOR_HIGHCONTRAST);
    aUsedDBTLB.SetCollapsedEntryBmp(pRet, rHCToInsert, BMP_COLOR_HIGHCONTRAST);
    pRet->SetUserData((void*)nCommandType);
    return pRet;
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
            *pTmp += DB_DELIM;
            *pTmp += aUsedDBTLB.GetEntryText( pEntry );
            *pTmp += DB_DELIM;
            int nCommandType = (int)(ULONG)pEntry->GetUserData();
            *pTmp += String::CreateFromInt32(nCommandType);
            aDBNames.Insert(pTmp, aDBNames.Count() );
        }
        pEntry = aUsedDBTLB.NextSelected(pEntry);
    }

    pSh->StartAllAction();
    String sTableName, sColumnName;
    sal_Bool bIsTable = sal_False;
    String sTemp(aAvailDBTLB.GetDBName(sTableName, sColumnName, &bIsTable));
    sTemp += DB_DELIM;
    sTemp += sTableName;
    sTemp += DB_DELIM;
    sTemp += bIsTable ? '0' : '1';
    pSh->ChangeDBFields( aDBNames, sTemp);
    pSh->EndAllAction();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwChangeDBDlg, ButtonHdl, Button *, EMPTYARG )
{
    String sTableName, sColumnName;
    SwDBData aData;
    sal_Bool bIsTable = sal_False;
    aData.sDataSource = aAvailDBTLB.GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;;
    pSh->ChgDBData(aData);
    ShowDBName(pSh->GetDBData());
    EndDialog(RET_OK);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwChangeDBDlg, TreeSelectHdl, SvTreeListBox *, EMPTYARG )
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

void SwChangeDBDlg::ShowDBName(const SwDBData& rDBData)
{
    String sTmp(rDBData.sDataSource);
    String sName;
    sTmp += '.';
    sTmp += (String)rDBData.sCommand;

    for (USHORT i = 0; i < sTmp.Len(); i++)
    {
        sName += sTmp.GetChar(i);
        if (sTmp.GetChar(i) == '~')
            sName += '~';
    }

    aDocDBNameFT.SetText(sName);
}
/*-- 27.05.2004 09:14:01---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwChangeDBDlg, AddDBHdl, PushButton *, EMPTYARG )
{
    String sNewDB = SwNewDBMgr::LoadAndRegisterDataSource();
    if(sNewDB.Len())
        aAvailDBTLB.AddDataSource(sNewDB);
    return 0;
}


