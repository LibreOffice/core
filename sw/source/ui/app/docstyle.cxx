/*************************************************************************
 *
 *  $RCSfile: docstyle.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
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

#define _SVSTDARR_USHORTS

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif

#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _CCOLL_HXX
#include <ccoll.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWSTYLE_H
#include <swstyle.h>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif


// MD 06.02.95: Die Formatnamen in der Liste aller Namen haben als
// erstes Zeichen die Familie:

#define cCHAR       (sal_Unicode)'c'
#define cPARA       (sal_Unicode)'p'
#define cFRAME      (sal_Unicode)'f'
#define cPAGE       (sal_Unicode)'g'
#define cNUMRULE    (sal_Unicode)'n'

// Dieses Zeichen wird bei der Herausgabe der Namen wieder entfernt und
// die Familie wird neu generiert.

// Ausserdem gibt es jetzt zusaetzlich das Bit bPhysical. Ist dieses Bit
// TRUE, werden die Pool-Formatnamen NICHT mit eingetragen.


class SwImplShellAction
{
    SwWrtShell* pSh;
    CurrShell* pCurrSh;
public:
    SwImplShellAction( SwDoc& rDoc );
    ~SwImplShellAction();

    SwWrtShell* GetSh() { return pSh; }
};

SwImplShellAction::SwImplShellAction( SwDoc& rDoc )
    : pCurrSh( 0 )
{
    if( rDoc.GetDocShell() )
        pSh = rDoc.GetDocShell()->GetWrtShell();
    else
        pSh = 0;

    if( pSh )
    {
        pCurrSh = new CurrShell( pSh );
        pSh->StartAllAction();
    }
}

SwImplShellAction::~SwImplShellAction()
{
    if( pCurrSh )
    {
        pSh->EndAllAction();
        delete pCurrSh;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   SwCharFormate finden/anlegen
                    evtl. Style fuellen
 --------------------------------------------------------------------*/

SwCharFmt* lcl_FindCharFmt( SwDoc& rDoc,
                            const String& rName,
                            SwDocStyleSheet* pStyle = 0,
                            FASTBOOL bCreate = TRUE )
{
    SwCharFmt*  pFmt = 0;
    if( rName.Len() )
    {
        pFmt = rDoc.FindCharFmtByName( rName );
        if( !pFmt && rName == *rDoc.GetTextNmArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] )
        {
            // Standard-Zeichenvorlage
            pFmt = (SwCharFmt*)rDoc.GetDfltCharFmt();
        }

        if( !pFmt && bCreate )
        {   // Pool abklappern
            const USHORT nId = rDoc.GetPoolId(rName, GET_POOLID_CHRFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetCharFmtFromPool(nId);
        }
    }
    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(TRUE);
            SwFmt* p = pFmt->DerivedFrom();
            if( p && !p->IsDefault() )
                pStyle->PresetParent( p->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(FALSE);
    }
    return pFmt;
}


/*--------------------------------------------------------------------
    Beschreibung:   ParaFormate finden/erzeugen
                    Style fuellen
 --------------------------------------------------------------------*/

SwTxtFmtColl* lcl_FindParaFmt(  SwDoc& rDoc,
                                const String& rName,
                                SwDocStyleSheet* pStyle = 0,
                                FASTBOOL bCreate = TRUE )
{
    SwTxtFmtColl*   pColl = 0;

    if( rName.Len() )
    {
        pColl = rDoc.FindTxtFmtCollByName( rName );
        if( !pColl && bCreate )
        {   // Pool abklappern
            const USHORT nId = rDoc.GetPoolId(rName, GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                pColl = rDoc.GetTxtCollFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pColl)
        {
            pStyle->SetPhysical(TRUE);
            if( pColl->DerivedFrom() && !pColl->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pColl->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );

            SwTxtFmtColl& rNext = pColl->GetNextTxtFmtColl();
            pStyle->PresetFollow(rNext.GetName());
        }
        else
            pStyle->SetPhysical(FALSE);
    }
    return pColl;
}


/*--------------------------------------------------------------------
    Beschreibung:   Rahmenformate
 --------------------------------------------------------------------*/


SwFrmFmt* lcl_FindFrmFmt(   SwDoc& rDoc,
                            const String& rName,
                            SwDocStyleSheet* pStyle = 0,
                            FASTBOOL bCreate = TRUE )
{
    SwFrmFmt* pFmt = 0;
    if( rName.Len() )
    {
        pFmt = rDoc.FindFrmFmtByName( rName );
        if( !pFmt && bCreate )
        {   // Pool abklappern
            const USHORT nId = rDoc.GetPoolId(rName, GET_POOLID_FRMFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetFrmFmtFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(TRUE);
            if( pFmt->DerivedFrom() && !pFmt->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pFmt->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(FALSE);
    }
    return pFmt;
}

/*--------------------------------------------------------------------
    Beschreibung:   Seitendescriptoren
 --------------------------------------------------------------------*/


const SwPageDesc* lcl_FindPageDesc( SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    FASTBOOL bCreate = TRUE )
{
    const SwPageDesc* pDesc = 0;

    if( rName.Len() )
    {
        pDesc = rDoc.FindPageDescByName( rName );
        if( !pDesc && bCreate )
        {
            USHORT nId = rDoc.GetPoolId(rName, GET_POOLID_PAGEDESC);
            if(nId != USHRT_MAX)
                pDesc = rDoc.GetPageDescFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pDesc)
        {
            pStyle->SetPhysical(TRUE);
            if(pDesc->GetFollow())
                pStyle->PresetFollow(pDesc->GetFollow()->GetName());
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(FALSE);
    }
    return pDesc;
}

const SwNumRule* lcl_FindNumRule(   SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    FASTBOOL bCreate = TRUE )
{
    const SwNumRule* pRule = 0;

    if( rName.Len() )
    {
        pRule = rDoc.FindNumRulePtr( rName );
        if( !pRule && bCreate )
        {
            USHORT nId = rDoc.GetPoolId(rName, GET_POOLID_NUMRULE);
            if(nId != USHRT_MAX)
                pRule = rDoc.GetNumRuleFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pRule)
        {
            pStyle->SetPhysical(TRUE);
            pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(FALSE);
    }
    return pRule;
}


USHORT lcl_FindName( const SwPoolFmtList& rLst, SfxStyleFamily eFam,
                        const String& rName )
{
    if( rLst.Count() )
    {
        // suchen
        String sSrch( ' ' );
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:  sSrch = cCHAR;     break;
        case SFX_STYLE_FAMILY_PARA:  sSrch = cPARA;     break;
        case SFX_STYLE_FAMILY_FRAME: sSrch = cFRAME;    break;
        case SFX_STYLE_FAMILY_PAGE:  sSrch = cPAGE;     break;
        case SFX_STYLE_FAMILY_PSEUDO: sSrch = cNUMRULE; break;
        }
        sSrch += rName;
        for( USHORT i=0; i < rLst.Count(); ++i )
            if( *rLst[i] == sSrch )
                return i;
    }
    return USHRT_MAX;
}

FASTBOOL FindPhyStyle( SwDoc& rDoc, const String& rName, SfxStyleFamily eFam )
{
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        return 0 != lcl_FindCharFmt( rDoc, rName, 0, FALSE );
    case SFX_STYLE_FAMILY_PARA :
        return 0 != lcl_FindParaFmt( rDoc, rName, 0, FALSE );
    case SFX_STYLE_FAMILY_FRAME:
        return 0 != lcl_FindFrmFmt( rDoc, rName, 0, FALSE );
    case SFX_STYLE_FAMILY_PAGE :
        return 0 != lcl_FindPageDesc( rDoc, rName, 0, FALSE );
    case SFX_STYLE_FAMILY_PSEUDO:
        return 0 != lcl_FindNumRule( rDoc, rName, 0, FALSE );
    }
    return FALSE;
}


/*--------------------------------------------------------------------
    Beschreibung:   Einfuegen von Strings in die Liste der Vorlagen
 --------------------------------------------------------------------*/


void SwPoolFmtList::Append( char cChar, const String& rStr )
{
    String* pStr = new String( cChar );
    *pStr += rStr;
    for ( USHORT i=0; i < Count(); ++i )
    {
        if( *operator[](i) == *pStr )
        {
            delete pStr;
            return;
        }
    }
    Insert( pStr, Count() );
}

/*--------------------------------------------------------------------
    Beschreibung:   Liste kompletti loeschen
 --------------------------------------------------------------------*/


void SwPoolFmtList::Erase()
{
    DeleteAndDestroy( 0, Count() );
}

/*  */

/*--------------------------------------------------------------------
    Beschreibung:  UI-seitige implementierung von StyleSheets
                   greift auf die Core-Engine zu
 --------------------------------------------------------------------*/

SwDocStyleSheet::SwDocStyleSheet(   SwDoc&          rDocument,
                                    const String&           rName,
                                    SwDocStyleSheetPool&    rPool,
                                    SfxStyleFamily          eFam,
                                    USHORT                  nMask) :

    SfxStyleSheetBase( rName, rPool, eFam, nMask ),
    rDoc(rDocument),
    pCharFmt(0),
    pFrmFmt(0),
    pColl(0),
    pDesc(0),
    pNumRule(0),
    bPhysical(FALSE),
    aCoreSet(GetPool().GetPool(),
            RES_CHRATR_BEGIN,       RES_CHRATR_END - 1,
            RES_PARATR_BEGIN,       RES_PARATR_END - 1,
            RES_FRMATR_BEGIN,       RES_FRMATR_END - 1,
            SID_ATTR_PAGE,          SID_ATTR_PAGE_EXT1,
            SID_ATTR_PAGE_HEADERSET,SID_ATTR_PAGE_FOOTERSET,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            FN_PARAM_FTN_INFO,      FN_PARAM_FTN_INFO,
            SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_MODEL,
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,
            SID_SWREGISTER_MODE,    SID_SWREGISTER_COLLECTION,
            FN_COND_COLL,           FN_COND_COLL,
            SID_ATTR_AUTO_STYLE_UPDATE, SID_ATTR_AUTO_STYLE_UPDATE,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            SID_PARA_BACKGRND_DESTINATION,  SID_ATTR_BRUSH_CHAR,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            0)
{
    nHelpId = UCHAR_MAX;
}


SwDocStyleSheet::SwDocStyleSheet( const SwDocStyleSheet& rOrg) :
    SfxStyleSheetBase(rOrg),
    pCharFmt(rOrg.pCharFmt),
    pFrmFmt(rOrg.pFrmFmt),
    pColl(rOrg.pColl),
    pDesc(rOrg.pDesc),
    pNumRule(rOrg.pNumRule),
    rDoc(rOrg.rDoc),
    bPhysical(rOrg.bPhysical),
    aCoreSet(rOrg.aCoreSet)
{
}


 SwDocStyleSheet::~SwDocStyleSheet()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   Zuruecksetzen
 --------------------------------------------------------------------*/


void  SwDocStyleSheet::Reset()
{
    aName.Erase();
    aFollow.Erase();
    aParent.Erase();
    SetPhysical(FALSE);
}

/*--------------------------------------------------------------------
    Beschreibung:   virtuelle Methoden
 --------------------------------------------------------------------*/


const String&  SwDocStyleSheet::GetParent() const
{
    if( !bPhysical )
    {
        // dann pruefe, ob schon im Doc vorhanden
        SwFmt* pFmt = 0;
        SwGetPoolIdFromName eGetType;
        switch(nFamily)
        {
        case SFX_STYLE_FAMILY_CHAR:
            pFmt = rDoc.FindCharFmtByName( aName );
            eGetType = GET_POOLID_CHRFMT;
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFmt = rDoc.FindTxtFmtCollByName( aName );
            eGetType = GET_POOLID_TXTCOLL;
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFmt = rDoc.FindFrmFmtByName( aName );
            eGetType = GET_POOLID_FRMFMT;
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
        default:
            return aEmptyStr;       // es gibt keinen Parent
        }

        String sTmp;
        if( !pFmt )         // noch nicht vorhanden, also dflt. Parent
        {
            USHORT i = rDoc.GetPoolId( aName, eGetType );
            i = ::GetPoolParent( i );
            if( i && USHRT_MAX != i )
                SwDoc::GetPoolNm( i, sTmp );
        }
        else
        {
            SwFmt* p = pFmt->DerivedFrom();
            if( p && !p->IsDefault() )
                sTmp = p->GetName();
        }
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->aParent = sTmp;
    }
    return aParent;
}

/*--------------------------------------------------------------------
   Beschreibung:    Nachfolger
 --------------------------------------------------------------------*/


const String&  SwDocStyleSheet::GetFollow() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->FillStyleSheet( FillAllInfo );
    }
    return aFollow;
}

/*--------------------------------------------------------------------
    Beschreibung:   Welche Verkettung ist moeglich
 --------------------------------------------------------------------*/


BOOL  SwDocStyleSheet::HasFollowSupport() const
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_PAGE : return TRUE;
        case SFX_STYLE_FAMILY_FRAME:
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PSEUDO: return FALSE;
        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Parent ?
 --------------------------------------------------------------------*/


BOOL  SwDocStyleSheet::HasParentSupport() const
{
    BOOL bRet = FALSE;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_FRAME: bRet = TRUE;
    }
    return bRet;
}


BOOL  SwDocStyleSheet::HasClearParentSupport() const
{
    BOOL bRet = FALSE;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_FRAME: bRet = TRUE;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   textuelle Beschreibung ermitteln
 --------------------------------------------------------------------*/
String  SwDocStyleSheet::GetDescription(SfxMapUnit eUnit)
{
    String sPlus(String::CreateFromAscii(" + "));
    if ( SFX_STYLE_FAMILY_PAGE == nFamily )
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        String aDesc;
        const SfxPoolItem* pItem = aIter.FirstItem();

        while ( pItem )
        {
            if(!IsInvalidItem(pItem))
                switch ( pItem->Which() )
                {
                    case RES_LR_SPACE:
                    case SID_ATTR_PAGE_SIZE:
                    case SID_ATTR_PAGE_MAXSIZE:
                    case SID_ATTR_PAGE_PAPERBIN:
                    case SID_ATTR_PAGE_APP:
                    case SID_ATTR_BORDER_INNER:
                        break;
                    default:
                    {
                        String aItemPresentation;
                        if ( !IsInvalidItem( pItem ) &&
                             rPool.GetPool().GetPresentation(
                                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                                eUnit, aItemPresentation ) )
                        {
                            if ( aDesc.Len() && aItemPresentation.Len() )
                                aDesc += sPlus;
                            aDesc += aItemPresentation;
                        }
                    }
                }
            pItem = aIter.NextItem();
        }
        return aDesc;
    }
    else if ( SFX_STYLE_FAMILY_FRAME == nFamily ||
                    SFX_STYLE_FAMILY_PARA == nFamily)
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        String aDesc;
        const SfxPoolItem* pItem = aIter.FirstItem();

        String sPageNum, sModel, sBreak;

        while ( pItem )
        {
            if(!IsInvalidItem(pItem))
                switch ( pItem->Which() )
                {
                    case SID_ATTR_AUTO_STYLE_UPDATE:
                    case SID_PARA_BACKGRND_DESTINATION:
                    case RES_PAGEDESC:
                        break;
                    default:
                    {
                        String aItemPresentation;
                        if ( !IsInvalidItem( pItem ) &&
                             rPool.GetPool().GetPresentation(
                                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                                eUnit, aItemPresentation ) )
                        {

                            switch ( pItem->Which() )
                            {
                                case SID_ATTR_PARA_PAGENUM:
                                    sPageNum = aItemPresentation;
                                    break;
                                case SID_ATTR_PARA_MODEL:
                                    sModel = aItemPresentation;
                                    break;
                                case RES_BREAK:
                                    sBreak = aItemPresentation;
                                    break;
                                default:
                                if ( aDesc.Len() && aItemPresentation.Len() )
                                    aDesc += sPlus;
                                aDesc += aItemPresentation;
                            }
                        }
                    }
                }
            pItem = aIter.NextItem();
        }
        //Sonderbehandlung fuer Umburch, Seitenvorlage und Seitenoffset
        if(sBreak.Len() && !sModel.Len())  // wemm Model. dann ist Break ungueltig
        {
            if(aDesc.Len())
                aDesc += sPlus;
            aDesc += sBreak;
        }
        if(sModel.Len())
        {
            if(aDesc.Len())
                aDesc += sPlus;
            aDesc += SW_RESSTR(STR_PAGEBREAK);
            aDesc += sPlus;
            aDesc += sModel;
            if(sPageNum != String(UniString::CreateFromInt32(0)))
            {
                aDesc += sPlus;
                aDesc += SW_RESSTR(STR_PAGEOFFSET);
                aDesc += sPageNum;
            }
        }
        return aDesc;
    }
    else if( SFX_STYLE_FAMILY_PSEUDO == nFamily )
    {
//      if( pNumRule )
//          return pNumRule->GetName();
        //os: was sollte man bei Numerierungen schon anzeigen?
        return aEmptyStr;
    }

    return SfxStyleSheetBase::GetDescription(eUnit);
}


String  SwDocStyleSheet::GetDescription()
{
    return GetDescription(SFX_MAPUNIT_CM);
}

/*--------------------------------------------------------------------
    Beschreibung:   Namen setzen
 --------------------------------------------------------------------*/


BOOL  SwDocStyleSheet::SetName( const String& rStr)
{
    if( !rStr.Len() )
        return FALSE;

    if( aName != rStr )
    {
        if( !SfxStyleSheetBase::SetName( rStr ))
            return FALSE;
    }
    else if(!bPhysical)
        FillStyleSheet( FillPhysical );

    int bChg = FALSE;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        {
            ASSERT(pCharFmt, "SwCharFormat fehlt!");
            if( pCharFmt && pCharFmt->GetName() != rStr )
            {
                pCharFmt->SetName( rStr );
                bChg = TRUE;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PARA :
        {
            ASSERT(pColl, "Collektion fehlt!");
            if( pColl && pColl->GetName() != rStr )
            {
                pColl->SetName( rStr );
                bChg = TRUE;
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            ASSERT(pFrmFmt, "FrmFmt fehlt!");
            if( pFrmFmt && pFrmFmt->GetName() != rStr )
            {
                pFrmFmt->SetName( rStr );
                bChg = TRUE;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PAGE :
            ASSERT(pDesc, "PageDesc fehlt!");
            if( pDesc && pDesc->GetName() != rStr )
            {
                //PageDesc setzen - mit vorherigem kopieren - ist fuer das
                //setzen des Namens wohl nicht notwendig. Deshalb erlauben
                //wir hier mal einen cast.
                ((SwPageDesc*)pDesc)->SetName( rStr );
                rDoc.SetModified();
                bChg = TRUE;
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            ASSERT(pNumRule, "NumRule fehlt!");
            if( pNumRule && pNumRule->GetName() != rStr )
            {
                ((SwNumRule*)pNumRule)->SetName( rStr );
                rDoc.SetModified();
                bChg = TRUE;
            }
            break;


#ifdef DBG_UTIL
        default:
            ASSERT(!this, "unbekannte Style-Familie");
#endif
    }

    if( bChg )
    {
        rPool.First();      // interne Liste muss geupdatet werden
        rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Ableitungshirachie
 --------------------------------------------------------------------*/


BOOL   SwDocStyleSheet::SetParent( const String& rStr)
{
    SwFmt* pFmt = 0, *pParent = 0;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            ASSERT( pCharFmt, "SwCharFormat fehlt!" )
            if( 0 != ( pFmt = pCharFmt ) && rStr.Len() )
                pParent = lcl_FindCharFmt(rDoc, rStr);
            break;

        case SFX_STYLE_FAMILY_PARA :
            ASSERT( pColl, "Collektion fehlt!")
            if( 0 != ( pFmt = pColl ) && rStr.Len() )
                pParent = lcl_FindParaFmt( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_FRAME:
            ASSERT(pFrmFmt, "FrameFormat fehlt!");
            if( 0 != ( pFmt = pFrmFmt ) && rStr.Len() )
                pParent = lcl_FindFrmFmt( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
            break;
        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }

    BOOL bRet = FALSE;
    if( pFmt && pFmt->DerivedFrom() &&
        pFmt->DerivedFrom()->GetName() != rStr )
    {
        {
            SwImplShellAction aTmp( rDoc );
            bRet = pFmt->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            aParent = rStr;
            rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
                            *this ) );
        }
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Nachfolger detzen
 --------------------------------------------------------------------*/


BOOL   SwDocStyleSheet::SetFollow( const String& rStr)
{
    if( rStr.Len() && !SfxStyleSheetBase::SetFollow( rStr ))
        return FALSE;

    SwImplShellAction aTmpSh( rDoc );
    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_PARA :
    {
        ASSERT(pColl, "Collection fehlt!");
        if( pColl )
        {
            SwTxtFmtColl* pFollow = pColl;
            if( rStr.Len() && 0 == (pFollow = lcl_FindParaFmt(rDoc, rStr) ))
                pFollow = pColl;

            pColl->SetNextTxtFmtColl(*pFollow);
        }
        break;
    }
    case SFX_STYLE_FAMILY_PAGE :
    {
        ASSERT(pDesc, "PageDesc fehlt!");
        if( pDesc )
        {
            const SwPageDesc* pFollowDesc = rStr.Len()
                                            ? lcl_FindPageDesc(rDoc, rStr)
                                            : 0;
            USHORT nId;
            if( rDoc.FindPageDescByName( pDesc->GetName(), &nId ))
            {
                SwPageDesc aDesc( *pDesc );
                aDesc.SetFollow( pFollowDesc );
                rDoc.ChgPageDesc( nId, aDesc );
                pDesc = &rDoc.GetPageDesc( nId );
            }
        }
        break;
    }
    case SFX_STYLE_FAMILY_CHAR:
    case SFX_STYLE_FAMILY_FRAME:
    case SFX_STYLE_FAMILY_PSEUDO:
        break;
    default:
        ASSERT(!this, "unbekannte Style-Familie");
    }

    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:   ueber Name und Family, Mask den ItemSet rausholen
 --------------------------------------------------------------------*/

SfxItemSet&   SwDocStyleSheet::GetItemSet()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            {
                ASSERT(pCharFmt, "Wo ist das SwCharFmt");
                aCoreSet.Put(pCharFmt->GetAttrSet());
                if(pCharFmt->DerivedFrom())
                    aCoreSet.SetParent(&pCharFmt->DerivedFrom()->GetAttrSet());
            }
            break;
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_FRAME:
            {
                SvxBoxInfoItem aBoxInfo;
                aBoxInfo.SetTable( FALSE );
                aBoxInfo.SetDist( TRUE);    // Abstandsfeld immer anzeigen
                aBoxInfo.SetMinDist( TRUE );// Minimalgroesse in Tabellen und Absaetzen setzen
                aBoxInfo.SetDefDist( MIN_BORDER_DIST );// Default-Abstand immer setzen
                    // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
                aBoxInfo.SetValid( VALID_DISABLE, TRUE );
                if ( nFamily == SFX_STYLE_FAMILY_PARA )
                {
                    ASSERT(pColl, "Wo ist die Collektion");
                    aCoreSet.Put(pColl->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pColl->IsAutoUpdateFmt()));
                    if(pColl->DerivedFrom())
                        aCoreSet.SetParent(&pColl->DerivedFrom()->GetAttrSet());
                }
                else
                {
                    ASSERT(pFrmFmt, "Wo ist das FrmFmt");
                    aCoreSet.Put(pFrmFmt->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pFrmFmt->IsAutoUpdateFmt()));
                    if(pFrmFmt->DerivedFrom())
                        aCoreSet.SetParent(&pFrmFmt->DerivedFrom()->GetAttrSet());
                }
            }
            break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                ASSERT(pDesc, "Kein PageDescriptor");
                ::PageDescToItemSet(*((SwPageDesc*)pDesc), aCoreSet);
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                ASSERT(pNumRule, "Keine NumRule");
                SvxNumRule aRule = pNumRule->MakeSvxNumRule();
                aCoreSet.Put(SvxNumBulletItem(aRule));
            }
            break;


#ifdef DBG_UTIL
        default:
            ASSERT(!this, "unbekannte Style-Familie");
#endif
    }
    // Member der Basisklasse
    pSet = &aCoreSet;

    return aCoreSet;
}

/*--------------------------------------------------------------------
    Beschreibung:   ItemSet setzen
 --------------------------------------------------------------------*/


void   SwDocStyleSheet::SetItemSet(const SfxItemSet& rSet)
{
    // gegebenenfalls Format erst ermitteln
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    SwImplShellAction aTmpSh( rDoc );

    ASSERT( &rSet != &aCoreSet, "SetItemSet mit eigenem Set ist nicht erlaubt" );

    SwFmt* pFmt = 0;
    SwPageDesc* pNewDsc = 0;
    USHORT nPgDscPos;

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            {
                ASSERT(pCharFmt, "Wo ist das CharFormat");
                pFmt = pCharFmt;
            }
            break;

        case SFX_STYLE_FAMILY_PARA :
        {
            ASSERT(pColl, "Wo ist die Collection");
            const SfxPoolItem* pAutoUpdate;
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,FALSE, &pAutoUpdate ))
            {
                pColl->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
            }

            const SwCondCollItem* pCondItem;
            if( SFX_ITEM_SET != rSet.GetItemState( FN_COND_COLL, FALSE,
                (const SfxPoolItem**)&pCondItem ))
                pCondItem = 0;

            if( RES_CONDTXTFMTCOLL == pColl->Which() && pCondItem )
            {
                SwFmt* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for(USHORT i = 0; i < COND_COMMAND_COUNT; i++)
                {
                    SwCollCondition aCond( 0, pCmds[ i ].nCnd, pCmds[ i ].nSubCond );
                    ((SwConditionTxtFmtColl*)pColl)->RemoveCondition( aCond );
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, TRUE )))
                    {
                        pFindFmt->Add( &aCond );
                        ((SwConditionTxtFmtColl*)pColl)->InsertCondition( aCond );
                    }
                }

                // Document auf die neue Bedingungen updaten
                SwCondCollCondChg aMsg( pColl );
                pColl->Modify( &aMsg, &aMsg );
            }
            else if( pCondItem && !pColl->GetDepends() )
            {
                // keine bedingte Vorlage, dann erstmal erzeugen und
                // alle wichtigen Werte uebernehmen
                SwConditionTxtFmtColl* pCColl = rDoc.MakeCondTxtFmtColl(
                        pColl->GetName(), (SwTxtFmtColl*)pColl->DerivedFrom() );
                if( pColl != &pColl->GetNextTxtFmtColl() )
                    pCColl->SetNextTxtFmtColl( pColl->GetNextTxtFmtColl() );

                pCColl->SetOutlineLevel( pColl->GetOutlineLevel() );

                SwTxtFmtColl* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for( USHORT i = 0; i < COND_COMMAND_COUNT; ++i )
                {
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, TRUE )))
                    {
                        pCColl->InsertCondition( SwCollCondition( pFindFmt,
                                    pCmds[ i ].nCnd, pCmds[ i ].nSubCond ) );
                    }
                }

                rDoc.DelTxtFmtColl( pColl );
                pColl = pCColl;
            }
            pFmt = pColl;

            USHORT nId = pColl->GetPoolFmtId() &
                            ~ ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID );
            switch( GetMask() & ( 0x0fff & ~SWSTYLEBIT_CONDCOLL ) )
            {
                case SWSTYLEBIT_TEXT:
                    nId |= COLL_TEXT_BITS;
                    break;
                case SWSTYLEBIT_CHAPTER:
                    nId |= COLL_DOC_BITS;
                    break;
                case SWSTYLEBIT_LIST:
                    nId |= COLL_LISTS_BITS;
                    break;
                case SWSTYLEBIT_IDX:
                    nId |= COLL_REGISTER_BITS;
                    break;
                case SWSTYLEBIT_EXTRA:
                    nId |= COLL_EXTRA_BITS;
                    break;
                case SWSTYLEBIT_HTML:
                    nId |= COLL_HTML_BITS;
                    break;
            }
            pColl->SetPoolFmtId( nId );
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            ASSERT(pFrmFmt, "Wo ist das FrmFmt");
            const SfxPoolItem* pAutoUpdate;
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,FALSE, &pAutoUpdate ))
            {
                pFrmFmt->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
            }
            pFmt = pFrmFmt;
        }
        break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                ASSERT(pDesc, "Wo ist der PageDescriptor");

                if( rDoc.FindPageDescByName( pDesc->GetName(), &nPgDscPos ))
                {
                    pNewDsc = new SwPageDesc( *pDesc );
                    pFmt = &pNewDsc->GetMaster();
                }
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                ASSERT(pNumRule, "Wo ist die NumRule");
                const SfxPoolItem* pItem;
                switch( rSet.GetItemState( SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
                {
                case SFX_ITEM_SET:
                {
                    SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                    pSetRule->UnLinkGraphics();
                    //SwNumRule aSetRule(rDoc.GetUniqueNumRuleName());
                    SwNumRule aSetRule(pNumRule->GetName());
                    aSetRule.SetSvxRule(*pSetRule, &rDoc);
                    rDoc.ChgNumRuleFmts( aSetRule );
                }
                break;
                case SFX_ITEM_DONTCARE:
                    // NumRule auf default Werte
                    // was sind die default Werte?
                    {
                        SwNumRule aRule( pNumRule->GetName() );
                        rDoc.ChgNumRuleFmts( aRule );
                    }
                    break;
                }
            }
            break;

#ifdef DBG_UTIL
        default:
            ASSERT(!this, "unbekannte Style-Familie");
#endif
    }

    if( pFmt && rSet.Count())
    {
        SfxItemIter aIter( rSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( TRUE )
        {
            if( IsInvalidItem( pItem ) )            // Clearen
                pFmt->ResetAttr( rSet.GetWhichByPos(aIter.GetCurPos()));

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
        SfxItemSet aSet(rSet);
        aSet.ClearInvalidItems();

        aCoreSet.ClearItem();

        if( pNewDsc )
        {
            ::ItemSetToPageDesc( aSet, *pNewDsc,
                                ::GetActiveWrtShell());
            rDoc.ChgPageDesc( nPgDscPos, *pNewDsc );
            pDesc = &rDoc.GetPageDesc( nPgDscPos );
            delete pNewDsc;
        }
        else
            pFmt->SetAttr( aSet );      // alles gesetzten Putten
    }
    else
    {
        aCoreSet.ClearItem();
        if( pNewDsc )           // den muessen wir noch vernichten!!
            delete pNewDsc;
    }
}

void lcl_SaveStyles( USHORT nFamily, SvPtrarr& rArr, SwDoc& rDoc )
{
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            const SwCharFmts& rTbl = *rDoc.GetCharFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                rArr.Insert( p, n );
            }
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            const SwTxtFmtColls& rTbl = *rDoc.GetTxtFmtColls();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                rArr.Insert( p, n );
            }
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            const SwFrmFmts& rTbl = *rDoc.GetFrmFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                rArr.Insert( p, n );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            for( sal_uInt16 n = 0, nCnt = rDoc.GetPageDescCnt(); n < nCnt; ++n )
            {
                void* p = (void*)&rDoc.GetPageDesc( n );
                rArr.Insert( p, n );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            const SwNumRuleTbl& rTbl = rDoc.GetNumRuleTbl();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                rArr.Insert( p, n );
            }
        }
        break;
    }
}

void lcl_DeleteInfoStyles( USHORT nFamily, SvPtrarr& rArr, SwDoc& rDoc )
{
    sal_uInt16 n, nCnt;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SvUShorts aDelArr;
            const SwCharFmts& rTbl = *rDoc.GetCharFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                if( USHRT_MAX == rArr.GetPos( p ))
                    aDelArr.Insert( n, 0 );
            }
            for( n = 0, nCnt = aDelArr.Count(); n < nCnt; ++n )
                rDoc.DelCharFmt( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PARA :
        {
            SvUShorts aDelArr;
            const SwTxtFmtColls& rTbl = *rDoc.GetTxtFmtColls();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                if( USHRT_MAX == rArr.GetPos( p ))
                    aDelArr.Insert( n, 0 );
            }
            for( n = 0, nCnt = aDelArr.Count(); n < nCnt; ++n )
                rDoc.DelTxtFmtColl( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        {
            SvPtrarr aDelArr;
            const SwFrmFmts& rTbl = *rDoc.GetFrmFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                if( USHRT_MAX == rArr.GetPos( p ))
                    aDelArr.Insert( p, 0 );
            }
            for( n = 0, nCnt = aDelArr.Count(); n < nCnt; ++n )
                rDoc.DelFrmFmt( (SwFrmFmt*)aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            SvUShorts aDelArr;
            for( n = 0, nCnt = rDoc.GetPageDescCnt(); n < nCnt; ++n )
            {
                void* p = (void*)&rDoc.GetPageDesc( n );
                if( USHRT_MAX == rArr.GetPos( p ))
                    aDelArr.Insert( n, 0 );
            }
            for( n = 0, nCnt = aDelArr.Count(); n < nCnt; ++n )
                rDoc.DelPageDesc( aDelArr[ n ] );
        }
        break;


    case SFX_STYLE_FAMILY_PSEUDO:
        {
            SvPtrarr aDelArr;
            const SwNumRuleTbl& rTbl = rDoc.GetNumRuleTbl();
            for( n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
            {
                void* p = (void*)rTbl[ n ];
                if( USHRT_MAX == rArr.GetPos( p ))
                    aDelArr.Insert( p, 0 );
            }
            for( n = 0, nCnt = aDelArr.Count(); n < nCnt; ++n )
                rDoc.DelNumRule( ((SwNumRule*)aDelArr[ n ])->GetName() );
        }
        break;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Das Format ermitteln
 --------------------------------------------------------------------*/

BOOL SwDocStyleSheet::FillStyleSheet( FillStyleType eFType )
{
    BOOL bRet = FALSE;
    USHORT nPoolId = USHRT_MAX;
    SwFmt* pFmt = 0;

    BOOL bCreate = FillPhysical == eFType;
    BOOL bDeleteInfo = FALSE;
    BOOL bFillOnlyInfo = FillAllInfo == eFType;
    SvPtrarr aDelArr;

    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_CHAR:
        pCharFmt = lcl_FindCharFmt(rDoc, aName, this, bCreate );
        bPhysical = 0 != pCharFmt;
        if( bFillOnlyInfo && !bPhysical )
        {
            bDeleteInfo = TRUE;
            ::lcl_SaveStyles( nFamily, aDelArr, rDoc );
            pCharFmt = lcl_FindCharFmt(rDoc, aName, this, TRUE );
        }

        pFmt = pCharFmt;
        if( !bCreate && !pFmt )
        {
            if( aName == *rDoc.GetTextNmArray()[ RES_POOLCOLL_STANDARD -
                                            RES_POOLCOLL_TEXT_BEGIN ] )
                nPoolId = 0;
            else
                nPoolId = rDoc.GetPoolId( aName, GET_POOLID_CHRFMT );
        }

        bRet = 0 != pCharFmt || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pCharFmt = 0;
        break;

    case SFX_STYLE_FAMILY_PARA:
        {
            pColl = lcl_FindParaFmt(rDoc, aName, this, bCreate);
            bPhysical = 0 != pColl;
            if( bFillOnlyInfo && !bPhysical )
            {
                bDeleteInfo = TRUE;
                ::lcl_SaveStyles( nFamily, aDelArr, rDoc );
                pColl = lcl_FindParaFmt(rDoc, aName, this, TRUE );
            }

            pFmt = pColl;
            if( pColl )
                PresetFollow( pColl->GetNextTxtFmtColl().GetName() );
            else if( !bCreate )
                nPoolId = rDoc.GetPoolId( aName, GET_POOLID_TXTCOLL );

            bRet = 0 != pColl || USHRT_MAX != nPoolId;

            if( bDeleteInfo )
                pColl = 0;
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        pFrmFmt = lcl_FindFrmFmt(rDoc,  aName, this, bCreate);
        bPhysical = 0 != pFrmFmt;
        if( bFillOnlyInfo && bPhysical )
        {
            bDeleteInfo = TRUE;
            ::lcl_SaveStyles( nFamily, aDelArr, rDoc );
            pFrmFmt = lcl_FindFrmFmt(rDoc, aName, this, TRUE );
        }
        pFmt = pFrmFmt;
        if( !bCreate && !pFmt )
            nPoolId = rDoc.GetPoolId( aName, GET_POOLID_FRMFMT );

        bRet = 0 != pFrmFmt || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pFrmFmt = 0;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        pDesc = lcl_FindPageDesc(rDoc, aName, this, bCreate);
        bPhysical = 0 != pDesc;
        if( bFillOnlyInfo && !pDesc )
        {
            bDeleteInfo = TRUE;
            ::lcl_SaveStyles( nFamily, aDelArr, rDoc );
            pDesc = lcl_FindPageDesc( rDoc, aName, this, TRUE );
        }

        if( pDesc )
        {
            nPoolId = pDesc->GetPoolFmtId();
            nHelpId = pDesc->GetPoolHelpId();
            if( pDesc->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pDesc->GetPoolHlpFileId() );
            else
                aHelpFile.Erase();
        }
        else if( !bCreate )
            nPoolId = rDoc.GetPoolId( aName, GET_POOLID_PAGEDESC );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = 0 != pDesc || USHRT_MAX != nPoolId;
        if( bDeleteInfo )
            pDesc = 0;
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        pNumRule = lcl_FindNumRule(rDoc, aName, this, bCreate);
        bPhysical = 0 != pNumRule;
        if( bFillOnlyInfo && !pNumRule )
        {
            bDeleteInfo = TRUE;
            ::lcl_SaveStyles( nFamily, aDelArr, rDoc );
            pNumRule = lcl_FindNumRule( rDoc, aName, this, TRUE );
        }

        if( pNumRule )
        {
            nPoolId = pNumRule->GetPoolFmtId();
            nHelpId = pNumRule->GetPoolHelpId();
            if( pNumRule->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pNumRule->GetPoolHlpFileId() );
            else
                aHelpFile.Erase();
        }
        else if( !bCreate )
            nPoolId = rDoc.GetPoolId( aName, GET_POOLID_NUMRULE );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = 0 != pNumRule || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pNumRule = 0;
        break;
    }

    if( SFX_STYLE_FAMILY_CHAR == nFamily ||
        SFX_STYLE_FAMILY_PARA == nFamily ||
        SFX_STYLE_FAMILY_FRAME == nFamily )
    {
        if( pFmt )
            nPoolId = pFmt->GetPoolFmtId();

        USHORT nMask = 0;
        if( pFmt == rDoc.GetDfltCharFmt() )
            nMask |= SFXSTYLEBIT_READONLY;
        else if( USER_FMT & nPoolId )
            nMask |= SFXSTYLEBIT_USERDEF;

        switch ( COLL_GET_RANGE_BITS & nPoolId )
        {
        case COLL_TEXT_BITS:     nMask |= SWSTYLEBIT_TEXT;    break;
        case COLL_DOC_BITS :     nMask |= SWSTYLEBIT_CHAPTER; break;
        case COLL_LISTS_BITS:    nMask |= SWSTYLEBIT_LIST;    break;
        case COLL_REGISTER_BITS: nMask |= SWSTYLEBIT_IDX;     break;
        case COLL_EXTRA_BITS:    nMask |= SWSTYLEBIT_EXTRA;   break;
        case COLL_HTML_BITS:     nMask |= SWSTYLEBIT_HTML;    break;
        }

        if( pFmt )
        {
            ASSERT( bPhysical, "Format nicht gefunden" );

            nHelpId = pFmt->GetPoolHelpId();
            if( pFmt->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pFmt->GetPoolHlpFileId() );
            else
                aHelpFile.Erase();

            if( RES_CONDTXTFMTCOLL == pFmt->Which() )
                nMask |= SWSTYLEBIT_CONDCOLL;
        }

        SetMask( nMask );
    }
    if( bDeleteInfo && bFillOnlyInfo )
        ::lcl_DeleteInfoStyles( nFamily, aDelArr, rDoc );
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Neues Format in der Core anlegen
 --------------------------------------------------------------------*/


void   SwDocStyleSheet::Create()
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            pCharFmt = lcl_FindCharFmt( rDoc, aName );
            if( !pCharFmt )
                pCharFmt = rDoc.MakeCharFmt(aName,
                                            rDoc.GetDfltCharFmt());
            pCharFmt->SetAuto( FALSE );
            break;

        case SFX_STYLE_FAMILY_PARA :
            pColl = lcl_FindParaFmt( rDoc, aName );
            if( !pColl )
            {
                SwTxtFmtColl *pPar = (*rDoc.GetTxtFmtColls())[0];
                if( nMask & SWSTYLEBIT_CONDCOLL )
                    pColl = rDoc.MakeCondTxtFmtColl( aName, pPar );
                else
                    pColl = rDoc.MakeTxtFmtColl( aName, pPar );
            }
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFrmFmt = lcl_FindFrmFmt( rDoc, aName );
            if( !pFrmFmt )
                pFrmFmt = rDoc.MakeFrmFmt(aName, rDoc.GetDfltFrmFmt());
            pFrmFmt->SetAuto( FALSE );
            break;

        case SFX_STYLE_FAMILY_PAGE :
            pDesc = lcl_FindPageDesc( rDoc, aName );
            if( !pDesc )
            {
                USHORT nId = rDoc.MakePageDesc(aName);
                pDesc = &rDoc.GetPageDesc(nId);
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            pNumRule = lcl_FindNumRule( rDoc, aName );
            if( !pNumRule )
            {
                //JP 05.02.99: temp Namen erzeugen, damit kein ASSERT kommt
                String sTmpNm( aName );
                if( !aName.Len() )
                    sTmpNm = rDoc.GetUniqueNumRuleName();

                SwNumRule* pRule = rDoc.GetNumRuleTbl()[
                                        rDoc.MakeNumRule( sTmpNm ) ];
                pRule->SetAutoRule( FALSE );
                if( !aName.Len() )
                    pRule->SetName( aName );
                pNumRule = pRule;
            }
            break;
    }
    bPhysical = TRUE;
    aCoreSet.ClearItem();
}

/*--------------------------------------------------------------------
    Beschreibung:   Konkrete Formate rausholen
 --------------------------------------------------------------------*/



SwCharFmt* SwDocStyleSheet::GetCharFmt()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pCharFmt;
}


SwTxtFmtColl* SwDocStyleSheet::GetCollection()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pColl;
}


const SwPageDesc* SwDocStyleSheet::GetPageDesc()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pDesc;
}

const SwNumRule * SwDocStyleSheet::GetNumRule()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pNumRule;
}

void SwDocStyleSheet::SetNumRule(const SwNumRule& rRule)
{
    DBG_ASSERT(pNumRule, "Wo ist die NumRule")
    rDoc.ChgNumRuleFmts( rRule );
}

// Namen UND Familie aus String re-generieren
// First() und Next() (s.u.) fuegen einen Kennbuchstaben an Pos.1 ein

void SwDocStyleSheet::PresetNameAndFamily(const String& rName)
{
    switch( rName.GetChar(0) )
    {
        case cPARA:     nFamily = SFX_STYLE_FAMILY_PARA; break;
        case cFRAME:    nFamily = SFX_STYLE_FAMILY_FRAME; break;
        case cPAGE:     nFamily = SFX_STYLE_FAMILY_PAGE; break;
        case cNUMRULE:  nFamily = SFX_STYLE_FAMILY_PSEUDO; break;
        default:        nFamily = SFX_STYLE_FAMILY_CHAR; break;
    }
    aName = rName;
    aName.Erase( 0, 1 );
}

/*--------------------------------------------------------------------
    Beschreibung:   Ist das Format physikalisch schon vorhanden
 --------------------------------------------------------------------*/


void SwDocStyleSheet::SetPhysical(BOOL bPhys)
{
    bPhysical = bPhys;

    if(!bPhys)
    {
        pCharFmt = 0;
        pColl    = 0;
        pFrmFmt  = 0;
        pDesc    = 0;
    }
}

SwFrmFmt* SwDocStyleSheet::GetFrmFmt()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pFrmFmt;
}


BOOL  SwDocStyleSheet::IsUsed() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->FillStyleSheet( FillOnlyName );
    }

    // immer noch nicht ?
    if( !bPhysical )
        return FALSE;

    const SwModify* pMod;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pMod = pCharFmt;   break;
    case SFX_STYLE_FAMILY_PARA : pMod = pColl;      break;
    case SFX_STYLE_FAMILY_FRAME: pMod = pFrmFmt;    break;
    case SFX_STYLE_FAMILY_PAGE : pMod = pDesc;      break;

    case SFX_STYLE_FAMILY_PSEUDO:
            return pNumRule ? rDoc.IsUsed( *pNumRule ) : FALSE;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        return FALSE;
    }
    return rDoc.IsUsed( *pMod );
}


ULONG  SwDocStyleSheet::GetHelpId( String& rFile )
{
static String sTemplateHelpFile = String::CreateFromAscii("swrhlppi.hlp");

    USHORT nId, nPoolId;
    unsigned char nFileId = UCHAR_MAX;

    rFile = sTemplateHelpFile;

    const SwFmt* pTmpFmt = 0;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( !pCharFmt &&
            0 == (pCharFmt = lcl_FindCharFmt( rDoc, aName, 0, FALSE )) )
        {
            nId = rDoc.GetPoolId( aName, GET_POOLID_CHRFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pCharFmt;
        break;

    case SFX_STYLE_FAMILY_PARA:
        if( !pColl &&
            0 == ( pColl = lcl_FindParaFmt( rDoc, aName, 0, FALSE )) )
        {
            nId = rDoc.GetPoolId( aName, GET_POOLID_TXTCOLL );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pColl;
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( !pFrmFmt &&
            0 == ( pFrmFmt = lcl_FindFrmFmt( rDoc, aName, 0, FALSE ) ) )
        {
            nId = rDoc.GetPoolId( aName, GET_POOLID_FRMFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pFrmFmt;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        if( !pDesc &&
            0 == ( pDesc = lcl_FindPageDesc( rDoc, aName, 0, FALSE ) ) )
        {
            nId = rDoc.GetPoolId( aName, GET_POOLID_PAGEDESC );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pDesc->GetPoolHelpId();
        nFileId = pDesc->GetPoolHlpFileId();
        nPoolId = pDesc->GetPoolFmtId();
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        if( !pNumRule &&
            0 == ( pNumRule = lcl_FindNumRule( rDoc, aName, 0, FALSE ) ) )
        {
            nId = rDoc.GetPoolId( aName, GET_POOLID_NUMRULE );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pNumRule->GetPoolHelpId();
        nFileId = pNumRule->GetPoolHlpFileId();
        nPoolId = pNumRule->GetPoolFmtId();
        break;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        return 0;
    }

    if( pTmpFmt )
    {
        nId = pTmpFmt->GetPoolHelpId();
        nFileId = pTmpFmt->GetPoolHlpFileId();
        nPoolId = pTmpFmt->GetPoolFmtId();
    }

    if( UCHAR_MAX != nFileId )
    {
        const String *pTemplate = rDoc.GetDocPattern( nFileId );
        if( pTemplate )
        {
//          const String aHelpPath(MakeHelpPath(*pTemplate));
            rFile = *pTemplate;
        }
    }
    else if( !IsPoolUserFmt( nPoolId ) )
    {
        nId = nPoolId;
    }

    // weil sich der SFX so anstellt mit der HilfeId:
    if( USHRT_MAX == nId )
        nId = 0;        // entsp. keine Hilfe anzeigen

    return nId;
}


void  SwDocStyleSheet::SetHelpId( const String& r, ULONG nId )
{
    BYTE nFileId = rDoc.SetDocPattern( r );
    USHORT nHId = nId;      //!! SFX hat eigenmaechtig auf ULONG umgestellt!

    SwFmt* pTmpFmt = 0;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pTmpFmt = pCharFmt;    break;
    case SFX_STYLE_FAMILY_PARA : pTmpFmt = pColl;       break;
    case SFX_STYLE_FAMILY_FRAME: pTmpFmt = pFrmFmt;     break;
    case SFX_STYLE_FAMILY_PAGE :
        ((SwPageDesc*)pDesc)->SetPoolHelpId( nHId );
        ((SwPageDesc*)pDesc)->SetPoolHlpFileId( nFileId );
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        ((SwNumRule*)pNumRule)->SetPoolHelpId( nHId );
        ((SwNumRule*)pNumRule)->SetPoolHlpFileId( nFileId );
        break;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        return ;
    }
    if( pTmpFmt )
    {
        pTmpFmt->SetPoolHelpId( nHId );
        pTmpFmt->SetPoolHlpFileId( nFileId );
    }
}


/*  */

/*--------------------------------------------------------------------
    Beschreibung:   Methoden fuer den DocStyleSheetPool
 --------------------------------------------------------------------*/

SwDocStyleSheetPool::SwDocStyleSheetPool( SwDoc& rDocument, BOOL bOrg )
    : SfxStyleSheetBasePool( rDocument.GetAttrPool() ),
    aStyleSheet( rDocument, aEmptyStr, *this, SFX_STYLE_FAMILY_CHAR, 0 ),
    rDoc( rDocument )
{
    bOrganizer = bOrg;
}

 SwDocStyleSheetPool::~SwDocStyleSheetPool()
{
}

SfxStyleSheetBase&   SwDocStyleSheetPool::Make(
        const String&   rName,
        SfxStyleFamily  eFam,
        USHORT          nMask,
        USHORT          nPos )
{
    aStyleSheet.PresetName(rName);
    aStyleSheet.PresetParent(aEmptyStr);
    aStyleSheet.PresetFollow(aEmptyStr);
    aStyleSheet.SetMask(nMask) ;
    aStyleSheet.SetFamily(eFam);
    aStyleSheet.SetPhysical(TRUE);
    aStyleSheet.Create();

    return aStyleSheet;
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const SfxStyleSheetBase& rOrg)
{
    ASSERT(!this , "Create im SW-Stylesheet-Pool geht nicht" );
    return NULL;
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const String &,
                                                SfxStyleFamily, USHORT )
{
    ASSERT( !this, "Create im SW-Stylesheet-Pool geht nicht" );
    return NULL;
}

void  SwDocStyleSheetPool::Replace( SfxStyleSheetBase& rSource,
                                            SfxStyleSheetBase& rTarget )
{
    SfxStyleFamily eFamily( rSource.GetFamily() );
    if( rSource.HasParentSupport())
    {
        const String& rParentName = rSource.GetParent();
        if( 0 != rParentName.Len() )
        {
            SfxStyleSheetBase* pParentOfNew = Find( rParentName, eFamily );
            if( pParentOfNew )
                rTarget.SetParent( rParentName );
        }
    }
    if( rSource.HasFollowSupport())
    {
        const String& rFollowName = rSource.GetFollow();
        if( 0 != rFollowName.Len() )
        {
            SfxStyleSheetBase* pFollowOfNew = Find( rFollowName, eFamily );
            if( pFollowOfNew )
                rTarget.SetFollow( rFollowName );
        }
    }

    SwImplShellAction aTmpSh( rDoc );

    BOOL bSwSrcPool = GetAppName() == rSource.GetPool().GetAppName();
    if( SFX_STYLE_FAMILY_PAGE == eFamily && bSwSrcPool )
    {
        // gesondert behandeln!!
        SwPageDesc* pDestDsc =
            (SwPageDesc*)((SwDocStyleSheet&)rTarget).GetPageDesc();
        SwPageDesc* pCpyDsc =
            (SwPageDesc*)((SwDocStyleSheet&)rSource).GetPageDesc();
        rDoc.CopyPageDesc( *pCpyDsc, *pDestDsc );
    }
    else
    {
        const SwFmt *pSourceFmt = 0;
        SwFmt *pTargetFmt = 0;
        SwPageDesc *pTargetDesc = 0;
        USHORT nPgDscPos = USHRT_MAX;
        switch( eFamily )
        {
        case SFX_STYLE_FAMILY_CHAR :
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetCharFmt();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetCharFmt();
            break;
        case SFX_STYLE_FAMILY_PARA :
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetCollection();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetCollection();
            break;
        case SFX_STYLE_FAMILY_FRAME:
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetFrmFmt();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetFrmFmt();
            break;
        case SFX_STYLE_FAMILY_PAGE:
            if( bSwSrcPool )
                pSourceFmt = &((SwDocStyleSheet&)rSource).GetPageDesc()
                                ->GetMaster();
            {
                SwPageDesc *pDesc = rDoc.FindPageDescByName(
                    ((SwDocStyleSheet&)rTarget).GetPageDesc()->GetName(),
                    &nPgDscPos );

                if( pDesc )
                    pTargetFmt = &pDesc->GetMaster();
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            // Eine NumRule besteht nur aus einem Item, also muss man
            // hier nichts loeschen.
            break;
        }
        if( pTargetFmt )
        {
            if( pSourceFmt )
                pTargetFmt->DelDiffs( *pSourceFmt );
            else if( USHRT_MAX != nPgDscPos )
                pTargetFmt->ResetAttr( RES_PAGEDESC, RES_FRMATR_END-1 );
            else
                pTargetFmt->ResetAllAttr();

            if( USHRT_MAX != nPgDscPos )
                rDoc.ChgPageDesc( nPgDscPos, rDoc.GetPageDesc(nPgDscPos) );
        }
        ((SwDocStyleSheet&)rTarget).SetItemSet( rSource.GetItemSet() );
    }
}

SfxStyleSheetIterator*  SwDocStyleSheetPool::CreateIterator(
                        SfxStyleFamily eFam, USHORT nMask )
{
    return new SwStyleSheetIterator( this, eFam, nMask );
}

void   SwDocStyleSheetPool::Erase( SfxStyleSheetBase* pStyle)
{
    if( !pStyle )
        return;

    BOOL bBroadcast = TRUE;
    SwImplShellAction aTmpSh( rDoc );
    const String& rName = pStyle->GetName();
    switch( pStyle->GetFamily() )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pFmt = lcl_FindCharFmt(rDoc, rName, 0, FALSE );
            if(pFmt)
                rDoc.DelCharFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = lcl_FindParaFmt(rDoc, rName, 0, FALSE );
            if(pColl)
                rDoc.DelTxtFmtColl(pColl);
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFmt = lcl_FindFrmFmt(rDoc, rName, 0, FALSE );
            if(pFmt)
                rDoc.DelFrmFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PAGE :
        {
            USHORT nPos;
            if( rDoc.FindPageDescByName( rName, &nPos ))
                rDoc.DelPageDesc( nPos );
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            if( !rDoc.DelNumRule( rName ) )
                // Broadcast nur versenden, wenn etwas geloescht wurde
                bBroadcast = FALSE;
        }
        break;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        bBroadcast = FALSE;
    }

    if( bBroadcast )
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *pStyle ) );
}



BOOL  SwDocStyleSheetPool::SetParent( SfxStyleFamily eFam,
                                const String &rStyle, const String &rParent )
{
    SwFmt* pFmt = 0, *pParent = 0;
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( 0 != ( pFmt = lcl_FindCharFmt( rDoc, rStyle ) ) && rParent.Len() )
            pParent = lcl_FindCharFmt(rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PARA :
        if( 0 != ( pFmt = lcl_FindParaFmt( rDoc, rStyle ) ) && rParent.Len() )
            pParent = lcl_FindParaFmt( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( 0 != ( pFmt = lcl_FindFrmFmt( rDoc, rStyle ) ) && rParent.Len() )
            pParent = lcl_FindFrmFmt( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PAGE:
    case SFX_STYLE_FAMILY_PSEUDO:
        break;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
    }

    BOOL bRet = FALSE;
    if( pFmt && pFmt->DerivedFrom() &&
        pFmt->DerivedFrom()->GetName() != rParent )
    {
        {
            SwImplShellAction aTmpSh( rDoc );
            bRet = pFmt->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            // nur fuer das Broadcasting
            aStyleSheet.PresetName( rStyle );
            aStyleSheet.PresetParent( rParent );
            if( SFX_STYLE_FAMILY_PARA == eFam )
                aStyleSheet.PresetFollow( ((SwTxtFmtColl*)pFmt)->
                        GetNextTxtFmtColl().GetName() );
            else
                aStyleSheet.PresetFollow( aEmptyStr );

            Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
                                            aStyleSheet ) );
        }
    }

    return bRet;
}

SfxStyleSheetBase* SwDocStyleSheetPool::Find( const String& rName,
                                            SfxStyleFamily eFam, USHORT n )
{
    USHORT nSMask = n;
    if( SFX_STYLE_FAMILY_PARA == eFam && rDoc.IsHTMLMode() )
    {
        // dann sind nur HTML-Vorlagen von Interesse
        if( USHRT_MAX == nSMask )
            nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED;
        else
            nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
                                SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
        if( !nSMask )
            nSMask = SWSTYLEBIT_HTML;
    }

    const BOOL bSearchUsed = ( n != SFXSTYLEBIT_ALL &&
                             n & SFXSTYLEBIT_USED ) ? TRUE : FALSE;
    const SwModify* pMod = 0;

    aStyleSheet.SetPhysical( FALSE );
    aStyleSheet.PresetName( rName );
    aStyleSheet.SetFamily( eFam );
    BOOL bFnd = aStyleSheet.FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    if( aStyleSheet.IsPhysical() )
    {
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:
            pMod = aStyleSheet.GetCharFmt();
            break;

        case SFX_STYLE_FAMILY_PARA:
            pMod = aStyleSheet.GetCollection();
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pMod = aStyleSheet.GetFrmFmt();
            break;

        case SFX_STYLE_FAMILY_PAGE:
            pMod = aStyleSheet.GetPageDesc();
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                const SwNumRule* pRule = aStyleSheet.GetNumRule();
                if( pRule &&
                    !(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pRule)) ) &&
                    (( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                            ? !(pRule->GetPoolFmtId() & USER_FMT)
                                // benutzte gesucht und keine gefunden
                            : bSearchUsed ))
                    bFnd = FALSE;
            }
            break;

        default:
            ASSERT(!this, "unbekannte Style-Familie");
        }
    }

    // dann noch die Maske auswerten:
    if( pMod && !(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pMod)) ) )
    {
        const USHORT nId = SFX_STYLE_FAMILY_PAGE == eFam
                        ? ((SwPageDesc*)pMod)->GetPoolFmtId()
                        : ((SwFmt*)pMod)->GetPoolFmtId();

        if( ( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
            ? !(nId & USER_FMT)
                // benutzte gesucht und keine gefunden
            : bSearchUsed )
            bFnd = FALSE;
    }
    return bFnd ? &aStyleSheet : 0;
}

/*  */

SwStyleSheetIterator::SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                                SfxStyleFamily eFam, USHORT n )
    : SfxStyleSheetIterator( pBase, eFam, n ),
    aIterSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 ),
    aStyleSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 )
{
    bFirstCalled = FALSE;
    nLastPos = 0;
    StartListening( *pBase );
}

 SwStyleSheetIterator::~SwStyleSheetIterator()
{
    EndListening( aIterSheet.GetPool() );
}

USHORT  SwStyleSheetIterator::Count()
{
    // Liste richtig fuellen lassen !!
    if( !bFirstCalled )
        First();
    return aLst.Count();
}

SfxStyleSheetBase*  SwStyleSheetIterator::operator[]( USHORT nIdx )
{
    // gefunden
    if( !bFirstCalled )
        First();
    aStyleSheet.PresetNameAndFamily( *aLst[ nIdx ] );
    aStyleSheet.SetPhysical( FALSE );
    aStyleSheet.FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    return &aStyleSheet;
}

SfxStyleSheetBase*  SwStyleSheetIterator::First()
{
    // Alte Liste loeschen
    bFirstCalled = TRUE;
    nLastPos = 0;
    aLst.Erase();

    // aktuellen loeschen
    aIterSheet.Reset();

    SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
    const USHORT nSrchMask = nMask;
    const BOOL bSearchUsed = SearchUsed();

    const BOOL bOrganizer = ((SwDocStyleSheetPool*)pBasePool)->IsOrganizerMode();

    if( nSearchFamily == SFX_STYLE_FAMILY_CHAR
     || nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const USHORT nArrLen = rDoc.GetCharFmts()->Count();
        for( USHORT i = 0; i < nArrLen; i++ )
        {
            SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];
            if( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() )
                continue;

            const BOOL  bUsed = bSearchUsed && (bOrganizer || rDoc.IsUsed(*pFmt));
            if( !bUsed )
            {
                // Standard ist keine Benutzervorlage #46181#
                const USHORT nId = rDoc.GetDfltCharFmt() == pFmt ?
                        RES_POOLCHR_INET_NORMAL :
                                pFmt->GetPoolFmtId();
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                        // benutzte gesucht und keine gefunden
                    : bSearchUsed )
                continue;

                if( rDoc.IsHTMLMode() && !(nId & USER_FMT) &&
                    !( RES_POOLCHR_HTML_BEGIN <= nId &&
                          nId < RES_POOLCHR_HTML_END ) &&
                    RES_POOLCHR_INET_NORMAL != nId &&
                    RES_POOLCHR_INET_VISIT != nId &&
                    RES_POOLCHR_FOOTNOTE  != nId &&
                    RES_POOLCHR_ENDNOTE != nId )
                    continue;
            }

            aLst.Append( cCHAR, pFmt == rDoc.GetDfltCharFmt()
                        ? (const String&) *rDoc.GetTextNmArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ]
                        : pFmt->GetName() );
        }

        // PoolFormate
        //
        if( nSrchMask == SFXSTYLEBIT_ALL )
        {
            if( !rDoc.IsHTMLMode() )
                AppendStyleList(rDoc.GetChrFmtNmArray(),
                                bSearchUsed, GET_POOLID_CHRFMT, cCHAR);
            else
            {
                aLst.Append( cCHAR, *rDoc.GetChrFmtNmArray()[
                        RES_POOLCHR_INET_NORMAL - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *rDoc.GetChrFmtNmArray()[
                        RES_POOLCHR_INET_VISIT - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *rDoc.GetChrFmtNmArray()[
                        RES_POOLCHR_ENDNOTE - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *rDoc.GetChrFmtNmArray()[
                        RES_POOLCHR_FOOTNOTE - RES_POOLCHR_BEGIN ] );
            }
            AppendStyleList(rDoc.GetHTMLChrFmtNmArray(),
                                bSearchUsed, GET_POOLID_CHRFMT, cCHAR);
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PARA ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        USHORT nSMask = nSrchMask;
        if( rDoc.IsHTMLMode() )
        {
            // dann sind nur HTML-Vorlagen von Interesse
            if( USHRT_MAX == nSMask )
                nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF |
                            SFXSTYLEBIT_USED;
            else
                nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
                                SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
            if( !nSMask )
                nSMask = SWSTYLEBIT_HTML;
        }

        const USHORT nArrLen = rDoc.GetTxtFmtColls()->Count();
        for( USHORT i = 0; i < nArrLen; i++ )
        {
            SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];

            if(pColl->IsDefault())
                continue;

            const BOOL bUsed = bOrganizer || rDoc.IsUsed(*pColl);
            if( !(bSearchUsed && bUsed ))
            {
                const USHORT nId = pColl->GetPoolFmtId();
                switch ( (nSMask & ~SFXSTYLEBIT_USED) )
                {
                case SFXSTYLEBIT_USERDEF:
                    if(!IsPoolUserFmt(nId)) continue;
                    break;
                case SWSTYLEBIT_TEXT:
                    if((nId & COLL_GET_RANGE_BITS) != COLL_TEXT_BITS) continue;
                    break;
                case SWSTYLEBIT_CHAPTER:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_DOC_BITS) continue;
                    break;
                case SWSTYLEBIT_LIST:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_LISTS_BITS) continue;
                    break;
                case SWSTYLEBIT_IDX:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_REGISTER_BITS) continue;
                    break;
                case SWSTYLEBIT_EXTRA:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_EXTRA_BITS) continue;
                    break;

                case SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF:
                    if(IsPoolUserFmt(nId))
                        break;
                    // ansonten weiter
                case SWSTYLEBIT_HTML:
                    if( (nId  & COLL_GET_RANGE_BITS) != COLL_HTML_BITS)
                    {
                        // einige wollen wir aber auch in dieser Section sehen
                        BOOL bWeiter = TRUE;
                        switch( nId )
                        {
                        case RES_POOLCOLL_SENDADRESS:   //  --> ADDRESS
                        case RES_POOLCOLL_TABLE_HDLN:   //  --> TH
                        case RES_POOLCOLL_TABLE:        //  --> TD
                        case RES_POOLCOLL_TEXT:         //  --> P
                        case RES_POOLCOLL_HEADLINE_BASE://  --> H
                        case RES_POOLCOLL_HEADLINE1:    //  --> H1
                        case RES_POOLCOLL_HEADLINE2:    //  --> H2
                        case RES_POOLCOLL_HEADLINE3:    //  --> H3
                        case RES_POOLCOLL_HEADLINE4:    //  --> H4
                        case RES_POOLCOLL_HEADLINE5:    //  --> H5
                        case RES_POOLCOLL_HEADLINE6:    //  --> H6
                        case RES_POOLCOLL_STANDARD:     //  --> P
                        case RES_POOLCOLL_FOOTNOTE:
                        case RES_POOLCOLL_ENDNOTE:
                            bWeiter = FALSE;
                            break;
                        }
                        if( bWeiter )
                            continue;
                    }
                    break;
                case SWSTYLEBIT_CONDCOLL:
                    if( RES_CONDTXTFMTCOLL != pColl->Which() ) continue;
                    break;
                default:
                    // benutzte gesucht und keine gefunden
                    if( bSearchUsed )
                        continue;
                }
            }
            aLst.Append( cPARA, pColl->GetName() );
        }

        const BOOL bAll = nSMask == SFXSTYLEBIT_ALL;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_TEXT )
            AppendStyleList(rDoc.GetTextNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA );
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CHAPTER )
            AppendStyleList(rDoc.GetDocNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_LIST )
            AppendStyleList(rDoc.GetListsNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_IDX )
            AppendStyleList(rDoc.GetRegisterNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_EXTRA )
            AppendStyleList(rDoc.GetExtraNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CONDCOLL )
        {
            if( !bSearchUsed ||
                rDoc.IsPoolTxtCollUsed( RES_POOLCOLL_TEXT ))
                aLst.Append( cPARA, *rDoc.GetTextNmArray()[
                        RES_POOLCOLL_TEXT - RES_POOLCOLL_TEXT_BEGIN ] );
        }
        if ( bAll ||
            (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_HTML ||
            (nSMask & ~SFXSTYLEBIT_USED) ==
                        (SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF) )
        {
            AppendStyleList(rDoc.GetHTMLNmArray(),
                            bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
            if( !bAll )
            {
                // dann auch die, die wir mappen:
                static USHORT aPoolIds[] = {
                    RES_POOLCOLL_SENDADRESS,    //  --> ADDRESS
                    RES_POOLCOLL_TABLE_HDLN,    //  --> TH
                    RES_POOLCOLL_TABLE,     //  --> TD
                    RES_POOLCOLL_STANDARD,      //  --> P
                    RES_POOLCOLL_TEXT,          //  --> P
                    RES_POOLCOLL_HEADLINE_BASE, //  --> H
                    RES_POOLCOLL_HEADLINE1, //  --> H1
                    RES_POOLCOLL_HEADLINE2, //  --> H2
                    RES_POOLCOLL_HEADLINE3, //  --> H3
                    RES_POOLCOLL_HEADLINE4, //  --> H4
                    RES_POOLCOLL_HEADLINE5, //  --> H5
                    RES_POOLCOLL_HEADLINE6, //  --> H6
                    RES_POOLCOLL_FOOTNOTE,
                    RES_POOLCOLL_ENDNOTE,
                    0
                    };

                USHORT* pPoolIds = aPoolIds;
                String s;
                while( *pPoolIds )
                {
                    if( !bSearchUsed || rDoc.IsPoolTxtCollUsed( *pPoolIds ) )
                        aLst.Append( cPARA, rDoc.GetPoolNm( *pPoolIds, s ));
                    ++pPoolIds;
                }
            }
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_FRAME ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const USHORT nArrLen = rDoc.GetFrmFmts()->Count();
        for( USHORT i = 0; i < nArrLen; i++ )
        {
            SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];
            if(pFmt->IsDefault() || pFmt->IsAuto())
                continue;

            const USHORT nId = pFmt->GetPoolFmtId();
            BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(*pFmt));
            if( !bUsed )
            {
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // benutzte gesucht und keine gefunden
                    : bSearchUsed )
                    continue;
            }

            aLst.Append( cFRAME, pFmt->GetName() );
        }

        // PoolFormate
        //
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(rDoc.GetFrmFmtNmArray(),
                                    bSearchUsed, GET_POOLID_FRMFMT, cFRAME);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PAGE ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const USHORT nCount = rDoc.GetPageDescCnt();
        for(USHORT i = 0; i < nCount; ++i)
        {
            const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
            const USHORT nId = rDesc.GetPoolFmtId();
            BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(rDesc));
            if( !bUsed )
            {
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // benutzte gesucht und keine gefunden
                    : bSearchUsed )
                    continue;
            }

            aLst.Append( cPAGE, rDesc.GetName() );
        }
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(rDoc.GetPageDescNmArray(),
                            bSearchUsed, GET_POOLID_PAGEDESC, cPAGE);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PSEUDO ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
        for(USHORT i = 0; i < rNumTbl.Count(); ++i)
        {
            const SwNumRule& rRule = *rNumTbl[ i ];
            if( !rRule.IsAutoRule() )
            {
                BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(rRule) );
                if( !bUsed )
                {
                    if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                        ? !(rRule.GetPoolFmtId() & USER_FMT)
                        // benutzte gesucht und keine gefunden
                        : bSearchUsed )
                        continue;
                }

                aLst.Append( cNUMRULE, rRule.GetName() );
            }
        }
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(rDoc.GetNumRuleNmArray(),
                            bSearchUsed, GET_POOLID_NUMRULE, cNUMRULE);
    }

    if(aLst.Count() > 0)
    {
        nLastPos = USHRT_MAX;
        return Next();
    }
    return 0;
}

SfxStyleSheetBase*  SwStyleSheetIterator::Next()
{
    nLastPos++;
    if(aLst.Count() > 0 && nLastPos < aLst.Count())
    {
        aIterSheet.PresetNameAndFamily(*aLst[nLastPos]);
        aIterSheet.SetPhysical( FALSE );
        aIterSheet.SetMask( nMask );
        if(aIterSheet.pSet)
        {
            aIterSheet.pSet->ClearItem(0);
            aIterSheet.pSet= 0;
        }
        return &aIterSheet;
    }
    return 0;
}

#ifndef ENABLEUNICODE
SfxStyleSheetBase*  SwStyleSheetIterator::Find( const String& rName )
#else
SfxStyleSheetBase*  SwStyleSheetIterator::Find( const UniString& rName )
#endif
{
    // suchen
    if( !bFirstCalled )
        First();

    nLastPos = lcl_FindName( aLst, nSearchFamily, rName );
    if( USHRT_MAX != nLastPos )
    {
        // gefunden
        aStyleSheet.PresetNameAndFamily(*aLst[nLastPos]);
        // neuer Name gesetzt, also bestimme seine Daten
        aStyleSheet.FillStyleSheet( SwDocStyleSheet::FillOnlyName );
        if( !aStyleSheet.IsPhysical() )
            aStyleSheet.SetPhysical( FALSE );

        return &aStyleSheet;
    }
    return 0;
}

void SwStyleSheetIterator::AppendStyleList(const SvStringsDtor& rList,
                                            BOOL    bTestUsed,
                                            USHORT nSection, char cType )
{
    if( bTestUsed )
    {
        SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
        for ( USHORT i=0; i < rList.Count(); ++i )
        {
            BOOL bUsed = FALSE;
            USHORT nId = rDoc.GetPoolId(*rList[i], (SwGetPoolIdFromName)nSection);
            switch ( nSection )
            {
                case GET_POOLID_TXTCOLL:
                        bUsed = rDoc.IsPoolTxtCollUsed( nId );
                        break;
                case GET_POOLID_CHRFMT:
                        bUsed = rDoc.IsPoolFmtUsed( nId );
                        break;
                case GET_POOLID_FRMFMT:
                        bUsed = rDoc.IsPoolFmtUsed( nId );
                case GET_POOLID_PAGEDESC:
                        bUsed = rDoc.IsPoolPageDescUsed( nId );
                        break;
                default:
                    ASSERT( !this, "unknown PoolFmt-Id" );
            }
            if ( bUsed )
                aLst.Append( cType, *rList[i] );
        }
    }
    else
        for ( USHORT i=0; i < rList.Count(); ++i )
            aLst.Append( cType, *rList[i] );
}

void  SwStyleSheetIterator::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // suchen und aus der Anzeige-Liste entfernen !!
    if( rHint.ISA( SfxStyleSheetHint ) &&
        SFX_STYLESHEET_ERASED == ((SfxStyleSheetHint&) rHint).GetHint() )
    {
        SfxStyleSheetBase* pStyle = ((SfxStyleSheetHint&)rHint).GetStyleSheet();

        USHORT nTmpPos = lcl_FindName( aLst, pStyle->GetFamily(),
                                        pStyle->GetName() );
        if( nTmpPos < aLst.Count() )
            aLst.DeleteAndDestroy( nTmpPos );
    }
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.157  2000/09/18 16:05:11  willem.vandorp
    OpenOffice header added.

    Revision 1.156  2000/06/28 13:04:04  jp
    FillStyleSheet dont create always a new Style

    Revision 1.155  2000/06/14 13:01:00  jp
    Bug #71738#: Changes on the templatename must be dispatch

    Revision 1.154  2000/04/20 12:49:35  os
    GetName() returns String&

    Revision 1.153  2000/04/18 15:23:06  os
    UNICODE

    Revision 1.152  2000/04/11 08:01:31  os
    UNICODE

    Revision 1.151  2000/03/10 14:10:27  jp
    Bug #74104#: SetItemSet - remove Dialog-Flag from the NumRules

    Revision 1.150  2000/02/11 14:42:51  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.149  2000/01/18 17:15:14  jp
    Bug #72023#: dont create CurrShell-Object with 0-Pointer

    Revision 1.148  1999/11/15 14:22:25  jp
    Bug #69833#: GetDescription - ignore hint Para-Bckgrnd-destination

    Revision 1.147  1999/02/05 16:43:58  JP
    Task #61467#/#61014#: neu FindPageDescByName


------------------------------------------------------------------------*/


