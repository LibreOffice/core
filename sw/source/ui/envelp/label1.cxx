/*************************************************************************
 *
 *  $RCSfile: label1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif
#ifndef _SBA_SBAOBJ_HXX //autogen
#include <offmgr/sbaobj.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _LABIMP_HXX
#include <labimp.hxx>
#endif
#ifndef _LABEL_HXX
#include <label.hxx>
#endif
#ifndef _LABFMT_HXX
#include <labfmt.hxx>
#endif
#ifndef _LABPRT_HXX
#include <labprt.hxx>
#endif
#ifndef _LABIMG_HXX
#include <labimg.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _UNOCRSR_HXX //autogen wg. SwUnoCrsr
#include <unocrsr.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOATXT_HXX //autogen wg. SwXAutoTextEntry
#include <unoatxt.hxx>
#endif
#ifndef _UNOOBJ_HXX //
#include <unoobj.hxx>
#endif
#ifndef _UNOPRNMS_HXX //
#include <unoprnms.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen wg. SwDocShell
#include <docsh.hxx>
#endif

#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _LABEL_HRC
#include <label.hrc>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifdef REPLACE_OFADBMGR
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
#define C2U(char) rtl::OUString::createFromAscii(char)
#else

#endif  //REPLACE_OFADBMGR

using namespace ::com::sun::star;
using namespace ::utl;
using namespace ::rtl;

#define CL2S(cChar) UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))

#define USER_FIELD_FIRSTNAME        "BC_PRIV_FIRSTNAME"
#define USER_FIELD_NAME             "BC_PRIV_NAME"
#define USER_FIELD_PRIVSHORTCUT     "BC_PRIV_INITIALS"
#define USER_FIELD_FIRSTNAME_2      "BC_PRIV_FIRSTNAME_2"
#define USER_FIELD_NAME_2           "BC_PRIV_NAME_2"
#define USER_FIELD_PRIVSHORTCUT_2   "BC_PRIV_INITIALS_2"

#define USER_FIELD_PRIVSTREET       "BC_PRIV_STREET"
#define USER_FIELD_PRIVZIP          "BC_PRIV_ZIP"
#define USER_FIELD_PRIVCITY         "BC_PRIV_CITY"
#define USER_FIELD_PRIVCOUNTRY      "BC_PRIV_COUNTRY"
#define USER_FIELD_PRIVSTATE        "BC_PRIV_STATE"
#define USER_FIELD_PRIVTITLE        "BC_PRIV_TITLE"
#define USER_FIELD_PRIVPROFESSION   "BC_PRIV_PROFESSION"
#define USER_FIELD_PRIVPHONE        "BC_PRIV_PHONE"
#define USER_FIELD_PRIVMOBILE       "BC_PRIV_MOBILE"
#define USER_FIELD_PRIVFAX          "BC_PRIV_FAX"
#define USER_FIELD_PRIVWWW          "BC_PRIV_WWW"
#define USER_FIELD_PRIVMAIL         "BC_PRIV_MAIL"
#define USER_FIELD_COMPCOMPANY      "BC_COMP_COMPANY"
#define USER_FIELD_COMPCOMPANYEXT   "BC_COMP_COMPANYEXT"
#define USER_FIELD_COMPSLOGAN       "BC_COMP_SLOGAN"
#define USER_FIELD_COMPSTREET       "BC_COMP_STREET"
#define USER_FIELD_COMPZIP          "BC_COMP_ZIP"
#define USER_FIELD_COMPCITY         "BC_COMP_CITY"
#define USER_FIELD_COMPCOUNTRY      "BC_COMP_COUNTRY"
#define USER_FIELD_COMPSTATE        "BC_COMP_STATE"
#define USER_FIELD_COMPPOSITION     "BC_COMP_POSITION"
#define USER_FIELD_COMPPHONE        "BC_COMP_PHONE"
#define USER_FIELD_COMPMOBILE       "BC_COMP_MOBILE"
#define USER_FIELD_COMPFAX          "BC_COMP_FAX"
#define USER_FIELD_COMPWWW          "BC_COMP_WWW"
#define USER_FIELD_COMPMAIL         "BC_COMP_MAIL"

SV_IMPL_PTRARR( SwLabRecs, SwLabRec* );

inline sal_Bool NextToken( String &rStr, sal_uInt16 &nPos, sal_uInt16 &nPos2 )
{
    nPos  = nPos2+1;
    nPos2 = rStr.Search( ';', nPos );
    return nPos2 != STRING_NOTFOUND;
}



SwLabRec::SwLabRec( String& rStr, MetricField& rField )
{
    //Fuer die ersten beiden Token duefen nur Trailing und Leading Blanks
    //vernichtet werden.
    sal_uInt16 nPos = rStr.Search( ';' );
    if ( nPos == STRING_NOTFOUND )
        return;
    aMake = rStr.Copy( 0, nPos );
    ++nPos;
    sal_uInt16 nPos2 = rStr.Search( ';', nPos );
    if ( nPos2 == STRING_NOTFOUND )
        return;
    aType = rStr.Copy( nPos, nPos2 - nPos );

    aMake.EraseLeadingChars(); aMake.EraseTrailingChars();
    aType.EraseLeadingChars(); aType.EraseTrailingChars();

    //Fuer alle weiteren Token koennen alle Blanks vernichtet werden.
    rStr.EraseAllChars();
    nPos  = rStr.Search( ';' );
    nPos2 = rStr.Search( ';', nPos+1 );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    bCont = (rStr.GetChar(nPos) == 'C' || rStr.GetChar(nPos) == 'c') ? sal_True : sal_False;

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lHDist = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lVDist = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lWidth = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lHeight = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lLeft = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;
    lUpper = GetLong( rStr.Copy( nPos, nPos2 - nPos ), rField );

    if ( !::NextToken( rStr, nPos, nPos2 ) )
        return;

    nCols = rStr.Copy( nPos, nPos2 - nPos ).ToInt32();

    nPos = nPos2+1;
    nRows = rStr.Copy( nPos ).ToInt32();

    // Ggf. Zeilenanzahl fuer Endlosetiketten automatisch berechnen
    if ( bCont && !nRows )
    {
        static const long lMax = 31748; // 56 cm
        nRows = (sal_uInt16)((lMax - lUpper) / Max(1L, lVDist));
    }
}

// --------------------------------------------------------------------------



long SwLabRec::GetLong(const String& rStr, MetricField& rField)
{
    rField.SetText( rStr );
    rField.Reformat();
    return GETFLDVAL( rField );
}

// --------------------------------------------------------------------------

String lcl_GetLabelsIni()
{
    String sRet( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                    "labels.ini" )));
    SfxIniManager* pIniManager = SFX_INIMANAGER();
    if( !pIniManager->SearchFile( sRet, SFX_KEY_USERCONFIG_PATH ))
    {
        sRet.Insert( INET_PATH_TOKEN, 0 );
        sRet.Insert( pIniManager->Get( SFX_KEY_USERCONFIG_PATH ), 0 );
    }
    return sRet;
}


void SwLabDlg::_ReplaceGroup( const String &rMake, SwLabItem *pItem )
{
    //Die alten Eintraege vernichten.
    pRecs->Remove( 1, pRecs->Count() - 1 );

    // Etiketten aus labels.ini lesen, zum Konvertieren verwenden wir ein
    // MetricField.
    MetricField aField( this, WinBits(0) );
    aField.SetUnit        (FUNIT_CM);
    aField.SetDecimalDigits(3);
    aField.SetMin         (0);
    aField.SetMax         (LONG_MAX);
    International aInter;
    aInter.SetNumThousandSep( '.' );
    aInter.SetNumDecimalSep ( ',' );
    aField.SetInternational( aInter );

    rtl_TextEncoding eSysEnc = gsl_getSystemTextEncoding();
    Config aCfg( ::lcl_GetLabelsIni() );
    aCfg.SetGroup( ByteString( rMake, eSysEnc ));

    String aTmp( rMake );
    aTmp += ';';
    const sal_uInt16 nCount = aCfg.GetKeyCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        String aStr( aTmp );
        aStr += String(aCfg.ReadKey( i ), eSysEnc);
        SwLabRec* pRec = new SwLabRec( aStr, aField );
        sal_Bool bDouble = sal_False;

        for (sal_uInt16 nRecPos = 0; nRecPos < pRecs->Count(); nRecPos++)
        {
            if (pRec->aMake == pRecs->GetObject(nRecPos)->aMake &&
                pRec->aType == pRecs->GetObject(nRecPos)->aType)
            {
                bDouble = sal_True;
                break;
            }
        }

        if (!bDouble)
        {
            pRecs->C40_INSERT( SwLabRec, pRec, pRecs->Count() );

            if ( pRec->aMake == pItem->aMake && pRec->aType == pItem->aType )
            {
                pItem->lHDist  = pRec->lHDist;
                pItem->lVDist  = pRec->lVDist;
                pItem->lWidth  = pRec->lWidth;
                pItem->lHeight = pRec->lHeight;
                pItem->lLeft   = pRec->lLeft;
                pItem->lUpper  = pRec->lUpper;
                pItem->nCols   = pRec->nCols;
                pItem->nRows   = pRec->nRows;
                pItem->bCont   = pRec->bCont;
            }
        }
    }
    aLstGroup = rMake;
}

// --------------------------------------------------------------------------



void SwLabDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == TP_LAB_LAB)
    {
        if(m_bLabel)
        {
            ((SwLabPage*)&rPage)->SetNewDBMgr(pNewDBMgr);
            ((SwLabPage*)&rPage)->InitDatabaseBox();
        }
        else
            ((SwLabPage*)&rPage)->SetToBusinessCard();
    }
    else if (nId == TP_LAB_PRT)
        pPrtPage = (SwLabPrtPage*)&rPage;
}

// --------------------------------------------------------------------------



SwLabDlg::SwLabDlg(Window* pParent, const SfxItemSet& rSet,
                                SwNewDBMgr* pDBMgr, sal_Bool bLabel) :
    SfxTabDialog( pParent, SW_RES(DLG_LAB), &rSet, sal_False ),
    sBusinessCardDlg(ResId(ST_BUSINESSCARDDLG)),
    sMedium(ResId(ST_FIRSTPAGE_BC)),
    sFormat(ResId(ST_FIRSTPAGE_LAB)),
    aTypeIds( 50, 10 ),
    aMakes  (  5,  0 ),
    pRecs   ( new SwLabRecs() ),
    pNewDBMgr(pDBMgr),
    pPrtPage(0),
    m_bLabel(bLabel)
{
    WaitObject aWait( pParent );

    FreeResource();

    GetOKButton().SetText(String(SW_RES(STR_BTN_NEW_DOC)));
    GetOKButton().SetHelpId(HID_LABEL_INSERT);
    GetOKButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird

    AddTabPage(TP_LAB_LAB, m_bLabel ? sFormat : sMedium ,SwLabPage   ::Create, 0, sal_False, 0);
    AddTabPage(TP_VISITING_CARDS, SwVisitingCardPage::Create, 0);
    AddTabPage(TP_LAB_FMT, SwLabFmtPage::Create, 0);
    AddTabPage(TP_LAB_PRT, SwLabPrtPage::Create, 0);
    AddTabPage(TP_BUSINESS_DATA, SwBusinessDataPage::Create, 0 );
    AddTabPage(TP_PRIVATE_DATA, SwPrivateDataPage::Create, 0);


    if(m_bLabel)
    {
        RemoveTabPage(TP_BUSINESS_DATA);
        RemoveTabPage(TP_PRIVATE_DATA);
        RemoveTabPage(TP_VISITING_CARDS);
    }
    else
    {
        SetText(sBusinessCardDlg);
    }
    // Benutzer-Etikette aus writer.cfg lesen
    SwLabItem aItem((const SwLabItem&)rSet.Get(FN_LABEL));
    SwLabRec* pRec = new SwLabRec;
    const String aTmp( SW_RES( STR_CUSTOM ) );
    pRec->aMake   = pRec->aType = aTmp;
    pRec->lHDist  = aItem.lHDist;
    pRec->lVDist  = aItem.lVDist;
    pRec->lWidth  = aItem.lWidth;
    pRec->lHeight = aItem.lHeight;
    pRec->lLeft   = aItem.lLeft;
    pRec->lUpper  = aItem.lUpper;
    pRec->nCols   = aItem.nCols;
    pRec->nRows   = aItem.nRows;
    pRec->bCont   = aItem.bCont;

    sal_Bool bDouble = sal_False;

    for (sal_uInt16 nRecPos = 0; nRecPos < pRecs->Count(); nRecPos++)
    {
        if (pRec->aMake == pRecs->GetObject(nRecPos)->aMake &&
            pRec->aType == pRecs->GetObject(nRecPos)->aType)
        {
            bDouble = sal_True;
            break;
        }
    }

    if (!bDouble)
        pRecs->C40_INSERT( SwLabRec, pRec, 0 );

    Config aCfg( ::lcl_GetLabelsIni() );
    aCfg.SetGroup( "Labels" );
    sal_uInt16 nLstGroup = 0;
    rtl_TextEncoding eSysEnc = gsl_getSystemTextEncoding();
    const sal_uInt16 nGroups = aCfg.GetKeyCount();
    for ( sal_uInt16 nGroup = 0; nGroup < nGroups; ++nGroup )
    {
        String *pMake = new String( aCfg.ReadKey( nGroup ), eSysEnc );
        pMake->EraseLeadingChars();
        pMake->EraseTrailingChars();
        aMakes.Insert( pMake, aMakes.Count() );
        if ( *pMake == aItem.aLstMake )
            nLstGroup = nGroup;
    }
    if ( aMakes.Count() )
        _ReplaceGroup( *aMakes[nLstGroup], &aItem );
    if (pExampleSet)
        pExampleSet->Put(aItem);
}

// --------------------------------------------------------------------------

SwLabDlg::~SwLabDlg()
{
    delete pRecs;
}

/* --------------------------------------------------------------------------

Diese Routine modifiziert ein uebergebenes SwLabItem so, dass es hernach in
der Config gespeichert werden kann. Durch die Angabe des Herstellers und des
Artikels sind die weiteren Groessenangaben redundant, daher werden in lHDist
und den folgenden Membern die Eigenschaften der benutzerdefinierten Etiketten
gespeichert.

-------------------------------------------------------------------------- */



void SwLabDlg::MakeConfigItem( SwLabItem& rItem ) const
{
    if ( rItem.aType != String(SW_RES(STR_CUSTOM)) )
    {
        SwLabRec* pRec = (*pRecs)[0];
        rItem.lHDist  = pRec->lHDist;
        rItem.lVDist  = pRec->lVDist;
        rItem.lWidth  = pRec->lWidth;
        rItem.lHeight = pRec->lHeight;
        rItem.lLeft   = pRec->lLeft;
        rItem.lUpper  = pRec->lUpper;
        rItem.nCols   = pRec->nCols;
        rItem.nRows   = pRec->nRows;
    }
    const SwLabItem& rActItem = (const SwLabItem&)GetExampleSet()->Get(FN_LABEL);
    rItem.bCont    = rActItem.bCont;
    rItem.aLstMake = rActItem.aLstMake;
    rItem.aLstType = rActItem.aLstType;
    rItem.sDBName  = rActItem.sDBName;
}

// --------------------------------------------------------------------------

void SwLabDlg::GetLabItem(SwLabItem &rItem)
{
    const SwLabItem& rActItem = (const SwLabItem&)GetExampleSet()->Get(FN_LABEL);
    const SwLabItem& rOldItem = (const SwLabItem&)GetInputSetImpl()->Get(FN_LABEL);

    if (rActItem != rOldItem)
    {   // Wurde schon mal mit (hoffentlich) korrektem Inhalt "geputtet"
        rItem = rActItem;
    }
    else
    {
        rItem = rOldItem;

        // Im rItem stehen (vom Namen mal abgesehen) immer nur die
        // benutzerdefinierbaren Einstellungen. Daher richtige Werte
        // direkt aus dem Record besorgen:
        SwLabRec* pRec = GetRecord(rItem.aType, rItem.bCont);

        rItem.lHDist  = pRec->lHDist;
        rItem.lVDist  = pRec->lVDist;
        rItem.lWidth  = pRec->lWidth;
        rItem.lHeight = pRec->lHeight;
        rItem.lLeft   = pRec->lLeft;
        rItem.lUpper  = pRec->lUpper;
        rItem.nCols   = pRec->nCols;
        rItem.nRows   = pRec->nRows;
    }
}

// --------------------------------------------------------------------------

SwLabRec* SwLabDlg::GetRecord(const String &rRecName, sal_Bool bCont)
{
    SwLabRec* pRec = NULL;
    sal_Bool bFound = sal_False;
    String sCustom(SW_RES(STR_CUSTOM));

    const sal_uInt16 nCount = Recs().Count();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        pRec = Recs()[i];
        if (pRec->aType != sCustom &&
            rRecName == pRec->aType && bCont == pRec->bCont)
        {
            bFound = sal_True;
            break;
        }
    }
    if (!bFound)    // Benutzerdefiniert
        pRec = Recs()[0];

    return(pRec);
}

// --------------------------------------------------------------------------



Printer *SwLabDlg::GetPrt()
{
    if (pPrtPage)
        return (pPrtPage->GetPrt());
    else
        return (NULL);
}

// --------------------------------------------------------------------------

SwLabPage::SwLabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_LAB_LAB), rSet),
    pNewDBMgr(NULL),
    aItem          ((const SwLabItem&) rSet.Get(FN_LABEL)),

    aWritingText   (this, SW_RES(TXT_WRITING)),
    aAddrBox       (this, SW_RES(BOX_ADDR   )),
    aWritingEdit   (this, SW_RES(EDT_WRITING)),
    aDatabaseFT    (this, SW_RES(FT_DATABASE)),
    aDatabaseLB    (this, SW_RES(LB_DATABASE)),
    aTableFT       (this, SW_RES(FT_TABLE   )),
    aTableLB       (this, SW_RES(LB_TABLE   )),
    aInsertBT      (this, SW_RES(BTN_INSERT )),
    aDBFieldFT     (this, SW_RES(FT_DBFIELD )),
    aDBFieldLB     (this, SW_RES(LB_DBFIELD )),
    aWritingGroup  (this, SW_RES(GRP_WRITING)),
    aContButton    (this, SW_RES(BTN_CONT   )),
    aSheetButton   (this, SW_RES(BTN_SHEET  )),
    aMakeText      (this, SW_RES(TXT_MAKE   )),
    aMakeBox       (this, SW_RES(BOX_MAKE   )),
    aTypeText      (this, SW_RES(TXT_TYPE   )),
    aTypeBox       (this, SW_RES(BOX_TYPE   )),
    aFormatInfo    (this, SW_RES(INF_FORMAT )),
    aFormatGroup   (this, SW_RES(GRP_FORMAT ))

{
    WaitObject aWait( pParent );

    FreeResource();
    SetExchangeSupport();


    // Handler installieren
    aAddrBox       .SetClickHdl (LINK(this, SwLabPage, AddrHdl         ));
    aDatabaseLB    .SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    aTableLB       .SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    aInsertBT      .SetClickHdl (LINK(this, SwLabPage, FieldHdl        ));
    aContButton    .SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    aSheetButton   .SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    aMakeBox       .SetSelectHdl(LINK(this, SwLabPage, MakeHdl         ));
    aTypeBox       .SetSelectHdl(LINK(this, SwLabPage, TypeHdl         ));

    InitDatabaseBox();

    sal_uInt16 nLstGroup = 0;

    const sal_uInt16 nCount = (sal_uInt16)GetParent()->Makes().Count();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        String &rStr = *GetParent()->Makes()[i];
        aMakeBox.InsertEntry( rStr );
        if ( rStr == aItem.aLstMake )
            nLstGroup = i;
    }
//  Reset(rSet);
    aMakeBox.SelectEntryPos( nLstGroup );
    aMakeBox.GetSelectHdl().Call( &aMakeBox );
}

// --------------------------------------------------------------------------



SwLabPage::~SwLabPage()
{
}
/* -----------------29.09.99 09:08-------------------

 --------------------------------------------------*/
void lcl_ChgYPos(Window& rWin, long nDiff)
{
    Point aTempPos(rWin.GetPosPixel());
    aTempPos.Y() += nDiff;
    rWin.SetPosPixel(aTempPos);
}

void SwLabPage::SetToBusinessCard()
{
    m_bLabel = sal_False;
    aWritingText.Hide();
    aAddrBox.Hide();
    aWritingEdit.Hide();
    aDatabaseFT.Hide();
    aDatabaseLB.Hide();
    aTableFT.Hide();
    aTableLB.Hide();
    aInsertBT.Hide();
    aDBFieldFT.Hide();
    aDBFieldLB.Hide();
    aWritingGroup.Hide();

    //resize the form
     Point aGBPos(aWritingGroup.GetPosPixel());
    long nDiffPos = aFormatGroup.GetPosPixel().Y() - aGBPos.Y();
     Size aGBSz(aFormatGroup.GetSizePixel());
    aGBSz.Height() += nDiffPos;
    aFormatGroup.SetPosSizePixel(aGBPos, aGBSz);

    // move all controls up
    lcl_ChgYPos(aContButton, -nDiffPos);
    lcl_ChgYPos(aSheetButton, -nDiffPos);
    lcl_ChgYPos(aMakeText, -nDiffPos);

    lcl_ChgYPos(aTypeText, -nDiffPos);
    lcl_ChgYPos(aFormatInfo, -nDiffPos);

    Size aTmpSz(3, 3);
    aTmpSz = LogicToPixel(aTmpSz, MAP_APPFONT);

    //
    aMakeBox.SetStyle((aMakeBox.GetStyle()&~WB_DROPDOWN));
    aTypeBox.SetStyle((aTypeBox.GetStyle()&~WB_DROPDOWN));

    lcl_ChgYPos(aMakeBox, - nDiffPos);
    Point aLBPos(aMakeBox.GetPosPixel());
    aLBPos.Y() += aMakeBox.GetSizePixel().Height() + aTmpSz.Height();
    aTypeBox.SetPosPixel(aLBPos);
};

// --------------------------------------------------------------------------

//impl in envimg.cxx
extern String MakeSender();



IMPL_LINK( SwLabPage, AddrHdl, Button *, EMPTYARG )
{
    String aWriting;
    if ( aAddrBox.IsChecked() )
        aWriting = MakeSender();
    aWritingEdit.SetText( aWriting.ConvertLineEnd() );
    aWritingEdit.GrabFocus();
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwLabPage, DatabaseHdl, ListBox *, pListBox )
{
    sActDBName = SFX_APP()->LocalizeDBName(NATIONAL2INI, aDatabaseLB.GetSelectEntry());

    WaitObject aObj( GetParent() );

    if (pListBox == &aDatabaseLB)
        GetNewDBMgr()->GetTableNames(&aTableLB, sActDBName);
#ifdef REPLACE_OFADBMGR
    GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, aTableLB.GetSelectEntry());
#else
    sActDBName += DB_DELIM;
    sActDBName += aTableLB.GetSelectEntry();
    GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName);
#endif
    return 0;
}



IMPL_LINK( SwLabPage, FieldHdl, Button *, EMPTYARG )
{
    String aStr ( '<' );
    aStr += aDatabaseLB.GetSelectEntry();
    aStr += '.';
    aStr += aTableLB.GetSelectEntry();
    aStr += '.';
    aStr += aDBFieldLB.GetSelectEntry();
    aStr += '>';
    aWritingEdit.ReplaceSelected(aStr);
    Selection aSel = aWritingEdit.GetSelection();
    aWritingEdit.GrabFocus();
    aWritingEdit.SetSelection(aSel);
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK_INLINE_START( SwLabPage, PageHdl, Button *, EMPTYARG )
{
    aMakeBox.GetSelectHdl().Call( &aMakeBox );
    return 0;
}
IMPL_LINK_INLINE_END( SwLabPage, PageHdl, Button *, EMPTYARG )

// --------------------------------------------------------------------------



IMPL_LINK( SwLabPage, MakeHdl, ListBox *, EMPTYARG )
{
    WaitObject aWait( GetParent() );

    aTypeBox.Clear();
    GetParent()->TypeIds().Remove( 0, GetParent()->TypeIds().Count() );

    const String aMake = aMakeBox.GetSelectEntry();
    GetParent()->ReplaceGroup( aMake, &aItem );
    aItem.aLstMake = aMake;

    const sal_Bool   bCont    = aContButton.IsChecked();
    const sal_uInt16 nCount   = GetParent()->Recs().Count();
          sal_uInt16 nLstType = 0;

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        if ( GetParent()->Recs()[i]->aType == String( SW_RES(STR_CUSTOM) ) ||
             GetParent()->Recs()[i]->bCont == bCont )
        {
            const String aType ( GetParent()->Recs()[i]->aType );
            if ( aTypeBox.GetEntryPos(aType) == LISTBOX_ENTRY_NOTFOUND )
            {
                if ( !nLstType && aType == aItem.aLstType )
                    nLstType = GetParent()->TypeIds().Count();
                aTypeBox.InsertEntry( aType );
                GetParent()->TypeIds().Insert(i, GetParent()->TypeIds().Count());
            }
        }
    }
    if (nLstType)
        aTypeBox.SelectEntry(aItem.aLstType);
    else
        aTypeBox.SelectEntryPos(0);
    aTypeBox.GetSelectHdl().Call( &aTypeBox );
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK_INLINE_START( SwLabPage, TypeHdl, ListBox *, EMPTYARG )
{
    DisplayFormat();
    aItem.aType = aTypeBox.GetSelectEntry();
    return 0;
}
IMPL_LINK_INLINE_END( SwLabPage, TypeHdl, ListBox *, EMPTYARG )

// --------------------------------------------------------------------------



void SwLabPage::DisplayFormat()
{
    MetricField aField(this, WinBits(0));
    aField.SetUnit        (FUNIT_CM);
    aField.SetDecimalDigits(2);
    aField.SetMin         (0);
    aField.SetMax         (LONG_MAX);

    SwLabRec* pRec = GetSelectedEntryPos();
    aItem.aLstType = pRec->aType;
    SETFLDVAL(aField, pRec->lWidth);
    aField.Reformat();
    const String aWString = aField.GetText();

    SETFLDVAL(aField, pRec->lHeight);
    aField.Reformat();

    String aText = pRec->aType;
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": "));
    aText += aWString;
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" x "));
    aText += aField.GetText();
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" ("));
    aText += String::CreateFromInt32( pRec->nCols );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" x "));
    aText += String::CreateFromInt32( pRec->nRows );
    aText += ')';
    aFormatInfo.SetText(aText);
}

// --------------------------------------------------------------------------

SwLabRec* SwLabPage::GetSelectedEntryPos()
{
    String sSelEntry(aTypeBox.GetSelectEntry());

    return GetParent()->GetRecord(sSelEntry, aContButton.IsChecked());
}

// --------------------------------------------------------------------------



void SwLabPage::InitDatabaseBox()
{
    if(GetNewDBMgr())
    {
        aDatabaseLB.Clear();
#ifdef REPLACE_OFADBMGR
        Sequence<OUString> aDataNames = SwNewDBMgr::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();
        for (long i = 0; i < aDataNames.getLength(); i++)
            aDatabaseLB.InsertEntry(pDataNames[i]);
        String sDBName = sActDBName.GetToken( 0, DB_DELIM );
        String sTableName = sActDBName.GetToken( 1, DB_DELIM );
        aDatabaseLB.SelectEntry(sDBName);
        if (GetNewDBMgr()->GetTableNames(&aTableLB, sDBName))
        {
            aTableLB.SelectEntry(sTableName);
            GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, sTableName);
        }
        else
            aDBFieldLB.Clear();
#else
        SbaObject *pSbaObject = GetNewDBMgr()->GetSbaObject();
        if(!pSbaObject)
            return;
        String sDBNames = pSbaObject->GetDatabaseNames();
        sActDBName = aItem.sDBName;
        if ( !sActDBName.Len() )
            sActDBName = GetNewDBMgr()->GetAddressDBName();

        sDBNames = SFX_APP()->LocalizeDBName(INI2NATIONAL, sDBNames);

        sal_uInt16 nCount = sDBNames.GetTokenCount();

        for (sal_uInt16 i = 0; i < nCount; i++)
            aDatabaseLB.InsertEntry(sDBNames.GetToken(i));

        String sDBName = SFX_APP()->LocalizeDBName( INI2NATIONAL,
                                        sActDBName.GetToken( 0, DB_DELIM ));
        aDatabaseLB.SelectEntry(sDBName);
        if (GetNewDBMgr()->GetTableNames(&aTableLB, sDBName))
        {
            aTableLB.SelectEntry(sActDBName.GetToken(1, DB_DELIM));
            GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName);
        }
        else
            aDBFieldLB.Clear();
#endif
    }
}

// --------------------------------------------------------------------------



SfxTabPage* SwLabPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabPage(pParent, rSet);
}

// --------------------------------------------------------------------------



void SwLabPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( rSet );
}

// --------------------------------------------------------------------------



int SwLabPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(*pSet);

    return sal_True;
}

// --------------------------------------------------------------------------



void SwLabPage::FillItem(SwLabItem& rItem)
{
    rItem.bAddr    = aAddrBox.IsChecked();
    rItem.aWriting = aWritingEdit.GetText();
    rItem.bCont    = aContButton.IsChecked();
    rItem.aMake    = aMakeBox.GetSelectEntry();
    rItem.aType    = aTypeBox.GetSelectEntry();
    rItem.sDBName  = sActDBName;

    SwLabRec* pRec = GetSelectedEntryPos();

    rItem.lHDist  = pRec->lHDist;
    rItem.lVDist  = pRec->lVDist;
    rItem.lWidth  = pRec->lWidth;
    rItem.lHeight = pRec->lHeight;
    rItem.lLeft   = pRec->lLeft;
    rItem.lUpper  = pRec->lUpper;
    rItem.nCols   = pRec->nCols;
    rItem.nRows   = pRec->nRows;

    rItem.aLstMake = aMakeBox.GetSelectEntry();
    rItem.aLstType = aTypeBox.GetSelectEntry();
}

// --------------------------------------------------------------------------



sal_Bool SwLabPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem( aItem );
    rSet.Put( aItem );

    return sal_True;
}

// --------------------------------------------------------------------------



void SwLabPage::Reset(const SfxItemSet& rSet)
{
    aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    String sDBName  = aItem.sDBName;

    String aWriting( aItem.aWriting );

    aAddrBox    .Check      ( aItem.bAddr );
    aWritingEdit.SetText    ( aWriting.ConvertLineEnd() );
    aMakeBox    .SelectEntry( aItem.aMake );

    if (aTypeBox.GetEntryPos(aItem.aType) != LISTBOX_ENTRY_NOTFOUND)
    {
        aTypeBox.SelectEntry(aItem.aType);
        aTypeBox.GetSelectHdl().Call(&aTypeBox);
    }
    if (aDatabaseLB.GetEntryPos(sDBName) != LISTBOX_ENTRY_NOTFOUND)
    {
        aDatabaseLB.SelectEntry(sDBName);
        aDatabaseLB.GetSelectHdl().Call(&aDatabaseLB);
    }

    if (aItem.bCont)
        aContButton .Check();
    else
        aSheetButton.Check();
}
/*-- 08.07.99 14:00:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwVisitingCardPage::SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_VISITING_CARDS), rSet),
    aAutoTextLB(this,       ResId( LB_AUTO_TEXT         )),
    aAutoTextGroupFT(this,  ResId( FT_AUTO_TEXT_GROUP   )),
    aAutoTextGroupLB(this,  ResId( LB_AUTO_TEXT_GROUP   )),
    aContentGB(this,        ResId( GB_CONTENT           )),
    aExampleWIN(this,       ResId( WIN_EXAMPLE          )),
    aExampleGB(this,        ResId( GB_EXAMPLE           )),
    sVisCardGroup(ResId(ST_VISCARD_GROUP)),
    pExampleFrame(0)
{
    FreeResource();
    aAutoTextLB.SetWindowBits( WB_HSCROLL );
    aAutoTextLB.SetSpaceBetweenEntries(0);
    aAutoTextLB.SetSelectionMode( SINGLE_SELECTION );
    aAutoTextLB.SetHelpId(HID_BUSINESS_CARD_CONTENT);

    SetExchangeSupport();
    aAutoTextLB.SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));
    aAutoTextGroupLB.SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));

    aAutoTextLB.Show();
    aAutoTextGroupFT.Show();
    aAutoTextGroupLB.Show();
    InitFrameControl();
    aExampleWIN.Hide();

}
/*-- 08.07.99 14:00:03---------------------------------------------------

  -----------------------------------------------------------------------*/
SwVisitingCardPage::~SwVisitingCardPage()
{
    for(sal_uInt16 i = 0; i < aAutoTextGroupLB.GetEntryCount(); i++)
        delete (String*)aAutoTextGroupLB.GetEntryData( i );
    _xAutoText = 0;
    SvLBoxEntry* pEntry = aAutoTextLB.First();
    while(pEntry)
    {
        String * pString = (String*)pEntry->GetUserData();
        delete pString;
        pEntry = aAutoTextLB.Next(pEntry);
    }
    delete pExampleFrame;
}
/*-- 08.07.99 14:00:03---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SwVisitingCardPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwVisitingCardPage(pParent, rSet);
}
/*-- 08.07.99 14:00:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwVisitingCardPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( rSet );
    UpdateFields();
}
/*-- 08.07.99 14:00:04---------------------------------------------------

  -----------------------------------------------------------------------*/
int  SwVisitingCardPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(*pSet);
    return LEAVE_PAGE;
}
/*-- 08.07.99 14:00:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwVisitingCardPage::FillItemSet(SfxItemSet& rSet)
{
    String* pGroup = (String*)aAutoTextGroupLB.GetEntryData(
                                    aAutoTextGroupLB.GetSelectEntryPos());
    DBG_ASSERT(pGroup, "no group selected?")
    if(pGroup)
        aLabItem.sGlossaryGroup = *pGroup;

    SvLBoxEntry* pSelEntry = aAutoTextLB.FirstSelected();
    if(pSelEntry)
        aLabItem.sGlossaryBlockName = *(String*)pSelEntry->GetUserData();
    rSet.Put(aLabItem);
    return sal_True;
}
/*-- 08.07.99 14:00:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_SelectBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvLBoxEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*(String*)pEntry->GetUserData() == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            rAutoTextLB.MakeVisible(pEntry);
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
}
//-----------------------------------------------------------------------------
sal_Bool lcl_FindBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvLBoxEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*(String*)pEntry->GetUserData() == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            return sal_True;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
    return sal_False;
}
//-----------------------------------------------------------------------------
void SwVisitingCardPage::Reset(const SfxItemSet& rSet)
{
    aLabItem = (const SwLabItem&) rSet.Get(FN_LABEL);

    sal_Bool bFound = sal_False;
    for(sal_uInt16 i = 0; i < aAutoTextGroupLB.GetEntryCount() && !bFound; i++)
        if(aLabItem.sGlossaryGroup == *(String*)aAutoTextGroupLB.GetEntryData( i ))
        {
            bFound = sal_True;
            break;
        }

    if(bFound)
    {
        if(aAutoTextGroupLB.GetSelectEntryPos() != i)
        {
            aAutoTextGroupLB.SelectEntryPos(i);
            AutoTextSelectHdl(&aAutoTextGroupLB);
        }
        if(lcl_FindBlock(aAutoTextLB, aLabItem.sGlossaryBlockName))
        {
            SvLBoxEntry* pSelEntry = aAutoTextLB.FirstSelected();
            if( pSelEntry &&
                *(String*)pSelEntry->GetUserData() != aLabItem.sGlossaryBlockName)
            {
                lcl_SelectBlock(aAutoTextLB, aLabItem.sGlossaryBlockName);
                AutoTextSelectHdl(&aAutoTextLB);
            }
        }
    }
}
/* -----------------08.07.99 15:15-------------------

 --------------------------------------------------*/
void SwVisitingCardPage::InitFrameControl()
{
    Link aLink(LINK(
                    this, SwVisitingCardPage, FrameControlInitializedHdl));
    pExampleFrame = new SwOneExampleFrame(aExampleWIN, EX_SHOW_BUSINESS_CARDS, &aLink);

    uno::Reference< lang::XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    //now the AutoText ListBoxes have to be filled

    uno::Reference< uno::XInterface >  xAText =
        xMgr->createInstance( C2U("com.sun.star.text.AutoTextContainer") );
    _xAutoText = uno::Reference< container::XNameAccess >(xAText, uno::UNO_QUERY);

    uno::Sequence<OUString> aNames = _xAutoText->getElementNames();
    const OUString* pGroups = aNames.getConstArray();
    for(sal_uInt16 i = 0; i < aNames.getLength(); i++)
    {
        uno::Any aGroup = _xAutoText->getByName(pGroups[i]);
        uno::Reference< text::XAutoTextGroup >  xGroup = *(uno::Reference< text::XAutoTextGroup > *)aGroup.getValue();
        uno::Reference< container::XIndexAccess >  xIdxAcc(xGroup, uno::UNO_QUERY);
        if(!xIdxAcc.is() || xIdxAcc->getCount())
        {
            uno::Reference< beans::XPropertySet >  xPrSet(xGroup, uno::UNO_QUERY);
            uno::Any aTitle = xPrSet->getPropertyValue(CL2S(UNO_NAME_TITLE));
            OUString uTitle;
            aTitle >>= uTitle;
            String sGroup(pGroups[i]);
            sal_uInt16 nEntry = aAutoTextGroupLB.InsertEntry(uTitle);
            aAutoTextGroupLB.SetEntryData(nEntry, new String(sGroup));
        }
    }
    if(LISTBOX_ENTRY_NOTFOUND == aAutoTextGroupLB.GetSelectEntryPos())
        aAutoTextGroupLB.SelectEntryPos(0);
    String sCurGroupName(
        *(String*)aAutoTextGroupLB.GetEntryData(aAutoTextGroupLB.GetSelectEntryPos()));
    if(_xAutoText->hasByName(sCurGroupName))
    {
        uno::Any aGroup = _xAutoText->getByName(sCurGroupName);
        try
        {
            uno::Reference< text::XAutoTextGroup >  xGroup = *(uno::Reference< text::XAutoTextGroup > *)aGroup.getValue();
            uno::Sequence<OUString> aBlockNames = xGroup->getElementNames();
            const OUString* pBlocks = aBlockNames.getConstArray();
            uno::Sequence< OUString > aTitles = xGroup->getTitles() ;
            const OUString* pTitles = aTitles.getConstArray();
            for(i = 0; i < aBlockNames.getLength();i++)
            {
                String sTitle(pTitles[i]);
                SvLBoxEntry* pEntry = aAutoTextLB.InsertEntry(sTitle);
                String sBlock(pBlocks[i]);
                pEntry->SetUserData(new String(sBlock));
            }
        }
        catch( uno::RuntimeException& )
        {
            // we'll be her if path settings were wrong
        }
    }
}
/* -----------------01.10.99 13:19-------------------

 --------------------------------------------------*/
IMPL_LINK( SwVisitingCardPage, FrameControlInitializedHdl, void*, EMPTYARG )
{
    AutoTextSelectHdl(&aAutoTextGroupLB);
    return 0;
};
/* -----------------22.07.99 11:06-------------------

 --------------------------------------------------*/
void lcl_SetUserField(uno::Reference< container::XNameAccess > & xFldMasters,
                const String& rFldName, const String& rContent )
{
    String sFldName( CL2S("com.sun.star.text.FieldMaster.User."));
    sFldName += rFldName;
    OUString uFldName(sFldName);
    if(xFldMasters->hasByName(uFldName))
    {
        uno::Any aFirstName = xFldMasters->getByName(uFldName);
        uno::Reference< beans::XPropertySet >  xFld = *(uno::Reference< beans::XPropertySet > *)aFirstName.getValue();
        uno::Any aContent;
        aContent <<= rtl::OUString(rContent);
        xFld->setPropertyValue(CL2S(UNO_NAME_CONTENT), aContent);
    }
}

IMPL_LINK( SwVisitingCardPage, AutoTextSelectHdl, void*, pBox )
{
    if(pExampleFrame->IsInitialized() && _xAutoText.is())
    {
        sal_Bool bGroup = &aAutoTextGroupLB == pBox;

        String sGroup(
            *(String*)aAutoTextGroupLB.GetEntryData(aAutoTextGroupLB.GetSelectEntryPos()));
        uno::Any aGroup = _xAutoText->getByName(sGroup);
        uno::Reference< text::XAutoTextGroup >  xGroup = *(uno::Reference< text::XAutoTextGroup > *)aGroup.getValue();

        if(bGroup)
        {
            SvLBoxEntry* pEntry = aAutoTextLB.First();
            while(pEntry)
            {
                String * pString = (String*)pEntry->GetUserData();
                delete pString;
                pEntry = aAutoTextLB.Next(pEntry);
            }
            aAutoTextLB.Clear();
            uno::Sequence<OUString> aBlockNames = xGroup->getElementNames();
            const OUString* pBlocks = aBlockNames.getConstArray();
            uno::Sequence< OUString > aTitles = xGroup->getTitles() ;
            const OUString* pTitles = aTitles.getConstArray();
            for(sal_uInt16 i = 0; i < aBlockNames.getLength();i++)
            {
                String sTitle(pTitles[i]);
                SvLBoxEntry* pEntry = aAutoTextLB.InsertEntry(sTitle);
                String sBlock(pBlocks[i]);
                pEntry->SetUserData(new String(sBlock));
            }
        }
        SvLBoxEntry* pSel = aAutoTextLB.FirstSelected();
        String sEntry;
        if(pSel)
            sEntry = *(String*)pSel->GetUserData();
        pExampleFrame->ExecUndo();
        uno::Reference< text::XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
        OUString uEntry(sEntry);
        if(sEntry.Len() && xGroup->hasByName(uEntry))
        {
            uno::Any aEntry(xGroup->getByName(uEntry));
            uno::Reference< text::XAutoTextEntry >  xEntry = *(uno::Reference< text::XAutoTextEntry > *)aEntry.getValue();
            if(xEntry.is())
            {
                uno::Reference< text::XTextRange >  xRange(xCrsr, uno::UNO_QUERY);
                xEntry->applyTo(xRange);
            }
            UpdateFields();
        }
    }
    return 0;
}
/* -----------------01.10.99 11:59-------------------

 --------------------------------------------------*/
void    SwVisitingCardPage::UpdateFields()
{
    uno::Reference< frame::XModel >  xModel;
    if(pExampleFrame && (xModel = pExampleFrame->GetModel()).is())
    {
        SwLabDlg::UpdateFieldInformation(xModel, aLabItem);
    }
}
/* -----------------01.10.99 15:16-------------------

 --------------------------------------------------*/
void SwLabDlg::UpdateFieldInformation(uno::Reference< frame::XModel > & xModel, const SwLabItem& rItem)
{
    uno::Reference< text::XTextFieldsSupplier >  xFlds(xModel, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess >  xFldMasters = xFlds->getTextFieldMasters();

    try
    {
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_FIRSTNAME     ), rItem.aPrivFirstName );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_NAME          ), rItem.aPrivName               );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVSHORTCUT  ), rItem.aPrivShortCut           );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_FIRSTNAME_2   ), rItem.aPrivFirstName2 );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_NAME_2        ), rItem.aPrivName2               );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVSHORTCUT_2 ), rItem.aPrivShortCut2           );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVSTREET        ), rItem.aPrivStreet             );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVZIP       ), rItem.aPrivZip                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVCITY      ), rItem.aPrivCity               );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVCOUNTRY   ), rItem.aPrivCountry            );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVSTATE     ), rItem.aPrivState              );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVTITLE     ), rItem.aPrivTitle              );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVPROFESSION    ), rItem.aPrivProfession         );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVPHONE     ), rItem.aPrivPhone              );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVMOBILE        ), rItem.aPrivMobile             );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVFAX       ), rItem.aPrivFax                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVWWW       ), rItem.aPrivWWW                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_PRIVMAIL      ), rItem.aPrivMail               );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPCOMPANY   ), rItem.aCompCompany            );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPCOMPANYEXT    ), rItem.aCompCompanyExt         );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPSLOGAN        ), rItem.aCompSlogan             );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPSTREET        ), rItem.aCompStreet             );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPZIP       ), rItem.aCompZip                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPCITY      ), rItem.aCompCity               );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPCOUNTRY   ), rItem.aCompCountry            );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPSTATE     ), rItem.aCompState              );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPPOSITION  ), rItem.aCompPosition           );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPPHONE     ), rItem.aCompPhone              );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPMOBILE        ), rItem.aCompMobile             );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPFAX       ), rItem.aCompFax                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPWWW       ), rItem.aCompWWW                );
        lcl_SetUserField(xFldMasters, CL2S(USER_FIELD_COMPMAIL      ), rItem.aCompMail               );
    }
    catch( uno::RuntimeException&)
    {
        //
    }

    uno::Reference< container::XEnumerationAccess >  xFldAcc = xFlds->getTextFields();
    uno::Reference< util::XRefreshable >  xRefresh(xFldAcc, uno::UNO_QUERY);
    xRefresh->refresh();
}

/* -----------------29.09.99 08:55-------------------

 --------------------------------------------------*/
SwPrivateDataPage::SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_PRIVATE_DATA), rSet),
    aDataGB(this, ResId(        GB_DATA         )),
    aNameFT(this, ResId(        FT_NAME         )),
    aFirstNameED(this, ResId(   ED_FIRSTNAME    )),
    aNameED(this, ResId(        ED_NAME         )),
    aShortCutED(this, ResId(    ED_SHORTCUT     )),
    aName2FT(this, ResId(       FT_NAME_2       )),
    aFirstName2ED(this, ResId(  ED_FIRSTNAME_2  )),
    aName2ED(this, ResId(       ED_NAME_2       )),
    aShortCut2ED(this, ResId(   ED_SHORTCUT_2   )),
    aStreetFT(this, ResId(      FT_STREET       )),
    aStreetED(this, ResId(      ED_STREET       )),
    aZipCityFT(this, ResId(     FT_ZIPCITY      )),
    aZipED(this, ResId(         ED_ZIP          )),
    aCityED(this, ResId(        ED_CITY         )),
    aCountryStateFT(this, ResId(FT_COUNTRYSTATE )),
    aCountryED(this, ResId(     ED_COUNTRY      )),
    aStateED(this, ResId(       ED_STATE        )),
    aTitleProfessionFT(this, ResId(FT_TITLEPROF )),
    aProfessionED(this, ResId(  ED_PROFESSION   )),
    aTitleED(this, ResId(       ED_TITLE        )),
    aPhoneFT(this, ResId(       FT_PHONE        )),
    aPhoneED(this, ResId(       ED_PHONE        )),
    aMobilePhoneED(this, ResId( ED_MOBILE       )),
    aFaxFT(this, ResId(         FT_FAX          )),
    aFaxED(this, ResId(         ED_FAX          )),
    aWWWMailFT(this, ResId(     FT_WWWMAIL      )),
    aHomePageED(this, ResId(    ED_WWW          )),
    aMailED(this, ResId(        ED_MAIL         ))
{
    FreeResource();
    SetExchangeSupport();
}

/*-- 29.09.99 08:55:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SwPrivateDataPage::~SwPrivateDataPage()
{
}
/*-- 29.09.99 08:55:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SwPrivateDataPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwPrivateDataPage(pParent, rSet);
}
/*-- 29.09.99 08:55:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwPrivateDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(rSet);
}
/*-- 29.09.99 08:55:58---------------------------------------------------

  -----------------------------------------------------------------------*/
int  SwPrivateDataPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(*pSet);
    return LEAVE_PAGE;
}
/*-- 29.09.99 08:55:58---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwPrivateDataPage::FillItemSet(SfxItemSet& rSet)
{

    SwLabItem aItem = (const SwLabItem&) GetTabDialog()->GetExampleSet()->Get(FN_LABEL);
    aItem.aPrivFirstName = aFirstNameED .GetText();
    aItem.aPrivName      = aNameED      .GetText(  );
    aItem.aPrivShortCut  = aShortCutED  .GetText(  );
    aItem.aPrivFirstName2 = aFirstName2ED   .GetText();
    aItem.aPrivName2     = aName2ED     .GetText(  );
    aItem.aPrivShortCut2 = aShortCut2ED .GetText(  );
    aItem.aPrivStreet    = aStreetED    .GetText(  );
    aItem.aPrivZip       = aZipED       .GetText(  );
    aItem.aPrivCity      = aCityED      .GetText(  );
    aItem.aPrivCountry   = aCountryED   .GetText(  );
    aItem.aPrivState     = aStateED     .GetText(  );
    aItem.aPrivTitle     = aTitleED     .GetText(  );
    aItem.aPrivProfession= aProfessionED.GetText(   );
    aItem.aPrivPhone     = aPhoneED     .GetText(  );
    aItem.aPrivMobile    = aMobilePhoneED.GetText(  );
    aItem.aPrivFax       = aFaxED       .GetText(  );
    aItem.aPrivWWW       = aHomePageED  .GetText(  );
    aItem.aPrivMail      = aMailED      .GetText(  );

    rSet.Put(aItem);
    return sal_True;
}
/*-- 29.09.99 08:55:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwPrivateDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    aFirstNameED.SetText(aItem.aPrivFirstName);
    aNameED     .SetText(aItem.aPrivName);
    aShortCutED .SetText(aItem.aPrivShortCut);
    aFirstName2ED.SetText(aItem.aPrivFirstName2);
    aName2ED     .SetText(aItem.aPrivName2);
    aShortCut2ED .SetText(aItem.aPrivShortCut2);
    aStreetED   .SetText(aItem.aPrivStreet);
    aZipED      .SetText(aItem.aPrivZip);
    aCityED     .SetText(aItem.aPrivCity);
    aCountryED  .SetText(aItem.aPrivCountry);
    aStateED    .SetText(aItem.aPrivState);
    aTitleED    .SetText(aItem.aPrivTitle);
    aProfessionED.SetText(aItem.aPrivProfession);
    aPhoneED    .SetText(aItem.aPrivPhone);
    aMobilePhoneED.SetText(aItem.aPrivMobile);
    aFaxED      .SetText(aItem.aPrivFax);
    aHomePageED .SetText(aItem.aPrivWWW);
    aMailED     .SetText(aItem.aPrivMail);
}
/* -----------------29.09.99 08:56-------------------

 --------------------------------------------------*/
SwBusinessDataPage::SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_BUSINESS_DATA), rSet),
    aDataGB(this, ResId(GB_DATA)),
    aCompanyFT(this, ResId(     FT_COMP     )),
    aCompanyED(this, ResId(     ED_COMP     )),
    aCompanyExtFT(this, ResId(  FT_COMP_EXT )),
    aCompanyExtED(this, ResId(  ED_COMP_EXT )),
    aSloganFT(this, ResId(      FT_SLOGAN   )),
    aSloganED(this, ResId(      ED_SLOGAN   )),
    aStreetFT(this, ResId(      FT_STREET   )),
    aStreetED(this, ResId(      ED_STREET   )),
    aZipCityFT(this, ResId(     FT_ZIPCITY  )),
    aZipED(this, ResId(         ED_ZIP      )),
    aCityED(this, ResId(        ED_CITY     )),
    aCountryStateFT(this, ResId(FT_COUNTRYSTATE )),
    aCountryED(this, ResId(     ED_COUNTRY  )),
    aStateED(this, ResId(       ED_STATE    )),
    aPositionFT(this, ResId(    FT_POSITION )),
    aPositionED(this, ResId(    ED_POSITION )),
    aPhoneFT(this, ResId(       FT_PHONE    )),
    aPhoneED(this, ResId(       ED_PHONE    )),
    aMobilePhoneED(this, ResId( ED_MOBILE   )),
    aFaxED(this, ResId(         ED_FAX      )),
    aWWWMailFT(this, ResId(     FT_WWWMAIL  )),
    aHomePageED(this, ResId(    ED_WWW      )),
    aMailED(this, ResId(        ED_MAIL     ))
{
    FreeResource();
    SetExchangeSupport();
}

/*-- 29.09.99 08:56:06---------------------------------------------------

  -----------------------------------------------------------------------*/
SwBusinessDataPage::~SwBusinessDataPage()
{
}
/*-- 29.09.99 08:56:06---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SwBusinessDataPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwBusinessDataPage(pParent, rSet);
}
/*-- 29.09.99 08:56:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwBusinessDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(rSet);
}
/*-- 29.09.99 08:56:06---------------------------------------------------

  -----------------------------------------------------------------------*/
int  SwBusinessDataPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(*pSet);
    return LEAVE_PAGE;
}
/*-- 29.09.99 08:56:06---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwBusinessDataPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem = (const SwLabItem&) GetTabDialog()->GetExampleSet()->Get(FN_LABEL);

    aItem.aCompCompany   = aCompanyED      .GetText();
    aItem.aCompCompanyExt= aCompanyExtED   .GetText();
    aItem.aCompSlogan    = aSloganED       .GetText();
    aItem.aCompStreet    = aStreetED       .GetText();
    aItem.aCompZip       = aZipED          .GetText();
    aItem.aCompCity      = aCityED         .GetText();
    aItem.aCompCountry   = aCountryED      .GetText();
    aItem.aCompState     = aStateED        .GetText();
    aItem.aCompPosition  = aPositionED     .GetText();
    aItem.aCompPhone     = aPhoneED        .GetText();
    aItem.aCompMobile    = aMobilePhoneED  .GetText();
    aItem.aCompFax       = aFaxED          .GetText();
    aItem.aCompWWW       = aHomePageED     .GetText();
    aItem.aCompMail      = aMailED         .GetText();

    rSet.Put(aItem);
    return sal_True;
}
/*-- 29.09.99 08:56:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwBusinessDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    aCompanyED      .SetText(aItem.aCompCompany);
    aCompanyExtED   .SetText(aItem.aCompCompanyExt);
    aSloganED       .SetText(aItem.aCompSlogan);
    aStreetED       .SetText(aItem.aCompStreet);
    aZipED          .SetText(aItem.aCompZip);
    aCityED         .SetText(aItem.aCompCity);
    aCountryED      .SetText(aItem.aCompCountry);
    aStateED        .SetText(aItem.aCompState);
    aPositionED     .SetText(aItem.aCompPosition);
    aPhoneED        .SetText(aItem.aCompPhone);
    aMobilePhoneED  .SetText(aItem.aCompMobile);
    aFaxED          .SetText(aItem.aCompFax);
    aHomePageED     .SetText(aItem.aCompWWW);
    aMailED         .SetText(aItem.aCompMail);
}

/*-----------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.136  2000/09/18 16:05:25  willem.vandorp
    OpenOffice header added.

    Revision 1.135  2000/07/18 12:50:08  os
    replace ofadbmgr

    Revision 1.134  2000/06/21 13:54:54  os
    unicode error remvoed

    Revision 1.133  2000/06/16 08:31:13  martin.hollmichel
    add: header

    Revision 1.132  2000/06/13 09:59:42  os
    using UCB

    Revision 1.131  2000/05/30 15:09:06  jp
    Bugfix and optimation for unicode

    Revision 1.130  2000/05/16 17:29:11  jp
    Changes for Unicode

    Revision 1.129  2000/05/16 09:15:12  os
    project usr removed

    Revision 1.128  2000/04/18 15:31:35  os
    UNICODE

    Revision 1.127  2000/03/23 08:20:15  os
    UNO III

    Revision 1.126  2000/03/06 15:45:05  os
    #73802# preview improved

    Revision 1.125  2000/02/24 17:37:08  hr
    43447#: gcc

    Revision 1.124  2000/02/14 14:47:58  os
    #70473# Unicode

    Revision 1.123  2000/02/10 11:31:39  os
    #70359# titles added to AutoText groups

    Revision 1.122  2000/02/09 14:08:55  os
    #72787# HelpId added

    Revision 1.121  2000/01/13 11:40:09  os
    #71840# check selected entry

    Revision 1.120  1999/12/27 09:52:27  os
    #71262# Undo in SwOneExampleFrame; TreeListBox as AutoText - box

    Revision 1.119  1999/12/07 16:34:13  os
    #70355##70559# SHORTCUT->INITIALS; second private name

    Revision 1.118  1999/11/26 11:27:36  os
    #70154# check if the text block is available

    Revision 1.117  1999/11/25 15:42:54  os
    headers corrected

    Revision 1.116  1999/11/23 08:14:51  os
    header corrected

    Revision 1.115  1999/11/19 16:40:22  os
    modules renamed

    Revision 1.114  1999/11/19 14:08:47  os
    #69462# Names corrected

    Revision 1.113  1999/11/10 15:39:22  os
    SwOneExampleFrame: CTor changed

    Revision 1.112  1999/11/10 10:53:07  tl
    includes rearranged

    Revision 1.111  1999/10/21 17:48:05  jp
    have to change - SearchFile with SfxIniManager, dont use SwFinder for this

    Revision 1.110  1999/10/08 10:00:05  jp
    no cast from GetpApp to SfxApp

    Revision 1.109  1999/10/01 13:41:17  os
    apply AutoText to business cards

    Revision 1.108  1999/10/01 12:03:55  os
    all fields are available

    Revision 1.107  1999/09/30 07:52:42  os
    new pages for business cards

    Revision 1.106  1999/09/28 13:36:41  os
    #67382# separate dialog for business cards; survive without data base

    Revision 1.105  1999/09/24 13:53:09  os
    chg: ODbRow/ODbVariant - includes moved

    Revision 1.104  1999/09/10 13:07:12  os
    TRY/CATCH block in StarOne AutoText access

    Revision 1.103  1999/08/26 17:36:00  JP
    no cast from GetpApp to SfxApp


      Rev 1.102   26 Aug 1999 19:36:00   JP
   no cast from GetpApp to SfxApp

      Rev 1.101   28 Jul 1999 13:05:08   OS
   new: SwOneExampleFrame

      Rev 1.100   23 Jul 1999 10:06:00   OS
   visiting cards tabpage

      Rev 1.99   19 Jul 1999 15:58:42   MA
   pass a Parent to the wait object

      Rev 1.98   13 Jul 1999 09:32:50   OS
   visiting cards TabPage

      Rev 1.97   09 Jun 1999 19:34:42   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.96   10 Feb 1999 14:33:10   MA
   #61674# Direktdruck fuer Etiketten entfernt

      Rev 1.95   07 Sep 1998 16:59:30   OM
   #55930# Einzelnes Etikett an der korrekten Position drucken

      Rev 1.94   09 Jul 1998 09:52:32   JP
   EmptyStr benutzen

      Rev 1.93   14 Mar 1998 15:12:50   OM
   ExchangeSupport repariert/implementiert

      Rev 1.92   14 Mar 1998 14:26:46   OM
   ExchangeSupport repariert/implementiert

      Rev 1.91   14 Mar 1998 14:10:22   OM
   ExchangeSupport repariert/implementiert

      Rev 1.90   24 Nov 1997 11:52:12   MA
   includes

      Rev 1.89   03 Nov 1997 13:17:12   MA
   precomp entfernt

      Rev 1.88   03 Sep 1997 13:59:14   OM
   #36627# Sinnvolle Fehlermeldungen liefern

      Rev 1.87   02 Sep 1997 09:58:24   OM
   SDB-Headeranpassung

      Rev 1.86   05 May 1997 11:16:30   OM
   Hilfetext fuer OK loeschen

      Rev 1.85   29 Apr 1997 17:09:50   OM
   HelpID fuer UserButton

      Rev 1.84   21 Apr 1997 15:54:50   OM
   #39002# GPF Etiketten gefixt

      Rev 1.83   14 Apr 1997 12:03:36   OM
   #38747# GPF nach Etikettenauswahl behoben

      Rev 1.82   08 Apr 1997 10:10:12   NF
   includes...

      Rev 1.81   04 Apr 1997 14:04:54   OM
   HelpIDs fuer DruckButton

      Rev 1.80   25 Feb 1997 22:47:40   OM
   Unvollstaendige Datenbank am Dialog korrigiert

      Rev 1.79   20 Feb 1997 18:50:12   MA
   fix: letzte Datenbank merken

      Rev 1.78   20 Feb 1997 17:51:40   MA
   fix: Deactivate ueber Fill

      Rev 1.77   04 Dec 1996 13:54:02   OM
   Kein konstanter AdressDBName mehr

      Rev 1.76   11 Nov 1996 09:44:18   MA
   ResMgr

      Rev 1.75   29 Oct 1996 17:59:28   HJS
   includes

      Rev 1.74   24 Oct 1996 13:36:10   JP
   String Umstellung: [] -> GetChar()

      Rev 1.73   25 Sep 1996 14:11:12   OM
   Neue Datenbanktrenner

      Rev 1.72   16 Aug 1996 13:46:18   TRI
   C40_INSERT statt Insert

      Rev 1.71   05 Aug 1996 15:17:36   OM
   Datenbankumstellung

      Rev 1.70   26 Jul 1996 20:36:36   MA
   includes

      Rev 1.69   17 Jul 1996 13:47:04   OM
   Datenbankumstellung 327

*/



