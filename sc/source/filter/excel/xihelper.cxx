/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "xihelper.hxx"
#include <svl/itemset.hxx>
#include <svl/sharedstringpool.hxx>
#include <editeng/editobj.hxx>
#include <tools/urlobj.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include "document.hxx"
#include "rangelst.hxx"
#include "editutil.hxx"
#include "attrib.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xistyle.hxx"
#include "excform.hxx"
#include "stringutil.hxx"
#include "scmatrix.hxx"
#include "documentimport.hxx"
#include <o3tl/make_unique.hxx>

// Excel->Calc cell address/range conversion ==================================

namespace {

/** Fills the passed Calc address with the passed Excel cell coordinates without checking any limits. */
inline void lclFillAddress( ScAddress& rScPos, sal_uInt16 nXclCol, sal_uInt32 nXclRow, SCTAB nScTab )
{
    rScPos.SetCol( static_cast< SCCOL >( nXclCol ) );
    rScPos.SetRow( static_cast< SCROW >( nXclRow ) );
    rScPos.SetTab( nScTab );
}

} // namespace

XclImpAddressConverter::XclImpAddressConverter( const XclImpRoot& rRoot ) :
    XclAddressConverterBase( rRoot.GetTracer(), rRoot.GetScMaxPos() )
{
}

// cell address ---------------------------------------------------------------

bool XclImpAddressConverter::CheckAddress( const XclAddress& rXclPos, bool bWarn )
{
    bool bValidCol = rXclPos.mnCol <= mnMaxCol;
    bool bValidRow = rXclPos.mnRow <= mnMaxRow;
    bool bValid = bValidCol && bValidRow;
    if( !bValid && bWarn )
    {
        mbColTrunc |= !bValidCol;
        mbRowTrunc |= !bValidRow;
        mrTracer.TraceInvalidAddress( ScAddress(
            static_cast< SCCOL >( rXclPos.mnCol ), static_cast< SCROW >( rXclPos.mnRow ), 0 ), maMaxPos );
    }
    return bValid;
}

bool XclImpAddressConverter::ConvertAddress( ScAddress& rScPos,
        const XclAddress& rXclPos, SCTAB nScTab, bool bWarn )
{
    bool bValid = CheckAddress( rXclPos, bWarn );
    if( bValid )
        lclFillAddress( rScPos, rXclPos.mnCol, rXclPos.mnRow, nScTab );
    return bValid;
}

ScAddress XclImpAddressConverter::CreateValidAddress(
        const XclAddress& rXclPos, SCTAB nScTab, bool bWarn )
{
    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( !ConvertAddress( aScPos, rXclPos, nScTab, bWarn ) )
    {
        aScPos.SetCol( static_cast< SCCOL >( ::std::min( rXclPos.mnCol, mnMaxCol ) ) );
        aScPos.SetRow( static_cast< SCROW >( ::std::min( rXclPos.mnRow, mnMaxRow ) ) );
        aScPos.SetTab( limit_cast< SCTAB >( nScTab, 0, maMaxPos.Tab() ) );
    }
    return aScPos;
}

// cell range -----------------------------------------------------------------

bool XclImpAddressConverter::ConvertRange( ScRange& rScRange,
        const XclRange& rXclRange, SCTAB nScTab1, SCTAB nScTab2, bool bWarn )
{
    // check start position
    bool bValidStart = CheckAddress( rXclRange.maFirst, bWarn );
    if( bValidStart )
    {
        lclFillAddress( rScRange.aStart, rXclRange.maFirst.mnCol, rXclRange.maFirst.mnRow, nScTab1 );

        // check & correct end position
        sal_uInt16 nXclCol2 = rXclRange.maLast.mnCol;
        sal_uInt32 nXclRow2 = rXclRange.maLast.mnRow;
        if( !CheckAddress( rXclRange.maLast, bWarn ) )
        {
            nXclCol2 = ::std::min( nXclCol2, mnMaxCol );
            nXclRow2 = ::std::min( nXclRow2, mnMaxRow );
        }
        lclFillAddress( rScRange.aEnd, nXclCol2, nXclRow2, nScTab2 );
    }
    return bValidStart;
}

// cell range list ------------------------------------------------------------

void XclImpAddressConverter::ConvertRangeList( ScRangeList& rScRanges,
        const XclRangeList& rXclRanges, SCTAB nScTab, bool bWarn )
{
    rScRanges.RemoveAll();
    for( XclRangeVector::const_iterator aIt = rXclRanges.begin(), aEnd = rXclRanges.end(); aIt != aEnd; ++aIt )
    {
        ScRange aScRange( ScAddress::UNINITIALIZED );
        if( ConvertRange( aScRange, *aIt, nScTab, nScTab, bWarn ) )
            rScRanges.Append( aScRange );
    }
}

// String->EditEngine conversion ==============================================

namespace {

EditTextObject* lclCreateTextObject( const XclImpRoot& rRoot,
        const XclImpString& rString, XclFontItemType eType, sal_uInt16 nXFIndex )
{
    EditTextObject* pTextObj = nullptr;

    const XclImpXFBuffer& rXFBuffer = rRoot.GetXFBuffer();
    const XclImpFont* pFirstFont = rXFBuffer.GetFont( nXFIndex );
    bool bFirstEscaped = pFirstFont && pFirstFont->HasEscapement();

    if( rString.IsRich() || bFirstEscaped )
    {
        const XclImpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
        const XclFormatRunVec& rFormats = rString.GetFormats();

        ScEditEngineDefaulter& rEE = (eType == EXC_FONTITEM_NOTE) ?
            static_cast< ScEditEngineDefaulter& >( rRoot.GetDoc().GetNoteEngine() ) : rRoot.GetEditEngine();
        rEE.SetText( rString.GetText() );

        SfxItemSet aItemSet( rEE.GetEmptyItemSet() );
        if( bFirstEscaped )
            rFontBuffer.FillToItemSet( aItemSet, eType, rXFBuffer.GetFontIndex( nXFIndex ) );
        ESelection aSelection;

        XclFormatRun aNextRun;
        XclFormatRunVec::const_iterator aIt = rFormats.begin();
        XclFormatRunVec::const_iterator aEnd = rFormats.end();

        if( aIt != aEnd )
            aNextRun = *aIt++;
        else
            aNextRun.mnChar = 0xFFFF;

        sal_Int32 nLen = rString.GetText().getLength();
        for( sal_Int32 nChar = 0; nChar < nLen; ++nChar )
        {
            // reached new different formatted text portion
            if( nChar >= aNextRun.mnChar )
            {
                // send items to edit engine
                rEE.QuickSetAttribs( aItemSet, aSelection );

                // start new item set
                aItemSet.ClearItem();
                rFontBuffer.FillToItemSet( aItemSet, eType, aNextRun.mnFontIdx );

                // read new formatting information
                if( aIt != aEnd )
                    aNextRun = *aIt++;
                else
                    aNextRun.mnChar = 0xFFFF;

                // reset selection start to current position
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos = aSelection.nEndPos;
            }

            // set end of selection to current position
            if( rString.GetText()[ nChar ] == '\n' )
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

} // namespace

EditTextObject* XclImpStringHelper::CreateTextObject(
        const XclImpRoot& rRoot, const XclImpString& rString )
{
    return lclCreateTextObject( rRoot, rString, EXC_FONTITEM_EDITENG, 0 );
}

void XclImpStringHelper::SetToDocument(
        ScDocumentImport& rDoc, const ScAddress& rPos, const XclImpRoot& rRoot,
        const XclImpString& rString, sal_uInt16 nXFIndex )
{
    if (rString.GetText().isEmpty())
        return;

    ::std::unique_ptr< EditTextObject > pTextObj( lclCreateTextObject( rRoot, rString, EXC_FONTITEM_EDITENG, nXFIndex ) );

    if (pTextObj.get())
    {
        rDoc.setEditCell(rPos, pTextObj.release());
    }
    else
    {
        OUString aStr = rString.GetText();
        if (aStr.indexOf('\n') != -1 || aStr.indexOf(CHAR_CR) != -1)
        {
            // Multiline content.
            ScFieldEditEngine& rEngine = rDoc.getDoc().GetEditEngine();
            rEngine.SetText(aStr);
            rDoc.setEditCell(rPos, rEngine.CreateTextObject());
        }
        else
        {
            // Normal text cell.
            rDoc.setStringCell(rPos, aStr);
        }
    }
}

// Header/footer conversion ===================================================

XclImpHFConverter::XclImpHFPortionInfo::XclImpHFPortionInfo() :
    mnHeight( 0 ),
    mnMaxLineHt( 0 )
{
    maSel.nStartPara = maSel.nEndPara = 0;
    maSel.nStartPos = maSel.nEndPos = 0;
}

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

void XclImpHFConverter::ParseString( const OUString& rHFString )
{
    // edit engine objects
    mrEE.SetText( EMPTY_OUSTRING );
    maInfos.clear();
    maInfos.resize( EXC_HF_PORTION_COUNT );
    meCurrObj = EXC_HF_CENTER;

    // parser temporaries
    maCurrText.clear();
    OUString aReadFont;           // current font name
    OUString aReadStyle;          // current font style
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

    const sal_Unicode* pChar = rHFString.getStr();
    const sal_Unicode* pNull = pChar + rHFString.getLength(); // pointer to teminating null char
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
                        maCurrText += OUString(*pChar);
                }
            }
            break;

// --- read control sequence ---

            case xlPSFunc:
            {
                eState = xlPSText;
                switch( *pChar )
                {
                    case '&':   maCurrText += "&";  break;  // the '&' character

                    case 'L':   SetNewPortion( EXC_HF_LEFT );   break;  // Left portion
                    case 'C':   SetNewPortion( EXC_HF_CENTER ); break;  // Center portion
                    case 'R':   SetNewPortion( EXC_HF_RIGHT );  break;  // Right portion

                    case 'P':   InsertField( SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ) );      break;  // page
                    case 'N':   InsertField( SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ) );     break;  // page count
                    case 'D':   InsertField( SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ) );      break;  // date
                    case 'T':   InsertField( SvxFieldItem( SvxTimeField(), EE_FEATURE_FIELD ) );      break;  // time
                    case 'A':   InsertField( SvxFieldItem( SvxTableField(), EE_FEATURE_FIELD ) );     break;  // table name

                    case 'Z':           // file path
                        InsertField( SvxFieldItem( SvxExtFileField(), EE_FEATURE_FIELD ) );   // convert to full name
                        if( (pNull - pChar >= 2) && (*(pChar + 1) == '&') && (*(pChar + 2) == 'F') )
                        {
                            // &Z&F found - ignore the &F part
                            pChar += 2;
                        }
                    break;
                    case 'F':           // file name
                        InsertField( SvxFieldItem( SvxExtFileField( EMPTY_OUSTRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ), EE_FEATURE_FIELD ) );
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
                        aReadFont.clear();
                        aReadStyle.clear();
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
                        SAL_FALLTHROUGH;
                    case ',':
                        eState = xlPSFontStyle;
                    break;
                    default:
                        aReadFont += OUString(*pChar);
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
                        if( !aReadFont.isEmpty() )
                            mxFontData->maName = aReadFont;
                        mxFontData->maStyle = aReadStyle;
                        eState = xlPSText;
                    break;
                    default:
                        aReadStyle += OUString(*pChar);
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
        XclImpFont aFont( GetRoot(), *mxFontData );
        aFont.FillToItemSet( aItemSet, EXC_FONTITEM_HF );
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
    if( !maCurrText.isEmpty() )
    {
        ESelection& rSel = GetCurrSel();
        mrEE.QuickInsertText( maCurrText, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
        rSel.nEndPos = rSel.nEndPos + maCurrText.getLength();
        maCurrText.clear();
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
    mrEE.QuickInsertText( OUString('\n'), ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
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
            mrEE.SetText( EMPTY_OUSTRING );
        ResetFontData();
    }
}

// URL conversion =============================================================

namespace {

void lclAppendUrlChar( OUString& rUrl, sal_Unicode cChar )
{
    // encode special characters
    switch( cChar )
    {
        case '#':   rUrl += "%23";  break;
        case '%':   rUrl += "%25";  break;
        default:    rUrl += OUString( cChar );
    }
}

} // namespace

void XclImpUrlHelper::DecodeUrl(
        OUString& rUrl, OUString& rTabName, bool& rbSameWb,
        const XclImpRoot& rRoot, const OUString& rEncodedUrl )
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
    OUString aDosBase( INetURLObject( rRoot.GetBasePath() ).getFSysPath( INetURLObject::FSYS_DOS ) );
    if (!aDosBase.isEmpty() && aDosBase.match(":\\", 1))
        cCurrDrive = aDosBase[0];

    const sal_Unicode* pChar = rEncodedUrl.getStr();
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
                    case '[':
                        bEncoded = false;
                        eState = xlUrlFileName;
                    break;
                    default:
                        bEncoded = false;
                        lclAppendUrlChar( rUrl, *pChar );
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
                                rUrl += "\\\\";
                            else
                            {
                                lclAppendUrlChar( rUrl, *pChar );
                                rUrl += ":\\";
                            }
                        }
                        else
                            rUrl += "<NULL-DRIVE!>";
                    }
                    break;
                    case EXC_URL_DRIVEROOT:
                        if( cCurrDrive )
                        {
                            lclAppendUrlChar( rUrl, cCurrDrive );
                            rUrl += ":";
                        }
                        SAL_FALLTHROUGH;
                    case EXC_URL_SUBDIR:
                        if( bEncoded )
                            rUrl += "\\";
                        else    // control character in raw name -> DDE link
                        {
                            rUrl += OUStringLiteral1<EXC_DDE_DELIM>();
                            eState = xlUrlRaw;
                        }
                    break;
                    case EXC_URL_PARENTDIR:
                        rUrl += "..\\";
                    break;
                    case EXC_URL_RAW:
                    {
                        if( *(pChar + 1) )
                        {
                            sal_Int32 nLen = *++pChar;
                            for( sal_Int32 nChar = 0; (nChar < nLen) && *(pChar + 1); ++nChar )
                                lclAppendUrlChar( rUrl, *++pChar );
//                            rUrl.Append( ':' );
                        }
                    }
                    break;
                    case '[':
                        eState = xlUrlFileName;
                    break;
                    default:
                        lclAppendUrlChar( rUrl, *pChar );
                }
            }
            break;

// --- file name ---

            case xlUrlFileName:
            {
                switch( *pChar )
                {
                    case ']':   eState = xlUrlSheetName;    break;
                    default:    lclAppendUrlChar( rUrl, *pChar );
                }
            }
            break;

// --- sheet name ---

            case xlUrlSheetName:
                rTabName += OUString( *pChar );
            break;

// --- raw read mode ---

            case xlUrlRaw:
                lclAppendUrlChar( rUrl, *pChar );
            break;
        }

        ++pChar;
    }
}

void XclImpUrlHelper::DecodeUrl(
    OUString& rUrl, bool& rbSameWb, const XclImpRoot& rRoot, const OUString& rEncodedUrl )
{
    OUString aTabName;
    OUString aUrl;
    DecodeUrl( aUrl, aTabName, rbSameWb, rRoot, rEncodedUrl );
    rUrl = aUrl;
    OSL_ENSURE( aTabName.isEmpty(), "XclImpUrlHelper::DecodeUrl - sheet name ignored" );
}

bool XclImpUrlHelper::DecodeLink( OUString& rApplic, OUString& rTopic, const OUString& rEncUrl )
{
    sal_Int32 nPos = rEncUrl.indexOf( EXC_DDE_DELIM );
    if( (nPos > 0) && (nPos + 1 < rEncUrl.getLength()) )
    {
        rApplic = rEncUrl.copy( 0, nPos );
        rTopic = rEncUrl.copy( nPos + 1 );
        return true;
    }
    return false;
}

// Cached Values ==============================================================

XclImpCachedValue::XclImpCachedValue( XclImpStream& rStrm ) :
    mfValue( 0.0 ),
    mnBoolErr( 0 )
{
    mnType = rStrm.ReaduInt8();
    switch( mnType )
    {
        case EXC_CACHEDVAL_EMPTY:
            rStrm.Ignore( 8 );
        break;
        case EXC_CACHEDVAL_DOUBLE:
            mfValue = rStrm.ReadDouble();
        break;
        case EXC_CACHEDVAL_STRING:
            maStr = rStrm.ReadUniString();
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            double fVal;
            mnBoolErr = rStrm.ReaduInt8();
            rStrm.Ignore( 7 );

            const ScTokenArray* pScTokArr = rStrm.GetRoot().GetOldFmlaConverter().GetBoolErr(
                XclTools::ErrorToEnum( fVal, mnType == EXC_CACHEDVAL_ERROR, mnBoolErr ) );
            if( pScTokArr )
                mxTokArr.reset( pScTokArr->Clone() );
        }
        break;
        default:
            OSL_FAIL( "XclImpCachedValue::XclImpCachedValue - unknown data type" );
    }
}

XclImpCachedValue::~XclImpCachedValue()
{
}

sal_uInt16 XclImpCachedValue::GetScError() const
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

    if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
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

    //assuming worse case scenario of unknown types
    const size_t nMinRecordSize = 1;
    const size_t nMaxRows = rStrm.GetRecLeft() / (nMinRecordSize * mnScCols);
    if (mnScRows > nMaxRows)
    {
        SAL_WARN("sc", "Parsing error: " << nMaxRows <<
                 " max possible rows, but " << mnScRows << " claimed, truncating");
        mnScRows = nMaxRows;
    }

    for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            maValueList.push_back( o3tl::make_unique<XclImpCachedValue>( rStrm ) );
}

XclImpCachedMatrix::~XclImpCachedMatrix()
{
}

ScMatrixRef XclImpCachedMatrix::CreateScMatrix( svl::SharedStringPool& rPool ) const
{
    ScMatrixRef xScMatrix;
    OSL_ENSURE( mnScCols * mnScRows == maValueList.size(), "XclImpCachedMatrix::CreateScMatrix - element count mismatch" );
    if( mnScCols && mnScRows && static_cast< sal_uLong >( mnScCols * mnScRows ) <= maValueList.size() )
    {
        xScMatrix = new ScFullMatrix(mnScCols, mnScRows, 0.0);
        XclImpValueList::const_iterator itValue = maValueList.begin();
        for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        {
            for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            {
                switch( (*itValue)->GetType() )
                {
                    case EXC_CACHEDVAL_EMPTY:
                        // Excel shows 0.0 here, not an empty cell
                        xScMatrix->PutEmpty( nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_DOUBLE:
                        xScMatrix->PutDouble( (*itValue)->GetValue(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_STRING:
                        xScMatrix->PutString(rPool.intern((*itValue)->GetString()), nScCol, nScRow);
                    break;
                    case EXC_CACHEDVAL_BOOL:
                        xScMatrix->PutBoolean( (*itValue)->GetBool(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_ERROR:
                        xScMatrix->PutError( (*itValue)->GetScError(), nScCol, nScRow );
                    break;
                    default:
                        OSL_FAIL( "XclImpCachedMatrix::CreateScMatrix - unknown value type" );
                        xScMatrix->PutEmpty( nScCol, nScRow );
                }
                ++itValue;
            }
        }
    }
    return xScMatrix;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
