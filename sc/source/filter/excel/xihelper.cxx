/*************************************************************************
 *
 *  $RCSfile: xihelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:36:27 $
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

// ============================================================================

#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif

#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif


#include "excform.hxx"


// Byte/Unicode strings =======================================================

/** All allowed flags for import. */
const XclStrFlags nAllowedFlags = EXC_STR_8BITLENGTH | EXC_STR_SMARTFLAGS;


// ----------------------------------------------------------------------------

XclImpString::XclImpString( const String& rString ) :
    maString( rString )
{
}

XclImpString::XclImpString( XclImpStream& rStrm, XclStrFlags nFlags )
{
    DBG_ASSERT( (nFlags & ~nAllowedFlags) == 0, "XclImpString::XclImpString - unknown flag" );

    // --- string header ---
    sal_uInt16 nChars = ::get_flag( nFlags, EXC_STR_8BITLENGTH ) ? rStrm.ReaduInt8() : rStrm.ReaduInt16();
    sal_uInt8 nFlagField = 0;
    if( nChars || !::get_flag( nFlags, EXC_STR_SMARTFLAGS ) )
        rStrm >> nFlagField;

    bool b16Bit, bRich, bFarEast;
    sal_uInt16 nRunCount;
    sal_uInt32 nExtInf;
    rStrm.ReadUniStringExtHeader( b16Bit, bRich, bFarEast, nRunCount, nExtInf, nFlagField );

    // --- character array ---
    rStrm.AppendRawUniString( maString, nChars, b16Bit );

    // --- formatting ---
    ReadFormats( rStrm, nRunCount );

    // --- extended (FarEast) information ---
    rStrm.SkipUniStringExtData( nExtInf );
}

XclImpString::~XclImpString()
{
}

void XclImpString::AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIx )
{
    DBG_ASSERT( maFormats.empty() || ((maFormats.end() - 1)->mnChar < nChar), "XclImpString::AppendFormat - wrong char order" );
    maFormats.push_back( XclFormatRun( nChar, nFontIx ) );
}

void XclImpString::ReadFormats( XclImpStream& rStrm )
{
    ReadFormats( rStrm, rStrm.ReaduInt16() );
}

void XclImpString::ReadFormats( XclImpStream& rStrm, sal_uInt16 nRunCount )
{
    maFormats.resize( nRunCount );
    for( sal_uInt16 nIndex = 0; nIndex < nRunCount; ++nIndex )
        rStrm >> maFormats[ nIndex ];
}


// String->EditEngine conversion ==============================================

EditTextObject* XclImpStringHelper::CreateTextObject(
        const XclImpRoot& rRoot, const XclImpString& rString, sal_uInt32 nXFIndex )
{
    EditTextObject* pTextObj = NULL;

    const XclImpXFBuffer& rXFBuffer = rRoot.GetXFBuffer();
    bool bFirstEscaped = rXFBuffer.HasEscapement( nXFIndex );

    if( rString.IsRich() || bFirstEscaped )
    {
        const XclImpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
        const XclFormatRunVec& rFormats = rString.GetFormats();

        EditEngine& rEE = rRoot.GetEditEngine();
        rEE.SetText( rString.GetText() );

        SfxItemSet aItemSet( rEE.GetEmptyItemSet() );
        if( bFirstEscaped )
            rFontBuffer.FillToItemSet( aItemSet, xlFontEEIDs, rXFBuffer.GetFontIndex( nXFIndex ) );
        ESelection aSelection;

        XclFormatRun aNextRun;
        XclFormatRunVec::const_iterator aIter = rFormats.begin();
        XclFormatRunVec::const_iterator aEndIter = rFormats.end();

        if( aIter != aEndIter )
            aNextRun = *aIter++;
        else
            aNextRun.mnChar = 0xFFFF;

        xub_StrLen nLen = rString.GetText().Len();
        for( sal_uInt16 nChar = 0; nChar < nLen; ++nChar )
        {
            // reached new different formatted text portion
            if( nChar >= aNextRun.mnChar )
            {
                // send items to edit engine
                rEE.QuickSetAttribs( aItemSet, aSelection );

                // start new item set
                aItemSet.ClearItem();
                rFontBuffer.FillToItemSet( aItemSet, xlFontEEIDs, aNextRun.mnFontIx );

                // read new formatting information
                if( aIter != aEndIter )
                    aNextRun = *aIter++;
                else
                    aNextRun.mnChar = 0xFFFF;

                // reset selection start to current position
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos = aSelection.nEndPos;
            }

            // set end of selection to current position
            if( rString.GetText().GetChar( nChar ) == '\n' )
            {
                ++aSelection.nEndPara;
                aSelection.nEndPos = 0;
            }
            else
                ++aSelection.nEndPos;
        }

        // send items of last text portion to edit engine
        rEE.QuickSetAttribs( aItemSet, aSelection );

        pTextObj = rEE.CreateTextObject();
    }

    return pTextObj;
}

ScBaseCell* XclImpStringHelper::CreateCell(
        const XclImpRoot& rRoot, const XclImpString& rString, sal_uInt32 nXFIndex )
{
    ScBaseCell* pCell = NULL;

    if( rString.GetText().Len() )
    {
        ::std::auto_ptr< EditTextObject > pTextObj( CreateTextObject( rRoot, rString, nXFIndex ) );
        ScDocument& rDoc = rRoot.GetDoc();

        if( pTextObj.get() )
            // ScEditCell creates own copy of text object
            pCell = new ScEditCell( pTextObj.get(), &rDoc, rRoot.GetEditEngine().GetEditTextObjectPool() );
        else
            pCell = ScBaseCell::CreateTextCell( rString.GetText(), &rDoc );
    }

    return pCell;
}


// Header/footer conversion ===================================================

XclImpHFConverter::XclImpHFConverter( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mrEE( rRoot.GetHFEditEngine() ),
    mpFontData( new XclFontData )
{
}

void XclImpHFConverter::ParseString( const String& rHFString )
{
    // edit engine objects
    mrEE.SetText( EMPTY_STRING );
    mppObjs[ xlHFLeft ].reset( mrEE.CreateTextObject() );
    mppObjs[ xlHFCenter ].reset( mrEE.CreateTextObject() );
    mppObjs[ xlHFRight ].reset( mrEE.CreateTextObject() );
    mpSels[ xlHFLeft ].nStartPara = mpSels[ xlHFLeft ].nEndPara = 0;
    mpSels[ xlHFLeft ].nStartPos = mpSels[ xlHFLeft ].nEndPos = 0;
    mpSels[ xlHFRight ] = mpSels[ xlHFCenter ] = mpSels[ xlHFLeft ];
    meCurrObj = xlHFCenter;

    // parser temporaries
    maCurrText.Erase();
    String aReadFont;           // current font name
    String aReadStyle;          // current font style
    sal_uInt16 nReadHeight;     // current font height

    // font information
    const XclImpFont* pFirstFont = GetFontBuffer().GetFont( 0 );
    if( pFirstFont )
        *mpFontData = pFirstFont->GetFontData();
    else
    {
        mpFontData->Clear();
        mpFontData->mnHeight = 200;
    }

    /** State of the parser. */
    enum XclHFParserState
    {
        xlPSText,           /// Read text, search for functions.
        xlPSFunc,           /// Read function (token following a '&').
        xlPSFont,           /// Read font name ('&' is followed by '"', reads until next '"' or ',').
        xlPSFontStyle,      /// Read font style name (font part after ',', reads until next '"').
        xlPSHeight          /// Read font height ('&' is followed by num. digits, reads until non-digit).
    } eState = xlPSText;

    const sal_Unicode* pChar = rHFString.GetBuffer();
    const sal_Unicode* pNull = pChar + rHFString.Len(); // pointer to teminating null char
    while( *pChar )
    {
        switch( eState )
        {

// --- read text character ---

            case xlPSText:
            {
                switch( *pChar )
                {
                    case '&':           // new command
                        InsertText();
                        eState = xlPSFunc;
                    break;
                    case '\n':          // line break
                        InsertText();
                        InsertLineBreak();
                    break;
                    default:
                        maCurrText += *pChar;
                }
            }
            break;

// --- read control sequence ---

            case xlPSFunc:
            {
                eState = xlPSText;
                switch( *pChar )
                {
                    case '&':   maCurrText += '&';  break;  // the '&' character

                    case 'L':   SetNewPortion( xlHFLeft );      break;  // Left portion
                    case 'C':   SetNewPortion( xlHFCenter );    break;  // Center portion
                    case 'R':   SetNewPortion( xlHFRight );     break;  // Right portion

                    case 'P':   InsertField( SvxFieldItem( SvxPageField() ) );      break;  // page
                    case 'N':   InsertField( SvxFieldItem( SvxPagesField() ) );     break;  // page count
                    case 'D':   InsertField( SvxFieldItem( SvxDateField() ) );      break;  // date
                    case 'T':   InsertField( SvxFieldItem( SvxTimeField() ) );      break;  // time
                    case 'A':   InsertField( SvxFieldItem( SvxTableField() ) );     break;  // table name

                    case 'Z':           // file path
                        InsertField( SvxFieldItem( SvxExtFileField() ) );   // convert to full name
                        if( (pNull - pChar >= 2) && (*(pChar + 1) == '&') && (*(pChar + 2) == 'F') )
                        {
                            // &Z&F found - ignore the &F part
                            pChar += 2;
                        }
                    break;
                    case 'F':           // file name
                        InsertField( SvxFieldItem( SvxExtFileField( EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ) ) );
                    break;

                    case 'U':           // underline
                        SetAttribs();
                        mpFontData->meUnderline = (mpFontData->meUnderline == xlUnderlSingle) ?
                            xlUnderlNone : xlUnderlSingle;
                    break;
                    case 'E':           // double underline
                        SetAttribs();
                        mpFontData->meUnderline = (mpFontData->meUnderline == xlUnderlDouble) ?
                            xlUnderlNone : xlUnderlDouble;
                    break;
                    case 'S':           // strikeout
                        SetAttribs();
                        mpFontData->mbStrikeout = !mpFontData->mbStrikeout;
                    break;
                    case 'X':           // superscript
                        SetAttribs();
                        mpFontData->meEscapem = (mpFontData->meEscapem == xlEscSuper) ? xlEscNone : xlEscSuper;
                    break;
                    case 'Y':           // subsrcipt
                        SetAttribs();
                        mpFontData->meEscapem = (mpFontData->meEscapem == xlEscSub) ? xlEscNone : xlEscSub;
                    break;

                    case '\"':          // font name
                        aReadFont.Erase();
                        aReadStyle.Erase();
                        eState = xlPSFont;
                    break;
                    default:
                        if( ('0' <= *pChar) && (*pChar <= '9') )    // font size
                        {
                            nReadHeight = *pChar - '0';
                            eState = xlPSHeight;
                        }
                }
            }
            break;

// --- read font name ---

            case xlPSFont:
            {
                switch( *pChar )
                {
                    case '\"':
                        --pChar;
                        // run through
                    case ',':
                        eState = xlPSFontStyle;
                    break;
                    default:
                        aReadFont += *pChar;
                }
            }
            break;

// --- read font style ---

            case xlPSFontStyle:
            {
                switch( *pChar )
                {
                    case '\"':
                        SetAttribs();
                        if( aReadFont.Len() )
                            mpFontData->maName = aReadFont;
                        mpFontData->maStyle = aReadStyle;
                        eState = xlPSText;
                    break;
                    default:
                        aReadStyle += *pChar;
                }
            }
            break;

// --- read font height ---

            case xlPSHeight:
            {
                if( ('0' <= *pChar) && (*pChar <= '9') )
                {
                    if( nReadHeight != 0xFFFF )
                    {
                        nReadHeight *= 10;
                        nReadHeight += (*pChar - '0');
                        if( nReadHeight > 1600 )    // max 1600pt = 32000twips
                            nReadHeight = 0xFFFF;
                    }
                }
                else
                {
                    if( (nReadHeight != 0) && (nReadHeight != 0xFFFF) )
                    {
                        SetAttribs();
                        mpFontData->mnHeight = nReadHeight * 20;
                    }
                    --pChar;
                    eState = xlPSText;
                }
            }
            break;
        }
        ++pChar;
    }
    CreateCurrObject();
}

void XclImpHFConverter::SetAttribs()
{
    ESelection& rSel = GetCurrSel();
    if( (rSel.nStartPara != rSel.nEndPara) || (rSel.nStartPos != rSel.nEndPos) )
    {
        SfxItemSet aItemSet( mrEE.GetEmptyItemSet() );
        XclImpFont aFont( *this, *mpFontData );
        aFont.FillToItemSet( aItemSet, xlFontHFIDs );
        mrEE.QuickSetAttribs( aItemSet, rSel );
        rSel.nStartPara = rSel.nEndPara;
        rSel.nStartPos = rSel.nEndPos;
    }
}

void XclImpHFConverter::InsertText()
{
    if( maCurrText.Len() )
    {
        ESelection& rSel = GetCurrSel();
        mrEE.QuickInsertText( maCurrText, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
        rSel.nEndPos += maCurrText.Len();
        maCurrText.Erase();
    }
}

void XclImpHFConverter::InsertLineBreak()
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertText( String( '\n' ), ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPara;
    rSel.nEndPos = 0;
}

void XclImpHFConverter::InsertField( const SvxFieldItem& rFieldItem )
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertField( rFieldItem, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPos;
}

void XclImpHFConverter::CreateCurrObject()
{
    InsertText();
    SetAttribs();
    GetCurrObj().reset( mrEE.CreateTextObject() );
}

void XclImpHFConverter::SetNewPortion( XclHFPortion eNew )
{
    if( eNew != meCurrObj )
    {
        CreateCurrObject();
        meCurrObj = eNew;
        if( GetCurrObj().get() )
            mrEE.SetText( *GetCurrObj() );
        else
            mrEE.SetText( EMPTY_STRING );
    }
}


// URL conversion =============================================================

void XclImpUrlHelper::DecodeUrl(
        String& rUrl, String& rTable, bool& rbSameWb,
        const XclImpRoot& rRoot, const String& rEncodedUrl )
{
    enum
    {
        xlUrlInit,              /// Initial state, read string mode character.
        xlUrlPath,              /// Read URL path.
        xlUrlFileName,          /// Read file name.
        xlUrlSheetName,         /// Read sheet name.
        xlUrlRaw                /// Raw mode. No control characters will occur.
    } eState = xlUrlInit;

    bool bEncoded = true;
    rbSameWb = false;

    sal_Unicode cCurrDrive = 0;
    String aDosBase( INetURLObject( rRoot.GetBasePath() ).getFSysPath( INetURLObject::FSYS_DOS ) );
    if( (aDosBase.Len() > 2) && aDosBase.EqualsAscii( ":\\", 1, 2 ) )
        cCurrDrive = aDosBase.GetChar( 0 );

    const sal_Unicode* pChar = rEncodedUrl.GetBuffer();
    while( *pChar )
    {
        switch( eState )
        {

// --- first character ---

            case xlUrlInit:
            {
                switch( *pChar )
                {
                    case EXC_URLSTART_ENCODED:
                        eState = xlUrlPath;
                    break;
                    case EXC_URLSTART_SELF:
                    case EXC_URLSTART_SELFENCODED:
                        rbSameWb = true;
                        eState = xlUrlSheetName;
                    break;
                    default:
                        bEncoded = false;
                        rUrl.Append( *pChar );
                        eState = xlUrlPath;
                }
            }
            break;

// --- URL path ---

            case xlUrlPath:
            {
                switch( *pChar )
                {
                    case EXC_URL_DOSDRIVE:
                    {
                        if( *(pChar + 1) )
                        {
                            ++pChar;
                            if( *pChar == '@' )
                                rUrl.AppendAscii( "\\\\" );
                            else
                                rUrl.Append( *pChar ).AppendAscii( ":\\" );
                        }
                        else
                            rUrl.AppendAscii( "<NULL-DRIVE!>" );
                    }
                    break;
                    case EXC_URL_DRIVEROOT:
                        if( cCurrDrive )
                            rUrl.Append( cCurrDrive ).Append( ':' );
                        // run through
                    case EXC_URL_SUBDIR:
                        if( bEncoded )
                            rUrl.Append( '\\' );
                        else    // control character in raw name -> DDE link
                        {
                            rUrl.Append( EXC_DDE_DELIM );
                            eState = xlUrlRaw;
                        }
                    break;
                    case EXC_URL_PARENTDIR:
                        rUrl.AppendAscii( "..\\" );
                    break;
                    case EXC_URL_MACVOLUME:
                    {
                        if( *(pChar + 1) )
                        {
                            xub_StrLen nVolLen = *++pChar;
                            for( xub_StrLen nChar = 0; (nChar < nVolLen) && *(pChar + 1); ++nChar )
                                rUrl.Append( *++pChar );
                            rUrl.Append( ':' );
                        }
                        else
                            rUrl.AppendAscii( "<ERROR IN MAC-LONG-VOLUME>" );
                    }
                    break;
                    case '[':
                        eState = xlUrlFileName;
                    break;
                    default:
                        rUrl.Append( *pChar );
                }
            }
            break;

// --- file name ---

            case xlUrlFileName:
            {
                switch( *pChar )
                {
                    case ']':   eState = xlUrlSheetName;    break;
                    default:    rUrl.Append( *pChar );
                }
            }
            break;

// --- sheet name ---

            case xlUrlSheetName:
                rTable.Append( *pChar );
            break;

// --- raw read mode ---

            case xlUrlRaw:
                rUrl.Append( *pChar );
            break;
        }

        ++pChar;
    }
}


// Cached Values ==============================================================

XclImpCachedValue::XclImpCachedValue( XclImpStream& rStrm ) :
    mfValue( 0.0 )
{
    rStrm >> mnType;
    switch( mnType )
    {
        case EXC_CACHEDVAL_EMPTY:
            rStrm.Ignore( 8 );
        break;
        case EXC_CACHEDVAL_DOUBLE:
            rStrm >> mfValue;
        break;
        case EXC_CACHEDVAL_STRING:
            mpStr.reset( new String );
            rStrm.AppendUniString( *mpStr );
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            bool bIsErr = (mnType == EXC_CACHEDVAL_ERROR);
            sal_uInt8 nErrBool;
            double fVal;
            rStrm.Ignore( 1 );
            rStrm >> nErrBool;
            rStrm.Ignore( 6 );

            const ScTokenArray* pTok = rStrm.GetRoot().GetFmlaConverter().GetBoolErr(
                XclTools::ErrorToEnum( fVal, bIsErr, nErrBool ) );
            if( pTok )
                mpTokArr.reset( new ScTokenArray( *pTok ) );
        }
        break;
        default:
            DBG_ERRORFILE( "XclImpCachedValue::XclImpCachedValue - unknown data type" );
    }
}

XclImpCachedValue::~XclImpCachedValue()
{
}


// ============================================================================

