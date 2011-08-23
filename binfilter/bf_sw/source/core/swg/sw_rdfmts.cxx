/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif
#include <bf_svtools/itemiter.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>

#include <fmtfsize.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <frmatr.hxx>

#include <errhdl.hxx>

#include <node.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <numrule.hxx>
#include "doc.hxx"
#include "docary.hxx"
#include "rdswg.hxx"
#include "swgpar.hxx"       // SWGRD_xxx-Flags
#include "poolfmt.hxx"
#include <SwStyleNameMapper.hxx>
namespace binfilter {

// Maximale Anzahl verarbeitbarer Formate

#define MAXFMTS (USHORT) ( 65000L / sizeof( FmtInfo ) )

// Diese Klasse steht als leerer Format-Container zur Verfuegung,
// der beliebig gefuellt werden kann.

class SwFmtContainer : public SwFmt
{
public:
    SwFmtContainer( SwDoc& rDoc )
    : SwFmt( rDoc.GetAttrPool(), String(), RES_CHRATR_BEGIN, RES_GRFATR_END,
             0, 0 )
    {}
};

//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////

static SwFmt* _GetUserPoolFmt( USHORT nId, const SvPtrarr* pFmtArr );

// Formatbeschreibungen

// Einlesen eines AttrSets
// Dies war frueher mal ein AutoFormat und ist als solches in der
// Datei abgelegt. Die Autoformat-Hierarchien werden hier entsprechend
// aufgeloest. Die Attribute werden als SwAttrs-Array registriert.
// Returnwert: das Format-ID des AttrSets

USHORT SwSwgReader::InAttrSet( SwAttrSet& rSet )
{
    // Format einlesen
    USHORT nFmtId, nDerived;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        if( r.next() != SWG_DATA ) Error();
        else r >> nDerived >> nFmtId;
        r.skipnext();   // Rest kann weg
        r.skip();       // Name kann weg
    }
    else
    {
        BYTE cFlags;
        r >> nDerived >> nFmtId;
        r.next(); r.skip(); // Name kann weg
        r >> cFlags;
    }
    // Attribute einlesen
    SfxItemSet* pSet = rSet.Clone( FALSE );
    if( r.next() == SWG_COMMENT ) r.skipnext();
    if( r.cur() == SWG_FMTHINTS )
    {
        UINT16 nHt;
        r >> nHt;
//      else
        {
            r.next();
            for( USHORT i = 0; i < nHt && r.good(); i++ )
            {
                InHint( *pSet );
                r.next();
            }
        }
    }
    if( ( nDerived & IDX_TYPEMASK ) == IDX_NAMEDFMT )
        nDerived = IDX_DFLT_VALUE;
    const SfxItemSet* pParent =
        ( nDerived == IDX_DFLT_VALUE || nDerived == IDX_NO_VALUE )
                         ? NULL : FindAttrSet( nDerived );
    pSet->SetParent( pParent );
    rSet.Put( *pSet );
    RegisterAttrSet( pSet, nFmtId );
    if( pParent )
        FillAttrSet( rSet, nDerived );
    return nFmtId;
}

void SwSwgReader::FillAttrSet( SwAttrSet& rSet, USHORT nFmtId )
{
    const SfxItemSet* pParent =
        ( nFmtId == IDX_DFLT_VALUE || nFmtId == IDX_NO_VALUE )
        ? NULL : FindAttrSet( nFmtId );
    // Das uebergebene AttrSet wird mit allen Attributen gefuellt
    while( pParent )
    {
        if( pParent->Count() )
        {
            SfxItemIter aIter( *pParent );
            const SfxPoolItem* pAttr = aIter.GetCurItem();
            for( USHORT i = 0; i < pParent->Count(); i++ )
            {
                // nur setzen, wenn nicht frisch eingelesen!
                if( rSet.GetItemState( pAttr->Which(), FALSE ) != SFX_ITEM_SET )
                    rSet.Put( *pAttr );
                pAttr = aIter.NextItem();
            }
        }
        pParent = pParent->GetParent();
    }
}

// Einlesen des Attribut-Sets eines CntntNodes

USHORT SwSwgReader::InAttrSet( SwCntntNode& rNd )
{
    USHORT nFmtId;
    if( rNd.GetDepends() )
    {
        SwAttrSet aTmpSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
        nFmtId = InAttrSet( aTmpSet );
        if( aTmpSet.Count() )
            rNd.SetAttr( aTmpSet );
    }
    else
    {
        if( !rNd.GetpSwAttrSet() )
            rNd.NewAttrSet( pDoc->GetAttrPool() );
        nFmtId = InAttrSet( *rNd.GetpSwAttrSet() );
    }
    return nFmtId;
}

// Ein Format einlesen
// Hier wird das Format auch erzeugt.
// Wird auch von Hints verwendet!

SwFmt* SwSwgReader::InFormat( SwFmt* pFmt, USHORT* pParentId )
{
    BYTE cFmtKind = r.cur();
    r.getskip();
    BYTE nHelpFid = (BYTE) nHelpFileId;
    USHORT nHelpId = USHRT_MAX;
    BYTE cFlags;
    USHORT nFmtId;
    if( cFmtKind == SWG_FLYFMT ) nFlyLevel++;
    nFmtLvl++;

    String aName( String::CreateFromAscii("dummy") );
    USHORT nDerived, nPoolId = IDX_NO_VALUE;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        if( r.next() != SWG_DATA ) Error();
        else r >> nDerived >> nFmtId >> nPoolId;
        // Korrektur fuer aeltere Pool-Ids:
        if( aHdr.nVersion <= SWG_VER_FRAMES3 ) nPoolId = IDX_NO_VALUE;
        else if( nPoolId < IDX_SPECIAL )
            // USER-Feld verkleinern
            nPoolId &= 0xBFFF;
        // Flag Bits:
        // 0x01 - Autoformat (tot)
        // 0x02 - Breakformat (tot)
        // 0x04 - Help ID folgt
        // 0x08 - Pattern-Name fuer Help folgt
        // 0x10 - Format-ID folgt, das bereits den Pattern-Namen hat
        r >> cFlags;
        if( cFlags & 0x04 )
        {
            BYTE n;
            r >> n;
            nHelpId = (UCHAR_MAX == n ? USHRT_MAX : (USHORT)n);
        }
        if( cFlags & 0x08 )
        {
            String aPattern = GetText();
            if( aPattern.Len() )
                nHelpFid = (BYTE) pDoc->SetDocPattern( aPattern );
        }
        if( cFlags & 0x10 )
        {
            USHORT nHelpFmt;
            r >> nHelpFmt;
            SwFmt* pHelpFmt = FindFmt( nHelpFmt, 0 );
            if( pHelpFmt )
                nHelpFid = pHelpFmt->GetPoolHlpFileId();
        }
        r.skip();
        aName = GetText();
    }
    else
    {
        r >> nDerived >> nFmtId;
        aName = GetText();
        r >> cFlags;
    }
    // Freundlicherweise treten wir hier 1 Bit an ST ab, der es
    // im PageDesc benoetigt (Version 1.x)
    nFlagBits = cFlags >> 4;

    // Format erzeugen
    if( !pFmt ) {
        switch( cFmtKind )
        {
            case SWG_FREEFMT:
                // Freifliegende Frameformate werden von Headers
                // und Footers verwendet.
                pFmt = new SwFrmFmt( pDoc->GetAttrPool(), aName,
                                     pDoc->GetDfltFrmFmt() );
                break;
            case SWG_FRAMEFMT:
                if( IDX_NO_VALUE != nPoolId )
                {
                    if( IsPoolUserFmt( nPoolId ) )
                    {
                        // wir suchen uns das richtige Format
                        if( 0 == ( pFmt = _GetUserPoolFmt( nPoolId,
                                                pDoc->GetFrmFmts() )) )
                            pFmt = _GetUserPoolFmt( nPoolId,
                                                pDoc->GetSpzFrmFmts() );
                        ASSERT( pFmt, "Format not found." );
                    }
                    else
                        pFmt = pDoc->GetFrmFmtFromPool( nPoolId );
                }
                if( !pFmt )
                {
                    if( IDX_DFLT_VALUE != nDerived && pParentId )
                    {
                        *pParentId = nDerived & ~IDX_TYPEMASK;
                        SwFmt* pParent = 0;
                        if( pFmts && pFmts[ *pParentId ].cFmt & FINFO_FORMAT )
                            pParent = pFmts[ *pParentId ].pFmt;
                        pFmt = pDoc->MakeFrmFmt( aName, (SwFrmFmt*)pParent );
                        if( pParent )
                            *pParentId = USHRT_MAX;
                    }
                    else
                        pFmt = pDoc->MakeFrmFmt( aName,
                                (SwFrmFmt*)FindFmt( nDerived, cFmtKind ));
                }
                else
                    pFmt->ResetAllAttr();
                break;
            case SWG_CHARFMT:
                if( IDX_NO_VALUE != nPoolId )
                {
                    if( IsPoolUserFmt( nPoolId ) )
                        // wir suchen uns das richtige Format
                        pFmt = _GetUserPoolFmt( nPoolId, pDoc->GetCharFmts());
                    else
                        pFmt = pDoc->GetCharFmtFromPool( nPoolId );
                }
                if( !pFmt )
                {
                    if( IDX_DFLT_VALUE != nDerived && pParentId )
                    {
                        *pParentId = nDerived & ~IDX_TYPEMASK;
                        SwFmt* pParent = 0;
                        if( pFmts && pFmts[ *pParentId ].cFmt & FINFO_FORMAT )
                            pParent = pFmts[ *pParentId ].pFmt;
                        pFmt = pDoc->MakeCharFmt( aName, (SwCharFmt*)pParent );
                        if( pParent )
                            *pParentId = USHRT_MAX;
                    }
                    else
                        pFmt = pDoc->MakeCharFmt( aName,
                            (SwCharFmt*) FindFmt( nDerived, cFmtKind ) );
                }
                else
                    pFmt->ResetAllAttr();
                break;
            case SWG_FLYFMT:
                pFmt = pDoc->MakeFlyFrmFmt( aName,
                        (SwFlyFrmFmt*) FindFmt( nDerived, cFmtKind ) );
            break;
        }
    } else if( nDerived != IDX_NO_VALUE )
        pFmt->SetDerivedFrom( FindFmt( nDerived, cFmtKind ) );

    pFmt->nFmtId = nFmtId;
    pFmt->SetPoolHelpId( nHelpId );
    pFmt->SetPoolHlpFileId( nHelpFid );
    pFmt->SetAuto( BOOL( ( cFlags & 0x01 ) != 0 ) );
    if( nPoolId != IDX_NO_VALUE ) pFmt->SetPoolFmtId( nPoolId );

    if( r.next() == SWG_COMMENT ) r.skipnext();
    if( r.cur() == SWG_FMTHINTS )
    {
        SwAttrSet aSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_GRFATR_END );
        short nHt;
        r >> nHt;
        if( nHt < 0 ) Error(), r.skipnext();
        else {
            r.next();
            for( int i = 0; i < nHt && r.good(); i++ )
            {
                InHint( aSet );
                r.next();
                // Wurde ein PageDesc-Attr gelesen? Dann das Format
                // im PageDescLink noch nachtragen!
                if( pPageLinks && !pPageLinks->pFmt )
                {
                    pPageLinks->pFmt = pFmt;
                    pPageLinks->cFmt = TRUE;
                }
            }
        }
        pFmt->SetAttr( aSet );
    }

    if( cFmtKind == SWG_FLYFMT )
    {
        nFlyLevel--;
        // Hot fix: Umsetzen des Attributes ATT_VAR_SIZE in ATT_MIN_SIZE
        // im Hint SwFmtFrmSize innerhalb von FlyFrames
        // MIB 18.11.96: und ausserdem noch die Groesse und Position
        // an die neuen Abstaende nach aussen anpassen
        SwFrmFmt* pFrmFmt = (SwFrmFmt*) pFmt;

        const SvxLRSpaceItem& rLRSpace = pFrmFmt->GetLRSpace();
        const SvxULSpaceItem& rULSpace = pFrmFmt->GetULSpace();

        const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
        Size aConvSize(
            rFrmSize.GetSizeConvertedFromSw31( &rLRSpace,
                                                &rULSpace ) );

        if( rFrmSize.GetSize() != aConvSize ||
            rFrmSize.GetSizeType() == ATT_VAR_SIZE )
        {
            SwFmtFrmSize aFrmSize( rFrmSize );
            aFrmSize.SetSize( aConvSize );
            if( aFrmSize.GetSizeType() == ATT_VAR_SIZE )
                aFrmSize.SetSizeType( ATT_MIN_SIZE );

            pFrmFmt->SetAttr( aFrmSize );
        }

        const SwFmtHoriOrient& rHori = pFmt->GetHoriOrient();
        SwTwips nConvPos =
            rHori.GetPosConvertedFromSw31( &rLRSpace );
        if( rHori.GetPos() != nConvPos )
        {
            SwFmtHoriOrient aHori( rHori );
            aHori.SetPos( nConvPos );
            pFrmFmt->SetAttr( aHori );
        }

        const SwFmtVertOrient& rVert = pFmt->GetVertOrient();
        nConvPos = rVert.GetPosConvertedFromSw31( &rULSpace );
        if( rVert.GetPos() != nConvPos )
        {
            SwFmtVertOrient aVert( rVert );
            aVert.SetPos( nConvPos );
            pFrmFmt->SetAttr( aVert );
        }
    }
    nFmtLvl--;
    return pFmt;
}

// Testen, ob ein Format einen Pool-Formatnamen hat.Dies fuehrt bei alten
// Dokumenten dazu, dass das Pool-ID gesetzt wird.

BOOL SwSwgReader::TestPoolFmt( SwFmt& rFmt, int eFlags )
{
    USHORT nPoolId = rFmt.GetPoolFmtId();
    if( aHdr.nVersion <= SWG_VER_FRAMES3 && nPoolId == IDX_NO_VALUE )
    {
        nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rFmt.GetName(),
                        (SwGetPoolIdFromName)eFlags );
        if( nPoolId != USHRT_MAX ) {
            rFmt.SetPoolFmtId( nPoolId );
            return TRUE;
        }
    }
    return FALSE;
}

// Suchen eines benannten Frame- oder Zeichenformats
// Falls ein gleichnamiges Format existiert wie das uebergebene,
// wird dieser Ptr zurueckgeliefert, sonst NULL,

static SwFmt* _findframefmt( const SwDoc* pDoc, const SwFmt* pFmt )
{
    USHORT nArrLen = pDoc->GetFrmFmts()->Count();
    for( USHORT j = 0; j < nArrLen; j++ )
    {
        SwFmt* pFmt2 = (*pDoc->GetFrmFmts())[ j ];
        if( ( pFmt != pFmt2 )
          && ( pFmt->GetName() == pFmt2->GetName() ) )
            return pFmt2;
    }
    return NULL;

}


static SwFmt* _findcharfmt( const SwDoc* pDoc, const SwFmt* pFmt )
{
    USHORT nArrLen = pDoc->GetCharFmts()->Count();
    for( USHORT j = 0; j < nArrLen; j++ )
    {
        SwFmt* pFmt2 = (*pDoc->GetCharFmts())[ j ];
        if( ( pFmt != pFmt2 )
          && ( pFmt->GetName() == pFmt2->GetName() ) )
            return pFmt2;
    }
    return NULL;

}

// Eine Text-Collection einlesen

void lcl_sw3io__ConvertNumTabStop( SvxTabStopItem& rTStop, long nOffset );

void SwSwgReader::InTxtFmtColl( short nIdx )
{
    BOOL bForceRead = BOOL( ( nOptions != SWGRD_NORMAL )
                         && ( nOptions & SWGRD_PARAFMTS )
                         && ( nOptions & SWGRD_FORCE ) );
    long skipit = r.getskip();
    SwTxtFmtColl* pColl = NULL;
    BYTE nLevel;
    USHORT nDerived = IDX_NO_VALUE, nFollow = IDX_NO_VALUE;
    USHORT nCharRef = IDX_NO_VALUE, nPoolId = IDX_NO_VALUE;
    String aName;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        if( r.next() != SWG_DATA )
        {
            Error(); return;
        }
        r >> nLevel >> nDerived >> nFollow >> nCharRef >> nPoolId;
        // Korrektur fuer aeltere Pool-Ids:
        if( aHdr.nVersion <= SWG_VER_FRAMES3 ) nPoolId = IDX_DFLT_VALUE;
        else if( nPoolId < IDX_SPECIAL )
            // USER-Feld verkleinern
            nPoolId &= 0xBFFF;
        r.skip();
        aName = GetText();
    } else {
        r >> nLevel;
        aName = GetText();
        r >> nDerived >> nFollow;
        if( aHdr.nVersion >= SWG_VER_COLLREF )
            r >> nCharRef;
    }

    // Fehler: Unbenannte Vorlage; Defaultnamen vergeben
    if( !aName.Len() )
    {
        static short n = 1;
        aName.AssignAscii( "NoName" );
        aName += String::CreateFromInt32( n++ );
    }

    // Gibt es diese Coll bereits?
    USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
    USHORT n;
    // Erste Runde: Ist bei alten Dokumenten der Name ein Poolformatname?
    if( aHdr.nVersion < SWG_VER_COMPAT || nPoolId == IDX_DFLT_VALUE )
    {
        nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_TXTCOLL );
        if( nPoolId == USHRT_MAX ) nPoolId = IDX_NO_VALUE;
    }
    // Zweite Runde: Nach dem Pool-ID suchen ( falls definiert! )
    if( nPoolId != IDX_NO_VALUE && !IsPoolUserFmt( nPoolId ))
      for( n = 0; n < nArrLen; n++ )
      {
        SwTxtFmtColl* pCur = (*pDoc->GetTxtFmtColls())[ n ];
        if( pCur->GetPoolFmtId() == nPoolId )
        {
            pColl = pCur;
            break;
        }
    }
    // Dritte Runde: Pool-Vorlage anfordern, falls moeglich
    if( !pColl && nPoolId != IDX_NO_VALUE && !IsPoolUserFmt( nPoolId ))
    {
        pColl = pDoc->GetTxtCollFromPool( nPoolId );
        ASSERT( pColl, "Keine TxtColl fuer PoolId gefunden" );
        if( !pColl )
            nPoolId = IDX_NO_VALUE;
    }
    // Vierte Runde: Ueber den Namen suchen
    if( !pColl )
      for( n = 0; n < nArrLen; n++ )
      {
        SwTxtFmtColl* pCur = (*pDoc->GetTxtFmtColls())[ n ];
        if( pCur->GetName() == aName )
            pColl = pCur;
        // Bei Einfuegen von Vorlagen muss ggf. der Outline-Level geloescht
        // werden, wenn bereits eine Vorlage mit diesem Level existiert.
        if( pCur->GetOutlineLevel() == nLevel )
            nLevel = NO_NUMBERING;
    }

    // Falls es sie gibt, muessen die Formate neu erstellt werden.
    // In Insert Mode darf die Coll jedoch nicht veraendert werden!

    if( !pColl || ( nPoolId != IDX_NO_VALUE ) )
    {
        if( !pColl )
        {
            SwTxtFmtColl* pDer = (*pDoc->GetTxtFmtColls())[ 0 ];
            pColl = pDoc->MakeTxtFmtColl( aName, pDer );
        }
        pCollIdx[ nIdx ].nDerived = nDerived | IDX_COLLECTION;
        pCollIdx[ nIdx ].nFollow  = nFollow | IDX_COLLECTION;
    }
    else
    {
        // Soll die Collection dennoch glesen werden?
        // Durch einen OT-Bug kommt es vor, dass eine Coll mehrfach
        // vorhanden ist. Wenn dies der Fall ist, nicht einlesen.
        BOOL bRead = bForceRead;
        // Die vorhandene Standardvorlage wird bei neuem Doc immer
        // uebergeplaettet.
        if( bNew && ( nIdx == 1 ) ) bRead = TRUE;
        if( !bRead )
        {
            // Also doch nicht einlesen
            pCollIdx[ nIdx ].nDerived =
            pCollIdx[ nIdx ].nFollow = IDX_NO_VALUE;
            pColl->nFmtId = IDX_COLLECTION + nIdx;
            RegisterFmt( *pColl );
            r.skip( skipit ); r.next(); return;
        }
    }

    pColl->nFmtId = IDX_COLLECTION + nIdx;
    pColl->SetOutlineLevel( nLevel );

    BOOL bDone = FALSE;
    pColl->ResetAllAttr();
    // HACK: So lange es die Referenz nicht gibt, wird das benannte
    // Format in das eigene CharFmt kopiert. Muss hier geschehen,
    // da Set() den Inhalt loescht.
    if( nCharRef != IDX_NO_VALUE )
    {
        SwCharFmt* pCfmt = (SwCharFmt*) FindFmt( nCharRef, SWG_CHARFMT );
        if( pCfmt && !pCfmt->IsDefault() )
        {
            SfxItemSet& rColl = (SfxItemSet&) pColl->GetAttrSet();
            rColl.Set( pCfmt->GetAttrSet() );
        }
    }
    r.next();
    while( !bDone ) {
        switch( r.cur() ) {
            case SWG_FRAMEFMT:
            case SWG_PARAFMT:
            case SWG_CHARFMT:
                // Bei CharFmts an den Hot fix denken, dass bei
                // Namensgleichheit mit einem benannten Format
                // kein Format gelesen, sondern statt dessen das
                // benannte Format angebunden wird!
                InFormat( pColl );
                break;
            case SWG_COMMENT:
                if( r.tell() < skipit ) r.skipnext();
            default:
                bDone = TRUE;
        }
    }
    if( nPoolId != IDX_NO_VALUE )
        pColl->SetPoolFmtId( nPoolId );

    const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
    if( nLevel < MAXLEVEL && pOutline )
    {
        const SwNumFmt& rFmt = pOutline->Get( nLevel );
        const SvxLRSpaceItem& rLRSpace = pColl->GetLRSpace();
        if( rFmt.GetAbsLSpace() != rLRSpace.GetTxtLeft() ||
            rFmt.GetFirstLineOffset() != rLRSpace.GetTxtFirstLineOfst() )
        {
            SwNumFmt aFmt( rFmt );
            aFmt.SetAbsLSpace( rLRSpace.GetTxtLeft() );
            aFmt.SetFirstLineOffset( rLRSpace.GetTxtFirstLineOfst() );

            SwNumRule aOutline( *pOutline );
            aOutline.Set( nLevel, aFmt );
            pDoc->SetOutlineNumRule( aOutline );
        }

#ifdef NUM_RELSPACE
        // Den linken-Einzug in der Vorlage auf 0 setzen, damit
        // er nicht doppelt gezaehlt wird. Wenn das
        SvxLRSpaceItem aLRSpace( rLRSpace );
        USHORT nOldLSpace = rLRSpace.GetTxtLeft();
        aLRSpace.SetTxtFirstLineOfst( 0 );
        aLRSpace.SetTxtLeft( 0U );
        SwFmt *pParFmt = pColl->DerivedFrom();
        if( pParFmt && pParFmt->GetLRSpace() == aLRSpace )
            pColl->ResetAttr( RES_LR_SPACE );
        else if( aLRSpace != rLRSpace )
            pColl->SetAttr( aLRSpace );
        if( nOldLSpace != 0 )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pColl->GetAttrSet().GetItemState(
                                        RES_PARATR_TABSTOP, TRUE, &pItem ))
            {
                SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
                lcl_sw3io__ConvertNumTabStop( aTStop, nOldLSpace );
                pColl->SetAttr( aTStop );
            }
        }
#endif
    }
    RegisterFmt( *pColl );
}

// Alle Collections einlesen


void SwSwgReader::InTxtFmtColls()
{
    USHORT i;
    r >> nColl;
    if( nColl ) {
        // Format-Indextabelle erzeugen
        TxtCollInfo* p =
        pCollIdx = new TxtCollInfo[ nColl + 1 ];
        for( i = 0; i <= nColl; i++, p++ )
        {
            p->nDerived =
            p->nFollow  = IDX_NO_VALUE;
        }
        r.next();
        for( i = 1; i <= nColl && r.good(); i++ )
            InTxtFmtColl( i );
        p = pCollIdx + 1;
        for( i = 1; i <= nColl && r.good(); i++, p++ )
        {
            SwTxtFmtColl* pCur = (SwTxtFmtColl*) FindFmt( IDX_COLLECTION + i, 0 );
            if( pCur )
            {
                // Derived setzen
                USHORT nIdx = p->nDerived;
                if( nIdx != IDX_NO_VALUE )
                {
                    // dann ist die Coll abhaengig
                    SwTxtFmtColl* pParent = (SwTxtFmtColl*) FindFmt( nIdx, 0 );
                    pCur->SetDerivedFrom( pParent );
                }
                // Next setzen
                nIdx = p->nFollow;
                if( nIdx != IDX_NO_VALUE )
                {
                    SwTxtFmtColl* pNext = (SwTxtFmtColl*) FindFmt( nIdx, 0 );
                    pCur->SetNextTxtFmtColl( *pNext );
                }
            }
        }
    } else r.next();

}

/////////////////////////////////////////////////////////////////////////////

// Einlesen der Default- und Autoformate


void SwSwgReader::InDfltFmts()
{
    if( !bNew )
    {
        // Einfuegen: Defaultformate lassen!
        r.skipnext();
        return;
    }
    r.next();
    BOOL bDone = FALSE;
    while( !bDone && r.good() )
    {
        switch( r.cur() )
        {
        case SWG_FRAMEFMT:
        case SWG_CHARFMT:
        case SWG_PARAFMT:
        case SWG_GRFFMT:
            {
                SwFmtContainer aContainer( *pDoc );
                InFormat( &aContainer );
                if( aContainer.GetAttrSet().Count() )
                {
                    SfxItemIter aIter( aContainer.GetAttrSet() );
                    while( TRUE )
                    {
                        const SfxPoolItem* pItem = aIter.GetCurItem();
                        pDoc->GetAttrPool().SetPoolDefaultItem( *pItem );

                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                }
            }
            break;

        default:
            bDone = TRUE;
        }
    }
}

// Einlesen der benannten Formate.

// Die Optionen lassen eine gezielte Auswahl zu.


void SwSwgReader::InNamedFmts( USHORT nOptions )
{
    SvPtrarr aFmtArr;   //JP 29.09.95: sind die Parents noch nicht eingelesen
    SvUShorts aIdArr;   // dann am Ende alle Verbinden

    USHORT i, n;
    r >> n;
    nNamedFmt = 0;
    r.next();
    for( i = 0; i < n && r.good(); i++ )
    {
        BYTE cType = r.cur();
        if( cType < SWG_LEFTFMT || cType > SWG_GRFFMT )
        {
            Error(); break;
        }
        // das geht hier OK, da das Format in jedem Fall gefuellt wird,
        // wenn es durch Text benoetigt werden sollte.
        SwFmt* pFmt = NULL;
        SwFmt* pFmt2;
        USHORT nParentId = USHRT_MAX;
        switch( cType )
        {
            case SWG_CHARFMT:
                if( nOptions & SWGRD_CHARFMTS )
                {
                    pFmt = InFormat( NULL, &nParentId );
                    TestPoolFmt( *pFmt, GET_POOLID_CHRFMT );
                    pFmt2 = _findcharfmt( pDoc, pFmt );
                    if( pFmt2 )
                    {
                        // Format bereits dem Namen nach drin
                        // Entweder ueberbuegeln oder vergessen
                        if( nOptions & SWGRD_FORCE )
                            *pFmt2 = *pFmt;

                        // Registrierung umsetzen !!
                        ReRegisterFmt( *pFmt, *pFmt2 );
                        pDoc->DelCharFmt( (SwCharFmt*) pFmt );
                        pFmt = pFmt2;
                        break;
                    }
                    else
                    {
                        RegisterFmt( *pFmt );
                        if( USHRT_MAX != nParentId )
                        {
                            // muss am Ende den richtigen Parent bekommen!
                            void* pPtr = pFmt;
                            aFmtArr.Insert( pPtr, aFmtArr.Count() );
                            aIdArr.Insert( nParentId, aIdArr.Count() );
                        }
                    }
                }
                else r.skipnext();
                break;
            case SWG_FRAMEFMT:
                if( nOptions & SWGRD_FRAMEFMTS )
                {
                    pFmt = InFormat( NULL, &nParentId );
                    TestPoolFmt( *pFmt, GET_POOLID_FRMFMT );
                    pFmt2 = _findframefmt( pDoc, pFmt );
                    if( pFmt2 )
                    {
                        // Format bereits dem Namen nach drin,
                        // Entweder ueberbuegeln oder vergessen
                        if( nOptions & SWGRD_FORCE )
                            *pFmt2 = *pFmt;

                        // Registrierung umsetzen !!
                        ReRegisterFmt( *pFmt, *pFmt2 );
                        pDoc->DelFrmFmt( (SwFrmFmt*) pFmt );
                        pFmt = pFmt2;
                        break;
                    }
                    else
                    {
                        RegisterFmt( *pFmt );
                        if( USHRT_MAX != nParentId )
                        {
                            // muss am Ende den richtigen Parent bekommen!
                            void* pPtr = pFmt;
                            aFmtArr.Insert( pPtr, aFmtArr.Count() );
                            aIdArr.Insert( nParentId, aIdArr.Count() );
                        }
                    }
                }
                else r.skipnext();
                break;
            default:
                r.skipnext();
        }
        if( pFmt )
            nNamedFmt++;
    }

    if( pFmts )
    for( i = aFmtArr.Count(); i; )
    {
        SwFmt* pFmt = (SwFmt*)aFmtArr[ --i ];
        SwFmt* pParent;
        if( pFmts[ n = aIdArr[ i ] ].cFmt & FINFO_FORMAT )
            pParent = pFmts[ n ].pFmt;
        else if( RES_CHRFMT == pFmt->Which() )
            pParent = pDoc->GetDfltCharFmt();
        else
            pParent = pDoc->GetDfltFrmFmt();

        pFmt->SetDerivedFrom( pParent );
    }
}

// Registrieren eines Formats
// Das Format wird mit seinem Wert in der Tabelle registriert.
// Die Collection-Formate wwerden an das Ende der Tabelle gehaengt, da
// die Format-ID sich mit den anderen IDs ueberschneiden.

void SwSwgReader::RegisterFmt( SwFmt& rFmt, const SwTable *pTable )
{
    USHORT nIdx = rFmt.nFmtId;
    if( nIdx == IDX_NO_VALUE || nIdx == IDX_DFLT_VALUE )
        return;
    // Collection-Index?
    if( ( nIdx & IDX_TYPEMASK ) == IDX_COLLECTION )
        nIdx = MAXFMTS - 1 - ( nIdx & ~IDX_TYPEMASK );
    else
        nIdx &= ~IDX_TYPEMASK;
    // Array eingerichtet?
    if( !pFmts ) {
        pFmts = new FmtInfo[ MAXFMTS ];
        memset( pFmts, 0, MAXFMTS * sizeof( FmtInfo ) );
    }
    // Bereits registriert?
    ASSERT( pFmts[ nIdx ].pFmt == 0, "Format bereits registriert!" );

    pFmts[ nIdx ].pFmt = &rFmt;
    pFmts[ nIdx ].cFmt = FINFO_FORMAT;
    if( nStatus & SWGSTAT_LOCALFMTS )
        pFmts[ nIdx ].cFmt |= FINFO_LOCAL;

    if( pTable )
        RegisterTable( nIdx, pTable );
}

// Registrieren eines AttrSets
// Ein AttrSet wird u.U. mehrfach registeriert, da es die Rolle
// der drei Autoformate am Node uebernommen hat.

void SwSwgReader::RegisterAttrSet( SfxItemSet* pSet, USHORT nIdx )
{
    if( nIdx == IDX_NO_VALUE || nIdx == IDX_DFLT_VALUE )
        return;
    // Array eingerichtet?
    if( !pFmts ) {
        pFmts = new FmtInfo[ MAXFMTS ];
        memset( pFmts, 0, MAXFMTS * sizeof( FmtInfo ) );
    }
    // Bereits registriert?
    ASSERT( pFmts[ nIdx ].pSet == 0, "AttrSet bereits registriert!" );
    pFmts[ nIdx ].pSet = pSet;
    pFmts[ nIdx ].cFmt = 0;
    if( nStatus & SWGSTAT_LOCALFMTS )
        pFmts[ nIdx ].cFmt |= FINFO_LOCAL;
}

// Freigabe aller Autoformate (AttrSets) vor dem Einlesen
// eines neuen Textbausteins (da die Autoformat-Nummern neu vergeben
// werden koennen)

void SwSwgReader::ReleaseAttrSets()
{
    FmtInfo* p = pFmts;
    if( p )
    {
        for( USHORT i = 0; i < MAXFMTS; i++, p++ )
        {
            if( p->cFmt & FINFO_LOCAL )
            {
                if( ! (p->cFmt & FINFO_FORMAT) )
                    delete p->pSet;
                p->pSet = NULL;
                p->cFmt = 0;
            }
        }
    }
    delete pTables; pTables = 0;
}

// Finden eines Formats nach Index

SwFmt* SwSwgReader::FindFmt( USHORT nIdx, BYTE cKind )
{
    SwFmt* pFmt = NULL;
    switch( nIdx )
    {
        case IDX_NO_VALUE:
            return NULL;    // Direkter Abbruch, kein Assert
        case IDX_COLUMN:
            pFmt = pDoc->GetColumnContFmt(); break;
        case IDX_EMPTYPAGE:
            pFmt = pDoc->GetEmptyPageFmt(); break;
        case IDX_DFLT_VALUE:
            switch( cKind )
            {
                case SWG_MASTERFMT:
                case SWG_LEFTFMT:
                case SWG_FLYFMT:
                case SWG_FREEFMT:
                case SWG_FRAMEFMT:
                    pFmt = pDoc->GetDfltFrmFmt(); break;
                case SWG_CHARFMT:
                case SWG_PARAFMT:
                    pFmt = pDoc->GetDfltCharFmt(); break;
                case SWG_GRFFMT:
                    pFmt = (SwFmt*) pDoc->GetDfltGrfFmtColl(); break;
                case 0:
                    return NULL;    // Direkter Abbruch, kein Assert
            } break;
        default:
            // Collection-Index?
            if( ( nIdx & IDX_TYPEMASK ) == IDX_COLLECTION )
                nIdx = MAXFMTS - 1 - ( nIdx & ~IDX_TYPEMASK );
            else
                nIdx &= ~IDX_TYPEMASK;
            if( pFmts && pFmts[ nIdx ].cFmt & FINFO_FORMAT )
                pFmt = pFmts[ nIdx ].pFmt;
    }
    ASSERT( pFmt, "Format-ID unbekannt" );
    return pFmt;
}

// Finden eines AttrSets nach Index

SfxItemSet* SwSwgReader::FindAttrSet( USHORT nIdx )
{
    // Standard-Werte
    if( nIdx == IDX_NO_VALUE || nIdx == IDX_DFLT_VALUE )
        return NULL;
    // Nicht von Autofmt abgeleitet?
    // dann gibt es keine Ableitung
    if( ( nIdx & IDX_TYPEMASK ) != IDX_AUTOFMT )
        return NULL;
    nIdx &= ~IDX_TYPEMASK;
    SfxItemSet* pSet = NULL;
    if( pFmts && !( pFmts[ nIdx ].cFmt & FINFO_FORMAT ) )
        pSet = pFmts[ nIdx ].pSet;
    ASSERT( pSet, "Format-ID (AttrSet) unbekannt" );
    return pSet;
}

static SwFmt* _GetUserPoolFmt( USHORT nId, const SvPtrarr* pFmtArr )
{
    SwFmt* pFmt;
    for( USHORT n = 0; n < pFmtArr->Count(); ++n )
        if( nId == (pFmt = (SwFmt*)(*pFmtArr)[n])->GetPoolFmtId() )
            return pFmt;
    return 0;
}

// Re-Registrierung eines Formats mit neuem Index
// wird bei shared Hdr/Ftr-Formaten in InPageDesc() verwendet
// Der Eintrag fuer pFmtOld wird auf pFmtNew gesetzt, so dass spaetere
// Referenzen auf den Index pFmtNew liefern

void SwSwgReader::ReRegisterFmt( const SwFmt& rFmtOld, const SwFmt& rFmtNew,
                                 const SwTable *pTable)
{
    USHORT nIdx = rFmtOld.nFmtId;
    if( !nIdx )
        nIdx = rFmtNew.nFmtId;
    ((SwFmt&)rFmtNew).nFmtId = nIdx;
    ASSERT( nIdx, "Format nicht registriert" );
    nIdx &= ~IDX_TYPEMASK;
    ASSERT( !( pFmts[ nIdx ].cFmt & FINFO_FORMAT ) ||
            !pFmts[ nIdx ].pSet ||
            pFmts[ nIdx ].pFmt == (SwFmt*) &rFmtNew ||
            (pTable && pTable != FindTable(nIdx)),
            "Bereits ein AttrSet definiert!" );
    pFmts[ nIdx ].pFmt = (SwFmt*) &rFmtNew;
    pFmts[ nIdx ].cFmt = FINFO_FORMAT;
    if( nStatus & SWGSTAT_LOCALFMTS )
        pFmts[ nIdx ].cFmt |= FINFO_LOCAL;

    if( pTable )
        RegisterTable( nIdx, pTable );
}

// Entfernen aller Format-IDs, damit der Writer nicht
// durcheinander kommt (er vergibt neue IDs)

void SwSwgReader::ClearFmtIds()
{
    FmtInfo* p = pFmts;
    for( USHORT i = 0; i < MAXFMTS; i++, p++ )
    {
        if( p->cFmt & FINFO_FORMAT )
            p->pFmt->nFmtId = 0;
        else
            delete p->pSet, p->pSet = NULL;
    }
    delete pTables; pTables = 0;
}

void SwSwgReader::RegisterTable( USHORT nIdx, const SwTable *pTable )
{
    if( !pTables )
        pTables = new SwgTables;
    while( pTables->Count() <= nIdx )
        pTables->Insert( (const SwTable *)0, pTables->Count() );
    pTables->Replace( pTable, nIdx );
}

const SwTable *SwSwgReader::FindTable( USHORT nIdx )
{
    const SwTable *pRet = 0;
    if( pTables && nIdx < pTables->Count() )
        pRet = (*pTables)[nIdx];

    return pRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
