/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <sfx2/objsh.hxx>
#include <vcl/font.hxx>
#include <tools/urlobj.hxx>
#include <svl/itemset.hxx>
#include <svtools/ctrltool.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include "scitems.hxx"
#include <editeng/fhgtitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/svxfont.hxx>

#include "document.hxx"
#include "docpool.hxx"
#include "formulacell.hxx"
#include "editutil.hxx"
#include "patattr.hxx"
#include "scmatrix.hxx"
#include "xestyle.hxx"
#include "fprogressbar.hxx"
#include "xltracer.hxx"
#include "xecontent.hxx"
#include "xelink.hxx"
#include "xehelper.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::i18n::XBreakIterator;



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

    

    mnSegRowFinal = mxProgress->AddSegment( 1000 );
    
}

void XclExpProgressBar::IncRowRecordCount()
{
    ++mnRowCount;
}

void XclExpProgressBar::ActivateCreateRowsSegment()
{
    OSL_ENSURE( (0 <= GetCurrScTab()) && (GetCurrScTab() < GetTabInfo().GetScTabCount()),
        "XclExpProgressBar::ActivateCreateRowsSegment - invalid sheet" );
    sal_Int32 nSeg = maSubSegRowCreate[ GetCurrScTab() ];
    OSL_ENSURE( nSeg != SCF_INV_SEGMENT, "XclExpProgressBar::ActivateCreateRowsSegment - invalid segment" );
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



namespace {

/** Fills the passed Excel address with the passed Calc cell coordinates without checking any limits. */
inline void lclFillAddress( XclAddress& rXclPos, SCCOL nScCol, SCROW nScRow )
{
    rXclPos.mnCol = static_cast< sal_uInt16 >( nScCol );
    rXclPos.mnRow = static_cast< sal_uInt32 >( nScRow );
}

} 



XclExpAddressConverter::XclExpAddressConverter( const XclExpRoot& rRoot ) :
    XclAddressConverterBase( rRoot.GetTracer(), rRoot.GetXclMaxPos() )
{
}



bool XclExpAddressConverter::CheckAddress( const ScAddress& rScPos, bool bWarn )
{
    
    bool bValidCol = (0 <= rScPos.Col()) && (rScPos.Col() <= maMaxPos.Col());
    bool bValidRow = (0 <= rScPos.Row()) && (rScPos.Row() <= maMaxPos.Row());
    bool bValidTab = (0 <= rScPos.Tab()) && (rScPos.Tab() <= maMaxPos.Tab());

    bool bValid = bValidCol && bValidRow && bValidTab;
    if( !bValid )
    {
        mbColTrunc |= !bValidCol;
        mbRowTrunc |= !bValidRow;
    }
    if( !bValid && bWarn )
    {
        mbTabTrunc |= (rScPos.Tab() > maMaxPos.Tab());  
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



bool XclExpAddressConverter::CheckRange( const ScRange& rScRange, bool bWarn )
{
    return CheckAddress( rScRange.aStart, bWarn ) && CheckAddress( rScRange.aEnd, bWarn );
}

bool XclExpAddressConverter::ValidateRange( ScRange& rScRange, bool bWarn )
{
    rScRange.Justify();

    
    bool bValidStart = CheckAddress( rScRange.aStart, bWarn );
    if( bValidStart )
    {
        
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
    
    bool bValidStart = CheckAddress( rScRange.aStart, bWarn );
    if( bValidStart )
    {
        lclFillAddress( rXclRange.maFirst, rScRange.aStart.Col(), rScRange.aStart.Row() );

        
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



void XclExpAddressConverter::ValidateRangeList( ScRangeList& rScRanges, bool bWarn )
{
    for ( size_t nRange = rScRanges.size(); nRange > 0; )
    {
        ScRange* pScRange = rScRanges[ --nRange ];
        if( !CheckRange( *pScRange, bWarn ) )
            delete rScRanges.Remove(nRange);
    }
}

void XclExpAddressConverter::ConvertRangeList( XclRangeList& rXclRanges,
        const ScRangeList& rScRanges, bool bWarn )
{
    rXclRanges.clear();
    for( size_t nPos = 0, nCount = rScRanges.size(); nPos < nCount; ++nPos )
    {
        if( const ScRange* pScRange = rScRanges[ nPos ] )
        {
            XclRange aXclRange( ScAddress::UNINITIALIZED );
            if( ConvertRange( aXclRange, *pScRange, bWarn ) )
                rXclRanges.push_back( aXclRange );
        }
    }
}



namespace {

OUString lclGetUrlRepresentation( const SvxURLField& rUrlField )
{
    const OUString& aRepr = rUrlField.GetRepresentation();
    
    return aRepr.isEmpty() ? rUrlField.GetURL() : aRepr;
}

} 



XclExpHyperlinkHelper::XclExpHyperlinkHelper( const XclExpRoot& rRoot, const ScAddress& rScPos ) :
    XclExpRoot( rRoot ),
    maScPos( rScPos ),
    mbMultipleUrls( false )
{
}

XclExpHyperlinkHelper::~XclExpHyperlinkHelper()
{
}

OUString XclExpHyperlinkHelper::ProcessUrlField( const SvxURLField& rUrlField )
{
    OUString aUrlRepr;

    if( GetBiff() == EXC_BIFF8 )    
    {
        
        mbMultipleUrls = static_cast< bool >(mxLinkRec);

        mxLinkRec.reset( new XclExpHyperlink( GetRoot(), rUrlField, maScPos ) );

        if( const OUString* pRepr = mxLinkRec->GetRepr() )
            aUrlRepr = *pRepr;

        
        maUrlList = ScGlobal::addToken( maUrlList, rUrlField.GetURL(), '\n' );
    }

    
    return aUrlRepr.isEmpty() ? lclGetUrlRepresentation(rUrlField) : aUrlRepr;
}

bool XclExpHyperlinkHelper::HasLinkRecord() const
{
    return !mbMultipleUrls && mxLinkRec;
}

XclExpHyperlinkHelper::XclExpHyperlinkRef XclExpHyperlinkHelper::GetLinkRecord()
{
    if( HasLinkRecord() )
        return mxLinkRec;
    return XclExpHyperlinkRef();
}



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
        const XclExpRoot& rRoot, const OUString& rText, const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    /*  Create an empty Excel string object with correctly initialized BIFF mode,
        because this function only uses Append() functions that require this. */
    XclExpStringRef xString = XclExpStringHelper::CreateString( rRoot, EMPTY_OUSTRING, nFlags, nMaxLen );

    
    Reference< XBreakIterator > xBreakIt = rRoot.GetDoc().GetBreakIterator();
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    
    sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( rRoot, rText );

    
    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    const SfxItemSet& rItemSet = pCellAttr ? pCellAttr->GetItemSet() : rRoot.GetDoc().GetDefPattern()->GetItemSet();

    
    OUString aOUText( rText );
    sal_Int32 nPortionPos = 0;
    sal_Int32 nTextLen = aOUText.getLength();
    while( nPortionPos < nTextLen )
    {
        
        sal_Int16 nScript = xBreakIt->getScriptType( aOUText, nPortionPos );
        sal_Int32 nPortionEnd = xBreakIt->endOfScript( aOUText, nPortionPos, nScript );

        
        if( nScript == ApiScriptType::WEAK )
            nScript = nLastScript;

        
        SvxFont aFont( XclExpFontHelper::GetFontFromItemSet( rRoot, rItemSet, nScript ) );

        
        sal_Int32 nXclPortionStart = xString->Len();
        
        XclExpStringHelper::AppendString( *xString, rRoot, aOUText.copy( nPortionPos, nPortionEnd - nPortionPos ) );
        if( nXclPortionStart < xString->Len() )
        {
            
            sal_uInt16 nFontIdx = rFontBuffer.Insert( aFont, EXC_COLOR_CELLTEXT );
            
            xString->AppendFormat( nXclPortionStart, nFontIdx );
        }

        
        nLastScript = nScript;
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
    XclExpStringRef xString = XclExpStringHelper::CreateString( rRoot, EMPTY_OUSTRING, nFlags, nMaxLen );

    
    XclExpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
    SfxItemSet aItemSet( *rRoot.GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );

    
    Reference< XBreakIterator > xBreakIt = rRoot.GetDoc().GetBreakIterator();
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    
    sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( rRoot, rEE.GetText() );

    
    sal_Int32 nParaCount = rEE.GetParagraphCount();
    for( sal_Int32 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        OUString aParaText( rEE.GetText( nPara ) );

        std::vector<sal_Int32> aPosList;
        rEE.GetPortions( nPara, aPosList );

        
        for( std::vector<sal_Int32>::const_iterator it(aPosList.begin()); it != aPosList.end(); ++it )
        {
            aSel.nEndPos =  *it;
            OUString aXclPortionText = aParaText.copy( aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

            aItemSet.ClearItem();
            SfxItemSet aEditSet( rEE.GetAttribs( aSel ) );
            ScPatternAttr::GetFromEditItemSet( aItemSet, aEditSet );

            
            short nEsc = GETITEM( aEditSet, SvxEscapementItem, EE_CHAR_ESCAPEMENT ).GetEsc();

            
            bool bIsHyperlink = false;
            if( aSel.nStartPos + 1 == aSel.nEndPos )
            {
                
                const SfxPoolItem* pItem;
                if( aEditSet.GetItemState( EE_FEATURE_FIELD, false, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxFieldData* pField = static_cast< const SvxFieldItem* >( pItem )->GetField();
                    if( const SvxURLField* pUrlField = PTR_CAST( SvxURLField, pField ) )
                    {
                        
                        aXclPortionText = pLinkHelper ?
                            pLinkHelper->ProcessUrlField( *pUrlField ) :
                            lclGetUrlRepresentation( *pUrlField );
                        bIsHyperlink = true;
                    }
                    else
                    {
                        OSL_FAIL( "lclCreateFormattedString - unknown text field" );
                        aXclPortionText = "";
                    }
                }
            }

            
            sal_Int32 nXclPortionStart = xString->Len();
            
            XclExpStringHelper::AppendString( *xString, rRoot, aXclPortionText );
            if( (nXclPortionStart < xString->Len()) || (aParaText.isEmpty()) )
            {
                /*  Construct font from current edit engine text portion. Edit engine
                    creates different portions for different script types, no need to loop. */
                sal_Int16 nScript = xBreakIt->getScriptType( aXclPortionText, 0 );
                if( nScript == ApiScriptType::WEAK )
                    nScript = nLastScript;
                SvxFont aFont( XclExpFontHelper::GetFontFromItemSet( rRoot, aItemSet, nScript ) );
                nLastScript = nScript;

                
                aFont.SetEscapement( nEsc );
                
                if( bIsHyperlink && (GETITEM( aItemSet, SvxColorItem, ATTR_FONT_COLOR ).GetValue().GetColor() == COL_AUTO) )
                    aFont.SetColor( Color( COL_LIGHTBLUE ) );

                
                sal_uInt16 nFontIdx = rFontBuffer.Insert( aFont, EXC_COLOR_CELLTEXT );
                
                xString->AppendFormat( nXclPortionStart, nFontIdx );
            }

            aSel.nStartPos = aSel.nEndPos;
        }

        
        if( nPara + 1 < nParaCount )
            XclExpStringHelper::AppendChar( *xString, rRoot, '\n' );
    }

    return xString;
}

} 



XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString( new XclExpString );
    if( rRoot.GetBiff() == EXC_BIFF8 )
        xString->Assign( rString, nFlags, nMaxLen );
    else
        xString->AssignByte( rString, rRoot.GetTextEncoding(), nFlags, nMaxLen );
    return xString;
}

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, sal_Unicode cChar, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString = CreateString( rRoot, EMPTY_OUSTRING, nFlags, nMaxLen );
    AppendChar( *xString, rRoot, cChar );
    return xString;
}

void XclExpStringHelper::AppendString( XclExpString& rXclString, const XclExpRoot& rRoot, const OUString& rString )
{
    if( rRoot.GetBiff() == EXC_BIFF8 )
        rXclString.Append( rString );
    else
        rXclString.AppendByte( rString, rRoot.GetTextEncoding() );
}

void XclExpStringHelper::AppendChar( XclExpString& rXclString, const XclExpRoot& rRoot, sal_Unicode cChar )
{
    if( rRoot.GetBiff() == EXC_BIFF8 )
        rXclString.Append( OUString(cChar) );
    else
        rXclString.AppendByte( cChar, rRoot.GetTextEncoding() );
}

XclExpStringRef XclExpStringHelper::CreateCellString(
        const XclExpRoot& rRoot, const OUString& rString, const ScPatternAttr* pCellAttr,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    return lclCreateFormattedString(rRoot, rString, pCellAttr, nFlags, nMaxLen);
}

XclExpStringRef XclExpStringHelper::CreateCellString(
        const XclExpRoot& rRoot, const EditTextObject& rEditText, const ScPatternAttr* pCellAttr,
        XclExpHyperlinkHelper& rLinkHelper, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString;

    
    ScEditEngineDefaulter& rEE = rRoot.GetEditEngine();
    sal_Bool bOldUpdateMode = rEE.GetUpdateMode();
    rEE.SetUpdateMode( true );

    
    const SfxItemSet& rItemSet = pCellAttr ? pCellAttr->GetItemSet() : rRoot.GetDoc().GetDefPattern()->GetItemSet();
    SfxItemSet* pEEItemSet = new SfxItemSet( rEE.GetEmptyItemSet() );
    ScPatternAttr::FillToEditItemSet( *pEEItemSet, rItemSet );
    rEE.SetDefaults( pEEItemSet );      

    
    rEE.SetText(rEditText);
    xString = lclCreateFormattedString( rRoot, rEE, &rLinkHelper, nFlags, nMaxLen );
    rEE.SetUpdateMode( bOldUpdateMode );

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
        sal_Bool bOldUpdateMode = rEE.GetUpdateMode();
        rEE.SetUpdateMode( true );
        
        rEE.SetText( pParaObj->GetTextObject() );
        xString = lclCreateFormattedString( rRoot, rEE, 0, nFlags, nMaxLen );
        rEE.SetUpdateMode( bOldUpdateMode );
        
        if( !xString->IsEmpty() )
        {
            xString->LimitFormatCount( EXC_MAXRECSIZE_BIFF8 / 8 - 1 );
            xString->AppendTrailingFormat( EXC_FONT_APP );
        }
    }
    else
    {
        OSL_FAIL( "XclExpStringHelper::CreateString - textbox without para object" );
        
        xString = CreateString( rRoot, EMPTY_OUSTRING, nFlags, nMaxLen );
    }
    return xString;
}

XclExpStringRef XclExpStringHelper::CreateString(
        const XclExpRoot& rRoot, const EditTextObject& rEditObj,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    XclExpStringRef xString;
    EditEngine& rEE = rRoot.GetDrawEditEngine();
    sal_Bool bOldUpdateMode = rEE.GetUpdateMode();
    rEE.SetUpdateMode( true );
    rEE.SetText( rEditObj );
    xString = lclCreateFormattedString( rRoot, rEE, 0, nFlags, nMaxLen );
    rEE.SetUpdateMode( bOldUpdateMode );
    
    if( !xString->IsEmpty() )
    {
        xString->LimitFormatCount( EXC_MAXRECSIZE_BIFF8 / 8 - 1 );
        xString->AppendTrailingFormat( EXC_FONT_APP );
    }
    return xString;
}

sal_Int16 XclExpStringHelper::GetLeadingScriptType( const XclExpRoot& rRoot, const OUString& rString )
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
    maHFString = "";
    mnTotalHeight = 0;
    AppendPortion( pLeftObj, 'L' );
    AppendPortion( pCenterObj, 'C' );
    AppendPortion( pRightObj, 'R' );
}

void XclExpHFConverter::AppendPortion( const EditTextObject* pTextObj, sal_Unicode cPortionCode )
{
    if( !pTextObj ) return;

    OUString aText;
    sal_Int32 nHeight = 0;
    SfxItemSet aItemSet( *GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );

    
    sal_Bool bOldUpdateMode = mrEE.GetUpdateMode();
    mrEE.SetUpdateMode( true );
    mrEE.SetText( *pTextObj );

    
    XclFontData aFontData, aNewData;
    if( const XclExpFont* pFirstFont = GetFontBuffer().GetFont( EXC_FONT_APP ) )
    {
        aFontData = pFirstFont->GetFontData();
        (aFontData.mnHeight += 10) /= 20;   
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

    sal_Int32 nParaCount = mrEE.GetParagraphCount();
    for( sal_Int32 nPara = 0; nPara < nParaCount; ++nPara )
    {
        ESelection aSel( nPara, 0 );
        OUString aParaText;
        sal_Int32 nParaHeight = 0;
        std::vector<sal_Int32> aPosList;
        mrEE.GetPortions( nPara, aPosList );

        for( std::vector<sal_Int32>::const_iterator it( aPosList.begin() ); it != aPosList.end(); ++it )
        {
            aSel.nEndPos = *it;
            if( aSel.nStartPos < aSel.nEndPos )
            {



                Font aFont;
                aItemSet.ClearItem();
                SfxItemSet aEditSet( mrEE.GetAttribs( aSel ) );
                ScPatternAttr::GetFromEditItemSet( aItemSet, aEditSet );
                ScPatternAttr::GetFont( aFont, aItemSet, SC_AUTOCOL_RAW );

                
                aNewData.maName = XclTools::GetXclFontName( aFont.GetName() );
                aNewData.mnWeight = (aFont.GetWeight() > WEIGHT_NORMAL) ? EXC_FONTWGHT_BOLD : EXC_FONTWGHT_NORMAL;
                aNewData.mbItalic = (aFont.GetItalic() != ITALIC_NONE);
                bool bNewFont = !(aFontData.maName == aNewData.maName);
                bool bNewStyle = (aFontData.mnWeight != aNewData.mnWeight) ||
                                 (aFontData.mbItalic != aNewData.mbItalic);
                if( bNewFont || (bNewStyle && pFontList) )
                {
                    aParaText = "&\"" + OUString(aNewData.maName);
                    if( pFontList )
                    {
                        FontInfo aFontInfo( pFontList->Get(
                            aNewData.maName,
                            (aNewData.mnWeight > EXC_FONTWGHT_NORMAL) ? WEIGHT_BOLD : WEIGHT_NORMAL,
                            aNewData.mbItalic ? ITALIC_NORMAL : ITALIC_NONE ) );
                        aNewData.maStyle = pFontList->GetStyleName( aFontInfo );
                        if( !aNewData.maStyle.isEmpty() )
                            aParaText += "," + aNewData.maStyle;
                    }
                    aParaText += "\"";
                }

                
                
                
                aNewData.mnHeight = ulimit_cast< sal_uInt16 >( GETITEM( aEditSet, SvxFontHeightItem, EE_CHAR_FONTHEIGHT ).GetHeight() );
                (aNewData.mnHeight += 10) /= 20;
                bool bFontHtChanged = (aFontData.mnHeight != aNewData.mnHeight);
                if( bFontHtChanged )
                    aParaText += "&" + OUString::number( aNewData.mnHeight );
                
                nParaHeight = ::std::max< sal_Int32 >( nParaHeight, aNewData.mnHeight * 20 );

                
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
                    (nTmpUnderl == EXC_FONTUNDERL_SINGLE)? aParaText += "&U" : aParaText += "&E";
                }

                
                aNewData.mbStrikeout = (aFont.GetStrikeout() != STRIKEOUT_NONE);
                if( aFontData.mbStrikeout != aNewData.mbStrikeout )
                    aParaText += "&S";

                
                const SvxEscapementItem& rEscapeItem = GETITEM( aEditSet, SvxEscapementItem, EE_CHAR_ESCAPEMENT );
                aNewData.SetScEscapement( rEscapeItem.GetEsc() );
                if( aFontData.mnEscapem != aNewData.mnEscapem )
                {
                    switch(aNewData.mnEscapem)
                    {
                        
                        case EXC_FONTESC_NONE:  (aFontData.mnEscapem == EXC_FONTESC_SUPER) ? aParaText += "&X" : aParaText += "&Y"; break;
                        case EXC_FONTESC_SUPER: aParaText += "&X";  break;
                        case EXC_FONTESC_SUB:   aParaText += "&Y";  break;
                        default: break;
                    }
                }

                aFontData = aNewData;



                const SfxPoolItem* pItem;
                if( (aSel.nStartPos + 1 == aSel.nEndPos) &&     
                    (aEditSet.GetItemState( EE_FEATURE_FIELD, false, &pItem ) == SFX_ITEM_SET) )
                {
                    if( const SvxFieldData* pFieldData = static_cast< const SvxFieldItem* >( pItem )->GetField() )
                    {
                        if( pFieldData->ISA( SvxPageField ) )
                            aParaText += "&P";
                        else if( pFieldData->ISA( SvxPagesField ) )
                            aParaText += "&N";
                        else if( pFieldData->ISA( SvxDateField ) )
                            aParaText += "&D";
                        else if( pFieldData->ISA( SvxTimeField ) || pFieldData->ISA( SvxExtTimeField ) )
                            aParaText += "&T";
                        else if( pFieldData->ISA( SvxTableField ) )
                            aParaText += "&A";
                        else if( pFieldData->ISA( SvxFileField ) )  
                            aParaText += "&F";
                        else if( const SvxExtFileField* pFileField = PTR_CAST( SvxExtFileField, pFieldData ) )
                        {
                            switch( pFileField->GetFormat() )
                            {
                                case SVXFILEFORMAT_NAME_EXT:
                                case SVXFILEFORMAT_NAME:
                                    aParaText += "&F";
                                break;
                                case SVXFILEFORMAT_PATH:
                                    aParaText += "&Z";
                                break;
                                case SVXFILEFORMAT_FULLPATH:
                                    aParaText += "&Z&F";
                                break;
                                default:
                                    OSL_FAIL( "XclExpHFConverter::AppendPortion - unknown file field" );
                            }
                        }
                    }
                }
                else
                {
                    OUString aPortionText( mrEE.GetText( aSel ) );
                    aPortionText = aPortionText.replaceAll( "&", "&&" );
                    
                    if( bFontHtChanged && aParaText.getLength() && !aPortionText.isEmpty() )
                    {
                        sal_Unicode cLast = aParaText[ aParaText.getLength() - 1 ];
                        sal_Unicode cFirst = aPortionText[0];
                        if( ('0' <= cLast) && (cLast <= '9') && ('0' <= cFirst) && (cFirst <= '9') )
                            aParaText += " ";
                    }
                    aParaText += aPortionText;
                }
            }

            aSel.nStartPos = aSel.nEndPos;
        }

        aText = ScGlobal::addToken( aText, aParaText, '\n' );
        if( nParaHeight == 0 )
            nParaHeight = aFontData.mnHeight * 20;  
        nHeight += nParaHeight;
    }

    mrEE.SetUpdateMode( bOldUpdateMode );

    if( !aText.isEmpty() )
    {
        maHFString += "&" + OUString(cPortionCode) + aText;
        mnTotalHeight = ::std::max( mnTotalHeight, nHeight );
    }
}



namespace {

/** Encodes special parts of the URL, i.e. directory separators and volume names.
    @param pTableName  Pointer to a table name to be encoded in this URL, or 0. */
OUString lclEncodeDosUrl(
    XclBiff eBiff, const OUString& rUrl, const OUString& rBase, const OUString* pTableName)
{
    OUStringBuffer aBuf;

    if (!rUrl.isEmpty())
    {
        OUString aOldUrl = rUrl;
        aBuf.append(EXC_URLSTART_ENCODED);

        if ( aOldUrl.getLength() > 2 && aOldUrl.copy(0,2) == "\\\\" )
        {
            
            aBuf.append(EXC_URL_DOSDRIVE).append('@');
            aOldUrl = aOldUrl.copy(2);
        }
        else if ( aOldUrl.getLength() > 2 && aOldUrl.copy(1,2) == ":\\" )
        {
            
            sal_Unicode cThisDrive = rBase.isEmpty() ? ' ' : rBase[0];
            sal_Unicode cDrive = aOldUrl[0];
            if (cThisDrive == cDrive)
                
                aBuf.append(EXC_URL_DRIVEROOT);
            else
                aBuf.append(EXC_URL_DOSDRIVE).append(cDrive);
            aOldUrl = aOldUrl.copy(3);
        }

        
        sal_Int32 nPos = -1;
        while((nPos = aOldUrl.indexOf('\\')) != -1)
        {
            if ( aOldUrl.copy(0,2) == ".." )
                
                
                aBuf.append(EXC_URL_PARENTDIR);
            else
                aBuf.append(aOldUrl.copy(0,nPos)).append(EXC_URL_SUBDIR);

            aOldUrl = aOldUrl.copy(nPos + 1);
        }

        
        if (pTableName)    
            aBuf.append('[').append(aOldUrl).append(']');
        else
            aBuf.append(aOldUrl);
    }
    else    
    {
        switch( eBiff )
        {
            case EXC_BIFF5:
                aBuf.append(pTableName ? EXC_URLSTART_SELFENCODED : EXC_URLSTART_SELF);
            break;
            case EXC_BIFF8:
                DBG_ASSERT( pTableName, "lclEncodeDosUrl - sheet name required for BIFF8" );
                aBuf.append(EXC_URLSTART_SELF);
            break;
            default:
                DBG_ERROR_BIFF();
        }
    }

    
    if (pTableName)
        aBuf.append(*pTableName);

    return aBuf.makeStringAndClear();
}

} 



OUString XclExpUrlHelper::EncodeUrl( const XclExpRoot& rRoot, const OUString& rAbsUrl, const OUString* pTableName )
{
    OUString aDosUrl = INetURLObject(rAbsUrl).getFSysPath(INetURLObject::FSYS_DOS);
    OUString aDosBase = INetURLObject(rRoot.GetBasePath()).getFSysPath(INetURLObject::FSYS_DOS);
    return lclEncodeDosUrl(rRoot.GetBiff(), aDosUrl, aDosBase, pTableName);
}

OUString XclExpUrlHelper::EncodeDde( const OUString& rApplic, const OUString& rTopic )
{
    OUStringBuffer aBuf;
    aBuf.append(rApplic).append(EXC_DDE_DELIM).append(rTopic);
    return aBuf.makeStringAndClear();
}



XclExpCachedMatrix::XclExpCachedMatrix( const ScMatrix& rMatrix )
    : mrMatrix( rMatrix )
{
    mrMatrix.IncRef();
}
XclExpCachedMatrix::~XclExpCachedMatrix()
{
    mrMatrix.DecRef();
}

void XclExpCachedMatrix::GetDimensions( SCSIZE & nCols, SCSIZE & nRows ) const
{
    mrMatrix.GetDimensions( nCols, nRows );

    OSL_ENSURE( nCols && nRows, "XclExpCachedMatrix::GetDimensions - empty matrix" );
    OSL_ENSURE( nCols <= 256, "XclExpCachedMatrix::GetDimensions - too many columns" );
}

sal_Size XclExpCachedMatrix::GetSize() const
{
    SCSIZE nCols, nRows;

    GetDimensions( nCols, nRows );

    /*  The returned size may be wrong if the matrix contains strings. The only
        effect is that the export stream has to update a wrong record size which is
        faster than to iterate through all cached values and calculate their sizes. */
    return 3 + 9 * (nCols * nRows);
}

void XclExpCachedMatrix::Save( XclExpStream& rStrm ) const
{
    SCSIZE nCols, nRows;

    GetDimensions( nCols, nRows );

    if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
        
        rStrm << static_cast< sal_uInt8 >( nCols ) << static_cast< sal_uInt16 >( nRows );
    else
        
        rStrm << static_cast< sal_uInt8 >( nCols - 1 ) << static_cast< sal_uInt16 >( nRows - 1 );

    for( SCSIZE nRow = 0; nRow < nRows; ++nRow )
    {
        for( SCSIZE nCol = 0; nCol < nCols; ++nCol )
        {
            ScMatrixValue nMatVal = mrMatrix.Get( nCol, nRow );

            if( SC_MATVAL_EMPTY == nMatVal.nType )
            {
                rStrm.SetSliceSize( 9 );
                rStrm << EXC_CACHEDVAL_EMPTY;
                rStrm.WriteZeroBytes( 8 );
            }
            else if( ScMatrix::IsNonValueType( nMatVal.nType ) )
            {
                XclExpString aStr( nMatVal.GetString().getString(), EXC_STR_DEFAULT );
                rStrm.SetSliceSize( 6 );
                rStrm << EXC_CACHEDVAL_STRING << aStr;
            }
            else if( SC_MATVAL_BOOLEAN == nMatVal.nType )
            {
                sal_Int8 nBool = sal_Int8(nMatVal.GetBoolean());
                rStrm.SetSliceSize( 9 );
                rStrm << EXC_CACHEDVAL_BOOL << nBool;
                rStrm.WriteZeroBytes( 7 );
            }
            else if( sal_uInt16 nScError = nMatVal.GetError() )
            {
                sal_Int8 nError ( XclTools::GetXclErrorCode( nScError ) );
                rStrm.SetSliceSize( 9 );
                rStrm << EXC_CACHEDVAL_ERROR << nError;
                rStrm.WriteZeroBytes( 7 );
            }
            else
            {
                rStrm.SetSliceSize( 9 );
                rStrm << EXC_CACHEDVAL_DOUBLE << nMatVal.fVal;
            }
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
