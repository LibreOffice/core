/*************************************************************************
 *
 *  $RCSfile: xihelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:01:28 $
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
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
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
    maString = rStrm.ReadRawUniString( nChars, b16Bit );

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
    for( XclFormatRunVec::iterator aIt = maFormats.begin(), aEnd = maFormats.end(); aIt != aEnd; ++aIt )
        rStrm >> aIt->mnChar >> aIt->mnFontIx;
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

XclImpHFConverter::XclImpHFPortionInfo::XclImpHFPortionInfo() :
    mnHeight( 0 ),
    mnMaxLineHt( 0 )
{
    maSel.nStartPara = maSel.nEndPara = 0;
    maSel.nStartPos = maSel.nEndPos = 0;
}

// ----------------------------------------------------------------------------

XclImpHFConverter::XclImpHFConverter( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mrEE( rRoot.GetHFEditEngine() ),
    mxFontData( new XclFontData ),
    meCurrObj( EXC_HF_CENTER )
{
}

XclImpHFConverter::~XclImpHFConverter()
{
}

void XclImpHFConverter::ParseString( const String& rHFString )
{
    // edit engine objects
    mrEE.SetText( EMPTY_STRING );
    maInfos.clear();
    maInfos.resize( EXC_HF_PORTION_COUNT );
    meCurrObj = EXC_HF_CENTER;

    // parser temporaries
    maCurrText.Erase();
    String aReadFont;           // current font name
    String aReadStyle;          // current font style
    sal_uInt16 nReadHeight = 0; // current font height
    ResetFontData();

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

                    case 'L':   SetNewPortion( EXC_HF_LEFT );   break;  // Left portion
                    case 'C':   SetNewPortion( EXC_HF_CENTER ); break;  // Center portion
                    case 'R':   SetNewPortion( EXC_HF_RIGHT );  break;  // Right portion

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
                        mxFontData->mnUnderline = (mxFontData->mnUnderline == EXC_FONTUNDERL_SINGLE) ?
                            EXC_FONTUNDERL_NONE : EXC_FONTUNDERL_SINGLE;
                    break;
                    case 'E':           // double underline
                        SetAttribs();
                        mxFontData->mnUnderline = (mxFontData->mnUnderline == EXC_FONTUNDERL_DOUBLE) ?
                            EXC_FONTUNDERL_NONE : EXC_FONTUNDERL_DOUBLE;
                    break;
                    case 'S':           // strikeout
                        SetAttribs();
                        mxFontData->mbStrikeout = !mxFontData->mbStrikeout;
                    break;
                    case 'X':           // superscript
                        SetAttribs();
                        mxFontData->mnEscapem = (mxFontData->mnEscapem == EXC_FONTESC_SUPER) ?
                            EXC_FONTESC_NONE : EXC_FONTESC_SUPER;
                    break;
                    case 'Y':           // subsrcipt
                        SetAttribs();
                        mxFontData->mnEscapem = (mxFontData->mnEscapem == EXC_FONTESC_SUB) ?
                            EXC_FONTESC_NONE : EXC_FONTESC_SUB;
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
                            mxFontData->maName = aReadFont;
                        mxFontData->maStyle = aReadStyle;
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
                        mxFontData->mnHeight = nReadHeight * 20;
                    }
                    --pChar;
                    eState = xlPSText;
                }
            }
            break;
        }
        ++pChar;
    }

    // finalize
    CreateCurrObject();
    maInfos[ EXC_HF_LEFT   ].mnHeight += GetMaxLineHeight( EXC_HF_LEFT );
    maInfos[ EXC_HF_CENTER ].mnHeight += GetMaxLineHeight( EXC_HF_CENTER );
    maInfos[ EXC_HF_RIGHT  ].mnHeight += GetMaxLineHeight( EXC_HF_RIGHT );
}

void XclImpHFConverter::FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nWhichId ) const
{
    ScPageHFItem aHFItem( nWhichId );
    if( maInfos[ EXC_HF_LEFT ].mxObj.get() )
        aHFItem.SetLeftArea( *maInfos[ EXC_HF_LEFT ].mxObj );
    if( maInfos[ EXC_HF_CENTER ].mxObj.get() )
        aHFItem.SetCenterArea( *maInfos[ EXC_HF_CENTER ].mxObj );
    if( maInfos[ EXC_HF_RIGHT ].mxObj.get() )
        aHFItem.SetRightArea( *maInfos[ EXC_HF_RIGHT ].mxObj );
    rItemSet.Put( aHFItem );
}

sal_Int32 XclImpHFConverter::GetTotalHeight() const
{
    return ::std::max( maInfos[ EXC_HF_LEFT ].mnHeight,
        ::std::max( maInfos[ EXC_HF_CENTER ].mnHeight, maInfos[ EXC_HF_RIGHT ].mnHeight ) );
}

// private --------------------------------------------------------------------

sal_uInt16 XclImpHFConverter::GetMaxLineHeight( XclImpHFPortion ePortion ) const
{
    sal_uInt16 nMaxHt = maInfos[ ePortion ].mnMaxLineHt;
    return (nMaxHt == 0) ? mxFontData->mnHeight : nMaxHt;
}

sal_uInt16 XclImpHFConverter::GetCurrMaxLineHeight() const
{
    return GetMaxLineHeight( meCurrObj );
}

void XclImpHFConverter::UpdateMaxLineHeight( XclImpHFPortion ePortion )
{
    sal_uInt16& rnMaxHt = maInfos[ ePortion ].mnMaxLineHt;
    rnMaxHt = ::std::max( rnMaxHt, mxFontData->mnHeight );
}

void XclImpHFConverter::UpdateCurrMaxLineHeight()
{
    UpdateMaxLineHeight( meCurrObj );
}

void XclImpHFConverter::SetAttribs()
{
    ESelection& rSel = GetCurrSel();
    if( (rSel.nStartPara != rSel.nEndPara) || (rSel.nStartPos != rSel.nEndPos) )
    {
        SfxItemSet aItemSet( mrEE.GetEmptyItemSet() );
        XclImpFont aFont( *this, *mxFontData );
        aFont.FillToItemSet( aItemSet, xlFontHFIDs );
        mrEE.QuickSetAttribs( aItemSet, rSel );
        rSel.nStartPara = rSel.nEndPara;
        rSel.nStartPos = rSel.nEndPos;
    }
}

void XclImpHFConverter::ResetFontData()
{
    if( const XclImpFont* pFirstFont = GetFontBuffer().GetFont( EXC_FONT_APP ) )
        *mxFontData = pFirstFont->GetFontData();
    else
    {
        mxFontData->Clear();
        mxFontData->mnHeight = 200;
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
        UpdateCurrMaxLineHeight();
    }
}

void XclImpHFConverter::InsertField( const SvxFieldItem& rFieldItem )
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertField( rFieldItem, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPos;
    UpdateCurrMaxLineHeight();
}

void XclImpHFConverter::InsertLineBreak()
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertText( String( '\n' ), ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPara;
    rSel.nEndPos = 0;
    GetCurrInfo().mnHeight += GetCurrMaxLineHeight();
    GetCurrInfo().mnMaxLineHt = 0;
}

void XclImpHFConverter::CreateCurrObject()
{
    InsertText();
    SetAttribs();
    GetCurrObj().reset( mrEE.CreateTextObject() );
}

void XclImpHFConverter::SetNewPortion( XclImpHFPortion eNew )
{
    if( eNew != meCurrObj )
    {
        CreateCurrObject();
        meCurrObj = eNew;
        if( GetCurrObj().get() )
            mrEE.SetText( *GetCurrObj() );
        else
            mrEE.SetText( EMPTY_STRING );
        ResetFontData();
    }
}


// URL conversion =============================================================

void XclImpUrlHelper::DecodeUrl(
        String& rUrl, String& rTabName, bool& rbSameWb,
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
                    case EXC_URL_RAW:
                    {
                        if( *(pChar + 1) )
                        {
                            xub_StrLen nLen = *++pChar;
                            for( xub_StrLen nChar = 0; (nChar < nLen) && *(pChar + 1); ++nChar )
                                rUrl.Append( *++pChar );
//                            rUrl.Append( ':' );
                        }
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
                rTabName.Append( *pChar );
            break;

// --- raw read mode ---

            case xlUrlRaw:
                rUrl.Append( *pChar );
            break;
        }

        ++pChar;
    }
}

void XclImpUrlHelper::DecodeUrl(
        String& rUrl, bool& rbSameWb, const XclImpRoot& rRoot, const String& rEncodedUrl )
{
    String aTabName;
    DecodeUrl( rUrl, aTabName, rbSameWb, rRoot, rEncodedUrl );
    DBG_ASSERT( !aTabName.Len(), "XclImpUrlHelper::DecodeUrl - sheet name ignored" );
}

bool XclImpUrlHelper::DecodeLink( String& rApplic, String& rTopic, const String rEncUrl )
{
    xub_StrLen nPos = rEncUrl.Search( EXC_DDE_DELIM );
    if( (nPos != STRING_NOTFOUND) && (0 < nPos) && (nPos + 1 < rEncUrl.Len()) )
    {
        rApplic = rEncUrl.Copy( 0, nPos );
        rTopic = rEncUrl.Copy( nPos + 1 );
        return true;
    }
    return false;
}


// Cached Values ==============================================================

XclImpCachedValue::XclImpCachedValue( XclImpStream& rStrm ) :
    mfValue( 0.0 ),
    mnBoolErr( 0 )
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
            mpStr.reset( new String( rStrm.ReadUniString() ) );
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            double fVal;
            rStrm.Ignore( 1 );
            rStrm >> mnBoolErr;
            rStrm.Ignore( 6 );

            const ScTokenArray* pScTokArr = rStrm.GetRoot().GetFmlaConverter().GetBoolErr(
                XclTools::ErrorToEnum( fVal, mnType == EXC_CACHEDVAL_ERROR, mnBoolErr ) );
            if( pScTokArr )
                mpTokArr.reset( pScTokArr->Clone() );
        }
        break;
        default:
            DBG_ERRORFILE( "XclImpCachedValue::XclImpCachedValue - unknown data type" );
    }
}

XclImpCachedValue::~XclImpCachedValue()
{
}

USHORT XclImpCachedValue::GetError() const
{
    return (mnType == EXC_CACHEDVAL_ERROR) ? XclTools::GetScErrorCode( mnBoolErr ) : 0;
}


// Matrix Cached Values ==============================================================

XclImpCachedMatrix::XclImpCachedMatrix( XclImpStream& rStrm ) :
    mnScCols( 0 ),
    mnScRows( 0 )
{
    mnScCols = rStrm.ReaduInt8();
    mnScRows = rStrm.ReaduInt16();

    if( rStrm.GetRoot().GetBiff() < xlBiff8 )
    {
        // in BIFF2-BIFF7: 256 columns represented by 0 columns
        if( mnScCols == 0 )
            mnScCols = 256;
    }
    else
    {
        // in BIFF8: columns and rows decreaed by 1
        ++mnScCols;
        ++mnScRows;
    }

    for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            maValueList.Append( new XclImpCachedValue( rStrm ) );
}

XclImpCachedMatrix::~XclImpCachedMatrix()
{
}

ScMatrixRef XclImpCachedMatrix::CreateScMatrix() const
{
    ScMatrixRef pScMatrix;
    DBG_ASSERT( mnScCols * mnScRows == maValueList.Count(), "XclImpCachedMatrix::CreateScMatrix - element count mismatch" );
    if( mnScCols && mnScRows && static_cast< sal_uInt32 >( mnScCols * mnScRows ) <= maValueList.Count() )
    {
        pScMatrix = new ScMatrix( mnScCols, mnScRows );
        const XclImpCachedValue* pValue = maValueList.First();
        for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        {
            for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            {
                switch( pValue->GetType() )
                {
                    case EXC_CACHEDVAL_EMPTY:
                        // Excel shows 0.0 here, not an empty cell
                        pScMatrix->PutDouble( 0.0, nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_DOUBLE:
                        pScMatrix->PutDouble( pValue->GetValue(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_STRING:
                        pScMatrix->PutString( pValue->GetString(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_BOOL:
                        pScMatrix->PutDouble( pValue->GetBool() ? 1.0 : 0.0, nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_ERROR:
                        pScMatrix->PutError( pValue->GetError(), nScCol, nScRow );
                    break;
                    default:
                        DBG_ERRORFILE( "XclImpCachedMatrix::CreateScMatrix - unknown value type" );
                        pScMatrix->PutEmpty( nScCol, nScRow );
                }
                pValue = maValueList.Next();
            }
        }
    }
    return pScMatrix;
}


// ============================================================================

