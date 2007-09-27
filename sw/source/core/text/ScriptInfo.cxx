/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptInfo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:11:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "errhdl.hxx" // ASSERT

#ifndef _SCRIPTINFO_HXX
#include <scriptinfo.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // GetCurFld
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_CHARHIDDENITEM_HXX
#include <svx/charhiddenitem.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif

#include <paratr.hxx>
#include <IDocumentRedlineAccess.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

#include <unicode/ubidi.h>

/*************************************************************************
 *                 lcl_IsLigature
 *
 * Checks if cCh + cNectCh builds a ligature (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_IsLigature( xub_Unicode cCh, xub_Unicode cNextCh )
{
            // Lam + Alef
    return ( 0x644 == cCh && 0x627 == cNextCh ) ||
            // Beh + Reh
           ( 0x628 == cCh && 0x631 == cNextCh );
}

/*************************************************************************
 *                 lcl_ConnectToPrev
 *
 * Checks if cCh is connectable to cPrevCh (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_ConnectToPrev( xub_Unicode cCh, xub_Unicode cPrevCh )
{
    // Alef, Dal, Thal, Reh, Zain, and Waw do not connect to the left
    // Uh, there seem to be some more characters that are not connectable
    // to the left. So we look for the characters that are actually connectable
    // to the left. Here is the complete list of WH:
    sal_Bool bRet = 0x628 == cPrevCh ||
                    ( 0x62A <= cPrevCh && cPrevCh <= 0x62E ) ||
                    ( 0x633 <= cPrevCh && cPrevCh <= 0x643 ) ||
                    ( 0x645 <= cPrevCh && cPrevCh <= 0x647 ) ||
                    0x64A == cPrevCh ||
                    ( 0x678 <= cPrevCh && cPrevCh <= 0x687 ) ||
                    ( 0x69A <= cPrevCh && cPrevCh <= 0x6B4 ) ||
                    ( 0x6B9 <= cPrevCh && cPrevCh <= 0x6C0 ) ||
                    ( 0x6C3 <= cPrevCh && cPrevCh <= 0x6D3 );

    // check for ligatures cPrevChar + cChar
    if ( bRet )
        bRet = ! lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}

/*************************************************************************
 *                      WhichFont()
 *
 * Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
 * Sw Script Types (SW_LATIN, SW_CJK, SW_CTL), used to identify the font
 *************************************************************************/

BYTE SwScriptInfo::WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI )
{
    ASSERT( pTxt || pSI,"How should I determine the script type?" );
    USHORT nScript;

    // First we try to use our SwScriptInfo
    if ( pSI )
        nScript = pSI->ScriptType( nIdx );
    else
        // Ok, we have to ask the break iterator
        nScript = pBreakIt->GetRealScriptOfText( *pTxt, nIdx );

    switch ( nScript ) {
        case i18n::ScriptType::LATIN : return SW_LATIN;
        case i18n::ScriptType::ASIAN : return SW_CJK;
        case i18n::ScriptType::COMPLEX : return SW_CTL;
    }

    ASSERT( sal_False, "Somebody tells lies about the script type!" );
    return SW_LATIN;
}

/*************************************************************************
 *                      SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode )
{
    InitScriptInfo( rNode, nDefaultDir == UBIDI_RTL );
}

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL )
{
    if( !pBreakIt->xBreak.is() )
        return;

    const String& rTxt = rNode.GetTxt();

    //
    // HIDDEN TEXT INFORMATION
    //
    Range aRange( 0, rTxt.Len() ? rTxt.Len() - 1 : 0 );
    MultiSelection aHiddenMulti( aRange );
    CalcHiddenRanges( rNode, aHiddenMulti );

    aHiddenChg.Remove( 0, aHiddenChg.Count() );
    USHORT nHiddenIdx = 0;
    USHORT i = 0;
    for( i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
    {
        const Range& rRange = aHiddenMulti.GetRange( i );
        const xub_StrLen nStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nEnd = (xub_StrLen)rRange.Max() + 1;

        aHiddenChg.Insert( nStart, nHiddenIdx++ );
        aHiddenChg.Insert( nEnd, nHiddenIdx++ );
    }

    //
    // SCRIPT AND SCRIPT RELATED INFORMATION
    //

    xub_StrLen nChg = nInvalidityPos;

    // STRING_LEN means the data structure is up to date
    nInvalidityPos = STRING_LEN;

    // this is the default direction
    nDefaultDir = bRTL ? UBIDI_RTL : UBIDI_LTR;

    // counter for script info arrays
    USHORT nCnt = 0;
    // counter for compression information arrays
    USHORT nCntComp = 0;
    // counter for kashida array
    USHORT nCntKash = 0;

    BYTE nScript;

    // compression type
    const SwCharCompressType aCompEnum = rNode.getIDocumentSettingAccess()->getCharacterCompressionType();

    // justification type
    const sal_Bool bAdjustBlock = SVX_ADJUST_BLOCK ==
                                  rNode.GetSwAttrSet().GetAdjust().GetAdjust();

    //
    // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:
    //

    if( nChg )
    {
        // if change position = 0 we do not use any data from the arrays
        // because by deleting all characters of the first group at the beginning
        // of a paragraph nScript is set to a wrong value
        ASSERT( CountScriptChg(), "Where're my changes of script?" );
        while( nCnt < CountScriptChg() )
        {
            if ( nChg > GetScriptChg( nCnt ) )
                nCnt++;
            else
            {
                nScript = GetScriptType( nCnt );
                break;
            }
        }
        if( CHARCOMPRESS_NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg > GetCompStart( nCntComp ) )
                    nCntComp++;
                else
                    break;
            }
        }
        if ( bAdjustBlock )
        {
            while( nCntKash < CountKashida() )
            {
                if ( nChg > GetKashida( nCntKash ) )
                    nCntKash++;
                else
                    break;
            }
        }
    }

    //
    // ADJUST nChg VALUE:
    //

    // by stepping back one position we know that we are inside a group
    // declared as an nScript group
    if ( nChg )
        --nChg;

    const xub_StrLen nGrpStart = nCnt ? GetScriptChg( nCnt - 1 ) : 0;

    // we go back in our group until we reach the first character of
    // type nScript
    while ( nChg > nGrpStart &&
            nScript != pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
        --nChg;

    // If we are at the start of a group, we do not trust nScript,
    // we better get nScript from the breakiterator:
    if ( nChg == nGrpStart )
        nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg );

    //
    // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:
    //

    // remove invalid entries from script information arrays
    const USHORT nScriptRemove = aScriptChg.Count() - nCnt;
    aScriptChg.Remove( nCnt, nScriptRemove );
    aScriptType.Remove( nCnt, nScriptRemove );

    // get the start of the last compression group
    USHORT nLastCompression = nChg;
    if( nCntComp )
    {
        --nCntComp;
        nLastCompression = GetCompStart( nCntComp );
        if( nChg >= nLastCompression + GetCompLen( nCntComp ) )
        {
            nLastCompression = nChg;
            ++nCntComp;
        }
    }

    // remove invalid entries from compression information arrays
    const USHORT nCompRemove = aCompChg.Count() - nCntComp;
    aCompChg.Remove( nCntComp, nCompRemove );
    aCompLen.Remove( nCntComp, nCompRemove );
    aCompType.Remove( nCntComp, nCompRemove );

    // get the start of the last kashida group
    USHORT nLastKashida = nChg;
    if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
    {
        --nCntKash;
        nLastKashida = GetKashida( nCntKash );
    }

    // remove invalid entries from kashida array
    aKashida.Remove( nCntKash, aKashida.Count() - nCntKash );

    //
    // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
    // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH
    //

    if( WEAK == pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
    {
        // If the beginning of the current group is weak, this means that
        // all of the characters in this grounp are weak. We have to assign
        // the scripts to these characters depending on the fonts which are
        // set for these characters to display them.
        xub_StrLen nEnd =
                (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, WEAK );

        if( nEnd > rTxt.Len() )
            nEnd = rTxt.Len();

        nScript = (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );

        ASSERT( i18n::ScriptType::LATIN == nScript ||
                i18n::ScriptType::ASIAN == nScript ||
                i18n::ScriptType::COMPLEX == nScript, "Wrong default language" );

        nChg = nEnd;

        // Get next script type or set to weak in order to exit
        BYTE nNextScript = ( nEnd < rTxt.Len() ) ?
           (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nEnd ) :
           (BYTE)WEAK;

        if ( nScript != nNextScript )
        {
            aScriptChg.Insert( nEnd, nCnt );
            aScriptType.Insert( nScript, nCnt++ );
            nScript = nNextScript;
        }
    }

    //
    // UPDATE THE SCRIPT INFO ARRAYS:
    //

    while ( nChg < rTxt.Len() || ( !aScriptChg.Count() && !rTxt.Len() ) )
    {
        ASSERT( i18n::ScriptType::WEAK != nScript,
                "Inserting WEAK into SwScriptInfo structure" );
        ASSERT( STRING_LEN != nChg, "65K? Strange length of script section" );

        nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );

        if ( nChg > rTxt.Len() )
            nChg = rTxt.Len();

        aScriptChg.Insert( nChg, nCnt );
        aScriptType.Insert( nScript, nCnt++ );

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CHARCOMPRESS_NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            BYTE ePrevState = NONE;
            BYTE eState;
            USHORT nPrevChg = nLastCompression;

            while ( nLastCompression < nChg )
            {
                xub_Unicode cChar = rTxt.GetChar( nLastCompression );

                // examine current character
                switch ( cChar )
                {
                // Left punctuation found
                case 0x3008: case 0x300A: case 0x300C: case 0x300E:
                case 0x3010: case 0x3014: case 0x3016: case 0x3018:
                case 0x301A: case 0x301D:
                    eState = SPECIAL_LEFT;
                    break;
                // Right punctuation found
                case 0x3001: case 0x3002: case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                    eState = SPECIAL_RIGHT;
                    break;
                default:
                    eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ?
                               KANA :
                               NONE;
                }

                // insert range of compressable characters
                if( ePrevState != eState )
                {
                    if ( ePrevState != NONE )
                    {
                        // insert start and type
                        if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                             ePrevState != KANA )
                        {
                            aCompChg.Insert( nPrevChg, nCntComp );
                            BYTE nTmpType = ePrevState;
                            aCompType.Insert( nTmpType, nCntComp );
                            aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                        }
                    }

                    ePrevState = eState;
                    nPrevChg = nLastCompression;
                }

                nLastCompression++;
            }

            // we still have to examine last entry
            if ( ePrevState != NONE )
            {
                // insert start and type
                if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                     ePrevState != KANA )
                {
                    aCompChg.Insert( nPrevChg, nCntComp );
                    BYTE nTmpType = ePrevState;
                    aCompType.Insert( nTmpType, nCntComp );
                    aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                }
            }
        }

        // we search for connecting opportunities (kashida)
        else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
        {
            SwScanner aScanner( rNode,
                                i18n::WordType::DICTIONARY_WORD,
                                nLastKashida, nChg );

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const XubString& rWord = aScanner.GetWord();

                xub_StrLen nIdx = 0;
                xub_StrLen nKashidaPos = STRING_LEN;
                xub_Unicode cCh;
                xub_Unicode cPrevCh = 0;

                while ( nIdx < rWord.Len() )
                {
                    cCh = rWord.GetChar( nIdx );

                    // 1. Priority:
                    // after user inserted kashida
                    if ( 0x640 == cCh )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        break;
                    }

                    // 2. Priority:
                    // after a Seen or Sad
                    if ( nIdx + 1 < rWord.Len() &&
                         ( 0x633 == cCh || 0x635 == cCh ) )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        break;
                    }

                    // 3. Priority:
                    // before final form of Teh Marbuta, Hah, Dal
                    // 4. Priority:
                    // before final form of Alef, Lam or Kaf
                    if ( nIdx && nIdx + 1 == rWord.Len() &&
                         ( 0x629 == cCh || 0x62D == cCh || 0x62F == cCh ||
                           0x627 == cCh || 0x644 == cCh || 0x643 == cCh ) )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if character is connectable to previous character,
                        if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                        {
                            nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                            break;
                        }
                    }

                    // 5. Priority:
                    // before media Bah
                    if ( nIdx && nIdx + 1 < rWord.Len() && 0x628 == cCh )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if next character is Reh, Yeh or Alef Maksura
                        xub_Unicode cNextCh = rWord.GetChar( nIdx + 1 );

                        if ( 0x631 == cNextCh || 0x64A == cNextCh ||
                             0x649 == cNextCh )
                        {
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                        }
                    }

                    // 6. Priority:
                    // other connecting possibilities
                    if ( nIdx && nIdx + 1 == rWord.Len() &&
                         0x60C <= cCh && 0x6FE >= cCh )
                    {
                        ASSERT( 0 != cPrevCh, "No previous character" )

                        // check if character is connectable to previous character,
                        if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                        {
                            // only choose this position if we did not find
                            // a better one:
                            if ( STRING_LEN == nKashidaPos )
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                            break;
                        }
                    }

                    // Do not consider Fathatan, Dammatan, Kasratan, Fatha,
                    // Damma, Kasra, Shadda and Sukun when checking if
                    // a character can be connected to previous character.
                    if ( cCh < 0x64B || cCh > 0x652 )
                        cPrevCh = cCh;

                    ++nIdx;
                } // end of current word

                if ( STRING_LEN != nKashidaPos )
                    aKashida.Insert( nKashidaPos, nCntKash++ );
            } // end of kashida search
        }

        if ( nChg < rTxt.Len() )
            nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg );

        nLastCompression = nChg;
        nLastKashida = nChg;
    };

#ifndef PRODUCT
    // check kashida data
    long nTmpKashidaPos = -1;
    sal_Bool bWrongKash = sal_False;
    for (i = 0; i < aKashida.Count(); ++i )
    {
        long nCurrKashidaPos = GetKashida( i );
        if ( nCurrKashidaPos <= nTmpKashidaPos )
        {
            bWrongKash = sal_True;
            break;
        }
        nTmpKashidaPos = nCurrKashidaPos;
    }
    ASSERT( ! bWrongKash, "Kashida array contains wrong data" )
#endif

    // remove invalid entries from direction information arrays
    const USHORT nDirRemove = aDirChg.Count();
    aDirChg.Remove( 0, nDirRemove );
    aDirType.Remove( 0, nDirRemove );

    // Perform Unicode Bidi Algorithm for text direction information
    bool bPerformUBA = UBIDI_LTR != nDefaultDir;
    nCnt = 0;
    while( !bPerformUBA && nCnt < CountScriptChg() )
    {
        if ( i18n::ScriptType::COMPLEX == GetScriptType( nCnt++ ) )
            bPerformUBA = true;
    }

    // do not call the unicode bidi algorithm if not required
    if ( bPerformUBA )
    {
        UpdateBidiInfo( rTxt );

        // #i16354# Change script type for RTL text to CTL.
        for ( USHORT nDirIdx = 0; nDirIdx < aDirChg.Count(); ++nDirIdx )
        {
            if ( GetDirType( nDirIdx ) == UBIDI_RTL )
            {
                // nStart ist start of RTL run:
                const xub_StrLen nStart = nDirIdx > 0 ? GetDirChg( nDirIdx - 1 ) : 0;
                // nEnd is end of RTL run:
                const xub_StrLen nEnd = GetDirChg( nDirIdx );
                // nScriptIdx points into the ScriptArrays:
                USHORT nScriptIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                // Make nScriptIdx become the index of the script group with
                // 1. nStartPosOfGroup <= nStart and
                // 2. nEndPosOfGroup > nStart
                while ( GetScriptChg( nScriptIdx ) <= nStart )
                    ++nScriptIdx;

                xub_StrLen nEndPosOfGroup = GetScriptChg( nScriptIdx );
                xub_StrLen nStartPosOfGroup = nScriptIdx ? GetScriptChg( nScriptIdx - 1 ) : 0;
                BYTE nScriptTypeOfGroup = GetScriptType( nScriptIdx );

                ASSERT( nStartPosOfGroup <= nStart && nEndPosOfGroup > nStart,
                        "Script override with CTL font trouble" )

                // Check if we have to insert a new script change at
                // position nStart. If nStartPosOfGroup < nStart,
                // we have to insert a new script change:
                if ( nStart > 0 && nStartPosOfGroup < nStart )
                {
                    aScriptChg.Insert( nStart, nScriptIdx );
                    aScriptType.Insert( nScriptTypeOfGroup, nScriptIdx );
                    ++nScriptIdx;
                }

                // Remove entries in ScriptArray which end inside the RTL run:
                while ( nScriptIdx < aScriptChg.Count() && GetScriptChg( nScriptIdx ) <= nEnd )
                {
                    aScriptChg.Remove( nScriptIdx, 1 );
                    aScriptType.Remove( nScriptIdx, 1 );
                }

                // Insert a new entry in ScriptArray for the end of the RTL run:
                aScriptChg.Insert( nEnd, nScriptIdx );
                aScriptType.Insert( i18n::ScriptType::COMPLEX, nScriptIdx );

#if OSL_DEBUG_LEVEL > 1
                BYTE nScriptType;
                BYTE nLastScriptType = i18n::ScriptType::WEAK;
                xub_StrLen nScriptChg;
                xub_StrLen nLastScriptChg = 0;

                for ( int i = 0; i < aScriptChg.Count(); ++i )
                {
                    nScriptChg = GetScriptChg( i );
                    nScriptType = GetScriptType( i );
                    ASSERT( nLastScriptType != nScriptType &&
                            nLastScriptChg < nScriptChg,
                            "Heavy InitScriptType() confusion" )
                }
#endif
            }
        }
    }
}

void SwScriptInfo::UpdateBidiInfo( const String& rTxt )
{
    // remove invalid entries from direction information arrays
    const USHORT nDirRemove = aDirChg.Count();
    aDirChg.Remove( 0, nDirRemove );
    aDirType.Remove( 0, nDirRemove );

    //
    // Bidi functions from icu 2.0
    //
    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( rTxt.Len(), 0, &nError );
    nError = U_ZERO_ERROR;

    ubidi_setPara( pBidi, rTxt.GetBuffer(), rTxt.Len(),
                   nDefaultDir, NULL, &nError );
    nError = U_ZERO_ERROR;
    long nCount = ubidi_countRuns( pBidi, &nError );
    int32_t nStart = 0;
    int32_t nEnd;
    UBiDiLevel nCurrDir;
    // counter for direction information arrays
    USHORT nCntDir = 0;

    for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
    {
        ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
        aDirChg.Insert( (USHORT)nEnd, nCntDir );
        aDirType.Insert( (BYTE)nCurrDir, nCntDir++ );
        nStart = nEnd;
    }

    ubidi_close( pBidi );
}


/*************************************************************************
 *                        SwScriptInfo::NextScriptChg(..)
 * returns the position of the next character which belongs to another script
 * than the character of the actual (input) position.
 * If there's no script change until the end of the paragraph, it will return
 * STRING_LEN.
 * Scripts are Asian (Chinese, Japanese, Korean),
 *             Latin ( English etc.)
 *         and Complex ( Hebrew, Arabian )
 *************************************************************************/

xub_StrLen SwScriptInfo::NextScriptChg( const xub_StrLen nPos )  const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    }

    return STRING_LEN;
}

/*************************************************************************
 *                        SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

BYTE SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    // the default is the application language script
    return (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
}

xub_StrLen SwScriptInfo::NextDirChg( const xub_StrLen nPos,
                                     const BYTE* pLevel )  const
{
    BYTE nCurrDir = pLevel ? *pLevel : 62;
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) &&
            ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
            return GetDirChg( nX );
    }

    return STRING_LEN;
}

BYTE SwScriptInfo::DirType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) )
            return GetDirType( nX );
    }

    return 0;
}

/*************************************************************************
 *                        SwScriptInfo::MaskHiddenRanges(..)
 * Takes a string and replaced the hidden ranges with cChar.
 **************************************************************************/

USHORT SwScriptInfo::MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
                                       const xub_StrLen nStt, const xub_StrLen nEnd,
                                       const xub_Unicode cChar )
{
    ASSERT( rNode.GetTxt().Len() == rText.Len(), "MaskHiddenRanges, string len mismatch" )

    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    USHORT nNumOfHiddenChars = 0;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        if ( nHiddenEnd < nStt || nHiddenStart > nEnd )
            continue;

        while ( nHiddenStart < nHiddenEnd && nHiddenStart < nEnd )
        {
            if ( nHiddenStart >= nStt && nHiddenStart < nEnd )
            {
                rText.SetChar( nHiddenStart, cChar );
                ++nNumOfHiddenChars;
            }
            ++nHiddenStart;
        }
    }

    return nNumOfHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos,
                                           xub_StrLen& rnStartPos, xub_StrLen& rnEndPos,
                                           PositionList* pList )
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    bool bNewContainsHiddenChars = false;

    //
    // Optimization: First examine the flags at the text node:
    //
    if ( !rNode.IsCalcHiddenCharFlags() )
    {
        bool bWholePara = rNode.HasHiddenCharAttribute( true );
        bool bContainsHiddenChars = rNode.HasHiddenCharAttribute( false );
        if ( !bContainsHiddenChars )
            return false;

        if ( bWholePara )
        {
            if ( pList )
            {
                pList->push_back( 0 );
                pList->push_back( rNode.GetTxt().Len() );
            }

            rnStartPos = 0;
            rnEndPos = rNode.GetTxt().Len();
            return true;
        }
    }

    const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rNode );
    if ( pSI )
    {
        //
        // Check first, if we have a valid SwScriptInfo object for this text node:
        //
        bNewContainsHiddenChars = pSI->GetBoundsOfHiddenRange( nPos, rnStartPos, rnEndPos, pList );
        const bool bNewHiddenCharsHidePara = ( rnStartPos == 0 && rnEndPos >= rNode.GetTxt().Len() );
        rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
    }
    else
    {
        //
        // No valid SwScriptInfo Object, we have to do it the hard way:
        //
        Range aRange( 0, rNode.GetTxt().Len() ? rNode.GetTxt().Len() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        SwScriptInfo::CalcHiddenRanges( rNode, aHiddenMulti );
        for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
            const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;

            if ( nHiddenStart > nPos )
                break;
            else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
            {
                rnStartPos = nHiddenStart;
                rnEndPos   = Min( nHiddenEnd, rNode.GetTxt().Len() );
                break;
            }
        }

        if ( pList )
        {
            for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                pList->push_back( (xub_StrLen)rRange.Min() );
                pList->push_back( (xub_StrLen)rRange.Max() + 1 );
            }
        }

        bNewContainsHiddenChars = aHiddenMulti.GetRangeCount() > 0;
    }

    return bNewContainsHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * non-static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( xub_StrLen nPos, xub_StrLen& rnStartPos,
                                           xub_StrLen& rnEndPos, PositionList* pList ) const
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    USHORT nEnd = CountHiddenChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        const xub_StrLen nHiddenStart = GetHiddenChg( nX++ );
        const xub_StrLen nHiddenEnd = GetHiddenChg( nX );

        if ( nHiddenStart > nPos )
            break;
        else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
        {
            rnStartPos = nHiddenStart;
            rnEndPos   = nHiddenEnd;
            break;
        }
    }

    if ( pList )
    {
        for( USHORT nX = 0; nX < nEnd; ++nX )
        {
            pList->push_back( GetHiddenChg( nX++ ) );
            pList->push_back( GetHiddenChg( nX ) );
        }
    }

    return CountHiddenChg() > 0;
}

/*************************************************************************
 *                        SwScriptInfo::IsInHiddenRange()
 **************************************************************************/

bool SwScriptInfo::IsInHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos )
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nPos, nStartPos, nEndPos );
    return nStartPos != STRING_LEN;
}


#if OSL_DEBUG_LEVEL > 1
/*************************************************************************
 *                        SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/

BYTE SwScriptInfo::CompType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountCompChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        xub_StrLen nChg = GetCompStart( nX );

        if ( nPos < nChg )
            return NONE;

        if( nPos < nChg + GetCompLen( nX ) )
            return GetCompType( nX );
    }
    return NONE;
}
#endif

/*************************************************************************
 *                      SwScriptInfo::HasKana()
 * returns, if there are compressable kanas or specials
 * betwenn nStart and nEnd
 *************************************************************************/

USHORT SwScriptInfo::HasKana( xub_StrLen nStart, const xub_StrLen nLen ) const
{
    USHORT nCnt = CountCompChg();
    xub_StrLen nEnd = nStart + nLen;

    for( USHORT nX = 0; nX < nCnt; ++nX )
    {
        xub_StrLen nKanaStart  = GetCompStart( nX );
        xub_StrLen nKanaEnd = nKanaStart + GetCompLen( nX );

        if ( nKanaStart >= nEnd )
            return USHRT_MAX;

        if ( nStart < nKanaEnd )
            return nX;
    }

    return USHRT_MAX;
}

/*************************************************************************
 *                      SwScriptInfo::Compress()
 *************************************************************************/

long SwScriptInfo::Compress( long* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                             const USHORT nCompress, const USHORT nFontHeight,
                             Point* pPoint ) const
{
    ASSERT( nCompress, "Compression without compression?!" );
    ASSERT( nLen, "Compression without text?!" );
    USHORT nCompCount = CountCompChg();

    // In asian typography, there are full width and half width characters.
    // Full width punctuation characters can be compressed by 50 %
    // to determine this, we compare the font width with 75 % of its height
    USHORT nMinWidth = ( 3 * nFontHeight ) / 4;

    USHORT nCompIdx = HasKana( nIdx, nLen );

    if ( USHRT_MAX == nCompIdx )
        return 0;

    xub_StrLen nChg = GetCompStart( nCompIdx );
    xub_StrLen nCompLen = GetCompLen( nCompIdx );
    USHORT nI = 0;
    nLen += nIdx;

    if( nChg > nIdx )
    {
        nI = nChg - nIdx;
        nIdx = nChg;
    }
    else if( nIdx < nChg + nCompLen )
        nCompLen -= nIdx - nChg;

    if( nIdx > nLen || nCompIdx >= nCompCount )
        return 0;

    long nSub = 0;
    long nLast = nI ? pKernArray[ nI - 1 ] : 0;
    do
    {
        USHORT nType = GetCompType( nCompIdx );
#if OSL_DEBUG_LEVEL > 1
        ASSERT( nType == CompType( nIdx ), "Gimme the right type!" );
#endif
        nCompLen += nIdx;
        if( nCompLen > nLen )
            nCompLen = nLen;

        // are we allowed to compress the character?
        if ( pKernArray[ nI ] - nLast < nMinWidth )
        {
            nIdx++; nI++;
        }
        else
        {
            while( nIdx < nCompLen )
            {
                ASSERT( SwScriptInfo::NONE != nType, "None compression?!" );

                // nLast is width of current character
                nLast -= pKernArray[ nI ];

                nLast *= nCompress;
                long nMove = 0;
                if( SwScriptInfo::KANA != nType )
                {
                    nLast /= 20000;
                    if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
                    {
                        if( nI )
                            nMove = nLast;
                        else
                        {
                            pPoint->X() += nLast;
                            nLast = 0;
                        }
                    }
                }
                else
                    nLast /= 100000;
                nSub -= nLast;
                nLast = pKernArray[ nI ];
                if( nMove )
                    pKernArray[ nI - 1 ] += nMove;
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }

        if( nIdx < nLen )
        {
            xub_StrLen nChg;
            if( ++nCompIdx < nCompCount )
            {
                nChg = GetCompStart( nCompIdx );
                if( nChg > nLen )
                    nChg = nLen;
                nCompLen = GetCompLen( nCompIdx );
            }
            else
                nChg = nLen;
            while( nIdx < nChg )
            {
                nLast = pKernArray[ nI ];
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }
        else
            break;
    } while( nIdx < nLen );
    return nSub;
}

/*************************************************************************
 *                      SwScriptInfo::KashidaJustify()
 *************************************************************************/

USHORT SwScriptInfo::KashidaJustify( long* pKernArray, long* pScrArray,
                                     xub_StrLen nStt, xub_StrLen nLen,
                                     long nSpaceAdd ) const
{
    ASSERT( nLen, "Kashida justification without text?!" )

    // evaluate kashida informatin in collected in SwScriptInfo

    USHORT nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    if ( ! pKernArray )
    {
        USHORT nCntKashEnd = nCntKash;
        while ( nCntKashEnd < CountKashida() )
        {
            if ( nEnd <= GetKashida( nCntKashEnd ) )
                break;
            else
                nCntKashEnd++;
        }

        return nCntKashEnd - nCntKash;
    }

    // do nothing if there is no more kashida
    if ( nCntKash < CountKashida() )
    {
        xub_StrLen nKashidaPos = GetKashida( nCntKash );
        xub_StrLen nIdx = nKashidaPos;
        long nKashAdd = nSpaceAdd;

        while ( nIdx < nEnd )
        {
            USHORT nArrayPos = nIdx - nStt;

            // next kashida position
            nIdx = ++nCntKash  < CountKashida() ? GetKashida( nCntKash ) : nEnd;
            if ( nIdx > nEnd )
                nIdx = nEnd;

            const USHORT nArrayEnd = nIdx - nStt;

            while ( nArrayPos < nArrayEnd )
            {
                pKernArray[ nArrayPos ] += nKashAdd;
                if ( pScrArray )
                   pScrArray[ nArrayPos ] += nKashAdd;
                ++nArrayPos;
            }

            nKashAdd += nSpaceAdd;
        }
    }

    return 0;
}

/*************************************************************************
 *                      SwScriptInfo::IsArabicLanguage()
 *************************************************************************/

sal_Bool SwScriptInfo::IsArabicLanguage( LanguageType aLang )
{
    return LANGUAGE_ARABIC == aLang || LANGUAGE_ARABIC_SAUDI_ARABIA == aLang ||
           LANGUAGE_ARABIC_IRAQ == aLang || LANGUAGE_ARABIC_EGYPT == aLang ||
           LANGUAGE_ARABIC_LIBYA == aLang || LANGUAGE_ARABIC_ALGERIA == aLang ||
           LANGUAGE_ARABIC_MOROCCO == aLang || LANGUAGE_ARABIC_TUNISIA == aLang ||
           LANGUAGE_ARABIC_OMAN == aLang || LANGUAGE_ARABIC_YEMEN == aLang ||
           LANGUAGE_ARABIC_SYRIA == aLang || LANGUAGE_ARABIC_JORDAN == aLang ||
           LANGUAGE_ARABIC_LEBANON == aLang || LANGUAGE_ARABIC_KUWAIT == aLang ||
           LANGUAGE_ARABIC_UAE == aLang || LANGUAGE_ARABIC_BAHRAIN == aLang ||
           LANGUAGE_ARABIC_QATAR == aLang;
}

/*************************************************************************
 *                      SwScriptInfo::ThaiJustify()
 *************************************************************************/

USHORT SwScriptInfo::ThaiJustify( const XubString& rTxt, long* pKernArray,
                                  long* pScrArray, xub_StrLen nStt,
                                  xub_StrLen nLen, xub_StrLen nNumberOfBlanks,
                                  long nSpaceAdd )
{
    ASSERT( nStt + nLen <= rTxt.Len(), "String in ThaiJustify too small" )

    SwTwips nNumOfTwipsToDistribute = nSpaceAdd * nNumberOfBlanks /
                                      SPACING_PRECISION_FACTOR;

    long nSpaceSum = 0;
    USHORT nCnt = 0;

    for ( USHORT nI = 0; nI < nLen; ++nI )
    {
        const xub_Unicode cCh = rTxt.GetChar( nStt + nI );

        // check if character is not above or below base
        if ( ( 0xE34 > cCh || cCh > 0xE3A ) &&
             ( 0xE47 > cCh || cCh > 0xE4E ) && cCh != 0xE31 )
        {
            if ( nNumberOfBlanks > 0 )
            {
                nSpaceAdd = nNumOfTwipsToDistribute / nNumberOfBlanks;
                --nNumberOfBlanks;
                nNumOfTwipsToDistribute -= nSpaceAdd;
            }
            nSpaceSum += nSpaceAdd;
            ++nCnt;
        }

        if ( pKernArray ) pKernArray[ nI ] += nSpaceSum;
        if ( pScrArray ) pScrArray[ nI ] += nSpaceSum;
    }

    return nCnt;
}

/*************************************************************************
 *                      SwScriptInfo::GetScriptInfo()
 *************************************************************************/

SwScriptInfo* SwScriptInfo::GetScriptInfo( const SwTxtNode& rTNd,
                                           sal_Bool bAllowInvalid )
{
    SwClientIter aClientIter( (SwTxtNode&)rTNd );
    SwClient* pLast = aClientIter.GoStart();
    SwScriptInfo* pScriptInfo = 0;

    while( pLast )
    {
        if ( pLast->ISA( SwTxtFrm ) )
        {
            pScriptInfo = (SwScriptInfo*)((SwTxtFrm*)pLast)->GetScriptInfo();
            if ( pScriptInfo )
            {
                if ( !bAllowInvalid && STRING_LEN != pScriptInfo->GetInvalidity() )
                    pScriptInfo = 0;
                else break;
            }
        }
        pLast = ++aClientIter;
    }

    return pScriptInfo;
}

/*************************************************************************
 *                      SwScriptInfo::CalcHiddenRanges()
 *
 * Returns a MultiSection indicating the hidden ranges.
 *************************************************************************/

void SwScriptInfo::CalcHiddenRanges( const SwTxtNode& rNode, MultiSelection& rHiddenMulti )
{
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState( RES_CHRATR_HIDDEN, TRUE, &pItem ) &&
        ((SvxCharHiddenItem*)pItem)->GetValue() )
    {
        rHiddenMulti.SelectAll();
    }

    const SwpHints* pHints = rNode.GetpSwpHints();
    const SwTxtAttr* pTxtAttr = 0;
    bool bHidden = false;
    bool bHiddenSelect = false;

    if( pHints )
    {
        USHORT nTmp = 0;

        while( nTmp < pHints->GetStartCount() )
        {
            pTxtAttr = pHints->GetStart( nTmp++ );
            switch ( pTxtAttr->Which() )
            {
                case RES_CHRATR_HIDDEN:
                {
                    bHidden = sal_True;
                    bHiddenSelect = pTxtAttr->GetCharHidden().GetValue();
                }
                break;
                case RES_TXTATR_CHARFMT:
                {
                    SwCharFmt* pFmt;
                    const SfxPoolItem* pItem;
                    pFmt = pTxtAttr->GetCharFmt().GetCharFmt();
                    if ( pFmt )
                    {
                        if( SFX_ITEM_SET == pFmt->GetAttrSet().
                            GetItemState( RES_CHRATR_HIDDEN, sal_True, &pItem ) )
                        {
                            bHidden = sal_True;
                            bHiddenSelect = ((SvxCharHiddenItem*)pItem)->GetValue();
                        }
                    }
                }
                break;
            }
            if( bHidden )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    if( bHidden )
                        rHiddenMulti.Select( aTmp, bHiddenSelect );
                }
                bHidden = sal_False;
            }
        }
    }

    // If there are any hidden ranges in the current text node, we have
    // to unhide the redlining ranges:
    const IDocumentRedlineAccess& rIDRA = *rNode.getIDocumentRedlineAccess();
    if ( rHiddenMulti.GetRangeCount() && IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
    {
        USHORT nAct = rIDRA.GetRedlinePos( rNode, USHRT_MAX );

        for ( ; nAct < rIDRA.GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = rIDRA.GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > rNode.GetIndex() )
                break;

            xub_StrLen nRedlStart;
            xub_StrLen nRedlnEnd;
            pRed->CalcStartEnd( rNode.GetIndex(), nRedlStart, nRedlnEnd );
            if ( nRedlnEnd > nRedlStart )
            {
                Range aTmp( nRedlStart, nRedlnEnd - 1 );
                rHiddenMulti.Select( aTmp, false );
            }
        }
    }

    //
    // We calculated a lot of stuff. Finally we can update the flags at the text node.
    //
    const bool bNewContainsHiddenChars = rHiddenMulti.GetRangeCount() > 0;
    bool bNewHiddenCharsHidePara = false;
    if ( bNewContainsHiddenChars )
    {
        const Range& rRange = rHiddenMulti.GetRange( 0 );
        const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;
        bNewHiddenCharsHidePara = ( nHiddenStart == 0 && nHiddenEnd >= rNode.GetTxt().Len() );
    }
    rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
}

