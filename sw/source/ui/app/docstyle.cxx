/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#define _SVSTDARR_USHORTS

#include <svl/smplhint.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/numitem.hxx>
// --> OD 2008-02-13 #newlistlevelattrs#
#include <editeng/lrspitem.hxx>
// <--
#include <fmtcol.hxx>
#include <uitool.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <errhdl.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <docstyle.hxx>
#include <docary.hxx>
#include <ccoll.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <cmdid.h>
#include <swstyle.h>
#include <app.hrc>
#include <paratr.hxx>
#include <SwStyleNameMapper.hxx>
#include <svl/cjkoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <numrule.hxx>
#include <fmthdft.hxx>
#include <svx/svxids.hrc>
// --> OD 2008-02-12 #newlistlevelattrs#
#include <SwRewriter.hxx>
// <--

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
// sal_True, werden die Pool-Formatnamen NICHT mit eingetragen.

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
                            sal_Bool bCreate = sal_True )
{
    SwCharFmt*  pFmt = 0;
    if( rName.Len() )
    {
        pFmt = rDoc.FindCharFmtByName( rName );
        if( !pFmt && rName == *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] )
        {
            // Standard-Zeichenvorlage
            pFmt = (SwCharFmt*)rDoc.GetDfltCharFmt();
        }

        if( !pFmt && bCreate )
        {   // Pool abklappern
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetCharFmtFromPool(nId);
        }
    }
    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(sal_True);
            SwFmt* p = pFmt->DerivedFrom();
            if( p && !p->IsDefault() )
                pStyle->PresetParent( p->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
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
                                sal_Bool bCreate = sal_True )
{
    SwTxtFmtColl*   pColl = 0;

    if( rName.Len() )
    {
        pColl = rDoc.FindTxtFmtCollByName( rName );
        if( !pColl && bCreate )
        {   // Pool abklappern
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                pColl = rDoc.GetTxtCollFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pColl)
        {
            pStyle->SetPhysical(sal_True);
            if( pColl->DerivedFrom() && !pColl->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pColl->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );

            SwTxtFmtColl& rNext = pColl->GetNextTxtFmtColl();
            pStyle->PresetFollow(rNext.GetName());
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pColl;
}


/*--------------------------------------------------------------------
    Beschreibung:   Rahmenformate
 --------------------------------------------------------------------*/


SwFrmFmt* lcl_FindFrmFmt(   SwDoc& rDoc,
                            const String& rName,
                            SwDocStyleSheet* pStyle = 0,
                            sal_Bool bCreate = sal_True )
{
    SwFrmFmt* pFmt = 0;
    if( rName.Len() )
    {
        pFmt = rDoc.FindFrmFmtByName( rName );
        if( !pFmt && bCreate )
        {   // Pool abklappern
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetFrmFmtFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(sal_True);
            if( pFmt->DerivedFrom() && !pFmt->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pFmt->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pFmt;
}

/*--------------------------------------------------------------------
    Beschreibung:   Seitendescriptoren
 --------------------------------------------------------------------*/


const SwPageDesc* lcl_FindPageDesc( SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    sal_Bool bCreate = sal_True )
{
    const SwPageDesc* pDesc = 0;

    if( rName.Len() )
    {
        pDesc = rDoc.FindPageDescByName( rName );
        if( !pDesc && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
            if(nId != USHRT_MAX)
                pDesc = rDoc.GetPageDescFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pDesc)
        {
            pStyle->SetPhysical(sal_True);
            if(pDesc->GetFollow())
                pStyle->PresetFollow(pDesc->GetFollow()->GetName());
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pDesc;
}

const SwNumRule* lcl_FindNumRule(   SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    sal_Bool bCreate = sal_True )
{
    const SwNumRule* pRule = 0;

    if( rName.Len() )
    {
        pRule = rDoc.FindNumRulePtr( rName );
        if( !pRule && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE);
            if(nId != USHRT_MAX)
                pRule = rDoc.GetNumRuleFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pRule)
        {
            pStyle->SetPhysical(sal_True);
            pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pRule;
}


sal_uInt16 lcl_FindName( const SwPoolFmtList& rLst, SfxStyleFamily eFam,
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
        default:; //prevent warning
        }
        sSrch += rName;
        for( sal_uInt16 i=0; i < rLst.Count(); ++i )
            if( *rLst[i] == sSrch )
                return i;
    }
    return USHRT_MAX;
}

sal_Bool FindPhyStyle( SwDoc& rDoc, const String& rName, SfxStyleFamily eFam )
{
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        return 0 != lcl_FindCharFmt( rDoc, rName, 0, sal_False );
    case SFX_STYLE_FAMILY_PARA :
        return 0 != lcl_FindParaFmt( rDoc, rName, 0, sal_False );
    case SFX_STYLE_FAMILY_FRAME:
        return 0 != lcl_FindFrmFmt( rDoc, rName, 0, sal_False );
    case SFX_STYLE_FAMILY_PAGE :
        return 0 != lcl_FindPageDesc( rDoc, rName, 0, sal_False );
    case SFX_STYLE_FAMILY_PSEUDO:
        return 0 != lcl_FindNumRule( rDoc, rName, 0, sal_False );
    default:; //prevent warning
    }
    return sal_False;
}


/*--------------------------------------------------------------------
    Beschreibung:   Einfuegen von Strings in die Liste der Vorlagen
 --------------------------------------------------------------------*/


void SwPoolFmtList::Append( char cChar, const String& rStr )
{
    String* pStr = new String( cChar );
    *pStr += rStr;
    for ( sal_uInt16 i=0; i < Count(); ++i )
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
                                    SwDocStyleSheetPool&    _rPool,
                                    SfxStyleFamily          eFam,
                                    sal_uInt16                  _nMask) :

    SfxStyleSheetBase( rName, _rPool, eFam, _nMask ),
    pCharFmt(0),
    pColl(0),
    pFrmFmt(0),
    pDesc(0),
    pNumRule(0),

    rDoc(rDocument),
    aCoreSet(GetPool().GetPool(),
            RES_CHRATR_BEGIN,       RES_CHRATR_END - 1,
            RES_PARATR_BEGIN,       RES_PARATR_END - 1,
            // --> OD 2008-02-25 #refactorlists#
            RES_PARATR_LIST_BEGIN,  RES_PARATR_LIST_END - 1,
            // <--
            RES_FRMATR_BEGIN,       RES_FRMATR_END - 1,
            RES_UNKNOWNATR_BEGIN,   RES_UNKNOWNATR_END-1,
            SID_ATTR_PAGE,          SID_ATTR_PAGE_EXT1,
            SID_ATTR_PAGE_HEADERSET,SID_ATTR_PAGE_FOOTERSET,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            FN_PARAM_FTN_INFO,      FN_PARAM_FTN_INFO,
            SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_MODEL,
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,
            SID_SWREGISTER_MODE,    SID_SWREGISTER_MODE,
            SID_SWREGISTER_COLLECTION, SID_SWREGISTER_COLLECTION,
            FN_COND_COLL,           FN_COND_COLL,
            SID_ATTR_AUTO_STYLE_UPDATE, SID_ATTR_AUTO_STYLE_UPDATE,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            SID_PARA_BACKGRND_DESTINATION,  SID_ATTR_BRUSH_CHAR,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            0),
    bPhysical(sal_False)
{
    nHelpId = UCHAR_MAX;
}


SwDocStyleSheet::SwDocStyleSheet( const SwDocStyleSheet& rOrg) :
    SfxStyleSheetBase(rOrg),
    pCharFmt(rOrg.pCharFmt),
    pColl(rOrg.pColl),
    pFrmFmt(rOrg.pFrmFmt),
    pDesc(rOrg.pDesc),
    pNumRule(rOrg.pNumRule),
    rDoc(rOrg.rDoc),
    aCoreSet(rOrg.aCoreSet),
    bPhysical(rOrg.bPhysical)
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
    SetPhysical(sal_False);
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
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFmt = rDoc.FindTxtFmtCollByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL;
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFmt = rDoc.FindFrmFmtByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_FRMFMT;
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
        default:
            return aEmptyStr;       // es gibt keinen Parent
        }

        String sTmp;
        if( !pFmt )         // noch nicht vorhanden, also dflt. Parent
        {
            sal_uInt16 i = SwStyleNameMapper::GetPoolIdFromUIName( aName, eGetType );
            i = ::GetPoolParent( i );
            if( i && USHRT_MAX != i )
                SwStyleNameMapper::FillUIName( i, sTmp );
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


sal_Bool  SwDocStyleSheet::HasFollowSupport() const
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_PAGE : return sal_True;
        case SFX_STYLE_FAMILY_FRAME:
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PSEUDO: return sal_False;
        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }
    return sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:   Parent ?
 --------------------------------------------------------------------*/


sal_Bool  SwDocStyleSheet::HasParentSupport() const
{
    sal_Bool bRet = sal_False;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_FRAME: bRet = sal_True;
        default:; //prevent warning
    }
    return bRet;
}


sal_Bool  SwDocStyleSheet::HasClearParentSupport() const
{
    sal_Bool bRet = sal_False;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_FRAME: bRet = sal_True;
        default:; //prevent warning
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   textuelle Beschreibung ermitteln
 --------------------------------------------------------------------*/
String  SwDocStyleSheet::GetDescription(SfxMapUnit eUnit)
{
    IntlWrapper aIntlWrapper(
        ::comphelper::getProcessServiceFactory(),
        SvtSysLocale().GetLocaleData().getLocale());

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
                                eUnit, aItemPresentation, &aIntlWrapper ) )
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
        sal_Bool bHasWesternFontPrefix = sal_False;
        sal_Bool bHasCJKFontPrefix = sal_False;
        SvtCJKOptions aCJKOptions;

        while ( pItem )
        {
            if(!IsInvalidItem(pItem))
                switch ( pItem->Which() )
                {
                    case SID_ATTR_AUTO_STYLE_UPDATE:
                    case SID_PARA_BACKGRND_DESTINATION:
                    case RES_PAGEDESC:
                    //CTL no yet supported
                    case RES_CHRATR_CTL_FONT:
                    case RES_CHRATR_CTL_FONTSIZE:
                    case RES_CHRATR_CTL_LANGUAGE:
                    case RES_CHRATR_CTL_POSTURE:
                    case RES_CHRATR_CTL_WEIGHT:
                        break;
                    default:
                    {
                        String aItemPresentation;
                        if ( !IsInvalidItem( pItem ) &&
                             rPool.GetPool().GetPresentation(
                                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                                eUnit, aItemPresentation, &aIntlWrapper ) )
                        {
                            sal_Bool bIsDefault = sal_False;
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
                                case RES_CHRATR_CJK_FONT:
                                case RES_CHRATR_CJK_FONTSIZE:
                                case RES_CHRATR_CJK_LANGUAGE:
                                case RES_CHRATR_CJK_POSTURE:
                                case RES_CHRATR_CJK_WEIGHT:
                                if(aCJKOptions.IsCJKFontEnabled())
                                    bIsDefault = sal_True;
                                if(!bHasCJKFontPrefix)
                                {
                                    aItemPresentation.Insert(SW_RESSTR(STR_CJK_FONT), 0);
                                    bHasCJKFontPrefix = sal_True;
                                }
                                break;
                                case RES_CHRATR_FONT:
                                case RES_CHRATR_FONTSIZE:
                                case RES_CHRATR_LANGUAGE:
                                case RES_CHRATR_POSTURE:
                                case RES_CHRATR_WEIGHT:
                                if(!bHasWesternFontPrefix)
                                {
                                    aItemPresentation.Insert(SW_RESSTR(STR_WESTERN_FONT), 0);
                                    bHasWesternFontPrefix = sal_True;
                                    bIsDefault = sal_True;
                                }
                                // no break;
                                default:
                                    bIsDefault = sal_True;
                            }
                            if(bIsDefault)
                            {
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


sal_Bool  SwDocStyleSheet::SetName( const String& rStr)
{
    if( !rStr.Len() )
        return sal_False;

    if( aName != rStr )
    {
        if( !SfxStyleSheetBase::SetName( rStr ))
            return sal_False;
    }
    else if(!bPhysical)
        FillStyleSheet( FillPhysical );

    int bChg = sal_False;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        {
            ASSERT(pCharFmt, "SwCharFormat fehlt!");
            if( pCharFmt && pCharFmt->GetName() != rStr )
            {
                pCharFmt->SetName( rStr );
                bChg = sal_True;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PARA :
        {
            ASSERT(pColl, "Collektion fehlt!");
            if( pColl && pColl->GetName() != rStr )
            {
                if (pColl->GetName().Len() > 0)
                    rDoc.RenameFmt(*pColl, rStr);
                else
                    pColl->SetName(rStr);

                bChg = sal_True;
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            ASSERT(pFrmFmt, "FrmFmt fehlt!");
            if( pFrmFmt && pFrmFmt->GetName() != rStr )
            {
                if (pFrmFmt->GetName().Len() > 0)
                    rDoc.RenameFmt(*pFrmFmt, rStr);
                else
                    pFrmFmt->SetName( rStr );

                bChg = sal_True;
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
                // -> #116530#
                SwPageDesc aPageDesc(*((SwPageDesc*)pDesc));
                String aOldName(aPageDesc.GetName());

                aPageDesc.SetName( rStr );
                bool const bDoesUndo = rDoc.GetIDocumentUndoRedo().DoesUndo();

                rDoc.GetIDocumentUndoRedo().DoUndo(aOldName.Len() > 0);
                rDoc.ChgPageDesc(aOldName, aPageDesc);
                rDoc.GetIDocumentUndoRedo().DoUndo(bDoesUndo);
                // <- #116530#

                rDoc.SetModified();
                bChg = sal_True;
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            ASSERT(pNumRule, "NumRule fehlt!");

            // -> #106897#
            if (pNumRule)
            {
                String aOldName = pNumRule->GetName();

                if (aOldName.Len() > 0)
                {
                    if ( aOldName != rStr &&
                         rDoc.RenameNumRule(aOldName, rStr))
                    {
                        pNumRule = rDoc.FindNumRulePtr(rStr);
                        rDoc.SetModified();

                        bChg = sal_True;
                    }
                }
                else
                {
                    // --> OD 2008-07-08 #i91400#
                    ((SwNumRule*)pNumRule)->SetName( rStr, rDoc );
                    // <--
                    rDoc.SetModified();

                    bChg = sal_True;
                }
            }
            // <- #106897#

            break;

        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }

    if( bChg )
    {
        rPool.First();      // interne Liste muss geupdatet werden
        rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung:   Ableitungshirachie
 --------------------------------------------------------------------*/


sal_Bool   SwDocStyleSheet::SetParent( const String& rStr)
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

    sal_Bool bRet = sal_False;
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


sal_Bool   SwDocStyleSheet::SetFollow( const String& rStr)
{
    if( rStr.Len() && !SfxStyleSheetBase::SetFollow( rStr ))
        return sal_False;

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
            sal_uInt16 nId;
            if( pFollowDesc != pDesc->GetFollow() &&
                rDoc.FindPageDescByName( pDesc->GetName(), &nId ) )
            {
                SwPageDesc aDesc( *pDesc );
                aDesc.SetFollow( pFollowDesc );
                rDoc.ChgPageDesc( nId, aDesc );
                pDesc = &const_cast<const SwDoc &>(rDoc).GetPageDesc( nId );
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

    return sal_True;
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
                SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                aBoxInfo.SetTable( sal_False );
                aBoxInfo.SetDist( sal_True);    // Abstandsfeld immer anzeigen
                aBoxInfo.SetMinDist( sal_True );// Minimalgroesse in Tabellen und Absaetzen setzen
                aBoxInfo.SetDefDist( MIN_BORDER_DIST );// Default-Abstand immer setzen
                    // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
                aBoxInfo.SetValid( VALID_DISABLE, sal_True );
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

        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }
    // Member der Basisklasse
    pSet = &aCoreSet;

    return aCoreSet;
}

// --> OD 2008-02-13 #newlistlevelattrs#
void SwDocStyleSheet::MergeIndentAttrsOfListStyle( SfxItemSet& rSet )
{
    if ( nFamily != SFX_STYLE_FAMILY_PARA )
    {
        return;
    }

    ASSERT( pColl, "<SwDocStyleSheet::MergeIndentAttrsOfListStyle(..)> - missing paragraph style");
    if ( pColl->AreListLevelIndentsApplicable() )
    {
        ASSERT( pColl->GetItemState( RES_PARATR_NUMRULE ) == SFX_ITEM_SET,
                "<SwDocStyleSheet::MergeIndentAttrsOfListStyle(..)> - list level indents are applicable at paragraph style, but no list style found. Serious defect -> please inform OD." );
        const String sNumRule = pColl->GetNumRule().GetValue();
        if( sNumRule.Len() )
        {
            const SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
            if( pRule )
            {
                const SwNumFmt& rFmt = pRule->Get( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    SvxLRSpaceItem aLR( RES_LR_SPACE );
                    aLR.SetTxtLeft( rFmt.GetIndentAt() );
                    aLR.SetTxtFirstLineOfst( static_cast<short>(rFmt.GetFirstLineIndent()) );
                    rSet.Put( aLR );
                }
            }
        }
    }
}
// <--

/*--------------------------------------------------------------------
    Beschreibung:   ItemSet setzen
 --------------------------------------------------------------------*/

// --> OD 2008-02-12 #newlistlevelattrs#
// handling of parameter <bResetIndentAttrsAtParagraphStyle>
void SwDocStyleSheet::SetItemSet( const SfxItemSet& rSet,
                                  const bool bResetIndentAttrsAtParagraphStyle )
{
    // gegebenenfalls Format erst ermitteln
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    SwImplShellAction aTmpSh( rDoc );

    ASSERT( &rSet != &aCoreSet, "SetItemSet mit eigenem Set ist nicht erlaubt" );

    // --> OD 2008-02-12 #newlistlevelattrs#
    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        SwRewriter aRewriter;
        aRewriter.AddRule( UNDO_ARG1, GetName() );
        rDoc.GetIDocumentUndoRedo().StartUndo( UNDO_INSFMTATTR, &aRewriter );
    }
    // <--

    SwFmt* pFmt = 0;
    SwPageDesc* pNewDsc = 0;
    sal_uInt16 nPgDscPos = 0;

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
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,sal_False, &pAutoUpdate ))
            {
                pColl->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
            }

            const SwCondCollItem* pCondItem;
            if( SFX_ITEM_SET != rSet.GetItemState( FN_COND_COLL, sal_False,
                (const SfxPoolItem**)&pCondItem ))
                pCondItem = 0;

            if( RES_CONDTXTFMTCOLL == pColl->Which() && pCondItem )
            {
                SwFmt* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for(sal_uInt16 i = 0; i < COND_COMMAND_COUNT; i++)
                {
                    SwCollCondition aCond( 0, pCmds[ i ].nCnd, pCmds[ i ].nSubCond );
                    ((SwConditionTxtFmtColl*)pColl)->RemoveCondition( aCond );
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, sal_True )))
                    {
                        aCond.RegisterToFormat( *pFindFmt );
                        ((SwConditionTxtFmtColl*)pColl)->InsertCondition( aCond );
                    }
                }

                // Document auf die neue Bedingungen updaten
                SwCondCollCondChg aMsg( pColl );
                pColl->ModifyNotification( &aMsg, &aMsg );
            }
            else if( pCondItem && !pColl->GetDepends() )
            {
                // keine bedingte Vorlage, dann erstmal erzeugen und
                // alle wichtigen Werte uebernehmen
                SwConditionTxtFmtColl* pCColl = rDoc.MakeCondTxtFmtColl(
                        pColl->GetName(), (SwTxtFmtColl*)pColl->DerivedFrom() );
                if( pColl != &pColl->GetNextTxtFmtColl() )
                    pCColl->SetNextTxtFmtColl( pColl->GetNextTxtFmtColl() );

                //pCColl->SetOutlineLevel( pColl->GetOutlineLevel() );//#outline level,zhaojianwei
                if( pColl->IsAssignedToListLevelOfOutlineStyle())
                    pCColl->AssignToListLevelOfOutlineStyle(pColl->GetAssignedOutlineStyleLevel());
                else
                    pCColl->DeleteAssignmentToListLevelOfOutlineStyle();//<--end,zhaojianwei



                SwTxtFmtColl* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for( sal_uInt16 i = 0; i < COND_COMMAND_COUNT; ++i )
                {
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, sal_True )))
                    {
                        pCColl->InsertCondition( SwCollCondition( pFindFmt,
                                    pCmds[ i ].nCnd, pCmds[ i ].nSubCond ) );
                    }
                }

                rDoc.DelTxtFmtColl( pColl );
                pColl = pCColl;
            }
            // --> OD 2008-02-12 #newlistlevelattrs#
            if ( bResetIndentAttrsAtParagraphStyle &&
                 rSet.GetItemState( RES_PARATR_NUMRULE, sal_False, 0 ) == SFX_ITEM_SET &&
                 rSet.GetItemState( RES_LR_SPACE, sal_False, 0 ) != SFX_ITEM_SET &&
                 pColl->GetItemState( RES_LR_SPACE, sal_False, 0 ) == SFX_ITEM_SET )
            {
                rDoc.ResetAttrAtFormat( RES_LR_SPACE, *pColl );
            }
            // <--

            // #i56252: If a standard numbering style is assigned to a standard paragraph style
            // we have to create a physical instance of the numbering style. If we do not and
            // neither the paragraph style nor the numbering style is used in the document
            // the numbering style will not be saved with the document and the assignment got lost.
            const SfxPoolItem* pNumRuleItem = 0;
            if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, sal_False, &pNumRuleItem ) )
            {   // Setting a numbering rule?
                String sNumRule = ((SwNumRuleItem*)pNumRuleItem)->GetValue();
                if( sNumRule.Len() )
                {
                    SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
                    if( !pRule )
                    {   // Numbering rule not in use yet.
                        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sNumRule, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
                        if( USHRT_MAX != nPoolId ) // It's a standard numbering rule
                        {
                            pRule = rDoc.GetNumRuleFromPool( nPoolId ); // Create numbering rule (physical)
                        }
                    }
                }
            }

            pFmt = pColl;

            sal_uInt16 nId = pColl->GetPoolFmtId() &
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
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,sal_False, &pAutoUpdate ))
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
                    // --> OD 2005-05-09 #i48949# - no undo actions for the
                    // copy of the page style
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.CopyPageDesc(*pDesc, *pNewDsc); // #i7983#
                    // <--

                    pFmt = &pNewDsc->GetMaster();
                }
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                ASSERT(pNumRule, "Wo ist die NumRule");

                if (!pNumRule)
                    break;

                const SfxPoolItem* pItem;
                switch( rSet.GetItemState( SID_ATTR_NUMBERING_RULE, sal_False, &pItem ))
                {
                case SFX_ITEM_SET:
                {
                    SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                    pSetRule->UnLinkGraphics();
                    //SwNumRule aSetRule(rDoc.GetUniqueNumRuleName());
                    SwNumRule aSetRule(*pNumRule);
                    aSetRule.SetSvxRule(*pSetRule, &rDoc);
                    rDoc.ChgNumRuleFmts( aSetRule );
                }
                break;
                case SFX_ITEM_DONTCARE:
                    // NumRule auf default Werte
                    // was sind die default Werte?
                    {
                        // --> OD 2008-02-11 #newlistlevelattrs#
                        SwNumRule aRule( pNumRule->GetName(),
                                         // --> OD 2008-06-06 #i89178#
                                         numfunc::GetDefaultPositionAndSpaceMode() );
                                         // <--
                        // <--
                        rDoc.ChgNumRuleFmts( aRule );
                    }
                    break;
                }
            }
            break;

        default:
            ASSERT(!this, "unbekannte Style-Familie");
    }

    if( pFmt && rSet.Count())
    {
        SfxItemIter aIter( rSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( sal_True )
        {
            if( IsInvalidItem( pItem ) )            // Clearen
            {
                // --> OD 2008-02-12 #newlistlevelattrs#
                // use method <SwDoc::ResetAttrAtFormat(..)> in order to
                // create an Undo object for the attribute reset.
//                pFmt->ResetAttr( rSet.GetWhichByPos(aIter.GetCurPos()));
                rDoc.ResetAttrAtFormat( rSet.GetWhichByPos(aIter.GetCurPos()),
                                        *pFmt );
            }

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
        SfxItemSet aSet(rSet);
        aSet.ClearInvalidItems();

        aCoreSet.ClearItem();

        if( pNewDsc )
        {
            ::ItemSetToPageDesc( aSet, *pNewDsc );
            rDoc.ChgPageDesc( nPgDscPos, *pNewDsc );
            pDesc = &const_cast<const SwDoc &>(rDoc).GetPageDesc( nPgDscPos );
            rDoc.PreDelPageDesc(pNewDsc); // #i7983#
            delete pNewDsc;
        }
        else
            rDoc.ChgFmt(*pFmt, aSet);       // alles gesetzten Putten
    }
    else
    {
        aCoreSet.ClearItem();
        if( pNewDsc )           // den muessen wir noch vernichten!!
        {
            rDoc.PreDelPageDesc(pNewDsc); // #i7983#
            delete pNewDsc;
        }
    }

    // --> OD 2008-02-12 #newlistlevelattrs#
    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
    }
    // <--
}

void lcl_SaveStyles( sal_uInt16 nFamily, SvPtrarr& rArr, SwDoc& rDoc )
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
                void* p =
                    (void*)&const_cast<const SwDoc &>(rDoc).GetPageDesc( n );
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

void lcl_DeleteInfoStyles( sal_uInt16 nFamily, SvPtrarr& rArr, SwDoc& rDoc )
{
    sal_uInt16 n, nCnt;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SvUShorts aDelArr;
            const SwCharFmts& rTbl = *rDoc.GetCharFmts();
            for( n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
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
            for( n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
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
            for( n = 0, nCnt = rTbl.Count(); n < nCnt; ++n )
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
                void* p =
                    (void*)&const_cast<const SwDoc &>(rDoc).GetPageDesc( n );
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

sal_Bool SwDocStyleSheet::FillStyleSheet( FillStyleType eFType )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nPoolId = USHRT_MAX;
    SwFmt* pFmt = 0;

    sal_Bool bCreate = FillPhysical == eFType;
    sal_Bool bDeleteInfo = sal_False;
    sal_Bool bFillOnlyInfo = FillAllInfo == eFType;
    SvPtrarr aDelArr;

    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_CHAR:
        pCharFmt = lcl_FindCharFmt(rDoc, aName, this, bCreate );
        bPhysical = 0 != pCharFmt;
        if( bFillOnlyInfo && !bPhysical )
        {
            bDeleteInfo = sal_True;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pCharFmt = lcl_FindCharFmt(rDoc, aName, this, sal_True );
        }

        pFmt = pCharFmt;
        if( !bCreate && !pFmt )
        {
            if( aName == *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                            RES_POOLCOLL_TEXT_BEGIN ] )
                nPoolId = 0;
            else
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
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
                bDeleteInfo = sal_True;
                ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
                pColl = lcl_FindParaFmt(rDoc, aName, this, sal_True );
            }

            pFmt = pColl;
            if( pColl )
                PresetFollow( pColl->GetNextTxtFmtColl().GetName() );
            else if( !bCreate )
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );

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
            bDeleteInfo = sal_True;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pFrmFmt = lcl_FindFrmFmt(rDoc, aName, this, sal_True );
        }
        pFmt = pFrmFmt;
        if( !bCreate && !pFmt )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );

        bRet = 0 != pFrmFmt || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pFrmFmt = 0;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        pDesc = lcl_FindPageDesc(rDoc, aName, this, bCreate);
        bPhysical = 0 != pDesc;
        if( bFillOnlyInfo && !pDesc )
        {
            bDeleteInfo = sal_True;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pDesc = lcl_FindPageDesc( rDoc, aName, this, sal_True );
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
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
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
            bDeleteInfo = sal_True;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pNumRule = lcl_FindNumRule( rDoc, aName, this, sal_True );
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
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = 0 != pNumRule || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pNumRule = 0;
        break;
        default:; //prevent warning
    }

    if( SFX_STYLE_FAMILY_CHAR == nFamily ||
        SFX_STYLE_FAMILY_PARA == nFamily ||
        SFX_STYLE_FAMILY_FRAME == nFamily )
    {
        if( pFmt )
            nPoolId = pFmt->GetPoolFmtId();

        sal_uInt16 _nMask = 0;
        if( pFmt == rDoc.GetDfltCharFmt() )
            _nMask |= SFXSTYLEBIT_READONLY;
        else if( USER_FMT & nPoolId )
            _nMask |= SFXSTYLEBIT_USERDEF;

        switch ( COLL_GET_RANGE_BITS & nPoolId )
        {
        case COLL_TEXT_BITS:     _nMask |= SWSTYLEBIT_TEXT;   break;
        case COLL_DOC_BITS :     _nMask |= SWSTYLEBIT_CHAPTER; break;
        case COLL_LISTS_BITS:    _nMask |= SWSTYLEBIT_LIST;   break;
        case COLL_REGISTER_BITS: _nMask |= SWSTYLEBIT_IDX;    break;
        case COLL_EXTRA_BITS:    _nMask |= SWSTYLEBIT_EXTRA;      break;
        case COLL_HTML_BITS:     _nMask |= SWSTYLEBIT_HTML;   break;
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
                _nMask |= SWSTYLEBIT_CONDCOLL;
        }

        SetMask( _nMask );
    }
    if( bDeleteInfo && bFillOnlyInfo )
        ::lcl_DeleteInfoStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Neues Format in der Core anlegen
 --------------------------------------------------------------------*/


void SwDocStyleSheet::Create()
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            pCharFmt = lcl_FindCharFmt( rDoc, aName );
            if( !pCharFmt )
                pCharFmt = rDoc.MakeCharFmt(aName,
                                            rDoc.GetDfltCharFmt());
            pCharFmt->SetAuto( sal_False );
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
                pFrmFmt = rDoc.MakeFrmFmt(aName, rDoc.GetDfltFrmFmt(), sal_False, sal_False);

            break;

        case SFX_STYLE_FAMILY_PAGE :
            pDesc = lcl_FindPageDesc( rDoc, aName );
            if( !pDesc )
            {
                sal_uInt16 nId = rDoc.MakePageDesc(aName);
                pDesc = &const_cast<const SwDoc &>(rDoc).GetPageDesc(nId);
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

                // --> OD 2008-02-11 #newlistlevelattrs#
                SwNumRule* pRule = rDoc.GetNumRuleTbl()[
                    rDoc.MakeNumRule( sTmpNm, 0, sal_False,
                                      // --> OD 2008-06-06 #i89178#
                                      numfunc::GetDefaultPositionAndSpaceMode() ) ];
                                      // <--
                // <--
                pRule->SetAutoRule( sal_False );
                if( !aName.Len() )
                {
                    // --> OD 2008-07-08 #i91400#
                    pRule->SetName( aName, rDoc );
                    // <--
                }
                pNumRule = pRule;
            }
            break;
        default:; //prevent warning
    }
    bPhysical = sal_True;
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
    DBG_ASSERT(pNumRule, "Wo ist die NumRule");
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


void SwDocStyleSheet::SetPhysical(sal_Bool bPhys)
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


sal_Bool  SwDocStyleSheet::IsUsed() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->FillStyleSheet( FillOnlyName );
    }

    // immer noch nicht ?
    if( !bPhysical )
        return sal_False;

    const SwModify* pMod;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pMod = pCharFmt;   break;
    case SFX_STYLE_FAMILY_PARA : pMod = pColl;      break;
    case SFX_STYLE_FAMILY_FRAME: pMod = pFrmFmt;    break;
    case SFX_STYLE_FAMILY_PAGE : pMod = pDesc;      break;

    case SFX_STYLE_FAMILY_PSEUDO:
            return pNumRule ? rDoc.IsUsed( *pNumRule ) : sal_False;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        return sal_False;
    }
    return rDoc.IsUsed( *pMod );
}


sal_uLong  SwDocStyleSheet::GetHelpId( String& rFile )
{
static String sTemplateHelpFile = String::CreateFromAscii("swrhlppi.hlp");

    sal_uInt16 nId = 0;
    sal_uInt16 nPoolId = 0;
    unsigned char nFileId = UCHAR_MAX;

    rFile = sTemplateHelpFile;

    const SwFmt* pTmpFmt = 0;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( !pCharFmt &&
            0 == (pCharFmt = lcl_FindCharFmt( rDoc, aName, 0, sal_False )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pCharFmt;
        break;

    case SFX_STYLE_FAMILY_PARA:
        if( !pColl &&
            0 == ( pColl = lcl_FindParaFmt( rDoc, aName, 0, sal_False )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pColl;
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( !pFrmFmt &&
            0 == ( pFrmFmt = lcl_FindFrmFmt( rDoc, aName, 0, sal_False ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pFrmFmt;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        if( !pDesc &&
            0 == ( pDesc = lcl_FindPageDesc( rDoc, aName, 0, sal_False ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pDesc->GetPoolHelpId();
        nFileId = pDesc->GetPoolHlpFileId();
        nPoolId = pDesc->GetPoolFmtId();
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        if( !pNumRule &&
            0 == ( pNumRule = lcl_FindNumRule( rDoc, aName, 0, sal_False ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
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


void  SwDocStyleSheet::SetHelpId( const String& r, sal_uLong nId )
{
    sal_uInt8 nFileId = static_cast< sal_uInt8 >(rDoc.SetDocPattern( r ));
    sal_uInt16 nHId = static_cast< sal_uInt16 >(nId);     //!! SFX hat eigenmaechtig auf sal_uLong umgestellt!

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

SwDocStyleSheetPool::SwDocStyleSheetPool( SwDoc& rDocument, sal_Bool bOrg )
: SfxStyleSheetBasePool( rDocument.GetAttrPool() )
, mxStyleSheet( new SwDocStyleSheet( rDocument, aEmptyStr, *this, SFX_STYLE_FAMILY_CHAR, 0 ) )
, rDoc( rDocument )
{
    bOrganizer = bOrg;
}

 SwDocStyleSheetPool::~SwDocStyleSheetPool()
{
}

void SAL_CALL SwDocStyleSheetPool::acquire(  ) throw ()
{
    comphelper::OWeakTypeObject::acquire();
}

void SAL_CALL SwDocStyleSheetPool::release(  ) throw ()
{
    comphelper::OWeakTypeObject::release();
}

SfxStyleSheetBase&   SwDocStyleSheetPool::Make(
        const String&   rName,
        SfxStyleFamily  eFam,
        sal_uInt16          _nMask,
        sal_uInt16          /*nPos*/ )
{
    mxStyleSheet->PresetName(rName);
    mxStyleSheet->PresetParent(aEmptyStr);
    mxStyleSheet->PresetFollow(aEmptyStr);
    mxStyleSheet->SetMask(_nMask) ;
    mxStyleSheet->SetFamily(eFam);
    mxStyleSheet->SetPhysical(sal_True);
    mxStyleSheet->Create();

    return *mxStyleSheet.get();
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const SfxStyleSheetBase& /*rOrg*/)
{
    ASSERT(!this , "Create im SW-Stylesheet-Pool geht nicht" );
    return NULL;
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const String &,
                                                SfxStyleFamily, sal_uInt16 )
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

    sal_Bool bSwSrcPool = GetAppName() == rSource.GetPool().GetAppName();
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
        sal_uInt16 nPgDscPos = USHRT_MAX;
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
        default:; //prevent warning
        }
        if( pTargetFmt )
        {
            if( pSourceFmt )
                pTargetFmt->DelDiffs( *pSourceFmt );
            else if( USHRT_MAX != nPgDscPos )
                pTargetFmt->ResetFmtAttr( RES_PAGEDESC, RES_FRMATR_END-1 );
            else
            {
                // --> OD 2007-01-25 #i73790# - method renamed
                pTargetFmt->ResetAllFmtAttr();
                // <--
            }

            if( USHRT_MAX != nPgDscPos )
                rDoc.ChgPageDesc( nPgDscPos,
                                  const_cast<const SwDoc &>(rDoc).
                                  GetPageDesc(nPgDscPos) );
        }
        ((SwDocStyleSheet&)rTarget).SetItemSet( rSource.GetItemSet() );
    }
}

SfxStyleSheetIterator*  SwDocStyleSheetPool::CreateIterator(
                        SfxStyleFamily eFam, sal_uInt16 _nMask )
{
    return new SwStyleSheetIterator( this, eFam, _nMask );
}

void SwDocStyleSheetPool::dispose()
{
    mxStyleSheet.clear();
}

void SwDocStyleSheetPool::Remove( SfxStyleSheetBase* pStyle)
{
    if( !pStyle )
        return;

    sal_Bool bBroadcast = sal_True;
    SwImplShellAction aTmpSh( rDoc );
    const String& rName = pStyle->GetName();
    switch( pStyle->GetFamily() )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pFmt = lcl_FindCharFmt(rDoc, rName, 0, sal_False );
            if(pFmt)
                rDoc.DelCharFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = lcl_FindParaFmt(rDoc, rName, 0, sal_False );
            if(pColl)
                rDoc.DelTxtFmtColl(pColl);
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFmt = lcl_FindFrmFmt(rDoc, rName, 0, sal_False );
            if(pFmt)
                rDoc.DelFrmFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PAGE :
        {
            sal_uInt16 nPos;
            if( rDoc.FindPageDescByName( rName, &nPos ))
                rDoc.DelPageDesc( nPos );
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            if( !rDoc.DelNumRule( rName ) )
                // Broadcast nur versenden, wenn etwas geloescht wurde
                bBroadcast = sal_False;
        }
        break;

    default:
        ASSERT(!this, "unbekannte Style-Familie");
        bBroadcast = sal_False;
    }

    if( bBroadcast )
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *pStyle ) );
}



sal_Bool  SwDocStyleSheetPool::SetParent( SfxStyleFamily eFam,
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

    sal_Bool bRet = sal_False;
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
            mxStyleSheet->PresetName( rStyle );
            mxStyleSheet->PresetParent( rParent );
            if( SFX_STYLE_FAMILY_PARA == eFam )
                mxStyleSheet->PresetFollow( ((SwTxtFmtColl*)pFmt)->
                        GetNextTxtFmtColl().GetName() );
            else
                mxStyleSheet->PresetFollow( aEmptyStr );

            Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
                                            *(mxStyleSheet.get()) ) );
        }
    }

    return bRet;
}

SfxStyleSheetBase* SwDocStyleSheetPool::Find( const String& rName,
                                            SfxStyleFamily eFam, sal_uInt16 n )
{
    sal_uInt16 nSMask = n;
    if( SFX_STYLE_FAMILY_PARA == eFam && rDoc.get(IDocumentSettingAccess::HTML_MODE) )
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

    const sal_Bool bSearchUsed = ( n != SFXSTYLEBIT_ALL &&
                             n & SFXSTYLEBIT_USED ) ? sal_True : sal_False;
    const SwModify* pMod = 0;

    mxStyleSheet->SetPhysical( sal_False );
    mxStyleSheet->PresetName( rName );
    mxStyleSheet->SetFamily( eFam );
    sal_Bool bFnd = mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    if( mxStyleSheet->IsPhysical() )
    {
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:
            pMod = mxStyleSheet->GetCharFmt();
            break;

        case SFX_STYLE_FAMILY_PARA:
            pMod = mxStyleSheet->GetCollection();
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pMod = mxStyleSheet->GetFrmFmt();
            break;

        case SFX_STYLE_FAMILY_PAGE:
            pMod = mxStyleSheet->GetPageDesc();
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                const SwNumRule* pRule = mxStyleSheet->GetNumRule();
                if( pRule &&
                    !(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pRule)) ) &&
                    (( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                            ? !(pRule->GetPoolFmtId() & USER_FMT)
                                // benutzte gesucht und keine gefunden
                            : bSearchUsed ))
                    bFnd = sal_False;
            }
            break;

        default:
            ASSERT(!this, "unbekannte Style-Familie");
        }
    }

    // dann noch die Maske auswerten:
    if( pMod && !(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pMod)) ) )
    {
        const sal_uInt16 nId = SFX_STYLE_FAMILY_PAGE == eFam
                        ? ((SwPageDesc*)pMod)->GetPoolFmtId()
                        : ((SwFmt*)pMod)->GetPoolFmtId();

        if( ( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
            ? !(nId & USER_FMT)
                // benutzte gesucht und keine gefunden
            : bSearchUsed )
            bFnd = sal_False;
    }
    return bFnd ? mxStyleSheet.get() : 0;
}

/*  */

SwStyleSheetIterator::SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                                SfxStyleFamily eFam, sal_uInt16 n )
    : SfxStyleSheetIterator( pBase, eFam, n ),
    mxIterSheet( new SwDocStyleSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 ) ),
    mxStyleSheet( new SwDocStyleSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 ) )
{
    bFirstCalled = sal_False;
    nLastPos = 0;
    StartListening( *pBase );
}

 SwStyleSheetIterator::~SwStyleSheetIterator()
{
    EndListening( mxIterSheet->GetPool() );
}

sal_uInt16  SwStyleSheetIterator::Count()
{
    // Liste richtig fuellen lassen !!
    if( !bFirstCalled )
        First();
    return aLst.Count();
}

SfxStyleSheetBase*  SwStyleSheetIterator::operator[]( sal_uInt16 nIdx )
{
    // gefunden
    if( !bFirstCalled )
        First();
    mxStyleSheet->PresetNameAndFamily( *aLst[ nIdx ] );
    mxStyleSheet->SetPhysical( sal_False );
    mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    return mxStyleSheet.get();
}

SfxStyleSheetBase*  SwStyleSheetIterator::First()
{
    // Alte Liste loeschen
    bFirstCalled = sal_True;
    nLastPos = 0;
    aLst.Erase();

    // aktuellen loeschen
    mxIterSheet->Reset();

    SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
    const sal_uInt16 nSrchMask = nMask;
    const sal_Bool bIsSearchUsed = SearchUsed();

    const sal_Bool bOrganizer = ((SwDocStyleSheetPool*)pBasePool)->IsOrganizerMode();

    if( nSearchFamily == SFX_STYLE_FAMILY_CHAR
     || nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nArrLen = rDoc.GetCharFmts()->Count();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];
            if( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() )
                continue;

            const sal_Bool  bUsed = bIsSearchUsed && (bOrganizer || rDoc.IsUsed(*pFmt));
            if( !bUsed )
            {
                // Standard ist keine Benutzervorlage #46181#
                const sal_uInt16 nId = rDoc.GetDfltCharFmt() == pFmt ?
                        sal_uInt16( RES_POOLCHR_INET_NORMAL ):
                                pFmt->GetPoolFmtId();
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                        // benutzte gesucht und keine gefunden
                    : bIsSearchUsed )
                continue;

                if( rDoc.get(IDocumentSettingAccess::HTML_MODE) && !(nId & USER_FMT) &&
                    !( RES_POOLCHR_HTML_BEGIN <= nId &&
                          nId < RES_POOLCHR_HTML_END ) &&
                    RES_POOLCHR_INET_NORMAL != nId &&
                    RES_POOLCHR_INET_VISIT != nId &&
                    RES_POOLCHR_FOOTNOTE  != nId &&
                    RES_POOLCHR_ENDNOTE != nId )
                    continue;
            }

            aLst.Append( cCHAR, pFmt == rDoc.GetDfltCharFmt()
                        ? (const String&) *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ]
                        : pFmt->GetName() );
        }

        // PoolFormate
        //
        if( nSrchMask == SFXSTYLEBIT_ALL )
        {
            if( !rDoc.get(IDocumentSettingAccess::HTML_MODE) )
                AppendStyleList(SwStyleNameMapper::GetChrFmtUINameArray(),
                                bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
            else
            {
                aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_INET_NORMAL - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_INET_VISIT - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_ENDNOTE - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_FOOTNOTE - RES_POOLCHR_BEGIN ] );
            }
            AppendStyleList(SwStyleNameMapper::GetHTMLChrFmtUINameArray(),
                                bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PARA ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        sal_uInt16 nSMask = nSrchMask;
        if( rDoc.get(IDocumentSettingAccess::HTML_MODE) )
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

        const sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->Count();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];

            if(pColl->IsDefault())
                continue;

            const sal_Bool bUsed = bOrganizer || rDoc.IsUsed(*pColl);
            if( !(bIsSearchUsed && bUsed ))
            {
                const sal_uInt16 nId = pColl->GetPoolFmtId();
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
                        sal_Bool bWeiter = sal_True;
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
                            bWeiter = sal_False;
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
                    if( bIsSearchUsed )
                        continue;
                }
            }
            aLst.Append( cPARA, pColl->GetName() );
        }

        const sal_Bool bAll = nSMask == SFXSTYLEBIT_ALL;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_TEXT )
            AppendStyleList(SwStyleNameMapper::GetTextUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA );
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CHAPTER )
            AppendStyleList(SwStyleNameMapper::GetDocUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_LIST )
            AppendStyleList(SwStyleNameMapper::GetListsUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_IDX )
            AppendStyleList(SwStyleNameMapper::GetRegisterUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_EXTRA )
            AppendStyleList(SwStyleNameMapper::GetExtraUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CONDCOLL )
        {
            if( !bIsSearchUsed ||
                rDoc.IsPoolTxtCollUsed( RES_POOLCOLL_TEXT ))
                aLst.Append( cPARA, *SwStyleNameMapper::GetTextUINameArray()[
                        RES_POOLCOLL_TEXT - RES_POOLCOLL_TEXT_BEGIN ] );
        }
        if ( bAll ||
            (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_HTML ||
            (nSMask & ~SFXSTYLEBIT_USED) ==
                        (SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF) )
        {
            AppendStyleList(SwStyleNameMapper::GetHTMLUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
            if( !bAll )
            {
                // dann auch die, die wir mappen:
                static sal_uInt16 aPoolIds[] = {
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

                sal_uInt16* pPoolIds = aPoolIds;
                String s;
                while( *pPoolIds )
                {
                    if( !bIsSearchUsed || rDoc.IsPoolTxtCollUsed( *pPoolIds ) )
                        aLst.Append( cPARA,
                            s = SwStyleNameMapper::GetUIName( *pPoolIds, s ));
                    ++pPoolIds;
                }
            }
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_FRAME ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nArrLen = rDoc.GetFrmFmts()->Count();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];

            if(pFmt->IsDefault() || pFmt->IsAuto())
            {
                continue;
            }

            const sal_uInt16 nId = pFmt->GetPoolFmtId();
            sal_Bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(*pFmt));
            if( !bUsed )
            {
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // benutzte gesucht und keine gefunden
                    : bIsSearchUsed )
                {
                    continue;
                }
            }

            aLst.Append( cFRAME, pFmt->GetName() );
        }

        // PoolFormate
        //
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(SwStyleNameMapper::GetFrmFmtUINameArray(),
                                    bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, cFRAME);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PAGE ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nCount = rDoc.GetPageDescCnt();
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            const SwPageDesc& rDesc =
                const_cast<const SwDoc &>(rDoc).GetPageDesc(i);
            const sal_uInt16 nId = rDesc.GetPoolFmtId();
            sal_Bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(rDesc));
            if( !bUsed )
            {
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // benutzte gesucht und keine gefunden
                    : bIsSearchUsed )
                    continue;
            }

            aLst.Append( cPAGE, rDesc.GetName() );
        }
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(SwStyleNameMapper::GetPageDescUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, cPAGE);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PSEUDO ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
        for(sal_uInt16 i = 0; i < rNumTbl.Count(); ++i)
        {
            const SwNumRule& rRule = *rNumTbl[ i ];
            if( !rRule.IsAutoRule() )
            {
                sal_Bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(rRule) );
                if( !bUsed )
                {
                    if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                        ? !(rRule.GetPoolFmtId() & USER_FMT)
                        // benutzte gesucht und keine gefunden
                        : bIsSearchUsed )
                        continue;
                }

                aLst.Append( cNUMRULE, rRule.GetName() );
            }
        }
        if ( nSrchMask == SFXSTYLEBIT_ALL )
            AppendStyleList(SwStyleNameMapper::GetNumRuleUINameArray(),
                            bIsSearchUsed, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE, cNUMRULE);
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
        mxIterSheet->PresetNameAndFamily(*aLst[nLastPos]);
        mxIterSheet->SetPhysical( sal_False );
        mxIterSheet->SetMask( nMask );
        if(mxIterSheet->pSet)
        {
            mxIterSheet->pSet->ClearItem(0);
            mxIterSheet->pSet= 0;
        }
        return mxIterSheet.get();
    }
    return 0;
}

SfxStyleSheetBase*  SwStyleSheetIterator::Find( const UniString& rName )
{
    // suchen
    if( !bFirstCalled )
        First();

    nLastPos = lcl_FindName( aLst, nSearchFamily, rName );
    if( USHRT_MAX != nLastPos )
    {
        // gefunden
        mxStyleSheet->PresetNameAndFamily(*aLst[nLastPos]);
        // neuer Name gesetzt, also bestimme seine Daten
        mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );
        if( !mxStyleSheet->IsPhysical() )
            mxStyleSheet->SetPhysical( sal_False );

        return mxStyleSheet.get();
    }
    return 0;
}

void SwStyleSheetIterator::AppendStyleList(const SvStringsDtor& rList,
                                            sal_Bool    bTestUsed,
                                            sal_uInt16 nSection, char cType )
{
    if( bTestUsed )
    {
        SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
        for ( sal_uInt16 i=0; i < rList.Count(); ++i )
        {
            sal_Bool bUsed = sal_False;
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(*rList[i], (SwGetPoolIdFromName)nSection);
            switch ( nSection )
            {
                case nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL:
                        bUsed = rDoc.IsPoolTxtCollUsed( nId );
                        break;
                case nsSwGetPoolIdFromName::GET_POOLID_CHRFMT:
                        bUsed = rDoc.IsPoolFmtUsed( nId );
                        break;
                case nsSwGetPoolIdFromName::GET_POOLID_FRMFMT:
                        bUsed = rDoc.IsPoolFmtUsed( nId );
                case nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC:
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
        for ( sal_uInt16 i=0; i < rList.Count(); ++i )
            aLst.Append( cType, *rList[i] );
}

void  SwStyleSheetIterator::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // suchen und aus der Anzeige-Liste entfernen !!
    if( rHint.ISA( SfxStyleSheetHint ) &&
        SFX_STYLESHEET_ERASED == ((SfxStyleSheetHint&) rHint).GetHint() )
    {
        SfxStyleSheetBase* pStyle = ((SfxStyleSheetHint&)rHint).GetStyleSheet();

        if (pStyle)
        {
            sal_uInt16 nTmpPos = lcl_FindName( aLst, pStyle->GetFamily(),
                                           pStyle->GetName() );
            if( nTmpPos < aLst.Count() )
                aLst.DeleteAndDestroy( nTmpPos );
        }
    }
}


