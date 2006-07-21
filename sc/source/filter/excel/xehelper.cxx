/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xehelper.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:56:27 $
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
#include "precompiled_sc.hxx"

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
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
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
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

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::i18n::XBreakIterator;

// Export progress bar ========================================================

XclExpProgressBar::XclExpProgressBar( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mxProgress( new ScfProgressBar( rRoot.GetDocShell(), STR_SAVE_DOC ) ),
    mpSubProgress( 0 ),
    mpSubRowCreate( 0 ),
    mpSubRowFinal( 0 ),
    mnSegRowFinal( SCF_INV_SEGMENT ),
    mnRowCount( 0 )
{
}

XclExpProgressBar::~XclExpProgressBar()
{
}

void XclExpProgressBar::Initialize()
{
    const ScDocument& rDoc = GetDoc();
    const XclExpTabInfo& rTabInfo = GetTabInfo();
    SCTAB nScTabCount = rTabInfo.GetScTabCount();

    // *** segment: creation of ROW records *** -------------------------------

    sal_Int32 nSegRowCreate = mxProgress->AddSegment( 2000 );
    mpSubRowCreate = &mxProgress->GetSegmentProgressBar( nSegRowCreate );
    maSubSegRowCreate.resize( nScTabCount, SCF_INV_SEGMENT );

    for( SCTAB nScTab = 0; nScTab < nScTabCount; ++nScTab )
    {
        if( rTabInfo.IsExportTab( nScTab ) )
        {
            SCCOL nLastUsedScCol;
            SCROW nLastUsedScRow;
            rDoc.GetTableArea( nScTab, nLastUsedScCol, nLastUsedScRow );
            sal_Size nSegSize = static_cast< sal_Size >( nLastUsedScRow + 1 );
            maSubSegRowCreate[ nScTab ] = mpSubRowCreate->AddSegment( nSegSize );
        }
    }

    // *** segment: writing all ROW records *** -------------------------------

    mnSegRowFinal = mxProgress->AddSegment( 1000 );
    // sub progress bar and segment are created later in ActivateFinalRowsSegment()
}

void XclExpProgressBar::IncRowRecordCount()
{
    ++mnRowCount;
}

void XclExpProgressBar::ActivateCreateRowsSegment()
{
    DBG_ASSERT( (0 <= GetCurrScTab()) && (GetCurrScTab() < GetTabInfo().GetScTabCount()),
        "XclExpProgressBar::ActivateCreateRowsSegment - invalid sheet" );
    sal_Int32 nSeg = maSubSegRowCreate[ GetCurrScTab() ];
    DBG_ASSERT( nSeg != SCF_INV_SEGMENT, "XclExpProgressBar::ActivateCreateRowsSegment - invalid segment" );
    if( nSeg != SCF_INV_SEGMENT )
    {
        mpSubProgress = mpSubRowCreate;
        mpSubProgress->ActivateSegment( nSeg );
    }
    else
        mpSubProgress = 0;
}

void XclExpProgressBar::ActivateFinalRowsSegment()
{
    if( !mpSubRowFinal && (mnRowCount > 0) )
    {
        mpSubRowFinal = &mxProgress->GetSegmentProgressBar( mnSegRowFinal );
        mpSubRowFinal->AddSegment( mnRowCount );
    }
    mpSubProgress = mpSubRowFinal;
    if( mpSubProgress )
        mpSubProgress->Activate();
}

void XclExpProgressBar::Progress()
{
    if( mpSubProgress && !mpSubProgress->IsFull() )
        mpSubProgress->Progress();
}

// Calc->Excel cell address/range conversion ==================================

namespace {

/** Fills the passed Excel address with the passed Calc cell coordinates without checking any limits. */
inline void lclFillAddress( XclAddress& rXclPos, SCCOL nScCol, SCROW nScRow )
{
    rXclPos.mnCol = static_cast< sal_uInt16 >( nScCol );
    rXclPos.mnRow = static_cast< sal_uInt16 >( nScRow );
}

} // namespace

// ----------------------------------------------------------------------------

XclExpAddressConverter::XclExpAddressConverter( const XclExpRoot& rRoot ) :
    XclAddressConverterBase( rRoot.GetTracer(), rRoot.GetXclMaxPos() )
{
}

// cell address ---------------------------------------------------------------

bool XclExpAddressConverter::CheckAddress( const ScAddress& rScPos, bool bWarn )
{
    // ScAddress::operator<=() doesn't do what we want here
    bool bValidCol = (0 <= rScPos.Col()) && (rScPos.Col() <= maMaxPos.Col());
    bool bValidRow = (0 <= rScPos.Row()) && (rScPos.Row() <= maMaxPos.Row());
    bool bValidTab = (0 <= rScPos.Tab()) && (rScPos.Tab() <= maMaxPos.Tab());

    bool bValid = bValidCol && bValidRow && bValidTab;
    if( !bValid && bWarn )
    {
        mbColTrunc |= !bValidCol;
        mbRowTrunc |= !bValidRow;
        mbTabTrunc |= (rScPos.Tab() > maMaxPos.Tab());  // do not warn for deleted refs
        mrTracer.TraceInvalidAddress( rScPos, maMaxPos );
    }
    return bValid;
}

bool XclExpAddressConverter::ConvertAddress( XclAddress& rXclPos,
        const ScAddress& rScPos, bool bWarn )
{
    bool bValid = CheckAddress( rScPos, bWarn );
    if( bValid )
        lclFillAddress( rXclPos, rScPos.Col(), rScPos.Row() );
    return bValid;
}

XclAddress XclExpAddressConverter::CreateValidAddress( const ScAddress& rScPos, bool bWarn )
{
    XclAddress aXclPos( ScAddress::UNINITIALIZED );
    if( !ConvertAddress( aXclPos, rScPos, bWarn ) )
        lclFillAddress( aXclPos, ::std::min( rScPos.Col(), maMaxPos.Col() ), ::std::min( rScPos.Row(), maMaxPos.Row() ) );
    return aXclPos;
}

// cell range -----------------------------------------------------------------

bool XclExpAddressConverter::CheckRange( const ScRange& rScRange, bool bWarn )
{
    return CheckAddress( rScRange.aStart, bWarn ) && CheckAddress( rScRange.aEnd, bWarn );
}

bool XclExpAddressConverter::ValidateRange( ScRange& rScRange, bool bWarn )
{
    rScRange.Justify();

    // check start position
    bool bValidStart = CheckAddress( rScRange.aStart, bWarn );
    if( bValidStart )
    {
        // check & correct end position
        ScAddress& rScEnd = rScRange.aEnd;
        if( !CheckAddress( rScEnd, bWarn ) )
        {
            rScEnd.SetCol( ::std::min( rScEnd.Col(), maMaxPos.Col() ) );
            rScEnd.SetRow( ::std::min( rScEnd.Row(), maMaxPos.Row() ) );
            rScEnd.SetTab( ::std::min( rScEnd.Tab(), maMaxPos.Tab() ) );
        }
    }

    return bValidStart;
}

bool XclExpAddressConverter::ConvertRange( XclRange& rXclRange,
        const ScRange& rScRange, bool bWarn )
{
    // check start position
    bool bValidStart = CheckAddress( rScRange.aStart, bWarn );
    if( bValidStart )
    {
        lclFillAddress( rXclRange.maFirst, rScRange.aStart.Col(), rScRange.aStart.Row() );

        // check & correct end position
        SCCOL nScCol2 = rScRange.aEnd.Col();
        SCROW nScRow2 = rScRange.aEnd.Row();
        if( !CheckAddress( rScRange.aEnd, bWarn ) )
        {
            nScCol2 = ::std::min( nScCol2, maMaxPos.Col() );
            nScRow2 = ::std::min( nScRow2, maMaxPos.Row() );
        }
        lclFillAddress( rXclRange.maLast, nScCol2, nScRow2 );
    }
    return bValidStart;
}

XclRange XclExpAddressConverter::CreateValidRange( const ScRange& rScRange, bool bWarn )
{
    return XclRange(
        CreateValidAddress( rScRange.aStart, bWarn ),
        CreateValidAddress( rScRange.aEnd, bWarn ) );
}

// cell range list ------------------------------------------------------------

bool XclExpAddressConverter::CheckRangeList( const ScRangeList& rScRanges, bool bWarn )
{
    for( ULONG nIdx = 0, nSize = rScRanges.Count(); nIdx < nSize; ++nIdx )
        if( const ScRange* pScRange = rScRanges.GetObject( nIdx ) )
            if( !CheckRange( *pScRange, bWarn ) )
                return false;
    return true;
}

void XclExpAddressConverter::ValidateRangeList( ScRangeList& rScRanges, bool bWarn )
{
    ULONG nIdx = rScRanges.Count();
    while( nIdx )
    {
        --nIdx; // backwards to keep nIdx valid
        ScRange* pScRange = rScRanges.GetObject( nIdx );
        if( pScRange && !CheckRange( *pScRange, bWarn ) )
            delete rScRanges.Remove( nIdx );
    }
}

void XclExpAddressConverter::ConvertRangeList( XclRangeList& rXclRanges,
        const ScRangeList& rScRanges, bool bWarn )
{
    rXclRanges.clear();
    for( ULONG nPos = 0, nCount = rScRanges.Count(); nPos < nCount; ++nPos )
    {
        if( const ScRange* pScRange = rScRanges.GetObject( nPos ) )
        {
            XclRange aXclRange( ScAddress::UNINITIALIZED );
            if( ConvertRange( aXclRange, *pScRange, bWarn ) )
                rXclRanges.push_back( aXclRange );
        }
    }
}

// EditEngine->String conversion ==============================================

namespace {

String lclGetUrlRepresentation( const SvxURLField& rUrlField )
{
    String aRepr( rUrlField.GetRepresentation() );
    // no representation -> use URL
    return aRepr.Len() ? aRepr : rUrlField.GetURL();
}

} // namespace

// ----------------------------------------------------------------------------

XclExpHyperlinkHelper::XclExpHyperlinkHelper( const XclExpRoot& rRoot, const ScAddress& rScPos ) :
    XclExpRoot( rRoot ),
    maScPos( rScPos ),
    mbMultipleUrls( false )
{
}

XclExpHyperlinkHelper::~XclExpHyperlinkHelper()
{
}

String XclExpHyperlinkHelper::ProcessUrlField( const SvxURLField& rUrlField )
{
    String aUrlRepr;

    if( GetBiff() == EXC_BIFF8 )    // no HLINK records in BIFF2-BIFF7
    {
        // there was/is already a HLINK record
        mbMultipleUrls = mxLinkRec.is();

        mxLinkRec.reset( new XclExpHyperlink( GetRoot(), rUrlField, maScPos ) );

        if( const String* pRepr = mxLinkRec->GetRepr() )
            aUrlRepr = *pRepr;

        // add URL to note text
        ScGlobal::AddToken( maUrlList, rUrlField.GetURL(), '\n' );
    }

    // no hyperlink representation from Excel HLINK record -> use it from text field
    return aUrlRepr.Len() ? aUrlRepr : lclGetUrlRepresentation( rUrlField );
}

bool XclExpHyperlinkHelper::HasLinkRecord() const
{
    return !mbMultipleUrls && mxLinkRec.is();
}

XclExpHyperlinkHelper::XclExpHyperlinkRef XclExpHyperlinkHelper::GetLinkRecord()
{
    if( HasLinkRecord() )
        return mxLinkRec;
    return XclExpHyperlinkRef();
}

// ----------------------------------------------------------------------------

namespace {

/** Creates a new formatted string from the passed unformatted string.

    Creates a Unicode string or a byte string, depending on the current BIFF
    version contained in the passed XclExpRoot object. May create a formatted
    string object, if the text contains different script types.

    @param pCellAttr
        Cell attributes used for font formatting.
    @param nFlags
        Modifiers for string export.
    @param nMaxLen
        The maximum number of characters to store in this string.
    @return
        The new string object.
 */
XclExpStringRef lclCreateFormattedString(
        const XclExpRoot& rRoot, const String& rText, const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    /*  Create an empty Excel string object with correctly initialized BIFF mode,
        because this function only uses Append() functions that require this. */
    XclExpStringRef xString = XclExpStringHelper::CreateString( rRoot, EMPTY_STRING, nFlags, nMaxLen );

    // font handling
    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    sal_uInt16 nLastXclFont = EXC_FONT_APP;

    // script type handling
    Reference< XBreakIterator > xBreakIt = rRoot.GetDoc().GetBreakIterator();
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    // #i63255# get script type for leading weak characters
    sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( rRoot, rText );

    // cell item set
    const SfxItemSet& rItemSet = pCellAttr ? pCellAttr->GetItemSet() : rRoot.GetDoc().GetDefPattern()->GetItemSet();

    // process all script portions
    OUString aOUText( rText );
    sal_Int32 nPortionPos = 0;
    sal_Int32 nTextLen = aOUText.getLength();
    while( nPortionPos < nTextLen )
    {
        // get script type and end position of next script portion
        sal_Int16 nScript = xBreakIt->getScriptType( aOUText, nPortionPos );
        sal_Int32 nPortionEnd = xBreakIt->endOfScript( aOUText, nPortionPos, nScript );

        // reuse previous script for following weak portions
        if( nScript == ApiScriptType::WEAK )
            nScript = nLastScript;

        // construct font from current text portion
        SvxFont aFont( XclExpFontBuffer::GetFontFromItemSet( rItemSet, nScript ) );
        nLastScript = nScript;

        // Excel start position of this portion
        sal_uInt16 nXclPortionStart = xString->Len();
        // add portion text to Excel string
        XclExpStringHelper::AppendString( *xString, rRoot, aOUText.copy( nPortionPos, nPortionEnd - nPortionPos ) );

        // insert font into buffer
        sal_uInt16 nXclFont = rFontBuffer.Insert( aFont );
        // current portion font differs from last? -> insert into format run vector
        if( (nXclPortionStart == 0) || ((nXclFont != nLastXclFont) && (nXclPortionStart < xString->Len())) )
        {
            xString->AppendFormat( nXclPortionStart, nXclFont );
            nLastXclFont = nXclFont;
        }

        // go to next script portion
        nPortionPos = nPortionEnd;
    }

    return xString;
}

/** Creates a new formatted string from an edit engine text object.

    Creates a Unicode string or a byte string, depending on the current BIFF
    version contained in the passed XclExpRoot object.

    @param rEE
        The edit engine in use. The text object must already be set.
    @param nFlags
        Modifiers for string export.
    @param nMaxLen
        The maximum number of characters to store in this string.
    @return
        The new string object.
 */
XclExpStringRef lclCreateFormattedString(
        const XclExpRoot& rRoot, EditEngine& rEE, XclExpHyperlinkHelper* pLinkHelper,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    /*  Create an empty Excel string object with correctly initialized BIFF mode,
        because this function only uses Append() functions that require this. */
    XclExpStringRef xString = XclExpStringHelper::CreateString( rRoot, EMPTY_STRING, nFlags, nMaxLen );

    // helper item set for edit engine -> Calc item conversion
    SfxItemSet aItemSet( *rRoot.GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );

    // font handling
    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    sal_uInt16 nLastXclFont = EXC_FONT_APP;

    // script type handling
    Reference< XBreakIterator > xBreakIt = rRoot.GetDoc().GetBreakIterator();
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    // #i63255# get script type for leading weak characters
    sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( rRoot, rEE.GetText() );

    // process all paragraphs
    sal_uInt16 nParaCount = rEE.GetParagraphCount();
    for( sal_uInt16 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        String aParaText( rEE.GetText( nPara ) );
        if( aParaText.Len() )
        {
            SvUShorts aPosList;
            rEE.GetPortions( nPara, aPosList );

            // process all portions in the paragraph
            sal_uInt16 nPosCount = aPosList.Count();
            for( sal_uInt16 nPos = 0; nPos < nPosCount; ++nPos )
            {
                aSel.nEndPos = static_cast< xub_StrLen >( aPosList.GetObject( nPos ) );
                String aXclPortionText( aParaText, aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

                aItemSet.ClearItem();
                SfxItemSet aEditSet( rEE.GetAttribs( aSel ) );
                ScPatternAttr::GetFromEditItemSet( aItemSet, aEditSet );

                // get escapement value
                short nEsc = GETITEM( aEditSet, SvxEscapementItem, EE_CHAR_ESCAPEMENT ).GetEsc();

                // process text fields
                bool bIsHyperlink = false;
                if( aSel.nStartPos + 1 == aSel.nEndPos )
                {
                    // test if the character is a text field
                    const SfxPoolItem* pItem;
                    if( aEditSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_SET )
                    {
                        const SvxFieldData* pField = static_cast< const SvxFieldItem* >( pItem )->GetField();
                        if( const SvxURLField* pUrlField = PTR_CAST( SvxURLField, pField ) )
                        {
                            // convert URL field to string representation
                            aXclPortionText = pLinkHelper ?
                                pLinkHelper->ProcessUrlField( *pUrlField ) :
                                lclGetUrlRepresentation( *pUrlField );
                            bIsHyperlink = true;
                        }
                        else
                        {
                            DBG_ERRORFILE( "lclCreateFormattedString - unknown text field" );
                            aXclPortionText.Erase();
                        }
                    }
                }

                // Excel start position of this portion
                sal_uInt16 nXclPortionStart = xString->Len();
                // add portion text to Excel string
                XclExpStringHelper::AppendString( *xString, rRoot, aXclPortionText );

                /*  Construct font from current edit engine text portion. Edit engine
                    creates different portions for different script types, no need to loop. */
                sal_Int16 nScript = xBreakIt->getScriptType( aXclPortionText, 0 );
                if( nScript == ApiScriptType::WEAK )
                    nScript = nLastScript;
                SvxFont aFont( XclExpFontBuffer::GetFontFromItemSet( aItemSet, nScript ) );
                nLastScript = nScript;

                // add escapement
                aFont.SetEscapement( nEsc );
                // modify automatic font color for hyperlinks
                if( bIsHyperlink && (GETITEM( aItemSet, SvxColorItem, ATTR_FONT_COLOR ).GetValue().GetColor() == COL_AUTO) )
                    aFont.SetColor( Color( COL_LIGHTBLUE ) );

                // insert font into buffer
                sal_uInt16 nXclFont = rFontBuffer.Insert( aFont );
                // current portion font differs from last? -> insert into format run vector
                if( (nXclPortionStart == 0) || ((nXclFont != nLastXclFont) && (nXclPortionStart < xString->Len())) )
                {
                    xString->AppendFormat( nXclPortionStart, nXclFont );
                    nLastXclFont = nXclFont;
                }

                aSel.nStartPos = aSel.nEndPos;
            }
        }
        // add trailing newline (important for correct character index calculation)
        if( nPara + 1 < nParaCount )
            XclExpStringHelper::AppendChar( *xString, rRoot, '\n' );
    }

    return xString;
}

} // namespace

// ----------------------------------------------------------------------------

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const String& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString( new XclExpString );
    if( rRoot.GetBiff() == EXC_BIFF8 )
        xString->Assign( rString, nFlags, nMaxLen );
    else
        xString->AssignByte( rString, rRoot.GetCharSet(), nFlags, nMaxLen );
    return xString;
}

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, sal_Unicode cChar, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString = CreateString( rRoot, EMPTY_STRING, nFlags, nMaxLen );
    AppendChar( *xString, rRoot, cChar );
    return xString;
}

void XclExpStringHelper::AppendString( XclExpString& rXclString, const XclExpRoot& rRoot, const String& rString )
{
    if( rRoot.GetBiff() == EXC_BIFF8 )
        rXclString.Append( rString );
    else
        rXclString.AppendByte( rString, rRoot.GetCharSet() );
}

void XclExpStringHelper::AppendChar( XclExpString& rXclString, const XclExpRoot& rRoot, sal_Unicode cChar )
{
    if( rRoot.GetBiff() == EXC_BIFF8 )
        rXclString.Append( cChar );
    else
        rXclString.AppendByte( cChar, rRoot.GetCharSet() );
}

XclExpStringRef XclExpStringHelper::CreateCellString(
        const XclExpRoot& rRoot, const ScStringCell& rStringCell, const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    String aCellText;
    rStringCell.GetString( aCellText );
    return lclCreateFormattedString( rRoot, aCellText, pCellAttr, nFlags, nMaxLen );
}

XclExpStringRef XclExpStringHelper::CreateCellString(
        const XclExpRoot& rRoot, const ScEditCell& rEditCell, const ScPatternAttr* pCellAttr,
        XclExpHyperlinkHelper& rLinkHelper, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString;
    if( const EditTextObject* pEditObj = rEditCell.GetData() )
    {
        // formatted cell
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
        xString = lclCreateFormattedString( rRoot, rEE, &rLinkHelper, nFlags, nMaxLen );
        rEE.SetUpdateMode( bOldUpdateMode );
    }
    else
    {
        // unformatted cell
        String aCellText;
        rEditCell.GetString( aCellText );
        xString = lclCreateFormattedString( rRoot, aCellText, pCellAttr, nFlags, nMaxLen );
    }
    return xString;
}

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const SdrTextObj& rTextObj,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString;
    if( const OutlinerParaObject* pParaObj = rTextObj.GetOutlinerParaObject() )
    {
        EditEngine& rEE = rRoot.GetDrawEditEngine();
        BOOL bOldUpdateMode = rEE.GetUpdateMode();
        rEE.SetUpdateMode( TRUE );
        // create the string
        rEE.SetText( pParaObj->GetTextObject() );
        xString = lclCreateFormattedString( rRoot, rEE, 0, nFlags, nMaxLen );
        rEE.SetUpdateMode( bOldUpdateMode );
        // limit formats - TODO: BIFF dependent
        if( !xString->IsEmpty() )
        {
            xString->LimitFormatCount( EXC_MAXRECSIZE_BIFF8 / 8 - 1 );
            xString->AppendFormat( xString->Len(), EXC_FONT_APP );
        }
    }
    else
    {
        DBG_ERRORFILE( "XclExpStringHelper::CreateString - textbox without para object" );
        // create BIFF dependent empty Excel string
        xString = CreateString( rRoot, EMPTY_STRING, nFlags, nMaxLen );
    }
    return xString;
}

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const EditTextObject& rEditObj,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString;
    EditEngine& rEE = rRoot.GetDrawEditEngine();
    BOOL bOldUpdateMode = rEE.GetUpdateMode();
    rEE.SetUpdateMode( TRUE );
    rEE.SetText( rEditObj );
    xString = lclCreateFormattedString( rRoot, rEE, 0, nFlags, nMaxLen );
    rEE.SetUpdateMode( bOldUpdateMode );
    // limit formats - TODO: BIFF dependent
    if( !xString->IsEmpty() )
    {
        xString->LimitFormatCount( EXC_MAXRECSIZE_BIFF8 / 8 - 1 );
        xString->AppendFormat( xString->Len(), EXC_FONT_APP );
    }
    return xString;
}

sal_Int16 XclExpStringHelper::GetLeadingScriptType( const XclExpRoot& rRoot, const String& rString )
{
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    Reference< XBreakIterator > xBreakIt = rRoot.GetDoc().GetBreakIterator();
    OUString aOUString( rString );
    sal_Int32 nStrPos = 0;
    sal_Int32 nStrLen = aOUString.getLength();
    sal_Int16 nScript = ApiScriptType::WEAK;
    while( (nStrPos < nStrLen) && (nScript == ApiScriptType::WEAK) )
    {
        nScript = xBreakIt->getScriptType( aOUString, nStrPos );
        nStrPos = xBreakIt->endOfScript( aOUString, nStrPos, nScript );
    }
    return (nScript == ApiScriptType::WEAK) ? rRoot.GetDefApiScript() : nScript;
}

// Header/footer conversion ===================================================

XclExpHFConverter::XclExpHFConverter( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mrEE( rRoot.GetHFEditEngine() ),
    mnTotalHeight( 0 )
{
}

void XclExpHFConverter::GenerateString(
        const EditTextObject* pLeftObj,
        const EditTextObject* pCenterObj,
        const EditTextObject* pRightObj )
{
    maHFString.Erase();
    mnTotalHeight = 0;
    AppendPortion( pLeftObj, 'L' );
    AppendPortion( pCenterObj, 'C' );
    AppendPortion( pRightObj, 'R' );
}

void XclExpHFConverter::AppendPortion( const EditTextObject* pTextObj, sal_Unicode cPortionCode )
{
    if( !pTextObj ) return;

    String aText;
    sal_Int32 nHeight = 0;
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

    const FontList* pFontList = 0;
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
        sal_Int32 nParaHeight = 0;
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
                aNewData.mnHeight = ulimit_cast< sal_uInt16 >( GETITEM( aEditSet, SvxFontHeightItem, EE_CHAR_FONTHEIGHT ).GetHeight() );
                (aNewData.mnHeight += 10) /= 20;
                bool bFontHtChanged = (aFontData.mnHeight != aNewData.mnHeight);
                if( bFontHtChanged )
                    aParaText.Append( '&' ).Append( String::CreateFromInt32( aNewData.mnHeight ) );
                // update maximum paragraph height, convert to twips
                nParaHeight = ::std::max< sal_Int32 >( nParaHeight, aNewData.mnHeight * 20 );

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
        if( nParaHeight == 0 )
            nParaHeight = aFontData.mnHeight * 20;  // points -> twips
        nHeight += nParaHeight;
    }

    mrEE.SetUpdateMode( bOldUpdateMode );

    if( aText.Len() )
    {
        maHFString.Append( '&' ).Append( cPortionCode ).Append( aText );
        mnTotalHeight = ::std::max( mnTotalHeight, nHeight );
    }
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
    @param pTableName  Pointer to a table name to be encoded in this URL, or 0. */
void lclEncodeDosUrl( XclBiff eBiff, String& rUrl, const String* pTableName = 0 )
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
        switch( eBiff )
        {
            case EXC_BIFF5:
                rUrl = pTableName ? EXC_URLSTART_SELFENCODED : EXC_URLSTART_SELF;
            break;
            case EXC_BIFF8:
                DBG_ASSERT( pTableName, "lclEncodeDosUrl - sheet name required for BIFF8" );
                rUrl = EXC_URLSTART_SELF;
            break;
            default:
                DBG_ERROR_BIFF();
        }
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
    if( !aDosUrl.Len() || lclConvertToDos( aDosUrl, rRoot.GetBasePath(), rRoot.IsRelUrl() ) )
        lclEncodeDosUrl( rRoot.GetBiff(), aDosUrl, pTableName );
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

void XclExpCachedString::Save( XclExpStream& rStrm ) const
{
    rStrm.SetSliceSize( 6 );
    rStrm << EXC_CACHEDVAL_STRING << maStr;
}

// ----------------------------------------------------------------------------

XclExpCachedError::XclExpCachedError( USHORT nScError ) :
    mnError( XclTools::GetXclErrorCode( nScError ) )
{
}

void XclExpCachedError::Save( XclExpStream& rStrm ) const
{
    rStrm.SetSliceSize( 9 );
    rStrm << EXC_CACHEDVAL_ERROR << mnError;
    rStrm.WriteZeroBytes( 7 );
}

// ----------------------------------------------------------------------------

XclExpCachedMatrix::XclExpCachedMatrix( const ScMatrix& rMatrix, XclStrFlags nFlags )
{
    rMatrix.GetDimensions( mnScCols, mnScRows );
    DBG_ASSERT( mnScCols && mnScRows, "XclExpCachedMatrix::XclExpCachedMatrix - empty matrix" );
    DBG_ASSERT( mnScCols <= 256, "XclExpCachedMatrix::XclExpCachedMatrix - too many columns" );

    for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
    {
        for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
        {
            XclExpCachedValue* pNewVal = 0;
            ScMatValType nMatValType = SC_MATVAL_VALUE;
            const ScMatrixValue* pMatVal = rMatrix.Get( nScCol, nScRow, nMatValType );
            if( !pMatVal )
                pNewVal = new XclExpCachedString( EMPTY_STRING, nFlags );
            else if( nMatValType != SC_MATVAL_VALUE )
                pNewVal = new XclExpCachedString( pMatVal->GetString(), nFlags );
            else if( USHORT nScError = pMatVal->GetError() )
                pNewVal = new XclExpCachedError( nScError );
            else
                pNewVal = new XclExpCachedDouble( pMatVal->fVal );
            maValueList.Append( pNewVal );
        }
    }
}

sal_Size XclExpCachedMatrix::GetSize() const
{
    /*  The returned size may be wrong if the matrix contains strings. The only
        effect is that the export stream has to update a wrong record size which is
        faster than to iterate through all cached values and calculate their sizes. */
    return 3 + 9 * maValueList.Count();
}

void XclExpCachedMatrix::Save( XclExpStream& rStrm ) const
{
    if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
        // in BIFF2-BIFF7: 256 columns represented by 0 columns
        rStrm << static_cast< sal_uInt8 >( mnScCols ) << static_cast< sal_uInt16 >( mnScRows );
    else
        // in BIFF8: columns and rows decreaed by 1
        rStrm << static_cast< sal_uInt8 >( mnScCols - 1 ) << static_cast< sal_uInt16 >( mnScRows - 1 );

    for( const XclExpCachedValue* pValue = maValueList.First(); pValue; pValue = maValueList.Next() )
        pValue->Save( rStrm );
}

// ============================================================================

