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


#include <hintids.hxx>          // fuer RES_..
#include <frame.hxx>            // fuer AttrCache
#include <format.hxx>
#include <hints.hxx>            // fuer SwFmtChg
#include <doc.hxx>
#include <paratr.hxx>           // fuer SwParaFmt - SwHyphenBug
#include <swcache.hxx>
// --> OD 2006-11-22 #i71574#
#include <fmtcolfunc.hxx>
// <--

TYPEINIT1( SwFmt, SwClient );   //rtti fuer SwFmt

/*************************************************************************
|*    SwFmt::SwFmt
*************************************************************************/


SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nFmtId( 0 ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    aFmtName.AssignAscii( pFmtNm );
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
    bAutoFmt = sal_True;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm,
            const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( rFmtNm ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nFmtId( 0 ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
    bAutoFmt = sal_True;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( const SwFmt& rFmt )
    : SwModify( rFmt.DerivedFrom() ),
    aFmtName( rFmt.aFmtName ),
    aSet( rFmt.aSet ),
    nWhichId( rFmt.nWhichId ),
    nFmtId( 0 ),
    nPoolFmtId( rFmt.GetPoolFmtId() ),
    nPoolHelpId( rFmt.GetPoolHelpId() ),
    nPoolHlpFileId( rFmt.GetPoolHlpFileId() )
{
    bWritten = bFmtInDTOR = sal_False; // LAYER_IMPL
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
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

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

void SwFmt::SetName( const String& rNewName, sal_Bool bBroadcast )
{
    ASSERT(!IsDefault(), "SetName: Defaultformat" );
    if( bBroadcast )
    {
        SwStringMsgPoolItem aOld( RES_NAME_CHANGED, aFmtName );
        SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
        aFmtName = rNewName;
        Modify( &aOld, &aNew );
    }
    else
    {
        aFmtName = rNewName;
    }
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


void SwFmt::CopyAttrs( const SwFmt& rFmt, sal_Bool bReplace )
{
    // kopiere nur das Attribut-Delta Array

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

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

        bFmtInDTOR = sal_True;

        SwFmt *pParentFmt = DerivedFrom();
        if (!pParentFmt)        // see #112405#
        {
            DBG_ERROR( "~SwFmt: parent format missing" );
        }
        else
        {
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
    sal_Bool bWeiter = sal_True;    // sal_True = Propagierung an die Abhaengigen

    sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
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
            bWeiter = sal_False;
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

            bWeiter = sal_False;
        }
        break;

    default:
        {
            // Ist das Attribut in diesem Format definiert, dann auf
            // NICHT weiter propagieren !!
            if( SFX_ITEM_SET == aSet.GetItemState( nWhich, sal_False ))
            {
// wie finde ich heraus, ob nicht ich die Message versende ??
// aber wer ruft das hier ????
//ASSERT( sal_False, "Modify ohne Absender verschickt" );
//JP 11.06.96: DropCaps koennen hierher kommen
ASSERT( RES_PARATR_DROP == nWhich, "Modify ohne Absender verschickt" );
                bWeiter = sal_False;
            }

        } // default
    } // switch

    if( bWeiter )
    {
        // laufe durch alle abhaengigen Formate
        SwModify::Modify( pOldValue, pNewValue );
    }

}


sal_Bool SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
{
    if ( pDerFrom )
    {
        // Zyklus?
        const SwFmt* pFmt = pDerFrom;
        while ( pFmt != 0 )
        {
            if ( pFmt == this )
                return sal_False;

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
        return sal_False;

    ASSERT( Which()==pDerFrom->Which()
            || ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
            || ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
            || ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
            "SetDerivedFrom: Aepfel von Birnen ableiten?");

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    pDerFrom->Add(this);
    aSet.SetParent( &pDerFrom->aSet );

    SwFmtChg aOldFmt(this);
    SwFmtChg aNewFmt(this);
    Modify( &aOldFmt, &aNewFmt );

    return sal_True;
}


sal_Bool SwFmt::SetFmtAttr(const SfxPoolItem& rAttr )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = rAttr.Which();
        CheckCaching( nWhich );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt;
    // fuer FrmFmt's immer das Modify verschicken!
    sal_Bool bRet = sal_False;
    const sal_uInt16 nFmtWhich = Which();
    if( IsModifyLocked() || (!GetDepends() &&
        (RES_GRFFMTCOLL == nFmtWhich  ||
         RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rAttr ))) )
            aSet.SetModifyAtAttr( this );
        // --> OD 2006-11-22 #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL && rAttr.Which() == RES_PARATR_NUMRULE )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
        // <--
    }
    else
    {
        // kopiere nur das Attribut-Delta Array
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                    aNew( *aSet.GetPool(), aSet.GetRanges() );

        bRet = 0 != aSet.Put_BC( rAttr, &aOld, &aNew );
        if( bRet )
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


sal_Bool SwFmt::SetFmtAttr( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return sal_False;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // wenn Modify gelockt ist, werden keine Modifies verschickt;
    // fuer FrmFmt's immer das Modify verschicken!
    sal_Bool bRet = sal_False;
    const sal_uInt16 nFmtWhich = Which();
    if ( IsModifyLocked() ||
         ( !GetDepends() &&
           ( RES_GRFFMTCOLL == nFmtWhich ||
             RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rSet ))) )
            aSet.SetModifyAtAttr( this );
        // --> OD 2006-11-22 #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
        // <--
    }
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                    aNew( *aSet.GetPool(), aSet.GetRanges() );
        bRet = 0 != aSet.Put_BC( rSet, &aOld, &aNew );
        if( bRet )
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


sal_Bool SwFmt::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !aSet.Count() )
        return sal_False;

    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1;      // dann setze auf 1. Id, nur dieses Item

    if ( IsInCache() || IsInSwFntCache() )
    {
        for( sal_uInt16 n = nWhich1; n < nWhich2; ++n )
            CheckCaching( n );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
        return 0 != (( nWhich2 == nWhich1 )
                ? aSet.ClearItem( nWhich1 )
                : aSet.ClearItem_BC( nWhich1, nWhich2 ));

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    sal_Bool bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
    }
    return bRet;
}



// --> OD 2007-01-24 #i73790#
// method renamed
sal_uInt16 SwFmt::ResetAllFmtAttr()
// <--
{
    if( !aSet.Count() )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
        return aSet.ClearItem( 0 );

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    sal_Bool bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );

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


sal_Bool SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    sal_Bool bRet = SwModify::GetInfo( rInfo );
    return bRet;
}


void SwFmt::DelDiffs( const SfxItemSet& rSet )
{
    if( !aSet.Count() )
        return;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        aSet.Intersect( rSet );
        return;
    }

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                aNew( *aSet.GetPool(), aSet.GetRanges() );
    sal_Bool bRet = 0 != aSet.Intersect_BC( rSet, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
    }
}

/** SwFmt::IsBackgroundTransparent - for feature #99657#

    OD 22.08.2002
    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent background.

    @author OD

    @return false, default implementation
*/
sal_Bool SwFmt::IsBackgroundTransparent() const
{
    return sal_False;
}

/** SwFmt::IsShadowTransparent - for feature #99657#

    OD 22.08.2002
    Virtual method to determine, if shadow of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent shadow.

    @author OD

    @return false, default implementation
*/
sal_Bool SwFmt::IsShadowTransparent() const
{
    return sal_False;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwFmt::getIDocumentSettingAccess() const { return GetDoc(); }
const IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() const { return GetDoc(); }
IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() { return GetDoc(); }
const IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() const { return GetDoc(); }
IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() { return GetDoc(); }
IDocumentTimerAccess* SwFmt::getIDocumentTimerAccess() { return GetDoc(); }
IDocumentFieldsAccess* SwFmt::getIDocumentFieldsAccess() { return GetDoc(); }
IDocumentChartDataProviderAccess* SwFmt::getIDocumentChartDataProviderAccess() { return GetDoc(); }

