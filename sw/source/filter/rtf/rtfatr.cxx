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
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des RTF-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */
#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hdl>
#include <vcl/cvtgrf.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svl/whiter.hxx>
#include <svtools/rtfout.hxx>
#include <svl/itemiter.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <unotools/charclass.hxx>
#include <reffld.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <fmtfld.hxx>
#include <fmtflcnt.hxx>
#include <fmtftn.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <txtftn.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <charfmt.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <paratr.hxx>
#include <fldbas.hxx>           // fuer SwField ...
#include <wrtrtf.hxx>
#include <rtf.hxx>          // fuer SwPictureType
#include <ndgrf.hxx>
#include <grfatr.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <pagedesc.hxx>     // fuer SwPageDesc ...
#include <swtable.hxx>      // fuer SwPageDesc ...
#include <docsh.hxx>
#include <swrect.hxx>
#include <section.hxx>
#include <wrtswtbl.hxx>
#include <htmltbl.hxx>
#include <fmtclbl.hxx>
#include <breakit.hxx>
#include <fmtruby.hxx>
#include <txtatr.hxx>
#include <fltini.hxx>
#include <fmtrowsplt.hxx>

/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
 */
#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC) && !defined(__MINGW32__) && !defined(OS2)

#define ATTRFNTAB_SIZE 130
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#error Attribut-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#ifdef FORMAT_TABELLE
// da sie nicht benutzt wird!
#define FORMATTAB_SIZE 7
#if FORMATTAB_SIZE != RES_FMT_END - RES_FMT_BEGIN
#error Format-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif
#endif

#define NODETAB_SIZE 3
#if NODETAB_SIZE != RES_NODE_END - RES_NODE_BEGIN
#error Node-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#endif

#ifdef WNT
// ueber xoutbmp.hxx wird das winuser.h angezogen. Dort gibt es ein
// define GetProp das mit unserem SvxEscapement kollidiert!
#undef GetProp
#endif


using namespace com::sun::star;

//-----------------------------------------------------------------------

static Writer& OutRTF_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt );

//-----------------------------------------------------------------------

SvStream& OutComment( Writer& rWrt, const sal_Char* pStr )
{
    return (rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << pStr);
}

SvStream& OutComment( Writer& rWrt, const char* pStr, BOOL bSetFlag )
{
    // setze Flag, damit bei der Ausgabe von Attributen zu erkennen,
    // ob ueberhaupt etwas ausgegeben wurde.
    ((SwRTFWriter&)rWrt).bOutFmtAttr = bSetFlag;
    return (rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << pStr);
}

Writer& OutRTF_AsByteString( Writer& rWrt, const String& rStr, rtl_TextEncoding eEncoding)
{
    ByteString sOutStr( rStr, eEncoding );
    rWrt.Strm() << sOutStr.GetBuffer();
    return rWrt;
}

void OutRTF_SfxItemSet( SwRTFWriter& rWrt, const SfxItemSet& rSet,
                        BOOL bDeep )
{
    bool bFrameDirOut=false;
    bool bAdjustOut=false;
    // erst die eigenen Attribute ausgeben
    SvPtrarr aAsian( 0, 5 ), aCmplx( 0, 5 ), aLatin( 0, 5 );

    const SfxItemPool& rPool = *rSet.GetPool();
    SfxWhichIter aIter( rSet );
    const SfxPoolItem* pItem;
    FnAttrOut pOut;
    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if( SFX_ITEM_SET == rSet.GetItemState( nWhich, bDeep, &pItem ))
        {
            pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
            if( pOut &&
                ( *pItem != rPool.GetDefaultItem( nWhich )
                    || ( rSet.GetParent() &&
                        *pItem != rSet.GetParent()->Get( nWhich ) )
                    || ( rWrt.GetAttrSet() &&
                        *pItem != rWrt.GetAttrSet()->Get( nWhich ) )
              ) )
                ;
            else
                pOut = 0;
        }
        else if( !bDeep )
            pOut = 0;
        else if( 0 != ( pItem = rPool.GetPoolDefaultItem( nWhich )) )
            pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
        else
            pOut = 0;

        if (!pOut && bDeep)
        {
            switch( nWhich )
            {
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CTL_FONTSIZE:
                case RES_CHRATR_LANGUAGE:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_CTL_LANGUAGE:
                    pItem = &rPool.GetDefaultItem( nWhich );
                    pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
                    break;
                default:
                    break;
            }
        }

        if( pOut )
        {
            void* pVoidItem = (void*)pItem;
            switch( nWhich )
            {
            case RES_CHRATR_FONT:
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_LANGUAGE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                aLatin.Insert( pVoidItem, aLatin.Count() );
                pOut = 0;
                break;

            case RES_CHRATR_CJK_FONT:
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_LANGUAGE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
                aAsian.Insert( pVoidItem, aAsian.Count() );
                pOut = 0;
                break;

            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                aCmplx.Insert( pVoidItem, aCmplx.Count() );
                pOut = 0;
                break;

            case RES_FRAMEDIR:
                bFrameDirOut=true;
                break;
            case RES_PARATR_ADJUST:
                bAdjustOut=true;
                break;
            }
        }

        if (pOut)
            (*pOut)( rWrt, *pItem );
        nWhich = aIter.NextWhich();
    }

    //If rtlpar set and no following alignment. And alignment is not rtl then
    //output alignment
    if (bFrameDirOut && !bAdjustOut && !rWrt.pFlyFmt && !rWrt.bOutPageDesc)
    {
        pOut = aRTFAttrFnTab[ static_cast< USHORT >(RES_PARATR_ADJUST) - RES_CHRATR_BEGIN];
        if (pOut)
            (*pOut)(rWrt, rSet.Get(RES_PARATR_ADJUST));
    }
    if (rWrt.pFlyFmt && !rWrt.bOutPageDesc && !bFrameDirOut)
    {
        pOut = aRTFAttrFnTab[ static_cast< USHORT >(RES_FRAMEDIR) - RES_CHRATR_BEGIN];
        if (pOut)
            (*pOut)(rWrt, rSet.Get(RES_FRAMEDIR));
    }

    if (aAsian.Count() || aCmplx.Count() || aLatin.Count())
    {
        SvPtrarr* aArr[4];
        switch (rWrt.GetCurrScriptType())
        {
        case i18n::ScriptType::LATIN:
            aArr[ 0 ] = &aCmplx;
            aArr[ 1 ] = &aAsian;
            aArr[ 2 ] = &aLatin;
            aArr[ 3 ] = &aLatin;
            break;

        case i18n::ScriptType::ASIAN:
            aArr[ 0 ] = &aCmplx;
            aArr[ 1 ] = &aLatin;
            aArr[ 2 ] = &aLatin;
            aArr[ 3 ] = &aAsian;
            break;

        case i18n::ScriptType::COMPLEX:
            aArr[ 0 ] = &aLatin;
            aArr[ 1 ] = &aLatin;
            aArr[ 2 ] = &aAsian;
            aArr[ 3 ] = &aCmplx;
            break;

        default:
            return ;
        }

        //The final entry is the one that is actually in use
        //so it uses e.g. \b \i \fs, the others are not in
        //use and so are "associated". Both asian and western
        //are ltr runs, with asian being the dbch varient
        //and western being the loch/hich varient
        bool bOutLTOR = true;
        bool bLowLTOR = false;
        for( int nArrCnt = 0; nArrCnt < 4; ++nArrCnt )
        {
            SvPtrarr* pCurArr = aArr[ nArrCnt ];
            if (pCurArr->Count())
            {
                bool bInUse = (aArr[nArrCnt] == aArr[3]);
                rWrt.SetAssociatedFlag(!bInUse);
                if (pCurArr == &aLatin)
                {
                    if (bOutLTOR)
                    {
                        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LTRCH;
                        bOutLTOR = false;
                    }

                    if (bLowLTOR)
                        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LOCH;
                    else
                    {
                        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_HICH;
                        bLowLTOR = true;
                    }
                }
                else if( pCurArr == &aAsian )
                {
                    if( bOutLTOR )
                    {
                        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LTRCH;
                        bOutLTOR = false;
                    }
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_DBCH;
                }
                else
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_RTLCH;

                for (USHORT n = 0; n < pCurArr->Count(); ++n)
                {
                    pItem = (const SfxPoolItem*)(*pCurArr)[ n ];
                    pOut = aRTFAttrFnTab[ pItem->Which() - RES_CHRATR_BEGIN];
                    (*pOut)( rWrt, *pItem );
                }
            }
        }
    }
}

// fuer die Formate muesste eine einzige Ausgabe-Funktion genuegen !
/*
 * Formate wie folgt ausgeben:
 *      - gebe alle Attribute vom Format aus
 */

bool SwFmtToSet(SwRTFWriter& rWrt, const SwFmt& rFmt, SfxItemSet &rSet)
{
    bool bOutItemSet = true;
    rSet.SetParent(rFmt.GetAttrSet().GetParent());

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        {
            USHORT nId = rWrt.GetId( (const SwTxtFmtColl&)rFmt );
            if (0 == nId )
                return false;       // Default-TextStyle nicht ausgeben !!

            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_S;
            rWrt.OutULong( nId );
            rWrt.bOutFmtAttr = TRUE;
//          BYTE nLvl = ((const SwTxtFmtColl&)rFmt).GetOutlineLevel();      //#outline level,zhaojianwei
//          if( MAXLEVEL > nLvl )
//          {
            if(((const SwTxtFmtColl&)rFmt).IsAssignedToListLevelOfOutlineStyle())
            {
                int nLvl = ((const SwTxtFmtColl&)rFmt).GetAssignedOutlineStyleLevel();  //<-end,zhaojianwei
                USHORT nNumId = rWrt.GetNumRuleId(
                                        *rWrt.pDoc->GetOutlineNumRule() );
                if( USHRT_MAX != nNumId )
                {
                    BYTE nWWLvl = 8 >= nLvl ? static_cast<BYTE>(nLvl) : 8;
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LS;
                    rWrt.OutULong( nNumId );
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ILVL; rWrt.OutULong( nWWLvl );
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_OUTLINELEVEL; rWrt.OutULong( nWWLvl );
                    if( nWWLvl != nLvl )            // RTF-kennt nur 9 Ebenen
                    {
                        OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_SOUTLVL );
                        rWrt.OutULong( nLvl ) << '}';
                    }
                }

                const SwNumFmt* pNFmt = &rWrt.pDoc->GetOutlineNumRule()->Get( static_cast<USHORT>(nLvl) );
                if( pNFmt->GetAbsLSpace() )
                {
                    SfxItemSet aSet( *rFmt.GetAttrSet().GetPool(),
                                    rFmt.GetAttrSet().GetRanges() );
                    aSet.SetParent( &rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)aSet.Get( RES_LR_SPACE ) );

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + pNFmt->GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( pNFmt->GetFirstLineOffset() );

                    aSet.Put(aLR);
                    rSet.Put(aSet);
                    bOutItemSet = false;
                }
            }
        }
        break;
    case RES_CHRFMT:
        {
            USHORT nId = rWrt.GetId( (const SwCharFmt&)rFmt );
            if (0 == nId)
                return false;       // Default-CharStyle nicht ausgeben !!

            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_CS;
            rWrt.OutULong( nId );
            rWrt.bOutFmtAttr = TRUE;
        }
        break;

//  case RES_GRFFMTCOLL:
// ?????
    }

    if (bOutItemSet)
        rSet.Put(rFmt.GetAttrSet());

    return true;
}

Writer& OutRTF_SwFmt(Writer& rWrt, const SwFmt& rFmt)
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    SfxItemSet aSet(*rFmt.GetAttrSet().GetPool(),
        rFmt.GetAttrSet().GetRanges() );
    if (SwFmtToSet(rRTFWrt, rFmt, aSet))
        OutRTF_SfxItemSet(rRTFWrt, aSet, TRUE);

    return rWrt;
}

void OutRTF_SwFlyFrmFmt( SwRTFWriter& rRTFWrt )
{
    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    ASSERT( rRTFWrt.pFlyFmt, "wo ist das Fly-Format?" );

    if( rRTFWrt.bOutFmtAttr )
    {
        rRTFWrt.Strm() << ' ';
        rRTFWrt.bOutFmtAttr = false;
    }
    // gebe erstmal alle RTF-Spezifischen Attribute aus
    rRTFWrt.bRTFFlySyntax = TRUE;
    OutRTF_SwFmt( rRTFWrt, *rRTFWrt.pFlyFmt );

    // dann gebe alle eigenen Attribute aus
    {
        // dazu erzeuge einen temp strstream, um festzustellen ob es
        // ueberhaupt eigene Attribute gibt !
        SvMemoryStream aTmpStrm;
        SvStream* pSaveStrm = &rRTFWrt.Strm();
        rRTFWrt.SetStream( &aTmpStrm );

        rRTFWrt.bRTFFlySyntax = false;
        OutRTF_SwFmt( rRTFWrt, *rRTFWrt.pFlyFmt );

        rRTFWrt.SetStream( pSaveStrm );   // Stream-Pointer wieder zurueck

        if ( aTmpStrm.GetEndOfData() ) // gibt es SWG spezifische Attribute?
        {
            aTmpStrm.Seek( 0L );
            rRTFWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << aTmpStrm << '}';
        }
    }
//  rRTFWrt.pFlyFmt = 0;        // wieder zuruecksetzen !!
}

/* Ausgabe der Nodes */

/*
 *  Ausgabe der Texte-Attribute:
 *      Die Text-Attribute sind in einem VarArray nach ihren Start-Positionen
 *      sortiert. Fuer den RTF-Writer ist aber auch das Ende von Bedeutung.
 *      Darum die Idee, sobald im SwpHints-Array ein Attribut mit der Start-
 *      Position gefunden wurde, in einem Sortierten-Array die Endposition
 *      zu speichern. Sobald der Writer die Position erreicht hat, wird die
 *      schliessende Klammer ausgegeben und die Position aus dem Sort.Array
 *      geloescht.
 *  15.3.93: Es reicht leider nicht aus nur Start und End zuspeichern, denn
 *      im SwpHints Array stehen sie nicht nach Ende sortiert, so dass
 *      Attribute die falsche schliessende Klammer bekommen. (z.B
 *      Bold von 0..3, dann folgt Underline von 0..4. Dann bekommt Underline
 *      die schliessende Klammer von Bold !!)
 *      Also muessen erst alle Attribute einer Position gesammelt, nach
 *      Ende sortiert und dann erst ausgegeben werden.
 */

SV_DECL_PTRARR( SfxPoolItems, SfxPoolItem*, 4, 4 )
class RTFEndPosLst;

class SttEndPos
{
    // falls mehrere Attribute den gleichen Bereich umspannen, sammeln
    SfxPoolItems aArr;
    xub_StrLen nStart, nEnd;

    SttEndPos( const SttEndPos & rSEPos );

public:
    SttEndPos( const SfxPoolItem& rAttr, xub_StrLen nStt, xub_StrLen nEnd );
    ~SttEndPos();

    xub_StrLen  GetStart() const            { return nStart; }
    xub_StrLen  GetEnd() const              { return nEnd; }

    const SfxPoolItems& GetAttrs() const    { return aArr; }
    void AddAttr( const SfxPoolItem& rAttr );
    BOOL HasScriptChange() const;
};


SV_DECL_PTRARR_DEL( _EndPosLst, SttEndPos*, 5, 5 )
SV_IMPL_PTRARR( _EndPosLst, SttEndPos* )

class RTFEndPosLst : private _EndPosLst
{
    const SwTxtNode& rNode;
    SwRTFWriter& rWrt;
    RTFEndPosLst* pOldPosLst;
    xub_StrLen nCurPos;
    USHORT nCurScript;

public:
    using _EndPosLst::Count;
    using _EndPosLst::operator[];
    using _EndPosLst::DeleteAndDestroy;

    RTFEndPosLst( SwRTFWriter& rWrt, const SwTxtNode& rNd, xub_StrLen nStart );
    ~RTFEndPosLst();

    USHORT GetCurScript() const         { return nCurScript; }
    BOOL MatchScriptToId( USHORT nId ) const;
    int Insert( const SfxPoolItem& rAttr, xub_StrLen nStt, xub_StrLen nEnd );

    void OutAttrs( xub_StrLen nStrPos );
    void EndAttrs( xub_StrLen nStrPos );
    void OutScriptChange( xub_StrLen nStrPos );

    const SfxPoolItem* HasItem( USHORT nWhich ) const;
    const SfxPoolItem& GetItem( USHORT nWhich ) const;
    void OutFontAttrs(const SfxPoolItem &rItem);
    void OutFontAttrs(USHORT nScript, bool bRTL);

    SfxItemPool& GetPool() {return *rNode.GetSwAttrSet().GetPool(); }
};


SttEndPos::SttEndPos( const SfxPoolItem& rAttr,
                        xub_StrLen nStt, xub_StrLen nEd )
    : nStart( nStt ), nEnd( nEd )
{
    AddAttr( rAttr );
}

SttEndPos::~SttEndPos()
{
    for( USHORT n = 0, nCount = aArr.Count(); n < nCount; ++n )
        if( RES_FLTRATTR_BEGIN <= aArr[ n ]->Which() )
            delete aArr[ n ];
}

BOOL SttEndPos::HasScriptChange() const
{
    for( USHORT n = 0, nCount = aArr.Count(); n < nCount; ++n )
        if( RES_FLTR_SCRIPTTYPE == aArr[ n ]->Which() )
            return TRUE;
    return FALSE;
}

void SttEndPos::AddAttr( const SfxPoolItem& rAttr )
{
    const SfxPoolItem* pI = &rAttr;
    aArr.Insert(pI, aArr.Count());
}

RTFEndPosLst::RTFEndPosLst(SwRTFWriter& rWriter, const SwTxtNode& rNd,
    xub_StrLen nStart)
    : rNode(rNd), rWrt(rWriter), nCurPos(STRING_NOTFOUND)
{
    pOldPosLst = rWrt.pCurEndPosLst;
    rWrt.pCurEndPosLst = this;

    using namespace sw::util;
    CharRuns aCharRuns(GetPseudoCharRuns(rNd, nStart));
    cCharRunIter aEnd = aCharRuns.end();
    xub_StrLen nSttPos = nStart;
    for (cCharRunIter aI = aCharRuns.begin(); aI != aEnd; ++aI)
    {
        if (nSttPos != aI->mnEndPos)
        {
            SfxPoolItem* pChg = new SfxUInt32Item(RES_FLTR_SCRIPTTYPE,
                (sal_uInt32(aI->mnScript) << 16) | static_cast<sal_uInt32>(aI->mbRTL));
            Insert(*pChg, nSttPos, aI->mnEndPos);
            nSttPos = aI->mnEndPos;
        }
    }
}

RTFEndPosLst::~RTFEndPosLst()
{
    rWrt.pCurEndPosLst = pOldPosLst;
}

int RTFEndPosLst::Insert( const SfxPoolItem& rAttr, xub_StrLen nStt,
                            xub_StrLen nEnd )
{
    if (rAttr.Which() == RES_TXTATR_INETFMT)
        return false;

    if( nStt == nEnd )
        return false;

    USHORT nPos;
    for( nPos = 0; nPos < Count(); ++nPos )
    {
        SttEndPos* pTmp = GetObject( nPos );
        if( pTmp->GetStart() == nStt && pTmp->GetEnd() == nEnd )
        {
            pTmp->AddAttr( rAttr );
            return false;       // schon vorhanden
        }
        if( nEnd < pTmp->GetEnd() )
        {
            SttEndPos* pNew = new SttEndPos( rAttr, nStt, nEnd );
            _EndPosLst::C40_INSERT( SttEndPos, pNew, nPos );
            return TRUE;
        }
    }

    SttEndPos* pNew = new SttEndPos( rAttr, nStt, nEnd );
    _EndPosLst::C40_INSERT( SttEndPos, pNew, nPos );
    return TRUE;
}

void RTFEndPosLst::OutScriptChange( xub_StrLen nStrPos )
{
    SttEndPos* pStt;
    for( USHORT n = 0, nEnd = Count(); n < nEnd; ++n )
        if( nStrPos == (pStt = GetObject( n ))->GetStart())
        {
            if( pStt->HasScriptChange() )
                OutAttrs( nStrPos );
            break;
        }
}

void RTFEndPosLst::OutAttrs( xub_StrLen nStrPos )
{
    SttEndPos* pStt;
    nCurPos = nStrPos;
    for( USHORT n = Count(); n ;  )
        if( nStrPos == (pStt = (*this)[ --n ])->GetStart() )
        {
            rWrt.Strm() << '{';
            for( USHORT i = 0; i < pStt->GetAttrs().Count(); ++i )
            {
                const SfxPoolItem* pItem = pStt->GetAttrs()[i];
                if( RES_FLTR_SCRIPTTYPE == pItem->Which() )
                    OutFontAttrs(*pItem);
                else
                    Out( aRTFAttrFnTab, *pItem, rWrt );
            }
        }

    nCurPos = STRING_NOTFOUND;
}

//Just a little decoding hack for the RES_FLTR_SCRIPTTYPE thing
void RTFEndPosLst::OutFontAttrs(const SfxPoolItem &rItem)
{
    sal_uInt32 nValue = ((const SfxUInt32Item&)rItem).GetValue();
    sal_uInt16 nScript = static_cast<sal_uInt16>(nValue >> 16);
    bool bBiDi = nValue & 0x1;
    OutFontAttrs(nScript, bBiDi);
}

void RTFEndPosLst::OutFontAttrs(USHORT nScript, bool bRTL)
{
    // script change, write the correct attributes:
    // start first with the Fontname

    rWrt.bOutFmtAttr = TRUE;
    nCurScript = nScript;
    rWrt.SetCurrScriptType( nScript );
    rWrt.SetAssociatedFlag(false);

    // the font MUST be at the first position !!!
    static const USHORT aLatinIds[] =
    {
        RES_CHRATR_FONT,
        RES_CHRATR_FONTSIZE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_POSTURE,  RES_CHRATR_WEIGHT,
        0
    };
    static const USHORT aAsianIds[] =
    {
        RES_CHRATR_CJK_FONT,
        RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CJK_WEIGHT,
        0
    };
    static const USHORT aCmplxIds[] =
    {
        RES_CHRATR_CTL_FONT,
        RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_LANGUAGE,
        RES_CHRATR_CTL_POSTURE,  RES_CHRATR_CTL_WEIGHT,
        0
    };

    /*
     You would have thought that
     rWrt.Strm() << (bRTL ? OOO_STRING_SVTOOLS_RTF_RTLCH : OOO_STRING_SVTOOLS_RTF_LTRCH); would be sufficent here ,
     but looks like word needs to see the other directional token to be
     satisified that all is kosher, otherwise it seems in ver 2003 to go and
     semi-randomlyly stick strike through about the place. Perhaps
     strikethrough is some ms developers "something is wrong signal" debugging
     code that we're triggering ?
    */
    if (bRTL)
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LTRCH;
        rWrt.Strm() << ' ';
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_RTLCH;
    }
    else
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_RTLCH;
        rWrt.Strm() << ' ';
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LTRCH;
    }

    // size/weight/posture optional
    const USHORT* pIdArr = 0;
    ByteString sOut;
    switch (nScript)
    {
        default:    //fall through
            ASSERT(pIdArr, "unknown script, strange");
        case i18n::ScriptType::LATIN:
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LOCH;
            pIdArr = aLatinIds;
            break;
        case i18n::ScriptType::ASIAN:
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_DBCH;
            pIdArr = aAsianIds;
            break;
        case i18n::ScriptType::COMPLEX:
            pIdArr = aCmplxIds;
            break;
    }

    for (const USHORT* pId = pIdArr; *pId; ++pId)
    {
        if (FnAttrOut pOut = aRTFAttrFnTab[ *pId - RES_CHRATR_BEGIN])
        {
            const SfxPoolItem* pItem = HasItem(*pId);
            if (!pItem)
                pItem = &GetPool().GetDefaultItem(*pId);
            (*pOut)(rWrt, *pItem);
        }
    }
}

void RTFEndPosLst::EndAttrs( xub_StrLen nStrPos )
{
    xub_StrLen nClipStart=STRING_MAXLEN;
    bool bClosed=false;
    SttEndPos* pSEPos;
    while( 0 != Count() && 0 != (pSEPos = GetObject( 0 )) &&
        ( STRING_MAXLEN == nStrPos || nStrPos == pSEPos->GetEnd() ))
    {
        const SfxPoolItems& rAttrs = pSEPos->GetAttrs();
        for( USHORT nAttr = rAttrs.Count(); nAttr; )
        {
            switch( rAttrs[ --nAttr ]->Which() )
            {
                case RES_TXTATR_CJK_RUBY:
                    rWrt.Strm() << ")}{" << OOO_STRING_SVTOOLS_RTF_FLDRSLT << " }}";
                    break;
            }
        }

        rWrt.Strm() << '}';     // end of all attributes from this position
        if (pSEPos->GetStart() < nClipStart)
            nClipStart = pSEPos->GetStart();
        bClosed=true;
        DeleteAndDestroy( 0, 1 );
    }

    if (bClosed)
    {
        //If there are open ranges whose start is before this point, and whose
        //start is after the cliping start then they have been closed whether
        //we wanted this or not. So accept that fact and then restart then
        //again
        USHORT nSize = Count();
        while (nSize > 0)
        {
            pSEPos = GetObject(--nSize);
            if ( pSEPos->GetStart() < nStrPos &&
                    pSEPos->GetStart() >= nClipStart)
            {
                rWrt.Strm() << '}';
            }
        }

        nSize = Count();
        USHORT n = 0;
        while (n < nSize)
        {
            SttEndPos* pStt = (*this)[n++];
            if (pStt->GetStart() < nStrPos && pStt->GetStart() >= nClipStart)
            {
                rWrt.Strm() << '{';
                for( USHORT i = 0; i < pStt->GetAttrs().Count(); ++i )
                {
                    const SfxPoolItem* pItem = pStt->GetAttrs()[i];
                    if( RES_FLTR_SCRIPTTYPE == pItem->Which() )
                        OutFontAttrs(*pItem);
                    else
                        Out( aRTFAttrFnTab, *pItem, rWrt );
                }
            }
        }
    }
}

BOOL RTFEndPosLst::MatchScriptToId( USHORT nWhich ) const
{
    BOOL bRet = false;
    switch( nWhich )
    {
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        bRet = nCurScript == i18n::ScriptType::LATIN;
        break;
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        bRet = nCurScript == i18n::ScriptType::ASIAN;
        break;
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        bRet = nCurScript == i18n::ScriptType::COMPLEX;
        break;
    }
    return bRet;
}

const SfxPoolItem& RTFEndPosLst::GetItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem = HasItem( nWhich );
    if( !pItem )
        pItem = &rNode.GetSwAttrSet().GetPool()->GetDefaultItem( nWhich );
    return *pItem;
}

const SfxPoolItem* RTFEndPosLst::HasItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem;
    if( RES_TXTATR_END > nWhich )
    {
        // it's a character/text attribute so look into the item array
        for( USHORT n = Count(); n; )
        {
            SttEndPos* pTmp = GetObject( --n );
            for( USHORT i = pTmp->GetAttrs().Count(); i; )
            {
                pItem = pTmp->GetAttrs()[ --i ];
                if( pItem->Which() == nWhich )
                    return pItem;

                // look into the charfmt?
                if( RES_TXTATR_CHARFMT == pItem->Which() &&
                    ((SwFmtCharFmt*) pItem)->GetCharFmt() &&
                    SFX_ITEM_SET == ((SwFmtCharFmt*) pItem)->GetCharFmt()->
                        GetItemState( nWhich, TRUE, &pItem ))
                    return pItem;
            }
        }
    }

    if( SFX_ITEM_SET != rNode.GetSwAttrSet().GetItemState(
                                nWhich, TRUE, &pItem ))
        pItem = 0;
    return pItem;
}

const SfxPoolItem& SwRTFWriter::GetItem( USHORT nWhich ) const
{
    if( pCurEndPosLst )
        return pCurEndPosLst->GetItem( nWhich );
    if( pAttrSet )
        return pAttrSet->Get( nWhich );

    return pDoc->GetAttrPool().GetDefaultItem( nWhich );
}

static void OutSvxFrmDir(SwRTFWriter& rRTFWrt, const SfxPoolItem& rHt )
{
    // write it only for pasgedesc's - not for frames
    SvxFrameDirectionItem aItem((const SvxFrameDirectionItem&)rHt);
    USHORT nVal = 0;
    const sal_Char* pStr = 0;
    bool bRTL = false;

    if (rRTFWrt.pFlyFmt)
        aItem.SetValue(rRTFWrt.TrueFrameDirection(*rRTFWrt.pFlyFmt));

    switch (aItem.GetValue())
    {
        case FRMDIR_ENVIRONMENT:
            ASSERT(0, "Not expected to see FRMDIR_ENVIRONMENT here");
            break;
        case FRMDIR_VERT_TOP_RIGHT:
            nVal = 1;
            pStr = OOO_STRING_SVTOOLS_RTF_FRMTXTBRLV;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            bRTL = true;
//          nVal = 3;
//          A val of three isn't working as expected in word :-( so leave it
//          as normal ltr 0 textflow with rtl sect property, neither does
//          the frame textflow
//          pStr = OOO_STRING_SVTOOLS_RTF_FRMTXTBRL;
            break;
        case FRMDIR_VERT_TOP_LEFT:
            nVal = 4;
            pStr = OOO_STRING_SVTOOLS_RTF_FRMTXLRTBV;
            break;
    }

    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax && pStr )
    {
        rRTFWrt.Strm() << pStr;
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    else if( rRTFWrt.bOutPageDesc)
    {
        if (nVal)
        {
            rRTFWrt.Strm() << OOO_STRING_SVTOOLS_RTF_STEXTFLOW;
            rRTFWrt.OutULong( nVal );
        }
        if (bRTL)
            rRTFWrt.Strm() << OOO_STRING_SVTOOLS_RTF_RTLSECT;
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    else if (!rRTFWrt.pFlyFmt && !rRTFWrt.bOutPageDesc)
    {
        rRTFWrt.Strm() << (bRTL ? OOO_STRING_SVTOOLS_RTF_RTLPAR : OOO_STRING_SVTOOLS_RTF_LTRPAR);
        rRTFWrt.bOutFmtAttr = TRUE;
    }
}

void OutRTF_SwRTL(SwRTFWriter& rWrt, const SwTxtNode *pNd)
{
    if (!pNd)
        return;
    SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
    if (const SvxFrameDirectionItem* pItem = (const SvxFrameDirectionItem*)
        pNd->GetSwAttrSet().GetItem(RES_FRAMEDIR))
    {
        eDir = static_cast<SvxFrameDirection>(pItem->GetValue());
    }
    if (eDir == FRMDIR_ENVIRONMENT)
    {
        SwPosition aPos(*pNd);
        eDir =
            static_cast<SvxFrameDirection>(rWrt.pDoc->GetTextDirection(aPos));
    }
    OutSvxFrmDir(rWrt, SvxFrameDirectionItem(eDir, RES_FRAMEDIR));
}

static Writer& OutRTF_SwTxtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtINetFmt& rURL = (const SwFmtINetFmt&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rURL.GetValue().Len() )
    {
        rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_FIELD << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE
                    << OOO_STRING_SVTOOLS_RTF_FLDINST << " HYPERLINK ";

        String sURL( rURL.GetValue() );
        if( INET_MARK_TOKEN != sURL.GetChar(0) )
        {
            INetURLObject aTmp( URIHelper::simpleNormalizedMakeRelative(rWrt.GetBaseURL(),
                                          sURL));


            sURL = aTmp.GetURLNoMark( INetURLObject::DECODE_UNAMBIGUOUS);
/*          if( INET_PROT_FILE == aTmp.GetProtocol() )
            {
                // WW97 wollen keine FILE-URL haben, sondern einen normalen
                // Dateinamen. Aber ab WW2000 kennen sie FileURLs.
                sURL = aTmp.GetFull();
            }
*/          rWrt.Strm() << '\"';
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, rRTFWrt.eCurrentEncoding,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
            sURL = aTmp.GetMark();
        }

        if( sURL.Len() )
        {
            rWrt.Strm() << "\\\\l \"";
            sURL.Erase( 0, 1 );
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, rRTFWrt.eCurrentEncoding,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        if( rURL.GetTargetFrame().Len() )
        {
            rWrt.Strm() << "\\\\t \"";
            RTFOutFuncs::Out_String( rWrt.Strm(), rURL.GetTargetFrame(),
                        rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        rWrt.Strm() << "}{" << OOO_STRING_SVTOOLS_RTF_FLDRSLT << ' ';
        rRTFWrt.bOutFmtAttr = false;

        // und dann noch die Attributierung ausgeben
        const SwCharFmt* pFmt;
        const SwTxtINetFmt* pTxtAtr = rURL.GetTxtINetFmt();
        if( pTxtAtr && 0 != ( pFmt = pTxtAtr->GetCharFmt() ))
            OutRTF_SwFmt( rWrt, *pFmt );
    }
    return rWrt;
}

void HandleHyperlinks(Writer& rWrt, const SwpHints* pTxtAttrs, xub_StrLen nPos )
{
    USHORT nCount = pTxtAttrs ? pTxtAttrs->Count() : 0;
    for(USHORT i = 0; i < nCount; ++i )
    {
        const SwTxtAttr* pHt = (*pTxtAttrs)[i];
        const SfxPoolItem &rItem = pHt->GetAttr();
        if (rItem.Which() == RES_TXTATR_INETFMT)
        {
            const xub_StrLen* pEnd;
            if (nPos == *pHt->GetStart())
                OutRTF_SwTxtINetFmt(rWrt, rItem);
            if (0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd)
            {
                // Hyperlinks werden als Felder geschrieben, aber der
                // "FieldResult" // steht als Text im TextNode. Also muss bei
                // diesen Attributen am // Ende 2 Klammern stehen!
                rWrt.Strm() << "}}";
            }
        }
    }
}

static Writer& OutRTF_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    xub_StrLen nStrPos = rRTFWrt.pCurPam->GetPoint()->nContent.GetIndex();
    RTFEndPosLst aEndPosLst( rRTFWrt, *pNd, nStrPos );
    USHORT nAttrPos = 0;

    const String& rStr = pNd->GetTxt();
    xub_StrLen nEnde = rStr.Len();
    if( rRTFWrt.pCurPam->GetPoint()->nNode == rRTFWrt.pCurPam->GetMark()->nNode )
        nEnde = Min( nEnde, rRTFWrt.pCurPam->GetMark()->nContent.GetIndex() );

    int bNewFmts = rRTFWrt.GetAttrSet() != &pNd->GetSwAttrSet();
    if( bNewFmts )
    {
        // harte Attributierung am Node und am Vorgaenger ?
        const SfxItemSet* pNdSet = pNd->GetpSwAttrSet();
        if( pNdSet && rRTFWrt.GetAttrSet() && rRTFWrt.bAutoAttrSet &&
            pNdSet->GetParent() == rRTFWrt.GetAttrSet()->GetParent() &&
            pNdSet->Count() == rRTFWrt.GetAttrSet()->Count() )
        {
            // die beiden Parents sind gleich, dann teste doch mal die
            // Attribute im Set

            int bEqual = TRUE;
            if( pNdSet->Count() )
            {
                SfxItemIter aIter( *rRTFWrt.GetAttrSet() );
                const SfxPoolItem *pItem, *pCurr = aIter.GetCurItem();
                while( TRUE )
                {
                    if( SFX_ITEM_SET != pNdSet->GetItemState( pCurr->Which(),
                        false, &pItem ) || *pItem != *pCurr )
                    {
                        bEqual = false;
                        break;
                    }

                    if( aIter.IsAtEnd() )
                        break;
                    pCurr = aIter.NextItem();
                }
            }
            if (bEqual)
                bNewFmts = false;
        }
        rRTFWrt.SetAttrSet( &pNd->GetSwAttrSet() );
        rRTFWrt.bAutoAttrSet = 0 != pNdSet;
    }

    // Flag zuruecksetzen, damit nach der Ausgabe der Collection
    // getestet werden kann, ob noch ein Blank auszugeben ist
    rRTFWrt.bOutFmtAttr = false;

    // in der Ausgabe eines Flys? Dann vorm ausgeben der AbsatzAttribute
    // den Format-Pointer auf 0 setzen!
    const SwFlyFrmFmt* pSaveFmt = rRTFWrt.pFlyFmt;

    SfxItemSet aMergedSet(rRTFWrt.pDoc->GetAttrPool(), POOLATTR_BEGIN,
        POOLATTR_END-1);
    bool bDeep = false;

    if( rRTFWrt.bWriteAll )
    {
        rRTFWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_PLAIN << ' ';       // alle Attribute zuruecksetzen
        if( rRTFWrt.bOutTable )
            rRTFWrt.Strm() << OOO_STRING_SVTOOLS_RTF_INTBL;

        // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
        // muessen noch die Attribute dafuer ausgegeben werden.
        if( pSaveFmt )
        {
            OutRTF_SwFlyFrmFmt( rRTFWrt );
            rRTFWrt.pFlyFmt = 0;
        }

        rRTFWrt.OutListNum( *pNd );
        OutRTF_SwRTL(rRTFWrt, pNd);
        SwFmtToSet(rRTFWrt, pNd->GetAnyFmtColl(), aMergedSet);
        bDeep = true;
    }
    else if( !rRTFWrt.bWriteAll && rRTFWrt.bFirstLine )
    {
        OutRTF_SwRTL(rRTFWrt, pNd);
        SwFmtToSet(rRTFWrt, pNd->GetAnyFmtColl(), aMergedSet);
        bDeep = true;
    }

    // gibt es harte Attributierung ?
    if( bNewFmts && pNd->HasSwAttrSet())
    {
        rRTFWrt.pFlyFmt = 0;

        const SfxItemSet& rNdSet = pNd->GetSwAttrSet();

        const SwNumRule* pRule = pNd->GetNumRule();
        // --> OD 2008-03-19 #refactorlists#
        if ( pRule && pNd->IsInList() )
        // <--
        {
            // --> OD 2008-03-18 #refactorlists#
            ASSERT( pNd->GetActualListLevel() >= 0 && pNd->GetActualListLevel() < MAXLEVEL,
                    "<OutRTF_SwTxtNode(..)> - text node does not have valid list level. Serious defect -> please inform OD" );
            // <--
            BYTE nLvl = static_cast< BYTE >(pNd->GetActualListLevel());
            const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
            if( !pFmt )
                pFmt = &pRule->Get( nLvl );

            SfxItemSet aSet( rNdSet );
            SvxLRSpaceItem aLR( (SvxLRSpaceItem&)rNdSet.Get( RES_LR_SPACE ) );

            aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );
            if( MAXLEVEL > pNd->GetActualListLevel() )
                aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
            else
                aSet.ClearItem( RES_PARATR_NUMRULE );
            aSet.Put( aLR );
            aMergedSet.Put(aSet);
        }
        else
            aMergedSet.Put(rNdSet);
    }

    SwTxtNode *txtNode=rNode.GetTxtNode();
    if (txtNode!=NULL && !txtNode->IsNumbered())
    {
        aMergedSet.ClearItem(RES_PARATR_NUMRULE);
    }
    OutRTF_SfxItemSet(rRTFWrt, aMergedSet, bDeep);

    rRTFWrt.pFlyFmt = pSaveFmt;

    rRTFWrt.bTxtAttr = true;
    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    const SwTxtAttr * pHt = 0;
    USHORT nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        // Ok, es gibt vorher Attribute, die ausgegeben werden muessen
        do {
            nAttrPos++;
            if( RES_TXTATR_FIELD == pHt->Which() )      // Felder nicht
                continue;                               // ausgeben

            if( pHt->GetEnd() )
            {
                xub_StrLen nHtEnd = *pHt->GetEnd(), nHtStt = *pHt->GetStart();
                if( !rRTFWrt.bWriteAll && nHtEnd <= nStrPos )
                    continue;

                // leere Hints am Anfang nicht beachten, oder ??
                if( nHtEnd == nHtStt )
                    continue;

                // Attribut in die Liste aufnehemen
                if( !rRTFWrt.bWriteAll )
                {
                    if( nHtStt < nStrPos ) nHtStt = nStrPos;
                    if( nHtEnd >= nEnde ) nHtEnd = nEnde;
                }
                aEndPosLst.Insert( pHt->GetAttr(), nHtStt, nHtEnd );
                continue;
                // aber nicht ausgeben, das erfolgt spaeter !!
            }
            Out( aRTFAttrFnTab, pHt->GetAttr(), rRTFWrt );

        } while( nAttrPos < nCntAttr && nStrPos >
            *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

        // dann gebe mal alle gesammelten Attribute von der String-Pos aus
        aEndPosLst.OutAttrs( nStrPos );
    }

    if( rRTFWrt.bOutFmtAttr &&
        ( nAttrPos < nCntAttr ? *pHt->GetStart() != nStrPos : TRUE ))
        rRTFWrt.Strm() << ' ';

    // das Flag gibt an, ob das SwTxtFld am Ende vom Absatz steht. Denn
    // dann ist vor dem Absatzumbruch die schliessende Klammer auszugeben
    xub_StrLen nChrCnt = 0;
    for( ; nStrPos <= nEnde; nStrPos++ )
    {
        rRTFWrt.bOutFmtAttr = false;
        if( nStrPos != nEnde && aEndPosLst.Count() )
            aEndPosLst.EndAttrs( nStrPos );

        // versuche nach ungefaehr 255 Zeichen eine neue Zeile zu beginnen
        if( nChrCnt != ( nStrPos & 0xff00 ))
        {
            rWrt.Strm() << SwRTFWriter::sNewLine;
            nChrCnt = nStrPos & 0xff00;
        }

        if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
            && nStrPos != nEnde )
        {
            do {
                BOOL bEmpty = false;
                if( pHt->GetEnd() )
                {
                    if (false == (bEmpty = *pHt->GetEnd() == nStrPos))
                    {
                        aEndPosLst.Insert( pHt->GetAttr(), nStrPos,
                                            *pHt->GetEnd() );
                        continue;
                    }
                    rRTFWrt.Strm() << '{';
                }
                Out( aRTFAttrFnTab, pHt->GetAttr(), rRTFWrt );
                if( bEmpty )
                {
                    rRTFWrt.Strm() << '}';
                    rRTFWrt.bOutFmtAttr = false;
                }
            } while( ++nAttrPos < nCntAttr && nStrPos ==
                *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

            // dann gebe mal alle gesammelten Attribute von der String-Pos aus
            aEndPosLst.OutAttrs( nStrPos );

        }
        else
            aEndPosLst.OutScriptChange( nStrPos );

        HandleHyperlinks(rWrt, pNd->GetpSwpHints(), nStrPos);

        if( rRTFWrt.bOutFmtAttr )
            rRTFWrt.Strm() << ' ';

        rRTFWrt.OutBookmarks( nStrPos );

        rRTFWrt.OutRedline( nStrPos);

        if (nStrPos != nEnde)
        {
            RTFOutFuncs::Out_String(rWrt.Strm(), String(rStr.GetChar(nStrPos)),
                rRTFWrt.eCurrentEncoding, rRTFWrt.bWriteHelpFmt);
        }
    }

    rRTFWrt.bTxtAttr = false;

    // noch eine schliesende Klammer da ??
    if( aEndPosLst.Count() )
        aEndPosLst.EndAttrs( USHRT_MAX );

    // wenn bis zum Ende vom Node, dann auch das AbsatzEnde ausgeben
    if( rRTFWrt.pCurPam->GetMark()->nNode.GetIndex() ==
        rRTFWrt.pCurPam->GetPoint()->nNode.GetIndex() )
    {
        if( pNd->Len() != rRTFWrt.pCurPam->GetMark()->nContent.GetIndex() )
            return rWrt;

        if( rRTFWrt.bOutTable )
        {
            rRTFWrt.Strm() << SwRTFWriter::sNewLine;
            return rWrt;
        }
    }

    rRTFWrt.Strm() << SwRTFWriter::sNewLine << OOO_STRING_SVTOOLS_RTF_PAR << ' ';
    return rRTFWrt;
}

bool IsEMF(const sal_uInt8 *pGraphicAry, unsigned long nSize)
{
    if (pGraphicAry && (nSize > 0x2c ))
    {
        // check the magic number
        if (
             (pGraphicAry[0x28] == 0x20 ) && (pGraphicAry[0x29] == 0x45) &&
             (pGraphicAry[0x2a] == 0x4d ) && (pGraphicAry[0x2b] == 0x46)
           )
        {
            //emf detected
            return true;
        }
    }
    return false;
}

bool StripMetafileHeader(const sal_uInt8 *&rpGraphicAry, unsigned long &rSize)
{
    if (rpGraphicAry && (rSize > 0x22))
    {
        if (
             (rpGraphicAry[0] == 0xd7) && (rpGraphicAry[1] == 0xcd) &&
             (rpGraphicAry[2] == 0xc6) && (rpGraphicAry[3] == 0x9a)
           )
        {   // we have to get rid of the metafileheader
            rpGraphicAry += 22;
            rSize -= 22;
            return true;
        }
    }
    return false;
}

void ExportPICT(const Size &rOrig, const Size &rRendered, const Size &rMapped,
    const SwCropGrf &rCr, const char *pBLIPType, const sal_uInt8 *pGraphicAry,
    unsigned long nSize, SwRTFWriter &rWrt)
{
    bool bIsWMF = (const char *)pBLIPType == (const char *)OOO_STRING_SVTOOLS_RTF_WMETAFILE ? true : false;
    if (pBLIPType && nSize && pGraphicAry)
    {
        rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_PICT;

        long nXCroppedSize = rOrig.Width()-(rCr.GetLeft() + rCr.GetRight());
        long nYCroppedSize = rOrig.Height()-(rCr.GetTop() + rCr.GetBottom());
        /* #127543#: Graphic with a zero height or width, typically copied from webpages, caused
        crashes. */
        if( !nXCroppedSize )
            nXCroppedSize = 100;
        if( !nYCroppedSize )
            nYCroppedSize = 100;

        //Given the original size and taking cropping into account
        //first, how much has the original been scaled to get the
        //final rendered size
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICSCALEX;
        rWrt.OutLong((100 * rRendered.Width()) / nXCroppedSize);
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICSCALEY;
        rWrt.OutLong((100 * rRendered.Height()) / nYCroppedSize);

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICCROPL;
        rWrt.OutLong(rCr.GetLeft());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICCROPR;
        rWrt.OutLong(rCr.GetRight());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICCROPT;
        rWrt.OutLong(rCr.GetTop());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICCROPB;
        rWrt.OutLong(rCr.GetBottom());

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICW;
        rWrt.OutLong(rMapped.Width());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICH;
        rWrt.OutLong(rMapped.Height());

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICWGOAL;
        rWrt.OutLong(rOrig.Width());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PICHGOAL;
        rWrt.OutLong(rOrig.Height());

        rWrt.Strm() << pBLIPType;
        if (bIsWMF)
        {
            rWrt.OutLong(8);
            StripMetafileHeader(pGraphicAry, nSize);
        }
        rWrt.Strm() << SwRTFWriter::sNewLine;

        sal_uInt32 nBreak = 0;
        for (sal_uInt32 nI = 0; nI < nSize; ++nI)
        {
            ByteString sNo = ByteString::CreateFromInt32(pGraphicAry[nI], 16);
            if (sNo.Len() < 2)
                rWrt.Strm() << '0';
            rWrt.Strm() << sNo.GetBuffer();
            if (++nBreak == 64)
            {
                rWrt.Strm() << SwRTFWriter::sNewLine;
                nBreak = 0;
            }
        }

        rWrt.Strm() << '}';
    }
}

static Writer& OutRTF_SwGrfNode(Writer& rWrt, SwCntntNode & rNode)
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    SwGrfNode &rNd = (SwGrfNode&)rNode;

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if (rRTFWrt.pFlyFmt && !ExportAsInline(*rRTFWrt.pFlyFmt))
        OutRTF_SwFlyFrmFmt(rRTFWrt);    //"classic" positioning and size export

#if 1
    SvMemoryStream aStream;
    const sal_uInt8* pGraphicAry = 0;
    sal_uInt32 nSize = 0;

    Graphic aGraphic(rNd.GetGrf());

    // If there is no graphic there is not much point in parsing it
    if(aGraphic.GetType()==GRAPHIC_NONE)
        return rRTFWrt;

    GfxLink aGraphicLink;
    const sal_Char* pBLIPType = 0;
    if (aGraphic.IsLink())
    {
        aGraphicLink = aGraphic.GetLink();
        nSize = aGraphicLink.GetDataSize();
        pGraphicAry = aGraphicLink.GetData();
        switch (aGraphicLink.GetType())
        {
            case GFX_LINK_TYPE_NATIVE_JPG:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_JPEGBLIP;
                break;
            case GFX_LINK_TYPE_NATIVE_PNG:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
            case GFX_LINK_TYPE_NATIVE_WMF:
                pBLIPType =
                    IsEMF(pGraphicAry, nSize) ? OOO_STRING_SVTOOLS_RTF_EMFBLIP : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
                break;
            default:
                break;
        }
    }

    GraphicType eGraphicType = aGraphic.GetType();
    if (!pGraphicAry)
    {
        if (ERRCODE_NONE == GraphicConverter::Export(aStream, aGraphic,
            (eGraphicType == GRAPHIC_BITMAP) ? CVT_PNG : CVT_WMF))
        {
            pBLIPType = (eGraphicType == GRAPHIC_BITMAP) ?
                OOO_STRING_SVTOOLS_RTF_PNGBLIP : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
            aStream.Seek(STREAM_SEEK_TO_END);
            nSize = aStream.Tell();
            pGraphicAry = (sal_uInt8*)aStream.GetData();
        }
    }

    Size aMapped(eGraphicType == GRAPHIC_BITMAP ? aGraphic.GetSizePixel() : aGraphic.GetPrefSize());

    const SwCropGrf &rCr = (const SwCropGrf &)rNd.GetAttr(RES_GRFATR_CROPGRF);

    //Get original size in twips
    Size aSize(sw::util::GetSwappedInSize(rNd));
    Size aRendered(aSize);
    if (rRTFWrt.pFlyFmt)
    {
        const SwFmtFrmSize& rS = rRTFWrt.pFlyFmt->GetFrmSize();
        aRendered.Width() = rS.GetWidth();
        aRendered.Height() = rS.GetHeight();
    }

    /*
     If the graphic is not of type WMF then we will have to store two
     graphics, one in the native format wrapped in shppict, and the other in
     the wmf format wrapped in nonshppict, so as to keep wordpad happy. If its
     a wmf already then we don't need any such wrapping
    */
    bool bIsWMF = (const sal_Char*)pBLIPType == (const sal_Char*)OOO_STRING_SVTOOLS_RTF_WMETAFILE ? true : false;
    if (!bIsWMF)
        OutComment(rRTFWrt, OOO_STRING_SVTOOLS_RTF_SHPPICT);

    if (pBLIPType)
        ExportPICT(aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, rRTFWrt);
    else
    {
        aStream.Seek(0);
        GraphicConverter::Export(aStream, aGraphic, CVT_WMF);
        pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
        aStream.Seek(STREAM_SEEK_TO_END);
        nSize = aStream.Tell();
        pGraphicAry = (sal_uInt8*)aStream.GetData();

        ExportPICT(aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize,
            rRTFWrt);
    }

    if (!bIsWMF)
    {
        rRTFWrt.Strm() << '}' << '{' << OOO_STRING_SVTOOLS_RTF_NONSHPPICT;

        aStream.Seek(0);
        GraphicConverter::Export(aStream, aGraphic, CVT_WMF);
        pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
        aStream.Seek(STREAM_SEEK_TO_END);
        nSize = aStream.Tell();
        pGraphicAry = (sal_uInt8*)aStream.GetData();

        ExportPICT(aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize,
            rRTFWrt);

        rRTFWrt.Strm() << '}';
    }


    rRTFWrt.Strm() << SwRTFWriter::sNewLine;
#else
    rRTFWrt.Strm() << "{{";

    // damit die eigenen Grafik-Attribute nach der PICT / import Anweisung
    // stehen, muessen die am Anfang ausgegeben werden.
    rRTFWrt.bOutFmtAttr = false;
    OutRTF_SwFmt( rRTFWrt, *pNd->GetFmtColl() );

    if( rRTFWrt.bOutFmtAttr )       // wurde ueberhaupt ein Attrribut
        rRTFWrt.Strm() << "}{";     // ausgegeben ??

    String aGrfNm;
    const SwMirrorGrf& rMirror = pNd->GetSwAttrSet().GetMirrorGrf();
    if( !pNd->IsLinkedFile() || RES_MIRROR_GRAPH_DONT != rMirror.GetValue() )
    {
        USHORT nErr = 1;
        // Grafik als File-Referenz speichern (als JPEG-Grafik speichern)
        // but only if we save into a file and have a URL
        if( rWrt.GetOrigFileName() )
        {
            aGrfNm = *rWrt.GetOrigFileName();
            pNd->SwapIn( TRUE );
            ULONG nFlags = XOUTBMP_USE_NATIVE_IF_POSSIBLE;
            switch( rMirror.GetValue() )
            {
            case RES_MIRROR_GRAPH_VERT: nFlags = XOUTBMP_MIRROR_HORZ; break;
            case RES_MIRROR_GRAPH_HOR:    nFlags = XOUTBMP_MIRROR_VERT; break;
            case RES_MIRROR_GRAPH_BOTH:
                nFlags = XOUTBMP_MIRROR_VERT | XOUTBMP_MIRROR_HORZ;
                break;
            }

            Size aMM100Size;
            Size* pMM100Size = 0;
            if( rRTFWrt.pFlyFmt )
            {
                const SwFmtFrmSize& rSize = rRTFWrt.pFlyFmt->GetFrmSize();
                aMM100Size = OutputDevice::LogicToLogic( rSize.GetSize(),
                                MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ));
                pMM100Size = &aMM100Size;
            }

            nErr = XOutBitmap::WriteGraphic( pNd->GetGrf(), aGrfNm,
                    String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "JPG" )),
                                                    nFlags, pMM100Size );
        }
        if( nErr )              // fehlerhaft, da ist nichts auszugeben
        {
            rRTFWrt.Strm() << "}}";
            return rWrt;
        }
    }
    else
        pNd->GetFileFilterNms( &aGrfNm, 0 );

    // MIB->JP: Warum erst AbsToRel und dann das URL-Objekt? So
    // kommt bei relativierbaren URLs als Protokoll "unknown" raus.
    // Ist das Absicht?
    aGrfNm = INetURLObject::AbsToRel( aGrfNm, INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS);
    INetURLObject aUrl( aGrfNm );
    if( aUrl.GetProtocol() == INET_PROT_FILE )
        aGrfNm = aUrl.PathToFileName();

    // Bitmap als File-Referenz speichern
    rRTFWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FIELD << OOO_STRING_SVTOOLS_RTF_FLDPRIV;
    OutComment( rRTFWrt, OOO_STRING_SVTOOLS_RTF_FLDINST ) << "{\\\\import ";
    RTFOutFuncs::Out_String( rWrt.Strm(), aGrfNm, rRTFWrt.eDefaultEncoding,
                                rRTFWrt.bWriteHelpFmt );
    rRTFWrt.Strm() << "}}{" << OOO_STRING_SVTOOLS_RTF_FLDRSLT << " }}";
    rRTFWrt.Strm() << '}' << SwRTFWriter::sNewLine;
#endif
    return rRTFWrt;
}

static Writer& OutRTF_SwOLENode( Writer& rWrt, SwCntntNode & /*rNode*/ )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rRTFWrt.pFlyFmt )
        OutRTF_SwFlyFrmFmt( rRTFWrt );

    rWrt.Strm() << SwRTFWriter::sNewLine << OOO_STRING_SVTOOLS_RTF_PAR;
    return rWrt;
}

static void OutTBLBorderLine(SwRTFWriter& rWrt, const SvxBorderLine* pLine,
    const sal_Char* pStr)
{
    ByteString sLineStr;
    if( pLine->GetInWidth() )
    {
        // doppelte Linie
        sLineStr = OOO_STRING_SVTOOLS_RTF_BRDRDB;
        switch( pLine->GetInWidth() )
        {
        case DEF_LINE_WIDTH_0:
            ( sLineStr += OOO_STRING_SVTOOLS_RTF_BRDRW ) += "15";
            break;
        case DEF_LINE_WIDTH_1:
            ( sLineStr += OOO_STRING_SVTOOLS_RTF_BRDRW ) += "30";
            break;
        case DEF_LINE_WIDTH_2:
        case DEF_LINE_WIDTH_3:
            ( sLineStr += OOO_STRING_SVTOOLS_RTF_BRDRW ) += "45";
            break;
        }
    }
    else
    {
        // einfache Linie
        if( DEF_LINE_WIDTH_1 >= pLine->GetOutWidth() )
            (( sLineStr = OOO_STRING_SVTOOLS_RTF_BRDRS ) += OOO_STRING_SVTOOLS_RTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() );
        else
            (( sLineStr = OOO_STRING_SVTOOLS_RTF_BRDRTH ) += OOO_STRING_SVTOOLS_RTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() / 2 );
    }

    rWrt.Strm() << pStr << sLineStr.GetBuffer() << OOO_STRING_SVTOOLS_RTF_BRDRCF;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) );
}

static void OutBorderLine(SwRTFWriter& rWrt, const SvxBorderLine* pLine,
    const sal_Char* pStr, USHORT nDist)
{
    OutTBLBorderLine(rWrt, pLine, pStr);
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_BRSP;
    rWrt.OutULong( nDist );
}

static void OutSwTblBorder(SwRTFWriter& rWrt, const SvxBoxItem& rBox,
    const SvxBoxItem *pDefault)
{
    static const USHORT aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
#ifdef __MINGW32__
    static const char* aBorderNames[] __attribute__((section(".data"))) =
#else
    static const char* aBorderNames[] =
#endif
    {
        OOO_STRING_SVTOOLS_RTF_CLBRDRT, OOO_STRING_SVTOOLS_RTF_CLBRDRL, OOO_STRING_SVTOOLS_RTF_CLBRDRB, OOO_STRING_SVTOOLS_RTF_CLBRDRR
    };
    //Yes left and top are swapped with eachother for cell padding! Because
    //that's what the thunderingly annoying rtf export/import word xp does.
#ifdef __MINGW32__
    static const char* aCellPadNames[] __attribute__((section(".data"))) =
#else
    static const char* aCellPadNames[] =
#endif
    {
        OOO_STRING_SVTOOLS_RTF_CLPADL, OOO_STRING_SVTOOLS_RTF_CLPADT, OOO_STRING_SVTOOLS_RTF_CLPADB, OOO_STRING_SVTOOLS_RTF_CLPADR
    };
#ifdef __MINGW32__
    static const char* aCellPadUnits[] __attribute__((section(".data"))) =
#else
    static const char* aCellPadUnits[] =
#endif
    {
        OOO_STRING_SVTOOLS_RTF_CLPADFL, OOO_STRING_SVTOOLS_RTF_CLPADFT, OOO_STRING_SVTOOLS_RTF_CLPADFB, OOO_STRING_SVTOOLS_RTF_CLPADFR
    };
    for (int i = 0; i < 4; ++i)
    {
        if (const SvxBorderLine* pLn = rBox.GetLine(aBorders[i]))
            OutTBLBorderLine(rWrt, pLn, aBorderNames[i]);
        if (!pDefault || pDefault->GetDistance(aBorders[i]) !=
            rBox.GetDistance(aBorders[i]))
        {
            rWrt.Strm() << aCellPadUnits[i];
            rWrt.OutULong(3);
            rWrt.Strm() << aCellPadNames[i];
            rWrt.OutULong(rBox.GetDistance(aBorders[i]));
        }
    }
}

static void OutSwTblBackground( SwRTFWriter& rWrt, const SvxBrushItem& rBack )
{
    if( !rBack.GetColor().GetTransparency() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CLCBPAT;
        rWrt.OutULong( rWrt.GetId( rBack.GetColor() ) );
    }
}


Writer& OutRTF_SwTblNode(Writer& rWrt, const SwTableNode & rNode)
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwTable& rTbl = rNode.GetTable();
    SwTwips nPageSize = 0, nTblOffset = 0;
    const bool bNewTableModel = rTbl.IsNewModel();

/*
//!!!!!!!!!!!!! for clipboard create any view if the table is complex !!!
    if( rTbl.IsTblComplex() )
    {
        // then we have to create any layout
        SFX_APP()->CreateViewFrame( *xDocSh, 0, TRUE );
    }
*/

    const SwFrmFmt *pFmt = rTbl.GetFrmFmt();
    ASSERT(pFmt, "Impossible");
    {
        Point aPt;
        SwRect aRect( pFmt->FindLayoutRect( false, &aPt ));
        if( aRect.IsEmpty() )
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pFrmFmt = rRTFWrt.pFlyFmt ? rRTFWrt.pFlyFmt :
                const_cast<const SwDoc *>(rWrt.pDoc)
                ->GetPageDesc(0).GetPageFmtOfNode(rNode, false);

            aRect = pFrmFmt->FindLayoutRect( TRUE );
            if( 0 == ( nPageSize = aRect.Width() ))
            {
                const SvxLRSpaceItem& rLR = pFrmFmt->GetLRSpace();
                nPageSize = pFrmFmt->GetFrmSize().GetWidth() -
                                rLR.GetLeft() - rLR.GetRight();
            }
        }
        else
            nPageSize = aRect.Width();
    }

    SwTwips nTblSz = pFmt->GetFrmSize().GetWidth();

    ByteString aTblAdjust( OOO_STRING_SVTOOLS_RTF_TRQL );
    switch (pFmt->GetHoriOrient().GetHoriOrient())
    {
        case text::HoriOrientation::CENTER:
            aTblAdjust = OOO_STRING_SVTOOLS_RTF_TRQC;
            break;
        case text::HoriOrientation::RIGHT:
            aTblAdjust = OOO_STRING_SVTOOLS_RTF_TRQR;
            break;
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
            {
                const SvxLRSpaceItem& rLRSp = pFmt->GetLRSpace();
                nTblOffset = rLRSp.GetLeft();
                nPageSize -= (nTblOffset + rLRSp.GetRight());
                aTblAdjust += OOO_STRING_SVTOOLS_RTF_TRLEFT;
                aTblAdjust += ByteString::CreateFromInt32( nTblOffset );
            }
            break;
        default:
            break;
    }

    if (rRTFWrt.TrueFrameDirection(*pFmt) == FRMDIR_HORI_RIGHT_TOP)
        aTblAdjust += OOO_STRING_SVTOOLS_RTF_RTLROW;

    // ist die Tabelle wesentlich (PageSize + 10%) groesser als die Seite,
    // dann sind die Box-Breiten relative Angaben.
    BOOL bRelBoxSize = TRUE /*ALWAYS relativ (nPageSize + ( nPageSize / 10 )) < nTblSz*/;

    SwWriteTable* pTableWrt;
    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        pTableWrt = new SwWriteTable( pLayout );
    else
        pTableWrt = new SwWriteTable(rTbl.GetTabLines(), (USHORT)nPageSize,
                                          (USHORT)nTblSz, false);

    // rCols are the array of all cols of the table
    const SwWriteTableCols& rCols = pTableWrt->GetCols();
    USHORT nColCnt = rCols.Count();
    SwWriteTableCellPtr* pBoxArr = new SwWriteTableCellPtr[ nColCnt ];
    USHORT* pRowSpans = new USHORT[ nColCnt ];
    memset( pBoxArr, 0, sizeof( pBoxArr[0] ) * nColCnt );
    memset( pRowSpans, 0, sizeof( pRowSpans[0] ) * nColCnt );
    const SwWriteTableRows& rRows = pTableWrt->GetRows();
    for( USHORT nLine = 0; nLine < rRows.Count(); ++nLine )
    {
        USHORT nBox;

        const SwWriteTableRow *pRow = rRows[ nLine ];
        const SwWriteTableCells& rCells = pRow->GetCells();

        BOOL bFixRowHeight = false;

        USHORT nBoxes = rCells.Count();
        if (nColCnt < nBoxes)
            nBoxes = nColCnt;

        for( nColCnt = 0, nBox = 0; nBox < rCells.Count() && nColCnt < nBoxes; ++nColCnt )
        {
            SwWriteTableCell* pCell = rCells[ nBox ];
            const bool bProcessCoveredCell = bNewTableModel && 0 == pCell->GetRowSpan();

            if( !pRowSpans[ nColCnt ] || bProcessCoveredCell )
            {
                // set new BoxPtr
                nBox++;
                pBoxArr[ nColCnt ] = pCell;
                if ( !bProcessCoveredCell )
                    pRowSpans[ nColCnt ] = pCell->GetRowSpan();
                for( USHORT nCellSpan = pCell->GetColSpan(), nCS = 1;
                        nCS < nCellSpan; ++nCS, ++nColCnt )
                {
                    ASSERT( nColCnt+1 < rCols.Count(), "More colspan than columns" );
                    if( nColCnt+1 < rCols.Count() ) // robust against wrong colspans
                    {
                        pBoxArr[ nColCnt+1 ] = pBoxArr[ nColCnt ];
                        pRowSpans[ nColCnt+1 ] = pRowSpans[ nColCnt ];
                    }
                }
            }
            if( 1 != pRowSpans[ nColCnt ] && !bNewTableModel )
                bFixRowHeight = TRUE;
        }

        for( ; nColCnt < rCols.Count() && pRowSpans[ nColCnt ]; ++nColCnt )
            bFixRowHeight = TRUE;

        nColCnt = rCols.Count(); // A wrong cellspan-value could cause a nColCnt > rCols.Count()

        // Start Tabellendefinition
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TROWD << aTblAdjust.GetBuffer();

        if(  rTbl.GetRowsToRepeat() > nLine )
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TRHDR;

        const SwTableLine* pLine = pBoxArr[ 0 ]->GetBox()->GetUpper();
        // Zeilenhoehe ausgeben
        long nHeight = 0;
        if( bFixRowHeight && rWrt.pDoc->GetRootFrm() )
        {
            nHeight = -pRow->GetPos();      //neg. => abs. height!
            if( nLine )
                nHeight += rRows[ nLine - 1 ]->GetPos();
        }
        else
        {
            const SwFmtFrmSize& rLSz = pLine->GetFrmFmt()->GetFrmSize();
            if( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
                nHeight = ATT_MIN_SIZE == rLSz.GetHeightSizeType()
                                                ? rLSz.GetHeight()
                                                : -rLSz.GetHeight();
        }

        //The rtf default is to allow a row to break, so if we are not
        //splittable export TRKEEP
        const SwFrmFmt *pLineFmt = pLine ? pLine->GetFrmFmt() : 0;
        if (!pLineFmt || pLineFmt->GetRowSplit().GetValue() == 0)
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TRKEEP;

        if( nHeight )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TRRH;
            rWrt.OutLong( nHeight );
        }

        const SvxBoxItem *pDefaultBox = 0;
        if (nColCnt)
        {
            pDefaultBox = &(pBoxArr[0]->GetBox()->GetFrmFmt()->GetBox());

            static const USHORT aBorders[] =
            {
                BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
            };
#ifdef __MINGW32__
            static const char* aRowPadNames[] __attribute__((section(".data"))) =
#else
            static const char* aRowPadNames[] =
#endif
            {
                OOO_STRING_SVTOOLS_RTF_TRPADDT, OOO_STRING_SVTOOLS_RTF_TRPADDL, OOO_STRING_SVTOOLS_RTF_TRPADDB, OOO_STRING_SVTOOLS_RTF_TRPADDR
            };
            static const char* aRowPadUnits[] =
            {
                OOO_STRING_SVTOOLS_RTF_TRPADDFT, OOO_STRING_SVTOOLS_RTF_TRPADDFL, OOO_STRING_SVTOOLS_RTF_TRPADDFB, OOO_STRING_SVTOOLS_RTF_TRPADDFR
            };
            for (int i = 0; i < 4; ++i)
            {
                rWrt.Strm() << aRowPadUnits[i];
                rWrt.OutULong(3);
                rWrt.Strm() << aRowPadNames[i];
                rWrt.OutULong(pDefaultBox->GetDistance(aBorders[i]));
            }
        }

        // Breite der Boxen ausgeben
        SwTwips nSz = 0, nCalc;
        for( nBox = 0; nBox < nColCnt; ++nBox )
        {
            SwWriteTableCell* pCell = pBoxArr[ nBox ];
            if( (nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ]) || (pCell == NULL) )
                continue;

            const SwFrmFmt& rFmt = *pCell->GetBox()->GetFrmFmt();
            if( 1 < pCell->GetRowSpan() || 0 == pCell->GetRowSpan() )
                rWrt.Strm() << ( pCell->GetRowSpan() == pRowSpans[ nBox ]
                                ? OOO_STRING_SVTOOLS_RTF_CLVMGF
                                : OOO_STRING_SVTOOLS_RTF_CLVMRG );

            const SfxPoolItem* pItem;
            if (SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(RES_BOX, TRUE,
                &pItem))
            {
                OutSwTblBorder(rRTFWrt, (SvxBoxItem&)*pItem, pDefaultBox);
            }

// RTF kennt Schattierung in unserem Sinne nicht!
//              if( SFX_ITEM_SET == pBoxFmt->GetAttrSet().GetItemState(
//                                          RES_SHADOW, TRUE, &pItem ) )
//                  OutSwTblShadow( rRTFWrt, *pItem );

            if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
                                            RES_BACKGROUND, TRUE, &pItem )
                || 0 != ( pItem = pCell->GetBackground() )
                || 0 != ( pItem = pRow->GetBackground() ) )
                OutSwTblBackground( rRTFWrt, (SvxBrushItem&)*pItem );

            if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
                                            RES_VERT_ORIENT, TRUE, &pItem ) )
                switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
                {
                case text::VertOrientation::CENTER:   rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CLVERTALC; break;
                case text::VertOrientation::BOTTOM:   rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CLVERTALB; break;
                default:            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CLVERTALT; break;
                }

            const SwFmtFrmSize& rLSz = rFmt.GetFrmSize();
            nSz += rLSz.GetWidth();
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CELLX;

            nCalc = nSz;
            if( bRelBoxSize )
            {
                nCalc *= nPageSize;
                nCalc /= nTblSz;
            }
            rWrt.OutLong( nTblOffset + nCalc );
        }

        // Inhalt der Boxen ausgeben
        rWrt.Strm() << SwRTFWriter::sNewLine << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_INTBL;
        for( nBox = 0; nBox < nBoxes; ++nBox )
        {
            SwWriteTableCell * pCell = pBoxArr[nBox];

            if( (nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ]) || pCell == NULL)
                continue;

            if( pCell->GetRowSpan() == pRowSpans[ nBox ] )
            {
                // new Box
                const SwStartNode* pSttNd = pCell->GetBox()->GetSttNd();
                RTFSaveData aSaveData( rRTFWrt,
                        pSttNd->GetIndex()+1, pSttNd->EndOfSectionIndex() );
                rRTFWrt.bOutTable = TRUE;
                rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
            }
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CELL;
        }

        // das wars mit der Line
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ROW << OOO_STRING_SVTOOLS_RTF_PARD << ' ';

        for( nBox = 0; nBox < nColCnt; ++nBox )
            --pRowSpans[ nBox ];
    }

    delete pTableWrt;
    delete[] pBoxArr;
    delete[] pRowSpans;

    // Pam hinter die Tabelle verschieben
    rRTFWrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();
    rRTFWrt.SetAttrSet( 0 );

    return rWrt;
}

Writer& OutRTF_SwSectionNode( Writer& rWrt, SwSectionNode& rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwSection& rSect = rNode.GetSection();

    // folgt dahinter noch ein SectionNode? Dann wird erst die innere
    // Section aktiv. Hier wird die Verschachtelung aufgebrochen, weil
    // RTF das nicht kennt
    BOOL bPgDscWrite = false;
    {
        SwNodeIndex aIdx( rNode, 1 );
        const SwNode& rNd = aIdx.GetNode();
        if( rNd.IsSectionNode() /*&& CONTENT_SECTION ==
            aIdx.GetNode().GetSectionNode()->GetSection().GetType()*/ )
            return rWrt;

        // falls als erstes Position ein Content- oder Tabellen-Node steht,
        // dann kann dieser einen PageDesc gesetzt haben und der muss vor
        // der Bereichsbeschreibung geschrieben werden!
        // Umgekehrt muss im OutBreaks dann
        if( rNd.IsCntntNode() )
        {
            bPgDscWrite = rRTFWrt.OutBreaks( ((SwCntntNode&)rNd).GetSwAttrSet() );
            rRTFWrt.bIgnoreNextPgBreak = TRUE;
        }
        else if( rNd.IsTableNode() )
        {
            bPgDscWrite = rRTFWrt.OutBreaks( ((SwTableNode&)rNd).GetTable().
                                            GetFrmFmt()->GetAttrSet() );
            rRTFWrt.bIgnoreNextPgBreak = TRUE;
        }
    }


//  if( CONTENT_SECTION == rSect.GetType() )
    {
        // als fortlaufenden Abschnittwechsel heraus schreiben
        if( !bPgDscWrite )
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_SECT << OOO_STRING_SVTOOLS_RTF_SBKNONE;
        //JP 19.03.99 - es muss fuer den Import auf jedenfall das Cols
        //              Token geschrieben werden. Sonst kann nicht erkannt
        //              werden, wann ein PageDesc & eine Section gueltig ist
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLS << '1';
        rRTFWrt.bOutFmtAttr = TRUE;
        const SfxPoolItem* pItem;
        const SwFrmFmt *pFmt = rSect.GetFmt();
        ASSERT(pFmt, "Impossible");
        const SfxItemSet& rSet = pFmt->GetAttrSet();
        if( SFX_ITEM_SET == rSet.GetItemState( RES_COL, false, &pItem ))
            OutRTF_SwFmtCol( rWrt, *pItem );
        else
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLS << '1' << OOO_STRING_SVTOOLS_RTF_COLSX;
            rWrt.OutULong(709);
        }

        if( SFX_ITEM_SET == rSet.GetItemState( RES_COLUMNBALANCE,
            false, &pItem ) && ((SwFmtNoBalancedColumns*)pItem)->GetValue() )
            OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_BALANCEDCOLUMN ) << '}';

        if (FRMDIR_HORI_RIGHT_TOP == rRTFWrt.TrueFrameDirection(*pFmt))
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_RTLSECT;
        else
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LTRSECT;

        rWrt.Strm() << SwRTFWriter::sNewLine;
    }

    return rWrt;
}


/* File CHRATR.HXX: */

static Writer& OutRTF_SwFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    /* trage den Font in die Font-Liste vom Writer ein und gebe hier nur
     *  die entsprechende Nummer aus. Der Font wird spaeter ueber diese
     * Tabelle im RTF-Dokument referenziert.
     */
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        rRTFWrt.bOutFmtAttr = true;
        const SvxFontItem&rFont = (const SvxFontItem&)rHt;
        bool bAssoc = rRTFWrt.IsAssociatedFlag();
        /*
         #109522#
         Word is a bit of a silly bugger of a program when its comes to symbol
         font useage. If a symbol font is actually being used, i.e.  exported
         here with bTxtAttr true then both AF and F must be set to the same
         value
        */
        if (rRTFWrt.bTxtAttr && (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL))
        {
            const sal_Char* pCmd = !bAssoc ? OOO_STRING_SVTOOLS_RTF_AF : OOO_STRING_SVTOOLS_RTF_F;
            rWrt.Strm() << pCmd;
        }
        const sal_Char* pCmd = bAssoc ? OOO_STRING_SVTOOLS_RTF_AF : OOO_STRING_SVTOOLS_RTF_F;
        rWrt.Strm() << pCmd;
        rWrt.OutULong(rRTFWrt.GetId(rFont));
        rRTFWrt.eCurrentEncoding = rtl_getTextEncodingFromWindowsCharset(sw::ms::rtl_TextEncodingToWinCharset(rFont.GetCharSet()));
    }
    return rWrt;
}

static Writer& OutRTF_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        if (rRTFWrt.IsAssociatedFlag() && rHt.Which() == RES_CHRATR_CJK_POSTURE)
        {
            /*
            #i21422#
            Sadly in word rtf we can't retain CJK italic when we are not
            exporting asian text as it doesn't have a seperate italic for
            western and asian.
            */
            return rWrt;
        }

        const FontItalic nPosture = ((const SvxPostureItem&)rHt).GetPosture();
        int bTxtOut = rRTFWrt.bTxtAttr && ITALIC_NONE == nPosture;
        if( ITALIC_NORMAL == nPosture || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? OOO_STRING_SVTOOLS_RTF_AI : OOO_STRING_SVTOOLS_RTF_I;
            rWrt.Strm() << pCmd;
        }
        if( bTxtOut )
            rWrt.Strm() << '0';     // wieder abschalten
    }
    return rWrt;
}


static Writer& OutRTF_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        if (rRTFWrt.IsAssociatedFlag() && rHt.Which() == RES_CHRATR_CJK_WEIGHT)
        {
            /*
            #i21422#
            Sadly in word rtf we can't retain CJK bold when we are not
            exporting asian text as it doesn't have a seperate bold for western
            and asian.
            */
            return rWrt;
        }

        const FontWeight nBold = ((const SvxWeightItem&)rHt).GetWeight();
        int bTxtOut = rRTFWrt.bTxtAttr && WEIGHT_NORMAL == nBold;
        if( WEIGHT_BOLD == nBold || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? OOO_STRING_SVTOOLS_RTF_AB : OOO_STRING_SVTOOLS_RTF_B;
            rWrt.Strm() << pCmd;
        }
        if( bTxtOut )
            rWrt.Strm() <<  '0';
    }
    return rWrt;
}

static Writer& OutRTF_SwEmphasisMark( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const sal_Char* pStr;
    switch( ((const SvxEmphasisMarkItem&)rHt).GetEmphasisMark())
    {
    case EMPHASISMARK_NONE:         pStr = OOO_STRING_SVTOOLS_RTF_ACCNONE;  break;
    case EMPHASISMARK_SIDE_DOTS:    pStr = OOO_STRING_SVTOOLS_RTF_ACCCOMMA; break;
    default:                        pStr = OOO_STRING_SVTOOLS_RTF_ACCDOT;       break;
    }

    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << pStr;
    return rWrt;
}

static Writer& OutRTF_SwTwoInOne( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ((SvxTwoLinesItem&)rHt).GetValue() )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        sal_Unicode cStart = ((SvxTwoLinesItem&)rHt).GetStartBracket();
        sal_Unicode cEnd = ((SvxTwoLinesItem&)rHt).GetStartBracket();

        USHORT nType;
        if( !cStart && !cEnd )
            nType = 0;
        else if( '{' == cStart || '}' == cEnd )
            nType = 4;
        else if( '<' == cStart || '>' == cEnd )
            nType = 3;
        else if( '[' == cStart || ']' == cEnd )
            nType = 2;
        else                            // all other kind of brackets
            nType = 1;

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TWOINONE;
        rWrt.OutULong( nType );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwCharRotate( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_HORZVERT;
    rWrt.OutLong( ((SvxCharRotateItem&)rHt).IsFitToLine() ? 1 : 0 );
    return rWrt;
}
static Writer& OutRTF_SwCharScaleW( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CHARSCALEX;
    rWrt.OutLong( ((SvxCharScaleWidthItem&)rHt).GetValue() );
    return rWrt;
}

static Writer& OutRTF_SwCharRelief(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxCharReliefItem& rAttr = (const SvxCharReliefItem&)rHt;
    const sal_Char* pStr;
    switch (rAttr.GetValue())
    {
        case RELIEF_EMBOSSED:
            pStr = OOO_STRING_SVTOOLS_RTF_EMBO;
            break;
        case RELIEF_ENGRAVED:
            pStr = OOO_STRING_SVTOOLS_RTF_IMPR;
            break;
        default:
            pStr = 0;
            break;
    }

    if (pStr)
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << pStr;
    }
    return rWrt;
}


static Writer& OutRTF_SwChrBckgrnd( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
    if( !rBack.GetColor().GetTransparency() )
    {
        ByteString sOut( OOO_STRING_SVTOOLS_RTF_CHCBPAT );
        sOut += ByteString::CreateFromInt32(
                            rRTFWrt.GetId( rBack.GetColor() ));

        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sOut.GetBuffer();
    }
    return rWrt;
}

static Writer& OutRTF_SwShadowed( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const BOOL bShadow = ((const SvxShadowedItem&)rHt).GetValue();
    int bTxtOut = rRTFWrt.bTxtAttr && !bShadow;
    if( bShadow || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_SHAD;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



static Writer& OutRTF_SwContour( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const BOOL bContour = ((const SvxContourItem&)rHt).GetValue();
    int bTxtOut = rRTFWrt.bTxtAttr && !bContour;
    if( bContour || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_OUTL;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}

static Writer& OutRTF_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const FontStrikeout nStrike = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    int bTxtOut = rRTFWrt.bTxtAttr && STRIKEOUT_NONE == nStrike;

    if( (STRIKEOUT_NONE != nStrike && STRIKEOUT_DONTKNOW != nStrike) || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        if( STRIKEOUT_DOUBLE == nStrike )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_STRIKED;
            if( !bTxtOut )
                rWrt.Strm() << '1';
        }
        else
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_STRIKE;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



static Writer& OutRTF_SwCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    switch( ((const SvxCaseMapItem&)rHt).GetValue() )
    {
    case SVX_CASEMAP_KAPITAELCHEN:
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_SCAPS;
        break;

    case SVX_CASEMAP_VERSALIEN:
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CAPS;
        break;

    case SVX_CASEMAP_NOT_MAPPED:
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CAPS << '0' << OOO_STRING_SVTOOLS_RTF_SCAPS << '0';       // beide aus !!
        break;

    default:
        return rWrt;
    }

    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}


static Writer& OutRTF_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    const char* pStr = 0;
    switch( ((const SvxUnderlineItem&)rHt).GetLineStyle() )
    {
        case UNDERLINE_SINGLE:
            pStr = OOO_STRING_SVTOOLS_RTF_UL;
            break;
        case UNDERLINE_DOUBLE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDB;
            break;
        case UNDERLINE_NONE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULNONE;
            break;
        case UNDERLINE_DOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULD;
            break;
        case UNDERLINE_DASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASH;
            break;
        case UNDERLINE_DASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHD;
            break;
        case UNDERLINE_DASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHDD;
            break;
        case UNDERLINE_BOLD:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTH;
            break;
        case UNDERLINE_WAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULWAVE;
            break;
        case UNDERLINE_BOLDDOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHD;
            break;
        case UNDERLINE_BOLDDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASH;
            break;
        case UNDERLINE_LONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULLDASH;
            break;
        case UNDERLINE_BOLDLONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHLDASH;
            break;
        case UNDERLINE_BOLDDASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHD;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHDD;
            break;
        case UNDERLINE_BOLDWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULHWAVE;
            break;
        case UNDERLINE_DOUBLEWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULULDBWAVE;
            break;
        default:
            break;
    }

    if( pStr )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        if( UNDERLINE_SINGLE == ((const SvxUnderlineItem&)rHt).GetLineStyle()
            && ((SvxWordLineModeItem&)rRTFWrt.GetItem(
                RES_CHRATR_WORDLINEMODE )).GetValue() )
            pStr = OOO_STRING_SVTOOLS_RTF_ULW;

        rRTFWrt.Strm() << pStr;
        rRTFWrt.bOutFmtAttr = TRUE;

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ULC;
        rWrt.OutULong( rRTFWrt.GetId(((const SvxUnderlineItem&)rHt).GetColor()) );

    }

    return rWrt;
}



static Writer& OutRTF_SwOverline( Writer& rWrt, const SfxPoolItem& rHt )
{
    const char* pStr = 0;
    switch( ((const SvxOverlineItem&)rHt).GetLineStyle() )
    {
        case UNDERLINE_SINGLE:
            pStr = OOO_STRING_SVTOOLS_RTF_OL;
            break;
        case UNDERLINE_DOUBLE:
            pStr = OOO_STRING_SVTOOLS_RTF_OLDB;
            break;
        case UNDERLINE_NONE:
            pStr = OOO_STRING_SVTOOLS_RTF_OLNONE;
            break;
        case UNDERLINE_DOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_OLD;
            break;
        case UNDERLINE_DASH:
            pStr = OOO_STRING_SVTOOLS_RTF_OLDASH;
            break;
        case UNDERLINE_DASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_OLDASHD;
            break;
        case UNDERLINE_DASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_OLDASHDD;
            break;
        case UNDERLINE_BOLD:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTH;
            break;
        case UNDERLINE_WAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_OLWAVE;
            break;
        case UNDERLINE_BOLDDOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTHD;
            break;
        case UNDERLINE_BOLDDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTHDASH;
            break;
        case UNDERLINE_LONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_OLLDASH;
            break;
        case UNDERLINE_BOLDLONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTHLDASH;
            break;
        case UNDERLINE_BOLDDASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTHDASHD;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_OLTHDASHDD;
            break;
        case UNDERLINE_BOLDWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_OLHWAVE;
            break;
        case UNDERLINE_DOUBLEWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_OLOLDBWAVE;
            break;
        default:
            break;
    }

    if( pStr )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
        if ( rRTFWrt.bNonStandard )
        {
            if( UNDERLINE_SINGLE == ((const SvxOverlineItem&)rHt).GetLineStyle()
                && ((SvxWordLineModeItem&)rRTFWrt.GetItem(
                    RES_CHRATR_WORDLINEMODE )).GetValue() )
                pStr = OOO_STRING_SVTOOLS_RTF_OLW;

            rRTFWrt.Strm() << pStr;
            rRTFWrt.bOutFmtAttr = TRUE;

            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_OLC;
            rWrt.OutULong( rRTFWrt.GetId(((const SvxOverlineItem&)rHt).GetColor()) );
        }

    }

    return rWrt;
}



static Writer& OutRTF_SwLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {

        rRTFWrt.bOutFmtAttr = TRUE;
        const sal_Char* p = RES_CHRATR_CJK_LANGUAGE == rHt.Which()
                                    ? OOO_STRING_SVTOOLS_RTF_LANGFE : OOO_STRING_SVTOOLS_RTF_LANG;
        rWrt.Strm() << p;
        rWrt.OutULong( ((const SvxLanguageItem&)rHt).GetLanguage() );
    }
    return rWrt;
}



static Writer& OutRTF_SwEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxEscapementItem& rEsc = (const SvxEscapementItem&)rHt;
    const char * pUpDn;

    SwTwips nH = ((SvxFontHeightItem&)rRTFWrt.GetItem(
                                RES_CHRATR_FONTSIZE )).GetHeight();

    if( 0 < rEsc.GetEsc() )
        pUpDn = OOO_STRING_SVTOOLS_RTF_UP;
    else if( 0 > rEsc.GetEsc() )
    {
        pUpDn = OOO_STRING_SVTOOLS_RTF_DN;
        nH = -nH;               // den negativen Wert herrausrechnen
    }
    else
        return rWrt;

    // prozentuale Veraenderung speichern !
    short nEsc = rEsc.GetEsc();
    short nProp = rEsc.GetProp() * 100;
    if( DFLT_ESC_AUTO_SUPER == nEsc )
    {
        nEsc = 100 - rEsc.GetProp();
        ++nProp;
    }
    else if( DFLT_ESC_AUTO_SUB == nEsc )
    {
        nEsc = - 100 + rEsc.GetProp();
        ++nProp;
    }

    OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_UPDNPROP, TRUE );
    rWrt.OutULong( nProp ) << '}' << pUpDn;

    /*
     * berechne aus der akt. FontSize und dem ProzentWert die Verschiebung,
     * wobei im RTF File 1/2 Points stehen muessen, waehrend intern
     * mit Twips gerechnet wird.
     * Formel :         (FontSize * 1/20 ) pts         x * 2
     *                  -----------------------  = ------------
     *                    100%                      Escapement
     */

    rWrt.OutULong( ( (long( nEsc ) * nH) + 500L ) / 1000L );
                                                // 500L zum Aufrunden !!
    return rWrt;
}



static Writer& OutRTF_SwSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        if (
             rRTFWrt.IsAssociatedFlag() &&
             rHt.Which() == RES_CHRATR_CJK_FONTSIZE
           )
        {
            /*
            #i21422#
            Sadly in word rtf we can't retain CJK fontsize when we are not
            exporting asian text as it doesn't have a seperate fontsize for
            western and asian.
            */
            return rWrt;
        }

        rRTFWrt.bOutFmtAttr = TRUE;

        const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? OOO_STRING_SVTOOLS_RTF_AFS : OOO_STRING_SVTOOLS_RTF_FS;
        rWrt.Strm() << pCmd;
        rWrt.OutULong( ((const SvxFontHeightItem&)rHt).GetHeight() / 10 );
    }
    return rWrt;
}



static Writer& OutRTF_SwColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    /* trage die Color in die Color-Liste vom Writer ein und gebe hier nur
     * die entsprechende Nummer aus. Die Color wird spaeter ueber diese
     * Tabelle im RTF-Dokument referenziert.
     */
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CF;
    rWrt.OutULong( rRTFWrt.GetId( ((const SvxColorItem&)rHt).GetValue() ));
    return rWrt;
}

static Writer& OutRTF_SvxCharHiddenItem(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    rRTFWrt.bOutFmtAttr = true;
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_V;
    if (!((const SvxCharHiddenItem&)rHt).GetValue())
        rWrt.OutULong(0);
    return rWrt;
}

extern void sw3io_ConvertToOldField( const SwField* pFld, USHORT& rWhich,
                              ULONG& rFmt, ULONG nFFVersion );

static Writer& OutRTF_SwField( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFmtFld & rFld = (SwFmtFld&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    const SwField* pFld = rFld.GetFld();

    ByteString aFldStt( '{' );
    ((((aFldStt += OOO_STRING_SVTOOLS_RTF_FIELD) += '{' ) += OOO_STRING_SVTOOLS_RTF_IGNORE) += OOO_STRING_SVTOOLS_RTF_FLDINST) += ' ';
    switch( pFld->GetTyp()->Which() )
    {
    case RES_COMBINED_CHARS:
        {
            /*
            We need a font size to fill in the defaults, if these are overridden
            (as they generally are) by character properties then those properties
            win.

            The fontsize that is used in MS for determing the defaults is always
            the CJK fontsize even if the text is not in that language, in OOo the
            largest fontsize used in the field is the one we should take, but
            whatever we do, word will actually render using the fontsize set for
            CJK text. Nevertheless we attempt to guess whether the script is in
            asian or western text based up on the first character and use the
            font size of that script as our default.
            */
            const String& rFldPar1 = pFld->GetPar1();
            USHORT nScript;
            if( pBreakIt->GetBreakIter().is() )
                nScript = pBreakIt->GetBreakIter()->getScriptType( rFldPar1, 0);
            else
                nScript = i18n::ScriptType::ASIAN;

            long nHeight = ((SvxFontHeightItem&)rRTFWrt.GetItem(
                GetWhichOfScript(RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
            nHeight = (nHeight + 10) / 20; //Font Size in points;

            /*
            Divide the combined char string into its up and down part. Get the
            font size and fill in the defaults as up == half the font size and
            down == a fifth the font size
            */
            xub_StrLen nAbove = (rFldPar1.Len()+1)/2;
            rWrt.Strm() << aFldStt.GetBuffer() << "EQ \\\\o (\\\\s\\\\up ";
            rWrt.OutLong( nHeight/2 ) << '(';
            RTFOutFuncs::Out_String( rWrt.Strm(), rFldPar1.Copy(0,nAbove),
                                    rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
            rWrt.Strm() << "), \\\\s\\\\do ";
            rWrt.OutLong( nHeight/5 ) << '(';
            RTFOutFuncs::Out_String( rWrt.Strm(), rFldPar1.Copy( nAbove ),
                                    rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt )
                    << "))";
        }
        break;

    case RES_DBFLD:
            aFldStt += "MERGEFIELD ";
            // kein break !!
    case RES_USERFLD:
            rWrt.Strm() << aFldStt.GetBuffer();
            RTFOutFuncs::Out_String( rWrt.Strm(), pFld->GetTyp()->GetName(),
                                    rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
            break;
    case RES_GETREFFLD:
        {
            BYTE nFldTyp = 0;
            rWrt.Strm() << aFldStt.GetBuffer() << " REF ";
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld;
            switch( pFld->GetSubType() )
            {
                case REF_SETREFATTR:
                case REF_BOOKMARK:
                    RTFOutFuncs::Out_String( rWrt.Strm(), rRFld.GetSetRefName(),
                                    rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
                    nFldTyp = 3;
                    break;
            }

            if( nFldTyp )
            {
                switch( pFld->GetFormat() )
                {
                    case REF_PAGE_PGDESC:
                    case REF_PAGE:
                        rWrt.Strm() << "SEITEN";
                        nFldTyp = 37;
                        break;
                    case REF_UPDOWN:
                        rWrt.Strm() << " \\p";
                        nFldTyp = 3;
                        break;
                    case REF_CHAPTER:
                        rWrt.Strm() << " \\n";
                        break;
                    case REF_ONLYNUMBER:
                    case REF_ONLYCAPTION:
                    case REF_ONLYSEQNO:
                        break;
                    // default:
                    // case REF_CONTENT:
                }
                rWrt.Strm() << " \\\\h ";       // insert hyperlink
            }
        }
        break;
//  case RES_CHAPTERFLD:
//          rWrt.Strm() << ' ';
//          break;
    case RES_PAGENUMBERFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\page";
            {
                char __READONLY_DATA
                    sNType0[] = "ALPHABETIC",       /* CHARS_UPPER_LETTER*/
                    sNType1[] = "alphabetic",       /* CHARS_LOWER_LETTER*/
                    sNType2[] = "ROMAN",            /* ROMAN_UPPER       */
                    sNType3[] = "roman",            /* ROMAN_LOWER       */
                    sNType4[] = "ARABIC";           /* ARABIC            */

                const char* pFmtStr = 0;
                switch( pFld->GetFormat() )
                {
                case SVX_NUM_CHARS_UPPER_LETTER:
                case SVX_NUM_CHARS_UPPER_LETTER_N:  pFmtStr = sNType0; break;
                case SVX_NUM_CHARS_LOWER_LETTER:
                case SVX_NUM_CHARS_LOWER_LETTER_N:  pFmtStr = sNType1; break;
                case SVX_NUM_ROMAN_UPPER:           pFmtStr = sNType2; break;
                case SVX_NUM_ROMAN_LOWER:           pFmtStr = sNType3; break;
                case SVX_NUM_ARABIC:                pFmtStr = sNType4; break;
                }

                if( pFmtStr )
                    rWrt.Strm() << "\\\\* " << pFmtStr;
            }
            break;
    case RES_FILENAMEFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\filename ";
            break;
    case RES_DBNAMEFLD:
    {
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\data ";
            SwDBData aData = rWrt.pDoc->GetDBData();
            String sOut(aData.sDataSource);
            sOut += DB_DELIM;
            sOut += (String)aData.sCommand;
            RTFOutFuncs::Out_String( rWrt.Strm(), sOut,
                                    rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
    }
    break;
    case RES_AUTHORFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\author ";
            break;

    case RES_HIDDENTXTFLD:
            if( TYP_CONDTXTFLD == ((SwHiddenTxtField*)pFld)->GetSubType() )
                RTFOutFuncs::Out_String( rWrt.Strm(),
                    pFld->ExpandField(rWrt.pDoc->IsClipBoard()),
                                        rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
            else
            {
                rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_V << ' ';
                OutRTF_AsByteString( rWrt, pFld->GetPar2(), rRTFWrt.eDefaultEncoding ).Strm()
                            << '}' << SwRTFWriter::sNewLine;
            }
            return rWrt;        // nicht bis zum Ende, kein RTF-Feld !!

    case RES_DATETIMEFLD:
            if (!(pFld->GetSubType() & FIXEDFLD))
            {
                USHORT nWhich = RES_DATETIMEFLD;
                ULONG nFmt = pFld->GetFormat();
                sw3io_ConvertToOldField(pFld, nWhich, nFmt, SOFFICE_FILEFORMAT_40 );
                if (pFld->GetSubType() & DATEFLD)
                {
                    rWrt.Strm() << aFldStt.GetBuffer() << "\\\\date \\\\@\"";
                    switch( nFmt )
                    {
                    case DF_SSYS:       rWrt.Strm() << "tt.MM.yy";              break;
                    case DF_LSYS:       rWrt.Strm() << "tttt, t. MMMM yyyy";    break;
                    case DF_SHORT:      rWrt.Strm() << "tt.MM.yy";              break;
                    case DF_SCENT:      rWrt.Strm() << "tt.MM.yyyy";            break;
                    case DF_LMON:       rWrt.Strm() << "t. MMM yyyy";           break;
                    case DF_LMONTH:     rWrt.Strm() << "t. MMMM yyyy";          break;
                    case DF_LDAYMON:    rWrt.Strm() << "ttt, t. MMMM yyyy";     break;
                    case DF_LDAYMONTH:  rWrt.Strm() << "tttt, t. MMMM yyyy";    break;
                    }
                }
                else
                {
                    rWrt.Strm() << aFldStt.GetBuffer() << "\\\\date \\\\@\"";
                    switch( nFmt )
                    {
                    case TF_SSMM_24:    rWrt.Strm() << "HH:mm"; break;
                    case TF_SSMM_12:    rWrt.Strm() << "hh:mm"; break;  // ???
                    case TF_SYSTEM:     rWrt.Strm() << "HH:mm"; break;
                    }
                }
                rWrt.Strm() << '\"';
                break;
            }
            // Kein break: in default-Zweig laufen!

    default:
        {
            aFldStt.Erase();

            // JP 20.07.95: warum nicht das Expandierte rausschreiben ?
            sal_Char cCh;
            rWrt.Strm().SeekRel(-1);
            rWrt.Strm() >> cCh;
            if( ' ' != cCh )            // vorweg immer einen Trenner
                rWrt.Strm() << ' ';
            RTFOutFuncs::Out_String( rWrt.Strm(),
                pFld->ExpandField(rWrt.pDoc->IsClipBoard()),
                                        rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
        }
        break;
    }

    if( aFldStt.Len() )
    {
        rWrt.Strm() << "}{" << OOO_STRING_SVTOOLS_RTF_FLDRSLT << ' ';
        RTFOutFuncs::Out_String( rWrt.Strm(),
                pFld->ExpandField(rWrt.pDoc->IsClipBoard()),
                                        rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
        rWrt.Strm() << "}}";
        rRTFWrt.bOutFmtAttr = FALSE;
    }

    return rWrt;
}

static Writer& OutRTF_SwFlyCntnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFrmFmt* pFmt = ((SwFmtFlyCnt&)rHt).GetFrmFmt();
    if (RES_DRAWFRMFMT != pFmt->Which())
    {
        ((SwRTFWriter&)rWrt).OutRTFFlyFrms( *((SwFlyFrmFmt*)pFmt) );
        ((SwRTFWriter&)rWrt).bOutFmtAttr = false;
    }
    return rWrt;
}

static Writer& OutRTF_SwFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFtn& rFtn = (const SwFmtFtn&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    do {    // middle-check-loop
        if( !rFtn.GetTxtFtn() || !rFtn.GetTxtFtn()->GetStartNode() )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG   nStart = rFtn.GetTxtFtn()->GetStartNode()->GetIndex()+1,
                nEnd = rWrt.pDoc->GetNodes()[ nStart-1 ]->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;

        rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_SUPER << ' ';
        if( !rFtn.GetNumStr().Len() )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_CHFTN;
            OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FTNALT;
            rWrt.Strm() << ' ' << OOO_STRING_SVTOOLS_RTF_CHFTN;
        }
        else
        {
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr(), rRTFWrt.eDefaultEncoding );
            OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FTNALT;
            rWrt.Strm() << ' ';
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr(), rRTFWrt.eDefaultEncoding );
        }
        RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
        // damit kein \par ausgegeben wird !!
        rRTFWrt.pCurPam->GetMark()->nContent++;
        rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );

        rWrt.Strm() << "}}" << SwRTFWriter::sNewLine;
        rRTFWrt.bOutFmtAttr = FALSE;

    } while( FALSE );
    return rWrt;
}

static Writer& OutRTF_SwTxtCharFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rHt;
    const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

    if( pFmt )
        OutRTF_SwFmt( rWrt, *pFmt );
    return rWrt;
}

static Writer& OutRTF_SwTxtAutoFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtAutoFmt& rAutoFmt = (const SwFmtAutoFmt&)rHt;
    const boost::shared_ptr<SfxItemSet> pSet = rAutoFmt.GetStyleHandle();

    if( pSet.get() )
    {
        SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
        OutRTF_SfxItemSet( rRTFWrt, *pSet.get(), FALSE );
    }
    return rWrt;
}

static Writer& OutRTF_SwTxtRuby( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SwFmtRuby& rRuby = (const SwFmtRuby&)rHt;
    const SwTxtRuby* pRubyTxt = rRuby.GetTxtRuby();
    const SwTxtNode* pNd;

    if( !pRubyTxt || 0 == (pNd = pRubyTxt->GetpTxtNode() ))
        return rWrt;

    sal_Char cDirective = 0, cJC = '0';
    switch( rRuby.GetAdjustment() )
    {
    case 0:     cJC = '3';  cDirective = 'l';   break;
    case 2:     cJC = '4';  cDirective = 'r';   break;
    case 3:     cJC = '1';  cDirective = 'd';   break;
    case 4:     cJC = '2';  cDirective = 'd';   break;
    case 1:     break;          //defaults to 0
    default:
        ASSERT( FALSE, "Unhandled Ruby justication code" );
        break;
    }

    /*
        MS needs to know the name and size of the font used in the ruby item,
        but we coud have written it in a mixture of asian and western
        scripts, and each of these can be a different font and size than the
        other, so we make a guess based upon the first character of the text,
        defaulting to asian.
        */
    USHORT nScript;
    if( pBreakIt->GetBreakIter().is() )
        nScript = pBreakIt->GetBreakIter()->getScriptType( rRuby.GetText(), 0);
    else
        nScript = i18n::ScriptType::ASIAN;

    const SwCharFmt* pFmt = pRubyTxt->GetCharFmt();
    const SvxFontItem *pFont;
    long nHeight;

    if( pFmt )
    {
        const SwAttrSet& rSet = pFmt->GetAttrSet();
        pFont = &(const SvxFontItem&)rSet.Get( GetWhichOfScript(
                                                RES_CHRATR_FONT, nScript ));

        nHeight = ((SvxFontHeightItem&)rSet.Get(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    }
    else
    {
        /*Get document defaults if no formatting on ruby text*/
        const SfxItemPool *pPool = pNd->GetSwAttrSet().GetPool();
        pFont = &(const SvxFontItem&)pPool->GetDefaultItem(
                            GetWhichOfScript( RES_CHRATR_FONT, nScript ));

        nHeight = ((SvxFontHeightItem&)pPool->GetDefaultItem(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    }
    ( nHeight += 5 ) /= 10;

    // don't change " EQ " to any other without changing the code in RTFFLD.CXX
    rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_FIELD << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_FLDINST
                << " EQ \\\\* jc" << cJC
                << " \\\\* \"Font:";
    RTFOutFuncs::Out_String( rWrt.Strm(), pFont->GetFamilyName(),
                            rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
    rWrt.Strm() << "\" \\\\* hps";
    rWrt.OutLong( nHeight );
    rWrt.Strm() << " \\\\o";
    if( cDirective )
        rWrt.Strm() << "\\\\a" << cDirective;
    rWrt.Strm() << "(\\\\s\\\\up ";

    if( pBreakIt->GetBreakIter().is() )
        nScript = pBreakIt->GetBreakIter()->getScriptType( pNd->GetTxt(),
                                                   *pRubyTxt->GetStart() );
    else
        nScript = i18n::ScriptType::ASIAN;

    const SwAttrSet& rSet = pNd->GetSwAttrSet();
    nHeight = ((SvxFontHeightItem&)rSet.Get(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    (nHeight += 10) /= 20-1;
    rWrt.OutLong( nHeight ) << '(';

    if( pFmt )
    {
        rWrt.Strm() << '{';
        OutRTF_SwFmt( rWrt, *pFmt );
        if( rRTFWrt.bOutFmtAttr )
            rWrt.Strm() << ' ';
    }
    RTFOutFuncs::Out_String( rWrt.Strm(), rRuby.GetText(),
                            rRTFWrt.eDefaultEncoding, rRTFWrt.bWriteHelpFmt );
    if( pFmt )
        rWrt.Strm() << '}';

    rWrt.Strm() << "),";
    rRTFWrt.bOutFmtAttr = FALSE;

    return rWrt;
}


/* File FRMATR.HXX */

static Writer& OutRTF_SwFrmSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SwFmtFrmSize& rSz = (const SwFmtFrmSize&)rHt;
    if( rRTFWrt.pFlyFmt )       // wird das FlyFrmFmt ausgegeben ?
    {
        if( !rRTFWrt.bRTFFlySyntax )
            return rWrt;

        if( rSz.GetWidth() )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ABSW;
            rWrt.OutLong( rSz.GetWidth() );
            rRTFWrt.bOutFmtAttr = TRUE;
        }

        if( rSz.GetHeight() )
        {
            long nH = rSz.GetHeight();
            if( ATT_FIX_SIZE == rSz.GetHeightSizeType() )
                nH = -nH;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ABSH;
            rWrt.OutLong( nH );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
    else if( rRTFWrt.bOutPageDesc )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PGWSXN;
        rWrt.OutLong( rSz.GetWidth() );
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_PGHSXN;
        rWrt.OutLong( rSz.GetHeight() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtLRSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxLRSpaceItem & rLR = (const SvxLRSpaceItem&) rHt;
    if( !rRTFWrt.pFlyFmt )
    {
        if( rRTFWrt.bOutPageDesc )
        {
            if( rLR.GetLeft() )
            {
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_MARGLSXN;
                rWrt.OutLong( rLR.GetLeft() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
            if( rLR.GetRight() )
            {
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_MARGRSXN;
                rWrt.OutLong( rLR.GetRight() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
        }
        else
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LI;
            rWrt.OutLong( rLR.GetTxtLeft() ) << OOO_STRING_SVTOOLS_RTF_RI;
            rWrt.OutLong( rLR.GetRight() );
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LIN;
            rWrt.OutLong( rLR.GetTxtLeft() ) << OOO_STRING_SVTOOLS_RTF_RIN;
            rWrt.OutLong( rLR.GetRight() );
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FI;
            rWrt.OutLong( rLR.GetTxtFirstLineOfst() );
        }
    }
    else if( rLR.GetLeft() == rLR.GetRight() && rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_DFRMTXTX;
        rWrt.OutLong( rLR.GetLeft() );
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtULSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxULSpaceItem & rUL = (const SvxULSpaceItem&) rHt;
    if( rRTFWrt.pFlyFmt )
    {
        if( rUL.GetUpper() == rUL.GetLower() && rRTFWrt.bRTFFlySyntax )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_DFRMTXTY;
            rWrt.OutLong( rUL.GetLower() );
        }
    }
    else
    {
        const char* p;
        USHORT nValue = rUL.GetUpper();
        if( rRTFWrt.bOutPageDesc )
        {
            p = OOO_STRING_SVTOOLS_RTF_MARGTSXN;
            if( !rRTFWrt.bOutPageDescTbl )
            {
                SwRect aRect;
                const SwFmtHeader* pHdr;
                if( SFX_ITEM_SET == rRTFWrt.pAktPageDesc->GetMaster().
                        GetItemState( RES_HEADER, FALSE,
                        (const SfxPoolItem**)&pHdr ) && pHdr->IsActive() )
                {
                    aRect = pHdr->GetHeaderFmt()->FindLayoutRect( FALSE );
                    if( aRect.Height() )
                        nValue = nValue + static_cast< USHORT >(aRect.Height());
                    else
                    {
                        const SwFmtFrmSize& rSz = pHdr->GetHeaderFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetHeightSizeType() )
                            nValue = nValue + static_cast< USHORT >(rSz.GetHeight());
                        else
                            nValue = nValue + 274;       // defaulten fuer 12pt Schrift
                        nValue = nValue + pHdr->GetHeaderFmt()->GetULSpace().GetLower();
                    }
                }
            }
        }
        else
            p = OOO_STRING_SVTOOLS_RTF_SB;

        if( rRTFWrt.bOutPageDesc || nValue )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << p;
            rWrt.OutLong( nValue );
        }


        nValue = rUL.GetLower();
        if( rRTFWrt.bOutPageDesc )
        {
            p = OOO_STRING_SVTOOLS_RTF_MARGBSXN;
            if( !rRTFWrt.bOutPageDescTbl )
            {
                SwRect aRect;
                const SwFmtFooter* pFtr;
                if( SFX_ITEM_SET == rRTFWrt.pAktPageDesc->GetMaster().
                        GetItemState( RES_FOOTER, FALSE,
                        (const SfxPoolItem**)&pFtr ) && pFtr->IsActive() )
                {
                    aRect = pFtr->GetFooterFmt()->FindLayoutRect( FALSE );
                    if( aRect.Height() )
                        nValue = nValue + static_cast< USHORT >(aRect.Height());
                    else
                    {
                        const SwFmtFrmSize& rSz = pFtr->GetFooterFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetHeightSizeType() )
                            nValue = nValue + static_cast< USHORT >(rSz.GetHeight());
                        else
                            nValue += 274;      // defaulten fuer 12pt Schrift
                        nValue = nValue + pFtr->GetFooterFmt()->GetULSpace().GetUpper();
                    }
                }
            }
        }
        else
            p = OOO_STRING_SVTOOLS_RTF_SA;

        if( rRTFWrt.bOutPageDesc || nValue )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << p;
            rWrt.OutLong( nValue );
        }
    }
    return rWrt;
}

// Header-Footer werden auch vom RTF-Writer direkt gerufen, also kein static!


Writer& OutRTF_SwFmtHeader( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtHeader& rHd = (const SwFmtHeader&)rHt;
    if( !rHd.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return rWrt;

    // hole einen Node zu dem Request
    SwStartNode *pSttNode = 0;
    const SwFmtCntnt& rCntnt = rHd.GetHeaderFmt()->GetCntnt();
    if( rCntnt.GetCntntIdx() )
        pSttNode = rCntnt.GetCntntIdx()->GetNode().GetStartNode();

    do {    // middle-check-loop
        if( !pSttNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pSttNode->GetIndex() + 1,
                nEnd = pSttNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( rRTFWrt.pAktPageDesc, "Header-Attribut ohne PageDesc" );

        const sal_Char * pHdNm = OOO_STRING_SVTOOLS_RTF_HEADER;
        rWrt.Strm() << pHdNm << 'y';
        if( rRTFWrt.bOutPageDescTbl )
        {
            // hole die Ober-/Unterkanten vom Header
            const SvxULSpaceItem& rUL = rHd.GetHeaderFmt()->GetULSpace();
            const SvxLRSpaceItem& rLR = rHd.GetHeaderFmt()->GetLRSpace();
            const SwFmtFrmSize& rSz = rHd.GetHeaderFmt()->GetFrmSize();

            rWrt.OutLong( rUL.GetUpper() );
            OutComment( rWrt, pHdNm ) << "yb";
            rWrt.OutLong( rUL.GetLower() ) << pHdNm << "xl";
            rWrt.OutLong( rLR.GetLeft() ) << pHdNm << "xr";
            rWrt.OutLong( rLR.GetRight() )  << pHdNm << "yh";
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetHeightSizeType()
                            ? -rSz.GetHeight()
                            : rSz.GetHeight() ) << '}';
        }
        else
            rWrt.OutLong( rRTFWrt.pAktPageDesc->GetMaster().
                            GetULSpace().GetUpper() );

        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.
        sal_Char cTyp = 0;
        if( rRTFWrt.pAktPageDesc->GetFollow() &&
            rRTFWrt.pAktPageDesc->GetFollow() != rRTFWrt.pAktPageDesc )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TITLEPG;        //i13107
            cTyp = 'f';     // dann FirstPage-Header
        }
        else if( !rRTFWrt.pAktPageDesc->IsHeaderShared() )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FACINGP;        //i13107
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';
        }

        rWrt.Strm() << '{'<< pHdNm;
        if( cTyp ) rWrt.Strm() << cTyp;
        rWrt.Strm() << ' ';

        {
            RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
            rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
        }

        rWrt.Strm() << '}' << SwRTFWriter::sNewLine;

    } while( FALSE );
    return rWrt;
}
// Header-Footer werden auch vom RTF-Writer direkt gerufen, also kein static!


Writer& OutRTF_SwFmtFooter( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFooter& rFt = (const SwFmtFooter&)rHt;
    if( !rFt.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return rWrt;

    SwStartNode *pSttNode = 0;
    const SwFmtCntnt& rCntnt = rFt.GetFooterFmt()->GetCntnt();
    if( rCntnt.GetCntntIdx() )
        pSttNode = rCntnt.GetCntntIdx()->GetNode().GetStartNode();

    do {    // middle-check-loop
        if( !pSttNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

            // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pSttNode->GetIndex()+1,
              nEnd = pSttNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( rRTFWrt.pAktPageDesc, "Footer-Attribut ohne PageDesc" );

        const sal_Char * pFtNm = OOO_STRING_SVTOOLS_RTF_FOOTER;
        rWrt.Strm() << pFtNm << 'y';
        if( rRTFWrt.bOutPageDescTbl )
        {
            // hole die Ober-/Unterkanten vom Footer
            const SvxULSpaceItem& rUL = rFt.GetFooterFmt()->GetULSpace();
            const SvxLRSpaceItem& rLR = rFt.GetFooterFmt()->GetLRSpace();
            const SwFmtFrmSize& rSz = rFt.GetFooterFmt()->GetFrmSize();

            rWrt.OutLong( rUL.GetLower() );
            OutComment( rWrt, pFtNm ) << "yt";
            rWrt.OutLong( rUL.GetUpper() ) << pFtNm << "xl";
            rWrt.OutLong( rLR.GetLeft() ) << pFtNm << "xr";
            rWrt.OutLong( rLR.GetRight() )  << pFtNm << "yh";
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetHeightSizeType()
                            ? -rSz.GetHeight()
                            : rSz.GetHeight() )  << '}';
        }
        else
            rWrt.OutLong( rRTFWrt.pAktPageDesc->GetMaster().
                            GetULSpace().GetLower() );

        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.
        sal_Char cTyp = 0;
        if( !rRTFWrt.bOutPageDesc && rRTFWrt.pAktPageDesc->GetFollow() &&
            rRTFWrt.pAktPageDesc->GetFollow() != rRTFWrt.pAktPageDesc )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TITLEPG;        //i13107
            cTyp = 'f';     // dann FirstPage-Header
        }
        else if( !rRTFWrt.pAktPageDesc->IsFooterShared() )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FACINGP;        //i13107
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';
        }

        rWrt.Strm() << '{'<< pFtNm;
        if( cTyp ) rWrt.Strm() << cTyp;
        rWrt.Strm() << ' ';

        {
            RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
            rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
        }

        rWrt.Strm() << '}' << SwRTFWriter::sNewLine;

    } while( FALSE );
    return rWrt;
}

static Writer& OutRTF_SwFmtPrint( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax && !((const SvxPrintItem&)rHt).GetValue() )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYPRINT;
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtOpaque( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax && !((const SvxOpaqueItem&)rHt).GetValue() )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYOPAQUE;
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtProtect( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax )
    {
        const SvxProtectItem & rFlyProtect = (const SvxProtectItem&) rHt;
        RTFProtect aP( rFlyProtect.IsCntntProtected(),
                        rFlyProtect.IsSizeProtected(),
                        rFlyProtect.IsPosProtected() );
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYPRTCTD;
        rWrt.OutULong( aP.GetValue() );
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtSurround( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtSurround& rFlySurround = (const SwFmtSurround&) rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rRTFWrt.bRTFFlySyntax )
    {
        if( SURROUND_NONE == rFlySurround.GetSurround() )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_NOWRAP;
        }
    }
    else
    {
        SwSurround eSurround = rFlySurround.GetSurround();
        BOOL bGold = SURROUND_IDEAL == eSurround;
        if( bGold )
            eSurround = SURROUND_PARALLEL;
        RTFSurround aMC( bGold, static_cast< BYTE >(eSurround) );
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYMAINCNT;
        rWrt.OutULong( aMC.GetValue() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtVertOrient ( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;
    RTFVertOrient aVO( static_cast< USHORT >(rFlyVert.GetVertOrient()), static_cast< USHORT >(rFlyVert.GetRelationOrient()) );
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pOrient;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        sal_Int16 eOrient = rFlyVert.GetRelationOrient();
        if (FLY_AT_PAGE == eAnchor)
        {
            if( text::RelOrientation::PAGE_FRAME == eOrient || text::RelOrientation::FRAME == eOrient )
                pOrient = OOO_STRING_SVTOOLS_RTF_PVPG;
            else
                pOrient = OOO_STRING_SVTOOLS_RTF_PVMRG;
        }
        else
            pOrient = OOO_STRING_SVTOOLS_RTF_PVPARA;
        rWrt.Strm() << pOrient;

        switch (rFlyVert.GetVertOrient())
        {
            case text::VertOrientation::TOP:
            case text::VertOrientation::LINE_TOP:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_POSYT;
                break;
            case text::VertOrientation::BOTTOM:
            case text::VertOrientation::LINE_BOTTOM:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_POSYB;
                break;
            case text::VertOrientation::CENTER:
            case text::VertOrientation::LINE_CENTER:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_POSYC;
                break;
            case text::VertOrientation::NONE:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_POSY;
                rWrt.OutULong(rFlyVert.GetPos());
                break;
            default:
                break;
        }
    }
    else if( !rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYVERT;
        rWrt.OutULong( aVO.GetValue() );
    }

    return rWrt;
}

static Writer& OutRTF_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtHoriOrient& rFlyHori = (const SwFmtHoriOrient&) rHt;
    RTFHoriOrient aHO( static_cast< USHORT >(rFlyHori.GetHoriOrient()),
                       static_cast< USHORT >(rFlyHori.GetRelationOrient()) );

    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pS;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        sal_Int16 eOrient = rFlyHori.GetRelationOrient();
        if (FLY_AT_PAGE == eAnchor)
        {
            if( text::RelOrientation::PAGE_FRAME == eOrient || text::RelOrientation::FRAME == eOrient )
                pS = OOO_STRING_SVTOOLS_RTF_PHPG;
            else
                pS = OOO_STRING_SVTOOLS_RTF_PHMRG;
        }
        else
            pS = OOO_STRING_SVTOOLS_RTF_PHCOL;
        rWrt.Strm() << pS;

        pS = 0;
        switch(rFlyHori.GetHoriOrient())
        {
            case text::HoriOrientation::RIGHT:
                pS = rFlyHori.IsPosToggle() ? OOO_STRING_SVTOOLS_RTF_POSXO : OOO_STRING_SVTOOLS_RTF_POSXR;
                break;
            case text::HoriOrientation::LEFT:
                pS = rFlyHori.IsPosToggle() ? OOO_STRING_SVTOOLS_RTF_POSXI : OOO_STRING_SVTOOLS_RTF_POSXL;
                break;
            case text::HoriOrientation::CENTER:
                pS = OOO_STRING_SVTOOLS_RTF_POSXC;
                break;
            case text::HoriOrientation::NONE:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_POSX;
                rWrt.OutULong( rFlyHori.GetPos() );
                break;
            default:
                break;
        }
        if (pS)
            rWrt.Strm() << pS;
    }
    else
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm()  << OOO_STRING_SVTOOLS_RTF_FLYHORZ;
        rWrt.OutULong( aHO.GetValue() );
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtAnchor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax )
    {
        const SwFmtAnchor& rAnchor = (const SwFmtAnchor&) rHt;
        USHORT nId = static_cast< USHORT >(rAnchor.GetAnchorId());
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYANCHOR;
        rWrt.OutULong( nId );
        rRTFWrt.bOutFmtAttr = TRUE;
        switch( nId )
        {
        case FLY_AT_PAGE:
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYPAGE;
                rWrt.OutULong( rAnchor.GetPageNum() );
            break;
        case FLY_AT_PARA:
        case FLY_AS_CHAR:
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FLYCNTNT;
            break;
        }
    }
    return rWrt;
}



static Writer& OutRTF_SwFmtBackground( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    // wird das FlyFrmFmt ausgegeben, dann Background nur ausgeben, wenn
    // RTF-Syntax gesetzt ist !
    if( !rRTFWrt.pFlyFmt || !rRTFWrt.bRTFFlySyntax )
    {
        const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
        if( !rBack.GetColor().GetTransparency() )
        {
            ByteString sOut( OOO_STRING_SVTOOLS_RTF_CBPAT );
            sOut += ByteString::CreateFromInt32(
                                rRTFWrt.GetId( rBack.GetColor() ));

            if( rRTFWrt.pFlyFmt || rRTFWrt.bOutPageDesc )
            {
                rWrt.Strm() << '{' << sOut.GetBuffer() << '}';
            }
            else
            {
                rRTFWrt.bOutFmtAttr = TRUE;
                rWrt.Strm() << sOut.GetBuffer();
            }
        }
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtShadow( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    // wird das FlyFrmFmt ausgegeben, dann Schatten nur ausgeben, wenn
    // nicht RTF-Syntax gesetzt ist !
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
        return rWrt;

    const SvxShadowItem& rShadow = (const SvxShadowItem&)rHt;
                             // FALSE wegen schliessender Klammer !!
    OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_SHADOW, FALSE );
    rWrt.OutULong( rShadow.GetLocation() ) << OOO_STRING_SVTOOLS_RTF_SHDWDIST;
    rWrt.OutULong( rShadow.GetWidth() ) << OOO_STRING_SVTOOLS_RTF_SHDWSTYLE;
    const Color& rColor = rShadow.GetColor();
    rWrt.OutULong( rColor.GetTransparency() ? SW_SV_BRUSH_NULL : SW_SV_BRUSH_SOLID );
    rWrt.OutULong( rRTFWrt.GetId( rColor ) ) << OOO_STRING_SVTOOLS_RTF_SHDWFCOL;
    rWrt.OutULong( 0 ) << '}';
    return rWrt;
}


static void OutBorderLine( SwRTFWriter& rWrt, const SvxBorderLine* pLine,
                            const char* pStr )
{
    rWrt.Strm() << pStr << OOO_STRING_SVTOOLS_RTF_BRDLNCOL;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) ) << OOO_STRING_SVTOOLS_RTF_BRDLNIN;
    rWrt.OutULong( pLine->GetInWidth() ) << OOO_STRING_SVTOOLS_RTF_BRDLNOUT;
    rWrt.OutULong( pLine->GetOutWidth() ) << OOO_STRING_SVTOOLS_RTF_BRDLNDIST;
    rWrt.OutULong( pLine->GetDistance() );
}


static Writer& OutRTF_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    const SvxBoxItem& rBox = (const SvxBoxItem&)rHt;

    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
#ifdef __MINGW32__ // for runtime pseudo reloc
    static const sal_Char* aBorderNames[] = {
#else
    static const sal_Char* __READONLY_DATA aBorderNames[] = {
#endif
            OOO_STRING_SVTOOLS_RTF_BRDRT, OOO_STRING_SVTOOLS_RTF_BRDRL, OOO_STRING_SVTOOLS_RTF_BRDRB, OOO_STRING_SVTOOLS_RTF_BRDRR };

    USHORT nDist = rBox.GetDistance();

    // wird das FlyFrmFmt ausgegeben, dann Border nur ausgeben, wenn
    // nicht RTF-Syntax gesetzt ist!
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
    {
/*
RTF kennt keine Rahmen Umrandung!
        // die normale RTF-Definition
        if( rBox.GetTop() && rBox.GetBottom() &&
            rBox.GetLeft() && rBox.GetRight() &&
            *rBox.GetTop() == *rBox.GetBottom() &&
            *rBox.GetTop() == *rBox.GetLeft() &&
            *rBox.GetTop() == *rBox.GetRight() )
            OutBorderLine( rRTFWrt, rBox.GetTop(), OOO_STRING_SVTOOLS_RTF_BOX, nDist );
        else
        {
            OUT_BRDLINE( rBox.GetTop(),     OOO_STRING_SVTOOLS_RTF_BRDRT, nDist );
            OUT_BRDLINE( rBox.GetBottom(),  OOO_STRING_SVTOOLS_RTF_BRDRB, nDist );
            OUT_BRDLINE( rBox.GetLeft(),    OOO_STRING_SVTOOLS_RTF_BRDRL, nDist );
            OUT_BRDLINE( rBox.GetRight(),   OOO_STRING_SVTOOLS_RTF_BRDRR, nDist );
        }
*/
        return rWrt;
    }
    else if( !rRTFWrt.pFlyFmt )
    {
        // erst die normale RTF-Definition, dann unsere eigene
        if( rBox.GetTop() && rBox.GetBottom() &&
            rBox.GetLeft() && rBox.GetRight() &&
            *rBox.GetTop() == *rBox.GetBottom() &&
            *rBox.GetTop() == *rBox.GetLeft() &&
            *rBox.GetTop() == *rBox.GetRight() &&
            nDist == rBox.GetDistance( BOX_LINE_TOP ) &&
            nDist == rBox.GetDistance( BOX_LINE_LEFT ) &&
            nDist == rBox.GetDistance( BOX_LINE_BOTTOM ) &&
            nDist == rBox.GetDistance( BOX_LINE_RIGHT ))
            OutBorderLine( rRTFWrt, rBox.GetTop(), OOO_STRING_SVTOOLS_RTF_BOX, nDist );
        else
        {
            const USHORT* pBrd = aBorders;
            const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
            for(int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms)
            {
                if (const SvxBorderLine* pLn = rBox.GetLine(*pBrd))
                {
                    OutBorderLine(rRTFWrt, pLn, *pBrdNms,
                        rBox.GetDistance(*pBrd));
                }
            }
        }
    }

    const USHORT* pBrd = aBorders;
    const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
    for( int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        if( pLn )
        {
            rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE;
            OutBorderLine( rRTFWrt, pLn, *pBrdNms );
            rWrt.Strm() << '}' << OOO_STRING_SVTOOLS_RTF_BRSP;
            rWrt.OutULong( rBox.GetDistance( *pBrd ));
        }
    }

    rRTFWrt.bOutFmtAttr = FALSE;
    return rWrt;
}

static Writer& OutRTF_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = ((SwRTFWriter&)rWrt);
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
        return rWrt;

    const SwFmtCol& rCol = (const SwFmtCol&)rHt;
    const SwColumns& rColumns = rCol.GetColumns();

    USHORT nCols = rColumns.Count();
    if( 1 < nCols )
    {
        // dann besorge mal die Seitenbreite ohne Raender !!
        const SwFrmFmt* pFmt;

        if( rRTFWrt.pFlyFmt )
            pFmt = rRTFWrt.pFlyFmt;
        else if( rRTFWrt.pAktPageDesc )
            pFmt = &rRTFWrt.pAktPageDesc->GetMaster();
        else
            pFmt = &const_cast<const SwDoc *>(rWrt.pDoc)
                ->GetPageDesc(0).GetMaster();

        const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();

        USHORT nPageSize = static_cast< USHORT >( pFmt->GetFrmSize().GetWidth() -
                            rLR.GetLeft() - rLR.GetRight() );

        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLS;
        rWrt.OutLong( nCols );

        if( rCol.IsOrtho() )
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLSX;
            rWrt.OutLong( rCol.GetGutterWidth( TRUE ) );
        }
        else
            for( USHORT n = 0; n < nCols; )
            {
                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLNO;
                rWrt.OutLong( n+1 );

                rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLW;
                rWrt.OutLong( rCol.CalcPrtColWidth( n, nPageSize ) );
                if( ++n != nCols )
                {
                    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_COLSR;
                    rWrt.OutLong( rColumns[ n-1 ]->GetRight() +
                                  rColumns[ n ]->GetLeft() );
                }
            }
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SvxFmtKeep( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFmtKeepItem& rItem = (const SvxFmtKeepItem&)rHt;
    if( rItem.GetValue() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_KEEPN;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SvxFrmDir( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = ((SwRTFWriter&)rWrt);
    if (rRTFWrt.pFlyFmt || rRTFWrt.bOutPageDesc)
        OutSvxFrmDir(rRTFWrt, rHt);
    return rWrt;
}

/* File GRFATR.HXX */

static Writer& OutRTF_SwMirrorGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwMirrorGrf & rMirror = (const SwMirrorGrf&)rHt;
    if( RES_MIRROR_GRAPH_DONT == rMirror.GetValue() )
        return rWrt;

    ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_GRFMIRROR;
    rWrt.OutULong( rMirror.GetValue() );
    return rWrt;
}

static Writer& OutRTF_SwCropGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwCropGrf & rCrop = (const SwCropGrf&)rHt;
    ByteString aStr;
    if( rCrop.GetLeft() )
        ( aStr += OOO_STRING_SVTOOLS_RTF_PICCROPL ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetLeft() );
    if( rCrop.GetRight() )
        ( aStr += OOO_STRING_SVTOOLS_RTF_PICCROPR ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetRight() );
    if( rCrop.GetTop() )
        ( aStr += OOO_STRING_SVTOOLS_RTF_PICCROPT ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetTop() );
    if( rCrop.GetBottom() )
        ( aStr += OOO_STRING_SVTOOLS_RTF_PICCROPB ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetBottom() );
    if( aStr.Len() )
    {
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
        rWrt.Strm() << aStr.GetBuffer();
    }
    return rWrt;
}



/* File PARATR.HXX  */

static Writer& OutRTF_SwLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxLineSpacingItem &rLs = (const SvxLineSpacingItem&)rHt;

    switch (rLs.GetLineSpaceRule())
    {
        default:
            break;
        case SVX_LINE_SPACE_AUTO:
        case SVX_LINE_SPACE_FIX:
        case SVX_LINE_SPACE_MIN:
        {
            ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_SL;
            sal_Char cMult = '0';
            switch (rLs.GetInterLineSpaceRule())
            {
                case SVX_INTER_LINE_SPACE_FIX:
                    // unser Durchschuss gibt es aber nicht in WW - also wie
                    // kommt man an die MaxLineHeight heran?
                    rWrt.OutLong((short)rLs.GetInterLineSpace());
                    break;
                case SVX_INTER_LINE_SPACE_PROP:
                    rWrt.OutLong((240L * rLs.GetPropLineSpace()) / 100L);
                    cMult = '1';
                    break;
                default:
                    if (SVX_LINE_SPACE_FIX == rLs.GetLineSpaceRule())
                        rWrt.Strm() << '-';
                    rWrt.OutLong( rLs.GetLineHeight() );
                    break;
            }
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_SLMULT << cMult;
        }
        break;
    }
    return rWrt;
}

static Writer& OutRTF_SwAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxAdjustItem & rAdjust = ((const SvxAdjustItem&)rHt);
    ByteString aAttr( "\\q" );
    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_LEFT:       aAttr += 'l';   break;
    case SVX_ADJUST_RIGHT:      aAttr += 'r';   break;
    case SVX_ADJUST_BLOCKLINE:
    case SVX_ADJUST_BLOCK:      aAttr += 'j';   break;
    case SVX_ADJUST_CENTER:     aAttr += 'c';   break;

    default:
        return rWrt;        // kein gueltiges Attriut, also returnen
    }
    ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    rWrt.Strm() << aAttr.GetBuffer();
    return rWrt;
}

static Writer& OutRTF_SvxFmtSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFmtSplitItem& rItem = (const SvxFmtSplitItem&)rHt;
    if( !rItem.GetValue() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_KEEP;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwTabStop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxTabStopItem & rTStops = (const SvxTabStopItem&)rHt;
    long nOffset = ((SvxLRSpaceItem&)rRTFWrt.GetItem( RES_LR_SPACE )).GetTxtLeft();
    for( USHORT n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop & rTS = rTStops[ n ];
        if( SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment() )
        {
            BOOL bOutDecimal = TRUE;
            const char* pFill = 0;
            switch( rTS.GetFill() )
            {
            case cDfltFillChar:
                break;

            case '.':   pFill = OOO_STRING_SVTOOLS_RTF_TLDOT;   break;
            case '_':   pFill = OOO_STRING_SVTOOLS_RTF_TLUL;    break;
            case '-':   pFill = OOO_STRING_SVTOOLS_RTF_TLTH;    break;
            case '=':   pFill = OOO_STRING_SVTOOLS_RTF_TLEQ;    break;
            default:
                if( !rRTFWrt.bWriteHelpFmt )
                {
                    OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_TLSWG, FALSE );
                    rWrt.OutULong( (((USHORT)rTS.GetFill()) << 8 ) +
                                    rTS.GetDecimal() )
                        << '}';
                    bOutDecimal = FALSE;
                }
            }
            if( pFill )
                rWrt.Strm() << pFill;

            if( !rRTFWrt.bWriteHelpFmt && bOutDecimal &&
                rTS.GetDecimal() != ((SvxTabStopItem&)*GetDfltAttr(
                    RES_PARATR_TABSTOP ))[ 0 ].GetDecimal() )
            {
                OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_TLSWG, FALSE );
                rWrt.OutULong( (((USHORT)rTS.GetFill()) << 8 ) +
                                rTS.GetDecimal() ) << '}';
            }

            const sal_Char* pAdjStr = 0;
            switch (rTS.GetAdjustment())
            {
                case SVX_TAB_ADJUST_RIGHT:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQR;
                    break;
                case SVX_TAB_ADJUST_DECIMAL:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQDEC;
                    break;
                case SVX_TAB_ADJUST_CENTER:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQC;
                    break;
                default:
                    break;
            }
            if (pAdjStr)
                rWrt.Strm() << pAdjStr;
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_TX;
            rWrt.OutLong(rTS.GetTabPos() + nOffset);
        }
    }
    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}

static Writer& OutRTF_SwHypenZone( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((SwRTFWriter&)rWrt).bWriteHelpFmt )
    {
        const SvxHyphenZoneItem& rAttr = (const SvxHyphenZoneItem&)rHt;
        USHORT nFlags = rAttr.IsHyphen() ? 1 : 0;
        if( rAttr.IsPageEnd() ) nFlags += 2;

        OutComment( rWrt, OOO_STRING_SVTOOLS_RTF_HYPHEN, FALSE );
        rWrt.OutULong( nFlags ) << OOO_STRING_SVTOOLS_RTF_HYPHLEAD;
        rWrt.OutULong( rAttr.GetMinLead() ) << OOO_STRING_SVTOOLS_RTF_HYPHTRAIL;
        rWrt.OutULong( rAttr.GetMinTrail() ) << OOO_STRING_SVTOOLS_RTF_HYPHMAX;
        rWrt.OutULong( rAttr.GetMaxHyphens() ) << '}';
    }
    return rWrt;
}

static Writer& OutRTF_SwNumRule( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bOutListNumTxt )
    {
        const SwNumRuleItem& rAttr = (const SwNumRuleItem&)rHt;
        USHORT nId;
        if( rAttr.GetValue().Len() &&
            USHRT_MAX != (nId = rRTFWrt.GetId( rAttr ) ))
        {
            rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_LS;
            rWrt.OutULong( nId );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
    return rWrt;
}

static Writer& OutRTF_SwScriptSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ((const SvxScriptSpaceItem&)rHt).GetValue() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_ASPALPHA;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}
static Writer& OutRTF_SwHangPunctuation( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((const SvxHangingPunctuationItem&)rHt).GetValue() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_NOOVERFLOW;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}
static Writer& OutRTF_SwForbiddenRule( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((const SvxForbiddenRuleItem&)rHt).GetValue() )
    {
        rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_NOCWRAP;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFontAlign( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxParaVertAlignItem & rAttr = (const SvxParaVertAlignItem &)rHt;
    const char* pStr;
    switch ( rAttr.GetValue() )
    {
    case SvxParaVertAlignItem::TOP:         pStr = OOO_STRING_SVTOOLS_RTF_FAHANG;       break;
    case SvxParaVertAlignItem::BOTTOM:      pStr = OOO_STRING_SVTOOLS_RTF_FAVAR;        break;
    case SvxParaVertAlignItem::CENTER:      pStr = OOO_STRING_SVTOOLS_RTF_FACENTER; break;
    case SvxParaVertAlignItem::BASELINE:    pStr = OOO_STRING_SVTOOLS_RTF_FAROMAN;  break;

//  case SvxParaVertAlignItem::AUTOMATIC:
    default:                                pStr = OOO_STRING_SVTOOLS_RTF_FAAUTO;       break;
    }
    rWrt.Strm() << pStr;
    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}

/*
 * lege hier die Tabellen fuer die RTF-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der RTF-DLL
 * bekannt sein muessen.
 */

SwAttrFnTab aRTFAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutRTF_SwCaseMap,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutRTF_SwColor,
/* RES_CHRATR_CONTOUR   */          OutRTF_SwContour,
/* RES_CHRATR_CROSSEDOUT    */      OutRTF_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutRTF_SwEscapement,
/* RES_CHRATR_FONT  */              OutRTF_SwFont,
/* RES_CHRATR_FONTSIZE  */          OutRTF_SwSize,
/* RES_CHRATR_KERNING   */          0, // NOT USED!! OutRTF_SwKerning,
/* RES_CHRATR_LANGUAGE  */          OutRTF_SwLanguage,
/* RES_CHRATR_POSTURE   */          OutRTF_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          OutRTF_SwShadowed,
/* RES_CHRATR_UNDERLINE */          OutRTF_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutRTF_SwWeight,
/* RES_CHRATR_WORDLINEMODE  */      0, // Neu: Wortweises Unter-/Durchstreichen
/* RES_CHRATR_AUTOKERN  */          0, // Neu: Automatisches Pairkerning
/* RES_CHRATR_BLINK */              0, // Neu: Blinkender Text
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutRTF_SwChrBckgrnd, // Neu: Zeichenhintergrund
/* RES_CHRATR_CJK_FONT */           OutRTF_SwFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutRTF_SwSize,
/* RES_CHRATR_CJK_LANGUAGE */       OutRTF_SwLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutRTF_SwPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutRTF_SwWeight,
/* RES_CHRATR_CTL_FONT */           OutRTF_SwFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutRTF_SwSize,
/* RES_CHRATR_CTL_LANGUAGE */       OutRTF_SwLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutRTF_SwPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutRTF_SwWeight,
/* RES_CHRATR_ROTATE */             OutRTF_SwCharRotate,
/* RES_CHRATR_EMPHASIS_MARK */      OutRTF_SwEmphasisMark,
/* RES_CHRATR_TWO_LINES */          OutRTF_SwTwoInOne,
/* RES_CHRATR_SCALEW */             OutRTF_SwCharScaleW,
/* RES_CHRATR_RELIEF */             OutRTF_SwCharRelief,
/* RES_CHRATR_HIDDEN */             OutRTF_SvxCharHiddenItem,
/* RES_CHRATR_OVERLINE */           OutRTF_SwOverline,
/* RES_CHRATR_DUMMY1 */             0,
/* RES_CHRATR_DUMMY2 */             0,

/* RES_TXTATR_REFMARK */            0, // NOT USED!! OutRTF_SwRefMark,
/* RES_TXTATR_TOXMARK */            0, // NOT USED!! OutRTF_SwTOXMark,
/* RES_TXTATR_META */               0,
/* RES_TXTATR_METAFIELD */          0,
/* RES_TXTATR_AUTOFMT */            OutRTF_SwTxtAutoFmt,
/* RES_TXTATR_INETFMT */            OutRTF_SwTxtINetFmt,
/* RES_TXTATR_CHARFMT */            OutRTF_SwTxtCharFmt,
/* RES_TXTATR_CJK_RUBY */           OutRTF_SwTxtRuby,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,

/* RES_TXTATR_FIELD */              OutRTF_SwField,
/* RES_TXTATR_FLYCNT */             OutRTF_SwFlyCntnt,
/* RES_TXTATR_FTN */                OutRTF_SwFtn,
/* RES_TXTATR_DUMMY4 */             0,
/* RES_TXTATR_DUMMY3 */             0,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      OutRTF_SwLineSpacing,
/* RES_PARATR_ADJUST    */          OutRTF_SwAdjust,
/* RES_PARATR_SPLIT */              OutRTF_SvxFmtSplit,
/* RES_PARATR_WIDOWS    */          0, // NOT USED!! OutRTF_SwWidows,
/* RES_PARATR_ORPHANS   */          0, // NOT USED!! OutRTF_SwOrphans,
/* RES_PARATR_TABSTOP   */          OutRTF_SwTabStop,
/* RES_PARATR_HYPHENZONE*/          OutRTF_SwHypenZone,
/* RES_PARATR_DROP */               0,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */            OutRTF_SwNumRule,
/* RES_PARATR_SCRIPTSPACE */        OutRTF_SwScriptSpace,
/* RES_PARATR_HANGINGPUNCTUATION */ OutRTF_SwHangPunctuation,
/* RES_PARATR_FORBIDDEN_RULE*/      OutRTF_SwForbiddenRule,
/* RES_PARATR_VERTALIGN */          OutRTF_SwFontAlign,
/* RES_PARATR_SNAPTOGRID*/          0, // new
/* RES_PARATR_CONNECT_TO_BORDER */  0, // new
/* RES_PARATR_OUTLINELEVEL */       0, // new - outlinelevel

/* RES_PARATR_LIST_ID */            0, // new
/* RES_PARATR_LIST_LEVEL */         0, // new
/* RES_PARATR_LIST_ISRESTART */     0, // new
/* RES_PARATR_LIST_RESTARTVALUE */  0, // new
/* RES_PARATR_LIST_ISCOUNTED */     0, // new

/* RES_FILL_ORDER   */              0, // NOT USED!! OutRTF_SwFillOrder,
/* RES_FRM_SIZE */                  OutRTF_SwFrmSize,
/* RES_PAPER_BIN    */              0, // NOT USED!! OutRTF_SwFmtPaperBin,
/* RES_LR_SPACE */                  OutRTF_SwFmtLRSpace,
/* RES_UL_SPACE */                  OutRTF_SwFmtULSpace,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    OutRTF_SwFmtHeader,
/* RES_FOOTER */                    OutRTF_SwFmtFooter,
/* RES_PRINT */                     OutRTF_SwFmtPrint,
/* RES_OPAQUE */                    OutRTF_SwFmtOpaque,
/* RES_PROTECT */                   OutRTF_SwFmtProtect,
/* RES_SURROUND */                  OutRTF_SwFmtSurround,
/* RES_VERT_ORIENT */               OutRTF_SwFmtVertOrient,
/* RES_HORI_ORIENT */               OutRTF_SwFmtHoriOrient,
/* RES_ANCHOR */                    OutRTF_SwFmtAnchor,
/* RES_BACKGROUND */                OutRTF_SwFmtBackground,
/* RES_BOX  */                      OutRTF_SwFmtBox,
/* RES_SHADOW */                    OutRTF_SwFmtShadow,
/* RES_FRMMACRO */                  0, // NOT USED!! OutRTF_SwFmtFrmMacro,
/* RES_COL */                       OutRTF_SwFmtCol,
/* RES_KEEP */                      OutRTF_SvxFmtKeep,
/* RES_URL */                       0, // URL
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_FRMATR_DUMMY1 */             0, // Dummy:
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_AUTO_STYLE */                0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRAMEDIR*/                   OutRTF_SvxFrmDir,
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_AUTO_STYLE */                0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:

/* RES_GRFATR_MIRRORGRF */          OutRTF_SwMirrorGrf,
/* RES_GRFATR_CROPGRF   */          OutRTF_SwCropGrf,
/* RES_GRFATR_ROTATION */           0,
/* RES_GRFATR_LUMINANCE */          0,
/* RES_GRFATR_CONTRAST */           0,
/* RES_GRFATR_CHANNELR */           0,
/* RES_GRFATR_CHANNELG */           0,
/* RES_GRFATR_CHANNELB */           0,
/* RES_GRFATR_GAMMA */              0,
/* RES_GRFATR_INVERT */             0,
/* RES_GRFATR_TRANSPARENCY */       0,
/* RES_GRFATR_DRWAMODE */           0,
/* RES_GRFATR_DUMMY1 */             0,
/* RES_GRFATR_DUMMY2 */             0,
/* RES_GRFATR_DUMMY3 */             0,
/* RES_GRFATR_DUMMY4 */             0,
/* RES_GRFATR_DUMMY5 */             0,

/* RES_BOXATR_FORMAT */             0,
/* RES_BOXATR_FORMULA */            0,
/* RES_BOXATR_VALUE */              0,

/* RES_UNKNOWNATR_CONTAINER */      0
};

SwNodeFnTab aRTFNodeFnTab = {
/* RES_TXTNODE  */                   OutRTF_SwTxtNode,
/* RES_GRFNODE  */                   OutRTF_SwGrfNode,
/* RES_OLENODE  */                   OutRTF_SwOLENode
};

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
