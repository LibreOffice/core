/*************************************************************************
 *
 *  $RCSfile: fldmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 11:24:22 $
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

#include "cmdid.h"
#include "view.hxx"
#include "wrtsh.hxx"        // Actives Fenster
#include "doc.hxx"      // Actives Fenster
#include "docsh.hxx"        // Actives Fenster
#include "swmodule.hxx"
#include "hintids.hxx"
#include "charatr.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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
#ifndef _BASOBJ_HXX //autogen
#include <basctl/basobj.hxx>
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
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#include "dbmgr.hxx"
#include "shellres.hxx"
#include "fldbas.hxx"
#include "docufld.hxx"
#include "chpfld.hxx"
#include "ddefld.hxx"
#include "expfld.hxx"
#include "reffld.hxx"
#include "usrfld.hxx"
#include "dbfld.hxx"
#include "authfld.hxx"
#include "flddat.hxx"
#include "fldmgr.hxx"

#include "fldui.hrc"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

/*--------------------------------------------------------------------
    Beschreibung: Gruppen der Felder
 --------------------------------------------------------------------*/
enum
{
    GRP_DOC_BEGIN   =  0,
    GRP_DOC_END     =  GRP_DOC_BEGIN + 11,

    GRP_FKT_BEGIN   =  GRP_DOC_END,
    GRP_FKT_END     =  GRP_FKT_BEGIN + 6,

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
    TYP_INPUTFLD,       FLD_INPUT_BEGIN,    FLD_INPUT_END,  0,                  0,
    TYP_MACROFLD,       0,                  0,              0,                  0,
    TYP_JUMPEDITFLD,    0,                  0,              FMT_MARK_BEGIN,     FMT_MARK_END,
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
    aSubLst.DeleteAndDestroy(0, aSubLst.Count());
}

/*--------------------------------------------------------------------
    Beschreibung: RefMark ueber Namen organisieren
 --------------------------------------------------------------------*/

BOOL  SwFldMgr::CanInsertRefMark( const String& rStr )
{
    SwWrtShell &rSh = pWrtShell ? *pWrtShell : *lcl_GetShell();
    USHORT nCnt = rSh.GetCrsrCnt();

    // der letzte Crsr muss keine aufgespannte Selektion
    if( 1 < nCnt && !rSh.SwCrsrShell::HasSelection() )
        --nCnt;

    return 2 > nCnt && 0 == rSh.GetRefMark( rStr );
}

/*--------------------------------------------------------------------
    Beschreibung: Alle DBTypes loeschen
 --------------------------------------------------------------------*/

void SwFldMgr::RemoveDBTypes()
{
    SwWrtShell &rSh = pWrtShell ? *pWrtShell : *lcl_GetShell();
    USHORT nCount = rSh.GetFldTypeCount(RES_DBFLD);
    for ( USHORT i=0; i < nCount ; ++i )
    {
        SwFieldType* pType = rSh.GetFldType( i, RES_DBFLD );
        if( !pType->GetDepends() )
        {
            rSh.RemoveFldType( i--, RES_DBFLD );
            nCount--;
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Zugriff ueber ResIds
 --------------------------------------------------------------------*/

USHORT SwFldMgr::GetFldTypeCount(USHORT nResId) const
{
    return pWrtShell ?
            pWrtShell->GetFldTypeCount(nResId) :
                ::lcl_GetShell()->GetFldTypeCount(nResId);
}


SwFieldType* SwFldMgr::GetFldType(USHORT nResId, USHORT nId) const
{
    return pWrtShell ?
            pWrtShell->GetFldType(nId, nResId) :
                ::lcl_GetShell()->GetFldType(nId, nResId);
}


SwFieldType* SwFldMgr::GetFldType(USHORT nResId, const String& rName) const
{
    return pWrtShell ?
            pWrtShell->GetFldType(nResId, rName) :
                ::lcl_GetShell()->GetFldType(nResId, rName);
}


void SwFldMgr::RemoveFldType(USHORT nResId, USHORT nId)
{
    pWrtShell ?
            pWrtShell->RemoveFldType(nId, nResId) :
                ::lcl_GetShell()->RemoveFldType(nId, nResId);
}


void SwFldMgr::RemoveFldType(USHORT nResId, const String& rName )
{
    pWrtShell ?
            pWrtShell->RemoveFldType(nResId, rName) :
                ::lcl_GetShell()->RemoveFldType(nResId, rName);
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


USHORT SwFldMgr::GetTypeId(USHORT nPos) const
{
    ASSERT(nPos < ::GetPackCount(), "unzulaessige Pos");
    return aSwFlds[ nPos ].nTypeId;
}


const String& SwFldMgr::GetTypeStr(USHORT nPos) const
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


USHORT SwFldMgr::GetPos(USHORT nTypeId) const
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

SvStringsDtor& SwFldMgr::GetSubTypes(USHORT nTypeId)
{
    // aSubLst.DeleteAndDestroy(0, aSubLst.Count());
    for (USHORT i=0; i < aSubLst.Count();  )
    {
        String* pStr = aSubLst[i];
        DELETEZ(pStr);
        aSubLst.Remove(i);
    }

    const USHORT nPos = GetPos(nTypeId);

    switch(nTypeId)
    {
        case TYP_SETREFFLD:
        case TYP_GETREFFLD:
        {
            // Referenzen sind keine Felder
            pWrtShell ?
                pWrtShell->GetRefMarks( &aSubLst ) :
                    ::lcl_GetShell()->GetRefMarks( &aSubLst );
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
                        aSubLst.Insert(pNew, aSubLst.Count());
                    }
                }
                else
                {
                    String* pNew = new String(sMacroPath);
                    aSubLst.Insert(pNew, aSubLst.Count());
                }
            }
            break;
        }
        case TYP_INPUTFLD:
        {   String* pNew = new SW_RESSTR(aSwFlds[nPos].nSubTypeStart);
            aSubLst.Insert(pNew, aSubLst.Count());
            // Weiter bei generischen Typen
        }
        case TYP_DDEFLD:
        case TYP_SEQFLD:
        case TYP_FORMELFLD:
        case TYP_GETFLD:
        case TYP_SETFLD:
        case TYP_USERFLD:
        {
            SwWrtShell &rSh = pWrtShell ? *pWrtShell : *::lcl_GetShell();
            const USHORT nCount = rSh.GetFldTypeCount();
            for(USHORT i = 0; i < nCount; ++i)
            {
                SwFieldType* pFldType = rSh.GetFldType( i );
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
                    aSubLst.Insert(pNew, aSubLst.Count());
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

                SwWrtShell &rSh = pWrtShell ? *pWrtShell : *::lcl_GetShell();
                for(USHORT i = 0; i < nCount; ++i)
                {
                    String* pNew;
                    if (nTypeId == TYP_DOCINFOFLD)
                    {
                        if (i >= DI_INFO1 && i <= DI_INFO4)
                            pNew = new String( rSh.GetInfo()->GetUserKey(i-DI_INFO1).GetTitle());
                        else
                            pNew = new String(*ViewShell::GetShellRes()->aDocInfoLst[i]);
                    }
                    else
                        pNew = new SW_RESSTR(aSwFlds[nPos].nSubTypeStart + i);

                    aSubLst.Insert(pNew, aSubLst.Count());
                }
            }
        }
    }
    return aSubLst;
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
    SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    ASSERT(nTypeId < TYP_END, "unzulaessige TypeId");

    const USHORT nPos = GetPos(nTypeId);

    if(nPos == USHRT_MAX)
        return aRet;

    ULONG nStart;

    nStart = aSwFlds[nPos].nFmtBegin;

    if (nTypeId == TYP_FILENAMEFLD)
        nFormatId &= ~FF_FIXED;     // Fixed-Flag ausmaskieren

    aRet = SW_RESSTR((USHORT)(nStart + nFormatId));

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
        switch( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
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
        break;

    case TYP_DDEFLD:
        switch ( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
        {
        case FMT_DDE_NORMAL:    nId = LINKUPDATE_ONCALL; break;
        case FMT_DDE_HOT:       nId = LINKUPDATE_ALWAYS; break;
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
    pWrtShell ?
            pWrtShell->InsertFldType(rType) :
                ::lcl_GetShell()->InsertFldType(rType);
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuelle TypeId ermitteln
 --------------------------------------------------------------------*/


USHORT SwFldMgr::GetCurTypeId() const
{
    return pCurFld->GetTypeId();
}

/*--------------------------------------------------------------------
    Beschreibung: Ueber String  Feld einfuegen oder Update
 --------------------------------------------------------------------*/


BOOL SwFldMgr::InsertFld(USHORT         nType,
                         USHORT         nSubType,
                         const  String& rPar1,
                         const  String& rPar2,
                         ULONG          nFormat,
                         SwWrtShell*    pSh,
                         sal_Unicode    cSeparator)
{
    SwField* pFld   = 0;
    BOOL bExp = FALSE;
    BOOL bTbl = FALSE;
    BOOL bPageVar = FALSE;

    if (pSh == NULL)
        pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();

    switch(nType)
    {   // ACHTUNG dieses Feld wird ueber einen gesonderten Dialog eingefuegt
        case TYP_POSTITFLD:
        {
            SwPostItFieldType* pType =
                (SwPostItFieldType*)pSh->GetFldType(0, RES_POSTITFLD);
            pFld = new SwPostItField(pType, rPar1, rPar2, Date());
            break;
        }
        case TYP_SCRIPTFLD:
        {
            SwScriptFieldType* pType =
                (SwScriptFieldType*)pSh->GetFldType(0, RES_SCRIPTFLD);
            pFld = new SwScriptField(pType, rPar1, rPar2, (BOOL)nFormat);
            break;
        }
        case TYP_AUTHORITY:
        {
            SwAuthorityFieldType* pType =
                (SwAuthorityFieldType*)pSh->GetFldType(0, RES_AUTHORITY);
            if(!pType)
            {
                pType =
                    (SwAuthorityFieldType*)pSh->InsertFldType(
                                    SwAuthorityFieldType(pSh->GetDoc()));
            }
            pFld = new SwAuthorityField(pType, rPar1);
        }
        break;
        case TYP_DATEFLD:
        case TYP_TIMEFLD:
        {
            USHORT nSub = (nType == TYP_DATEFLD) ? DATEFLD : TIMEFLD;
            nSub |= nSubType == DATE_VAR ? 0 : FIXEDFLD;

            SwDateTimeFieldType* pTyp =
                (SwDateTimeFieldType*)pSh->GetFldType(0, RES_DATETIMEFLD);
            pFld = new SwDateTimeField(pTyp, nSub, nFormat);
            pFld->SetPar2(rPar2);
            break;
        }
        case TYP_FILENAMEFLD:
        {
            SwFileNameFieldType* pTyp =
                (SwFileNameFieldType*)pSh->GetFldType(0, RES_FILENAMEFLD);
            pFld = new SwFileNameField(pTyp, nFormat);
            break;
        }
        case TYP_TEMPLNAMEFLD:
        {
            SwTemplNameFieldType* pTyp =
                (SwTemplNameFieldType*)pSh->GetFldType(0, RES_TEMPLNAMEFLD);
            pFld = new SwTemplNameField(pTyp, nFormat);
            break;
        }
        case TYP_CHAPTERFLD:
        {   USHORT nByte = rPar2.ToInt32();
            SwChapterFieldType* pTyp =
                (SwChapterFieldType*)pSh->GetFldType(0, RES_CHAPTERFLD);
            pFld = new SwChapterField(pTyp, nFormat);
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
            short nOff  = rPar2.ToInt32();

            if(nType == TYP_NEXTPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormat )
                    nOff = 1;
                else
                    nOff += 1;
                nSubType = PG_NEXT;
            }
            else if(nType == TYP_PREVPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormat )
                    nOff = -1;
                else
                    nOff -= 1;
                nSubType =  PG_PREV;
            }
            else
                nSubType = PG_RANDOM;

            SwPageNumberFieldType* pTyp =
                (SwPageNumberFieldType*)pSh->GetFldType(0, RES_PAGENUMBERFLD);
            pFld = new SwPageNumberField(pTyp, nSubType, nFormat, nOff);

            if( SVX_NUM_CHAR_SPECIAL == nFormat &&
                ( PG_PREV == nSubType || PG_NEXT == nSubType ) )
                ((SwPageNumberField*)pFld)->SetUserString( rPar2 );
            break;
        }
        case TYP_DOCSTATFLD:
        {   SwDocStatFieldType* pTyp =
                (SwDocStatFieldType*)pSh->GetFldType(0, RES_DOCSTATFLD);
            pFld = new SwDocStatField(pTyp, nSubType, nFormat);
            break;
        }
        case TYP_AUTHORFLD:
        {   SwAuthorFieldType* pTyp =
                (SwAuthorFieldType*)pSh->GetFldType(0, RES_AUTHORFLD);
            pFld = new SwAuthorField(pTyp, nFormat);
            break;
        }
        case TYP_CONDTXTFLD:
        case TYP_HIDDENTXTFLD:
        {
            SwHiddenTxtFieldType* pTyp =
                (SwHiddenTxtFieldType*)pSh->GetFldType(0, RES_HIDDENTXTFLD);
            pFld = new SwHiddenTxtField(pTyp, TRUE, rPar1, rPar2, FALSE, nType);
            bExp = TRUE;
            break;
        }
        case TYP_HIDDENPARAFLD:
        {
            SwHiddenParaFieldType* pTyp =
                (SwHiddenParaFieldType*)pSh->GetFldType(0, RES_HIDDENPARAFLD);
            pFld = new SwHiddenParaField(pTyp, rPar1);
            bExp = TRUE;
            break;
        }
        case TYP_SETREFFLD:
        {
            if( rPar1.Len() > 0 && CanInsertRefMark( rPar1 ) )
            {
                pSh->SetAttr( SwFmtRefMark( rPar1 ) );
                return TRUE;
            }
            return FALSE;
        }
        case TYP_GETREFFLD:
        {
            SwGetRefFieldType* pTyp =
                (SwGetRefFieldType*)pSh->GetFldType(0, RES_GETREFFLD);
            USHORT nSeqNo = rPar2.ToInt32();
            pFld = new SwGetRefField(pTyp, rPar1, nSubType, nSeqNo, nFormat);
            bExp = TRUE;
            break;
        }
        case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
            //              Namen haben! Wird hier noch nicht beachtet.
            String sCmd( rPar2 );
            USHORT nTmpPos = sCmd.SearchAndReplace( ' ', cTokenSeperator );
            sCmd.SearchAndReplace( ' ', cTokenSeperator, nTmpPos );

            SwDDEFieldType* pTyp = (SwDDEFieldType*)pSh->InsertFldType(
                    SwDDEFieldType( rPar1, sCmd, (USHORT)nFormat ));
            pFld = new SwDDEField( pTyp );
            break;
        }
        case TYP_MACROFLD:
        {
            SwMacroFieldType* pTyp =
                (SwMacroFieldType*)pSh->GetFldType(0, RES_MACROFLD);

            pFld = new SwMacroField(pTyp, rPar1, rPar2);
            break;
        }
        case TYP_INTERNETFLD:
        {
            SwFmtINetFmt aFmt( rPar1, sCurFrame );
            if( pMacroItem )
                aFmt.SetMacroTbl( &pMacroItem->GetMacroTable() );
            return pSh->InsertURL( aFmt, rPar2 );
        }
        case TYP_JUMPEDITFLD:
        {
            SwJumpEditFieldType* pTyp =
                (SwJumpEditFieldType*)pSh->GetFldType(0, RES_JUMPEDITFLD);

            pFld = new SwJumpEditField(pTyp, nFormat, rPar1, rPar2 );
            break;
        }
        case TYP_DOCINFOFLD:
        {
            SwDocInfoFieldType* pTyp = (SwDocInfoFieldType*)pSh->GetFldType(
                                            0, RES_DOCINFOFLD );
            pFld = new SwDocInfoField(pTyp, nSubType, nFormat);
            break;
        }
        case TYP_EXTUSERFLD:
        {
            SwExtUserFieldType* pTyp = (SwExtUserFieldType*)pSh->GetFldType(
                                            0, RES_EXTUSERFLD);
            pFld = new SwExtUserField(pTyp, nSubType, nFormat);
            break;
        }
        case TYP_DBFLD:
        {
            String sDBName;
            String sPar1;

            if (rPar1.Search(DB_DELIM) == STRING_NOTFOUND)
            {
                sDBName = pSh->GetDBName();
                sPar1 = rPar1;
            }
            else
            {
                sDBName = rPar1.GetToken(0, DB_DELIM);
                sDBName += DB_DELIM;
                sDBName += rPar1.GetToken(1, DB_DELIM);
                sPar1 = rPar1.GetToken(2, DB_DELIM);
            }

            if (sDBName.Len() && pSh->GetDBName() != sDBName)
                pSh->ChgDBName(sDBName);

            SwDBFieldType* pTyp = (SwDBFieldType*)pSh->InsertFldType(
                    SwDBFieldType(pSh->GetDoc(), sPar1, sDBName) );
            pFld = new SwDBField(pTyp);
            pFld->SetSubType(nSubType);

            if( !(nSubType & SUB_OWN_FMT) ) // Datenbankformat ermitteln
            {
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(1, DB_DELIM));
                nFormat = pSh->GetNewDBMgr()->GetColumnFmt( sSourceName, sTableName, sPar1,
                                        pSh->GetNumberFormatter(), GetCurrLanguage() );
            }
            pFld->ChangeFormat( nFormat );

            bExp = TRUE;
            break;
        }
        case TYP_DBSETNUMBERFLD:
        case TYP_DBNUMSETFLD:
        case TYP_DBNEXTSETFLD:
        case TYP_DBNAMEFLD:
        {
            USHORT nPos, nTablePos, nExpPos;
            String sDBName, sPar1;

            // DBName aus rPar1 extrahieren. Format: DBName.TableName.ExpStrg
            if ((nTablePos = rPar1.Search(DB_DELIM)) != STRING_NOTFOUND)
                sDBName = rPar1.Copy(0, nTablePos++);
            if ((nExpPos = rPar1.Search(DB_DELIM, nTablePos)) != STRING_NOTFOUND)
            {
                sDBName += DB_DELIM;
                sDBName += rPar1.Copy(nTablePos, nExpPos++ - nTablePos);
            }
            if (nExpPos != STRING_NOTFOUND)
                nPos = nExpPos;
            else if (nTablePos != STRING_NOTFOUND)
                nPos = nTablePos;
            else
                nPos = 0;
            sPar1 = rPar1.Copy(nPos);

            if (sDBName.Len() && pSh->GetDBName() != sDBName)
                pSh->ChgDBName(sDBName);

            switch(nType)
            {
                case TYP_DBNAMEFLD:
                {
                    SwDBNameFieldType* pTyp =
                        (SwDBNameFieldType*)pSh->GetFldType(0, RES_DBNAMEFLD);
                    pFld = new SwDBNameField(pTyp, sDBName);

                    break;
                }
                case TYP_DBNEXTSETFLD:
                {
                    SwDBNextSetFieldType* pTyp = (SwDBNextSetFieldType*)pSh->GetFldType(
                                                    0, RES_DBNEXTSETFLD);
                    pFld = new SwDBNextSetField(pTyp, sPar1, rPar2, sDBName);
                    bExp = TRUE;
                    break;
                }
                case TYP_DBNUMSETFLD:
                {
                    SwDBNumSetFieldType* pTyp = (SwDBNumSetFieldType*)pSh->GetFldType(
                                                    0, RES_DBNUMSETFLD);
                    pFld = new SwDBNumSetField( pTyp, sPar1, rPar2, sDBName);
                    bExp = TRUE;
                    break;
                }
                case TYP_DBSETNUMBERFLD:
                {
                    SwDBSetNumberFieldType* pTyp = (SwDBSetNumberFieldType*)
                                                pSh->GetFldType(0, RES_DBSETNUMBERFLD);
                    pFld = new SwDBSetNumberField( pTyp, sDBName, nFormat);
                    bExp = TRUE;
                    break;
                }
            }
            break;
        }
        case TYP_USERFLD:
        {
            SwUserFieldType* pTyp =
                (SwUserFieldType*)pSh->GetFldType(RES_USERFLD, rPar1);

            // nur wenn vorhanden
            if(!pTyp)
            {
                pTyp = (SwUserFieldType*)pSh->InsertFldType(
                        SwUserFieldType(pSh->GetDoc(), rPar1));
            }
            if (pTyp->GetContent(nFormat) != rPar2)
                pTyp->SetContent(rPar2, nFormat);
            pFld = new SwUserField(pTyp, 0, nFormat);
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
                                    pSh->GetFldType(RES_SETEXPFLD, rPar1);

                // kein Experssion Type mit dem Namen vorhanden -> anlegen
                if(pTyp)
                {
                    SwSetExpField* pExpFld =
                        new SwSetExpField(pTyp, aEmptyStr, nFormat);

                    // Typ vom SwSetExpFieldType nicht veraendern:
                    USHORT nOldSubType = pExpFld->GetSubType();
                    pExpFld->SetSubType(nOldSubType | (nSubType & 0xff00));

                    pExpFld->SetPromptText(rPar2);
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
                    (SwInputFieldType*)pSh->GetFldType(0, RES_INPUTFLD);

                SwInputField* pInpFld =
                    new SwInputField(pTyp, rPar1, rPar2, nSubType|SUB_INVISIBLE, nFormat);
                pFld = pInpFld;
            }

            // Dialog starten
            //
            pSh->StartInputFldDlg(pFld, FALSE);
            break;
        }
        case TYP_SETFLD:
        {
            if (!rPar2.Len())   // Leere Variablen sind nicht erlaubt
                return FALSE;

            SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pSh->InsertFldType(
                    SwSetExpFieldType(pSh->GetDoc(), rPar1) );

            SwSetExpField* pExpFld = new SwSetExpField( pTyp, rPar2, nFormat);
            pExpFld->SetSubType(nSubType);
            pExpFld->SetPar2(rPar2);
            bExp = TRUE;
            pFld = pExpFld;
            break;
        }
        case TYP_SEQFLD:
        {
            SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pSh->InsertFldType(
                    SwSetExpFieldType(pSh->GetDoc(), rPar1, GSE_SEQ));

            BYTE nLevel = nSubType & 0xff;

            pTyp->SetOutlineLvl(nLevel);
            if (nLevel != 0x7f && cSeparator == 0)
                cSeparator = '.';

            pTyp->SetDelimiter(cSeparator);
            SwSetExpField* pExpFld = new SwSetExpField(pTyp, rPar2, nFormat);
            bExp = TRUE;
            pFld = pExpFld;
            nSubType = GSE_SEQ;
            break;
        }
        case TYP_GETFLD:
        {
            // gibt es ein entprechendes SetField
            SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                    pSh->GetFldType(RES_SETEXPFLD, rPar1);

            if(pSetTyp)
            {
                SwGetExpFieldType* pTyp = (SwGetExpFieldType*)pSh->GetFldType(
                                                0, RES_GETEXPFLD);
                pFld = new SwGetExpField(pTyp, rPar1, pSetTyp->GetType(), nFormat);
                pFld->SetSubType(nSubType | pSetTyp->GetType());
                bExp = TRUE;
            }
            else
                return FALSE;
            break;
        }
        case TYP_FORMELFLD:
        {
            if(pSh->GetFrmType(0,FALSE) & FRMTYPE_TABLE)
            {
                pSh->StartAllAction();

                SvNumberFormatter* pFormatter = pSh->GetDoc()->GetNumberFormatter();
                const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);

                if (pEntry)
                {
                    SfxStringItem aFormat(FN_NUMBER_FORMAT, pEntry->GetFormatstring());
                    pSh->GetView().GetViewFrame()->GetDispatcher()->
                        Execute(FN_NUMBER_FORMAT, SFX_CALLMODE_SYNCHRON, &aFormat, 0L);
                }

                SfxItemSet aBoxSet( pSh->GetAttrPool(),
                                RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );

                String sFml( rPar2 );
                if( sFml.EraseLeadingChars().Len() &&
                    '=' == sFml.GetChar( 0 ) )
                    sFml.Erase( 0, 1 );

                aBoxSet.Put( SwTblBoxFormula( sFml ));
                pSh->SetTblBoxFormulaAttrs( aBoxSet );
                pSh->UpdateTable();

                pSh->EndAllAction();
                return TRUE;

/*              // In der Tabelle Tabellenformeln einfuegen
                SwTblFieldType* pTyp = (SwTblFieldType*)pSh->GetFldType(
                                                        0, RES_TABLEFLD);
                pFld = new SwTblField(pTyp, rPar2, GSE_EXPR, nFormat);
                bTbl = TRUE;*/
            }
            else
            {
                SwGetExpFieldType* pTyp = (SwGetExpFieldType*)
                                            pSh->GetFldType(0, RES_GETEXPFLD);
                pFld = new SwGetExpField(pTyp, rPar2, GSE_FORMULA, nFormat);
                pFld->SetSubType(nSubType);
                bExp = TRUE;
            }
            break;
        }
        case TYP_SETREFPAGEFLD:
            pFld = new SwRefPageSetField( (SwRefPageSetFieldType*)
                                pSh->GetFldType( 0, RES_REFPAGESETFLD ),
                                rPar2.ToInt32(), 0 != nSubType  );
            bPageVar = TRUE;
            break;

        case TYP_GETREFPAGEFLD:
            pFld = new SwRefPageGetField( (SwRefPageGetFieldType*)
                            pSh->GetFldType( 0, RES_REFPAGEGETFLD ), nFormat );
            bPageVar = TRUE;
            break;

        default:
        {   ASSERT(!this, "Falscher Feldtyp");
            return FALSE;
        }
    }
    ASSERT(pFld, "Feld nicht vorhanden");

    USHORT nLang = GetCurrLanguage();

    // Language
    pFld->SetLanguage(nLang);

    // Einfuegen
    pSh->StartAllAction();

    pSh->Insert(*pFld);

    if(bExp && bEvalExp)
        pSh->UpdateExpFlds(TRUE);

    if(bTbl)
    {
        pSh->Left();
        pSh->UpdateFlds(*pFld);
        pSh->Right();
    }
    else if( bPageVar )
        ((SwRefPageGetFieldType*)pSh->GetFldType( 0, RES_REFPAGEGETFLD ))->UpdateFlds();
    else if( TYP_GETREFFLD == nType )
        pFld->GetTyp()->Modify( 0, 0 );

    // temporaeres Feld loeschen
    delete pFld;

    pSh->EndAllAction();
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

    SwWrtShell &rSh = pWrtShell ? *pWrtShell : *::lcl_GetShell();
    rSh.StartAllAction();

    BOOL bSetPar2 = TRUE;
    String sPar1( rPar1 );
    String sPar2( rPar2 );

    // Order to Format
    switch( nTypeId )
    {
        case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
            //              Namen haben! Wird hier noch nicht beachtet.
            USHORT nTmpPos = sPar2.SearchAndReplace( ' ', cTokenSeperator );
            sPar2.SearchAndReplace( ' ', cTokenSeperator, nTmpPos );
            break;
        }

        case TYP_CHAPTERFLD:
        {
            USHORT nByte = rPar2.ToInt32();
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
                short nOff = sPar2.ToInt32();
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
                short nOff = sPar2.ToInt32();
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
                ((SwGetRefField*)pCurFld)->SetSubType( rPar2.ToInt32() );
                USHORT nPos = rPar2.Search( '|' );
                if( STRING_NOTFOUND != nPos )
                    ((SwGetRefField*)pCurFld)->SetSeqNo( rPar2.Copy( nPos + 1 ).ToInt32());
            }
            break;
    }

    // Format setzen
    // Format wegen NumberFormatter vor SetPar2 einstellen!
    pCurFld->ChangeFormat(nFormat);

    pCurFld->SetPar1( sPar1 );
    if( bSetPar2 )
        pCurFld->SetPar2( sPar2 );

    // Update anschmeissen
    if(nTypeId == TYP_DDEFLD ||
       nTypeId == TYP_USERFLD ||
       nTypeId == TYP_USRINPFLD)
    {
        pType->UpdateFlds();
        rSh.SetModified();
    }
    else
        rSh.SwEditShell::UpdateFlds(*pCurFld);

    rSh.EndAllAction();
}

/*------------------------------------------------------------------------
 Beschreibung:  Setzen / Erfragen Werte von Benutzerfeldern aus BASIC
------------------------------------------------------------------------*/


BOOL SwFldMgr::SetFieldValue(const String &rFieldName,
                             const String &rValue)
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    SwUserFieldType* pType = (SwUserFieldType*)pWrtShell->InsertFldType(
                                    SwUserFieldType( pSh->GetDoc(), rFieldName ));

    if(pType)
        pType->SetContent(rValue);
    return 0 != pType;
}

/*------------------------------------------------------------------------
 Beschreibung: Wert Datenbankfeld erfragen
------------------------------------------------------------------------*/


String SwFldMgr::GetDataBaseFieldValue(const String &rDBName, const String &rFieldName, SwWrtShell* pSh)
{
    String sFieldName(rFieldName);

    if (rDBName.Len())
        sFieldName = rDBName;
    else
        sFieldName = pSh->GetDBName();
    sFieldName += DB_DELIM;
    sFieldName += rFieldName;

    SwFieldList aLst(pWrtShell ? pWrtShell : ::lcl_GetShell());

    aLst.InsertFields(RES_DBFLD, &sFieldName);
    SwField* pFld = aLst.GetLastField();

    if(!pFld)
        pFld = aLst.GetNextField();

    if(pFld)
        return pFld->Expand();

    return aEmptyStr;
}

/*--------------------------------------------------------------------
     Beschreibung: Ist das Datenbankfeld numerisch?
     Anm: Im Fehlerfall wird TRUE returnt.
 --------------------------------------------------------------------*/

BOOL SwFldMgr::IsDBNumeric(const String& rDBName, const String& rTblQryName,
                                    BOOL bIsTable, const String& rFldName)
{
    BOOL bNumeric = TRUE;
    if(!GetDBContext().is() || !xDBContext->hasByName(rDBName))
        return bNumeric;
    Any aDBSource = xDBContext->getByName(rDBName);
    Reference<XDataSource>* pxSource = (Reference<XDataSource>*)aDBSource.getValue();
    Reference<XConnection> xConnection;
    try
    {
        OUString sDummy;
        xConnection = (*pxSource)->getConnection(sDummy, sDummy);
    }
    catch(...) {}
    Reference<XColumnsSupplier> xColsSupplier;

    if(bIsTable)
    {
        Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTbls = xTSupplier->getTables();
            DBG_ASSERT(xTbls->hasByName(rTblQryName), "table not available anymore?")
            try
            {
                Any aTable = xTbls->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet = *(Reference<XPropertySet>*)aTable.getValue();
                xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
            }
            catch(...){}
        }
    }
    else
    {
        Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            DBG_ASSERT(xQueries->hasByName(rTblQryName), "table not available anymore?")
            try
            {
                Any aQuery = xQueries->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet = *(Reference<XPropertySet>*)aQuery.getValue();
                xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
            }
            catch(...){}
        }
    }

    if(xColsSupplier.is())
    {
        Reference <XNameAccess> xCols = xColsSupplier->getColumns();
        if(xCols.is() && xCols->hasByName(rFldName))
        {
            Any aCol = xCols->getByName(rFldName);
            Reference <XPropertySet> xCol = *(Reference <XPropertySet>*)aCol.getValue();
            Any aType = xCol->getPropertyValue(C2S("Type"));
            sal_Int32 eDataType;
            aType >>= eDataType;
            switch(eDataType)
            {
                case DataType::BIT:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                case DataType::BIGINT:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::SQLNULL:
                case DataType::OTHER:
                case DataType::OBJECT:
                case DataType::DISTINCT:
                case DataType::STRUCT:
                case DataType::ARRAY:
                case DataType::BLOB:
                case DataType::CLOB:
                case DataType::REF:
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                default:
                    bNumeric = FALSE;
            }
        }
    }
    return bNumeric;
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
    if(pSh)
    {
        SfxItemSet aSet( pSh->GetAttrPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE);
        pSh->GetAttr(aSet);
        return ((const SvxLanguageItem&)aSet.Get( RES_CHRATR_LANGUAGE ) ).GetLanguage();
    }

    LanguageType eCurLanguage = GetpApp()->GetAppInternational().GetLanguage();

    return (USHORT)eCurLanguage;
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
        STR_AUTHORITY
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
    SwUserFieldType *pType =
        (SwUserFieldType *) (pWrtShell ?
            pWrtShell->GetFldType(RES_USERFLD, rName) :
                ::lcl_GetShell()->GetFldType(RES_USERFLD, rName));

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
    BOOL bRet = InsertFld(TYP_INTERNETFLD, 0, rName, rVal, 0);
    sCurFrame = aEmptyStr;
    pMacroItem = 0;
    return bRet;

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwFldMgr::ChooseMacro(const String &rSelMacro)
{
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
    SbMethod* pMethod = BasicIDE::ChooseMacro(FALSE, TRUE, sSelMacro);
    BOOL bRet = FALSE;

    if (pMethod)
    {
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();

        SetMacroModule(pModule);

        ASSERT(pObject->IsA(TYPE(StarBASIC)), "Kein Basic gefunden!");

        String sMacro(pMethod->GetName());
        sMacro += '.';
        sMacro += pModule->GetName();
        sMacro += '.';
        sMacro += pObject->GetName();
        sMacro += '.';

        StarBASIC* pLib = (StarBASIC*)pObject;
        pSfxApp->EnterBasicCall();
        BOOL bFound = FALSE;
        BasicManager* pBasicMgr = pSfxApp->GetBasicManager();
        SfxObjectShell* pDocShell = 0;
        while ( !bFound && pBasicMgr )
        {
            USHORT nLibs = pBasicMgr->GetLibCount();
            for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
            {
                StarBASIC* pL = pBasicMgr->GetLib( nLib );
                if ( pL == pLib )
                {
                    bFound = TRUE;
                    break;
                }
            }
            if(!bFound)
            {
                if ( pDocShell  )
                    pDocShell = SfxObjectShell::GetNext( *pDocShell );
                else
                    pDocShell = SfxObjectShell::GetFirst();
            }

            pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
        }
        pSfxApp->LeaveBasicCall();
        if(pDocShell)
            sMacro += pDocShell->GetName();
        else
            sMacro += pSfxApp->GetName();


        SetMacroPath(sMacro);
        bRet = TRUE;
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
            Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
            xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
        }
        DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
    }
    return xDBContext;
}


