/*************************************************************************
 *
 *  $RCSfile: xehelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:34 $
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

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif

#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif


#include "root.hxx"


using ::rtl::OUString;


// Byte/Unicode Strings =======================================================

/** All allowed flags for exporting a BIFF2-BIFF7 byte string. */
const XclStrFlags nAllowedFlags27 = EXC_STR_8BITLENGTH | EXC_STR_KEEPZEROCHARS;
/** All allowed flags for export. */
const XclStrFlags nAllowedFlags = nAllowedFlags27 | EXC_STR_FORCEUNICODE | EXC_STR_SMARTFLAGS;


inline XclExpStream& operator<<( XclExpStream& rStrm, const XclFormatRun& rRun )
{
    return rStrm << rRun.mnChar << rRun.mnFontIx;
}


// ----------------------------------------------------------------------------

XclExpString::XclExpString( XclStrFlags nFlags )
{
    Init( nFlags, 0, 0, true );
}

XclExpString::XclExpString( const String& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
}

XclExpString::XclExpString(
    const String& rString, const XclFormatRunVec& rFormats,
    XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, rFormats, nFlags, nMaxLen );
}

XclExpString::XclExpString( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
}

XclExpString::XclExpString(
        const OUString& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, rFormats, nFlags, nMaxLen );
}

XclExpString::~XclExpString()
{
}

void XclExpString::Assign( const String& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( rString.GetBuffer(), nFlags, rString.Len(), nMaxLen );
}

void XclExpString::Assign(
        const String& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
    SetFormats( rFormats );
}

void XclExpString::Assign( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( rString.getStr(), nFlags, rString.getLength(), nMaxLen );
}

void XclExpString::Assign(
        const OUString& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
    SetFormats( rFormats );
}

void XclExpString::Assign( sal_Unicode cChar, XclStrFlags nFlags )
{
    Build( &cChar, nFlags, 1, 1 );
}

void XclExpString::AssignByte(
        const String& rString, CharSet eCharSet, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( ByteString( rString, eCharSet ).GetBuffer(), nFlags, rString.Len(), nMaxLen );
}

void XclExpString::SetFormats( const XclFormatRunVec& rFormats )
{
    DBG_ASSERT( mbIsBiff8, "XclExpString::SetFormats - not allowed for old byte strings" );
    maFormats = rFormats;
#ifdef DBG_UTIL
    if( IsRich() )
    {
        XclFormatRunVec::const_iterator aCurr = maFormats.begin();
        XclFormatRunVec::const_iterator aPrev = aCurr;
        XclFormatRunVec::const_iterator aEnd = maFormats.end();
        for( ++aCurr; aCurr != aEnd; ++aCurr, ++aPrev )
            DBG_ASSERT( aPrev->mnChar < aCurr->mnChar, "XclExpString::SetFormats - invalid char order" );
        DBG_ASSERT( aPrev->mnChar <= mnLen, "XclExpString::SetFormats - invalid char index" );
    }
#endif
}

sal_uInt8 XclExpString::GetFlagField() const
{
    return (mbIsUnicode ? EXC_STRF_16BIT : 0) | (IsRich() ? EXC_STRF_RICH : 0);
}

sal_uInt32 XclExpString::GetBufferSize() const
{
    return mnLen * (mbIsUnicode ? 2 : 1);
}

sal_uInt32 XclExpString::GetSize() const
{
    return
        (mb8BitLen ? 1 : 2) +                           // length field
        (IsWriteFlags() ? 1 : 0) +                      // flag field
        GetBufferSize() +                               // character buffer
        (IsRich() ? (4 * maFormats.size() + 2) : 0);    // richtext formattting
}

void XclExpString::WriteFlagField( XclExpStream& rStrm ) const
{
    if( mbIsBiff8 )
    {
        PrepareWrite( rStrm, 1 );
        rStrm << GetFlagField();
        rStrm.SetSliceSize( 0 );
    }
}

void XclExpString::WriteHeader( XclExpStream& rStrm ) const
{
    DBG_ASSERT( !mb8BitLen || (mnLen < 256), "XclExpString::WriteHeader - string too long" );

    bool bWriteFlags = IsWriteFlags();
    PrepareWrite( rStrm, (mb8BitLen ? 1 : 2) + (bWriteFlags ? 1 : 0) + (IsRich() ? 2 : 0) );

    // length
    if( mb8BitLen )
        rStrm << static_cast< sal_uInt8 >( mnLen );
    else
        rStrm << mnLen;
    // flag field
    if( bWriteFlags )
        rStrm << GetFlagField();
    // format run count
    if( IsRich() )
        rStrm << static_cast< sal_uInt16 >( maFormats.size() );

    rStrm.SetSliceSize( 0 );
}

void XclExpString::WriteBuffer( XclExpStream& rStrm ) const
{
    if( mbIsBiff8 )
        rStrm.WriteUnicodeBuffer( maUniBuffer, GetFlagField() );
    else
        rStrm.WriteCharBuffer( maCharBuffer );
}

void XclExpString::Write( XclExpStream& rStrm ) const
{
    WriteHeader( rStrm );
    WriteBuffer( rStrm );
    if( IsRich() )
    {
        rStrm.SetSliceSize( 4 );
        ::std::for_each( maFormats.begin(), maFormats.end(), XclWriteFunc< XclFormatRun >( rStrm ) );
        rStrm.SetSliceSize( 0 );
    }
}

void XclExpString::WriteBuffer( void* pDest ) const
{
    if( pDest && !IsEmpty() )
    {
        if( mbIsBiff8 )
        {
            sal_uInt8* pDest8 = reinterpret_cast< sal_uInt8* >( pDest );
            for( ScfUInt16Vec::const_iterator aIter = maUniBuffer.begin(), aEnd = maUniBuffer.end(); aIter != aEnd; ++aIter )
            {
                sal_uInt16 nChar = *aIter;
                *pDest8 = static_cast< sal_uInt8 >( nChar );
                ++pDest8;
                if( mbIsUnicode )
                {
                    *pDest8 = static_cast< sal_uInt8 >( nChar >> 8 );
                    ++pDest8;
                }
            }
        }
        else
            memcpy( pDest, &maCharBuffer[ 0 ], mnLen );
    }
}

bool XclExpString::IsWriteFlags() const
{
    return mbIsBiff8 && (!IsEmpty() || !mbSmartFlags);
}

sal_uInt16 XclExpString::CalcStrLen( sal_Int32 nCurrLen, sal_uInt16 nMaxLen )
{
    sal_Int32 nAllowedLen = (mb8BitLen && (nMaxLen > 255)) ? 255 : nMaxLen;
    return static_cast< sal_uInt16 >( ::std::min( ::std::max( nCurrLen, 0L ), nAllowedLen ) );
}

void XclExpString::Init( XclStrFlags nFlags, sal_Int32 nCurrLen, sal_uInt16 nMaxLen, bool bBiff8 )
{
    DBG_ASSERT( (nFlags & ~(bBiff8 ? nAllowedFlags : nAllowedFlags27)) == 0, "XclExpString::Init - unknown flag" );
    mnLen = CalcStrLen( nCurrLen, nMaxLen );
    mbIsBiff8 = bBiff8;
    mbIsUnicode = bBiff8 && ::get_flag( nFlags, EXC_STR_FORCEUNICODE );
    mb8BitLen = ::get_flag( nFlags, EXC_STR_8BITLENGTH );
    mbSmartFlags = bBiff8 && ::get_flag( nFlags, EXC_STR_SMARTFLAGS );
    mbKeepZero = ::get_flag( nFlags, EXC_STR_KEEPZEROCHARS );
    mbWrapped = false;

    maFormats.clear();
    if( mbIsBiff8 )
    {
        maCharBuffer.clear();
        maUniBuffer.resize( mnLen );
    }
    else
    {
        maUniBuffer.clear();
        maCharBuffer.resize( mnLen );
    }
}

void XclExpString::Build( const sal_Unicode* pSource, XclStrFlags nFlags, sal_Int32 nCurrLen, sal_uInt16 nMaxLen )
{
    Init( nFlags, nCurrLen, nMaxLen, true );

    const sal_Unicode* pSrcChar = pSource;
    for( ScfUInt16Vec::iterator aIter = maUniBuffer.begin(), aEnd = maUniBuffer.end(); aIter != aEnd; ++aIter, ++pSrcChar )
    {
        *aIter = static_cast< sal_uInt16 >( (*pSrcChar || mbKeepZero) ? *pSrcChar : '?' );
        if( *aIter & 0xFF00 )
            mbIsUnicode = true;
    }

    mbWrapped = ::std::find( maUniBuffer.begin(), maUniBuffer.end(), EXC_NEWLINE ) != maUniBuffer.end();
}

void XclExpString::Build( const sal_Char* pSource, XclStrFlags nFlags, sal_Int32 nCurrLen, sal_uInt16 nMaxLen )
{
    Init( nFlags, nCurrLen, nMaxLen, false );

    const sal_Char* pSrcChar = pSource;
    for( ScfUInt8Vec::iterator aIter = maCharBuffer.begin(), aEnd = maCharBuffer.end(); aIter != aEnd; ++aIter, ++pSrcChar )
        *aIter = static_cast< sal_uInt8 >( (*pSrcChar || mbKeepZero) ? *pSrcChar : '?' );

    mbIsUnicode = false;
    mbWrapped = ::std::find( maCharBuffer.begin(), maCharBuffer.end(), EXC_NEWLINE_CHAR ) != maCharBuffer.end();
}

void XclExpString::PrepareWrite( XclExpStream& rStrm, sal_uInt32 nBytes ) const
{
    rStrm.SetSliceSize( nBytes + (mbIsUnicode ? 2 : 1) );
}


// EditEngine->String conversion ==============================================

XclExpString* XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot,
        const EditTextObject& rTextObj,
        const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags,
        sal_uInt16 nMaxLen )
{
    String aText;               // complete edit object text
    String aNoteText;           // cell note text for multiple hyperlinks
    XclFormatRunVec aFormats;   // the formatting runs

    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    ScPatternAttr aAttr( rRoot.GetDoc().GetPool() );

    ScEditEngineDefaulter& rEE = rRoot.GetEditEngine();
    BOOL bOldUpdateMode = rEE.GetUpdateMode();
    rEE.SetUpdateMode( TRUE );
    // set default item set on edit engine
    const ScPatternAttr& rAttr = pCellAttr ? *pCellAttr : *rRoot.GetDoc().GetDefPattern();
    SfxItemSet* pDefaultSet = new SfxItemSet( rEE.GetEmptyItemSet() );
    rAttr.FillEditItemSet( pDefaultSet );
    rEE.SetDefaults( pDefaultSet );     // edit engine takes ownership

    rEE.SetText( rTextObj );

    // hyperlink handling
    XclExpHyperlink*& rpLastHlink = rRoot.mpRD->pLastHlink;
    DELETEZ( rpLastHlink );
    bool bMultipleHlink = false;

    // first font is the cell font, following font changes are stored in richstring
    sal_uInt16 nLastFontIx = rFontBuffer.Insert( rAttr );

    sal_uInt16 nParaCount = rEE.GetParagraphCount();
    for( sal_uInt16 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        xub_StrLen nParaStartPos = aText.Len();
        String aParaText( rEE.GetText( nPara ) );
        if( aParaText.Len() )
        {
            String aXclParaText;
            SvUShorts aPosList;
            rEE.GetPortions( nPara, aPosList );

            sal_uInt16 nPosCount = aPosList.Count();
            for( sal_uInt16 nPos = 0; nPos < nPosCount; ++nPos )
            {
                aSel.nEndPos = static_cast< xub_StrLen >( aPosList.GetObject( nPos ) );
                xub_StrLen nXclStartPos = nParaStartPos + aXclParaText.Len();
                aXclParaText += aParaText.Copy( aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

                // construct font from current edit engine text portion
                SfxItemSet aItemSet( rEE.GetAttribs( aSel ) );
                aAttr.GetItemSet().ClearItem();
                aAttr.GetFromEditItemSet( &aItemSet );
                Font aFont;
                aAttr.GetFont( aFont, SC_AUTOCOL_RAW );

                // detect hyperlinks, export single hyperlink, create note if multiple
                // hyperlinks are present, but always export hyperlink text
                if( aSel.nStartPos + 1 == aSel.nEndPos )
                {
                    const SfxPoolItem* pItem;
                    if( aItemSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_SET )
                    {
                        const SvxFieldData* pField = static_cast< const SvxFieldItem* >( pItem )->GetField();
                        if( pField && pField->ISA( SvxURLField ) )
                        {
                            // create new Excel hyperlink and add URL to cell text
                            const SvxURLField& rURLField = static_cast< const SvxURLField& >( *pField );
                            XclExpHyperlink* pNewHlink = new XclExpHyperlink( rRoot, rURLField );
                            const String* pReprString = pNewHlink->GetRepr();
                            if( pReprString )
                            {
                                aXclParaText.Erase( aXclParaText.Len() - 1 );
                                aXclParaText += *pReprString;
                            }

                            if( rpLastHlink )
                            {
                                bMultipleHlink = true;
                                DELETEZ( rpLastHlink );
                            }
                            if( bMultipleHlink )
                                delete pNewHlink;
                            else
                                rpLastHlink = pNewHlink;

                            // add URL to note text
                            ScfTools::AddToken( aNoteText, rURLField.GetURL(), EXC_NEWLINE );

                            // modify font (TODO: correct font attributes?)
                            aFont.SetColor( Color( COL_LIGHTBLUE ) );
                            aFont.SetUnderline( UNDERLINE_SINGLE );
                        }
                    }
                }

                // insert font into buffer and formatting run vector
                sal_uInt16 nFontIx = rFontBuffer.Insert( aFont );
                if( (nFontIx != nLastFontIx) && (nXclStartPos <= nMaxLen) )
                {
                    aFormats.push_back( XclFormatRun( static_cast< sal_uInt16 >( nXclStartPos ), nFontIx ) );
                    nLastFontIx = nFontIx;
                }

                aSel.nStartPos = aSel.nEndPos;
            }

            // add current paragraph and trailing newline
            // (important for correct character index calculation)
            aText += aXclParaText;
            if( nPara + 1 < nParaCount )
                aText += EXC_NEWLINE;
        }
    }

    rEE.SetUpdateMode( bOldUpdateMode );

    // multiple hyperlinks: append all URLs to note text
    if( bMultipleHlink && aNoteText.Len() )
    {
        if( rRoot.mpRD->sAddNoteText.Len() )
            rRoot.mpRD->sAddNoteText.Append( EXC_NEWLINE ).Append( EXC_NEWLINE );
        rRoot.mpRD->sAddNoteText += aNoteText;
    }

    return new XclExpString( aText, aFormats, nFlags, nMaxLen );
}

XclExpString* XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot,
        const ScEditCell& rEditCell,
        const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags,
        sal_uInt16 nMaxLen )
{
    const EditTextObject* pEditObj = rEditCell.GetData();
    if( pEditObj )
        return CreateString( rRoot, *pEditObj, pCellAttr, nFlags, nMaxLen );

    // create unformatted string
    String aCellText;
    rEditCell.GetString( aCellText );
    return new XclExpString( aCellText, nFlags, nMaxLen );
}


// Header/footer conversion ===================================================

XclExpHFConverter::XclExpHFConverter( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mrEE( rRoot.GetHFEditEngine() )
{
}

String XclExpHFConverter::GenerateString(
        const EditTextObject* pLeftObj,
        const EditTextObject* pCenterObj,
        const EditTextObject* pRightObj )
{
    String aHFString;
    AppendPortion( aHFString, pLeftObj, 'L' );
    AppendPortion( aHFString, pCenterObj, 'C' );
    AppendPortion( aHFString, pRightObj, 'R' );
    return aHFString;
}

void XclExpHFConverter::AppendPortion( String& rHFString, const EditTextObject* pTextObj, sal_Unicode cPortionCode )
{
    if( !pTextObj ) return;

    String aText;
    ScPatternAttr aAttr( GetDoc().GetPool() );

    // edit engine
    BOOL bOldUpdateMode = mrEE.GetUpdateMode();
    mrEE.SetUpdateMode( TRUE );
    mrEE.SetText( *pTextObj );

    // font information
    const XclExpFont* pFirstFont = GetFontBuffer().GetFont( 0 );
    XclFontData aFontData, aNewData;
    if( pFirstFont )
        aFontData = pFirstFont->GetFontData();
    else
        aFontData.mnHeight = 10;    // using pt here, not twips

    const FontList* pFontList = NULL;
    if( SfxObjectShell* pDocShell = GetDocShell() )
    {
        if( const SvxFontListItem* pInfoItem = static_cast< const SvxFontListItem* >(
                pDocShell->GetItem( SID_ATTR_CHAR_FONTLIST ) ) )
            pFontList = pInfoItem->GetFontList();
    }

    sal_uInt16 nParaCount = mrEE.GetParagraphCount();
    for( sal_uInt16 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        String aParaText;
        SvUShorts aPosList;
        mrEE.GetPortions( nPara, aPosList );

        sal_uInt16 nPosCount = aPosList.Count();
        for( sal_uInt16 nPos = 0; nPos < nPosCount; ++nPos )
        {
            aSel.nEndPos = static_cast< xub_StrLen >( aPosList.GetObject( nPos ) );
            if( aSel.nStartPos < aSel.nEndPos )
            {

// --- font attributes ---

                SfxItemSet aItemSet( mrEE.GetAttribs( aSel ) );
                aAttr.GetItemSet().ClearItem();
                aAttr.GetFromEditItemSet( &aItemSet );
                Font aFont;
                aAttr.GetFont( aFont, SC_AUTOCOL_RAW );

                // font name and style
                aNewData.maName = aFont.GetName();
                aNewData.mnWeight = (aFont.GetWeight() > WEIGHT_NORMAL) ? EXC_FONTWGHT_BOLD : EXC_FONTWGHT_NORMAL;
                aNewData.mbItalic = (aFont.GetItalic() != ITALIC_NONE);
                bool bNewFont = !(aFontData.maName == aNewData.maName);
                bool bNewStyle = (aFontData.mnWeight != aNewData.mnWeight) ||
                                 (aFontData.mbItalic != aNewData.mbItalic);
                if( bNewFont || (bNewStyle && pFontList) )
                {
                    aParaText.AppendAscii( "&\"" ).Append( aNewData.maName );
                    if( pFontList )
                    {
                        FontInfo aFontInfo( pFontList->Get(
                            aNewData.maName,
                            (aNewData.mnWeight > EXC_FONTWGHT_NORMAL) ? WEIGHT_BOLD : WEIGHT_NORMAL,
                            aNewData.mbItalic ? ITALIC_NORMAL : ITALIC_NONE ) );
                        aNewData.maStyle = pFontList->GetStyleName( aFontInfo );
                        if( aNewData.maStyle.Len() )
                            aParaText.Append( ',' ).Append( aNewData.maStyle );
                    }
                    aParaText.Append( '"' );
                }

                // height
                aNewData.mnHeight = static_cast< sal_uInt16 >(
                    static_cast< const SvxFontHeightItem& >( aItemSet.Get( EE_CHAR_FONTHEIGHT ) ).GetHeight() );
                (aNewData.mnHeight += 10) /= 20;
                if( aFontData.mnHeight != aNewData.mnHeight )
                    aParaText.Append( '&' ).Append( String::CreateFromInt32( aNewData.mnHeight ) );

                // underline
                aNewData.meUnderline = xlUnderlNone;
                switch( aFont.GetUnderline() )
                {
                    case UNDERLINE_NONE:    aNewData.meUnderline = xlUnderlNone;    break;
                    case UNDERLINE_SINGLE:  aNewData.meUnderline = xlUnderlSingle;  break;
                    case UNDERLINE_DOUBLE:  aNewData.meUnderline = xlUnderlDouble;  break;
                    default:                aNewData.meUnderline = xlUnderlSingle;
                }
                if( aFontData.meUnderline != aNewData.meUnderline )
                {
                    XclUnderline eTmpUnderl = (aNewData.meUnderline == xlUnderlNone) ?
                        aFontData.meUnderline : aNewData.meUnderline;
                    aParaText.AppendAscii( (eTmpUnderl == xlUnderlSingle) ? "&U" : "&E" );
                }

                // strikeout
                aNewData.mbStrikeout = (aFont.GetStrikeout() != STRIKEOUT_NONE);
                if( aFontData.mbStrikeout != aNewData.mbStrikeout )
                    aParaText.AppendAscii( "&S" );

                aFontData = aNewData;

// --- text content or text fields ---

                const SfxPoolItem* pItem;
                if( (aSel.nStartPos + 1 == aSel.nEndPos) &&     // fields are single characters
                    (aItemSet.GetItemState( EE_FEATURE_FIELD, sal_False, &pItem ) == SFX_ITEM_SET) )
                {
                    const SvxFieldData* pFieldData = static_cast< const SvxFieldItem* >( pItem )->GetField();
                    if( pFieldData )
                    {
                        if( pFieldData->ISA( SvxPageField ) )
                            aParaText.AppendAscii( "&P" );
                        else if( pFieldData->ISA( SvxPagesField ) )
                            aParaText.AppendAscii( "&N" );
                        else if( pFieldData->ISA( SvxDateField ) )
                            aParaText.AppendAscii( "&D" );
                        else if( pFieldData->ISA( SvxTimeField ) || pFieldData->ISA( SvxExtTimeField ) )
                            aParaText.AppendAscii( "&T" );
                        else if( pFieldData->ISA( SvxTableField ) )
                            aParaText.AppendAscii( "&A" );
                        else if( pFieldData->ISA( SvxFileField ) )  // title -> file name
                            aParaText.AppendAscii( "&F" );
                        else if( pFieldData->ISA( SvxExtFileField ) )
                        {
                            const SvxExtFileField* pFileField = static_cast< const SvxExtFileField* >( pFieldData );
                            switch( pFileField->GetFormat() )
                            {
                                case SVXFILEFORMAT_NAME_EXT:
                                case SVXFILEFORMAT_NAME:
                                    aParaText.AppendAscii( "&F" );
                                break;
                                case SVXFILEFORMAT_PATH:
                                    aParaText.AppendAscii( "&Z" );
                                break;
                                case SVXFILEFORMAT_FULLPATH:
                                    aParaText.AppendAscii( "&Z&F" );
                                break;
                                default:
                                    DBG_ERRORFILE( "XclExpHFConverter::AppendPortion - unknown file field" );
                            }
                        }
                    }
                }
                else
                {
                    String aPortionText( mrEE.GetText( aSel ) );
                    aPortionText.SearchAndReplaceAll( String( '&' ), String::CreateFromAscii( "&&" ) );
                    aParaText.Append( aPortionText );
                }
            }

            aSel.nStartPos = aSel.nEndPos;
        }

        ScfTools::AddToken( aText, aParaText, EXC_NEWLINE );
    }

    mrEE.SetUpdateMode( bOldUpdateMode );

    if( aText.Len() )
        rHFString.Append( '&' ).Append( cPortionCode ).Append( aText );
}


// URL conversion =============================================================

String XclExpUrlHelper::EncodeUrl( const XclExpRoot& rRoot, const String& rAbsUrl, const String* pTableName )
{
    String aUrl( rAbsUrl );
    if( ConvertToDos( aUrl, rRoot ) )
        EncodeUrl_Impl( aUrl, pTableName );
    return aUrl;
}

String XclExpUrlHelper::EncodeDde( const String& rApplic, const String rTopic )
{
    String aDde( rApplic );
    aDde.Append( EXC_DDE_DELIM ).Append( rTopic );
    return aDde;
}

bool XclExpUrlHelper::ConvertToDos( String& rUrl, const XclExpRoot& rRoot )
{
    String aDosUrl( INetURLObject( rUrl ).getFSysPath( INetURLObject::FSYS_DOS ) );
    bool bRet = (aDosUrl.Len() > 0);
    if( bRet && rRoot.IsRelUrl() )
    {
        // try to convert to relative path
        String aDosBase( INetURLObject( rRoot.GetBasePath() ).getFSysPath( INetURLObject::FSYS_DOS ) );
        if( aDosBase.Len() )
        {
            xub_StrLen nPos;

            // --- 1st step: delete equal subdirectories ---

            // special handling for UNC
            xub_StrLen nStartSearch = aDosBase.EqualsAscii( "\\\\", 0, 2 ) ? 2 : 0;
            bool bEqualBase = false;
            bool bLoop = true;
            while( bLoop && ((nPos = aDosBase.Search( '\\', nStartSearch )) != STRING_NOTFOUND) )
            {
                bLoop = (TRUE == aDosBase.Equals( aDosUrl, 0, nPos + 1 ));
                if( bLoop )
                {
                    aDosBase.Erase( 0, nPos + 1 );
                    aDosUrl.Erase( 0, nPos + 1 );
                    nStartSearch = 0;
                    bEqualBase = true;
                }
            }

            // --- 2nd step: add parent directory levels ---

            if( bEqualBase )
            {
                while( (nPos = aDosBase.Search( '\\' )) != STRING_NOTFOUND )
                {
                    aDosBase.Erase( 0, nPos + 1 );
                    aDosUrl.InsertAscii( "..\\", 0 );
                }
            }
        }
        rUrl = aDosUrl;
    }
    return bRet;
}

void XclExpUrlHelper::EncodeUrl_Impl( String& rUrl, const String* pTableName = NULL )
{
    if( rUrl.Len() )
    {
        String aOldUrl( rUrl );
        rUrl = EXC_URLSTART_ENCODED;

        if( (aOldUrl.Len() > 2) && aOldUrl.EqualsAscii( "\\\\", 0, 2 ) )
        {
            // UNC
            rUrl.Append( EXC_URL_DOSDRIVE ).Append( '@' );
            aOldUrl.Erase( 0, 2 );
        }
        else if( (aOldUrl.Len() > 2) && aOldUrl.EqualsAscii( ":\\", 1, 2 ) )
        {
            // drive letter
            rUrl.Append( EXC_URL_DOSDRIVE ).Append( aOldUrl.GetChar( 0 ) );
            aOldUrl.Erase( 0, 3 );
        }

        // directories
        xub_StrLen nPos;
        while( (nPos = aOldUrl.Search( '\\' )) != STRING_NOTFOUND )
        {
            if( aOldUrl.EqualsAscii( "..", 0, 2 ) )
                rUrl.Append( EXC_URL_PARENTDIR );   // parent dir
            else
                rUrl.Append( aOldUrl.GetBuffer(), nPos ).Append( EXC_URL_SUBDIR );
            aOldUrl.Erase( 0, nPos + 1 );
        }

        // file name
        if( pTableName )    // enclose file name in brackets if table name follows
            rUrl.Append( '[' ).Append( aOldUrl ).Append( ']' );
        else
            rUrl.Append( aOldUrl );
    }
    else    // empty URL -> self reference
        rUrl = pTableName ? EXC_URLSTART_SELFENCODED : EXC_URLSTART_SELF;

    // table name
    if( pTableName )
        rUrl.Append( *pTableName );
}


// Cached Value Lists =========================================================

XclExpCachedValue::~XclExpCachedValue()
{
}


// ----------------------------------------------------------------------------

sal_uInt32 XclExpCachedDouble::GetSize() const
{
    return 9;
}

void XclExpCachedDouble::Save( XclExpStream& rStrm ) const
{
    rStrm.SetSliceSize( 9 );
    rStrm << EXC_CACHEDVAL_DOUBLE << mfVal;
}


// ----------------------------------------------------------------------------

XclExpCachedString::XclExpCachedString( const String& rStr, XclStrFlags nFlags ) :
    maStr( rStr, nFlags )
{
}

sal_uInt32 XclExpCachedString::GetSize() const
{
    return 1 + maStr.GetSize();
}

void XclExpCachedString::Save( XclExpStream& rStrm ) const
{
    rStrm.SetSliceSize( 6 );
    rStrm << EXC_CACHEDVAL_STRING << maStr;
}


// ----------------------------------------------------------------------------

XclExpCachedMatrix::XclExpCachedMatrix(
        ScDocument& rDoc,
        sal_uInt16 nCols, sal_uInt16 nRows,
        const ScMatrix* pMatrix,
        XclStrFlags nFlags ) :
    mnCols( nCols ),
    mnRows( nRows )
{
    DBG_ASSERT( pMatrix && mnCols && mnRows, "XclExpCachedMatrix::XclExpCachedMatrix - missing matrix" );
    DBG_ASSERT( mnCols <= 256, "XclExpCachedMatrix::XclExpCachedMatrix - too many columns" );

    String aString;
    double fValue;
    BOOL bIsString;
    for( sal_uInt16 nRow = 0; nRow < mnRows; ++nRow )
    {
        for( sal_uInt16 nCol = 0; nCol < mnCols; ++nCol )
        {
            if( rDoc.GetDdeLinkResult( pMatrix, nCol, nRow, aString, fValue, bIsString ) )
                // return value "true" means empty result
                Append( EMPTY_STRING, nFlags );
            else if( bIsString )
                Append( aString, nFlags );
            else
                Append( fValue );
        }
    }
}

sal_uInt32 XclExpCachedMatrix::GetSize() const
{
    sal_uInt32 nSize = 3;
    for( XclExpCachedValue* pVal = maValueList.First(); pVal; pVal = maValueList.Next() )
        nSize += pVal->GetSize();
    return nSize;
}

void XclExpCachedMatrix::Save( XclExpStream& rStrm ) const
{
    if( rStrm.GetRoot().GetBiff() < xlBiff8 )
        // 256 columns are saved as 0 columns
        rStrm << static_cast< sal_uInt8 >( mnCols ) << mnRows;
    else
        rStrm << static_cast< sal_uInt8 >( mnCols - 1 ) << static_cast< sal_uInt16 >( mnRows - 1 );
    for( XclExpCachedValue* pVal = maValueList.First(); pVal; pVal = maValueList.Next() )
        rStrm << *pVal;
}

void XclExpCachedMatrix::Append( double fVal )
{
    maValueList.Append( new XclExpCachedDouble( fVal ) );
}

void XclExpCachedMatrix::Append( const String& rStr, XclStrFlags nFlags )
{
    maValueList.Append( new XclExpCachedString( rStr, nFlags ) );
}


// ============================================================================

