/*************************************************************************
 *
 *  $RCSfile: xehelper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:34:13 $
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
#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
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
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svx/svxfont.hxx>
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


// EditEngine->String conversion ==============================================

/** Helper to create HLINK records during creation of formatted cell strings.
    @descr  In Excel it is not possible to have more than one hyperlink in a cell.
    This helper detects multiple occurences of hyperlinks and fills a string which
    is used to create a cell note containing all URLs. Only cells containing one
    hyperlink are exported as hyperlink cells. */
class XclExpHlinkHelper : protected XclExpRoot
{
public:
    /** @param bCreateHlinkRecs  Controls the behaviour of ProcessUrlField():
        true = Create the HLINK records, false = Only return URL representation. */
    explicit                    XclExpHlinkHelper( const XclExpRoot& rRoot, bool bCreateHlinkRecs );
    /** Appends the note text for multiple hyperlinks to the root data. */
                                ~XclExpHlinkHelper();

    /** Processes the passed URL field (tries to create a HLINK record).
        @return  The representation string of the URL field. */
    String                      ProcessUrlField( const SvxURLField& rUrlField );

private:
    String                      maNoteText;         /// Alternative note text for multiple hyperlinks.
    XclExpHyperlink*&           mrpLastHlink;       /// Reference to last HLINK record.
    bool                        mbCreateHlinkRecs;  /// true = Create HLINKs; false = Just return URL text.
    bool                        mbMultipleHlink;    /// true = Tried to create multiple HLINKs.
};

XclExpHlinkHelper::XclExpHlinkHelper( const XclExpRoot& rRoot, bool bCreateHlinkRecs ) :
    XclExpRoot( rRoot ),
    mrpLastHlink( rRoot.mpRD->pLastHlink ),
    mbCreateHlinkRecs( bCreateHlinkRecs ),
    mbMultipleHlink( false )
{
    DELETEZ( mrpLastHlink );
}

XclExpHlinkHelper::~XclExpHlinkHelper()
{
    /*  Append the note text to 'mpRD->sAddNoteText'. This string will be appended
        to the note of the cell currently exported and cleared afterwards. */
    if( mbCreateHlinkRecs && mbMultipleHlink )
        ScGlobal::AddToken( mpRD->sAddNoteText, maNoteText, '\n', 2 );
}

String XclExpHlinkHelper::ProcessUrlField( const SvxURLField& rUrlField )
{
    String aRepr;
    if( mbCreateHlinkRecs )
    {
        XclExpHyperlink* pNewHlink = new XclExpHyperlink( GetRoot(), rUrlField );
        if( const String* pReprString = pNewHlink->GetRepr() )
            aRepr = *pReprString;

        if( mrpLastHlink )
        {
            // there was already a hyperlink -> delete it, set multiple flag
            mbMultipleHlink = true;
            DELETEZ( mrpLastHlink );
        }
        if( mbMultipleHlink )
            delete pNewHlink;               // multiple -> delete new hyperlink
        else
            mrpLastHlink = pNewHlink;       // pNewHlink was first hyperlink

        // add URL to note text
        ScGlobal::AddToken( maNoteText, rUrlField.GetURL(), '\n' );
    }
    // no hyperlink representation from Excel HLINK record -> use it from text field
    if( !aRepr.Len() )
        aRepr = rUrlField.GetRepresentation();
    // no representation at all -> use URL
    if( !aRepr.Len() )
        aRepr = rUrlField.GetURL();

    return aRepr;
}


// ----------------------------------------------------------------------------

namespace {

/** Creates a new formatted string from an edit engine text object.
    @param rEE  The edit engine in use. The text object must already be set.
    @param nFlags  Modifiers for string export.
    @param nMaxLen  The maximum number of characters to store in this string.
    @param bCreateHlinkRecs  true = Creates HLINK records for hyperlink fields.
    @return  The new string object. */
XclExpStringPtr lclCreateString(
        const XclExpRoot& rRoot, EditEngine& rEE,
        XclStrFlags nFlags, sal_uInt16 nMaxLen, bool bCreateHlinkRecs )
{
    String aXclText;            // complete edit object text
    XclFormatRunVec aFormats;   // the formatting runs

    // helper item set for edit engine -> Calc item conversion
    SfxItemSet aItemSet( *rRoot.GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );

    // font handling
    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    sal_uInt16 nLastFontIx = 0;

    // hyperlink handling - local class, see above
    XclExpHlinkHelper aHlinkHelper( rRoot, bCreateHlinkRecs );

    // process all paragraphs
    sal_uInt16 nParaCount = rEE.GetParagraphCount();
    for( sal_uInt16 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        xub_StrLen nXclParaStart = aXclText.Len();
        String aParaText( rEE.GetText( nPara ) );
        if( aParaText.Len() )
        {
            String aXclParaText;
            SvUShorts aPosList;
            rEE.GetPortions( nPara, aPosList );

            // process all portions in the paragraph
            sal_uInt16 nPosCount = aPosList.Count();
            for( sal_uInt16 nPos = 0; nPos < nPosCount; ++nPos )
            {
                aSel.nEndPos = static_cast< xub_StrLen >( aPosList.GetObject( nPos ) );
                String aXclPortionText( aParaText, aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

                // construct font from current edit engine text portion
                SvxFont aFont;
                aItemSet.ClearItem();
                SfxItemSet aEditSet( rEE.GetAttribs( aSel ) );
                ScPatternAttr::GetFromEditItemSet( aItemSet, aEditSet );
                ScPatternAttr::GetFont( aFont, aItemSet, SC_AUTOCOL_RAW );

                // process text fields
                if( aSel.nStartPos + 1 == aSel.nEndPos )
                {
                    // test if the character is a text field
                    const SfxPoolItem* pItem;
                    if( aEditSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_SET )
                    {
                        const SvxFieldData* pField = static_cast< const SvxFieldItem* >( pItem )->GetField();
                        if( const SvxURLField* pUrlField = PTR_CAST( SvxURLField, pField ) )
                        {
                            aXclPortionText = aHlinkHelper.ProcessUrlField( *pUrlField );
                            // modify font (TODO: correct font attributes?)
                            aFont.SetColor( Color( COL_LIGHTBLUE ) );
                            aFont.SetUnderline( UNDERLINE_SINGLE );
                        }
                        else
                        {
                            DBG_ERRORFILE( "lcl_xehelper_CreateString - unknown text field" );
                            aXclPortionText.Erase();
                        }
                    }
                }

                // test if this contains a super/sub script
                const SvxEscapementItem& rEscapeItem = GETITEM( aEditSet, SvxEscapementItem, EE_CHAR_ESCAPEMENT );
                aFont.SetEscapement( rEscapeItem.GetEsc() );

                // Excel start position of this portion
                xub_StrLen nXclPortionStart = nXclParaStart + aXclParaText.Len();
                // add portion text to Excel string
                aXclParaText.Append( aXclPortionText );

                // insert font into buffer
                sal_uInt16 nFontIx = rFontBuffer.Insert( aFont );
                // current portion font differs from last? -> insert into format run vector
                if( (nXclPortionStart == 0) || ((nFontIx != nLastFontIx) && (nXclPortionStart <= nMaxLen)) )
                {
                    aFormats.push_back( XclFormatRun( static_cast< sal_uInt16 >( nXclPortionStart ), nFontIx ) );
                    nLastFontIx = nFontIx;
                }

                aSel.nStartPos = aSel.nEndPos;
            }

            aXclText.Append( aXclParaText );
        }
        // add trailing newline (important for correct character index calculation)
        if( nPara + 1 < nParaCount )
            aXclText.Append( '\n' );
    }

    return XclExpStringPtr( new XclExpString( aXclText, aFormats, nFlags, nMaxLen ) );
}

} // namespace


// ----------------------------------------------------------------------------

XclExpStringPtr XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const ScEditCell& rEditCell, const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringPtr pString;
    if( const EditTextObject* pEditObj = rEditCell.GetData() )
    {
        // formatted string
        ScEditEngineDefaulter& rEE = rRoot.GetEditEngine();
        BOOL bOldUpdateMode = rEE.GetUpdateMode();
        rEE.SetUpdateMode( TRUE );
        // default items
        const SfxItemSet& rItemSet = pCellAttr ? pCellAttr->GetItemSet() : rRoot.GetDoc().GetDefPattern()->GetItemSet();
        SfxItemSet* pEEItemSet = new SfxItemSet( rEE.GetEmptyItemSet() );
        ScPatternAttr::FillToEditItemSet( *pEEItemSet, rItemSet );
        rEE.SetDefaults( pEEItemSet );      // edit engine takes ownership
        // create the string
        rEE.SetText( *pEditObj );
        pString = lclCreateString( rRoot, rEE, nFlags, nMaxLen, true );
        rEE.SetUpdateMode( bOldUpdateMode );
    }
    else
    {
        // unformatted string
        String aCellText;
        rEditCell.GetString( aCellText );
        pString.reset( new XclExpString( aCellText, nFlags, nMaxLen ) );
    }
    return pString;
}

XclExpStringPtr XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const SdrTextObj& rTextObj,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringPtr pString;
    if( const OutlinerParaObject* pParaObj = rTextObj.GetOutlinerParaObject() )
    {
        EditEngine& rEE = rRoot.GetDrawEditEngine();
        BOOL bOldUpdateMode = rEE.GetUpdateMode();
        rEE.SetUpdateMode( TRUE );
        // create the string
        rEE.SetText( pParaObj->GetTextObject() );
        pString = lclCreateString( rRoot, rEE, nFlags, nMaxLen, false );
        rEE.SetUpdateMode( bOldUpdateMode );
        if( !pString->IsEmpty() )
        {
            pString->LimitFormatCount( EXC_MAXRECSIZE_BIFF8 / 8 - 1 );
            pString->AppendFormat( pString->Len(), EXC_FONT_APP );
        }
    }
    else
    {
        DBG_ERRORFILE( "XclExpStringHelper::CreateString - textbox without para object" );
        pString.reset( new XclExpString );
    }
    return pString;
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
    SfxItemSet aItemSet( *GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );

    // edit engine
    BOOL bOldUpdateMode = mrEE.GetUpdateMode();
    mrEE.SetUpdateMode( TRUE );
    mrEE.SetText( *pTextObj );

    // font information
    XclFontData aFontData, aNewData;
    if( const XclExpFont* pFirstFont = GetFontBuffer().GetFont( EXC_FONT_APP ) )
    {
        aFontData = pFirstFont->GetFontData();
        (aFontData.mnHeight += 10) /= 20;   // using pt here, not twips
    }
    else
        aFontData.mnHeight = 10;

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

                Font aFont;
                aItemSet.ClearItem();
                SfxItemSet aEditSet( mrEE.GetAttribs( aSel ) );
                ScPatternAttr::GetFromEditItemSet( aItemSet, aEditSet );
                ScPatternAttr::GetFont( aFont, aItemSet, SC_AUTOCOL_RAW );

                // font name and style
                aNewData.maName = XclTools::GetXclFontName( aFont.GetName() );
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
                // is calculated wrong in ScPatternAttr::GetFromEditItemSet, because already in twips and not 100thmm
                // -> get it directly from edit engine item set
                aNewData.mnHeight = static_cast< sal_uInt16 >(
                    static_cast< const SvxFontHeightItem& >( aEditSet.Get( EE_CHAR_FONTHEIGHT ) ).GetHeight() );
                (aNewData.mnHeight += 10) /= 20;
                bool bFontHtChanged = (aFontData.mnHeight != aNewData.mnHeight);
                if( bFontHtChanged )
                    aParaText.Append( '&' ).Append( String::CreateFromInt32( aNewData.mnHeight ) );

                // underline
                aNewData.mnUnderline = EXC_FONTUNDERL_NONE;
                switch( aFont.GetUnderline() )
                {
                    case UNDERLINE_NONE:    aNewData.mnUnderline = EXC_FONTUNDERL_NONE;    break;
                    case UNDERLINE_SINGLE:  aNewData.mnUnderline = EXC_FONTUNDERL_SINGLE;  break;
                    case UNDERLINE_DOUBLE:  aNewData.mnUnderline = EXC_FONTUNDERL_DOUBLE;  break;
                    default:                aNewData.mnUnderline = EXC_FONTUNDERL_SINGLE;
                }
                if( aFontData.mnUnderline != aNewData.mnUnderline )
                {
                    sal_uInt8 nTmpUnderl = (aNewData.mnUnderline == EXC_FONTUNDERL_NONE) ?
                        aFontData.mnUnderline : aNewData.mnUnderline;
                    aParaText.AppendAscii( (nTmpUnderl == EXC_FONTUNDERL_SINGLE) ? "&U" : "&E" );
                }

                // strikeout
                aNewData.mbStrikeout = (aFont.GetStrikeout() != STRIKEOUT_NONE);
                if( aFontData.mbStrikeout != aNewData.mbStrikeout )
                    aParaText.AppendAscii( "&S" );

                // super/sub script
                const SvxEscapementItem& rEscapeItem = GETITEM( aEditSet, SvxEscapementItem, EE_CHAR_ESCAPEMENT );
                aNewData.SetScEscapement( rEscapeItem.GetEsc() );
                if( aFontData.mnEscapem != aNewData.mnEscapem )
                {
                    switch(aNewData.mnEscapem)
                    {
                        // close the previous super/sub script.
                        case EXC_FONTESC_NONE:  aParaText.AppendAscii( (aFontData.mnEscapem == EXC_FONTESC_SUPER) ? "&X" : "&Y" ); break;
                        case EXC_FONTESC_SUPER: aParaText.AppendAscii( "&X" );  break;
                        case EXC_FONTESC_SUB:   aParaText.AppendAscii( "&Y" );  break;
                        default: break;
                    }
                }

                aFontData = aNewData;

// --- text content or text fields ---

                const SfxPoolItem* pItem;
                if( (aSel.nStartPos + 1 == aSel.nEndPos) &&     // fields are single characters
                    (aEditSet.GetItemState( EE_FEATURE_FIELD, sal_False, &pItem ) == SFX_ITEM_SET) )
                {
                    if( const SvxFieldData* pFieldData = static_cast< const SvxFieldItem* >( pItem )->GetField() )
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
                        else if( const SvxExtFileField* pFileField = PTR_CAST( SvxExtFileField, pFieldData ) )
                        {
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
                    aPortionText.SearchAndReplaceAll( String( '&' ), String( RTL_CONSTASCII_USTRINGPARAM( "&&" ) ) );
                    // #i17440# space between font height and numbers in text
                    if( bFontHtChanged && aParaText.Len() && aPortionText.Len() )
                    {
                        sal_Unicode cLast = aParaText.GetChar( aParaText.Len() - 1 );
                        sal_Unicode cFirst = aPortionText.GetChar( 0 );
                        if( ('0' <= cLast) && (cLast <= '9') && ('0' <= cFirst) && (cFirst <= '9') )
                            aParaText.Append( ' ' );
                    }
                    aParaText.Append( aPortionText );
                }
            }

            aSel.nStartPos = aSel.nEndPos;
        }

        ScGlobal::AddToken( aText, aParaText, '\n' );
    }

    mrEE.SetUpdateMode( bOldUpdateMode );

    if( aText.Len() )
        rHFString.Append( '&' ).Append( cPortionCode ).Append( aText );
}


// URL conversion =============================================================

namespace {

/** Converts the file URL passed in rUrl to a URL in DOS notation (local or UNC).
    @param rUrl  (in/out-param) In: URL to convert; Out: Converted URL in DOS notation.
    @param rBasePath  Base path for relative URLs.
    @param bSaveRelUrl  Converts to a relative URL, using rBasePath.
    @return  True = Conversion successful, rUrl contains converted file URL. */
bool lclConvertToDos( String& rUrl, const String& rBasePath, bool bSaveRelUrl )
{
    String aDosUrl( INetURLObject( rUrl ).getFSysPath( INetURLObject::FSYS_DOS ) );
    bool bRet = (aDosUrl.Len() > 0);
    if( bRet && bSaveRelUrl )
    {
        // try to convert to relative path
        String aDosBase( INetURLObject( rBasePath ).getFSysPath( INetURLObject::FSYS_DOS ) );
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

/** Encodes special parts of the URL, i.e. directory separators and volume names.
    @param pTableName  Pointer to a table name to be encoded in this URL, or NULL. */
void lclEncodeDosUrl( String& rUrl, const String* pTableName = NULL )
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
    {
        rUrl = pTableName ? EXC_URLSTART_SELFENCODED : EXC_URLSTART_SELF;
    }

    // table name
    if( pTableName )
        rUrl.Append( *pTableName );
}

} // namespace


// ----------------------------------------------------------------------------

String XclExpUrlHelper::EncodeUrl( const XclExpRoot& rRoot, const String& rAbsUrl, const String* pTableName )
{
    String aDosUrl( rAbsUrl );
    if( lclConvertToDos( aDosUrl, rRoot.GetBasePath(), rRoot.IsRelUrl() ) )
        lclEncodeDosUrl( aDosUrl, pTableName );
    return aDosUrl;
}

String XclExpUrlHelper::EncodeDde( const String& rApplic, const String rTopic )
{
    String aDde( rApplic );
    aDde.Append( EXC_DDE_DELIM ).Append( rTopic );
    return aDde;
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

