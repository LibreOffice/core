/*************************************************************************
 *
 *  $RCSfile: wrtww8.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-14 15:54:34 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include <string.h>             // memcpy()

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _MSOLEEXP_HXX
#include <svx/msoleexp.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _WW8STRUC_HXX
#include <ww8struc.hxx>
#endif
#ifndef _WRT_FN_HXX
#include <wrt_fn.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>          // RES_POOLCOLL_STANDARD
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif

#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif

class WW8_WrFkp
{
    BYTE* pFkp;         // gesamter Fkp ( zuerst nur FCs und Sprms )
    BYTE* pOfs;         // Pointer auf Offset-Bereich, spaeter nach pFkp kopiert
    ePLCFT ePlc;
    short nStartGrp;    // ab hier grpprls
    short nOldStartGrp;
    BYTE nItemSize;
    BYTE nIMax;         // Anzahl der Eintrags-Paare
    BYTE nOldVarLen;
    BOOL bCombined;     // TRUE : Einfuegen verboten

    BYTE SearchSameSprm( USHORT nVarLen, const BYTE* pSprms );
public:
    WW8_WrFkp( ePLCFT ePl, WW8_FC nStartFc, BOOL bWrtWW8 );
    ~WW8_WrFkp();
    BOOL Append( WW8_FC nEndFc, USHORT nVarLen = 0, const BYTE* pSprms = 0 );
    BOOL Combine();
    void Write( SvStream& rStrm, SwWW8WrGrf& rGrf );

    BOOL IsEqualPos( WW8_FC nEndFc ) const
    {   return !bCombined && nIMax && nEndFc == ((INT32*)pFkp)[nIMax]; }
    void MergeToNew( short& rVarLen, BYTE *& pNewSprms );
    BOOL IsEmptySprm() const
    {   return !bCombined && nIMax && !nOldVarLen;  }
    void SetNewEnd( WW8_FC nEnd )
    {   ((INT32*)pFkp)[nIMax] = nEnd; }

#ifdef __WW8_NEEDS_COPY
    WW8_FC GetStartFc() const;
    WW8_FC GetEndFc() const;
#else
    WW8_FC GetStartFc() const { return ((INT32*)pFkp)[0]; };
    WW8_FC GetEndFc() const { return ((INT32*)pFkp)[nIMax]; };
#endif // defined __WW8_NEEDS_COPY
};


// -------------------------------------------------------------------------
// class WW8_WrPc sammelt alle Piece-Eintraege fuer ein Piece
// -------------------------------------------------------------------------

class WW8_WrPc
{
    WW8_CP nStartCp;                    // Start ZeichenPosition vom Text
    WW8_FC nStartFc;                    // Start File Position vom Text
    USHORT nStatus;                     // Absatzende im Piece ?

public:
    WW8_WrPc(WW8_FC nSFc, WW8_CP nSCp )
        : nStartCp( nSCp ), nStartFc( nSFc ), nStatus( 0x0040 )
    {}

    void SetStatus()                { nStatus = 0x0050; }
    USHORT GetStatus()  const       { return nStatus; }
    WW8_CP GetStartCp() const       { return nStartCp; }
    WW8_FC GetStartFc() const       { return nStartFc; }
};


class WW8_WrtBookmarks
{
    SvULongs aSttCps, aEndCps;              // Array of Start- and End CPs
    SvStringsDtor aSwBkmkNms;               // Array of Sw - Bookmarknames
//  SvStringsDtor aWWBkmkNms;               // Array of WW - Bookmarknames

    USHORT GetPos( const String& rNm );
public:
    WW8_WrtBookmarks();
    ~WW8_WrtBookmarks();

    void Append( WW8_CP nStartCp, const String& rNm );
    void Write( SwWW8Writer& rWrt );

//  String GetWWBkmkName( const String& rName ) const;
};

class WW8_WrtRedlineAuthor
{
    SvStringsDtor aAuthors;             // Array of Sw - Bookmarknames

    USHORT GetPos( const String& rNm );
public:
    WW8_WrtRedlineAuthor() : aAuthors( 0, 4 ) {}

    USHORT AddName( const String& rNm );
    void Write( SwWW8Writer& rWrt );
};


#define ANZ_DEFAULT_STYLES 16

// die Namen der StorageStreams
#define sMainStream         String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "WordDocument" ))
#define sDataStream         String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "Data" ))
#define sTableStream        String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "1Table" ))


SV_IMPL_VARARR( WW8Bytes, BYTE )
SV_IMPL_PTRARR( WW8_WrFkpPtrs, WW8_FkpPtr )

typedef WW8_WrPc* WW8_WrPcPtr;
SV_DECL_PTRARR_DEL( WW8_WrPcPtrs, WW8_WrPcPtr, 4, 4 )
SV_IMPL_PTRARR( WW8_WrPcPtrs, WW8_WrPcPtr )

static void WriteDop( SwWW8Writer& rWrt )
{
    WW8Dop& rDop = *rWrt.pDop;

    // default TabStop schreiben
    const SvxTabStopItem& rTabStop = (SvxTabStopItem& )rWrt.pDoc->
                    GetAttrPool().GetDefaultItem( RES_PARATR_TABSTOP );
    rDop.dxaTab = (USHORT)rTabStop[0].GetTabPos();


    // Werte aus der DocStatistik (werden aufjedenfall fuer die
    // DocStat-Felder benoetigt!)
    rDop.fWCFtnEdn = TRUE; // because they are included in StarWriter

    const SwDocStat& rDStat = rWrt.pDoc->GetDocStat();
    rDop.cWords = rDStat.nWord;
    rDop.cCh    = rDStat.nChar;
    rDop.cPg    = (INT16)rDStat.nPage;
    rDop.cParas = rDStat.nPara;
    rDop.cLines = rDStat.nPara;

//  auch damit werden die DocStat-Felder in Kopf-/Fusszeilen nicht korrekt
//  berechnet.
//  ( we do not have this fields! )

    // und noch fuer die Header und Footers
    rDop.cWordsFtnEnd   = rDStat.nWord;
    rDop.cChFtnEdn      = rDStat.nChar;
    rDop.cPgFtnEdn      = (INT16)rDStat.nPage;
    rDop.cParasFtnEdn   = rDStat.nPara;
    rDop.cLinesFtnEdn   = rDStat.nPara;

    rDop.Write( *rWrt.pTableStrm, *rWrt.pFib );
}

static sal_Unicode __READONLY_DATA aLangNotBegin[4][101] = {
//Japanese Level 1
{
0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d,
0x00a2, 0x00b0, 0x2019, 0x201d, 0x2030, 0x2032, 0x2033, 0x2103, 0x3001, 0x3002,
0x3005, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x3041, 0x3043, 0x3045,
0x3047, 0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308e, 0x309b, 0x309c, 0x309d,
0x309e, 0x30a1, 0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30c3, 0x30e3, 0x30e5, 0x30e7,
0x30ee, 0x30f5, 0x30f6, 0x30fb, 0x30fc, 0x30fd, 0x30fe, 0xff01, 0xff05, 0xff09,
0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d, 0xff5d, 0xff61, 0xff63, 0xff64,
0xff65, 0xff67, 0xff68, 0xff69, 0xff6a, 0xff6b, 0xff6c, 0xff6d, 0xff6e, 0xff6f,
0xff70, 0xff9e, 0xff9f, 0xffe0

},
//Simplified Chinese
{
0x0021, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d, 0x00a8,
0x00b7, 0x02c7, 0x02c9, 0x2015, 0x2016, 0x2019, 0x201d, 0x2026, 0x2236, 0x3001,
0x3002, 0x3003, 0x3005, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x3017,
0xff01, 0xff02, 0xff07, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d,
0xff40, 0xff5c, 0xff5d, 0xff5e, 0xffe0
},
//Korean
{
0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d,
0x00a2, 0x00b0, 0x2019, 0x201d, 0x2032, 0x2033, 0x2103, 0x3009, 0x300b, 0x300d,
0x300f, 0x3011, 0x3015, 0xff01, 0xff05, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b,
0xff1f, 0xff3d, 0xff5d, 0xffe0
},
//Traditional Chinese
{
0x0021, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d, 0x00a2,
0x00b7, 0x2013, 0x2014, 0x2019, 0x201d, 0x2022, 0x2025, 0x2026, 0x2027, 0x2032,
0x2574, 0x3001, 0x3002, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x301e,
0xfe30, 0xfe31, 0xfe33, 0xfe34, 0xfe36, 0xfe38, 0xfe3a, 0xfe3c, 0xfe3e, 0xfe40,
0xfe42, 0xfe44, 0xfe4f, 0xfe50, 0xfe51, 0xfe52, 0xfe54, 0xfe55, 0xfe56, 0xfe57,
0xfe5a, 0xfe5c, 0xfe5e, 0xff01, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f,
0xff5c, 0xff5d, 0xff64
},
};

static sal_Unicode __READONLY_DATA aLangNotEnd[4][51] = {
//Japanese Level 1
{
0x0024, 0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c, 0x3008,
0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0xff04, 0xff08, 0xff3b, 0xff5b, 0xff62,
0xffe1, 0xffe5

},
//Simplified Chinese
{
0x0028, 0x005b, 0x007b, 0x00b7, 0x2018, 0x201c, 0x3008, 0x300a, 0x300c, 0x300e,
0x3010, 0x3014, 0x3016, 0xff08, 0xff0e, 0xff3b, 0xff5b, 0xffe1, 0xffe5
},
//Korean
{
0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c, 0x3008, 0x300a,
0x300c, 0x300e, 0x3010, 0x3014, 0xff04, 0xff08, 0xff3b, 0xff5b, 0xffe6
},
//Traditional Chinese
{
0x0028, 0x005b, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c, 0x2035, 0x3008, 0x300a,
0x300c, 0x300e, 0x3010, 0x3014, 0x301d, 0xfe35, 0xfe37, 0xfe39, 0xfe3b, 0xfe3d,
0xfe3f, 0xfe41, 0xfe43, 0xfe59, 0xfe5b, 0xfe5d, 0xff08, 0xff5b
},
};

sal_Unicode WW8DopTypography::aJapanNotBeginLevel1[101] =
//Japanese Level 1
{
0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d,
0x00a2, 0x00b0, 0x2019, 0x201d, 0x2030, 0x2032, 0x2033, 0x2103, 0x3001, 0x3002,
0x3005, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x309b, 0x309c, 0x309d,
0x309e, 0x30fb, 0x30fd, 0x30fe, 0xff01, 0xff05, 0xff09, 0xff0c, 0xff0e, 0xff1a,
0xff1b, 0xff1f, 0xff3d, 0xff5d, 0xff61, 0xff63, 0xff64, 0xff65, 0xff9e, 0xff9f,
0xffe0
};

sal_Unicode WW8DopTypography::aJapanNotEndLevel1[51] =
//Japanese Level 1
{
0x0024, 0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c, 0x3008,
0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0xff04, 0xff08, 0xff3b, 0xff5b, 0xff62,
0xffe1, 0xffe5
};

int lcl_CmpBeginEndChars( const rtl::OUString& rSWStr,
                          const sal_Unicode* pMSStr, int nMSStrByteLen )
{
    nMSStrByteLen /= sizeof( sal_Unicode );
    if( nMSStrByteLen > rSWStr.getLength() )
        nMSStrByteLen = rSWStr.getLength()+1;
    nMSStrByteLen *= sizeof( sal_Unicode );

    return memcmp( rSWStr.getStr(), pMSStr, nMSStrByteLen );
}

/*
Converts the OOo Asian Typography into a best fit match for Microsoft
Asian typography. This structure is actually dumped to disk within the
Dop Writer. Assumption is that rTypo is cleared to 0 on entry
*/
void SwWW8Writer::ExportDopTypography(WW8DopTypography &rTypo)
{
    const com::sun::star::i18n::ForbiddenCharacters *pForbidden = 0;
    const com::sun::star::i18n::ForbiddenCharacters *pUseMe = 0;
    BYTE nUseReserved=0;
    int nNoNeeded=0;
    /*Now we have some minor difficult issues, to wit...
    a. MicroSoft Office can only store one set of begin and end characters in
    a given document, not one per language.
    b. StarOffice has only a concept of one set of begin and end characters for
    a given language, i.e. not the two levels of kinsoku in japanese

    What is unknown as yet is if our default begin and end chars for
    japanese, chinese tradition, chinese simplified and korean are different
    in Word and Writer. I already suspect that they are different between
    different version of word itself.

    So what have come up with is to simply see if any of the four languages
    in OOo have been changed away from OUR defaults, and if one has then
    export that. If more than one has in the future we may hack in something
    which examines our document properties to see which language is used the
    most and choose that, for now we choose the first and throw an ASSERT.
    */

    /*Our default Japanese Level is 2, this is a special MS hack to set this*/
    rTypo.reserved2 = 1;

    for (rTypo.reserved1=8;rTypo.reserved1>0;rTypo.reserved1-=2)
    {
        if (pForbidden = pDoc->GetForbiddenCharacters(rTypo.GetConvertedLang(),
            FALSE))
        {
            int nIdx = (rTypo.reserved1-2)/2;
            if( lcl_CmpBeginEndChars( pForbidden->endLine,
                    aLangNotEnd[ nIdx ], sizeof(aLangNotEnd[ nIdx ]) ) ||
                lcl_CmpBeginEndChars( pForbidden->beginLine,
                    aLangNotBegin[ nIdx ], sizeof(aLangNotBegin[ nIdx ]) ) )
            {
                //One exception for Japanese, if it matches a level 1 we
                //can use one extra flag for that, rather than use a custom
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                {
                    if( !lcl_CmpBeginEndChars( pForbidden->endLine,
                            WW8DopTypography::aJapanNotEndLevel1,
                            sizeof(WW8DopTypography::aJapanNotEndLevel1 )) &&
                        !lcl_CmpBeginEndChars( pForbidden->beginLine,
                            WW8DopTypography::aJapanNotBeginLevel1,
                            sizeof(WW8DopTypography::aJapanNotBeginLevel1 )) )
                    {
                        rTypo.reserved2 = 0;
                        continue;
                    }
                }

                if (!pUseMe)
                {
                    pUseMe = pForbidden;
                    nUseReserved = rTypo.reserved1;
                    rTypo.iLevelOfKinsoku = 2;
                }
                nNoNeeded++;
            }
        }
    }

    ASSERT( nNoNeeded<=1, "Example of unexportable forbidden chars" );
    rTypo.reserved1=nUseReserved;
    if (rTypo.iLevelOfKinsoku)
    {
        rTypo.cchFollowingPunct = static_cast<INT16>
            (pUseMe->beginLine.getLength());
        if (rTypo.cchFollowingPunct > WW8DopTypography::MaxFollowing)
            rTypo.cchFollowingPunct = WW8DopTypography::MaxFollowing;

        rTypo.cchLeadingPunct = static_cast<INT16>
            (pUseMe->endLine.getLength());
        if (rTypo.cchLeadingPunct > WW8DopTypography::MaxLeading)
            rTypo.cchLeadingPunct = WW8DopTypography::MaxLeading;

        memcpy(rTypo.rgxchFPunct,pUseMe->beginLine.getStr(),
            (rTypo.cchFollowingPunct+1)*2);

        memcpy(rTypo.rgxchLPunct,pUseMe->endLine.getStr(),
            (rTypo.cchLeadingPunct+1)*2);
    }
}

// HasItem ist fuer die Zusammenfassung der Doppel-Attribute
// Underline / WordLineMode und Box / Shadow.
// Es kann nur etwas gefunden werden, wenn diese Methode innerhalb
// der aufgerufenen Methoden WW8_SwAttrIter::OutAttr() und
// SwWW8Writer::Out_SfxItemSet() benutzt wird.
const SfxPoolItem* SwWW8Writer::HasItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem;
    if( pISet )
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // ourer own Ids. So the Id have to translate from our into the
        // EditEngine Range
        if( RES_WHICHHINT_END < *pISet->GetRanges() )
        {
            USHORT nSlotId = pDoc->GetAttrPool().GetSlotId( nWhich );
            if( !nSlotId || nWhich == nSlotId ||
                0 == ( nWhich = pISet->GetPool()->GetWhich( nSlotId ) ) ||
                nWhich != nSlotId )
                nWhich = 0;
        }
        if( nWhich &&
            SFX_ITEM_SET != pISet->GetItemState( nWhich, TRUE, &pItem ) )
            pItem = 0;
    }
    else if( pChpIter )
        pItem = pChpIter->HasTextItem( nWhich );
    else
    {
        ASSERT( !this, "Wo ist mein ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return pItem;
}

const SfxPoolItem& SwWW8Writer::GetItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem;
    if( pISet )
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // ourer own Ids. So the Id have to translate from our into the
        // EditEngine Range
        if( RES_WHICHHINT_END < *pISet->GetRanges() )
        {
            USHORT nSlotId = pDoc->GetAttrPool().GetSlotId( nWhich );
            if( !nSlotId || nWhich == nSlotId ||
                0 == ( nWhich = pISet->GetPool()->GetWhich( nSlotId ) ) ||
                nWhich != nSlotId )
                nWhich = 0;
        }
        pItem = &pISet->Get( nWhich, TRUE );
    }
    else if( pChpIter )
        pItem = &pChpIter->GetItem( nWhich );
    else
    {
        ASSERT( !this, "Wo ist mein ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return *pItem;
}

//------------------------------------------------------------------------------

WW8_WrPlc1::WW8_WrPlc1( USHORT nStructSz )
    : aPos( 16, 16 ), aDat( 16 * (int)nStructSz, 16 * (int)nStructSz ),
    nStructSiz( nStructSz )
{
}

void WW8_WrPlc1::Append( WW8_CP nCp, const void* pData )
{
    aPos.Insert( nCp, aPos.Count() );
    aDat.Insert( (BYTE*)pData, nStructSiz, aDat.Count() );
}

void WW8_WrPlc1::Finish( ULONG nLastCp, ULONG nSttCp )
{
    if( aPos.Count() )
    {
        aPos.Insert( nLastCp, aPos.Count() );
        if( nSttCp )
            for( USHORT n = 0; n < aPos.Count(); ++n )
                aPos[ n ] -= nSttCp;
    }
}


void WW8_WrPlc1::Write( SvStream& rStrm )
{
    for( USHORT i = 0; i < aPos.Count(); ++i )
        SwWW8Writer::WriteLong( rStrm, aPos[i] );
    rStrm.Write( aDat.GetData(), aDat.Count() );    // Anz Eintraege
}

//------------------------------------------------------------------------------
//  Klasse WW8_WrPlcFld fuer Felder
//------------------------------------------------------------------------------


BOOL WW8_WrPlcFld::Write( SwWW8Writer& rWrt )
{
    if( WW8_WrPlc1::Count() <= 1 )
        return FALSE;

    long *pfc, *plc;
    switch( nTxtTyp )
    {
    case TXT_MAINTEXT:
        pfc = &rWrt.pFib->fcPlcffldMom;
        plc = &rWrt.pFib->lcbPlcffldMom;
        break;
    case TXT_HDFT:
        pfc = &rWrt.pFib->fcPlcffldHdr;
        plc = &rWrt.pFib->lcbPlcffldHdr;
        break;

    case TXT_FTN:
        pfc = &rWrt.pFib->fcPlcffldFtn;
        plc = &rWrt.pFib->lcbPlcffldFtn;
        break;

    case TXT_EDN:
        pfc = &rWrt.pFib->fcPlcffldEdn;
        plc = &rWrt.pFib->lcbPlcffldEdn;
        break;

    case TXT_TXTBOX:
        pfc = &rWrt.pFib->fcPlcffldTxbx;
        plc = &rWrt.pFib->lcbPlcffldTxbx;
        break;

    case TXT_HFTXTBOX:
        pfc = &rWrt.pFib->fcPlcffldHdrTxbx;
        plc = &rWrt.pFib->lcbPlcffldHdrTxbx;
        break;

    default:
        pfc = plc = 0;
        break;
    }

    if( pfc && plc )
    {
        ULONG nFcStart = rWrt.pTableStrm->Tell();
        WW8_WrPlc1::Write( *rWrt.pTableStrm );
        *pfc = nFcStart;
        *plc = rWrt.pTableStrm->Tell() - nFcStart;
    }
    return TRUE;
}


//--------------------------------------------------------------------------

void SwWW8Writer::FillCount( SvStream& rStrm, ULONG nCount )
{
    static UINT32 __READONLY_DATA aNulls[16] = {
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 64 Byte

    while( nCount > 64 )
    {
        rStrm.Write( aNulls, 64 );          // in 64-Byte-Schritten
        nCount -= 64;
    }
    rStrm.Write( aNulls, nCount );          // Rest ( 0 .. 64 Bytes ) schreiben
}


ULONG SwWW8Writer::FillUntil( SvStream& rStrm, ULONG nEndPos )
{
    ULONG nCurPos = rStrm.Tell();
    if( !nEndPos )                          // nEndPos == 0 -> next Page
        nEndPos = nCurPos + 0x1ff & ~0x1ffUL;

    if( nEndPos > nCurPos )
        SwWW8Writer::FillCount( rStrm, nEndPos - nCurPos );
#ifndef PRODUCT
    else
        ASSERT( nEndPos == nCurPos, "Falsches FillUntil()" );
#endif
    return rStrm.Tell();
}


//--------------------------------------------------------------------------
/*  */

WW8_WrPlcPn::WW8_WrPlcPn( SwWW8Writer& rWr, ePLCFT ePl, WW8_FC nStartFc )
    : rWrt( rWr ), ePlc( ePl ), nFkpStartPage( 0 )
{
    WW8_FkpPtr pF = new WW8_WrFkp( ePlc, nStartFc, rWrt.bWrtWW8 );
    aFkps.Insert( pF, aFkps.Count() );
}

WW8_WrPlcPn::~WW8_WrPlcPn()
{
    aFkps.DeleteAndDestroy( 0, aFkps.Count() );
}

void WW8_WrPlcPn::AppendFkpEntry( WW8_FC nEndFc, short nVarLen,
                                    const BYTE* pSprms )
{
    WW8_FkpPtr pF = aFkps.GetObject( aFkps.Count() - 1 );

    // big sprm? build the sprmPHugePapx
    BYTE* pNewSprms = (BYTE*)pSprms;
    BYTE aHugePapx[ 8 ];
    if( rWrt.bWrtWW8 && PAP == ePlc && 488 < nVarLen )
    {
        BYTE* p = aHugePapx;
        *p++ = *pSprms++;           // set style Id
        *p++ = *pSprms++;
        nVarLen -= 2;

        long nDataPos = rWrt.pDataStrm->Tell();
        SwWW8Writer::WriteShort( *rWrt.pDataStrm, nVarLen );
        rWrt.pDataStrm->Write( pSprms, nVarLen );

        Set_UInt16( p, 0x6646 );    // set SprmCode
        Set_UInt32( p, nDataPos );  // set startpos (FC) in the datastream
        nVarLen = p - aHugePapx;
        pSprms = pNewSprms = aHugePapx;
    }
    // if append at the same FC-EndPos and there are sprms, then get the old
    // sprms and erase it; they will append now with the new sprms
    else if( nVarLen && pF->IsEqualPos( nEndFc ))
        pF->MergeToNew( nVarLen, pNewSprms );
    // has the prev EndFC an empty sprm and the current is empty too, then
    // expand only the old EndFc to the new EndFc
    else if( !nVarLen && pF->IsEmptySprm() )
    {
        pF->SetNewEnd( nEndFc );
        return ;
    }

    BOOL bOk = pF->Append( nEndFc, nVarLen, pNewSprms );
    if( !bOk )
    {
        pF->Combine();
        pF = new WW8_WrFkp( ePlc, pF->GetEndFc(), rWrt.bWrtWW8 );// Anfang neuer Fkp
                                                    // == Ende alter Fkp
        aFkps.Insert( pF, aFkps.Count() );
        if( !pF->Append( nEndFc, nVarLen, pNewSprms ) )
        {
            ASSERT( !this, "Sprm liess sich nicht einfuegen" );
        }
    }
    if( pNewSprms != pSprms )
        delete pNewSprms;
}

void WW8_WrPlcPn::WriteFkps()
{
    nFkpStartPage = (USHORT) ( SwWW8Writer::FillUntil( rWrt.Strm() ) >> 9 );

    for( USHORT i = 0; i < aFkps.Count(); i++ )
        aFkps.GetObject( i )->Write( rWrt.Strm(), *rWrt.pGrf );

    if( CHP == ePlc )
    {
        rWrt.pFib->pnChpFirst = nFkpStartPage;
        rWrt.pFib->cpnBteChp = aFkps.Count();
    }
    else
    {
        rWrt.pFib->pnPapFirst = nFkpStartPage;
        rWrt.pFib->cpnBtePap = aFkps.Count();
    }
}

void WW8_WrPlcPn::WritePlc()
{
    ULONG nFcStart = rWrt.pTableStrm->Tell();
    USHORT i;

    for( i = 0; i < aFkps.Count(); i++ )
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps.GetObject( i )->GetStartFc() );

    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps.GetObject( i - 1 )->GetEndFc() );

    // fuer jedes FKP die Page ausgeben
    if( rWrt.bWrtWW8)                   // fuer WW97 Long-Ausgabe
        for ( i = 0; i < aFkps.Count(); i++)
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, i + nFkpStartPage );
    else                            // fuer WW95 Short-Ausgabe
        for ( i = 0; i < aFkps.Count(); i++)
            SwWW8Writer::WriteShort( *rWrt.pTableStrm, i + nFkpStartPage );

    if( CHP == ePlc )
    {
        rWrt.pFib->fcPlcfbteChpx = nFcStart;
        rWrt.pFib->lcbPlcfbteChpx = rWrt.pTableStrm->Tell() - nFcStart;
    }
    else
    {
        rWrt.pFib->fcPlcfbtePapx = nFcStart;
        rWrt.pFib->lcbPlcfbtePapx = rWrt.pTableStrm->Tell() - nFcStart;
    }
}

//--------------------------------------------------------------------------
/*  */

WW8_WrFkp::WW8_WrFkp( ePLCFT ePl, WW8_FC nStartFc, BOOL bWrtWW8 )
    : nItemSize( ( CHP == ePl ) ? 1 : ( bWrtWW8 ? 13 : 7 )),
         nIMax( 0 ), ePlc( ePl ), bCombined( FALSE ), nStartGrp( 511 ),
    nOldStartGrp( 511 ), nOldVarLen( 0 )
{
    pFkp = (BYTE*)new INT32[128];           // 512 Byte
    pOfs = (BYTE*)new INT32[128];           // 512 Byte
    memset( pFkp, 0, 4 * 128 );
    memset( pOfs, 0, 4 * 128 );
    ( (INT32*)pFkp )[0] = nStartFc;         // 0. FC-Eintrag auf nStartFc
}

WW8_WrFkp::~WW8_WrFkp()
{
    delete( pFkp );
    delete( pOfs );
}

BYTE WW8_WrFkp::SearchSameSprm( USHORT nVarLen, const BYTE* pSprms )
{
    if( 3 < nVarLen )
    {
        // if the sprms contained picture-references then never equal!
        for( BYTE n = nVarLen - 1; 3 < n; --n )
            if( pSprms[ n ] == GRF_MAGIC_3 &&
                pSprms[ n-1 ] == GRF_MAGIC_2 &&
                pSprms[ n-2 ] == GRF_MAGIC_1 )
                    return 0;
    }

    short i;
    for( i = 0; i < nIMax; i++ )
    {
        BYTE nStart = pOfs[i * nItemSize];
        if( nStart )
        {                               // Hat Sprms
            const BYTE* p = pFkp + ( (USHORT)nStart << 1 );
            if( ( CHP == ePlc
                    ? (*p++ == nVarLen)
                    : (((USHORT)*p++ << 1 ) == (( nVarLen+1) & 0xfffe)) )
                && !memcmp( p, pSprms, nVarLen ) )
                    return nStart;                      // gefunden
        }
    }
    return 0;           // nicht gefunden
}

BOOL WW8_WrFkp::Append( WW8_FC nEndFc, USHORT nVarLen, const BYTE* pSprms )
{
    ASSERT( !nVarLen || pSprms, "Item-Pointer fehlt" );
    ASSERT( nVarLen < ( ( ePlc == PAP ) ? 497U : 502U ), "Sprms zu lang !" );

    if( bCombined )
    {
        ASSERT( !this, "Fkp::Append: Fkp is already combined" );
        return FALSE;
    }
    INT32 n = ((INT32*)pFkp)[nIMax];        // letzter Eintrag
    if( nEndFc <= n )
    {
        ASSERT( nEndFc >= n, "+Fkp: FC rueckwaerts" );
        ASSERT( !nVarLen || !pSprms || nEndFc != n,
                                    "+Fkp: selber FC mehrfach benutzt" );
                        // selber FC ohne Sprm wird ohne zu mosern ignoriert.

        return TRUE;    // ignorieren, keinen neuen Fkp anlegen
    }

    BYTE nOldP = ( nVarLen ) ? SearchSameSprm( nVarLen, pSprms ) : 0;
                                            // Kombinieren gleicher Eintraege
    short nOffset, nPos = nStartGrp;
    if( nVarLen && !nOldP )
    {
        nPos = PAP == ePlc
                ? ( 13 == nItemSize     // HACK: PAP und bWrtWW8 !!
                     ? (nStartGrp & 0xFFFE ) - nVarLen - 1
                     : (nStartGrp - (((nVarLen + 1) & 0xFFFE)+1)) & 0xFFFE )
                : ((nStartGrp - nVarLen - 1) & 0xFFFE);
        if( nPos < 0 )
            return FALSE;           // Passt absolut nicht
        nOffset = nPos;             // Offset merken (kann auch ungerade sein!)
        nPos &= 0xFFFE;             // Pos fuer Sprms ( gerade Pos )
    }

    if( (USHORT)nPos <= ( nIMax + 2U ) * 4U + ( nIMax + 1U ) * nItemSize )
                                            // Passt hinter CPs und Offsets ?
        return FALSE;                       // Nein

    ((INT32*)pFkp)[nIMax + 1] = nEndFc;     // FC eintragen

    nOldVarLen = (BYTE)nVarLen;
    if( nVarLen && !nOldP )
    {               // echt eintragen
        nOldStartGrp = nStartGrp;

        nStartGrp = nPos;
        pOfs[nIMax * nItemSize] = (BYTE)( nStartGrp >> 1 );
                                            // ( DatenAnfg >> 1 ) eintragen
        BYTE nCnt = CHP == ePlc
                        ? ( nVarLen < 256 ) ? (BYTE) nVarLen : 255
                        : ( ( nVarLen + 1 ) >> 1 );

        pFkp[ nOffset ] = nCnt;                     // DatenLaenge eintragen
        memcpy( pFkp + nOffset + 1, pSprms, nVarLen );  // Sprms speichern
    }
    else
    {
        // nicht echt eintragen ( keine Sprms oder Wiederholung )
        // DatenAnfg 0 ( keine Daten ) oder Wiederholung
        pOfs[nIMax * nItemSize] = nOldP;
    }
    nIMax++;
    return TRUE;
}

BOOL WW8_WrFkp::Combine()
{
    if( bCombined )
        return FALSE;
    if( nIMax )
        memcpy( pFkp + ( nIMax + 1 ) * 4, pOfs, nIMax * nItemSize );
    DELETEZ( pOfs );
    ((BYTE*)pFkp)[511] = nIMax;
    bCombined = TRUE;

#if defined __BIGENDIAN         // Hier werden nur die FCs gedreht, die
    register USHORT i;          // Sprms muessen an anderer Stelle gedreht
                                // werden
    register UINT32* p;
    for( i = 0, p = (UINT32*)pFkp; i <= nIMax; i++, p++ )
        *p = SWAPLONG( *p );
#endif // ifdef __BIGENDIAN

    return TRUE;
}

void WW8_WrFkp::Write( SvStream& rStrm, SwWW8WrGrf& rGrf )
{
    Combine();                      // Falls noch nicht Combined

    register BYTE* p;               //  Suche Magic fuer nPicLocFc
    BYTE* pEnd = pFkp + nStartGrp;
    for( p = pFkp + 511 - 4; p >= pEnd; p-- )
    {
        if( *p != GRF_MAGIC_1 )     // Suche nach Signatur 0x12 0x34 0x56 0xXX
            continue;
        if( *(p+1) != GRF_MAGIC_2 )
            continue;
        if( *(p+2) != GRF_MAGIC_3 )
            continue;

        SVBT32 nPos;                // Signatur gefunden
        LongToSVBT32( rGrf.GetFPos(), nPos );   // FilePos der Grafik
        memcpy( p, nPos, 4 );       // Patche FilePos ueber Signatur
    }
    rStrm.Write( pFkp, 512 );
}

void WW8_WrFkp::MergeToNew( short& rVarLen, BYTE *& rpNewSprms )
{
    BYTE nStart = pOfs[ (nIMax-1) * nItemSize ];
    if( nStart )
    {   // Hat Sprms
        BYTE* p = pFkp + ( (USHORT)nStart << 1 );

        // old and new equal? Then copy only one into the new sprms
        if( nOldVarLen == rVarLen && !memcmp( p+1, rpNewSprms, nOldVarLen ))
        {
            BYTE* pNew = new BYTE[ nOldVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            rpNewSprms = pNew;
        }
        else
        {
            BYTE* pNew = new BYTE[ nOldVarLen + rVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            memcpy( pNew + nOldVarLen, rpNewSprms, rVarLen );

            rpNewSprms = pNew;
            rVarLen += nOldVarLen;
        }
        --nIMax;
        // if this sprms dont used from others, remove it
        FASTBOOL bFnd = FALSE;
        for( USHORT n = 0; n < nIMax; ++n )
            if( nStart == pOfs[ n * nItemSize ] )
            {
                bFnd = TRUE;
                break;
            }
        if( !bFnd )
        {
            nStartGrp = nOldStartGrp;
            memset( p, 0, nOldVarLen+1 );
        }
    }
}

#ifdef __WW8_NEEDS_COPY

WW8_FC WW8_WrFkp::GetStartFc() const
{
// wenn bCombined, dann ist das Array ab pFkp schon Bytemaessig auf LittleEndian
// umgedreht, d.h. zum Herausholen der Anfangs- und Endpositionen muss
// zurueckgedreht werden.
    if( bCombined )
        return SVBT32ToLong( pFkp );        // 0. Element
    return ((INT32*)pFkp)[0];
}

WW8_FC WW8_WrFkp::GetEndFc() const
{
    if( bCombined )
        return SVBT32ToLong( &(pFkp[nIMax*4]) );    // nIMax-tes SVBT32-Element
    return ((INT32*)pFkp)[nIMax];
}

#endif // defined __WW8_NEEDS_COPY



//--------------------------------------------------------------------------
// Methoden fuer Piece-Table-Verwaltung
//--------------------------------------------------------------------------

WW8_WrPct::WW8_WrPct( WW8_FC nfcMin, BOOL bSaveUniCode )
    : nOldFc( nfcMin ), bIsUni( bSaveUniCode ), pPcts( new WW8_WrPcPtrs )
{
    AppendPc( nOldFc, bIsUni );
}

WW8_WrPct::~WW8_WrPct()
{
    delete pPcts;
}

// Piece fuellen und neues Piece erzeugen
void WW8_WrPct::AppendPc( WW8_FC nStartFc, BOOL bIsUnicode )
{
    WW8_CP nStartCp = nStartFc - nOldFc;    // Textbeginn abziehen
    if ( !nStartCp )
    {
        if ( 0 != pPcts->Count() )
        {
            ASSERT( 1 == pPcts->Count(), "Leeres Piece !!");
            pPcts->DeleteAndDestroy( pPcts->Count() - 1 , 1);
        }
    }

    nOldFc = nStartFc;                      // StartFc als alten merken

    if( bIsUni )
        nStartCp >>= 1;                 // Bei Unicode Anzahl der Zeichen / 2


    if ( !bIsUnicode )
    {
        nStartFc <<= 1;                 // Adresse * 2
        nStartFc |= 0x40000000;         // Vorletztes Bit setzen fuer !Unicode
    }

    if( pPcts->Count() )
        nStartCp += pPcts->GetObject( pPcts->Count()- 1 )->GetStartCp();

    WW8_WrPcPtr pPc = new WW8_WrPc( nStartFc, nStartCp );
    pPcts->Insert( pPc, pPcts->Count() );

    bIsUni = bIsUnicode;
}


void WW8_WrPct::WritePc( SwWW8Writer& rWrt )
{
    ULONG nPctStart;
    ULONG nOldPos, nEndPos;
    USHORT i;

    nPctStart = rWrt.pTableStrm->Tell();                    // Beginn Piece-Table
    *rWrt.pTableStrm << ( char )0x02;                       // Statusbyte PCT
    nOldPos = nPctStart + 1;                                // Position merken
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );          // Laenge folgt
    for( i = 0; i < pPcts->Count(); ++i )                   // Bereiche
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                pPcts->GetObject( i )->GetStartCp() );


    // die letzte Pos noch errechnen
    ULONG nStartCp = rWrt.pFib->fcMac - nOldFc;
    if( bIsUni )
        nStartCp >>= 1;             // Bei Unicode Anzahl der Zeichen / 2
    nStartCp += pPcts->GetObject( i-1 )->GetStartCp();
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nStartCp );

    // Pieceverweise
    for ( i = 0; i < pPcts->Count(); ++i )
    {
        WW8_WrPcPtr pPc = pPcts->GetObject( i );

        SwWW8Writer::WriteShort( *rWrt.pTableStrm, pPc->GetStatus());
        SwWW8Writer::WriteLong( *rWrt.pTableStrm, pPc->GetStartFc());
        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0);          // PRM=0
    }

    // Eintraege im FIB
    rWrt.pFib->fcClx = nPctStart;
    nEndPos = rWrt.pTableStrm->Tell();
    rWrt.pFib->lcbClx = nEndPos - nPctStart;

    // und noch die Laenge eintragen
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nOldPos,
                            nEndPos - nPctStart-5 );

}

void WW8_WrPct::SetParaBreak()
{
    ASSERT( pPcts->Count(),"SetParaBreak : aPcts.Count = 0" );
    pPcts->GetObject( pPcts->Count() - 1)->SetStatus();
}

ULONG WW8_WrPct::Fc2Cp( ULONG nFc ) const
{
    ASSERT( nFc >= (ULONG)nOldFc, "FilePos liegt vorm letzten Piece" );
    ASSERT( pPcts->Count(), "Fc2Cp noch kein Piece vorhanden" );

    nFc -= nOldFc;
    if( bIsUni )
        nFc /= 2;
    return nFc + pPcts->GetObject( pPcts->Count() - 1 )->GetStartCp();
}

//--------------------------------------------------------------------------
/*  */

WW8_WrtBookmarks::WW8_WrtBookmarks()
    : aSttCps( 0, 16 ), aEndCps( 0, 16 ),
    aSwBkmkNms( 0, 16 )
// JP 24.06.99: not used at time    ,aWWBkmkNms( 0, 16 ),
{
}

WW8_WrtBookmarks::~WW8_WrtBookmarks()
{
}

void WW8_WrtBookmarks::Append( WW8_CP nStartCp, const String& rNm )
{
    USHORT nPos = GetPos( rNm );
    if( USHRT_MAX == nPos )
    {
        // new -> insert as start position
        nPos = aSttCps.Count();
        // sort by startposition
        //      theory: write continuous -> then the new position is at end
        while( nPos && aSttCps[ nPos - 1 ] > ULONG( nStartCp ))
            --nPos;

        aSttCps.Insert( nStartCp, nPos );
        aEndCps.Insert( nStartCp, nPos );
        String* p = new String( rNm );
        aSwBkmkNms.Insert( p, nPos );
// JP 24.06.99: not used at time
//      p = new String( GetWWBkmkName(rNm ));
//      aWWBkmkNms.Insert( p, nPos );
    }
    else
    {
        // old -> its the end position
        ASSERT( aEndCps[ nPos ] == aSttCps[ nPos ], "end position is valid" );
        aEndCps.Replace( nStartCp, nPos );
    }
}


void WW8_WrtBookmarks::Write( SwWW8Writer& rWrt )
{
    USHORT nCount = aSttCps.Count(), i;
    if( nCount )
    {
        SvULongs aEndSortTab( 255 < nCount ? 255 : nCount, 4 );
        // sort then endpositions
        for( i = 0; i < nCount; ++i )
        {
            ULONG nCP = aEndCps[ i ];
            USHORT nPos = i;
            while( nPos && aEndSortTab[ nPos - 1 ] > nCP )
                --nPos;
            aEndSortTab.Insert( nCP, nPos );
        }

        // we have some bookmarks found in the document -> write them
        // first the Bookmark Name Stringtable
        rWrt.WriteAsStringTable( (SvStrings&)aSwBkmkNms,
                                    rWrt.pFib->fcSttbfbkmk,
                                    rWrt.pFib->lcbSttbfbkmk );

        // second the Bookmark start positions as pcf of longs
        SvStream& rStrm = rWrt.bWrtWW8 ? *rWrt.pTableStrm : rWrt.Strm();
        rWrt.pFib->fcPlcfbkf = rStrm.Tell();
        for( i = 0; i < nCount; ++i )
            SwWW8Writer::WriteLong( rStrm, aSttCps[ i ] );
        SwWW8Writer::WriteLong( rStrm, rWrt.pFib->ccpText );
        for( i = 0; i < nCount; ++i )
        {
            ULONG nEndCP = aEndCps[ i ];
            USHORT nPos = i;
            if( aEndSortTab[ nPos ] > nEndCP )
            {
                while( aEndSortTab[ --nPos ] != nEndCP )
                    ;
            }
            else if( aEndSortTab[ nPos ] < nEndCP )
                while( aEndSortTab[ ++nPos ] != nEndCP )
                    ;

            SwWW8Writer::WriteLong( rStrm, nPos );
        }
        rWrt.pFib->lcbPlcfbkf = rStrm.Tell() - rWrt.pFib->fcPlcfbkf;

        // third the Bookmark end positions
        rWrt.pFib->fcPlcfbkl = rStrm.Tell();
        for( i = 0; i < nCount; ++i )
            SwWW8Writer::WriteLong( rStrm, aEndSortTab[ i ] );
        SwWW8Writer::WriteLong( rStrm, rWrt.pFib->ccpText );
        rWrt.pFib->lcbPlcfbkl = rStrm.Tell() - rWrt.pFib->fcPlcfbkl;
    }
}

// JP 24.06.99: not used at time
/*String WW8_WrtBookmarks::GetWWBkmkName( const String& rName ) const
{
    // replace all characters that will WinWord not understand
    return rName;
}
*/

USHORT WW8_WrtBookmarks::GetPos( const String& rNm )
{
    USHORT nRet = USHRT_MAX, n;
    for( n = 0; n < aSttCps.Count(); ++n )
        if( rNm == *aSwBkmkNms.GetObject( n ))
        {
            nRet = n;
            break;
        }
    return nRet;
}

void SwWW8Writer::AppendBookmarks( const SwTxtNode& rNd,
                                    xub_StrLen nAktPos, xub_StrLen nLen )
{
    SvPtrarr aArr( 8, 8 );
    USHORT nCntnt;
    xub_StrLen nAktEnd = nAktPos + nLen;
    if( GetBookmarks( rNd, nAktPos, nAktEnd, aArr ))
    {
        ULONG nNd = rNd.GetIndex(), nSttCP = Fc2Cp( Strm().Tell() );
        for( USHORT n = 0; n < aArr.Count(); ++n )
        {
            const SwBookmark& rBkmk = *(SwBookmark*)aArr[ n ];
            const SwPosition* pPos = &rBkmk.GetPos(),
                            * pOPos = rBkmk.GetOtherPos();
            if( pOPos && pOPos->nNode == pPos->nNode &&
                pOPos->nContent < pPos->nContent )
            {
                pOPos = pPos;
                pPos = rBkmk.GetOtherPos();
            }

            if( !pOPos || ( nNd == pPos->nNode.GetIndex() &&
                ( nCntnt = pPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd ) )
            {
                ULONG nCp = nSttCP + pPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append( nCp, rBkmk.GetName() );
            }
            if( pOPos && nNd == pOPos->nNode.GetIndex() &&
                ( nCntnt = pOPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd )
            {
                ULONG nCp = nSttCP + pOPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append( nCp, rBkmk.GetName() );
            }
        }
    }
}

void SwWW8Writer::AppendBookmark( const String& rName, USHORT nOffset )
{
    ULONG nSttCP = Fc2Cp( Strm().Tell() ) + nOffset;
    pBkmks->Append( nSttCP, rName );
}


//--------------------------------------------------------------------------
/*  */

USHORT WW8_WrtRedlineAuthor::AddName( const String& rNm )
{
    for( USHORT n = 0, nCnt = aAuthors.Count(); n < nCnt; ++n )
        if( *aAuthors[ n ] == rNm )
            return n;

    String* pNew = new String( rNm );
    aAuthors.Insert( pNew, n );
    return n;
}

void WW8_WrtRedlineAuthor::Write( SwWW8Writer& rWrt )
{
    rWrt.WriteAsStringTable( (SvStrings&)aAuthors,
                                rWrt.pFib->fcSttbfRMark,
                                rWrt.pFib->lcbSttbfRMark,
                                rWrt.bWrtWW8 ? 0 : 2 );
}

USHORT SwWW8Writer::AddRedlineAuthor( USHORT nId )
{
    if( !pRedlAuthors )
    {
        pRedlAuthors = new WW8_WrtRedlineAuthor;
        pRedlAuthors->AddName( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Unknown" )));
    }
    return pRedlAuthors->AddName( SW_MOD()->GetRedlineAuthor( nId ) );
}

//--------------------------------------------------------------------------
/*  */

void SwWW8Writer::WriteAsStringTable( const SvStrings& rStrings,
                                        INT32& rfcSttbf, INT32& rlcbSttbf,
                                        USHORT nExtraLen )
{
    USHORT n, nCount = rStrings.Count();
    if( nCount )
    {
        // we have some Redlines found in the document -> the
        // Author Name Stringtable
        SvStream& rStrm = bWrtWW8 ? *pTableStrm : Strm();
        rfcSttbf = rStrm.Tell();
        if( bWrtWW8 )
        {
            SwWW8Writer::WriteShort( rStrm, -1 );
            SwWW8Writer::WriteLong( rStrm, nCount );
            for( n = 0; n < nCount; ++n )
            {
                const String& rNm = *rStrings[ n ];
                SwWW8Writer::WriteShort( rStrm, rNm.Len() );
                SwWW8Writer::WriteString16( rStrm, rNm, FALSE );
                if( nExtraLen )
                    SwWW8Writer::FillCount( rStrm, nExtraLen );
            }
        }
        else
        {
            SwWW8Writer::WriteShort( rStrm, 0 );
            for( n = 0; n < nCount; ++n )
            {
                const String aNm( rStrings[ n ]->Copy(0, 255 ));
                rStrm << (BYTE)aNm.Len();
                SwWW8Writer::WriteString8( rStrm, aNm, FALSE,
                                                RTL_TEXTENCODING_MS_1252 );
                if( nExtraLen )
                    SwWW8Writer::FillCount( rStrm, nExtraLen );
            }
        }
        rlcbSttbf = rStrm.Tell() - rfcSttbf;
        if( !bWrtWW8 )
            SwWW8Writer::WriteShort( rStrm, rfcSttbf, (USHORT)rlcbSttbf );
    }
}

// WriteShort() traegt an FilePos nPos den Wert nVal ein und seekt auf die
// alte FilePos zurueck. Benutzt zum Nachtragen von Laengen.
void SwWW8Writer::WriteShort( SvStream& rStrm, ULONG nPos, INT16 nVal )
{
    ULONG nOldPos = rStrm.Tell();       // Pos merken
    rStrm.Seek( nPos );
    SwWW8Writer::WriteShort( rStrm, nVal );
    rStrm.Seek( nOldPos );
}

void SwWW8Writer::WriteLong( SvStream& rStrm, ULONG nPos, INT32 nVal )
{
    ULONG nOldPos = rStrm.Tell();       // Pos merken
    rStrm.Seek( nPos );
    SwWW8Writer::WriteLong( rStrm, nVal );
    rStrm.Seek( nOldPos );
}

#ifdef __WW8_NEEDS_COPY

void SwWW8Writer::InsUInt16( WW8Bytes& rO, UINT16 n )
{
    SVBT16 nL;
    ShortToSVBT16( n, nL );
    rO.Insert( nL, 2, rO.Count() );
}
void SwWW8Writer::InsUInt32( WW8Bytes& rO, UINT32 n )
{
    SVBT32 nL;
    LongToSVBT32( n, nL );
    rO.Insert( nL, 4, rO.Count() );
}

#else

void SwWW8Writer::InsUInt16( WW8Bytes& rO, UINT16 n )
{
    rO.Insert( (BYTE*)&n, 2, rO.Count() );
}
void SwWW8Writer::InsUInt32( WW8Bytes& rO, UINT32 n )
{
    rO.Insert( (BYTE*)&n, 4, rO.Count() );
}

#endif // defined __WW8_NEEDS_COPY

void SwWW8Writer::InsAsString16( WW8Bytes& rO, const String& rStr )
{
    const sal_Unicode* pStr = rStr.GetBuffer();
    for( xub_StrLen n = 0, nLen = rStr.Len(); n < nLen; ++n, ++pStr )
        SwWW8Writer::InsUInt16( rO, *pStr );
}

void SwWW8Writer::InsAsString8( WW8Bytes& rO, const String& rStr,
                                rtl_TextEncoding eCodeSet )
{
    ByteString sTmp( rStr, eCodeSet );
    rO.Insert( (BYTE*)sTmp.GetBuffer(), sTmp.Len(), rO.Count() );
}

void SwWW8Writer::WriteString16( SvStream& rStrm, const String& rStr,
                                    BOOL bAddZero )
{
    WW8Bytes aBytes;
    SwWW8Writer::InsAsString16( aBytes, rStr );
    if( bAddZero )
        SwWW8Writer::InsUInt16( aBytes, 0 );
    rStrm.Write( aBytes.GetData(), aBytes.Count() );
}

void SwWW8Writer::WriteString8( SvStream& rStrm, const String& rStr,
                                BOOL bAddZero, rtl_TextEncoding eCodeSet )
{
    WW8Bytes aBytes;
    SwWW8Writer::InsAsString8( aBytes, rStr, eCodeSet );
    if( bAddZero )
        aBytes.Insert( (BYTE)0, aBytes.Count() );
    rStrm.Write( aBytes.GetData(), aBytes.Count() );
}

void SwWW8Writer::WriteStringAsPara( const String& rTxt, USHORT nStyleId )
{
    if( rTxt.Len() )
        OutSwString( rTxt, 0, rTxt.Len(), IsUnicode(), RTL_TEXTENCODING_MS_1252 );
    WriteCR();              // CR danach

    WW8Bytes aArr( 10, 10 );
    SwWW8Writer::InsUInt16( aArr, nStyleId );
    if( bOutTable )
    {                                               // Tab-Attr
        // sprmPFInTable
        if( bWrtWW8 )
            SwWW8Writer::InsUInt16( aArr, 0x2416 );
        else
            aArr.Insert( 24, aArr.Count() );
        aArr.Insert( 1, aArr.Count() );
    }

    ULONG nPos = Strm().Tell();
    pPapPlc->AppendFkpEntry( nPos, aArr.Count(), aArr.GetData() );
    pChpPlc->AppendFkpEntry( nPos );
}

void SwWW8Writer::WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp )
{
    BYTE nOldTyp = nTxtTyp;
    nTxtTyp = nTTyp;
    SwPaM* pOldPam = pCurPam;       //!! Einfaches Umsetzen des PaM ohne
    SwPaM* pOldEnd = pOrigPam;          // Wiederherstellen muesste es auch tun
    BOOL bOldPageDescs = bOutPageDescs;
    bOutPageDescs = FALSE;
                                    // bOutKF wird in WriteKF1 gemerkt / gesetzt
    pCurPam = NewSwPaM( *pDoc, nStart, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if(    (nStart != pCurPam->GetMark()->nNode.GetIndex())
        && pDoc->GetNodes()[ nStart ]->IsTableNode() )
        pCurPam->GetMark()->nNode = nStart;

    pOrigPam = pCurPam;
    pCurPam->Exchange();

    WriteText();

    delete pCurPam;                    // Pam wieder loeschen
    bOutPageDescs = bOldPageDescs;
    pCurPam = pOldPam;
    pOrigPam = pOldEnd;
    nTxtTyp = nOldTyp;
}

void SwWW8Writer::OutSwString( const String& rStr, xub_StrLen nStt,
                                xub_StrLen nLen, BOOL bUnicode,
                                rtl_TextEncoding eChrSet )

{
    if( nLen )
    {
        if ( bUnicode != pPiece->IsUnicode() )
            pPiece->AppendPc ( Strm().Tell(), bUnicode );

        if( nStt || nLen != rStr.Len() )
        {
            String sOut( rStr.Copy( nStt, nLen ) );
            if( bUnicode )
                SwWW8Writer::WriteString16( Strm(), sOut, FALSE );
            else
                SwWW8Writer::WriteString8( Strm(), sOut, FALSE, eChrSet );
        }
        else if( bUnicode )
            SwWW8Writer::WriteString16( Strm(), rStr, FALSE );
        else
            SwWW8Writer::WriteString8( Strm(), rStr, FALSE, eChrSet );
    }
}

void SwWW8Writer::WriteCR()
{
    WriteChar( '\015' );
    pPiece->SetParaBreak();
}

void SwWW8Writer::WriteChar( sal_Unicode c )
{
    if( pPiece->IsUnicode() )
        Strm() << c;
    else
        Strm() << (BYTE)c;
}

long SwWW8Writer::GetDTTM( const DateTime& rDT )
{
/*
mint    short   :6  0000003F    minutes (0-59)
hr      short   :5  000007C0    hours (0-23)
dom     short   :5  0000F800    days of month (1-31)
mon     short   :4  000F0000    months (1-12)
yr      short   :9  1FF00000    years (1900-2411)-1900
wdy     short   :3  E0000000    weekday(Sunday=0
                                        Monday=1
( wdy can be ignored )                  Tuesday=2
                                        Wednesday=3
                                        Thursday=4
                                        Friday=5
                                        Saturday=6)
*/

    long nDT = ( rDT.GetDayOfWeek() + 1 ) % 7;
    nDT <<= 9;
    nDT += ( rDT.GetYear() - 1900 ) & 0x1ff;
    nDT <<= 4;
    nDT += rDT.GetMonth() & 0xf;
    nDT <<= 5;
    nDT += rDT.GetDay() & 0x1f;
    nDT <<= 5;
    nDT += rDT.GetHour() & 0x1f;
    nDT <<= 6;
    nDT += rDT.GetMin() & 0x3f;
    return nDT;
}

/*  */
//---------------------------------------------------------------------------
//       Hilfsroutinen fuer Flys
//---------------------------------------------------------------------------
// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
WW8SaveData::WW8SaveData( SwWW8Writer& rWriter, ULONG nStt, ULONG nEnd )
    : rWrt( rWriter ),
    pOldPam( rWrt.pCurPam ), pOldEnd( rWrt.GetEndPaM() ),
    pOldFlyFmt( rWrt.pFlyFmt ), pOldPageDesc( rWrt.pAktPageDesc )

{
    pOldFlyOffset = rWrt.pFlyOffset;
    if( rWrt.pO->Count() )
    {
        pOOld = rWrt.pO;
        rWrt.pO = new WW8Bytes( 128, 128 );
    }
    else
        pOOld = 0;
    bOldWriteAll = rWrt.bWriteAll;
    bOldOutTable = rWrt.bOutTable;
    bOldIsInTable= rWrt.bIsInTable;
    bOldFlyFrmAttrs = rWrt.bOutFlyFrmAttrs;
    bOldStartTOX = rWrt.bStartTOX;
    bOldInWriteTOX = rWrt.bInWriteTOX;

    rWrt.pCurPam = rWrt.NewSwPaM( *rWrt.pDoc, nStt, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if( nStt != rWrt.pCurPam->GetMark()->nNode.GetIndex() &&
        rWrt.pDoc->GetNodes()[ nStt ]->IsTableNode() )
        rWrt.pCurPam->GetMark()->nNode = nStt;

    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = TRUE;
    rWrt.bOutTable = FALSE;
    // Vorsicht:  rWrt.bIsInTable  darf hier NICHT veraendert werden!
    rWrt.bOutFlyFrmAttrs = FALSE;
//  rWrt.pAttrSet = 0;
    rWrt.bStartTOX = FALSE;
    rWrt.bInWriteTOX = FALSE;
}


WW8SaveData::~WW8SaveData()
{
    delete rWrt.pCurPam;                    // Pam wieder loeschen

    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.bOutTable = bOldOutTable;
    rWrt.bIsInTable= bOldIsInTable;
    rWrt.bOutFlyFrmAttrs = bOldFlyFrmAttrs;
    rWrt.bStartTOX = bOldStartTOX;
    rWrt.bInWriteTOX = bOldInWriteTOX;
    rWrt.pFlyFmt = pOldFlyFmt;
    rWrt.pAktPageDesc = pOldPageDesc;
//  rWrt.pAttrSet = pOldAttrSet;
    ASSERT( !rWrt.pO->Count(), " pO ist am Ende von WW8SaveData nicht leer" );
    if( pOOld )
    {
        delete rWrt.pO;
        rWrt.pO = pOOld;
    }
    rWrt.pFlyOffset = pOldFlyOffset;
}


void SwWW8Writer::WriteText()
{
    while( pCurPam->GetPoint()->nNode < pCurPam->GetMark()->nNode ||
            (pCurPam->GetPoint()->nNode == pCurPam->GetMark()->nNode &&
            pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
    {
        SwNode * pNd = pCurPam->GetNode();

        if( pNd->IsTxtNode() ) // blitzschnelle Methode
        {
            const SwTxtNode* pTxtNode = pNd->GetTxtNode();
            if( pTxtNode->GetpSwAttrSet() )
                Out_SfxBreakItems( *pTxtNode->GetpSwAttrSet(), *pTxtNode );
            if( !bIsInTable )
                OutWW8FlyFrmsInCntnt( *pTxtNode ); // als Zeichen gebundene Flys
        }

        if( pNd->IsCntntNode() )
        {
            SwCntntNode* pCNd = (SwCntntNode*)pNd;

            // erfrage den aktuellen PageDescriptor.
            if( !bInWriteEscher )
                OutFlyFrms( *pCNd );

            const SwPageDesc* pTemp = pCNd->GetSwAttrSet().GetPageDesc().GetPageDesc();
            if( pTemp )
                pAktPageDesc = pTemp;

            pCurPam->GetPoint()->nContent.Assign( pCNd, 0 );
            Out( aWW8NodeFnTab, *pCNd, *this );
        }
        else if( pNd->IsTableNode() && !bIsInTable )
            OutWW8_SwTblNode( *this, *(SwTableNode*)pNd );
        else if( pNd->IsSectionNode() && TXT_MAINTEXT == nTxtTyp )
            OutWW8_SwSectionNode( *this, *pNd->GetSectionNode() );
        else if( TXT_MAINTEXT == nTxtTyp && pNd->IsEndNode() &&
                 pNd->FindStartNode()->IsSectionNode() )
        {
            const SwSection& rSect = pNd->FindStartNode()->GetSectionNode()
                                        ->GetSection();
            if( bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
                bStartTOX = FALSE;

            SwNodeIndex aIdx( *pNd, 1 );
            if(    aIdx.GetNode().IsEndNode()
                && aIdx.GetNode().FindStartNode()->IsSectionNode() )
                ;
            else
            if(    aIdx.GetNode().IsSectionNode() )
                ;
            else
            {
                ReplaceCr( (char)0xc ); // Indikator fuer Page/Section-Break

                const SwSectionFmt* pParentFmt = rSect.GetFmt()->GetParent();
                if( !pParentFmt )
                    pParentFmt = (SwSectionFmt*)0xFFFFFFFF;

                ULONG nRstLnNum;
                if( aIdx.GetNode().IsCntntNode() )
                    nRstLnNum = ((SwCntntNode&)aIdx.GetNode()).GetSwAttrSet().
                                            GetLineNumber().GetStartValue();
                else
                    nRstLnNum = 0;

                pSepx->AppendSep( Fc2Cp( Strm().Tell() ),
                                    pAktPageDesc, pParentFmt, nRstLnNum );
            }
        }
        else if( pNd == &pNd->GetNodes().GetEndOfContent() )
            break;

        ULONG nPos = pCurPam->GetPoint()->nNode++;  // Bewegen
        ::SetProgressState( nPos, pCurPam->GetDoc()->GetDocShell() );   // Wie weit ?
    }
}

void SwWW8Writer::WriteMainText()
{
    pFib->fcMin = Strm().Tell();

    do {
        WriteText();
    } while( FALSE );       //!! Temporaer nur 1 Bereich
//  } while( CopyNextPam( &pCurPam ) );     // bis alle Pam bearbeitet

    if( 0 == Strm().Tell() - pFib->fcMin )  // kein Text ?
        WriteCR();                  // dann CR ans Ende ( sonst mault WW )

    pFib->ccpText = Fc2Cp( Strm().Tell() );
    pFldMain->Finish( pFib->ccpText, 0 );

                    // ccpText beinhaltet Ftn- und KF-Texte
                    // deshalb wird pFib->ccpText evtl. noch geupdated
    // save the StyleId of the last paragraph. Because WW97 take the style
    // from the last CR, that will be write after footer/Header/fontnotes/
    // annotation usw.
    const SwTxtNode* pLastNd = pCurPam->GetMark()->nNode.GetNode().GetTxtNode();
    if( pLastNd )
        nLastFmtId = GetId( (SwTxtFmtColl&)pLastNd->GetAnyFmtColl() );
}

void SwWW8Writer::WriteFkpPlcUsw()
{
    if( !bWrtWW8 )
    {
        static BYTE __READONLY_DATA aSpec[2] = { 117, 1 };

        pChpPlc->AppendFkpEntry( Strm().Tell() );   // Sepx mit fSpecial
        pSepx->WriteSepx( Strm() );                 // Slcx.Sepx
        pGrf->Write();                              // Grafiken
        pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aSpec ), aSpec );

        pChpPlc->WriteFkps();                   // Fkp.Chpx
        pPapPlc->WriteFkps();                   // Fkp.Papx
        pStyles->OutStyleTab();                 // Styles
        pFtn->WritePlc( *this );                // Footnote-Ref & Text Plc
        pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
        pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc
        pSepx->WritePlcSed( *this );            // Slcx.PlcSed
        pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd
        pChpPlc->WritePlc();                    // Plcx.Chpx
        pPapPlc->WritePlc();                    // Plcx.Papx
        OutFontTab( *pFib );                    // FFNs
        if( pRedlAuthors )
            pRedlAuthors->Write( *this );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
        pBkmks->Write( *this );                 // Bookmarks - sttbfBkmk/
                                                // plcfBkmkf/plcfBkmkl
        WriteDop( *this );                      // Document-Properties

    }
    else
    {
        // Grafiken in den Data-Tream
//???       static BYTE __READONLY_DATA aSpec[] = { 0x55,0x08,0x01 };
//???       pChpPlc->AppendFkpEntry( pDataStrm->Tell() );   // Sepx mit fSpecial

        pGrf->Write();                      // Grafiken
//???       pChpPlc->AppendFkpEntry( pDataStrm->Tell(), sizeof( aSpec ), aSpec );

        // Ausgabe in WordDocument-Stream
        pChpPlc->WriteFkps();                   // Fkp.Chpx
        pPapPlc->WriteFkps();                   // Fkp.Papx
        pSepx->WriteSepx( Strm() );             // Sepx

        // Ausagbe in Table-Stream
        pStyles->OutStyleTab();                 // fuer WW8 StyleTab
        pFtn->WritePlc( *this );                // Footnote-Ref & Text Plc
        pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
        pTxtBxs->WritePlc( *this );             // Textbox Text Plc
        pHFTxtBxs->WritePlc( *this );           // Head/Foot-Textbox Text Plc
        pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc

        pSepx->WritePlcSed( *this );            // Slcx.PlcSed
        pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd

        pChpPlc->WritePlc();                    // Plcx.Chpx
        pPapPlc->WritePlc();                    // Plcx.Papx

        if( pRedlAuthors )
            pRedlAuthors->Write( *this );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
        pFldTxtBxs->Write( *this );             // Fields ( Textboxes )
        pFldHFTxtBxs->Write( *this );           // Fields ( Head/Foot-Textboxes )

        // dggInfo - escher stream
        WriteEscher();

        pSdrObjs->WritePlc( *this );
        pHFSdrObjs->WritePlc( *this );
        // spamom - office drawing table
        // spahdr - header office drawing table

        pBkmks->Write( *this );                 // Bookmarks - sttbfBkmk/
                                                // plcfBkmkf/plcfBkmkl

        OutListTab();                           // listformats  - LSTF
        OutOverrideListTab();                   //   - "" -     - LFO
        OutListNamesTab();                      //   - "" -     - ListNames

        pPiece->WritePc( *this );               // Piece-Table
        OutFontTab( *pFib );                    // FFNs

        //Convert OOo asian typography into MS typography structure
        ExportDopTypography(pDop->doptypography);

        WriteDop( *this );                      // Document-Properties
    }
    Strm().Seek( 0 );

    pFib->Write( Strm() );  // FIB
}

void SwWW8Writer::StoreDoc1()
{
    // Start of Text ( Mangel ueber )
    SwWW8Writer::FillUntil( Strm(), pFib->fcMin );

    WriteMainText();                    // HauptText
    pFtn->WriteTxt( *this );            // Footnote-Text
    pSepx->WriteKFTxt( *this );         // K/F-Text
    pAtn->WriteTxt( *this );            // Annotation-Text
    pEdn->WriteTxt( *this );            // EndNote-Text

    // create the escher streams
    if( bWrtWW8 )
        CreateEscher();

    pTxtBxs->WriteTxt( *this );         // Textbox Text Plc
    pHFTxtBxs->WriteTxt( *this );       // Head/Foot-Textbox Text Plc

    WriteStringAsPara( aEmptyStr, nLastFmtId ); // CR ans Ende ( sonst mault WW )

    pSepx->Finish( Fc2Cp( Strm().Tell() ));// Text + Ftn + HdFt als Section-Ende

    pFib->fcMac = Strm().Tell();        // Ende aller Texte

    WriteFkpPlcUsw();                   // FKP, PLC, .....
}

ULONG SwWW8Writer::StoreDoc()
{
    {
        static const sal_Char* aNames[ 1 ] = { "WinWord/WWWR" };
        sal_uInt32 aVal[ 1 ];
        SwFilterOptions aOpt( 1, aNames, aVal );
        nIniFlags = aVal[ 0 ];
    }
    nCharFmtStart = ANZ_DEFAULT_STYLES;
    nFmtCollStart = nCharFmtStart + pDoc->GetCharFmts()->Count() - 1;

    bStyDef = bBreakBefore = bOutKF =
        bOutFlyFrmAttrs = bOutPageDescs = bOutTable = bOutFirstPage =
        bIsInTable = bOutGrf = bInWriteEscher = bStartTOX =
        bInWriteTOX = FALSE;

    bFtnAtTxtEnd = bEndAtTxtEnd = TRUE;

    pFlyFmt = 0;
    pFlyOffset = 0;
    nTxtTyp = TXT_MAINTEXT;
    nFlyWidth = nFlyHeight = 0;
    nStyleBeforeFly = nLastFmtId = 0;
    pStyAttr = 0;
    pOutFmtNode = 0;
    pUsedNumTbl = 0;
    pEscher = 0;
    pRedlAuthors = 0;
    if( aTOXArr.Count() )
        aTOXArr.Remove( 0, aTOXArr.Count() );

    if( !pOLEExp )
    {
        UINT32 nSvxMSDffOLEConvFlags = 0;
        const OfaFilterOptions* pOpt = OFF_APP()->GetFilterOptions();
        if( pOpt->IsMath2MathType() )
            nSvxMSDffOLEConvFlags |= OLE_STARMATH_2_MATHTYPE;
        if( pOpt->IsWriter2WinWord() )
            nSvxMSDffOLEConvFlags |= OLE_STARWRITER_2_WINWORD;
        if( pOpt->IsCalc2Excel() )
            nSvxMSDffOLEConvFlags |= OLE_STARCALC_2_EXCEL;
        if( pOpt->IsImpress2PowerPoint() )
            nSvxMSDffOLEConvFlags |= OLE_STARIMPRESS_2_POWERPOINT;

        pOLEExp = new SvxMSExportOLEObjects( nSvxMSDffOLEConvFlags );
    }

    if( !pOCXExp )
        pOCXExp = new SwMSConvertControls(pDoc->GetDocShell(),pCurPam);

    PrepareStorage();

    PutNumFmtFontsInAttrPool();
    PutEditEngFontsInAttrPool();
    PutCJKandCTLFontsInAttrPool();

    pFib = new WW8Fib( bWrtWW8 ? 8 : 6 );

    SvStream* pOldStrm = pStrm;         // JP 19.05.99: wozu das ???
    SvStorageStreamRef xWwStrm( pStg->OpenStream( aMainStg ) );
    SvStorageStreamRef xTableStrm( xWwStrm ), xDataStrm( xWwStrm );
    xWwStrm->SetBufferSize( 32768 );

    if( bWrtWW8 )
    {
        pFib->fWhichTblStm = 1;
        xTableStrm = pStg->OpenStream( sTableStream, STREAM_STD_WRITE );
        xDataStrm = pStg->OpenStream( sDataStream, STREAM_STD_WRITE );

        xDataStrm->SetBufferSize( 32768 );  // fuer Grafiken
        xTableStrm->SetBufferSize( 16384 ); // fuer die Font-/Style-Table, usw.

        xTableStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        xDataStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }

    pStrm = &xWwStrm;
    pTableStrm = &xTableStrm;
    pDataStrm = &xDataStrm;

    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    pAktPageDesc = &pDoc->GetPageDesc( 0 );     // Default: "Standard"
    pSepx = new WW8_WrPlcSepx;      // Sepx
    {
        const SfxPoolItem* pI;
        const SwNode* pNd = pCurPam->GetCntntNode();
        const SfxItemSet* pSet = pNd ? &((SwCntntNode*)pNd)->GetSwAttrSet()
                                     : 0;

        ULONG nRstLnNum =  pSet ? ((SwFmtLineNumber&)pSet->Get(
                                            RES_LINENUMBER )).GetStartValue()
                                : 0;

        const SwTableNode* pTblNd = pCurPam->GetNode()->FindTableNode();
        const SwSectionNode* pSectNd;
        if( pTblNd )
        {
            pSet = &pTblNd->GetTable().GetFrmFmt()->GetAttrSet();
            pNd = pTblNd;
        }
        else if( 0 != ( pSectNd = pNd->FindSectionNode() ) )
        {
            if( TOX_HEADER_SECTION == pSectNd->GetSection().GetType() &&
                pSectNd->FindStartNode()->IsSectionNode() )
                pSectNd = pSectNd->FindStartNode()->GetSectionNode();

            if( TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
            {
                pNd = pSectNd;
                pCurPam->GetPoint()->nNode = *pNd;
            }
        }

        // Hole evtl. Pagedesc des 1. Nodes
        if( pSet && SFX_ITEM_ON == pSet->GetItemState(RES_PAGEDESC,TRUE,&pI)
            && ((SwFmtPageDesc*)pI)->GetPageDesc() )
            pSepx->AppendSep( 0, *(SwFmtPageDesc*)pI, *pNd, nRstLnNum );
        else
            pSepx->AppendSep( 0, pAktPageDesc, 0, nRstLnNum );
    }

    pFtn = new WW8_WrPlcFtnEdn( TXT_FTN );                      // Ftn
    pEdn = new WW8_WrPlcFtnEdn( TXT_EDN );                      // Edn
    pAtn = new WW8_WrPlcPostIt;                                 // PostIts
    pTxtBxs = new WW8_WrPlcTxtBoxes( TXT_TXTBOX );
    pHFTxtBxs = new WW8_WrPlcTxtBoxes( TXT_HFTXTBOX );

    pSdrObjs = new WW8_WrPlcDrawObj( TXT_TXTBOX );      // Draw-/Fly-Objects
    pHFSdrObjs = new WW8_WrPlcDrawObj( TXT_HFTXTBOX );  // Draw-/Fly-Objects
                                                        // in header or footer

    pBkmks = new WW8_WrtBookmarks;                          // Bookmarks
    CreateBookmarkTbl();

    pPapPlc = new WW8_WrPlcPn( *this, PAP, pFib->fcMin );
    pChpPlc = new WW8_WrPlcPn( *this, CHP, pFib->fcMin );
    pO = new WW8Bytes( 128, 128 );
    pStyles = new WW8WrtStyle( *this );
    pFldMain = new WW8_WrPlcFld( 2, TXT_MAINTEXT );
    pFldHdFt = new WW8_WrPlcFld( 2, TXT_HDFT );
    pFldFtn = new WW8_WrPlcFld( 2, TXT_FTN );
    pFldEdn = new WW8_WrPlcFld( 2, TXT_EDN );
    pFldTxtBxs = new WW8_WrPlcFld( 2, TXT_TXTBOX );
    pFldHFTxtBxs = new WW8_WrPlcFld( 2, TXT_HFTXTBOX );

    pGrf = new SwWW8WrGrf( *this );
    pPiece = new WW8_WrPct( pFib->fcMin, bWrtWW8 );
    pDop = new WW8Dop;

    USHORT nRedlineMode = pDoc->GetRedlineMode();
    pDop->fRevMarking = 0 != (REDLINE_ON & nRedlineMode);
    pDop->fRMView = 0 != ( REDLINE_SHOW_DELETE & nRedlineMode );
    if( pDoc->GetRedlineTbl().Count() )
        pDoc->SetRedlineMode( nRedlineMode | REDLINE_SHOW_DELETE |
                                            REDLINE_SHOW_INSERT );

    // set AutoHyphenation flag if found in default para style
    const SfxPoolItem* pItem;
    SwTxtFmtColl* pStdTxtFmtColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    if(    pStdTxtFmtColl
        && SFX_ITEM_SET == pStdTxtFmtColl->GetItemState(
                                        RES_PARATR_HYPHENZONE, FALSE, &pItem ) )
        pDop->fAutoHyphen = ((SvxHyphenZoneItem*)pItem)->IsHyphen();

    // make unique OrdNums (Z-Order) for all drawing-/fly Objects
    if( pDoc->GetDrawModel() )
        pDoc->GetDrawModel()->GetPage( 0 )->RecalcObjOrdNums();

    StoreDoc1();

    if( nRedlineMode != pDoc->GetRedlineMode() )
        pDoc->SetRedlineMode( nRedlineMode );

    if( pUsedNumTbl )           // all used NumRules
    {
        // clear array  - it's an auto delete array
        pUsedNumTbl->Remove( 0, pUsedNumTbl->Count() );
        delete pUsedNumTbl;
    }

    DELETEZ( pGrf );
    DELETEZ( pFldFtn );
    DELETEZ( pFldTxtBxs );
    DELETEZ( pFldHFTxtBxs );
    DELETEZ( pFldEdn );
    DELETEZ( pFldHdFt );
    DELETEZ( pFldMain );
    DELETEZ( pStyles );
    DELETEZ( pO );
    DELETEZ( pChpPlc );
    DELETEZ( pPapPlc );
    DELETEZ( pSepx );

    delete pRedlAuthors;
    delete pSdrObjs;
    delete pHFSdrObjs;
    delete pTxtBxs;
    delete pHFTxtBxs;
    delete pAtn;
    delete pEdn;
    delete pFtn;
    delete pBkmks;
    delete pPiece;
    delete pDop;
    delete pFib;
    pStrm = pOldStrm;


    xWwStrm->SetBufferSize( 0 );
    if( bWrtWW8 )
    {
        xTableStrm->SetBufferSize( 0 );
        xDataStrm->SetBufferSize( 0 );
        if( 0 == pDataStrm->Seek( STREAM_SEEK_TO_END ))
        {
            xDataStrm.Clear();
            pDataStrm = 0;
            pStg->Remove( sDataStream );
        }
    }

    return 0;
}


void SwWW8Writer::PrepareStorage()
{
    ULONG nLen;
    const BYTE* pData;
    const char* pName;
    UINT32 nId1;

    if( bWrtWW8 )
    {
        static char __READONLY_DATA aUserName[] = "Microsoft Word-Document";
        static BYTE __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x09, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x46, 0x18, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x57, 0x6F, 0x72, 0x64, 0x2D, 0x44,
            0x6F, 0x6B, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x00,
            0x0A, 0x00, 0x00, 0x00, 0x4D, 0x53, 0x57, 0x6F,
            0x72, 0x64, 0x44, 0x6F, 0x63, 0x00, 0x10, 0x00,
            0x00, 0x00, 0x57, 0x6F, 0x72, 0x64, 0x2E, 0x44,
            0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x2E,
            0x38, 0x00, 0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00 };

        pName = aUserName;
        pData = aCompObj;
        nLen = sizeof( aCompObj );
        nId1 = 0x00020906L;
    }
    else
    {
        static char __READONLY_DATA aUserName[] = "Microsoft Word 6.0 Document";
        static BYTE __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x09, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x46, 0x1C, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x57, 0x6F, 0x72, 0x64, 0x20, 0x36,
            0x2E, 0x30, 0x2D, 0x44, 0x6F, 0x6B, 0x75, 0x6D,
            0x65, 0x6E, 0x74, 0x00, 0x0A, 0x00, 0x00, 0x00,
            0x4D, 0x53, 0x57, 0x6F, 0x72, 0x64, 0x44, 0x6F,
            0x63, 0x00, 0x10, 0x00, 0x00, 0x00, 0x57, 0x6F,
            0x72, 0x64, 0x2E, 0x44, 0x6F, 0x63, 0x75, 0x6D,
            0x65, 0x6E, 0x74, 0x2E, 0x36, 0x00, 0x00, 0x00,
            0x00, 0x00 };

        pName = aUserName;
        pData = aCompObj;
        nLen = sizeof( aCompObj );
        nId1 = 0x00020900L;
    }

    SvGlobalName aGName( nId1, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x46 );
    pStg->SetClass( aGName, 0, String::CreateFromAscii( pName ));
    SvStorageStreamRef xStor( pStg->OpenStream( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "\1CompObj" ))));
    xStor->Write( pData, nLen );
                // noch mal ueberplaetten, um auch Clipboardformat zu setzen
    pDoc->GetInfo()->SavePropertySet( pStg );   // DocInfo
}


ULONG SwWW8Writer::WriteStorage()
{
    long nMaxNode = pDoc->GetNodes().Count();
    ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

    // Tabelle am Doc.-Anfang beachten
    {
        SwTableNode * pTNd = pCurPam->GetNode()->FindTableNode();
        if( pTNd && bWriteAll )
            // mit dem TabellenNode anfangen !!
            pCurPam->GetPoint()->nNode = *pTNd;
    }

    // Tabelle fuer die freifliegenden Rahmen erzeugen, aber nur wenn
    // das gesamte Dokument geschrieben wird
    nAktFlyPos = 0;
    SwPosFlyFrms aFlyPos;
    pDoc->GetAllFlyFmts( aFlyPos, bWriteAll ? 0 : pOrigPam, bWrtWW8 );
        // Die Sonderbehandlung fuer Teilausgabe fehlt noch ( siehe RTF )
    pFlyPos = &aFlyPos;

    ULONG nRet = StoreDoc();

    // loesche die Tabelle mit den freifliegenden Rahmen
    for( USHORT i = aFlyPos.Count(); i > 0; )
        delete aFlyPos[ --i ];

    ::EndProgress( pDoc->GetDocShell() );
    bWrtWW8 = FALSE;        // sicherheitshalber: Default fuer's naechste Mal
    return nRet;
}

SwWW8Writer::SwWW8Writer( const String& rFltName )
    : pChpIter( 0 ), aMainStg( sMainStream ), pPapPlc( 0 ), pChpPlc( 0 ),
    pO( 0 ), pAktPageDesc( 0 ), pISet( 0 ), pUsedNumTbl( 0 ), pBmpPal( 0 ),
    pKeyMap( 0 ), pOLEExp( 0 ), pOCXExp(0)
{
    bWrtWW8 = rFltName.EqualsAscii( FILTER_WW8 );
}

SwWW8Writer::~SwWW8Writer()
{
    if( pBmpPal )
        delete pBmpPal;
    if( pKeyMap )
    {
        NfKeywordTable* pDel = (NfKeywordTable*)pKeyMap;
        delete [] pDel;
    }
    if( pOLEExp )
        delete pOLEExp;
    if( pOCXExp )
        delete pOCXExp;
}

void GetWW8Writer( const String& rFltName, WriterRef& xRet )
{
    xRet = new SwWW8Writer( rFltName );
}



/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/wrtww8.cxx,v 1.13 2001-03-14 15:54:34 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.12  2001/03/14 10:24:04  jp
      PieceTableCTOR: depends unicode flag on W95/W97 export

      Revision 1.11  2001/02/16 10:08:12  cmc
      Normalize japanese doptypography variable names

      Revision 1.10  2001/02/15 20:03:11  jp
      ExportDopTypography: better compare for the forbidden chars

      Revision 1.9  2001/02/13 15:36:26  jp
      Bug #83797#: default is write UniCode

      Revision 1.8  2001/02/07 12:41:56  jp
      Bug #80745#: InsAsString16 - use Len instead of zero character

      Revision 1.7  2001/02/01 16:11:30  cmc
      #83362# Missing i18n header include

      Revision 1.6  2001/01/30 20:11:06  cmc
      #83362# CJK Forbidden Character {Im|Ex}port

      Revision 1.5  2001/01/23 10:14:33  os
      update of filter configuration

      Revision 1.4  2000/11/20 14:12:26  jp
      ReadFilterFlags removed, use new class SwFilterOptions

      Revision 1.3  2000/11/13 17:30:15  jp
      new method PutCJKandCTLFontsInAttrPool

      Revision 1.2  2000/10/10 16:54:06  cmc
      MSOffice 97/2000 Controls {Im|Ex}port

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.54  2000/09/18 16:04:58  willem.vandorp
      OpenOffice header added.

      Revision 1.53  2000/08/21 10:14:06  khz
      Export Line Numbering (restart on new section)

      Revision 1.52  2000/07/25 15:16:21  khz
      #76811# read/write AutoHyphenation flag from/into Document Properties

      Revision 1.51  2000/07/06 15:55:55  khz
      Document Statistics

      Revision 1.50  2000/07/05 16:07:30  jp
      new MS Filteroptions - change StarOffice Objects to OLE-Objects

      Revision 1.49  2000/05/15 16:40:34  jp
      Changes for Unicode

      Revision 1.48  2000/05/12 16:14:07  jp
      Changes for Unicode

      Revision 1.47  2000/03/28 18:47:53  jp
      Bug #74360#: WrFkp::Append - calculate the correct offset for W95

      Revision 1.46  2000/03/09 09:23:04  jp
      Bug #74053#: write sectionbreaks only in maintext

      Revision 1.45  2000/03/03 16:08:37  pl
      #73771# workaround for c50 intel compiler

      Revision 1.44  2000/03/01 14:21:03  jp
      Bug #73720#: StoreDoc1 - search the TOX content section

      Revision 1.43  2000/02/11 14:39:55  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.42  1999/12/20 17:41:33  jp
      Bug #70749#: Date/Time-Numberformats in german format from the formatter

      Revision 1.41  1999/12/15 17:06:18  jp
      Bug #70913#: write Ftn-/EndNoteInfo - at end of section or page/doc

      Revision 1.40  1999/12/13 13:56:06  jp
      Bug #70766#: StoreDoc - look for TOX at start of Document

      Revision 1.39  1999/11/25 21:22:29  jp
      Bug #70131#: OutFields: append the charattributes of the node to the sprms

      Revision 1.38  1999/11/22 18:26:07  jp
      export of TOX and TOX entries

      Revision 1.37  1999/11/12 13:06:17  jp
      write correct PAP-Fkps for Version 8

      Revision 1.36  1999/11/02 09:44:13  jp
      write sprmPHugePapx

      Revision 1.35  1999/10/29 17:40:18  jp
      export of 'complex'(vert. merged) tables

      Revision 1.34  1999/10/14 21:06:09  jp
      Bug #68617#,#68705# and other little things

      Revision 1.33  1999/10/14 11:09:20  jp
      export Redlines into W95/W97

      Revision 1.32  1999/10/13 18:09:45  jp
      write redlines

      Revision 1.31  1999/10/13 09:08:51  jp
      write character attributs of SdrObjects

      Revision 1.30  1999/10/11 11:00:14  jp
      some changes for export EditEngine text in WW-Format

      Revision 1.29  1999/10/04 16:44:40  jp
      Bug #68679#: check content range of bookmarkpos

      Revision 1.28  1999/09/08 16:29:21  jp
      Bug #68618#: recognize the write of escher stream

      Revision 1.27  1999/08/31 07:51:34  JP
      set NumberFormatInt at streams to littleendian


      Rev 1.26   31 Aug 1999 09:51:34   JP
   set NumberFormatInt at streams to littleendian

      Rev 1.25   24 Aug 1999 20:15:50   JP
   Escher Export

      Rev 1.24   16 Aug 1999 17:16:28   JP
   W97 Export: experimental escher export (1)

      Rev 1.23   16 Aug 1999 13:05:44   JP
   write EscherObjects - only a testversion!

      Rev 1.22   21 Jul 1999 18:28:24   JP
   W97 Export: experimental escher export

      Rev 1.21   06 Jul 1999 14:54:00   JP
   W97-Export corrections

      Rev 1.20   30 Jun 1999 19:56:42   JP
   W97-Export: Fields/PostIts/References

      Rev 1.19   24 Jun 1999 22:52:18   JP
   Export of Bookmarks and Hyperlinks

      Rev 1.18   23 Jun 1999 20:14:40   JP
   W97-Export: write Bookmarks

      Rev 1.17   23 Jun 1999 12:46:16   JP
   Export of numbers und outlinenumbers

      Rev 1.16   16 Jun 1999 19:55:20   JP
   Change interface of base class Writer, Export of W97 NumRules

      Rev 1.15   04 Jun 1999 17:47:52   JP
   WW97-Export: Code aufgeteilt

      Rev 1.14   03 Jun 1999 21:04:10   JP
   Export von Seitenvorlagen verbessert: Abstaende/Umrandung/ErsteSeite/...

      Rev 1.13   02 Jun 1999 18:45:40   JP
   WinWord-Export um Kopf-/Fusszeilen erweitert

      Rev 1.12   31 May 1999 23:14:24   JP
   Erweiterungen/Umstellungen fuer Export

      Rev 1.11   31 May 1999 18:54:34   JP
   Bug #66446#,#66457#: Asserts entfernt

      Rev 1.10   28 May 1999 17:09:06   MPA
   WinWord97 Export

      Rev 1.9   27 May 1999 23:45:06   JP
   PiceTable eingebaut, Schnittstellen angepasst, usw.

      Rev 1.8   26 May 1999 16:44:28   MPA
   Anfaenge Piece-Table

      Rev 1.7   26 May 1999 09:39:30   JP
   OutStyleTab entfernt, OutFontTab mit Fib-Ref, WritePlc ohne Flag

      Rev 1.6   25 May 1999 17:00:10   MPA
   Anpassung PAPX/CHPX WW97

      Rev 1.5   21 May 1999 19:20:00   JP
   kleinere Korrekturen

      Rev 1.4   21 May 1999 17:01:00   MPA
   Anf,nge WW97

      Rev 1.3   21 May 1999 10:51:52   MPA
   SwAttrItr:CTor von Pointer nach Ref

      Rev 1.2   21 May 1999 02:00:50   JP
   Export von WW97 Format: allgemeine Methoden und FontTabelle

      Rev 1.1   19 May 1999 19:34:16   JP
   WinWord97-ExportFilter

      Rev 1.0   19 May 1999 11:20:02   JP
   WinWord97-ExportFilter

*************************************************************************/

