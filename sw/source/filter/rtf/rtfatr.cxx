/*************************************************************************
 *
 *  $RCSfile: rtfatr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:37:54 $
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

/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des RTF-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _COM_SUN_STAR_TEXT_SCRIPTTYPE_HDL_
#include <com/sun/star/text/ScriptType.hdl>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _RTFKEYWD_HXX //autogen
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <svtools/rtfout.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif


#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>           // fuer SwField ...
#endif
#ifndef _WRTRTF_HXX
#include <wrtrtf.hxx>
#endif
#ifndef _RTF_HXX
#include <rtf.hxx>          // fuer SwPictureType
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc ...
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer SwPageDesc ...
#endif
#ifndef _LINETYPE_HXX
#include <linetype.hxx>     // fuer SwPageDesc ...
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif



/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
 */
#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

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

//-----------------------------------------------------------------------

static Writer& OutRTF_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt );

//-----------------------------------------------------------------------

SvStream& OutComment( Writer& rWrt, const sal_Char* pStr )
{
    return (rWrt.Strm() << '{' << sRTF_IGNORE << pStr);
}

SvStream& OutComment( Writer& rWrt, const char* pStr, BOOL bSetFlag )
{
    // setze Flag, damit bei der Ausgabe von Attributen zu erkennen,
    // ob ueberhaupt etwas ausgegeben wurde.
    ((SwRTFWriter&)rWrt).bOutFmtAttr = bSetFlag;
    return (rWrt.Strm() << '{' << sRTF_IGNORE << pStr);
}

Writer& OutRTF_AsByteString( Writer& rWrt, const String& rStr )
{
    ByteString sOutStr( rStr, DEF_ENCODING );
    rWrt.Strm() << sOutStr.GetBuffer();
    return rWrt;
}

// fuer die Formate muesste eine einzige Ausgabe-Funktion genuegen !
/*
 * Formate wie folgt ausgeben:
 *      - gebe alle Attribute vom Format aus
 */

Writer& OutRTF_SwFmt( Writer& rWrt, const SwFmt& rFmt )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    BOOL bOutItemSet = TRUE;

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        {
            USHORT nId = rRTFWrt.GetId( (const SwTxtFmtColl&)rFmt );
            if( 0 == nId )
                return rWrt;        // Default-TextStyle nicht ausgeben !!

            rWrt.Strm() << sRTF_S;
            rWrt.OutULong( nId );
            rRTFWrt.bOutFmtAttr = TRUE;
            BYTE nLvl = ((const SwTxtFmtColl&)rFmt).GetOutlineLevel();
            if( MAXLEVEL > nLvl )
            {
                BYTE nWWLvl = 8 >= nLvl ? nLvl : 8;
                rWrt.Strm() << sRTF_LS;
                rWrt.OutULong( rWrt.pDoc->GetNumRuleTbl().Count() );
                rWrt.Strm() << sRTF_ILVL; rWrt.OutULong( nWWLvl );
                rWrt.Strm() << sRTF_OUTLINELEVEL; rWrt.OutULong( nWWLvl );
                if( nWWLvl != nLvl )            // RTF-kennt nur 9 Ebenen
                {
                    OutComment( rWrt, sRTF_SOUTLVL );
                    rWrt.OutULong( nLvl ) << '}';
                }

                const SwNumFmt* pNFmt = &rWrt.pDoc->GetOutlineNumRule()->Get( nLvl );
                if( pNFmt->GetAbsLSpace() )
                {
                    SfxItemSet aSet( *rFmt.GetAttrSet().GetPool(),
                                    rFmt.GetAttrSet().GetRanges() );
                    aSet.SetParent( &rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)aSet.Get( RES_LR_SPACE ) );

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + pNFmt->GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( pNFmt->GetFirstLineOffset() );

                    aSet.Put( aLR );
                    Out_SfxItemSet( aRTFAttrFnTab, rWrt, aSet, TRUE );
                    bOutItemSet = FALSE;
                }
            }
        }
        break;
    case RES_CHRFMT:
        {
            USHORT nId = rRTFWrt.GetId( (const SwCharFmt&)rFmt );
            if( 0 == nId )
                return rWrt;        // Default-CharStyle nicht ausgeben !!

            rWrt.Strm() << sRTF_IGNORE << sRTF_CS;
            rWrt.OutULong( nId );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
        break;

//  case RES_GRFFMTCOLL:
// ?????
    }

    if( bOutItemSet )
        Out_SfxItemSet( aRTFAttrFnTab, rWrt, rFmt.GetAttrSet(), TRUE );
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
        rRTFWrt.bOutFmtAttr = FALSE;
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
        rRTFWrt.SetStrm( aTmpStrm );

        rRTFWrt.bRTFFlySyntax = FALSE;
        OutRTF_SwFmt( rRTFWrt, *rRTFWrt.pFlyFmt );

        rRTFWrt.SetStrm( *pSaveStrm );  // Stream-Pointer wieder zurueck

        if( aTmpStrm.GetSize() )            // gibt es SWG spezifische Attribute ??
        {
            aTmpStrm.Seek( 0L );
            rRTFWrt.Strm() << '{' << sRTF_IGNORE << aTmpStrm << '}';
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
    _EndPosLst::Count;
    _EndPosLst::operator[];
    _EndPosLst::DeleteAndDestroy;

    RTFEndPosLst( SwRTFWriter& rWrt, const SwTxtNode& rNd );
    ~RTFEndPosLst();

    USHORT GetCurScript() const         { return nCurScript; }
    BOOL MatchScriptToId( USHORT nId ) const;
    int Insert( const SfxPoolItem& rAttr, xub_StrLen nStt, xub_StrLen nEnd );

    void OutAttrs( xub_StrLen nStrPos );
    void EndAttrs( xub_StrLen nStrPos );
    void OutScriptChange( xub_StrLen nStrPos );

    const SfxPoolItem* HasItem( USHORT nWhich ) const;
    const SfxPoolItem& GetItem( USHORT nWhich ) const;
    void OutFontAttrs( USHORT nScript );
};


SttEndPos::SttEndPos( const SfxPoolItem& rAttr,
                        xub_StrLen nStt, xub_StrLen nEd )
    : nStart( nStt ), nEnd( nEd )
{
    AddAttr( rAttr );
}

SttEndPos::~SttEndPos()
{
    for( USHORT n = 0, nEnd = aArr.Count(); n < nEnd; ++n )
        if( RES_FLTRATTR_BEGIN <= aArr[ n ]->Which() )
            delete aArr[ n ];
}

BOOL SttEndPos::HasScriptChange() const
{
    for( USHORT n = 0, nEnd = aArr.Count(); n < nEnd; ++n )
        if( RES_FLTR_SCRIPTTYPE == aArr[ n ]->Which() )
            return TRUE;
    return FALSE;
}

void SttEndPos::AddAttr( const SfxPoolItem& rAttr )
{
    const SfxPoolItem* pI = &rAttr;
    USHORT n = aArr.Count();
    switch( rAttr.Which() )
    {
    case RES_FLTR_SCRIPTTYPE:
        {
            while( n )
                switch( aArr[ --n ]->Which() )
                {
                case RES_CHRATR_FONT:
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_LANGUAGE:
                case RES_CHRATR_POSTURE:
                case RES_CHRATR_WEIGHT:
                case RES_CHRATR_CJK_FONT:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_CJK_POSTURE:
                case RES_CHRATR_CJK_WEIGHT:
                case RES_CHRATR_CTL_FONT:
                case RES_CHRATR_CTL_FONTSIZE:
                case RES_CHRATR_CTL_LANGUAGE:
                case RES_CHRATR_CTL_POSTURE:
                case RES_CHRATR_CTL_WEIGHT:
                    aArr.Remove( n, 1 );
                    break;
                }
        }
        break;
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        while( n )
            if( RES_FLTR_SCRIPTTYPE == aArr[ --n ]->Which() )
            {
                pI = 0;
                break;
            }
        break;
    }
    if( pI )
        aArr.Insert( pI, aArr.Count() );
}

RTFEndPosLst::RTFEndPosLst( SwRTFWriter& rWriter, const SwTxtNode& rNd )
    : rNode( rNd ), rWrt( rWriter ), nCurPos( -1 )
{
    pOldPosLst = rWrt.pCurEndPosLst;
    rWrt.pCurEndPosLst = this;

    // JP 15.11.00: first default to latin - this must be change, if it
    //              clear how we handle empty paragraphs
    nCurScript = ::com::sun::star::text::ScriptType::LATIN;

    // search the script changes and add this positions as a Item into
    // the attribute list
    if( pBreakIt->xBreak.is() )
    {
        const String& rTxt = rNode.GetTxt();
        xub_StrLen nChg = 0, nLen = rTxt.Len(), nSttPos = 0;

        while( nChg < nLen )
        {
            USHORT nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
            nChg = pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
               switch( nScript )
            {
            case ::com::sun::star::text::ScriptType::LATIN:
            case ::com::sun::star::text::ScriptType::ASIAN:
            case ::com::sun::star::text::ScriptType::COMPLEX:
                if( nSttPos != nChg )
                {
                    SfxPoolItem* pAttr = new
                        SfxInt16Item( RES_FLTR_SCRIPTTYPE, nScript );
                    Insert( *pAttr, nSttPos, nChg );
                    nSttPos = nChg;
                }
                break;
            }
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
    if( nStt == nEnd )
        return FALSE;

    USHORT nPos;
    for( nPos = 0; nPos < Count(); ++nPos )
    {
        SttEndPos* pTmp = GetObject( nPos );
        if( pTmp->GetStart() == nStt && pTmp->GetEnd() == nEnd )
        {
            pTmp->AddAttr( rAttr );
            return FALSE;       // schon vorhanden
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
                    OutFontAttrs( ((SfxUInt16Item*)pItem)->GetValue() );
                else
                    Out( aRTFAttrFnTab, *pItem, rWrt );
            }
        }

    nCurPos = -1;
}

void RTFEndPosLst::OutFontAttrs( USHORT nScript )
{
    // script change, write the correct attributes:
    // start first with the Fontname

    rWrt.bOutFmtAttr = TRUE;
    nCurScript = nScript;

    USHORT nLatin = rWrt.GetId( (SvxFontItem&)GetItem( RES_CHRATR_FONT )),
            nAsian = rWrt.GetId( (SvxFontItem&)GetItem( RES_CHRATR_CJK_FONT )),
            nCmplx = rWrt.GetId( (SvxFontItem&)GetItem( RES_CHRATR_CTL_FONT ));

    static const USHORT aLatinIds[] =  {
            RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CJK_WEIGHT,
            0
        },
        aAsianIds[] =  {
            RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CJK_WEIGHT,
            0
        },
        aCmplxIds[] =  {
            RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CJK_WEIGHT,
            0
        };
    const USHORT* pIdArr = aLatinIds;

    // size/weight/posture optional
    switch( nScript )
    {
    case ::com::sun::star::text::ScriptType::LATIN:
        rWrt.Strm() << sRTF_RTLCH << sRTF_AF;
        rWrt.OutULong( nCmplx );
        rWrt.Strm() << sRTF_LTRCH << sRTF_DBCH << sRTF_AF;
        rWrt.OutULong( nAsian ) << sRTF_HICH << sRTF_AF;
        rWrt.OutULong( nLatin ) << sRTF_LOCH << sRTF_F;
        rWrt.OutULong( nLatin );
        break;
    case ::com::sun::star::text::ScriptType::ASIAN:
        rWrt.Strm() << sRTF_RTLCH << sRTF_AF;
        rWrt.OutULong( nCmplx );
        rWrt.Strm() << sRTF_LTRCH << sRTF_LOCH << sRTF_AF;
        rWrt.OutULong( nLatin ) << sRTF_HICH << sRTF_AF;
        rWrt.OutULong( nLatin ) << sRTF_DBCH << sRTF_F;
        rWrt.OutULong( nAsian );
        pIdArr = aAsianIds;
        break;

    case ::com::sun::star::text::ScriptType::COMPLEX:
        rWrt.Strm() << sRTF_LTRCH << sRTF_DBCH << sRTF_AF;
        rWrt.OutULong( nAsian ) << sRTF_LOCH << sRTF_AF;
        rWrt.OutULong( nLatin ) << sRTF_HICH << sRTF_AF;
        rWrt.OutULong( nLatin );
        rWrt.Strm() << sRTF_RTLCH << sRTF_F;
        rWrt.OutULong( nCmplx );
        pIdArr = aCmplxIds;
        break;
    }

    FnAttrOut pOut;
    const SfxPoolItem* pItem;
    for( ; *pIdArr; ++pIdArr )
    {
        if( 0 != (pItem = HasItem( *pIdArr )) &&
            0 != ( pOut = aRTFAttrFnTab[ *pIdArr - RES_CHRATR_BEGIN] ))
            (*pOut)( rWrt, *pItem );
    }
}

void RTFEndPosLst::EndAttrs( xub_StrLen nStrPos )
{
    SttEndPos* pSEPos;
    while( 0 != Count() && 0 != (pSEPos = GetObject( 0 )) &&
        ( STRING_MAXLEN == nStrPos || nStrPos == pSEPos->GetEnd() ))
    {
        rWrt.Strm() << '}';

        const SfxPoolItems& rAttrs = pSEPos->GetAttrs();
        for( USHORT nAttr = rAttrs.Count(); nAttr; )
            if( RES_TXTATR_INETFMT == rAttrs[ --nAttr ]->Which() )
            {
                // Hyperlinks werden als Felder geschrieben, aber der
                // "FieldResult" steht als Text im TextNode. Also muss
                // bei diesen Attributen am Ende 2 Klammern stehen!
                rWrt.Strm() << "}}";
            }

        DeleteAndDestroy( 0, 1 );
    }
}

BOOL RTFEndPosLst::MatchScriptToId( USHORT nWhich ) const
{
    BOOL bRet = FALSE;
    switch( nWhich )
    {
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        bRet = nCurScript == ::com::sun::star::text::ScriptType::LATIN;
        break;
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        bRet = nCurScript == ::com::sun::star::text::ScriptType::ASIAN;
        break;
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        bRet = nCurScript == ::com::sun::star::text::ScriptType::COMPLEX;
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

static Writer& OutRTF_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    RTFEndPosLst aEndPosLst( rRTFWrt, *pNd );
    USHORT nAttrPos = 0;
    xub_StrLen nStrPos = rRTFWrt.pCurPam->GetPoint()->nContent.GetIndex();

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
                        FALSE, &pItem ) || *pItem != *pCurr )
                    {
                        bEqual = FALSE;
                        break;
                    }

                    if( aIter.IsAtEnd() )
                        break;
                    pCurr = aIter.NextItem();
                }
            }
            if( bEqual )
                bNewFmts = FALSE;
        }
        rRTFWrt.SetAttrSet( &pNd->GetSwAttrSet() );
        rRTFWrt.bAutoAttrSet = 0 != pNdSet;
    }

    // Flag zuruecksetzen, damit nach der Ausgabe der Collection
    // getestet werden kann, ob noch ein Blank auszugeben ist
    rRTFWrt.bOutFmtAttr = FALSE;

    // in der Ausgabe eines Flys? Dann vorm ausgeben der AbsatzAttribute
    // den Format-Pointer auf 0 setzen!
    SwFlyFrmFmt* pSaveFmt = rRTFWrt.pFlyFmt;

    if( bNewFmts && rRTFWrt.bWriteAll )
    {
        rRTFWrt.Strm() << sRTF_PARD << sRTF_PLAIN << ' ';       // alle Attribute zuruecksetzen
        if( rRTFWrt.bOutTable )
            rRTFWrt.Strm() << sRTF_INTBL;

        // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
        // muessen noch die Attribute dafuer ausgegeben werden.
        if( pSaveFmt )
        {
            OutRTF_SwFlyFrmFmt( rRTFWrt );
            rRTFWrt.pFlyFmt = 0;
        }

        rRTFWrt.OutListNum( *pNd );

        OutRTF_SwFmt( rRTFWrt, pNd->GetAnyFmtColl() );
    }

    // gibt es harte Attributierung ?
    if( bNewFmts && pNd->GetpSwAttrSet() )
    {
        rRTFWrt.pFlyFmt = 0;

        const SfxItemSet& rNdSet = pNd->GetSwAttrSet();

        const SwNumRule* pRule;
        const SwNodeNum* pNum;
        if( (( 0 != ( pNum = pNd->GetNum() ) &&
                0 != ( pRule = pNd->GetNumRule() )) ||
                ( 0 != ( pNum = pNd->GetOutlineNum() ) &&
                0 != ( pRule = rWrt.pDoc->GetOutlineNumRule() ) ) ) &&
                pNum->GetLevel() < NO_NUM )
        {
            BYTE nLvl = GetRealLevel( pNum->GetLevel() );
            const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
            if( !pFmt )
                pFmt = &pRule->Get( nLvl );

            SfxItemSet aSet( rNdSet );
            SvxLRSpaceItem aLR( (SvxLRSpaceItem&)rNdSet.Get( RES_LR_SPACE ) );

            aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );
            if( MAXLEVEL > pNum->GetLevel() )
                aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
            else
                aSet.ClearItem( RES_PARATR_NUMRULE );
            aSet.Put( aLR );
            Out_SfxItemSet( aRTFAttrFnTab, rWrt, aSet, FALSE );
        }
        else
            Out_SfxItemSet( aRTFAttrFnTab, rWrt, rNdSet, FALSE );
    }

    rRTFWrt.pFlyFmt = pSaveFmt;

    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    const SwTxtAttr * pHt = 0;
    USHORT nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        rRTFWrt.bTxtAttr = TRUE;
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

        rRTFWrt.bTxtAttr = FALSE;
    }

    if( !rRTFWrt.bWriteAll && rRTFWrt.bFirstLine )
    {
        if( rRTFWrt.bOutFmtAttr )
        {
            rRTFWrt.Strm() << ' ';
            rRTFWrt.bOutFmtAttr = FALSE;
        }
        OutRTF_SwFmt( rRTFWrt, pNd->GetAnyFmtColl() );
    }

    if( rRTFWrt.bOutFmtAttr &&
        ( nAttrPos < nCntAttr ? *pHt->GetStart() != nStrPos : TRUE ))
        rRTFWrt.Strm() << ' ';

    // das Flag gibt an, ob das SwTxtFld am Ende vom Absatz steht. Denn
    // dann ist vor dem Absatzumbruch die schliessende Klammer auszugeben
    xub_StrLen nChrCnt = 0;
    for( ; nStrPos <= nEnde; nStrPos++ )
    {
        if( nStrPos != nEnde && aEndPosLst.Count() )
            aEndPosLst.EndAttrs( nStrPos );

        // versuche nach ungefaehr 255 Zeichen eine neue Zeile zu beginnen
        if( nChrCnt != ( nStrPos & 0xff00 ))
        {
            rWrt.Strm() << SwRTFWriter::sNewLine;
            nChrCnt = nStrPos & 0xff00;
        }

        rRTFWrt.bTxtAttr = TRUE;
        rRTFWrt.bOutFmtAttr = FALSE;

        if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
            && nStrPos != nEnde )
        {
            do {
                BOOL bEmpty = FALSE;
                if( pHt->GetEnd() )
                {
                    if( FALSE == (bEmpty = *pHt->GetEnd() == nStrPos) )
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
                    rRTFWrt.bOutFmtAttr = FALSE;
                }
            } while( ++nAttrPos < nCntAttr && nStrPos ==
                *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

            // dann gebe mal alle gesammelten Attribute von der String-Pos aus
            aEndPosLst.OutAttrs( nStrPos );

        }
        else
            aEndPosLst.OutScriptChange( nStrPos );

        if( rRTFWrt.bOutFmtAttr )
            rRTFWrt.Strm() << ' ';
        rRTFWrt.bTxtAttr = FALSE;

        rRTFWrt.OutBookmarks( nStrPos );

        if( nStrPos != nEnde )
            RTFOutFuncs::Out_Char( rWrt.Strm(), rStr.GetChar( nStrPos ),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
    }

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

    rRTFWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PAR << ' ';
    return rRTFWrt;
}



static Writer& OutRTF_SwGrfNode( Writer& rWrt, SwCntntNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    SwGrfNode * pNd = &((SwGrfNode&)rNode);

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rRTFWrt.pFlyFmt )
        OutRTF_SwFlyFrmFmt( rRTFWrt );
    rRTFWrt.Strm() << "{{";

    // damit die eigenen Grafik-Attribute nach der PICT / import Anweisung
    // stehen, muessen die am Anfang ausgegeben werden.
    rRTFWrt.bOutFmtAttr = FALSE;
    OutRTF_SwFmt( rRTFWrt, *pNd->GetFmtColl() );

    if( rRTFWrt.bOutFmtAttr )       // wurde ueberhaupt ein Attrribut
        rRTFWrt.Strm() << "}{";     // ausgegeben ??

    String aGrfNm;
    const SwMirrorGrf& rMirror = pNd->GetSwAttrSet().GetMirrorGrf();
    if( !pNd->IsLinkedFile() || RES_DONT_MIRROR_GRF != rMirror.GetValue() )
    {
        // Grafik als File-Referenz speichern (als JPEG-Grafik speichern)
        aGrfNm = *rWrt.GetOrigFileName();
        pNd->SwapIn( TRUE );
        ULONG nFlags = 0;
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRF_VERT:   nFlags = XOUTBMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRF_HOR:    nFlags = XOUTBMP_MIRROR_VERT; break;
        case RES_MIRROR_GRF_BOTH:
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

        USHORT nErr = XOutBitmap::WriteGraphic( pNd->GetGrf(), aGrfNm,
                String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "JPG" )),
                                                nFlags, pMM100Size );
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
    rRTFWrt.Strm() << sRTF_FIELD << sRTF_FLDPRIV;
    OutComment( rRTFWrt, sRTF_FLDINST ) << "{\\\\import ";
    RTFOutFuncs::Out_String( rWrt.Strm(), aGrfNm, DEF_ENCODING,
                                rRTFWrt.bWriteHelpFmt );
    rRTFWrt.Strm() << "}}{" << sRTF_FLDRSLT << " }}";

    rRTFWrt.Strm() << '}' << SwRTFWriter::sNewLine;
    return rRTFWrt;
}

static Writer& OutRTF_SwOLENode( Writer& rWrt, SwCntntNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rRTFWrt.pFlyFmt )
        OutRTF_SwFlyFrmFmt( rRTFWrt );

    rWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PAR;
    return rWrt;
}


static void OutTBLBorderLine( SwRTFWriter& rWrt, const SvxBorderLine* pLine,
                                const sal_Char* pStr, USHORT nDist )
{
    ByteString sLineStr;
    if( pLine->GetInWidth() )
    {
        // doppelte Linie
        sLineStr = sRTF_BRDRDB;
        switch( pLine->GetInWidth() )
        {
        case DEF_LINE_WIDTH_0:
            ( sLineStr += sRTF_BRDRW ) += "15";
            break;
        case DEF_LINE_WIDTH_1:
            ( sLineStr += sRTF_BRDRW ) += "30";
            break;
        case DEF_LINE_WIDTH_2:
        case DEF_LINE_WIDTH_3:
            ( sLineStr += sRTF_BRDRW ) += "45";
            break;
        }
    }
    else
    {
        // einfache Linie
        if( DEF_LINE_WIDTH_1 >= pLine->GetOutWidth() )
            (( sLineStr = sRTF_BRDRS ) += sRTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() );
        else
            (( sLineStr = sRTF_BRDRTH ) += sRTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() / 2 );
    }

    rWrt.Strm() << pStr << sLineStr.GetBuffer() << sRTF_BRDRCF;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) ) << sRTF_BRSP;
    rWrt.OutULong( nDist );
}

static void OutSwTblBorder( SwRTFWriter& rWrt, const SvxBoxItem& rBox )
{
    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    static const char* __READONLY_DATA aBorderNames[] = {
            sRTF_CLBRDRT, sRTF_CLBRDRL, sRTF_CLBRDRB, sRTF_CLBRDRR };
    const USHORT* pBrd = aBorders;
    const char** pBrdNms = (const char**)aBorderNames;
    for( int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms )
    {
        // funktioniert im WinWord so nicht
        USHORT nDist = 0; //rBox.GetDistance( *pBrd );
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        if( pLn )
            OutTBLBorderLine( rWrt, pLn, *pBrdNms, nDist );
        else if( nDist )
        {
            rWrt.Strm() << *pBrdNms << sRTF_BRSP;
            rWrt.OutULong( nDist );
        }
    }
}

static void OutSwTblBackground( SwRTFWriter& rWrt, const SvxBrushItem& rBack )
{
    if( !rBack.GetColor().GetTransparency() )
    {
        rWrt.Strm() << sRTF_CLCBPAT;
        rWrt.OutULong( rWrt.GetId( rBack.GetColor() ) );
    }
}


Writer& OutRTF_SwTblNode( Writer& rWrt, SwTableNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwTable& rTbl = rNode.GetTable();
    const SwTableLines& rLns = rTbl.GetTabLines();
    SwTwips nPageSize = 0, nTblOffset = 0;

/*
//!!!!!!!!!!!!! for clipboard create any view if the table is complex !!!
    if( rTbl.IsTblComplex() )
    {
        // then we have to create any layout
        SFX_APP()->CreateViewFrame( *xDocSh, 0, TRUE );
    }
*/

    {
        Point aPt;
        SwRect aRect( rTbl.GetFrmFmt()->FindLayoutRect( FALSE, &aPt ));
        if( aRect.IsEmpty() )
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pFmt = rRTFWrt.pFlyFmt ? rRTFWrt.pFlyFmt :
                rWrt.pDoc->GetPageDesc(0).GetPageFmtOfNode( rNode, FALSE );

            aRect = pFmt->FindLayoutRect( TRUE );
            if( 0 == ( nPageSize = aRect.Width() ))
            {
                const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
                nPageSize = pFmt->GetFrmSize().GetWidth() -
                                rLR.GetLeft() - rLR.GetRight();
            }
        }
        else
            nPageSize = aRect.Width();
    }

    SwTwips nTblSz = rTbl.GetFrmFmt()->GetFrmSize().GetWidth();

    ByteString aTblAdjust( sRTF_TRQL );
    switch( rTbl.GetFrmFmt()->GetHoriOrient().GetHoriOrient() )
    {
    case HORI_CENTER:   aTblAdjust = sRTF_TRQC; break;
    case HORI_RIGHT:    aTblAdjust = sRTF_TRQR; break;
    case HORI_NONE:
    case HORI_LEFT_AND_WIDTH:
            {
                const SvxLRSpaceItem& rLRSp = rTbl.GetFrmFmt()->GetLRSpace();
                nTblOffset = rLRSp.GetLeft();
                nPageSize -= (nTblOffset + rLRSp.GetRight());
                aTblAdjust += sRTF_TRLEFT;
                aTblAdjust += ByteString::CreateFromInt32( nTblOffset );
            }
            break;
//  case case FLY_HORI_FULL:
    }

    // ist die Tabelle wesentlich (PageSize + 10%) groesser als die Seite,
    // dann sind die Box-Breiten relative Angaben.
    BOOL bRelBoxSize = TRUE /*ALWAYS relativ (nPageSize + ( nPageSize / 10 )) < nTblSz*/;

    SwWriteTable* pTableWrt;
    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        pTableWrt = new SwWriteTable( pLayout );
    else
        pTableWrt = new SwWriteTable( rTbl.GetTabLines(), (USHORT)nPageSize,
                                          (USHORT)nTblSz, FALSE );

    // rCols are the array of all cols of the table
    const SwWriteTableCols& rCols = pTableWrt->GetCols();
    USHORT nColCnt = rCols.Count();
    SwWriteTableCellPtr* pBoxArr = new SwWriteTableCellPtr[ nColCnt ];
    USHORT* pRowSpans = new USHORT[ nColCnt ];
    memset( pBoxArr, 0, sizeof( pBoxArr[0] ) * nColCnt );
    memset( pRowSpans, 0, sizeof( pRowSpans[0] ) * nColCnt );
    long nLastHeight = 0;
    const SwWriteTableRows& rRows = pTableWrt->GetRows();
    for( USHORT nLine = 0; nLine < rRows.Count(); ++nLine )
    {
        USHORT nBox;

        const SwWriteTableRow *pRow = rRows[ nLine ];
        const SwWriteTableCells& rCells = pRow->GetCells();

        BOOL bFixRowHeight = FALSE;
        for( nColCnt = 0, nBox = 0; nBox < rCells.Count(); ++nColCnt )
        {
            if( !pRowSpans[ nColCnt ] )
            {
                // set new BoxPtr
                SwWriteTableCell* pCell = rCells[ nBox++ ];
                pBoxArr[ nColCnt ] = pCell;
                pRowSpans[ nColCnt ] = pCell->GetRowSpan();
                for( USHORT nCellSpan = pCell->GetColSpan(), nCS = 1;
                        nCS < nCellSpan; ++nCS, ++nColCnt )
                {
                    pBoxArr[ nColCnt+1 ] = pBoxArr[ nColCnt ];
                    pRowSpans[ nColCnt+1 ] = pRowSpans[ nColCnt ];
                }
            }
            if( 1 != pRowSpans[ nColCnt ] )
                bFixRowHeight = TRUE;
        }

        for( ; nColCnt < rCols.Count() && pRowSpans[ nColCnt ]; ++nColCnt )
            bFixRowHeight = TRUE;

        // Start Tabellendefinition
        rWrt.Strm() << sRTF_TROWD << aTblAdjust.GetBuffer();

        if( !nLine && rTbl.IsHeadlineRepeat() )
            rWrt.Strm() << sRTF_TRHDR;

        // Zeilenhoehe ausgeben
        long nHeight = 0;
        if( bFixRowHeight && rWrt.pDoc->GetRootFrm() )
        {
            nHeight = -pRow->GetPos();      //neg. => abs. height!
            if( nLine )
                nHeight += rRows[ nLine - 1 ]->GetPos();

            // merged line -> dont split it
            rWrt.Strm() << sRTF_TRKEEP;
        }
        else
        {
            const SwTableLine* pLine = pBoxArr[ 0 ]->GetBox()->GetUpper();
            const SwFmtFrmSize& rLSz = pLine->GetFrmFmt()->GetFrmSize();
            if( ATT_VAR_SIZE != rLSz.GetSizeType() && rLSz.GetHeight() )
                nHeight = ATT_MIN_SIZE == rLSz.GetSizeType()
                                                ? rLSz.GetHeight()
                                                : -rLSz.GetHeight();
        }
        if( nHeight )
        {
            rWrt.Strm() << sRTF_TRRH;
            rWrt.OutLong( nHeight );
        }

        // Breite der Boxen ausgeben
        SwTwips nSz = 0, nCalc;
        for( nBox = 0; nBox < nColCnt; ++nBox )
        {
            SwWriteTableCell* pCell = pBoxArr[ nBox ];
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            const SwFrmFmt& rFmt = *pCell->GetBox()->GetFrmFmt();
            if( 1 < pCell->GetRowSpan() )
                rWrt.Strm() << ( pCell->GetRowSpan() == pRowSpans[ nBox ]
                                ? sRTF_CLVMGF
                                : sRTF_CLVMRG );

            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
                                            RES_BOX, TRUE, &pItem ) )
                OutSwTblBorder( rRTFWrt, (SvxBoxItem&)*pItem );

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
                case VERT_CENTER:   rWrt.Strm() << sRTF_CLVERTALC; break;
                case VERT_BOTTOM:   rWrt.Strm() << sRTF_CLVERTALB; break;
                default:            rWrt.Strm() << sRTF_CLVERTALT; break;
                }

            const SwFmtFrmSize& rLSz = rFmt.GetFrmSize();
            nSz += rLSz.GetWidth();
            rWrt.Strm() << sRTF_CELLX;

            nCalc = nSz;
            if( bRelBoxSize )
            {
                nCalc *= nPageSize;
                nCalc /= nTblSz;
            }
            rWrt.OutLong( nTblOffset + nCalc );
        }

        // Inhalt der Boxen ausgeben
        rWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PARD << sRTF_INTBL;
        for( nBox = 0; nBox < nColCnt; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            if( pBoxArr[ nBox ]->GetRowSpan() == pRowSpans[ nBox ] )
            {
                // new Box
                const SwStartNode* pSttNd = pBoxArr[ nBox ]->GetBox()->GetSttNd();
                RTFSaveData aSaveData( rRTFWrt,
                        pSttNd->GetIndex()+1, pSttNd->EndOfSectionIndex() );
                rRTFWrt.bOutTable = TRUE;
                rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
            }
            rWrt.Strm() << sRTF_CELL;
        }

        // das wars mit der Line
        rWrt.Strm() << sRTF_ROW << sRTF_PARD << ' ';

        for( nBox = 0; nBox < nColCnt; ++nBox )
            --pRowSpans[ nBox ];
    }

    delete pTableWrt;
    delete pBoxArr;
    delete pRowSpans;

    // Pam hinter die Tabelle verschieben
    rRTFWrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

    return rWrt;
}

Writer& OutRTF_SwSectionNode( Writer& rWrt, SwSectionNode& rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwSection& rSect = rNode.GetSection();

    // folgt dahinter noch ein SectionNode? Dann wird erst die innere
    // Section aktiv. Hier wird die Verschachtelung aufgebrochen, weil
    // RTF das nicht kennt
    BOOL bPgDscWrite = FALSE;
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
            rWrt.Strm() << sRTF_SECT << sRTF_SBKNONE;
        //JP 19.03.99 - es muss fuer den Import auf jedenfall das Cols
        //              Token geschrieben werden. Sonst kann nicht erkannt
        //              werden, wann ein PageDesc & eine Section gueltig ist
        rWrt.Strm() << sRTF_COLS << '1';
        rRTFWrt.bOutFmtAttr = TRUE;
        const SfxPoolItem* pItem;
        const SfxItemSet& rSet = rSect.GetFmt()->GetAttrSet();
        if( SFX_ITEM_SET == rSet.GetItemState( RES_COL, FALSE, &pItem ))
            OutRTF_SwFmtCol( rWrt, *pItem );
        else
            rWrt.Strm() << sRTF_COLS << '1' << sRTF_COLSX << '709';

        if( SFX_ITEM_SET == rSet.GetItemState( RES_COLUMNBALANCE,
            FALSE, &pItem ) && ((SwFmtNoBalancedColumns*)pItem)->GetValue() )
            OutComment( rWrt, sRTF_BALANCEDCOLUMN ) << '}';

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
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_F;
        rWrt.OutULong( rRTFWrt.GetId( (const SvxFontItem&)rHt ) );
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
        const FontItalic nPosture = ((const SvxPostureItem&)rHt).GetPosture();
        int bTxtOut = rRTFWrt.bTxtAttr && ITALIC_NONE == nPosture;
        if( ITALIC_NORMAL == nPosture || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_I;
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
        const FontWeight nBold = ((const SvxWeightItem&)rHt).GetWeight();
        int bTxtOut = rRTFWrt.bTxtAttr && WEIGHT_NORMAL == nBold;
        if( WEIGHT_BOLD == nBold || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_B;
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
    case EMPHASISMARK_NONE:         pStr = sRTF_ACCNONE;    break;
    case EMPHASISMARK_SIDE_DOTS:    pStr = sRTF_ACCCOMMA;   break;
    default:                        pStr = sRTF_ACCDOT;     break;
    }

    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << pStr;
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
        rWrt.Strm() << sRTF_SHAD;
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
        rWrt.Strm() << sRTF_OUTL;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwKerning( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


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
            rWrt.Strm() << sRTF_STRIKED;
            if( !bTxtOut )
                rWrt.Strm() << '1';
        }
        else
            rWrt.Strm() << sRTF_STRIKE;
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
        rWrt.Strm() << sRTF_SCAPS;
        break;

    case SVX_CASEMAP_VERSALIEN:
        rWrt.Strm() << sRTF_CAPS;
        break;

    case SVX_CASEMAP_NOT_MAPPED:
        rWrt.Strm() << sRTF_CAPS << '0' << sRTF_SCAPS << '0';       // beide aus !!
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
    switch( ((const SvxUnderlineItem&)rHt).GetUnderline() )
    {
    case UNDERLINE_SINGLE:          pStr = sRTF_UL;         break;
    case UNDERLINE_DOUBLE:          pStr = sRTF_ULDB;       break;
    case UNDERLINE_NONE:            pStr = sRTF_ULNONE;     break;
    case UNDERLINE_DOTTED:          pStr = sRTF_ULD;        break;
    case UNDERLINE_DASH:            pStr = sRTF_ULDASH;     break;
    case UNDERLINE_DASHDOT:         pStr = sRTF_ULDASHD;    break;
    case UNDERLINE_DASHDOTDOT:      pStr = sRTF_ULDASHDD;   break;
    case UNDERLINE_BOLD:            pStr = sRTF_ULTH;       break;
    case UNDERLINE_WAVE:            pStr = sRTF_ULWAVE;     break;
    case UNDERLINE_BOLDDOTTED:      pStr = sRTF_ULTHD;      break;
    case UNDERLINE_BOLDDASH:        pStr = sRTF_ULTHDASH;   break;
    case UNDERLINE_LONGDASH:        pStr = sRTF_ULLDASH;    break;
    case UNDERLINE_BOLDLONGDASH:    pStr = sRTF_ULTHLDASH;  break;
    case UNDERLINE_BOLDDASHDOT:     pStr = sRTF_ULTHDASHD;  break;
    case UNDERLINE_BOLDDASHDOTDOT:  pStr = sRTF_ULTHDASHDD; break;
    case UNDERLINE_BOLDWAVE:        pStr = sRTF_ULHWAVE;    break;
    case UNDERLINE_DOUBLEWAVE:      pStr = sRTF_ULULDBWAVE; break;
    }

    if( pStr )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        if( UNDERLINE_SINGLE == ((const SvxUnderlineItem&)rHt).GetUnderline()
            && ((SvxWordLineModeItem&)rRTFWrt.GetItem(
                RES_CHRATR_WORDLINEMODE )).GetValue() )
            pStr = sRTF_ULW;

        rRTFWrt.Strm() << pStr;
        rRTFWrt.bOutFmtAttr = TRUE;
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
        const sal_Char* p = RES_CHRATR_CJK_LANGUAGE ? sRTF_LANGFE : sRTF_LANG;
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
        pUpDn = sRTF_UP;
    else if( 0 > rEsc.GetEsc() )
    {
        pUpDn = sRTF_DN;
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

    OutComment( rWrt, sRTF_UPDNPROP, TRUE );
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
        rRTFWrt.bOutFmtAttr = TRUE;

        rWrt.Strm() << sRTF_FS;
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
    rWrt.Strm() << sRTF_CF;
    rWrt.OutULong( rRTFWrt.GetId( ((const SvxColorItem&)rHt).GetValue() ));
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
    ((((aFldStt += sRTF_FIELD) += '{' ) += sRTF_IGNORE ) += sRTF_FLDINST) += ' ';
    switch( pFld->GetTyp()->Which() )
    {
    case RES_DBFLD:
            aFldStt += "MERGEFIELD ";
            // kein break !!
    case RES_USERFLD:
            rWrt.Strm() << aFldStt.GetBuffer();
            RTFOutFuncs::Out_String( rWrt.Strm(), pFld->GetTyp()->GetName(),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
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
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\data ";
            RTFOutFuncs::Out_String( rWrt.Strm(), rWrt.pDoc->GetDBName(),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
            break;
    case RES_AUTHORFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\author ";
            break;

    case RES_HIDDENTXTFLD:
            if( TYP_CONDTXTFLD == ((SwHiddenTxtField*)pFld)->GetSubType() )
                RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
            else
            {
                rWrt.Strm() << '{' << sRTF_V << ' ';
                OutRTF_AsByteString( rWrt, pFld->GetPar2() ).Strm()
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
            RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
        }
        break;
    }

    if( aFldStt.Len() )
    {
        rWrt.Strm() << "}{" << sRTF_FLDRSLT << ' ';
        RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
        rWrt.Strm() << "}}";
        rRTFWrt.bOutFmtAttr = FALSE;
    }

    return rWrt;
}

static Writer& OutRTF_SwFlyCntnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFrmFmt* pFmt = ((SwFmtFlyCnt&)rHt).GetFrmFmt();
    if( RES_DRAWFRMFMT != pFmt->Which() )
    {
        OutComment( rWrt, sRTF_FLYINPARA );
        ((SwRTFWriter&)rWrt).OutRTFFlyFrms( *((SwFlyFrmFmt*)pFmt) );
        rWrt.Strm() << '}';
        ((SwRTFWriter&)rWrt).bOutFmtAttr = FALSE;
    }
// Zeichengebundene Zeichenobjekte werden ignoriert.
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwTOXMark( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

#ifdef USED
static Writer& OutRTF_SwNoHyphen( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


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

        rWrt.Strm() << '{' << sRTF_SUPER << ' ';
        if( !rFtn.GetNumStr().Len() )
        {
            rWrt.Strm() << sRTF_CHFTN;
            OutComment( rWrt, sRTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << sRTF_FTNALT;
            rWrt.Strm() << ' ' << sRTF_CHFTN;
        }
        else
        {
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr() );
            OutComment( rWrt, sRTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << sRTF_FTNALT;
            rWrt.Strm() << ' ';
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr() );
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

static Writer& OutRTF_SwHardBlank( Writer& rWrt, const SfxPoolItem& rHt)
{
    RTFOutFuncs::Out_Char( rWrt.Strm(), ((SwFmtHardBlank&)rHt).GetChar(),
                        DEF_ENCODING, ((SwRTFWriter&)rWrt).bWriteHelpFmt );
    return rWrt;
}

#ifdef USED
static Writer& OutRTF_SwRefMark( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


#ifdef USED
static Writer& OutRTF_SwNoLinebreakHere( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

static Writer& OutRTF_SwTxtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtINetFmt& rURL = (const SwFmtINetFmt&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rURL.GetValue().Len() )
    {
        rWrt.Strm() << '{' << sRTF_FIELD << '{' << sRTF_IGNORE
                    << sRTF_FLDINST << " HYPERLINK ";

        String sURL( rURL.GetValue() );
        if( INET_MARK_TOKEN != sURL.GetChar(0) )
        {
            INetURLObject aTmp( INetURLObject::AbsToRel( sURL ) );
            sURL = aTmp.GetURLNoMark( INetURLObject::DECODE_UNAMBIGUOUS);
/*          if( INET_PROT_FILE == aTmp.GetProtocol() )
            {
                // WW97 wollen keine FILE-URL haben, sondern einen normalen
                // Dateinamen. Aber ab WW2000 kennen sie FileURLs.
                sURL = aTmp.GetFull();
            }
*/          rWrt.Strm() << '\"';
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, DEF_ENCODING,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
            sURL = aTmp.GetMark();
        }

        if( sURL.Len() )
        {
            rWrt.Strm() << "\\\\l \"";
            sURL.Erase( 0, 1 );
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, DEF_ENCODING,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        if( rURL.GetTargetFrame().Len() )
        {
            rWrt.Strm() << "\\\\t \"";
            RTFOutFuncs::Out_String( rWrt.Strm(), rURL.GetTargetFrame(),
                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        rWrt.Strm() << "}{" << sRTF_FLDRSLT << ' ';
        rRTFWrt.bOutFmtAttr = FALSE;

        // und dann noch die Attributierung ausgeben
        const SwCharFmt* pFmt;
        const SwTxtINetFmt* pTxtAtr = rURL.GetTxtINetFmt();
        if( pTxtAtr && 0 != ( pFmt = pTxtAtr->GetCharFmt() ))
            OutRTF_SwFmt( rWrt, *pFmt );
    }
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
            rWrt.Strm() << sRTF_ABSW;
            rWrt.OutLong( rSz.GetWidth() );
            rRTFWrt.bOutFmtAttr = TRUE;
        }

        if( rSz.GetHeight() )
        {
            long nH = rSz.GetHeight();
            if( ATT_FIX_SIZE == rSz.GetSizeType() )
                nH = -nH;
            rWrt.Strm() << sRTF_ABSH;
            rWrt.OutLong( nH );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
    else if( rRTFWrt.bOutPageDesc )
    {
        rWrt.Strm() << sRTF_PGWSXN;
        rWrt.OutLong( rSz.GetWidth() );
        rWrt.Strm() << sRTF_PGHSXN;
        rWrt.OutLong( rSz.GetHeight() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwFillOrder( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


#ifdef USED
static Writer& OutRTF_SwFmtPaperBin( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif



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
                rWrt.Strm() << sRTF_MARGLSXN;
                rWrt.OutLong( rLR.GetLeft() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
            if( rLR.GetRight() )
            {
                rWrt.Strm() << sRTF_MARGRSXN;
                rWrt.OutLong( rLR.GetRight() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
        }
        else
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_LI;
            rWrt.OutLong( rLR.GetTxtLeft() ) << sRTF_RI;
            rWrt.OutLong( rLR.GetRight() );
            rWrt.Strm() << sRTF_FI;
            rWrt.OutLong( rLR.GetTxtFirstLineOfst() );
        }
    }
    else if( rLR.GetLeft() == rLR.GetRight() && rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_DFRMTXTX;
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
            rWrt.Strm() << sRTF_DFRMTXTY;
            rWrt.OutLong( rUL.GetLower() );
        }
    }
    else
    {
        const char* p;
        USHORT nValue = rUL.GetUpper();
        if( rRTFWrt.bOutPageDesc )
        {
            p = sRTF_MARGTSXN;
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
                        nValue += aRect.Height();
                    else
                    {
                        const SwFmtFrmSize& rSz = pHdr->GetHeaderFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetSizeType() )
                            nValue += rSz.GetHeight();
                        else
                            nValue += 274;      // defaulten fuer 12pt Schrift
                        nValue += pHdr->GetHeaderFmt()->GetULSpace().GetLower();
                    }
                }
            }
        }
        else
            p = sRTF_SB;

        if( rRTFWrt.bOutPageDesc || nValue )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << p;
            rWrt.OutLong( nValue );
        }


        nValue = rUL.GetLower();
        if( rRTFWrt.bOutPageDesc )
        {
            p = sRTF_MARGBSXN;
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
                        nValue += aRect.Height();
                    else
                    {
                        const SwFmtFrmSize& rSz = pFtr->GetFooterFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetSizeType() )
                            nValue += rSz.GetHeight();
                        else
                            nValue += 274;      // defaulten fuer 12pt Schrift
                        nValue += pFtr->GetFooterFmt()->GetULSpace().GetUpper();
                    }
                }
            }
        }
        else
            p = sRTF_SA;

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

        const sal_Char * pHdNm = sRTF_HEADER;
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
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetSizeType()
                            ? -rSz.GetHeight()
                            : rSz.GetHeight() ) << '}';
        }
        else
            rWrt.OutLong( rRTFWrt.pAktPageDesc->GetMaster().
                            GetULSpace().GetUpper() );

        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.
        sal_Char cTyp = 0;
        if( !rRTFWrt.bOutPageDesc && rRTFWrt.pAktPageDesc->GetFollow() &&
            rRTFWrt.pAktPageDesc->GetFollow() != rRTFWrt.pAktPageDesc )
            cTyp = 'f';     // dann FirstPage-Header
        else if( !rRTFWrt.pAktPageDesc->IsHeaderShared() )
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';

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

        const sal_Char * pFtNm = sRTF_FOOTER;
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
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetSizeType()
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
            cTyp = 'f';     // dann FirstPage-Header
        else if( !rRTFWrt.pAktPageDesc->IsFooterShared() )
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';

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
        rWrt.Strm() << sRTF_FLYPRINT;
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtOpaque( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax && !((const SvxOpaqueItem&)rHt).GetValue() )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYOPAQUE;
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
        rWrt.Strm() << sRTF_FLYPRTCTD;
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
            rWrt.Strm() << sRTF_NOWRAP;
        }
    }
    else
    {
        SwSurround eSurround = rFlySurround.GetSurround();
        BOOL bGold = SURROUND_IDEAL == eSurround;
        if( bGold )
            eSurround = SURROUND_PARALLEL;
        RTFSurround aMC( bGold, eSurround );
        rWrt.Strm() << sRTF_FLYMAINCNT;
        rWrt.OutULong( aMC.GetValue() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtVertOrient ( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;
    RTFVertOrient aVO( rFlyVert.GetVertOrient(), rFlyVert.GetRelationOrient() );
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pOrient;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        SwRelationOrient eOrient = rFlyVert.GetRelationOrient();
        if( FLY_PAGE == eAnchor )
        {
            if( REL_PG_FRAME == eOrient || FRAME == eOrient )
                pOrient = sRTF_PVPG;
            else
                pOrient = sRTF_PVMRG;
        }
        else
            pOrient = sRTF_PVPARA;
        rWrt.Strm() << pOrient;

        switch( rFlyVert.GetVertOrient() )
        {
        case VERT_TOP:
        case VERT_LINE_TOP:     rWrt.Strm() << sRTF_POSYT;  break;
        case VERT_BOTTOM:
        case VERT_LINE_BOTTOM:  rWrt.Strm() << sRTF_POSYB;  break;
        case VERT_CENTER:
        case VERT_LINE_CENTER:  rWrt.Strm() << sRTF_POSYC;  break;
        case VERT_NONE:
            {
                rWrt.Strm() << sRTF_POSY;
                rWrt.OutULong( rFlyVert.GetPos() );
            }
            break;
        }
    }
    else if( !rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYVERT;
        rWrt.OutULong( aVO.GetValue() );
    }

    return rWrt;
}

static Writer& OutRTF_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtHoriOrient& rFlyHori = (const SwFmtHoriOrient&) rHt;
    RTFHoriOrient aHO( rFlyHori.GetHoriOrient(),
                          rFlyHori.GetRelationOrient() );

    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pS;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        SwRelationOrient eOrient = rFlyHori.GetRelationOrient();
        if( FLY_PAGE == eAnchor )
        {
            if( REL_PG_FRAME == eOrient || FRAME == eOrient )
                pS = sRTF_PHPG;
            else
                pS = sRTF_PHMRG;
        }
        else
            pS = sRTF_PHCOL;
        rWrt.Strm() << pS;

        pS = 0;
        switch( rFlyHori.GetHoriOrient() )
        {
        case HORI_RIGHT:        pS = rFlyHori.IsPosToggle()
                                    ? sRTF_POSXO
                                    : sRTF_POSXR;
                                break;
        case HORI_LEFT:         pS = rFlyHori.IsPosToggle()
                                    ? sRTF_POSXI
                                    : sRTF_POSXL;
                                break;
        case HORI_CENTER:       pS = sRTF_POSXC;    break;
        case HORI_NONE:
                {
                    rWrt.Strm() << sRTF_POSX;
                    rWrt.OutULong( rFlyHori.GetPos() );
                }
                break;
        }
        if( pS )
            rWrt.Strm() << pS;
    }
    else
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm()  << sRTF_FLYHORZ;
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
        USHORT nId = rAnchor.GetAnchorId();
        rWrt.Strm() << sRTF_FLYANCHOR;
        rWrt.OutULong( nId );
        rRTFWrt.bOutFmtAttr = TRUE;
        switch( nId )
        {
        case FLY_PAGE:
                rWrt.Strm() << sRTF_FLYPAGE;
                rWrt.OutULong( rAnchor.GetPageNum() );
            break;
        case FLY_AT_CNTNT:
        case FLY_IN_CNTNT:
            rWrt.Strm() << sRTF_FLYCNTNT;
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
            ByteString sOut( sRTF_CBPAT );
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
    OutComment( rWrt, sRTF_SHADOW, FALSE );
    rWrt.OutULong( rShadow.GetLocation() ) << sRTF_SHDWDIST;
    rWrt.OutULong( rShadow.GetWidth() ) << sRTF_SHDWSTYLE;
    const Color& rColor = rShadow.GetColor();
    rWrt.OutULong( rColor.GetTransparency() ? SW_SV_BRUSH_NULL : SW_SV_BRUSH_SOLID );
    rWrt.OutULong( rRTFWrt.GetId( rColor ) ) << sRTF_SHDWFCOL;
    rWrt.OutULong( 0 ) << '}';
    return rWrt;
}


static void OutBorderLine( SwRTFWriter& rWrt, const SvxBorderLine* pLine,
                            const char* pStr )
{
    rWrt.Strm() << pStr << sRTF_BRDLNCOL;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) ) << sRTF_BRDLNIN;
    rWrt.OutULong( pLine->GetInWidth() ) << sRTF_BRDLNOUT;
    rWrt.OutULong( pLine->GetOutWidth() ) << sRTF_BRDLNDIST;
    rWrt.OutULong( pLine->GetDistance() );
}


static Writer& OutRTF_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    const SvxBoxItem& rBox = (const SvxBoxItem&)rHt;

    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    static const sal_Char* __READONLY_DATA aBorderNames[] = {
            sRTF_BRDRT, sRTF_BRDRL, sRTF_BRDRB, sRTF_BRDRR };

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
            OutTBLBorderLine( rRTFWrt, rBox.GetTop(), sRTF_BOX, nDist );
        else
        {
            OUT_BRDLINE( rBox.GetTop(),     sRTF_BRDRT, nDist );
            OUT_BRDLINE( rBox.GetBottom(),  sRTF_BRDRB, nDist );
            OUT_BRDLINE( rBox.GetLeft(),    sRTF_BRDRL, nDist );
            OUT_BRDLINE( rBox.GetRight(),   sRTF_BRDRR, nDist );
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
            OutTBLBorderLine( rRTFWrt, rBox.GetTop(), sRTF_BOX, nDist );
        else
        {
            const USHORT* pBrd = aBorders;
            const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
            for( int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms )
            {
                const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
                if( pLn )
                    OutTBLBorderLine( rRTFWrt, pLn, *pBrdNms,
                                        rBox.GetDistance( *pBrd ) );
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
            rWrt.Strm() << '{' << sRTF_IGNORE;
            OutBorderLine( rRTFWrt, pLn, *pBrdNms );
            rWrt.Strm() << '}' << sRTF_BRSP;
            rWrt.OutULong( rBox.GetDistance( *pBrd ));
        }
    }

    rRTFWrt.bOutFmtAttr = FALSE;
    return rWrt;
}


#ifdef USED
static Writer& OutRTF_SwFmtFrmMacro( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

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
            pFmt = &rWrt.pDoc->GetPageDesc(0).GetMaster();

        const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();

        USHORT nPageSize = pFmt->GetFrmSize().GetWidth() -
                            rLR.GetLeft() - rLR.GetRight();

        rWrt.Strm() << sRTF_COLS;
        rWrt.OutLong( nCols );

        if( rCol.IsOrtho() )
        {
            rWrt.Strm() << sRTF_COLSX;
            rWrt.OutLong( rCol.GetGutterWidth( TRUE ) );
        }
        else
            for( USHORT n = 0; n < nCols; )
            {
                rWrt.Strm() << sRTF_COLNO;
                rWrt.OutLong( n+1 );

                rWrt.Strm() << sRTF_COLW;
                rWrt.OutLong( rCol.CalcPrtColWidth( n, nPageSize ) );
                if( ++n != nCols )
                {
                    rWrt.Strm() << sRTF_COLSR;
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
        rWrt.Strm() << sRTF_KEEPN;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

/* File GRFATR.HXX */

static Writer& OutRTF_SwMirrorGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwMirrorGrf & rMirror = (const SwMirrorGrf&)rHt;
    if( RES_DONT_MIRROR_GRF == rMirror.GetValue() )
        return rWrt;

    ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    rWrt.Strm() << sRTF_IGNORE << sRTF_GRFMIRROR;
    rWrt.OutULong( rMirror.GetValue() );
    return rWrt;
}

static Writer& OutRTF_SwCropGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwCropGrf & rCrop = (const SwCropGrf&)rHt;
    ByteString aStr;
    if( rCrop.GetLeft() )
        ( aStr += sRTF_PICCROPL ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetLeft() );
    if( rCrop.GetRight() )
        ( aStr += sRTF_PICCROPR ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetRight() );
    if( rCrop.GetTop() )
        ( aStr += sRTF_PICCROPT ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetTop() );
    if( rCrop.GetBottom() )
        ( aStr += sRTF_PICCROPB ) +=
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

    switch( rLs.GetLineSpaceRule() )
    {
    case SVX_LINE_SPACE_AUTO:
    case SVX_LINE_SPACE_FIX:
    case SVX_LINE_SPACE_MIN:
        {
            ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_SL;
            sal_Char cMult = '0';
            switch( rLs.GetInterLineSpaceRule() )
            {
            case SVX_INTER_LINE_SPACE_FIX:      // unser Durchschuss
                // gibt es aber nicht in WW - also wie kommt man an
                // die MaxLineHeight heran?
                rWrt.OutLong( (short)rLs.GetInterLineSpace() );
                break;

            case SVX_INTER_LINE_SPACE_PROP:
                rWrt.OutLong( (240L * rLs.GetPropLineSpace()) / 100L );
                cMult = '1';
                break;

            default:
                if( SVX_LINE_SPACE_FIX == rLs.GetLineSpaceRule() )
                    rWrt.Strm() << '-';

                rWrt.OutLong( rLs.GetLineHeight() );
                break;
            }
            rWrt.Strm() << sRTF_SLMULT << cMult;
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
        rWrt.Strm() << sRTF_KEEP;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}


#ifdef USED
static Writer& OutRTF_SwWidows( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

#ifdef USED
static Writer& OutRTF_SwOrphans( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif



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

            case '.':   pFill = sRTF_TLDOT; break;
            case '_':   pFill = sRTF_TLUL;  break;
            case '-':   pFill = sRTF_TLTH;  break;
            case '=':   pFill = sRTF_TLEQ;  break;
            default:
                if( !rRTFWrt.bWriteHelpFmt )
                {
                    OutComment( rWrt, sRTF_TLSWG, FALSE );
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
                OutComment( rWrt, sRTF_TLSWG, FALSE );
                rWrt.OutULong( (((USHORT)rTS.GetFill()) << 8 ) +
                                rTS.GetDecimal() ) << '}';
            }

            const sal_Char* pAdjStr = 0;
            switch( rTS.GetAdjustment() )
            {
            case SVX_TAB_ADJUST_RIGHT:      pAdjStr = sRTF_TQR;     break;
            case SVX_TAB_ADJUST_DECIMAL:    pAdjStr = sRTF_TQDEC;   break;
            case SVX_TAB_ADJUST_CENTER:     pAdjStr = sRTF_TQC;     break;
            }
            if( pAdjStr )
                rWrt.Strm() << pAdjStr;
            rWrt.Strm() << sRTF_TX;
            rWrt.OutLong( rTS.GetTabPos() + nOffset );
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

        OutComment( rWrt, sRTF_HYPHEN, FALSE );
        rWrt.OutULong( nFlags ) << sRTF_HYPHLEAD;
        rWrt.OutULong( rAttr.GetMinLead() ) << sRTF_HYPHTRAIL;
        rWrt.OutULong( rAttr.GetMinTrail() ) << sRTF_HYPHMAX;
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
            USHRT_MAX != (nId = rRTFWrt.GetId( rAttr ) ) )
        {
            rWrt.Strm() << sRTF_LS;
            rWrt.OutULong( nId );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
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
/* RES_CHRATR_BACKGROUND */         0, // Neu: Zeichenhintergrund
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
/* RES_CHRATR_WRITING_DIRECTION */  0,
/* RES_CHRATR_EMPHASIS_MARK */      OutRTF_SwEmphasisMark,
/* RES_CHRATR_DUMMY3 */             0,
/* RES_CHRATR_DUMMY4 */             0,
/* RES_CHRATR_DUMMY5 */             0,
/* RES_CHRATR_DUMMY1 */             0, // Dummy:

/* RES_TXTATR_INETFMT   */          OutRTF_SwTxtINetFmt, // Dummy
/* RES_TXTATR_NOHYPHEN  */          0, // Dummy
/* RES_TXTATR_REFMARK*/             0, // NOT USED!! OutRTF_SwRefMark,
/* RES_TXTATR_TOXMARK */            0, // NOT USED!! OutRTF_SwTOXMark,
/* RES_TXTATR_CHARFMT   */          OutRTF_SwTxtCharFmt,
/* RES_TXTATR_TWO_LINES */          0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,
/* RES_TXTATR_DUMMY6 */             0,

/* RES_TXTATR_FIELD */              OutRTF_SwField,
/* RES_TXTATR_FLYCNT */             OutRTF_SwFlyCntnt,
/* RES_TXTATR_FTN */                OutRTF_SwFtn,
/* RES_TXTATR_SOFTHYPH */           0,  // old attr. - coded now by character
/* RES_TXTATR_HARDBLANK*/           OutRTF_SwHardBlank,
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
/* RES_PARATR_SCRIPTSPACE */        0, // Dummy:
/* RES_PARATR_HANGINGPUNCTUATION */ 0, // Dummy:
/* RES_PARATR_DUMMY1 */             0, // Dummy:
/* RES_PARATR_DUMMY2 */             0, // Dummy:
/* RES_PARATR_DUMMY3 */             0, // Dummy:
/* RES_PARATR_DUMMY4 */             0, // Dummy:
/* RES_PARATR_DUMMY5 */             0, // Dummy:
/* RES_PARATR_DUMMY6 */             0, // Dummy:
/* RES_PARATR_DUMMY7 */             0, // Dummy:
/* RES_PARATR_DUMMY8 */             0, // Dummy:

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
/* RES_FRMATR_DUMMY3 */             0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRMATR_DUMMY7 */             0, // Dummy:
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:

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

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/rtf/rtfatr.cxx,v 1.6 2000-11-20 09:37:54 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.5  2000/11/16 09:57:35  jp
      export CJK attributes

      Revision 1.4  2000/11/13 12:27:20  jp
      token definition changed

      Revision 1.3  2000/11/01 19:25:51  jp
      export of mail graphics removed

      Revision 1.2  2000/10/20 13:43:28  jp
      use correct INetURL-Decode enum

      Revision 1.1.1.1  2000/09/18 17:14:56  hr
      initial import

      Revision 1.344  2000/09/18 16:04:50  willem.vandorp
      OpenOffice header added.

      Revision 1.343  2000/08/30 16:46:03  jp
      use CharClass instead of international

      Revision 1.342  2000/08/25 12:28:03  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.341  2000/08/18 13:02:02  jp
      don't export escaped URLs

      Revision 1.340  2000/08/04 10:48:05  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character; use rtfout functions

      Revision 1.339  2000/07/31 19:24:34  jp
      new attributes for CJK/CTL and graphic

      Revision 1.338  2000/07/20 13:16:22  jp
      change old txtatr-character to the two new characters

      Revision 1.337  2000/06/26 12:53:08  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.336  2000/05/09 17:22:42  jp
      Changes for Unicode

      Revision 1.335  2000/03/24 14:10:38  jp
      Bug #74428#: LRSpace - GetLeft returns the right value for pagedescs

      Revision 1.334  2000/03/15 13:50:27  jp
      Bug #74070#: WriteTable - look for colspans

      Revision 1.333  2000/03/03 15:21:02  os
      StarView remainders removed

      Revision 1.332  2000/02/24 18:33:21  jp
      Bug #73098#: read & write list entries without number

      Revision 1.331  2000/02/11 14:37:51  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.330  2000/01/25 20:11:50  jp
      Bug #72146#: OutSwChar - write the original character code if it can not be converted

      Revision 1.329  2000/01/07 12:26:49  jp
      Task #71411#: read/write NoBalancedColumns

      Revision 1.328  1999/12/08 13:51:01  jp
      Task #70258#: textattr for XMLAttrContainer

*************************************************************************/


