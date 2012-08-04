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

#include "xicontent.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/itemset.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include "stringutil.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "cell.hxx"
#include "validat.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "rangenam.hxx"
#include "arealink.hxx"
#include "stlsheet.hxx"
#include "scextopt.hxx"
#include "xlformula.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xistyle.hxx"
#include "xiescher.hxx"
#include "xiname.hxx"

#include "excform.hxx"
#include "tabprotection.hxx"

#include <memory>

using ::com::sun::star::uno::Sequence;
using ::std::auto_ptr;

// Shared string table ========================================================

XclImpSst::XclImpSst( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpSst::ReadSst( XclImpStream& rStrm )
{
    rStrm.Ignore( 4 );
    sal_uInt32 nStrCount(0);
    rStrm >> nStrCount;
    maStrings.clear();
    maStrings.reserve( static_cast< size_t >( nStrCount ) );
    while( (nStrCount > 0) && rStrm.IsValid() )
    {
        XclImpString aString;
        aString.Read( rStrm );
        maStrings.push_back( aString );
        --nStrCount;
    }
}

const XclImpString* XclImpSst::GetString( sal_uInt32 nSstIndex ) const
{
    return (nSstIndex < maStrings.size()) ? &maStrings[ nSstIndex ] : 0;
}

ScBaseCell* XclImpSst::CreateCell( sal_uInt32 nSstIndex, sal_uInt16 nXFIndex ) const
{
    ScBaseCell* pCell = 0;
    if( const XclImpString* pString = GetString( nSstIndex ) )
        pCell = XclImpStringHelper::CreateCell( *this, *pString, nXFIndex );
    return pCell;
}

// Hyperlinks =================================================================

namespace {

/** Reads character array and stores it into rString.
    @param nChars  Number of following characters (not byte count!).
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclAppendString32( String& rString, XclImpStream& rStrm, sal_uInt32 nChars, bool b16Bit )
{
    sal_uInt16 nReadChars = ulimit_cast< sal_uInt16 >( nChars );
    rString.Append( rStrm.ReadRawUniString( nReadChars, b16Bit ) );
    // ignore remaining chars
    sal_Size nIgnore = nChars - nReadChars;
    if( b16Bit )
        nIgnore *= 2;
    rStrm.Ignore( nIgnore );
}

/** Reads 32-bit string length and the character array and stores it into rString.
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclAppendString32( String& rString, XclImpStream& rStrm, bool b16Bit )
{
    lclAppendString32( rString, rStrm, rStrm.ReaduInt32(), b16Bit );
}

/** Reads 32-bit string length and ignores following character array.
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclIgnoreString32( XclImpStream& rStrm, bool b16Bit )
{
    sal_uInt32 nChars(0);
    rStrm >> nChars;
    if( b16Bit )
        nChars *= 2;
    rStrm.Ignore( nChars );
}

/** Converts a path to an absolute path.
    @param rPath  The source path. The resulting path is returned here.
    @param nLevel  Number of parent directories to add in front of the path. */
void lclGetAbsPath( String& rPath, sal_uInt16 nLevel, SfxObjectShell* pDocShell )
{
    String aTmpStr;
    while( nLevel )
    {
        aTmpStr.AppendAscii( "../" );
        --nLevel;
    }
    aTmpStr += rPath;

    if( pDocShell )
    {
        bool bWasAbs = false;
        rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr, bWasAbs ).GetMainURL( INetURLObject::NO_DECODE );
        // full path as stored in SvxURLField must be encoded
    }
    else
        rPath = aTmpStr;
}

/** Inserts the URL into a text cell. Does not modify value or formula cells. */
void lclInsertUrl( const XclImpRoot& rRoot, const String& rUrl, SCCOL nScCol, SCROW nScRow, SCTAB nScTab )
{
    ScDocument& rDoc = rRoot.GetDoc();
    ScAddress aScPos( nScCol, nScRow, nScTab );
    CellType eCellType = rDoc.GetCellType( aScPos );
    switch( eCellType )
    {
        // #i54261# hyperlinks in string cells
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
        {
            String aDisplText;
            rDoc.GetString( nScCol, nScRow, nScTab, aDisplText );
            if( !aDisplText.Len() )
                aDisplText = rUrl;

            ScEditEngineDefaulter& rEE = rRoot.GetEditEngine();
            SvxURLField aUrlField( rUrl, aDisplText, SVXURLFORMAT_APPDEFAULT );

            const ScEditCell* pEditCell = (eCellType == CELLTYPE_EDIT) ? static_cast< const ScEditCell* >( rDoc.GetCell( aScPos ) ) : 0;
            const EditTextObject* pEditObj = pEditCell ? pEditCell->GetData() : 0;
            if( pEditObj )
            {
                rEE.SetText( *pEditObj );
                rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ), ESelection( 0, 0, 0xFFFF, 0 ) );
            }
            else
            {
                rEE.SetText( EMPTY_STRING );
                rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ), ESelection() );
                if( const ScPatternAttr* pPattern = rDoc.GetPattern( aScPos.Col(), aScPos.Row(), nScTab ) )
                {
                    SfxItemSet aItemSet( rEE.GetEmptyItemSet() );
                    pPattern->FillEditItemSet( &aItemSet );
                    rEE.QuickSetAttribs( aItemSet, ESelection( 0, 0, 0xFFFF, 0 ) );
                }
            }
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr< EditTextObject > xTextObj( rEE.CreateTextObject() );
            SAL_WNODEPRECATED_DECLARATIONS_POP

            ScEditCell* pCell = new ScEditCell( xTextObj.get(), &rDoc, rEE.GetEditTextObjectPool() );
            rDoc.PutCell( aScPos, pCell );
        }
        break;

        default:;
    }
}

} // namespace

// ----------------------------------------------------------------------------

void XclImpHyperlink::ReadHlink( XclImpStream& rStrm )
{
    XclRange aXclRange( ScAddress::UNINITIALIZED );
    rStrm >> aXclRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aXclRange.maFirst.mnCol &= 0xFF;
    aXclRange.maLast.mnCol &= 0xFF;
    String aString = ReadEmbeddedData( rStrm );
    if ( aString.Len() > 0 )
        rStrm.GetRoot().GetXFRangeBuffer().SetHyperlink( aXclRange, aString );
}

String XclImpHyperlink::ReadEmbeddedData( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    SfxObjectShell* pDocShell = rRoot.GetDocShell();

    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    XclGuid aGuid;
    rStrm >> aGuid;
    rStrm.Ignore( 4 );
    sal_uInt32 nFlags(0);
    rStrm >> nFlags;

    OSL_ENSURE( aGuid == XclTools::maGuidStdLink, "XclImpHyperlink::ReadEmbeddedData - unknown header GUID" );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< String > xLongName;    // link / file name
    ::std::auto_ptr< String > xShortName;   // 8.3-representation of file name
    ::std::auto_ptr< String > xTextMark;    // text mark
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // description (ignore)
    if( ::get_flag( nFlags, EXC_HLINK_DESCR ) )
        lclIgnoreString32( rStrm, true );
    // target frame (ignore) !! DESCR/FRAME - is this the right order? (never seen them together)
    if( ::get_flag( nFlags, EXC_HLINK_FRAME ) )
        lclIgnoreString32( rStrm, true );

    // URL fields are zero-terminated - do not let the stream replace them
    // in the lclAppendString32() with the '?' character.
    rStrm.SetNulSubstChar( '\0' );

    // UNC path
    if( ::get_flag( nFlags, EXC_HLINK_UNC ) )
    {
        xLongName.reset( new String );
        lclAppendString32( *xLongName, rStrm, true );
        lclGetAbsPath( *xLongName, 0, pDocShell );
    }
    // file link or URL
    else if( ::get_flag( nFlags, EXC_HLINK_BODY ) )
    {
        rStrm >> aGuid;

        if( aGuid == XclTools::maGuidFileMoniker )
        {
            sal_uInt16 nLevel = 0; // counter for level to climb down in path
            rStrm >> nLevel;
            xShortName.reset( new String );
            lclAppendString32( *xShortName, rStrm, false );
            rStrm.Ignore( 24 );

            sal_uInt32 nStrLen = 0;
            rStrm >> nStrLen;
            if( nStrLen )
            {
                nStrLen = 0;
                rStrm >> nStrLen;
                nStrLen /= 2;       // it's byte count here...
                rStrm.Ignore( 2 );
                xLongName.reset( new String );
                lclAppendString32( *xLongName, rStrm, nStrLen, true );
                lclGetAbsPath( *xLongName, nLevel, pDocShell );
            }
            else
                lclGetAbsPath( *xShortName, nLevel, pDocShell );
        }
        else if( aGuid == XclTools::maGuidUrlMoniker )
        {
            sal_uInt32 nStrLen(0);
            rStrm >> nStrLen;
            nStrLen /= 2;       // it's byte count here...
            xLongName.reset( new String );
            lclAppendString32( *xLongName, rStrm, nStrLen, true );
            if( !::get_flag( nFlags, EXC_HLINK_ABS ) )
                lclGetAbsPath( *xLongName, 0, pDocShell );
        }
        else
        {
            OSL_FAIL( "XclImpHyperlink::ReadEmbeddedData - unknown content GUID" );
        }
    }

    // text mark
    if( ::get_flag( nFlags, EXC_HLINK_MARK ) )
    {
        xTextMark.reset( new String );
        lclAppendString32( *xTextMark, rStrm, true );
    }

    rStrm.SetNulSubstChar();    // back to default

    OSL_ENSURE( rStrm.GetRecLeft() == 0, "XclImpHyperlink::ReadEmbeddedData - record size mismatch" );

    if( !xLongName.get() && xShortName.get() )
        xLongName = xShortName;
    else if( !xLongName.get() && xTextMark.get() )
        xLongName.reset( new String );

    if( xLongName.get() )
    {
        if( xTextMark.get() )
        {
            if( xLongName->Len() == 0 )
                xTextMark->SearchAndReplaceAll( '!', '.' );
            xLongName->Append( '#' );
            xLongName->Append( *xTextMark );
        }
        return *xLongName;
    }
    return String::EmptyString();
}

void XclImpHyperlink::ConvertToValidTabName(String& rUrl)
{
    xub_StrLen n = rUrl.Len();
    if (n < 4)
        // Needs at least 4 characters.
        return;

    sal_Unicode c = rUrl.GetChar(0);
    if (c != sal_Unicode('#'))
        // the 1st character must be '#'.
        return;

    String aNewUrl(rtl::OUString('#')), aTabName;

    bool bInQuote = false;
    bool bQuoteTabName = false;
    for (xub_StrLen i = 1; i < n; ++i)
    {
        c = rUrl.GetChar(i);
        if (c == sal_Unicode('\''))
        {
            if (bInQuote && i+1 < n && rUrl.GetChar(i+1) == sal_Unicode('\''))
            {
                // Two consecutive single quotes ('') signify a single literal
                // quite.  When this occurs, the whole table name needs to be
                // quoted.
                bQuoteTabName = true;
                aTabName.Append(c);
                aTabName.Append(c);
                ++i;
                continue;
            }

            bInQuote = !bInQuote;
            if (!bInQuote && aTabName.Len() > 0)
            {
                if (bQuoteTabName)
                    aNewUrl.Append(sal_Unicode('\''));
                aNewUrl.Append(aTabName);
                if (bQuoteTabName)
                    aNewUrl.Append(sal_Unicode('\''));
            }
        }
        else if (bInQuote)
            aTabName.Append(c);
        else
            aNewUrl.Append(c);
    }

    if (bInQuote)
        // It should be outside the quotes!
        return;

    // All is good.  Pass the new URL.
    rUrl = aNewUrl;
}

void XclImpHyperlink::InsertUrl( const XclImpRoot& rRoot, const XclRange& rXclRange, const String& rUrl )
{
    String aUrl(rUrl);
    ConvertToValidTabName(aUrl);

    SCTAB nScTab = rRoot.GetCurrScTab();
    ScRange aScRange( ScAddress::UNINITIALIZED );
    if( rRoot.GetAddressConverter().ConvertRange( aScRange, rXclRange, nScTab, nScTab, true ) )
    {
        SCCOL nScCol1, nScCol2;
        SCROW nScRow1, nScRow2;
        aScRange.GetVars( nScCol1, nScRow1, nScTab, nScCol2, nScRow2, nScTab );
        for( SCCOL nScCol = nScCol1; nScCol <= nScCol2; ++nScCol )
            for( SCROW nScRow = nScRow1; nScRow <= nScRow2; ++nScRow )
                lclInsertUrl( rRoot, aUrl, nScCol, nScRow, nScTab );
    }
}

// Label ranges ===============================================================

void XclImpLabelranges::ReadLabelranges( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    ScDocument& rDoc = rRoot.GetDoc();
    SCTAB nScTab = rRoot.GetCurrScTab();
    XclImpAddressConverter& rAddrConv = rRoot.GetAddressConverter();
    ScRangePairListRef xLabelRangesRef;
    const ScRange* pScRange = 0;

    XclRangeList aRowXclRanges, aColXclRanges;
    rStrm >> aRowXclRanges >> aColXclRanges;

    // row label ranges
    ScRangeList aRowScRanges;
    rAddrConv.ConvertRangeList( aRowScRanges, aRowXclRanges, nScTab, false );
    xLabelRangesRef = rDoc.GetRowNameRangesRef();
    for ( size_t i = 0, nRanges = aRowScRanges.size(); i < nRanges; ++i )
    {
        pScRange = aRowScRanges[ i ];
        ScRange aDataRange( *pScRange );
        if( aDataRange.aEnd.Col() < MAXCOL )
        {
            aDataRange.aStart.SetCol( aDataRange.aEnd.Col() + 1 );
            aDataRange.aEnd.SetCol( MAXCOL );
        }
        else if( aDataRange.aStart.Col() > 0 )
        {
            aDataRange.aEnd.SetCol( aDataRange.aStart.Col() - 1 );
            aDataRange.aStart.SetCol( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pScRange, aDataRange ) );
    }

    // column label ranges
    ScRangeList aColScRanges;
    rAddrConv.ConvertRangeList( aColScRanges, aColXclRanges, nScTab, false );
    xLabelRangesRef = rDoc.GetColNameRangesRef();

    for ( size_t i = 0, nRanges = aColScRanges.size(); i < nRanges; ++i )
    {
        pScRange = aColScRanges[ i ];
        ScRange aDataRange( *pScRange );
        if( aDataRange.aEnd.Row() < MAXROW )
        {
            aDataRange.aStart.SetRow( aDataRange.aEnd.Row() + 1 );
            aDataRange.aEnd.SetRow( MAXROW );
        }
        else if( aDataRange.aStart.Row() > 0 )
        {
            aDataRange.aEnd.SetRow( aDataRange.aStart.Row() - 1 );
            aDataRange.aStart.SetRow( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pScRange, aDataRange ) );
    }
}

// Conditional formatting =====================================================

XclImpCondFormat::XclImpCondFormat( const XclImpRoot& rRoot, sal_uInt32 nFormatIndex ) :
    XclImpRoot( rRoot ),
    mnFormatIndex( nFormatIndex ),
    mnCondCount( 0 ),
    mnCondIndex( 0 )
{
}

XclImpCondFormat::~XclImpCondFormat()
{
}

void XclImpCondFormat::ReadCondfmt( XclImpStream& rStrm )
{
    OSL_ENSURE( !mnCondCount, "XclImpCondFormat::ReadCondfmt - already initialized" );
    XclRangeList aXclRanges;
    rStrm >> mnCondCount;
    rStrm.Ignore( 10 );
    rStrm >> aXclRanges;
    GetAddressConverter().ConvertRangeList( maRanges, aXclRanges, GetCurrScTab(), true );
}

void XclImpCondFormat::ReadCF( XclImpStream& rStrm )
{
    if( mnCondIndex >= mnCondCount )
    {
        OSL_FAIL( "XclImpCondFormat::ReadCF - CF without leading CONDFMT" );
        return;
    }

    // entire conditional format outside of valid range?
    if( maRanges.empty() )
        return;

    sal_uInt8 nType(0), nOperator(0);
    sal_uInt16 nFmlaSize1(0), nFmlaSize2(0);
    sal_uInt32 nFlags(0);

    rStrm >> nType >> nOperator >> nFmlaSize1 >> nFmlaSize2 >> nFlags;
    rStrm.Ignore( 2 );

    // *** mode and comparison operator ***

    ScConditionMode eMode = SC_COND_NONE;
    switch( nType )
    {
        case EXC_CF_TYPE_CELL:
        {
            switch( nOperator )
            {
                case EXC_CF_CMP_BETWEEN:        eMode = SC_COND_BETWEEN;    break;
                case EXC_CF_CMP_NOT_BETWEEN:    eMode = SC_COND_NOTBETWEEN; break;
                case EXC_CF_CMP_EQUAL:          eMode = SC_COND_EQUAL;      break;
                case EXC_CF_CMP_NOT_EQUAL:      eMode = SC_COND_NOTEQUAL;   break;
                case EXC_CF_CMP_GREATER:        eMode = SC_COND_GREATER;    break;
                case EXC_CF_CMP_LESS:           eMode = SC_COND_LESS;       break;
                case EXC_CF_CMP_GREATER_EQUAL:  eMode = SC_COND_EQGREATER;  break;
                case EXC_CF_CMP_LESS_EQUAL:     eMode = SC_COND_EQLESS;     break;
                default:
                    OSL_TRACE( "XclImpCondFormat::ReadCF - unknown CF comparison 0x%02hX", nOperator );
            }
        }
        break;

        case EXC_CF_TYPE_FMLA:
            eMode = SC_COND_DIRECT;
        break;

        default:
            OSL_TRACE( "XclImpCondFormat::ReadCF - unknown CF mode 0x%02hX", nType );
            return;
    }

    // *** create style sheet ***

    String aStyleName( XclTools::GetCondFormatStyleName( GetCurrScTab(), mnFormatIndex, mnCondIndex ) );
    SfxItemSet& rStyleItemSet = ScfTools::MakeCellStyleSheet( GetStyleSheetPool(), aStyleName, true ).GetItemSet();

    const XclImpPalette& rPalette = GetPalette();

    // *** font block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_FONT ) )
    {
        XclImpFont aFont( GetRoot() );
        aFont.ReadCFFontBlock( rStrm );
        aFont.FillToItemSet( rStyleItemSet, EXC_FONTITEM_CELL );
    }

    // *** border block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_BORDER ) )
    {
        sal_uInt16 nLineStyle(0);
        sal_uInt32 nLineColor(0);
        rStrm >> nLineStyle >> nLineColor;
        rStrm.Ignore( 2 );

        XclImpCellBorder aBorder;
        aBorder.FillFromCF8( nLineStyle, nLineColor, nFlags );
        aBorder.FillToItemSet( rStyleItemSet, rPalette );
    }

    // *** pattern block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_AREA ) )
    {
        sal_uInt16 nPattern(0), nColor(0);
        rStrm >> nPattern >> nColor;

        XclImpCellArea aArea;
        aArea.FillFromCF8( nPattern, nColor, nFlags );
        aArea.FillToItemSet( rStyleItemSet, rPalette );
    }

    // *** formulas ***

    const ScAddress& rPos = maRanges.front()->aStart;    // assured above that maRanges is not empty
    ExcelToSc& rFmlaConv = GetOldFmlaConverter();

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScTokenArray > xTokArr1;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if( nFmlaSize1 > 0 )
    {
        const ScTokenArray* pTokArr = 0;
        rFmlaConv.Reset( rPos );
        rFmlaConv.Convert( pTokArr, rStrm, nFmlaSize1, false, FT_RangeName );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr1.reset( pTokArr->Clone() );
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScTokenArray > pTokArr2;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if( nFmlaSize2 > 0 )
    {
        const ScTokenArray* pTokArr = 0;
        rFmlaConv.Reset( rPos );
        rFmlaConv.Convert( pTokArr, rStrm, nFmlaSize2, false, FT_RangeName );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            pTokArr2.reset( pTokArr->Clone() );
    }

    // *** create the Calc conditional formatting ***

    if( !mxScCondFmt.get() )
    {
        sal_uLong nKey = 0;
        mxScCondFmt.reset( new ScConditionalFormat( nKey, GetDocPtr() ) );
    }

    ScCondFormatEntry* pEntry = new ScCondFormatEntry( eMode, xTokArr1.get(), pTokArr2.get(), GetDocPtr(), rPos, aStyleName );
    mxScCondFmt->AddEntry( pEntry );
    ++mnCondIndex;
}

void XclImpCondFormat::Apply()
{
    if( mxScCondFmt.get() )
    {
        ScDocument& rDoc = GetDoc();

        sal_uLong nKey = rDoc.AddCondFormat( mxScCondFmt->Clone(), maRanges.front()->aStart.Tab() );
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_CONDITIONAL, nKey ) );

        // maRanges contains only valid cell ranges
        for ( size_t i = 0, nRanges = maRanges.size(); i < nRanges; ++i )
        {
            const ScRange* pScRange = maRanges[ i ];
            rDoc.ApplyPatternAreaTab(
                pScRange->aStart.Col(), pScRange->aStart.Row(),
                pScRange->aEnd.Col(), pScRange->aEnd.Row(),
                pScRange->aStart.Tab(), aPattern );
        }
    }
}

// ----------------------------------------------------------------------------

XclImpCondFormatManager::XclImpCondFormatManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpCondFormatManager::ReadCondfmt( XclImpStream& rStrm )
{
    XclImpCondFormat* pFmt = new XclImpCondFormat( GetRoot(), maCondFmtList.size() );
    pFmt->ReadCondfmt( rStrm );
    maCondFmtList.push_back( pFmt );
}

void XclImpCondFormatManager::ReadCF( XclImpStream& rStrm )
{
    OSL_ENSURE( !maCondFmtList.empty(), "XclImpCondFormatManager::ReadCF - CF without leading CONDFMT" );
    if( !maCondFmtList.empty() )
        maCondFmtList.back().ReadCF( rStrm );
}

void XclImpCondFormatManager::Apply()
{
    for( XclImpCondFmtList::iterator itFmt = maCondFmtList.begin(); itFmt != maCondFmtList.end(); ++itFmt )
        itFmt->Apply();
    maCondFmtList.clear();
}

// Data Validation ============================================================

XclImpValidationManager::DVItem::DVItem( const ScRangeList& rRanges, const ScValidationData& rValidData ) :
    maRanges(rRanges), maValidData(rValidData) {}

XclImpValidationManager::XclImpValidationManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpValidationManager::ReadDval( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    sal_uInt32 nObjId(0);
    rStrm.Ignore( 10 );
    rStrm >> nObjId;
    if( nObjId != EXC_DVAL_NOOBJ )
    {
        OSL_ENSURE( nObjId <= 0xFFFF, "XclImpValidation::ReadDval - invalid object ID" );
        rRoot.GetCurrSheetDrawing().SetSkipObj( static_cast< sal_uInt16 >( nObjId ) );
    }
}

void XclImpValidationManager::ReadDV( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    ScDocument& rDoc = rRoot.GetDoc();
    SCTAB nScTab = rRoot.GetCurrScTab();
    ExcelToSc& rFmlaConv = rRoot.GetOldFmlaConverter();

    // flags
    sal_uInt32 nFlags(0);
    rStrm >> nFlags;

    // message strings
    /*  Empty strings are single NUL characters in Excel (string length is 1).
        -> Do not let the stream replace them with '?' characters. */
    rStrm.SetNulSubstChar( '\0' );
    String aPromptTitle(   rStrm.ReadUniString() );
    String aErrorTitle(    rStrm.ReadUniString() );
    String aPromptMessage( rStrm.ReadUniString() );
    String aErrorMessage(  rStrm.ReadUniString() );
    rStrm.SetNulSubstChar();    // back to default

    // formula(s)
    if ( rStrm.GetRecLeft() <= 8 )
        // Not enough bytes left in the record.  Bail out.
        return;


    // first formula
    // string list is single tStr token with NUL separators -> replace them with LF
    rStrm.SetNulSubstChar( '\n' );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScTokenArray > xTokArr1;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    sal_uInt16 nLen = 0;
    rStrm >> nLen;
    rStrm.Ignore( 2 );
    if( nLen > 0 )
    {
        const ScTokenArray* pTokArr = 0;
        rFmlaConv.Reset();
        rFmlaConv.Convert( pTokArr, rStrm, nLen, false, FT_RangeName );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr1.reset( pTokArr->Clone() );
    }
    rStrm.SetNulSubstChar();    // back to default

    // second formula
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScTokenArray > xTokArr2;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    nLen = 0;
    rStrm >> nLen;
    rStrm.Ignore( 2 );
    if( nLen > 0 )
    {
        const ScTokenArray* pTokArr = 0;
        rFmlaConv.Reset();
        rFmlaConv.Convert( pTokArr, rStrm, nLen, false, FT_RangeName );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr2.reset( pTokArr->Clone() );
    }

    // read all cell ranges
    XclRangeList aXclRanges;
    rStrm >> aXclRanges;

    // convert to Calc range list
    ScRangeList aScRanges;
    rRoot.GetAddressConverter().ConvertRangeList( aScRanges, aXclRanges, nScTab, true );

    // only continue if there are valid ranges
    if ( aScRanges.empty() )
        return;

    bool bIsValid = true;   // valid settings in flags field

    ScValidationMode eValMode = SC_VALID_ANY;
    switch( nFlags & EXC_DV_MODE_MASK )
    {
        case EXC_DV_MODE_ANY:       eValMode = SC_VALID_ANY;        break;
        case EXC_DV_MODE_WHOLE:     eValMode = SC_VALID_WHOLE;      break;
        case EXC_DV_MODE_DECIMAL:   eValMode = SC_VALID_DECIMAL;    break;
        case EXC_DV_MODE_LIST:      eValMode = SC_VALID_LIST;       break;
        case EXC_DV_MODE_DATE:      eValMode = SC_VALID_DATE;       break;
        case EXC_DV_MODE_TIME:      eValMode = SC_VALID_TIME;       break;
        case EXC_DV_MODE_TEXTLEN:   eValMode = SC_VALID_TEXTLEN;    break;
        case EXC_DV_MODE_CUSTOM:    eValMode = SC_VALID_CUSTOM;     break;
        default:                    bIsValid = false;
    }
    rRoot.GetTracer().TraceDVType(eValMode == SC_VALID_CUSTOM);

    ScConditionMode eCondMode = SC_COND_BETWEEN;
    switch( nFlags & EXC_DV_COND_MASK )
    {
        case EXC_DV_COND_BETWEEN:   eCondMode = SC_COND_BETWEEN;    break;
        case EXC_DV_COND_NOTBETWEEN:eCondMode = SC_COND_NOTBETWEEN; break;
        case EXC_DV_COND_EQUAL:     eCondMode = SC_COND_EQUAL;      break;
        case EXC_DV_COND_NOTEQUAL:  eCondMode = SC_COND_NOTEQUAL;   break;
        case EXC_DV_COND_GREATER:   eCondMode = SC_COND_GREATER;    break;
        case EXC_DV_COND_LESS:      eCondMode = SC_COND_LESS;       break;
        case EXC_DV_COND_EQGREATER: eCondMode = SC_COND_EQGREATER;  break;
        case EXC_DV_COND_EQLESS:    eCondMode = SC_COND_EQLESS;     break;
        default:                    bIsValid = false;
    }

    if ( !bIsValid )
        // No valid validation found.  Bail out.
        return;


    // first range for base address for relative references
    const ScRange& rScRange = *aScRanges.front();    // aScRanges is not empty

    // process string list of a list validity (convert to list of string tokens)
    if( xTokArr1.get() && (eValMode == SC_VALID_LIST) && ::get_flag( nFlags, EXC_DV_STRINGLIST ) )
        XclTokenArrayHelper::ConvertStringToList( *xTokArr1, '\n', true );

    maDVItems.push_back(
        new DVItem(aScRanges, ScValidationData(eValMode, eCondMode, xTokArr1.get(), xTokArr2.get(), &rDoc, rScRange.aStart)));
    DVItem& rItem = maDVItems.back();

    rItem.maValidData.SetIgnoreBlank( ::get_flag( nFlags, EXC_DV_IGNOREBLANK ) );
    rItem.maValidData.SetListType( ::get_flagvalue( nFlags, EXC_DV_SUPPRESSDROPDOWN, ValidListType::INVISIBLE, ValidListType::UNSORTED ) );

    // *** prompt box ***
    if( aPromptTitle.Len() || aPromptMessage.Len() )
    {
        // set any text stored in the record
        rItem.maValidData.SetInput( aPromptTitle, aPromptMessage );
        if( !::get_flag( nFlags, EXC_DV_SHOWPROMPT ) )
            rItem.maValidData.ResetInput();
    }

    // *** error box ***
    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
    switch( nFlags & EXC_DV_ERROR_MASK )
    {
        case EXC_DV_ERROR_WARNING:  eErrStyle = SC_VALERR_WARNING;  break;
        case EXC_DV_ERROR_INFO:     eErrStyle = SC_VALERR_INFO;     break;
    }
    // set texts and error style
    rItem.maValidData.SetError( aErrorTitle, aErrorMessage, eErrStyle );
    if( !::get_flag( nFlags, EXC_DV_SHOWERROR ) )
        rItem.maValidData.ResetError();
}

void XclImpValidationManager::Apply()
{
    ScDocument& rDoc = GetRoot().GetDoc();
    DVItemList::iterator itr = maDVItems.begin(), itrEnd = maDVItems.end();
    for (; itr != itrEnd; ++itr)
    {
        DVItem& rItem = *itr;
        // set the handle ID
        sal_uLong nHandle = rDoc.AddValidationEntry( rItem.maValidData );
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nHandle ) );

        // apply all ranges
        for ( size_t i = 0, nRanges = rItem.maRanges.size(); i < nRanges; ++i )
        {
            const ScRange* pScRange = rItem.maRanges[ i ];
            rDoc.ApplyPatternAreaTab( pScRange->aStart.Col(), pScRange->aStart.Row(),
                pScRange->aEnd.Col(), pScRange->aEnd.Row(), pScRange->aStart.Tab(), aPattern );
        }
    }
    maDVItems.clear();
}

// Web queries ================================================================

XclImpWebQuery::XclImpWebQuery( const ScRange& rDestRange ) :
    maDestRange( rDestRange ),
    meMode( xlWQUnknown ),
    mnRefresh( 0 )
{
}

void XclImpWebQuery::ReadParamqry( XclImpStream& rStrm )
{
    sal_uInt16 nFlags = rStrm.ReaduInt16();
    sal_uInt16 nType = ::extract_value< sal_uInt16 >( nFlags, 0, 3 );
    if( (nType == EXC_PQRYTYPE_WEBQUERY) && ::get_flag( nFlags, EXC_PQRY_WEBQUERY ) )
    {
        if( ::get_flag( nFlags, EXC_PQRY_TABLES ) )
        {
            meMode = xlWQAllTables;
            maTables = ScfTools::GetHTMLTablesName();
        }
        else
        {
            meMode = xlWQDocument;
            maTables = ScfTools::GetHTMLDocName();
        }
    }
}

void XclImpWebQuery::ReadWqstring( XclImpStream& rStrm )
{
    maURL = rStrm.ReadUniString();
}

void XclImpWebQuery::ReadWqsettings( XclImpStream& rStrm )
{
    rStrm.Ignore( 10 );
    sal_uInt16 nFlags(0);
    rStrm >> nFlags;
    rStrm.Ignore( 10 );
    rStrm >> mnRefresh;

    if( ::get_flag( nFlags, EXC_WQSETT_SPECTABLES ) && (meMode == xlWQAllTables) )
        meMode = xlWQSpecTables;
}

void XclImpWebQuery::ReadWqtables( XclImpStream& rStrm )
{
    if( meMode == xlWQSpecTables )
    {
        rStrm.Ignore( 4 );
        String aTables( rStrm.ReadUniString() );

        const sal_Unicode cSep = ';';
        String aQuotedPairs( RTL_CONSTASCII_USTRINGPARAM( "\"\"" ) );
        xub_StrLen nTokenCnt = ScStringUtil::GetQuotedTokenCount( aTables, aQuotedPairs, ',' );
        maTables.Erase();
        xub_StrLen nStringIx = 0;
        for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
        {
            String aToken( ScStringUtil::GetQuotedToken( aTables, 0, aQuotedPairs, ',', nStringIx ) );
            sal_Int32 nTabNum = CharClass::isAsciiNumeric( aToken ) ? aToken.ToInt32() : 0;
            if( nTabNum > 0 )
                ScGlobal::AddToken( maTables, ScfTools::GetNameFromHTMLIndex( static_cast< sal_uInt32 >( nTabNum ) ), cSep );
            else
            {
                ScGlobal::EraseQuotes( aToken, '"', false );
                if( aToken.Len() )
                    ScGlobal::AddToken( maTables, ScfTools::GetNameFromHTMLName( aToken ), cSep );
            }
        }
    }
}

void XclImpWebQuery::Apply( ScDocument& rDoc, const String& rFilterName )
{
    if( maURL.Len() && (meMode != xlWQUnknown) && rDoc.GetDocumentShell() )
    {
        ScAreaLink* pLink = new ScAreaLink( rDoc.GetDocumentShell(),
            maURL, rFilterName, EMPTY_STRING, maTables, maDestRange, mnRefresh * 60UL );
        rDoc.GetLinkManager()->InsertFileLink( *pLink, OBJECT_CLIENT_FILE,
            maURL, &rFilterName, &maTables );
    }
}

// ----------------------------------------------------------------------------

XclImpWebQueryBuffer::XclImpWebQueryBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpWebQueryBuffer::ReadQsi( XclImpStream& rStrm )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm.Ignore( 10 );
        String aXclName( rStrm.ReadUniString() );

        // #i64794# Excel replaces spaces with underscores
        aXclName.SearchAndReplaceAll( ' ', '_' );

        // find the defined name used in Calc
        if( const XclImpName* pName = GetNameManager().FindName( aXclName, GetCurrScTab() ) )
        {
            if( const ScRangeData* pRangeData = pName->GetScRangeData() )
            {
                ScRange aRange;
                if( pRangeData->IsReference( aRange ) )
                    maWQList.push_back( new XclImpWebQuery( aRange ) );
            }
        }
    }
    else
    {
        DBG_ERROR_BIFF();
    }
}

void XclImpWebQueryBuffer::ReadParamqry( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadParamqry( rStrm );
}

void XclImpWebQueryBuffer::ReadWqstring( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqstring( rStrm );
}

void XclImpWebQueryBuffer::ReadWqsettings( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqsettings( rStrm );
}

void XclImpWebQueryBuffer::ReadWqtables( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqtables( rStrm );
}

void XclImpWebQueryBuffer::Apply()
{
    ScDocument& rDoc = GetDoc();
    String aFilterName( RTL_CONSTASCII_USTRINGPARAM( EXC_WEBQRY_FILTER ) );
    for( XclImpWebQueryList::iterator itQuery = maWQList.begin(); itQuery != maWQList.end(); ++itQuery )
        itQuery->Apply( rDoc, aFilterName );
}

// Decryption =================================================================

namespace {

XclImpDecrypterRef lclReadFilepass5( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    OSL_ENSURE( rStrm.GetRecLeft() == 4, "lclReadFilepass5 - wrong record size" );
    if( rStrm.GetRecLeft() == 4 )
    {
        sal_uInt16 nKey(0), nHash(0);
        rStrm >> nKey >> nHash;
        xDecr.reset( new XclImpBiff5Decrypter( nKey, nHash ) );
    }
    return xDecr;
}

XclImpDecrypterRef lclReadFilepass8_Standard( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    OSL_ENSURE( rStrm.GetRecLeft() == 48, "lclReadFilepass8 - wrong record size" );
    if( rStrm.GetRecLeft() == 48 )
    {
        sal_uInt8 pnSalt[ 16 ];
        sal_uInt8 pnVerifier[ 16 ];
        sal_uInt8 pnVerifierHash[ 16 ];
        rStrm.Read( pnSalt, 16 );
        rStrm.Read( pnVerifier, 16 );
        rStrm.Read( pnVerifierHash, 16 );
        xDecr.reset( new XclImpBiff8Decrypter( pnSalt, pnVerifier, pnVerifierHash ) );
    }
    return xDecr;
}

XclImpDecrypterRef lclReadFilepass8_Strong( XclImpStream& /*rStrm*/ )
{
    // not supported
    return XclImpDecrypterRef();
}

XclImpDecrypterRef lclReadFilepass8( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;

    sal_uInt16 nMode(0);
    rStrm >> nMode;
    switch( nMode )
    {
        case EXC_FILEPASS_BIFF5:
            xDecr = lclReadFilepass5( rStrm );
        break;

        case EXC_FILEPASS_BIFF8:
        {
            rStrm.Ignore( 2 );
            sal_uInt16 nSubMode(0);
            rStrm >> nSubMode;
            switch( nSubMode )
            {
                case EXC_FILEPASS_BIFF8_STD:
                    xDecr = lclReadFilepass8_Standard( rStrm );
                break;
                case EXC_FILEPASS_BIFF8_STRONG:
                    xDecr = lclReadFilepass8_Strong( rStrm );
                break;
                default:
                    OSL_FAIL( "lclReadFilepass8 - unknown BIFF8 encryption sub mode" );
            }
        }
        break;

        default:
            OSL_FAIL( "lclReadFilepass8 - unknown encryption mode" );
    }

    return xDecr;
}

} // namespace

// ----------------------------------------------------------------------------

ErrCode XclImpDecryptHelper::ReadFilepass( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    rStrm.DisableDecryption();

    // read the FILEPASS record and create a new decrypter object
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5: xDecr = lclReadFilepass5( rStrm );  break;
        case EXC_BIFF8: xDecr = lclReadFilepass8( rStrm );  break;
        default:        DBG_ERROR_BIFF();
    };

    // set decrypter at import stream
    rStrm.SetDecrypter( xDecr );

    // request and verify a password (decrypter implements IDocPasswordVerifier)
    if( xDecr )
        rStrm.GetRoot().RequestEncryptionData( *xDecr );

    // return error code (success, wrong password, etc.)
    return xDecr ? xDecr->GetError() : EXC_ENCR_ERROR_UNSUPP_CRYPT;
}

// Document protection ========================================================

XclImpDocProtectBuffer::XclImpDocProtectBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnPassHash(0x0000),
    mbDocProtect(false),
    mbWinProtect(false)
{
}

void XclImpDocProtectBuffer::ReadDocProtect( XclImpStream& rStrm )
{
    mbDocProtect = rStrm.ReaduInt16() ? true : false;
}

void XclImpDocProtectBuffer::ReadWinProtect( XclImpStream& rStrm )
{
    mbWinProtect = rStrm.ReaduInt16() ? true : false;
}

void XclImpDocProtectBuffer::ReadPasswordHash( XclImpStream& rStrm )
{
    rStrm.EnableDecryption();
    mnPassHash = rStrm.ReaduInt16();
}

void XclImpDocProtectBuffer::Apply() const
{
    if (!mbDocProtect && !mbWinProtect)
        // Excel requires either the structure or windows protection is set.
        // If neither is set then the document is not protected at all.
        return;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDocProtection> pProtect(new ScDocProtection);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pProtect->setProtected(true);

    if (mnPassHash)
    {
        // 16-bit password pash.
        Sequence<sal_Int8> aPass(2);
        aPass[0] = (mnPassHash >> 8) & 0xFF;
        aPass[1] = mnPassHash & 0xFF;
        pProtect->setPasswordHash(aPass, PASSHASH_XL);
    }

    // document protection options
    pProtect->setOption(ScDocProtection::STRUCTURE, mbDocProtect);
    pProtect->setOption(ScDocProtection::WINDOWS,   mbWinProtect);

    GetDoc().SetDocProtection(pProtect.get());
}

// Sheet Protection ===========================================================

XclImpSheetProtectBuffer::Sheet::Sheet() :
    mbProtected(false),
    mnPasswordHash(0x0000),
    mnOptions(0x4400)
{
}

// ----------------------------------------------------------------------------

XclImpSheetProtectBuffer::Sheet::Sheet(const Sheet& r) :
    mbProtected(r.mbProtected),
    mnPasswordHash(r.mnPasswordHash),
    mnOptions(r.mnOptions)
{
}

XclImpSheetProtectBuffer::XclImpSheetProtectBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpSheetProtectBuffer::ReadProtect( XclImpStream& rStrm, SCTAB nTab )
{
    if ( rStrm.ReaduInt16() )
    {
        Sheet* pSheet = GetSheetItem(nTab);
        if (pSheet)
            pSheet->mbProtected = true;
    }
}

void XclImpSheetProtectBuffer::ReadOptions( XclImpStream& rStrm, SCTAB nTab )
{
    rStrm.Ignore(12);

    // feature type can be either 2 or 4.  If 2, this record stores flag for
    // enhanced protection, whereas if 4 it stores flag for smart tag.
    sal_uInt16 nFeatureType(0);
    rStrm >> nFeatureType;
    if (nFeatureType != 2)
        // We currently only support import of enhanced protection data.
        return;

    rStrm.Ignore(1); // always 1

    // The flag size specifies the size of bytes that follows that stores
    // feature data.  If -1 it depends on the feature type imported earlier.
    // For enhanced protection data, the size is always 4.  For the most xls
    // documents out there this value is almost always -1.
    sal_Int32 nFlagSize(0);
    rStrm >> nFlagSize;
    if (nFlagSize != -1)
        return;

    // There are actually 4 bytes to read, but the upper 2 bytes currently
    // don't store any bits.
    sal_uInt16 nOptions(0);
    rStrm >> nOptions;

    Sheet* pSheet = GetSheetItem(nTab);
    if (pSheet)
        pSheet->mnOptions = nOptions;
}

void XclImpSheetProtectBuffer::ReadPasswordHash( XclImpStream& rStrm, SCTAB nTab )
{
    sal_uInt16 nHash(0);
    rStrm >> nHash;
    Sheet* pSheet = GetSheetItem(nTab);
    if (pSheet)
        pSheet->mnPasswordHash = nHash;
}

void XclImpSheetProtectBuffer::Apply() const
{
    for (ProtectedSheetMap::const_iterator itr = maProtectedSheets.begin(), itrEnd = maProtectedSheets.end();
         itr != itrEnd; ++itr)
    {
        if (!itr->second.mbProtected)
            // This sheet is (for whatever reason) not protected.
            continue;

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScTableProtection> pProtect(new ScTableProtection);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        pProtect->setProtected(true);

        // 16-bit hash password
        const sal_uInt16 nHash = itr->second.mnPasswordHash;
        if (nHash)
        {
            Sequence<sal_Int8> aPass(2);
            aPass[0] = (nHash >> 8) & 0xFF;
            aPass[1] = nHash & 0xFF;
            pProtect->setPasswordHash(aPass, PASSHASH_XL);
        }

        // sheet protection options
        const sal_uInt16 nOptions = itr->second.mnOptions;
        pProtect->setOption( ScTableProtection::OBJECTS,               (nOptions & 0x0001) );
        pProtect->setOption( ScTableProtection::SCENARIOS,             (nOptions & 0x0002) );
        pProtect->setOption( ScTableProtection::FORMAT_CELLS,          (nOptions & 0x0004) );
        pProtect->setOption( ScTableProtection::FORMAT_COLUMNS,        (nOptions & 0x0008) );
        pProtect->setOption( ScTableProtection::FORMAT_ROWS,           (nOptions & 0x0010) );
        pProtect->setOption( ScTableProtection::INSERT_COLUMNS,        (nOptions & 0x0020) );
        pProtect->setOption( ScTableProtection::INSERT_ROWS,           (nOptions & 0x0040) );
        pProtect->setOption( ScTableProtection::INSERT_HYPERLINKS,     (nOptions & 0x0080) );
        pProtect->setOption( ScTableProtection::DELETE_COLUMNS,        (nOptions & 0x0100) );
        pProtect->setOption( ScTableProtection::DELETE_ROWS,           (nOptions & 0x0200) );
        pProtect->setOption( ScTableProtection::SELECT_LOCKED_CELLS,   (nOptions & 0x0400) );
        pProtect->setOption( ScTableProtection::SORT,                  (nOptions & 0x0800) );
        pProtect->setOption( ScTableProtection::AUTOFILTER,            (nOptions & 0x1000) );
        pProtect->setOption( ScTableProtection::PIVOT_TABLES,          (nOptions & 0x2000) );
        pProtect->setOption( ScTableProtection::SELECT_UNLOCKED_CELLS, (nOptions & 0x4000) );

        // all done.  now commit.
        GetDoc().SetTabProtection(itr->first, pProtect.get());
    }
}

XclImpSheetProtectBuffer::Sheet* XclImpSheetProtectBuffer::GetSheetItem( SCTAB nTab )
{
    ProtectedSheetMap::iterator itr = maProtectedSheets.find(nTab);
    if (itr == maProtectedSheets.end())
    {
        // new sheet
        if ( !maProtectedSheets.insert( ProtectedSheetMap::value_type(nTab, Sheet()) ).second )
            return NULL;

        itr = maProtectedSheets.find(nTab);
    }

    return &itr->second;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
