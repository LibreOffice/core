/*************************************************************************
 *
 *  $RCSfile: format.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>          // fuer RES_..
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>            // fuer AttrCache
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>
#endif

#ifndef _HINTS_HXX
#include <hints.hxx>            // fuer SwFmtChg
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>           // fuer SwParaFmt - SwHyphenBug
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif


TYPEINIT1( SwFmt, SwClient );   //rtti fuer SwFmt

/*************************************************************************
|*    SwFmt::SwFmt
*************************************************************************/


SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const USHORT* pWhichRanges, SwFmt *pDrvdFrm, USHORT nFmtWhich )
    : SwModify( pDrvdFrm ),
    aSet( rPool, pWhichRanges ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    nFmtId( 0 ),
    nWhichId( nFmtWhich )
{
    aFmtName.AssignAscii( pFmtNm );
    bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
    bAutoFmt = TRUE;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm,
            const USHORT* pWhichRanges, SwFmt *pDrvdFrm, USHORT nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( rFmtNm ),
    aSet( rPool, pWhichRanges ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    nFmtId( 0 ),
    nWhichId( nFmtWhich )
{
    bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
    bAutoFmt = TRUE;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm, USHORT nWhich1,
                USHORT nWhich2, SwFmt *pDrvdFrm, USHORT nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( rFmtNm ),
    aSet( rPool, nWhich1, nWhich2 ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    nFmtId( 0 ),
    nWhichId( nFmtWhich )
{
    bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
    bAutoFmt = TRUE;
    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( const SwFmt& rFmt )
    : SwModify( rFmt.DerivedFrom() ),
    aFmtName( rFmt.aFmtName ),
    aSet( rFmt.aSet ),
    nPoolFmtId( rFmt.GetPoolFmtId() ),
    nPoolHelpId( rFmt.GetPoolHelpId() ),
    nPoolHlpFileId( rFmt.GetPoolHlpFileId() ),
    nFmtId( 0 ),
    nWhichId( rFmt.nWhichId )
{
    bWritten = bFmtInDTOR = bLayerFmt = FALSE; // LAYER_IMPL
    bAutoFmt = rFmt.bAutoFmt;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;

    if( rFmt.DerivedFrom() )
        aSet.SetParent( &rFmt.DerivedFrom()->aSet );
    // einige Sonderbehandlungen fuer Attribute
    aSet.SetModifyAtAttr( this );
}

/*************************************************************************
|*    SwFmt &SwFmt::operator=(const SwFmt& aFmt)
|*
|*    Beschreibung      Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


SwFmt &SwFmt::operator=(const SwFmt& rFmt)
{
    nWhichId = rFmt.nWhichId;
    nPoolFmtId = rFmt.GetPoolFmtId();
    nPoolHelpId = rFmt.GetPoolHelpId();
    nPoolHlpFileId = rFmt.GetPoolHlpFileId();

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    // kopiere nur das Attribut-Delta Array
    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    aSet.Intersect_BC( rFmt.aSet, &aOld, &aNew );
    aSet.Put_BC( rFmt.aSet, &aOld, &aNew );

    // einige Sonderbehandlungen fuer Attribute
    aSet.SetModifyAtAttr( this );

    // PoolItem-Attr erzeugen fuers Modify !!!
    if( aOld.Count() )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );        // alle veraenderten werden verschickt
    }

    if( pRegisteredIn != rFmt.pRegisteredIn )
    {
        if( pRegisteredIn )
            pRegisteredIn->Remove(this);
        if(rFmt.pRegisteredIn)
        {
            rFmt.pRegisteredIn->Add(this);
            aSet.SetParent( &rFmt.aSet );
        }
        else
            aSet.SetParent( 0 );
    }
    bAutoFmt = rFmt.bAutoFmt;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
    return *this;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * diese Funktion wird in jedem Copy-Ctor gerufen, damit die
 * Attribute kopiert werden. Diese koennen erst kopiert werden,
 * wenn die abgeleitet Klasse existiert, denn beim Setzen der
 * Attribute wird die Which()-Methode gerufen, die hier in der
 * Basis-Klasse auf 0 defaultet ist.
 *
 * Zusatz: JP 8.4.1994
 *  Wird ueber Dokumentgrenzen kopiert, so muss das neue Dokument
 *  mit angeben werden, in dem this steht. Z.Z. ist das fuers
 *  DropCaps wichtig, dieses haelt Daten, die tief kopiert werden
 *  muessen !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


void SwFmt::CopyAttrs( const SwFmt& rFmt, BOOL bReplace )
{
    // kopiere nur das Attribut-Delta Array
    register SwCharFmt* pDropCharFmt = 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    // Sonderbehandlung fuer einige Attribute
    SwAttrSet* pChgSet = (SwAttrSet*)&rFmt.aSet;

    if( !bReplace )     // nur die neu, die nicht gesetzt sind ??
    {
        if( pChgSet == (SwAttrSet*)&rFmt.aSet )     // Set hier kopieren
            pChgSet = new SwAttrSet( rFmt.aSet );
        pChgSet->Differentiate( aSet );
    }

    // kopiere nur das Attribut-Delta Array
    if( pChgSet->GetPool() != aSet.GetPool() )
        pChgSet->CopyToModify( *this );
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );

        if ( 0 != aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
        {
            // einige Sonderbehandlungen fuer Attribute
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }

    if( pChgSet != (SwAttrSet*)&rFmt.aSet )     // Set hier angelegt ?
        delete pChgSet;
}

/*************************************************************************
|*    SwFmt::~SwFmt()
|*
|*    Beschreibung      Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 14.02.91
*************************************************************************/


SwFmt::~SwFmt()
{
    /* das passiert bei der ObjectDying Message */
    /* alle Abhaengigen auf DerivedFrom umhaengen */
    if( GetDepends() )
    {
        ASSERT(DerivedFrom(), "SwFmt::~SwFmt: Def Abhaengige!" );

        bFmtInDTOR = TRUE;

        SwFmt *pParentFmt = DerivedFrom();
        while( GetDepends() )
        {
            SwFmtChg aOldFmt(this);
            SwFmtChg aNewFmt(pParentFmt);
            SwClient * pDepend = (SwClient*)GetDepends();
            pParentFmt->Add(pDepend);
            pDepend->Modify(&aOldFmt, &aNewFmt);
        }
    }
}


/*************************************************************************
|*    void SwFmt::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
|*
|*    Beschreibung      Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


void SwFmt::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
{
    BOOL bWeiter = TRUE;    // TRUE = Propagierung an die Abhaengigen

    USHORT nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;
    switch( nWhich )
    {
    case 0:     break;          // Which-Id von 0 ???

    case RES_OBJECTDYING :
        {
            // ist das sterbende Object das "Parent"-Format von diesen Format,
            // dann haengt sich dieses Format an den Parent vom Parent
            SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;

            // nicht umhaengen wenn dieses das oberste Format ist !!
            if( pRegisteredIn && pRegisteredIn == pFmt )
            {
                if( pFmt->pRegisteredIn )
                {
                    // wenn Parent, dann im neuen Parent wieder anmelden
                    pFmt->DerivedFrom()->Add( this );
                    aSet.SetParent( &DerivedFrom()->aSet );
                }
                else
                {
                    // sonst auf jeden Fall beim sterbenden abmelden
                    DerivedFrom()->Remove( this );
                    aSet.SetParent( 0 );
                }
            }
        } // OBJECTDYING
        break;

    case RES_ATTRSET_CHG:
        if( ((SwAttrSetChg*)pOldValue)->GetTheChgdSet() != &aSet )
        {
            //nur die weiter geben, die hier nicht gesetzt sind !!
            SwAttrSetChg aOld( *(SwAttrSetChg*)pOldValue );
            SwAttrSetChg aNew( *(SwAttrSetChg*)pNewValue );

            aOld.GetChgSet()->Differentiate( aSet );
            aNew.GetChgSet()->Differentiate( aSet );

            if( aNew.Count() )
                // keine mehr gesetzt, dann Ende !!
                SwModify::Modify( &aOld, &aNew );
            bWeiter = FALSE;
        }
        break;
    case RES_FMT_CHG:
        // falls mein Format Parent umgesetzt wird, dann melde ich
        // meinen Attrset beim Neuen an.

        // sein eigenes Modify ueberspringen !!
        if( ((SwFmtChg*)pOldValue)->pChangedFmt != this &&
            ((SwFmtChg*)pNewValue)->pChangedFmt == DerivedFrom() )
        {
            // den Set an den neuen Parent haengen
            aSet.SetParent( DerivedFrom() ? &DerivedFrom()->aSet : 0 );
        }
        break;

    case RES_RESET_FMTWRITTEN:
        {
            // IsWritten-Flag zuruecksetzen. Hint nur an abhanegige
            // Formate (und keine Frames) propagieren.
            ResetWritten();
            SwClientIter aIter( *this );
            for( SwClient *pClient = aIter.First( TYPE(SwFmt) ); pClient;
                        pClient = aIter.Next() )
                pClient->Modify( pOldValue, pNewValue );

            bWeiter = FALSE;
        }
        break;

    default:
        {
            // Ist das Attribut in diesem Format definiert, dann auf
            // NICHT weiter propagieren !!
            if( SFX_ITEM_SET == aSet.GetItemState( nWhich, FALSE ))
            {
// wie finde ich heraus, ob nicht ich die Message versende ??
// aber wer ruft das hier ????
//ASSERT( FALSE, "Modify ohne Absender verschickt" );
//JP 11.06.96: DropCaps koennen hierher kommen
ASSERT( RES_PARATR_DROP == nWhich, "Modify ohne Absender verschickt" );
                bWeiter = FALSE;
            }

        } // default
    } // switch

    if( bWeiter )
    {
        // laufe durch alle abhaengigen Formate
        SwModify::Modify( pOldValue, pNewValue );
    }

}


BOOL SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
{
    if ( pDerFrom )
    {
        // Zyklus?
        const SwFmt* pFmt = pDerFrom;
        while ( pFmt != 0 )
        {
            if ( pFmt == this )
                return FALSE;

            pFmt=pFmt->DerivedFrom();
        }
    }
    else
    {
        // Nichts angegeben, Dflt-Format suchen
        pDerFrom = this;
        while ( pDerFrom->DerivedFrom() )
            pDerFrom = pDerFrom->DerivedFrom();
    }
    if ( (pDerFrom == DerivedFrom()) || (pDerFrom == this) )
        return FALSE;

    ASSERT( Which()==pDerFrom->Which()
            || ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
            || ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
            || ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
            "SetDerivedFrom: Aepfel von Birnen ableiten?");

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    pDerFrom->Add(this);
    aSet.SetParent( &pDerFrom->aSet );

    SwFmtChg aOldFmt(this);
    SwFmtChg aNewFmt(this);
    Modify( &aOldFmt, &aNewFmt );

    return TRUE;
}


BOOL SwFmt::SetAttr(const SfxPoolItem& rAttr )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const USHORT nWhich = rAttr.Which();
        CheckCaching( nWhich );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt;
    // fuer FrmFmt's immer das Modify verschicken!
    BOOL bRet = FALSE;
    USHORT nFmtWhich;
    if( IsModifyLocked() || (!GetDepends() &&
        (RES_GRFFMTCOLL == (nFmtWhich = Which()) ||
         RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rAttr ))) )
            aSet.SetModifyAtAttr( this );
    }
    else
    {
        // kopiere nur das Attribut-Delta Array
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                    aNew( *aSet.GetPool(), aSet.GetRanges() );

        if( 0 != (bRet = aSet.Put_BC( rAttr, &aOld, &aNew )))
        {
            // einige Sonderbehandlungen fuer Attribute
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}


BOOL SwFmt::SetAttr( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return FALSE;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    // wenn Modify gelockt ist, werden keine Modifies verschickt;
    // fuer FrmFmt's immer das Modify verschicken!
    BOOL bRet = FALSE;
    USHORT nFmtWhich;
    if( IsModifyLocked() || (!GetDepends() &&
        (RES_GRFFMTCOLL == (nFmtWhich = Which()) ||
         RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rSet ))) )
            aSet.SetModifyAtAttr( this );
    }
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                    aNew( *aSet.GetPool(), aSet.GetRanges() );
        if( 0 != ( bRet = aSet.Put_BC( rSet, &aOld, &aNew ) ) )
        {
            // einige Sonderbehandlungen fuer Attribute
            aSet.SetModifyAtAttr( this );
            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}

// Nimmt den Hint mit nWhich aus dem Delta-Array


BOOL SwFmt::ResetAttr( USHORT nWhich1, USHORT nWhich2 )
{
    if( !aSet.Count() )
        return FALSE;

    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1;      // dann setze auf 1. Id, nur dieses Item

    if ( IsInCache() || IsInSwFntCache() )
    {
        for( USHORT n = nWhich1; n < nWhich2; ++n )
            CheckCaching( n );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
        return 0 != (( nWhich2 == nWhich1 )
                ? aSet.ClearItem( nWhich1 )
                : aSet.ClearItem_BC( nWhich1, nWhich2 ));

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    BOOL bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
    }
    return bRet;
}



USHORT SwFmt::ResetAllAttr()
{
    if( !aSet.Count() )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
        return aSet.ClearItem( 0 );

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    BOOL bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
    }
    return aNew.Count();
}


/*************************************************************************
|*    void SwFmt::GetInfo( const SfxPoolItem& ) const
|*
|*    Beschreibung
|*    Ersterstellung    JP 18.04.94
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


BOOL SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    BOOL bRet = SwModify::GetInfo( rInfo );
    return bRet;
}


void SwFmt::DelDiffs( const SfxItemSet& rSet )
{
    if( !aSet.Count() )
        return;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        aSet.Intersect( rSet );
        return;
    }

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    BOOL bRet = 0 != aSet.Intersect_BC( rSet, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
    }
}




