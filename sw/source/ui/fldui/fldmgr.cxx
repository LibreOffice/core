/*************************************************************************
 *
 *  $RCSfile: fldmgr.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:58:41 $
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


#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEFAULTNUMBERINGPROVIDER_HPP_
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGTYPEINFO_HPP_
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>        // Actives Fenster
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // Actives Fenster
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        // Actives Fenster
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _FLDDROPDOWN_HXX
#include <flddropdown.hxx>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::text;
using namespace com::sun::star::style;
using namespace com::sun::star::sdbc;

/*--------------------------------------------------------------------
    Beschreibung: Gruppen der Felder
 --------------------------------------------------------------------*/
enum
{
    GRP_DOC_BEGIN   =  0,
    GRP_DOC_END     =  GRP_DOC_BEGIN + 11,

    GRP_FKT_BEGIN   =  GRP_DOC_END,
    GRP_FKT_END     =  GRP_FKT_BEGIN + 8,

    GRP_REF_BEGIN   =  GRP_FKT_END,
    GRP_REF_END     =  GRP_REF_BEGIN + 2,

    GRP_REG_BEGIN   =  GRP_REF_END,
    GRP_REG_END     =  GRP_REG_BEGIN + 1,

    GRP_DB_BEGIN    =  GRP_REG_END,
    GRP_DB_END      =  GRP_DB_BEGIN  + 5,

    GRP_VAR_BEGIN   =  GRP_DB_END,
    GRP_VAR_END     =  GRP_VAR_BEGIN + 9
};

enum
{
    GRP_WEB_DOC_BEGIN   =  0,
    GRP_WEB_DOC_END     =  GRP_WEB_DOC_BEGIN + 9,

    GRP_WEB_FKT_BEGIN   =  GRP_WEB_DOC_END + 2,
    GRP_WEB_FKT_END     =  GRP_WEB_FKT_BEGIN + 0,   // Die Gruppe ist leer!

    GRP_WEB_REF_BEGIN   =  GRP_WEB_FKT_END + 6,     // Die Gruppe ist leer!
    GRP_WEB_REF_END     =  GRP_WEB_REF_BEGIN + 0,

    GRP_WEB_REG_BEGIN   =  GRP_WEB_REF_END + 2,
    GRP_WEB_REG_END     =  GRP_WEB_REG_BEGIN + 1,

    GRP_WEB_DB_BEGIN    =  GRP_WEB_REG_END,         // Die Gruppe ist leer!
    GRP_WEB_DB_END      =  GRP_WEB_DB_BEGIN  + 0,

    GRP_WEB_VAR_BEGIN   =  GRP_WEB_DB_END + 5,
    GRP_WEB_VAR_END     =  GRP_WEB_VAR_BEGIN + 1
};

/*--------------------------------------------------------------------
    Beschreibung: Formate in der richtigen Reihenfolge
 --------------------------------------------------------------------*/
static const USHORT __FAR_DATA aSetFmt[] =
{
// die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_SETVAR_???
// uebereinstimmen
    0,
    0
};

static const USHORT __FAR_DATA aGetFmt[] =
{
// die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_GETVAR_???
// uebereinstimmen
    0
};

static const USHORT __FAR_DATA aUsrFmt[] =
{
// die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_SETVAR_???
// uebereinstimmen
    0,
    SUB_CMD
};

static const USHORT __FAR_DATA aDBFmt[] =
{
// die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_DBFLD_???
// uebereinstimmen
    SUB_OWN_FMT
};

static const USHORT VF_COUNT        = sizeof(aGetFmt) / sizeof(USHORT);
static const USHORT VF_USR_COUNT    = sizeof(aUsrFmt) / sizeof(USHORT);
static const USHORT VF_DB_COUNT     = sizeof(aDBFmt)  / sizeof(USHORT);

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen und Subtypes
 --------------------------------------------------------------------*/
struct SwFldPack
{
    USHORT  nTypeId;

    USHORT  nSubTypeStart;
    USHORT  nSubTypeEnd;

    ULONG   nFmtBegin;
    ULONG   nFmtEnd;
};

/*--------------------------------------------------------------------
    Beschreibung: Strings und Formate
 --------------------------------------------------------------------*/
static const SwFldPack __FAR_DATA aSwFlds[] =
{
    // Dokument
    TYP_EXTUSERFLD,     FLD_EU_BEGIN,       FLD_EU_END,     0,                  0,
    TYP_AUTHORFLD,      0,                  0,              FMT_AUTHOR_BEGIN,   FMT_AUTHOR_END,
    TYP_DATEFLD,        FLD_DATE_BEGIN,     FLD_DATE_END,   0,                  0,
    TYP_TIMEFLD,        FLD_TIME_BEGIN,     FLD_TIME_END,   0,                  0,
    TYP_PAGENUMBERFLD,  0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-1,
    TYP_NEXTPAGEFLD,    0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END,
    TYP_PREVPAGEFLD,    0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END,
    TYP_FILENAMEFLD,    0,                  0,              FMT_FF_BEGIN,       FMT_FF_END,
    TYP_DOCSTATFLD,     FLD_STAT_BEGIN,     FLD_STAT_END,   FMT_NUM_BEGIN,      FMT_NUM_END-1,

    TYP_CHAPTERFLD,     0,                  0,              FMT_CHAPTER_BEGIN,  FMT_CHAPTER_END,
    TYP_TEMPLNAMEFLD,   0,                  0,              FMT_FF_BEGIN,       FMT_FF_END,

    // Funktion
    TYP_CONDTXTFLD,     0,                  0,              0,                  0,
    TYP_DROPDOWN,       0,                  0,              0,                  0,
    TYP_INPUTFLD,       FLD_INPUT_BEGIN,    FLD_INPUT_END,  0,                  0,
    TYP_MACROFLD,       0,                  0,              0,                  0,
    TYP_JUMPEDITFLD,    0,                  0,              FMT_MARK_BEGIN,     FMT_MARK_END,
    TYP_COMBINED_CHARS,  0,                 0,              0,                  0,
    TYP_HIDDENTXTFLD,   0,                  0,              0,                  0,
    TYP_HIDDENPARAFLD,  0,                  0,              0,                  0,

    // Referenzen
    TYP_SETREFFLD,      0,                  0,              0,                  0,
    TYP_GETREFFLD,      0,                  0,              FMT_REF_BEGIN,      FMT_REF_END,

    // Ablage
    TYP_DOCINFOFLD,     0,                  0,              FMT_REG_BEGIN,      FMT_REG_END,

    // Datenbank
    TYP_DBFLD,          0,                  0,              FMT_DBFLD_BEGIN,    FMT_DBFLD_END,
    TYP_DBNEXTSETFLD,   0,                  0,              0,                  0,
    TYP_DBNUMSETFLD,    0,                  0,              0,                  0,
    TYP_DBSETNUMBERFLD, 0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-2,
    TYP_DBNAMEFLD,      0,                  0,              0,                  0,

    // Variablen
    TYP_SETFLD,         0,                  0,              FMT_SETVAR_BEGIN,   FMT_SETVAR_END,

    TYP_GETFLD,         0,                  0,              FMT_GETVAR_BEGIN,   FMT_GETVAR_END,
    TYP_DDEFLD,         0,                  0,              FMT_DDE_BEGIN,      FMT_DDE_END,
    TYP_FORMELFLD,      0,                  0,              FMT_GETVAR_BEGIN,   FMT_GETVAR_END,
    TYP_INPUTFLD,       FLD_INPUT_BEGIN,    FLD_INPUT_END,  0,                  0,
    TYP_SEQFLD,         0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-2,
    TYP_SETREFPAGEFLD,  FLD_PAGEREF_BEGIN,  FLD_PAGEREF_END,0,                  0,
    TYP_GETREFPAGEFLD,  0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-1,
    TYP_USERFLD,        0,                  0,              FMT_USERVAR_BEGIN,  FMT_USERVAR_END
};

String* SwFldMgr::pDate = 0;
String* SwFldMgr::pTime = 0;


/*--------------------------------------------------------------------
    Beschreibung: Zugriff auf die Shell
 --------------------------------------------------------------------*/

SwWrtShell* lcl_GetShell()
{
    SwView* pView;
    if ( 0 != (pView = ::GetActiveView()) )
        return pView->GetWrtShellPtr();
    DBG_ERROR("no current shell found!")
    return 0;
}

inline USHORT GetPackCount() {  return sizeof(aSwFlds) / sizeof(SwFldPack); }

/*--------------------------------------------------------------------
    Beschreibung: FieldManager regelt das Einfuegen und Updaten
                  von Feldern
 --------------------------------------------------------------------*/

SwFldMgr::SwFldMgr(SwWrtShell* pSh ) :
    bEvalExp(TRUE),
    pModule(0),
    pMacroItem(0),
    pWrtShell(pSh)
{
    // aktuelles Feld ermitteln falls vorhanden
    GetCurFld();
}


SwFldMgr::~SwFldMgr()
{
}

/*--------------------------------------------------------------------
    Beschreibung: RefMark ueber Namen organisieren
 --------------------------------------------------------------------*/

BOOL  SwFldMgr::CanInsertRefMark( const String& rStr )
{
    BOOL bRet = FALSE;
    SwWrtShell *pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(pSh)
    {
        USHORT nCnt = pSh->GetCrsrCnt();

        // der letzte Crsr muss keine aufgespannte Selektion
        if( 1 < nCnt && !pSh->SwCrsrShell::HasSelection() )
            --nCnt;

        bRet =  2 > nCnt && 0 == pSh->GetRefMark( rStr );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Alle DBTypes loeschen
 --------------------------------------------------------------------*/

void SwFldMgr::RemoveDBTypes()
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(pSh)
    {
        USHORT nCount = pSh->GetFldTypeCount(RES_DBFLD);
        for ( USHORT i=0; i < nCount ; ++i )
        {
            SwFieldType* pType = pSh->GetFldType( i, RES_DBFLD );
            if( !pType->GetDepends() )
            {
                pSh->RemoveFldType( i--, RES_DBFLD );
                nCount--;
            }
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Zugriff ueber ResIds
 --------------------------------------------------------------------*/

USHORT SwFldMgr::GetFldTypeCount(USHORT nResId) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    return pSh ? pSh->GetFldTypeCount(nResId) : 0;
}


SwFieldType* SwFldMgr::GetFldType(USHORT nResId, USHORT nId) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    return pSh ? pSh->GetFldType(nId, nResId) : 0;
}


SwFieldType* SwFldMgr::GetFldType(USHORT nResId, const String& rName) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    return pSh ? pSh->GetFldType(nResId, rName) : 0;
}


void SwFldMgr::RemoveFldType(USHORT nResId, USHORT nId)
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if( pSh )
        pSh->RemoveFldType(nId, nResId);
}


void SwFldMgr::RemoveFldType(USHORT nResId, const String& rName )
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if( pSh )
        pSh->RemoveFldType(nResId, rName);
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuelles Feld ermitteln
 --------------------------------------------------------------------*/


SwField* SwFldMgr::GetCurFld()
{
    SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if ( pSh )
        pCurFld = pSh->GetCurFld();
    else
        pCurFld = NULL;

    // Strings und Format initialisieren
    //
    aCurPar1.Erase();
    aCurPar2.Erase();
    sCurFrame.Erase();
    nCurFmt = 0;

    if(!pCurFld)
        return 0;

    // Aktuelle Werte aufbereiten Parameter 1 und Parameter 2
    // als auch das Format ermitteln
    //
    const USHORT nTypeId = pCurFld->GetTypeId();

    nCurFmt     = pCurFld->GetFormat();
    aCurPar1    = pCurFld->GetPar1();
    aCurPar2    = pCurFld->GetPar2();

    switch( nTypeId )
    {
        case TYP_PAGENUMBERFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_GETREFPAGEFLD:
            if( nCurFmt == SVX_NUM_PAGEDESC )
                nCurFmt -= 2;
            break;
    }
    return pCurFld;
}

/*--------------------------------------------------------------------
    Beschreibung: Gruppen-Bereich liefern
 --------------------------------------------------------------------*/


const SwFldGroupRgn& SwFldMgr::GetGroupRange(BOOL bHtmlMode, USHORT nGrpId) const
{
static SwFldGroupRgn __READONLY_DATA aRanges[] =
{
    /* Dokument     */  GRP_DOC_BEGIN,  GRP_DOC_END,
    /* Funktionen   */  GRP_FKT_BEGIN,  GRP_FKT_END,
    /* Referenzen   */  GRP_REF_BEGIN,  GRP_REF_END,
    /* Ablage       */  GRP_REG_BEGIN,  GRP_REG_END,
    /* DB           */  GRP_DB_BEGIN,   GRP_DB_END,
    /* User         */  GRP_VAR_BEGIN,  GRP_VAR_END,
};
static SwFldGroupRgn __READONLY_DATA aWebRanges[] =
{
    /* Dokument     */  GRP_WEB_DOC_BEGIN,  GRP_WEB_DOC_END,
    /* Funktionen   */  GRP_WEB_FKT_BEGIN,  GRP_WEB_FKT_END,
    /* Referenzen   */  GRP_WEB_REF_BEGIN,  GRP_WEB_REF_END,
    /* Ablage       */  GRP_WEB_REG_BEGIN,  GRP_WEB_REG_END,
    /* DB           */  GRP_WEB_DB_BEGIN,   GRP_WEB_DB_END,
    /* User         */  GRP_WEB_VAR_BEGIN,  GRP_WEB_VAR_END,
};

    if (bHtmlMode)
        return aWebRanges[(USHORT)nGrpId];
    else
        return aRanges[(USHORT)nGrpId];
}

/*--------------------------------------------------------------------
    Beschreibung: GroupId bestimmen
 --------------------------------------------------------------------*/

USHORT SwFldMgr::GetGroup(BOOL bHtmlMode, USHORT nTypeId, USHORT nSubType) const
{
    if (nTypeId == TYP_SETINPFLD)
        nTypeId = TYP_SETFLD;

    if (nTypeId == TYP_INPUTFLD && (nSubType & INP_USR))
        nTypeId = TYP_USERFLD;

    if (nTypeId == TYP_FIXDATEFLD)
        nTypeId = TYP_DATEFLD;

    if (nTypeId == TYP_FIXTIMEFLD)
        nTypeId = TYP_TIMEFLD;

    for (USHORT i = GRP_DOC; i <= GRP_VAR; i++)
    {
        const SwFldGroupRgn& rRange = GetGroupRange(bHtmlMode, i);
        for (USHORT nPos = rRange.nStart; nPos < rRange.nEnd; nPos++)
        {
            if (aSwFlds[nPos].nTypeId == nTypeId)
                return i;
        }
    }
    return USHRT_MAX;
}

/*--------------------------------------------------------------------
    Beschreibung: Namen zur TypeId ermitteln
                  ZUGRIFF ueber TYP_....
 --------------------------------------------------------------------*/


USHORT SwFldMgr::GetTypeId(USHORT nPos)
{
    ASSERT(nPos < ::GetPackCount(), "unzulaessige Pos");
    return aSwFlds[ nPos ].nTypeId;
}


const String& SwFldMgr::GetTypeStr(USHORT nPos)
{
    ASSERT(nPos < ::GetPackCount(), "unzulaessige TypeId");

    USHORT nFldWh = aSwFlds[ nPos ].nTypeId;

    // Sonderbehandlung fuer Datum/Zeit Felder (ohne var/fix)
    if( TYP_DATEFLD == nFldWh )
    {
        if( !pDate )
            pDate = new SW_RESSTR( STR_DATEFLD );
        return *pDate;
    }
    if( TYP_TIMEFLD == nFldWh )
    {
        if( !pTime )
            pTime = new SW_RESSTR( STR_TIMEFLD );
        return *pTime;
    }

    return *SwFieldType::GetFldNames()->GetObject( nFldWh );
}

/*--------------------------------------------------------------------
    Beschreibung: Pos in der Liste bestimmen
 --------------------------------------------------------------------*/


USHORT SwFldMgr::GetPos(USHORT nTypeId)
{
    switch( nTypeId )
    {
        case TYP_FIXDATEFLD:        nTypeId = TYP_DATEFLD;      break;
        case TYP_FIXTIMEFLD:        nTypeId = TYP_TIMEFLD;      break;
        case TYP_SETINPFLD:         nTypeId = TYP_SETFLD;       break;
        case TYP_USRINPFLD:         nTypeId = TYP_USERFLD;      break;
    }

    for(USHORT i = 0; i < GetPackCount(); i++)
        if(aSwFlds[i].nTypeId == nTypeId)
            return i;

    return USHRT_MAX;
}

/*--------------------------------------------------------------------
    Beschreibung: Subtypen eines Feldes lokalisieren
 --------------------------------------------------------------------*/

BOOL SwFldMgr::GetSubTypes(USHORT nTypeId, SvStringsDtor& rToFill)
{
    BOOL bRet = FALSE;
    SwWrtShell *pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(pSh)
    {
        const USHORT nPos = GetPos(nTypeId);

        switch(nTypeId)
        {
            case TYP_SETREFFLD:
            case TYP_GETREFFLD:
            {
                // Referenzen sind keine Felder
                pSh->GetRefMarks( &rToFill );
                break;
            }
            case TYP_MACROFLD:
            {
                if (sMacroPath.Len())
                {
                    String sPath = sMacroPath.Copy(sMacroPath.Search('.', 0));
                    if (pModule)
                    {
                        SbxArray* pSbxArray = pModule->GetMethods();

                        for (USHORT nMethod = 0; nMethod < pSbxArray->Count(); nMethod++)
                        {
                            SbMethod* pMethod = (SbMethod*)pSbxArray->Get(nMethod);
                            DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );

                            String* pNew = new String(pMethod->GetName());
                            *pNew += sPath;
                            rToFill.Insert(pNew, rToFill.Count());
                        }
                    }
                    else
                    {
                        String* pNew = new String(sMacroPath);
                        rToFill.Insert(pNew, rToFill.Count());
                    }
                }
                break;
            }
            case TYP_INPUTFLD:
            {   String* pNew = new SW_RESSTR(aSwFlds[nPos].nSubTypeStart);
                rToFill.Insert(pNew, rToFill.Count());
                // Weiter bei generischen Typen
            }
            case TYP_DDEFLD:
            case TYP_SEQFLD:
            case TYP_FORMELFLD:
            case TYP_GETFLD:
            case TYP_SETFLD:
            case TYP_USERFLD:
            {

                const USHORT nCount = pSh->GetFldTypeCount();
                for(USHORT i = 0; i < nCount; ++i)
                {
                    SwFieldType* pFldType = pSh->GetFldType( i );
                    const USHORT nWhich = pFldType->Which();

                    if((nTypeId == TYP_DDEFLD && pFldType->Which() == RES_DDEFLD) ||

                       (nTypeId == TYP_USERFLD && nWhich == RES_USERFLD) ||

                       (nTypeId == TYP_GETFLD && nWhich == RES_SETEXPFLD &&
                        !(((SwSetExpFieldType*)pFldType)->GetType() & GSE_SEQ)) ||

                       (nTypeId == TYP_SETFLD && nWhich == RES_SETEXPFLD &&
                        !(((SwSetExpFieldType*)pFldType)->GetType() & GSE_SEQ)) ||

                       (nTypeId == TYP_SEQFLD && nWhich == RES_SETEXPFLD  &&
                       (((SwSetExpFieldType*)pFldType)->GetType() & GSE_SEQ)) ||

                       ((nTypeId == TYP_INPUTFLD  || nTypeId == TYP_FORMELFLD) &&
                         (nWhich == RES_USERFLD ||
                          nWhich == RES_SETEXPFLD &&
                          !(((SwSetExpFieldType*)pFldType)->GetType() & GSE_SEQ)) ) )
                    {
                        String* pNew = new String(pFldType->GetName());
                        rToFill.Insert(pNew, rToFill.Count());
                    }
                }
                break;
            }
            case TYP_DBNEXTSETFLD:
            case TYP_DBNUMSETFLD:
            case TYP_DBNAMEFLD:
            case TYP_DBSETNUMBERFLD:
                break;

            default:
            {
                // statische SubTypes
                if(nPos != USHRT_MAX)
                {
                    USHORT nCount;
                    if (nTypeId == TYP_DOCINFOFLD)
                        nCount = DI_SUBTYPE_END - DI_SUBTYPE_BEGIN;
                    else
                        nCount = aSwFlds[nPos].nSubTypeEnd - aSwFlds[nPos].nSubTypeStart;

                    for(USHORT i = 0; i < nCount; ++i)
                    {
                        String* pNew;
                        if (nTypeId == TYP_DOCINFOFLD)
                        {
                            if (i >= DI_INFO1 && i <= DI_INFO4)
                                pNew = new String( pSh->GetInfo()->GetUserKey(i-DI_INFO1).GetTitle());
                            else
                                pNew = new String(*ViewShell::GetShellRes()->aDocInfoLst[i]);
                        }
                        else
                            pNew = new SW_RESSTR(aSwFlds[nPos].nSubTypeStart + i);

                        rToFill.Insert(pNew, rToFill.Count());
                    }
                }
            }
        }
        bRet = TRUE;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Format ermitteln
                  ZUGRIFF ueber TYP_....
 --------------------------------------------------------------------*/


USHORT SwFldMgr::GetFormatCount(USHORT nTypeId, BOOL bIsText, BOOL bHtmlMode) const
{
    ASSERT(nTypeId < TYP_END, "unzulaessige TypeId");

    {
        const USHORT nPos = GetPos(nTypeId);

        if(nPos == USHRT_MAX || (bHtmlMode && nTypeId == TYP_SETFLD))
            return 0;

        ULONG nStart = aSwFlds[nPos].nFmtBegin;
        ULONG nEnd   = aSwFlds[nPos].nFmtEnd;

        if (bIsText && nEnd - nStart >= 2)
            return 2;

        if (nTypeId == TYP_FILENAMEFLD)
            nEnd -= 2;  // Kein Bereich oder Vorlage

        switch(nStart)
        {
            case FMT_GETVAR_BEGIN:
            case FMT_SETVAR_BEGIN:  return VF_COUNT;
            case FMT_USERVAR_BEGIN: return VF_USR_COUNT;
            case FMT_DBFLD_BEGIN:   return VF_DB_COUNT;
            case FMT_NUM_BEGIN:
            {
                USHORT nCount = (USHORT)(nEnd - nStart);
                GetNumberingInfo();
                SvtLanguageOptions aLangOpt;
                if(xNumberingInfo.is() &&
                    (aLangOpt.IsCJKFontEnabled()||aLangOpt.IsCTLFontEnabled()))
                {
                    Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
                    const sal_Int16* pTypes = aTypes.getConstArray();
                    for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
                    {
                        sal_Int16 nCurrent = pTypes[nType];
                        //skip all values below or equal to CHARS_LOWER_LETTER_N
                        if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                        {
                            nCount += aTypes.getLength() - nType;
                            break;
                        }
                    }
                }
                return nCount;
            }

        }
        return (USHORT)(nEnd - nStart);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   FormatString zu einem Typ ermitteln
 --------------------------------------------------------------------*/


String SwFldMgr::GetFormatStr(USHORT nTypeId, ULONG nFormatId) const
{
    String aRet;
    ASSERT(nTypeId < TYP_END, "unzulaessige TypeId");

    const USHORT nPos = GetPos(nTypeId);

    if(nPos == USHRT_MAX)
        return aRet;

    ULONG nStart;

    nStart = aSwFlds[nPos].nFmtBegin;

    if (TYP_AUTHORFLD == nTypeId|| TYP_FILENAMEFLD == nTypeId)
        nFormatId &= ~FF_FIXED;     // Fixed-Flag ausmaskieren

    if((nStart + nFormatId) < aSwFlds[nPos].nFmtEnd)
        aRet = SW_RESSTR((USHORT)(nStart + nFormatId));
    else if( FMT_NUM_BEGIN == nStart)
    {
        if(xNumberingInfo.is())
        {
            Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
            const sal_Int16* pTypes = aTypes.getConstArray();
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    sal_Int32 nOffset = nFormatId - (aSwFlds[nPos].nFmtEnd - nStart);
                    if(aTypes.getLength() > (nOffset + nType))
                        aRet = xNumberingInfo->getNumberingIdentifier( pTypes[nOffset + nType] );
                    break;
                }
            }
        }
    }

    return aRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   FormatId aus Pseudo-ID ermitteln
 --------------------------------------------------------------------*/

USHORT SwFldMgr::GetFormatId(USHORT nTypeId, ULONG nFormatId) const
{
    USHORT nId = (USHORT)nFormatId;

    switch( nTypeId )
    {
    case TYP_DOCINFOFLD:
        switch( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
        {
        case FMT_REG_AUTHOR:    nId = DI_SUB_AUTHOR;    break;
        case FMT_REG_TIME:      nId = DI_SUB_TIME;      break;
        case FMT_REG_DATE:      nId = DI_SUB_DATE;      break;
        }
        break;

    case TYP_PAGENUMBERFLD:
    case TYP_NEXTPAGEFLD:
    case TYP_PREVPAGEFLD:
    case TYP_DOCSTATFLD:
    case TYP_DBSETNUMBERFLD:
    case TYP_SEQFLD:
    case TYP_GETREFPAGEFLD:
    {
        USHORT nPos = GetPos( nTypeId );
        ULONG nBegin = aSwFlds[ nPos ].nFmtBegin;
        ULONG nEnd = aSwFlds[nPos].nFmtEnd;
        if((nBegin + nFormatId) < nEnd)
        {
            switch( nBegin + nFormatId )
            {
            case FMT_NUM_ABC:               nId = SVX_NUM_CHARS_UPPER_LETTER;   break;
            case FMT_NUM_SABC:              nId = SVX_NUM_CHARS_LOWER_LETTER;   break;
            case FMT_NUM_ROMAN:             nId = SVX_NUM_ROMAN_UPPER;          break;
            case FMT_NUM_SROMAN:            nId = SVX_NUM_ROMAN_LOWER;          break;
            case FMT_NUM_ARABIC:            nId = SVX_NUM_ARABIC;               break;
            case FMT_NUM_PAGEDESC:          nId = SVX_NUM_PAGEDESC;             break;
            case FMT_NUM_PAGESPECIAL:       nId = SVX_NUM_CHAR_SPECIAL;         break;
            case FMT_NUM_ABC_N:             nId = SVX_NUM_CHARS_UPPER_LETTER_N; break;
            case FMT_NUM_SABC_N:            nId = SVX_NUM_CHARS_LOWER_LETTER_N; break;
            }
        }
        else if(xNumberingInfo.is())
        {
            Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
            const sal_Int16* pTypes = aTypes.getConstArray();
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                //skip all values below or equal to CHARS_LOWER_LETTER_N
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    sal_Int32 nOffset = nFormatId + nBegin - aSwFlds[nPos].nFmtEnd;
                    if((nOffset + nType) < aTypes.getLength())
                    {
                        nId = pTypes[nOffset + nType];
                        break;
                    }
                }
            }
        }
    }
    break;
    case TYP_DDEFLD:
        switch ( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
        {
        case FMT_DDE_NORMAL:    nId = so3::LINKUPDATE_ONCALL; break;
        case FMT_DDE_HOT:       nId = so3::LINKUPDATE_ALWAYS; break;
        }
        break;
    }

    return nId;

}

/*--------------------------------------------------------------------
    Beschreibung: Traveling
 --------------------------------------------------------------------*/


BOOL SwFldMgr::GoNextPrev( BOOL bNext, SwFieldType* pTyp )
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if(!pSh)
        return FALSE;

    if( !pTyp && pCurFld )
    {
        const USHORT nTypeId = pCurFld->GetTypeId();
        if( TYP_SETINPFLD == nTypeId || TYP_USRINPFLD == nTypeId )
            pTyp = pSh->GetFldType( 0, RES_INPUTFLD );
        else
            pTyp = pCurFld->GetTyp();
    }

    if (pTyp && pTyp->Which() == RES_DBFLD)
    {
        // Fuer Feldbefehl-bearbeiten (alle DB-Felder anspringen)
        return pSh->MoveFldType( 0, bNext, USHRT_MAX, RES_DBFLD );
    }

    return pTyp && pSh ? pSh->MoveFldType( pTyp, bNext ) : FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen einfuegen
 --------------------------------------------------------------------*/


void SwFldMgr::InsertFldType(SwFieldType& rType)
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(pSh)
        pSh->InsertFldType(rType);
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuelle TypeId ermitteln
 --------------------------------------------------------------------*/


USHORT SwFldMgr::GetCurTypeId() const
{
    return pCurFld ? pCurFld->GetTypeId() : USHRT_MAX;
}

/*--------------------------------------------------------------------
    Beschreibung: Ueber String  Feld einfuegen oder Update
 --------------------------------------------------------------------*/


BOOL SwFldMgr::InsertFld(  const SwInsertFld_Data& rData )
{
    SwField* pFld   = 0;
    BOOL bExp = FALSE;
    BOOL bTbl = FALSE;
    BOOL bPageVar = FALSE;
    ULONG nFormatId = rData.nFormatId;
    USHORT nSubType = rData.nSubType;
    sal_Unicode cSeparator = rData.cSeparator;
    SwWrtShell* pCurShell = rData.pSh;
    if(!pCurShell)
        pCurShell = pWrtShell ? pWrtShell : ::lcl_GetShell();
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    DBG_ASSERT(pCurShell, "no SwWrtShell found")
    if(!pCurShell)
        return FALSE;

    switch(rData.nTypeId)
    {   // ACHTUNG dieses Feld wird ueber einen gesonderten Dialog eingefuegt
        case TYP_POSTITFLD:
        {
            SwPostItFieldType* pType =
                (SwPostItFieldType*)pCurShell->GetFldType(0, RES_POSTITFLD);
            pFld = new SwPostItField(pType, rData.sPar1, rData.sPar2, Date());
            break;
        }
        case TYP_SCRIPTFLD:
        {
            SwScriptFieldType* pType =
                (SwScriptFieldType*)pCurShell->GetFldType(0, RES_SCRIPTFLD);
            pFld = new SwScriptField(pType, rData.sPar1, rData.sPar2, (BOOL)nFormatId);
            break;
        }
        case TYP_COMBINED_CHARS:
        {
            SwCombinedCharFieldType* pType = (SwCombinedCharFieldType*)
                                pCurShell->GetFldType( 0, RES_COMBINED_CHARS );
            pFld = new SwCombinedCharField( pType, rData.sPar1 );
        }
        break;
        case TYP_AUTHORITY:
        {
            SwAuthorityFieldType* pType =
                (SwAuthorityFieldType*)pCurShell->GetFldType(0, RES_AUTHORITY);
            if(!pType)
            {
                pType =
                    (SwAuthorityFieldType*)pCurShell->InsertFldType(
                                    SwAuthorityFieldType(pCurShell->GetDoc()));
            }
            pFld = new SwAuthorityField(pType, rData.sPar1);
        }
        break;
        case TYP_DATEFLD:
        case TYP_TIMEFLD:
        {
            USHORT nSub = (rData.nTypeId == TYP_DATEFLD) ? DATEFLD : TIMEFLD;
            nSub |= nSubType == DATE_VAR ? 0 : FIXEDFLD;

            SwDateTimeFieldType* pTyp =
                (SwDateTimeFieldType*)pCurShell->GetFldType(0, RES_DATETIMEFLD);
            pFld = new SwDateTimeField(pTyp, nSub, nFormatId);
            pFld->SetPar2(rData.sPar2);
            break;
        }
        case TYP_FILENAMEFLD:
        {
            SwFileNameFieldType* pTyp =
                (SwFileNameFieldType*)pCurShell->GetFldType(0, RES_FILENAMEFLD);
            pFld = new SwFileNameField(pTyp, nFormatId);
            break;
        }
        case TYP_TEMPLNAMEFLD:
        {
            SwTemplNameFieldType* pTyp =
                (SwTemplNameFieldType*)pCurShell->GetFldType(0, RES_TEMPLNAMEFLD);
            pFld = new SwTemplNameField(pTyp, nFormatId);
            break;
        }
        case TYP_CHAPTERFLD:
        {
            USHORT nByte = (USHORT)rData.sPar2.ToInt32();
            SwChapterFieldType* pTyp =
                (SwChapterFieldType*)pCurShell->GetFldType(0, RES_CHAPTERFLD);
            pFld = new SwChapterField(pTyp, nFormatId);
            nByte = Max(USHORT(1), nByte);
            nByte = Min(nByte, USHORT(MAXLEVEL));
            nByte -= 1;
            ((SwChapterField*)pFld)->SetLevel((BYTE)nByte);
            break;
        }
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_PAGENUMBERFLD:
        {
            short nOff  = (short)rData.sPar2.ToInt32();

            if(rData.nTypeId == TYP_NEXTPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormatId )
                    nOff = 1;
                else
                    nOff += 1;
                nSubType = PG_NEXT;
            }
            else if(rData.nTypeId == TYP_PREVPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormatId )
                    nOff = -1;
                else
                    nOff -= 1;
                nSubType =  PG_PREV;
            }
            else
                nSubType = PG_RANDOM;

            SwPageNumberFieldType* pTyp =
                (SwPageNumberFieldType*)pCurShell->GetFldType(0, RES_PAGENUMBERFLD);
            pFld = new SwPageNumberField(pTyp, nSubType, nFormatId, nOff);

            if( SVX_NUM_CHAR_SPECIAL == nFormatId &&
                ( PG_PREV == nSubType || PG_NEXT == nSubType ) )
                ((SwPageNumberField*)pFld)->SetUserString( rData.sPar2 );
            break;
        }
        case TYP_DOCSTATFLD:
        {   SwDocStatFieldType* pTyp =
                (SwDocStatFieldType*)pCurShell->GetFldType(0, RES_DOCSTATFLD);
            pFld = new SwDocStatField(pTyp, nSubType, nFormatId);
            break;
        }
        case TYP_AUTHORFLD:
        {   SwAuthorFieldType* pTyp =
                (SwAuthorFieldType*)pCurShell->GetFldType(0, RES_AUTHORFLD);
            pFld = new SwAuthorField(pTyp, nFormatId);
            break;
        }
        case TYP_CONDTXTFLD:
        case TYP_HIDDENTXTFLD:
        {
            SwHiddenTxtFieldType* pTyp =
                (SwHiddenTxtFieldType*)pCurShell->GetFldType(0, RES_HIDDENTXTFLD);
            pFld = new SwHiddenTxtField(pTyp, TRUE, rData.sPar1, rData.sPar2, FALSE, rData.nTypeId);
            bExp = TRUE;
            break;
        }
        case TYP_HIDDENPARAFLD:
        {
            SwHiddenParaFieldType* pTyp =
                (SwHiddenParaFieldType*)pCurShell->GetFldType(0, RES_HIDDENPARAFLD);
            pFld = new SwHiddenParaField(pTyp, rData.sPar1);
            bExp = TRUE;
            break;
        }
        case TYP_SETREFFLD:
        {
            if( rData.sPar1.Len() > 0 && CanInsertRefMark( rData.sPar1 ) )
            {
                pCurShell->SetAttr( SwFmtRefMark( rData.sPar1 ) );
                return TRUE;
            }
            return FALSE;
        }
        case TYP_GETREFFLD:
        {
            SwGetRefFieldType* pTyp =
                (SwGetRefFieldType*)pCurShell->GetFldType(0, RES_GETREFFLD);
            USHORT nSeqNo = (USHORT)rData.sPar2.ToInt32();
            pFld = new SwGetRefField(pTyp, rData.sPar1, nSubType, nSeqNo, nFormatId);
            bExp = TRUE;
            break;
        }
        case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
            //              Namen haben! Wird hier noch nicht beachtet.
            String sCmd( rData.sPar2 );
            USHORT nTmpPos = sCmd.SearchAndReplace( ' ', so3::cTokenSeperator );
            sCmd.SearchAndReplace( ' ', so3::cTokenSeperator, nTmpPos );

            SwDDEFieldType* pTyp = (SwDDEFieldType*)pCurShell->InsertFldType(
                    SwDDEFieldType( rData.sPar1, sCmd, (USHORT)nFormatId ));
            pFld = new SwDDEField( pTyp );
            break;
        }
        case TYP_MACROFLD:
        {
            SwMacroFieldType* pTyp =
                (SwMacroFieldType*)pCurShell->GetFldType(0, RES_MACROFLD);

            pFld = new SwMacroField(pTyp, rData.sPar1, rData.sPar2);
            break;
        }
        case TYP_INTERNETFLD:
        {
            SwFmtINetFmt aFmt( rData.sPar1, sCurFrame );
            if( pMacroItem )
                aFmt.SetMacroTbl( &pMacroItem->GetMacroTable() );
            return pCurShell->InsertURL( aFmt, rData.sPar2 );
        }
        case TYP_JUMPEDITFLD:
        {
            SwJumpEditFieldType* pTyp =
                (SwJumpEditFieldType*)pCurShell->GetFldType(0, RES_JUMPEDITFLD);

            pFld = new SwJumpEditField(pTyp, nFormatId, rData.sPar1, rData.sPar2 );
            break;
        }
        case TYP_DOCINFOFLD:
        {
            SwDocInfoFieldType* pTyp = (SwDocInfoFieldType*)pCurShell->GetFldType(
                                            0, RES_DOCINFOFLD );
            pFld = new SwDocInfoField(pTyp, nSubType, nFormatId);
            break;
        }
        case TYP_EXTUSERFLD:
        {
            SwExtUserFieldType* pTyp = (SwExtUserFieldType*)pCurShell->GetFldType(
                                            0, RES_EXTUSERFLD);
            pFld = new SwExtUserField(pTyp, nSubType, nFormatId);
            break;
        }
        case TYP_DBFLD:
        {
            SwDBData aDBData;
            String sPar1;

            if (rData.sPar1.Search(DB_DELIM) == STRING_NOTFOUND)
            {
                aDBData = pCurShell->GetDBData();
                sPar1 = rData.sPar1;
            }
            else
            {
                aDBData.sDataSource = rData.sPar1.GetToken(0, DB_DELIM);
                aDBData.sCommand = rData.sPar1.GetToken(1, DB_DELIM);
                aDBData.nCommandType = rData.sPar1.GetToken(2, DB_DELIM).ToInt32();
                sPar1 = rData.sPar1.GetToken(3, DB_DELIM);
            }

            if(aDBData.sDataSource.getLength() && pCurShell->GetDBData() != aDBData)
                pCurShell->ChgDBData(aDBData);

            SwDBFieldType* pTyp = (SwDBFieldType*)pCurShell->InsertFldType(
                    SwDBFieldType(pCurShell->GetDoc(), sPar1, aDBData) );
            pFld = new SwDBField(pTyp);
            pFld->SetSubType(nSubType);

            if( !(nSubType & SUB_OWN_FMT) ) // Datenbankformat ermitteln
            {
                Reference< XDataSource> xSource;
                rData.aDBDataSource >>= xSource;
                Reference<XConnection> xConnection;
                rData.aDBConnection >>= xConnection;
                Reference<XPropertySet> xColumn;
                rData.aDBColumn >>= xColumn;
                if(xColumn.is())
                {
                    nFormatId = pCurShell->GetNewDBMgr()->GetColumnFmt(xSource, xConnection, xColumn,
                                        pCurShell->GetNumberFormatter(), GetCurrLanguage() );
                }
                else
                    nFormatId = pCurShell->GetNewDBMgr()->GetColumnFmt(
                                        aDBData.sDataSource, aDBData.sCommand, sPar1,
                                        pCurShell->GetNumberFormatter(), GetCurrLanguage() );
            }
            pFld->ChangeFormat( nFormatId );

            bExp = TRUE;
            break;
        }
        case TYP_DBSETNUMBERFLD:
        case TYP_DBNUMSETFLD:
        case TYP_DBNEXTSETFLD:
        case TYP_DBNAMEFLD:
        {
            USHORT nPos, nTablePos, nCmdTypePos, nExpPos;
            String sPar1;
            SwDBData aDBData;

            // DBName aus rData.sPar1 extrahieren. Format: DBName.TableName.CommandType.ExpStrg
            if ((nTablePos = rData.sPar1.Search(DB_DELIM)) != STRING_NOTFOUND)
                aDBData.sDataSource = rData.sPar1.Copy(0, nTablePos++);
            if ((nCmdTypePos = rData.sPar1.Search(DB_DELIM, nTablePos)) != STRING_NOTFOUND)
            {
                aDBData.sCommand = rData.sPar1.Copy(nTablePos, nCmdTypePos++ - nTablePos);
            }
            if ((nExpPos = rData.sPar1.Search(DB_DELIM, nCmdTypePos)) != STRING_NOTFOUND)
            {
                aDBData.nCommandType = rData.sPar1.Copy(nCmdTypePos, nExpPos++ - nCmdTypePos).ToInt32();
            }
            if (nExpPos != STRING_NOTFOUND)
                nPos = nExpPos;
            else if (nTablePos != STRING_NOTFOUND)
                nPos = nTablePos;
            else
                nPos = 0;
            sPar1 = rData.sPar1.Copy(nPos);

            if (aDBData.sDataSource.getLength() && pCurShell->GetDBData() != aDBData)
                pCurShell->ChgDBData(aDBData);

            switch(rData.nTypeId)
            {
                case TYP_DBNAMEFLD:
                {
                    SwDBNameFieldType* pTyp =
                        (SwDBNameFieldType*)pCurShell->GetFldType(0, RES_DBNAMEFLD);
                    pFld = new SwDBNameField(pTyp, aDBData);

                    break;
                }
                case TYP_DBNEXTSETFLD:
                {
                    SwDBNextSetFieldType* pTyp = (SwDBNextSetFieldType*)pCurShell->GetFldType(
                                                    0, RES_DBNEXTSETFLD);
                    pFld = new SwDBNextSetField(pTyp, sPar1, rData.sPar2, aDBData);
                    bExp = TRUE;
                    break;
                }
                case TYP_DBNUMSETFLD:
                {
                    SwDBNumSetFieldType* pTyp = (SwDBNumSetFieldType*)pCurShell->GetFldType(
                                                    0, RES_DBNUMSETFLD);
                    pFld = new SwDBNumSetField( pTyp, sPar1, rData.sPar2, aDBData);
                    bExp = TRUE;
                    break;
                }
                case TYP_DBSETNUMBERFLD:
                {
                    SwDBSetNumberFieldType* pTyp = (SwDBSetNumberFieldType*)
                                                pCurShell->GetFldType(0, RES_DBSETNUMBERFLD);
                    pFld = new SwDBSetNumberField( pTyp, aDBData, nFormatId);
                    bExp = TRUE;
                    break;
                }
            }
            break;
        }
        case TYP_USERFLD:
        {
            SwUserFieldType* pTyp =
                (SwUserFieldType*)pCurShell->GetFldType(RES_USERFLD, rData.sPar1);

            // nur wenn vorhanden
            if(!pTyp)
            {
                pTyp = (SwUserFieldType*)pCurShell->InsertFldType(
                        SwUserFieldType(pCurShell->GetDoc(), rData.sPar1));
            }
            if (pTyp->GetContent(nFormatId) != rData.sPar2)
                pTyp->SetContent(rData.sPar2, nFormatId);
            pFld = new SwUserField(pTyp, 0, nFormatId);
            if (pFld->GetSubType() != nSubType)
                pFld->SetSubType(nSubType);
            bTbl = TRUE;
            break;
        }
        case TYP_INPUTFLD:
        {
            if ((nSubType & 0x00ff) == INP_VAR)
            {
                SwSetExpFieldType* pTyp = (SwSetExpFieldType*)
                                    pCurShell->GetFldType(RES_SETEXPFLD, rData.sPar1);

                // kein Experssion Type mit dem Namen vorhanden -> anlegen
                if(pTyp)
                {
                    SwSetExpField* pExpFld =
                        new SwSetExpField(pTyp, aEmptyStr, nFormatId);

                    // Typ vom SwSetExpFieldType nicht veraendern:
                    USHORT nOldSubType = pExpFld->GetSubType();
                    pExpFld->SetSubType(nOldSubType | (nSubType & 0xff00));

                    pExpFld->SetPromptText(rData.sPar2);
                    pExpFld->SetInputFlag(TRUE) ;
                    bExp = TRUE;
                    pFld = pExpFld;
                }
                else
                    return FALSE;
            }
            else
            {
                SwInputFieldType* pTyp =
                    (SwInputFieldType*)pCurShell->GetFldType(0, RES_INPUTFLD);

                SwInputField* pInpFld =
                    new SwInputField(pTyp, rData.sPar1, rData.sPar2, nSubType|SUB_INVISIBLE, nFormatId);
                pFld = pInpFld;
            }

            // Dialog starten
            //
            pCurShell->StartInputFldDlg(pFld, FALSE);
            break;
        }
        case TYP_SETFLD:
        {
            if (!rData.sPar2.Len())   // Leere Variablen sind nicht erlaubt
                return FALSE;

            SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pCurShell->InsertFldType(
                    SwSetExpFieldType(pCurShell->GetDoc(), rData.sPar1) );

            SwSetExpField* pExpFld = new SwSetExpField( pTyp, rData.sPar2, nFormatId);
            pExpFld->SetSubType(nSubType);
            pExpFld->SetPar2(rData.sPar2);
            bExp = TRUE;
            pFld = pExpFld;
            break;
        }
        case TYP_SEQFLD:
        {
            SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pCurShell->InsertFldType(
                    SwSetExpFieldType(pCurShell->GetDoc(), rData.sPar1, GSE_SEQ));

            BYTE nLevel = nSubType & 0xff;

            pTyp->SetOutlineLvl(nLevel);
            if (nLevel != 0x7f && cSeparator == 0)
                cSeparator = '.';

            pTyp->SetDelimiter(cSeparator);
            SwSetExpField* pExpFld = new SwSetExpField(pTyp, rData.sPar2, nFormatId);
            bExp = TRUE;
            pFld = pExpFld;
            nSubType = GSE_SEQ;
            break;
        }
        case TYP_GETFLD:
        {
            // gibt es ein entprechendes SetField
            SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                    pCurShell->GetFldType(RES_SETEXPFLD, rData.sPar1);

            if(pSetTyp)
            {
                SwGetExpFieldType* pTyp = (SwGetExpFieldType*)pCurShell->GetFldType(
                                                0, RES_GETEXPFLD);
                pFld = new SwGetExpField(pTyp, rData.sPar1, pSetTyp->GetType(), nFormatId);
                pFld->SetSubType(nSubType | pSetTyp->GetType());
                bExp = TRUE;
            }
            else
                return FALSE;
            break;
        }
        case TYP_FORMELFLD:
        {
            if(pCurShell->GetFrmType(0,FALSE) & FRMTYPE_TABLE)
            {
                pCurShell->StartAllAction();

                SvNumberFormatter* pFormatter = pCurShell->GetDoc()->GetNumberFormatter();
                const SvNumberformat* pEntry = pFormatter->GetEntry(nFormatId);

                if (pEntry)
                {
                    SfxStringItem aFormat(FN_NUMBER_FORMAT, pEntry->GetFormatstring());
                    pCurShell->GetView().GetViewFrame()->GetDispatcher()->
                        Execute(FN_NUMBER_FORMAT, SFX_CALLMODE_SYNCHRON, &aFormat, 0L);
                }

                SfxItemSet aBoxSet( pCurShell->GetAttrPool(),
                                RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );

                String sFml( rData.sPar2 );
                if( sFml.EraseLeadingChars().Len() &&
                    '=' == sFml.GetChar( 0 ) )
                    sFml.Erase( 0, 1 );

                aBoxSet.Put( SwTblBoxFormula( sFml ));
                pCurShell->SetTblBoxFormulaAttrs( aBoxSet );
                pCurShell->UpdateTable();

                pCurShell->EndAllAction();
                return TRUE;

/*              // In der Tabelle Tabellenformeln einfuegen
                SwTblFieldType* pTyp = (SwTblFieldType*)pCurShell->GetFldType(
                                                        0, RES_TABLEFLD);
                pFld = new SwTblField(pTyp, rData.sPar2, GSE_EXPR, nFormatId);
                bTbl = TRUE;*/
            }
            else
            {
                SwGetExpFieldType* pTyp = (SwGetExpFieldType*)
                                            pCurShell->GetFldType(0, RES_GETEXPFLD);
                pFld = new SwGetExpField(pTyp, rData.sPar2, GSE_FORMULA, nFormatId);
                pFld->SetSubType(nSubType);
                bExp = TRUE;
            }
            break;
        }
        case TYP_SETREFPAGEFLD:
            pFld = new SwRefPageSetField( (SwRefPageSetFieldType*)
                                pCurShell->GetFldType( 0, RES_REFPAGESETFLD ),
                                (short)rData.sPar2.ToInt32(), 0 != nSubType  );
            bPageVar = TRUE;
            break;

        case TYP_GETREFPAGEFLD:
            pFld = new SwRefPageGetField( (SwRefPageGetFieldType*)
                            pCurShell->GetFldType( 0, RES_REFPAGEGETFLD ), nFormatId );
            bPageVar = TRUE;
            break;
        case TYP_DROPDOWN :
        {
            pFld = new SwDropDownField(pCurShell->GetFldType( 0, RES_DROPDOWN ));
            xub_StrLen nTokenCount = rData.sPar2.Len() ? rData.sPar2.GetTokenCount(DB_DELIM) : 0;
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(xub_StrLen nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = rData.sPar2.GetToken(nToken, DB_DELIM);
            ((SwDropDownField*)pFld)->SetItems(aEntries);
            ((SwDropDownField*)pFld)->SetName(rData.sPar1);
        }
        break;
        default:
        {   ASSERT(!this, "Falscher Feldtyp");
            return FALSE;
        }
    }
    ASSERT(pFld, "Feld nicht vorhanden");


     //the auto language flag has to be set prior to the language!
     pFld->SetAutomaticLanguage(rData.bIsAutomaticLanguage);
     USHORT nLang = GetCurrLanguage();
     pFld->SetLanguage(nLang);

    // Einfuegen
    pCurShell->StartAllAction();

    pCurShell->Insert(*pFld);

    if(bExp && bEvalExp)
        pCurShell->UpdateExpFlds(TRUE);

    if(bTbl)
    {
        pCurShell->Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
        pCurShell->UpdateFlds(*pFld);
        pCurShell->Right(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
    }
    else if( bPageVar )
        ((SwRefPageGetFieldType*)pCurShell->GetFldType( 0, RES_REFPAGEGETFLD ))->UpdateFlds();
    else if( TYP_GETREFFLD == rData.nTypeId )
        pFld->GetTyp()->Modify( 0, 0 );

    // temporaeres Feld loeschen
    delete pFld;

    pCurShell->EndAllAction();
    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung: Felder Update
 --------------------------------------------------------------------*/


void SwFldMgr::UpdateCurFld(ULONG nFormat,
                            const String& rPar1,
                            const String& rPar2)
{
    // Format aendern
    ASSERT(pCurFld, "kein Feld an der CursorPos");

    SwFieldType* pType   = pCurFld->GetTyp();
    const USHORT nTypeId = pCurFld->GetTypeId();

    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(!pSh)
        return;
    pSh->StartAllAction();

    BOOL bSetPar2 = TRUE;
    BOOL bSetPar1 = TRUE;
    String sPar1( rPar1 );
    String sPar2( rPar2 );

    // Order to Format
    switch( nTypeId )
    {
        case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
            //              Namen haben! Wird hier noch nicht beachtet.
            USHORT nTmpPos = sPar2.SearchAndReplace( ' ', so3::cTokenSeperator );
            sPar2.SearchAndReplace( ' ', so3::cTokenSeperator, nTmpPos );
            break;
        }

        case TYP_CHAPTERFLD:
        {
            USHORT nByte = (USHORT)rPar2.ToInt32();
            nByte = Max(USHORT(1), nByte);
            nByte = Min(nByte, USHORT(MAXLEVEL));
            nByte -= 1;
            ((SwChapterField*)pCurFld)->SetLevel((BYTE)nByte);
            bSetPar2 = FALSE;
            break;
        }

        case TYP_SCRIPTFLD:
            ((SwScriptField*)pCurFld)->SetCodeURL((BOOL)nFormat);
            break;

        case TYP_NEXTPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                ((SwPageNumberField*)pCurFld)->SetUserString( sPar2 );
                sPar2 = 1;
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = (short)sPar2.ToInt32();
                nOff += 1;
                sPar2 = nOff;
            }
            break;

        case TYP_PREVPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                ((SwPageNumberField*)pCurFld)->SetUserString( sPar2 );
                sPar2 = -1;
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = (short)sPar2.ToInt32();
                nOff -= 1;
                sPar2 = nOff;
            }
            break;

        case TYP_PAGENUMBERFLD:
        case TYP_GETREFPAGEFLD:
            if( nFormat + 2 == SVX_NUM_PAGEDESC )
                nFormat = SVX_NUM_PAGEDESC;
            break;

        case TYP_GETREFFLD:
            {
                bSetPar2 = FALSE;
                ((SwGetRefField*)pCurFld)->SetSubType( (USHORT)rPar2.ToInt32() );
                USHORT nPos = rPar2.Search( '|' );
                if( STRING_NOTFOUND != nPos )
                    ((SwGetRefField*)pCurFld)->SetSeqNo( (USHORT)rPar2.Copy( nPos + 1 ).ToInt32());
            }
            break;
        case TYP_DROPDOWN:
        {
            xub_StrLen nTokenCount = sPar2.Len() ? sPar2.GetTokenCount(DB_DELIM) : 0;
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(xub_StrLen nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = sPar2.GetToken(nToken, DB_DELIM);
            ((SwDropDownField*)pCurFld)->SetItems(aEntries);
            ((SwDropDownField*)pCurFld)->SetName(sPar1);
            bSetPar1 = bSetPar2 = FALSE;
        }
        break;
    }

    // Format setzen
    // Format wegen NumberFormatter vor SetPar2 einstellen!
    pCurFld->ChangeFormat(nFormat);

    if(bSetPar1)
        pCurFld->SetPar1( sPar1 );
    if( bSetPar2 )
        pCurFld->SetPar2( sPar2 );

    // Update anschmeissen
    if(nTypeId == TYP_DDEFLD ||
       nTypeId == TYP_USERFLD ||
       nTypeId == TYP_USRINPFLD)
    {
        pType->UpdateFlds();
        pSh->SetModified();
    }
    else
        pSh->SwEditShell::UpdateFlds(*pCurFld);

    pSh->EndAllAction();
}

/*------------------------------------------------------------------------
 Beschreibung:  Setzen / Erfragen Werte von Benutzerfeldern aus BASIC
------------------------------------------------------------------------*/


BOOL SwFldMgr::SetFieldValue(const String &rFieldName,
                             const String &rValue)
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    if(!pSh)
        return FALSE;
    SwUserFieldType* pType = (SwUserFieldType*)pSh->InsertFldType(
                                    SwUserFieldType( pSh->GetDoc(), rFieldName ));

    if(pType)
        pType->SetContent(rValue);
    return 0 != pType;
}

/*--------------------------------------------------------------------
    Beschreibung: ExpressionFields explizit evaluieren
 --------------------------------------------------------------------*/
void SwFldMgr::EvalExpFlds(SwWrtShell* pSh)
{
    if (pSh == NULL)
        pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();

    if(pSh)
    {
        pSh->StartAllAction();
        pSh->UpdateExpFlds(TRUE);
        pSh->EndAllAction();
    }
}
USHORT SwFldMgr::GetCurrLanguage() const
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if( pSh )
        return pSh->GetCurLang();
    return SvxLocaleToLanguage( GetAppLocaleData().getLocale() );
}

void SwFieldType::_GetFldName()
{
    static const USHORT coFldCnt = STR_TYPE_END - STR_TYPE_BEGIN;

    static USHORT __READONLY_DATA coFldNms[ coFldCnt ] = {
        FLD_DATE_STD,
        FLD_TIME_STD,
        STR_FILENAMEFLD,
        STR_DBNAMEFLD,
        STR_CHAPTERFLD,
        STR_PAGENUMBERFLD,
        STR_DOCSTATFLD,
        STR_AUTHORFLD,
        STR_SETFLD,
        STR_GETFLD,
        STR_FORMELFLD,
        STR_HIDDENTXTFLD,
        STR_SETREFFLD,
        STR_GETREFFLD,
        STR_DDEFLD,
        STR_MACROFLD,
        STR_INPUTFLD,
        STR_HIDDENPARAFLD,
        STR_DOCINFOFLD,
        STR_DBFLD,
        STR_USERFLD,
        STR_POSTITFLD,
        STR_TEMPLNAMEFLD,
        STR_SEQFLD,
        STR_DBNEXTSETFLD,
        STR_DBNUMSETFLD,
        STR_DBSETNUMBERFLD,
        STR_CONDTXTFLD,
        STR_NEXTPAGEFLD,
        STR_PREVPAGEFLD,
        STR_EXTUSERFLD,
        FLD_DATE_FIX,
        FLD_TIME_FIX,
        STR_SETINPUTFLD,
        STR_USRINPUTFLD,
        STR_SETREFPAGEFLD,
        STR_GETREFPAGEFLD,
        STR_INTERNETFLD,
        STR_JUMPEDITFLD,
        STR_SCRIPTFLD,
        STR_AUTHORITY,
        STR_COMBINED_CHARS,
        STR_DROPDOWN
    };

    // Infos fuer Felder einfuegen
    SwFieldType::pFldNames = new SvStringsDtor( (BYTE)coFldCnt, 2 );
    for( USHORT nIdx = 0; nIdx < coFldCnt; ++nIdx )
    {
        String* pTmp = new SW_RESSTR( coFldNms[ nIdx ] );
        pTmp->EraseAllChars('~');
        SwFieldType::pFldNames->Insert(pTmp, nIdx );
    }
}


BOOL SwFldMgr::SetUserSubType(const String& rName, USHORT nType)
{
    BOOL bRet = FALSE;
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    DBG_ASSERT(pSh, "no SwWrtShell found")
    SwUserFieldType *pType =
        (SwUserFieldType *) (pSh ?
            pSh->GetFldType(RES_USERFLD, rName) : 0);

    if(pType)
    {
        pType->SetType(nType);
        bRet = TRUE;
    }
    return bRet;
}

BOOL SwFldMgr::InsertURL(  const String& rName,
                                const String& rVal,
                                const String& rFrame,
                                const SvxMacroItem* pItem)
{
    sCurFrame = rFrame;
    pMacroItem = pItem;
    SwInsertFld_Data aData(TYP_INTERNETFLD, 0, rName, rVal, 0);
    BOOL bRet = InsertFld(aData);
    sCurFrame = aEmptyStr;
    pMacroItem = 0;
    return bRet;

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwFldMgr::ChooseMacro(const String &rSelMacro)
{
    BOOL bRet = FALSE;
    String sSelMacro(rSelMacro);
    SfxApplication* pSfxApp = SFX_APP();

    if (sSelMacro.GetToken(0) == pSfxApp->GetName())
    {   // Malte arbeitet (faelschlicherweise) in ChooseMacro mit
        // GetTitle statt GetName, daher diese Konvertierung:
        sSelMacro.SetToken( 0, ';', Application::GetAppName());
    }
    else
    {
        SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
        if (pSh)
        {
            SwDocShell* pDocSh = pSh->GetView().GetDocShell();

            if (sSelMacro.GetToken(0) == pDocSh->GetName())
            {   // Malte arbeitet (faelschlicherweise) in ChooseMacro mit
                // GetTitle statt GetName, daher diese Konvertierung:
                sSelMacro.SetToken( 0, ';', pDocSh->GetTitle( SFX_TITLE_FILENAME ));
            }
        }
    }

    // choose macro dialog
    String aScriptURL = OfficeApplication::ChooseMacro(FALSE, TRUE, sSelMacro);

    // aScriptURL has the following format:
    // vnd.sun.star.script:language=[language],macro=[macro],location=[location]
    // [language] = StarBasic
    // [macro] = libname.modulename.macroname
    // [location] = application|document
    // e.g. 'vnd.sun.star.script:language=StarBasic,macro=Standard.Module1.Main,location=document'
    //
    // but for the UI we need this format:
    // 'macroname.modulename.libname.[appname|docname]'

    if ( aScriptURL.Len() != 0 )
    {
        // parse script URL
        BOOL bFound;
        String aValue;
        INetURLObject aINetScriptURL( aScriptURL );

        // get language
        String aLanguage;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("language") ), &aValue );
        if ( bFound )
            aLanguage = aValue;

        // get macro
        String aMacro;
        String aLibName;
        String aModuleName;
        String aMacroName;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("macro") ), &aValue );
        if ( bFound )
        {
            aMacro = aValue;
            aLibName    = aMacro.GetToken(0, sal_Unicode('.'));
            aModuleName = aMacro.GetToken(1, sal_Unicode('.'));
            aMacroName  = aMacro.GetToken(2, sal_Unicode('.'));
        }

        // get location
        String aLocation;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("location") ), &aValue );
        if ( bFound )
            aLocation = aValue;

        pSfxApp->EnterBasicCall();

        // find doc shell and basic manager
        SfxObjectShell* pShell = 0;
        BasicManager* pBasMgr = 0;
        if ( aLocation.EqualsIgnoreCaseAscii( "application" ) )
        {
            // application basic
            pBasMgr = pSfxApp->GetBasicManager();
        }
        else if ( aLocation.EqualsIgnoreCaseAscii( "document" ) )
        {
            // document basic
            SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
            if (pSh)
            {
                pShell = (SfxObjectShell*)pSh->GetView().GetDocShell();
                pBasMgr = ( pShell ? pShell->GetBasicManager() : 0 );
            }
        }
        ASSERT(pBasMgr, "SwFldMgr::ChooseMacro: No BasicManager found!");

        if ( pBasMgr)
        {
            StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
            ASSERT(pBasic, "SwFldMgr::ChooseMacro: No Basic found!");

            if ( pBasic )
            {
                SbModule* pModule = pBasic->FindModule( aModuleName );

                if ( pModule )
                {
                    SetMacroModule( pModule );

                    // construct macro path
                    String aMacroPath( aMacroName );
                    aMacroPath += '.';
                    aMacroPath += aModuleName;
                    aMacroPath += '.';
                    aMacroPath += aLibName;
                    aMacroPath += '.';

                    if ( pShell )
                        aMacroPath += pShell->GetName();
                    else
                        aMacroPath += pSfxApp->GetName();

                    SetMacroPath( aMacroPath );
                    bRet = TRUE;
                }
            }
        }
        pSfxApp->LeaveBasicCall();
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ULONG SwFldMgr::GetDefaultFormat(USHORT nTypeId, BOOL bIsText, SvNumberFormatter* pFormatter, double* pVal)
{
    double fValue;
    short  nDefFormat;

    switch (nTypeId)
    {
        case TYP_TIMEFLD:
        case TYP_DATEFLD:
        {
            Date aDate;
            Date* pNullDate = pFormatter->GetNullDate();

            fValue = aDate - *pNullDate;

            Time aTime;

            ULONG nNumFmtTime = (ULONG)aTime.GetSec() + (ULONG)aTime.GetMin() * 60L +
                          (ULONG)aTime.GetHour() * 3600L;

            fValue += (double)nNumFmtTime / 86400.0;

            nDefFormat = (nTypeId == TYP_DATEFLD) ? NUMBERFORMAT_DATE : NUMBERFORMAT_TIME;
        }
        break;

        default:
            if (bIsText)
            {
                fValue = 0.0;
                nDefFormat = NUMBERFORMAT_TEXT;
            }
            else
            {
                fValue = 0.0;
                nDefFormat = NUMBERFORMAT_ALL;
            }
            break;
    }

    if (pVal)
        *pVal = fValue;

    return pFormatter->GetStandardFormat(nDefFormat, GetCurrLanguage());
}
/* -----------------------------23.06.00 17:32--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XNameAccess> SwFldMgr::GetDBContext()
{
    if(!xDBContext.is())
    {
        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if( xMgr.is() )
        {
            Reference<XInterface> xInstance = xMgr->createInstance(
                    rtl::OUString::createFromAscii(
                                    "com.sun.star.sdb.DatabaseContext" ));
            xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
        }
        DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
    }
    return xDBContext;
}
/* -----------------------------01.03.01 16:46--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XNumberingTypeInfo> SwFldMgr::GetNumberingInfo() const
{
    if(!xNumberingInfo.is())
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        Reference < XInterface > xI = xMSF->createInstance(
            ::rtl::OUString::createFromAscii(
                            "com.sun.star.text.DefaultNumberingProvider" ));
        Reference<XDefaultNumberingProvider> xDefNum(xI, UNO_QUERY);
        DBG_ASSERT(xDefNum.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"")
        ((SwFldMgr*)this)->xNumberingInfo = Reference<XNumberingTypeInfo>(xDefNum, UNO_QUERY);
    }
    return xNumberingInfo;
}
