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

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/justifyitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <svl/inethist.hxx>
#include <unotools/syslocale.hxx>

#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "editutil.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "compiler.hxx"

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

//  Delimiters zusaetzlich zu EditEngine-Default:

const sal_Char ScEditUtil::pCalcDelimiters[] = "=()+-*/^&<>";


//------------------------------------------------------------------------

String ScEditUtil::ModifyDelimiters( const String& rOld )
{
    // underscore is used in function argument names
    String aRet = comphelper::string::remove(rOld, '_');
    aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( pCalcDelimiters ) );
    aRet.Append(ScCompiler::GetNativeSymbol(ocSep)); // argument separator is localized.
    return aRet;
}

static String lcl_GetDelimitedString( const EditEngine& rEngine, const sal_Char c )
{
    String aRet;
    sal_uInt16 nParCount = rEngine.GetParagraphCount();
    for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
    {
        if (nPar > 0)
            aRet += c;
        aRet += rEngine.GetText( nPar );
    }
    return aRet;
}

String ScEditUtil::GetSpaceDelimitedString( const EditEngine& rEngine )
{
    return lcl_GetDelimitedString(rEngine, ' ');
}

String ScEditUtil::GetMultilineString( const EditEngine& rEngine )
{
    return lcl_GetDelimitedString(rEngine, '\n');
}

//------------------------------------------------------------------------

Rectangle ScEditUtil::GetEditArea( const ScPatternAttr* pPattern, sal_Bool bForceToTop )
{
    // bForceToTop = always align to top, for editing
    // (sal_False for querying URLs etc.)

    if (!pPattern)
        pPattern = pDoc->GetPattern( nCol, nRow, nTab );

    Point aStartPos = aScrPos;

    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    const ScMergeAttr* pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
    long nCellX = (long) ( pDoc->GetColWidth(nCol,nTab) * nPPTX );
    if ( pMerge->GetColMerge() > 1 )
    {
        SCCOL nCountX = pMerge->GetColMerge();
        for (SCCOL i=1; i<nCountX; i++)
            nCellX += (long) ( pDoc->GetColWidth(nCol+i,nTab) * nPPTX );
    }
    long nCellY = (long) ( pDoc->GetRowHeight(nRow,nTab) * nPPTY );
    if ( pMerge->GetRowMerge() > 1 )
    {
        SCROW nCountY = pMerge->GetRowMerge();
        nCellY += (long) pDoc->GetScaledRowHeight( nRow+1, nRow+nCountY-1, nTab, nPPTY);
    }

    const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
    sal_uInt16 nIndent = 0;
    if ( ((const SvxHorJustifyItem&)pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue() ==
                SVX_HOR_JUSTIFY_LEFT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
    long nPixDifX   = (long) ( ( pMargin->GetLeftMargin() + nIndent ) * nPPTX );
    aStartPos.X()   += nPixDifX * nLayoutSign;
    nCellX          -= nPixDifX + (long) ( pMargin->GetRightMargin() * nPPTX );     // wegen Umbruch etc.

    //  vertikale Position auf die in der Tabelle anpassen

    long nPixDifY;
    long nTopMargin = (long) ( pMargin->GetTopMargin() * nPPTY );
    SvxCellVerJustify eJust = (SvxCellVerJustify) ((const SvxVerJustifyItem&)pPattern->
                                                GetItem(ATTR_VER_JUSTIFY)).GetValue();

    //  asian vertical is always edited top-aligned
    sal_Bool bAsianVertical = ((const SfxBoolItem&)pPattern->GetItem( ATTR_STACKED )).GetValue() &&
        ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN )).GetValue();

    if ( eJust == SVX_VER_JUSTIFY_TOP ||
            ( bForceToTop && ( SC_MOD()->GetInputOptions().GetTextWysiwyg() || bAsianVertical ) ) )
        nPixDifY = nTopMargin;
    else
    {
        MapMode aMode = pDev->GetMapMode();
        pDev->SetMapMode( MAP_PIXEL );

        long nTextHeight = pDoc->GetNeededSize( nCol, nRow, nTab,
                                                pDev, nPPTX, nPPTY, aZoomX, aZoomY, false );
        if (!nTextHeight)
        {                                   // leere Zelle
            Font aFont;
            // font color doesn't matter here
            pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aZoomY );
            pDev->SetFont(aFont);
            nTextHeight = pDev->GetTextHeight() + nTopMargin +
                            (long) ( pMargin->GetBottomMargin() * nPPTY );
        }

        pDev->SetMapMode(aMode);

        if ( nTextHeight > nCellY + nTopMargin || bForceToTop )
            nPixDifY = 0;                           // zu gross -> oben anfangen
        else
        {
            if ( eJust == SVX_VER_JUSTIFY_CENTER )
                nPixDifY = nTopMargin + ( nCellY - nTextHeight ) / 2;
            else
                nPixDifY = nCellY - nTextHeight + nTopMargin;       // JUSTIFY_BOTTOM
        }
    }

    aStartPos.Y() += nPixDifY;
    nCellY      -= nPixDifY;

    if ( bLayoutRTL )
        aStartPos.X() -= nCellX - 2;    // excluding grid on both sides

                                                        //  -1 -> Gitter nicht ueberschreiben
    return Rectangle( aStartPos, Size(nCellX-1,nCellY-1) );
}

//------------------------------------------------------------------------

ScEditAttrTester::ScEditAttrTester( ScEditEngineDefaulter* pEng ) :
    pEngine( pEng ),
    pEditAttrs( NULL ),
    bNeedsObject( false ),
    bNeedsCellAttr( false )
{
    if ( pEngine->GetParagraphCount() > 1 )
    {
        bNeedsObject = sal_True;            //! Zellatribute finden ?
    }
    else
    {
        const SfxPoolItem* pItem = NULL;
        pEditAttrs = new SfxItemSet( pEngine->GetAttribs(
                                        ESelection(0,0,0,pEngine->GetTextLen(0)), EditEngineAttribs_OnlyHard ) );
        const SfxItemSet& rEditDefaults = pEngine->GetDefaults();

        for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END && !bNeedsObject; nId++)
        {
            SfxItemState eState = pEditAttrs->GetItemState( nId, false, &pItem );
            if (eState == SFX_ITEM_DONTCARE)
                bNeedsObject = sal_True;
            else if (eState == SFX_ITEM_SET)
            {
                if ( nId == EE_CHAR_ESCAPEMENT || nId == EE_CHAR_PAIRKERNING ||
                        nId == EE_CHAR_KERNING || nId == EE_CHAR_XMLATTRIBS )
                {
                    //  Escapement and kerning are kept in EditEngine because there are no
                    //  corresponding cell format items. User defined attributes are kept in
                    //  EditEngine because "user attributes applied to all the text" is different
                    //  from "user attributes applied to the cell".

                    if ( *pItem != rEditDefaults.Get(nId) )
                        bNeedsObject = sal_True;
                }
                else
                    if (!bNeedsCellAttr)
                        if ( *pItem != rEditDefaults.Get(nId) )
                            bNeedsCellAttr = sal_True;
                //  rEditDefaults contains the defaults from the cell format
            }
        }

        //  Feldbefehle enthalten?

        SfxItemState eFieldState = pEditAttrs->GetItemState( EE_FEATURE_FIELD, false );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
            bNeedsObject = sal_True;

        //  not converted characters?

        SfxItemState eConvState = pEditAttrs->GetItemState( EE_FEATURE_NOTCONV, false );
        if ( eConvState == SFX_ITEM_DONTCARE || eConvState == SFX_ITEM_SET )
            bNeedsObject = sal_True;
    }
}

ScEditAttrTester::~ScEditAttrTester()
{
    delete pEditAttrs;
}


//------------------------------------------------------------------------

ScEnginePoolHelper::ScEnginePoolHelper( SfxItemPool* pEnginePoolP,
                sal_Bool bDeleteEnginePoolP )
            :
            pEnginePool( pEnginePoolP ),
            pDefaults( NULL ),
            bDeleteEnginePool( bDeleteEnginePoolP ),
            bDeleteDefaults( false )
{
}


ScEnginePoolHelper::ScEnginePoolHelper( const ScEnginePoolHelper& rOrg )
            :
            pEnginePool( rOrg.bDeleteEnginePool ? rOrg.pEnginePool->Clone() : rOrg.pEnginePool ),
            pDefaults( NULL ),
            bDeleteEnginePool( rOrg.bDeleteEnginePool ),
            bDeleteDefaults( false )
{
}


ScEnginePoolHelper::~ScEnginePoolHelper()
{
    if ( bDeleteDefaults )
        delete pDefaults;
    if ( bDeleteEnginePool )
        SfxItemPool::Free(pEnginePool);
}


//------------------------------------------------------------------------

ScEditEngineDefaulter::ScEditEngineDefaulter( SfxItemPool* pEnginePoolP,
                sal_Bool bDeleteEnginePoolP )
            :
            ScEnginePoolHelper( pEnginePoolP, bDeleteEnginePoolP ),
            EditEngine( pEnginePoolP )
{
    //  All EditEngines use ScGlobal::GetEditDefaultLanguage as DefaultLanguage.
    //  DefaultLanguage for InputHandler's EditEngine is updated later.

    SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );
}


ScEditEngineDefaulter::ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg )
            :
            ScEnginePoolHelper( rOrg ),
            EditEngine( pEnginePool )
{
    SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );
}


ScEditEngineDefaulter::~ScEditEngineDefaulter()
{
}


void ScEditEngineDefaulter::SetDefaults( const SfxItemSet& rSet, sal_Bool bRememberCopy )
{
    if ( bRememberCopy )
    {
        if ( bDeleteDefaults )
            delete pDefaults;
        pDefaults = new SfxItemSet( rSet );
        bDeleteDefaults = sal_True;
    }
    const SfxItemSet& rNewSet = bRememberCopy ? *pDefaults : rSet;
    sal_Bool bUndo = IsUndoEnabled();
    EnableUndo( false );
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    sal_uInt16 nPara = GetParagraphCount();
    for ( sal_uInt16 j=0; j<nPara; j++ )
    {
        SetParaAttribs( j, rNewSet );
    }
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
    if ( bUndo )
        EnableUndo( sal_True );
}


void ScEditEngineDefaulter::SetDefaults( SfxItemSet* pSet, sal_Bool bTakeOwnership )
{
    if ( bDeleteDefaults )
        delete pDefaults;
    pDefaults = pSet;
    bDeleteDefaults = bTakeOwnership;
    if ( pDefaults )
        SetDefaults( *pDefaults, false );
}


void ScEditEngineDefaulter::SetDefaultItem( const SfxPoolItem& rItem )
{
    if ( !pDefaults )
    {
        pDefaults = new SfxItemSet( GetEmptyItemSet() );
        bDeleteDefaults = sal_True;
    }
    pDefaults->Put( rItem );
    SetDefaults( *pDefaults, false );
}

const SfxItemSet& ScEditEngineDefaulter::GetDefaults()
{
    if ( !pDefaults )
    {
        pDefaults = new SfxItemSet( GetEmptyItemSet() );
        bDeleteDefaults = sal_True;
    }
    return *pDefaults;
}

void ScEditEngineDefaulter::SetText( const EditTextObject& rTextObject )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rTextObject );
    if ( pDefaults )
        SetDefaults( *pDefaults, false );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

void ScEditEngineDefaulter::SetTextNewDefaults( const EditTextObject& rTextObject,
            const SfxItemSet& rSet, sal_Bool bRememberCopy )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rTextObject );
    SetDefaults( rSet, bRememberCopy );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

void ScEditEngineDefaulter::SetTextNewDefaults( const EditTextObject& rTextObject,
            SfxItemSet* pSet, sal_Bool bTakeOwnership )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rTextObject );
    SetDefaults( pSet, bTakeOwnership );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}


void ScEditEngineDefaulter::SetText( const String& rText )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rText );
    if ( pDefaults )
        SetDefaults( *pDefaults, false );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

void ScEditEngineDefaulter::SetTextNewDefaults( const String& rText,
            const SfxItemSet& rSet, sal_Bool bRememberCopy )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rText );
    SetDefaults( rSet, bRememberCopy );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

void ScEditEngineDefaulter::SetTextNewDefaults( const String& rText,
            SfxItemSet* pSet, sal_Bool bTakeOwnership )
{
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    EditEngine::SetText( rText );
    SetDefaults( pSet, bTakeOwnership );
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

void ScEditEngineDefaulter::RepeatDefaults()
{
    if ( pDefaults )
    {
        sal_uInt16 nPara = GetParagraphCount();
        for ( sal_uInt16 j=0; j<nPara; j++ )
            SetParaAttribs( j, *pDefaults );
    }
}

void ScEditEngineDefaulter::RemoveParaAttribs()
{
    SfxItemSet* pCharItems = NULL;
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    sal_uInt16 nParCount = GetParagraphCount();
    for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
    {
        const SfxItemSet& rParaAttribs = GetParaAttribs( nPar );
        sal_uInt16 nWhich;
        for (nWhich = EE_CHAR_START; nWhich <= EE_CHAR_END; nWhich ++)
        {
            const SfxPoolItem* pParaItem;
            if ( rParaAttribs.GetItemState( nWhich, false, &pParaItem ) == SFX_ITEM_SET )
            {
                //  if defaults are set, use only items that are different from default
                if ( !pDefaults || *pParaItem != pDefaults->Get(nWhich) )
                {
                    if (!pCharItems)
                        pCharItems = new SfxItemSet( GetEmptyItemSet() );
                    pCharItems->Put( *pParaItem );
                }
            }
        }

        if ( pCharItems )
        {
            std::vector<sal_uInt16> aPortions;
            GetPortions( nPar, aPortions );

            //  loop through the portions of the paragraph, and set only those items
            //  that are not overridden by existing character attributes

            sal_uInt16 nStart = 0;
            for ( std::vector<sal_uInt16>::const_iterator it(aPortions.begin()); it != aPortions.end(); ++it )
            {
                sal_uInt16 nEnd = *it;
                ESelection aSel( nPar, nStart, nPar, nEnd );
                SfxItemSet aOldCharAttrs = GetAttribs( aSel );
                SfxItemSet aNewCharAttrs = *pCharItems;
                for (nWhich = EE_CHAR_START; nWhich <= EE_CHAR_END; nWhich ++)
                {
                    //  Clear those items that are different from existing character attributes.
                    //  Where no character attributes are set, GetAttribs returns the paragraph attributes.
                    const SfxPoolItem* pItem;
                    if ( aNewCharAttrs.GetItemState( nWhich, false, &pItem ) == SFX_ITEM_SET &&
                         *pItem != aOldCharAttrs.Get(nWhich) )
                    {
                        aNewCharAttrs.ClearItem(nWhich);
                    }
                }
                if ( aNewCharAttrs.Count() )
                    QuickSetAttribs( aNewCharAttrs, aSel );

                nStart = nEnd;
            }

            DELETEZ( pCharItems );
        }

        if ( rParaAttribs.Count() )
        {
            //  clear all paragraph attributes (including defaults),
            //  so they are not contained in resulting EditTextObjects

            SetParaAttribs( nPar, SfxItemSet( *rParaAttribs.GetPool(), rParaAttribs.GetRanges() ) );
        }
    }
    if ( bUpdateMode )
        SetUpdateMode( sal_True );
}

//------------------------------------------------------------------------

ScTabEditEngine::ScTabEditEngine( ScDocument* pDoc )
        : ScEditEngineDefaulter( pDoc->GetEnginePool() )
{
    SetEditTextObjectPool( pDoc->GetEditPool() );
    Init((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN));
}

ScTabEditEngine::ScTabEditEngine( const ScPatternAttr& rPattern,
            SfxItemPool* pEnginePoolP, SfxItemPool* pTextObjectPool )
        : ScEditEngineDefaulter( pEnginePoolP )
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    Init( rPattern );
}

void ScTabEditEngine::Init( const ScPatternAttr& rPattern )
{
    SetRefMapMode(MAP_100TH_MM);
    SfxItemSet* pEditDefaults = new SfxItemSet( GetEmptyItemSet() );
    rPattern.FillEditItemSet( pEditDefaults );
    SetDefaults( pEditDefaults );
    // wir haben keine StyleSheets fuer Text
    SetControlWord( GetControlWord() & ~EE_CNTRL_RTFSTYLESHEETS );
}

//------------------------------------------------------------------------
//      Feldbefehle fuer Kopf- und Fusszeilen
//------------------------------------------------------------------------

//
//      Zahlen aus \sw\source\core\doc\numbers.cxx
//

static String lcl_GetCharStr( sal_Int32 nNo )
{
    OSL_ENSURE( nNo, "0 ist eine ungueltige Nummer !!" );
    String aStr;

    const sal_Int32 coDiff = 'Z' - 'A' +1;
    sal_Int32 nCalc;

    do {
        nCalc = nNo % coDiff;
        if( !nCalc )
            nCalc = coDiff;
        aStr.Insert( (sal_Unicode)('a' - 1 + nCalc ), 0 );
        nNo = sal::static_int_cast<sal_Int32>( nNo - nCalc );
        if( nNo )
            nNo /= coDiff;
    } while( nNo );
    return aStr;
}

static String lcl_GetNumStr( sal_Int32 nNo, SvxNumType eType )
{
    String aTmpStr(rtl::OUString('0'));
    if( nNo )
    {
        switch( eType )
        {
        case SVX_CHARS_UPPER_LETTER:
        case SVX_CHARS_LOWER_LETTER:
            aTmpStr = lcl_GetCharStr( nNo );
            break;

        case SVX_ROMAN_UPPER:
        case SVX_ROMAN_LOWER:
            if( nNo < 4000 )
                aTmpStr = SvxNumberFormat::CreateRomanString( nNo, ( eType == SVX_ROMAN_UPPER ) );
            else
                aTmpStr.Erase();
            break;

        case SVX_NUMBER_NONE:
            aTmpStr.Erase();
            break;

//      CHAR_SPECIAL:
//          ????

//      case ARABIC:    ist jetzt default
        default:
            aTmpStr = String::CreateFromInt32( nNo );
            break;
        }

        if( SVX_CHARS_UPPER_LETTER == eType )
            aTmpStr.ToUpperAscii();
    }
    return aTmpStr;
}

ScHeaderFieldData::ScHeaderFieldData()
    :
        aDate( Date::EMPTY ),
        aTime( Time::EMPTY )
{
    nPageNo = nTotalPages = 0;
    eNumType = SVX_ARABIC;
}

ScHeaderEditEngine::ScHeaderEditEngine( SfxItemPool* pEnginePoolP, sal_Bool bDeleteEnginePoolP )
        : ScEditEngineDefaulter( pEnginePoolP, bDeleteEnginePoolP )
{
}

String ScHeaderEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    sal_uInt16 /* nPara */, sal_uInt16 /* nPos */,
                                    Color*& /* rTxtColor */, Color*& /* rFldColor */ )
{
    const SvxFieldData* pFieldData = rField.GetField();
    if (!pFieldData)
        return rtl::OUString("?");

    rtl::OUString aRet;
    sal_Int32 nClsId = pFieldData->GetClassId();
    switch (nClsId)
    {
        case text::textfield::Type::PAGE:
            aRet = lcl_GetNumStr( aData.nPageNo,aData.eNumType );
        break;
        case text::textfield::Type::PAGES:
            aRet = lcl_GetNumStr( aData.nTotalPages,aData.eNumType );
        break;
        case text::textfield::Type::TIME:
            aRet = ScGlobal::pLocaleData->getTime(aData.aTime);
        break;
        case text::textfield::Type::DOCINFO_TITLE:
            aRet = aData.aTitle;
        break;
        case text::textfield::Type::EXTENDED_FILE:
        {
            switch (static_cast<const SvxExtFileField*>(pFieldData)->GetFormat())
            {
                case SVXFILEFORMAT_FULLPATH :
                    aRet = aData.aLongDocName;
                break;
                default:
                    aRet = aData.aShortDocName;
            }
        }
        break;
        case text::textfield::Type::TABLE:
            aRet = aData.aTabName;
        break;
        case text::textfield::Type::DATE:
            aRet = ScGlobal::pLocaleData->getDate(aData.aDate);
        break;
        default:
            aRet = "?";
    }

    return aRet;
}

//------------------------------------------------------------------------
//
//                          Feld-Daten
//
//------------------------------------------------------------------------

ScFieldEditEngine::ScFieldEditEngine(
    ScDocument* pDoc, SfxItemPool* pEnginePoolP,
    SfxItemPool* pTextObjectPool, bool bDeleteEnginePoolP) :
        ScEditEngineDefaulter( pEnginePoolP, bDeleteEnginePoolP ),
        mpDoc(pDoc), bExecuteURL(true)
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    //  EE_CNTRL_URLSFXEXECUTE nicht, weil die Edit-Engine den ViewFrame nicht kennt
    // wir haben keine StyleSheets fuer Text
    SetControlWord( (GetControlWord() | EE_CNTRL_MARKFIELDS) & ~EE_CNTRL_RTFSTYLESHEETS );
}

String ScFieldEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    sal_uInt16 /* nPara */, sal_uInt16 /* nPos */,
                                    Color*& rTxtColor, Color*& /* rFldColor */ )
{
    rtl::OUString aRet;
    const SvxFieldData* pFieldData = rField.GetField();

    if (!pFieldData)
        return rtl::OUString(" ");

    sal_uInt16 nClsId = pFieldData->GetClassId();
    switch (nClsId)
    {
        case text::textfield::Type::URL:
        {
            const SvxURLField* pField = static_cast<const SvxURLField*>(pFieldData);
            rtl::OUString aURL = pField->GetURL();

            switch (pField->GetFormat())
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
                case SVXURLFORMAT_REPR:
                    aRet = pField->GetRepresentation();
                break;
                case SVXURLFORMAT_URL:
                    aRet = aURL;
                break;
                default:
                    ;
            }

            svtools::ColorConfigEntry eEntry =
                INetURLHistory::GetOrCreate()->QueryUrl(String(aURL)) ? svtools::LINKSVISITED : svtools::LINKS;
            rTxtColor = new Color( SC_MOD()->GetColorConfig().GetColorValue(eEntry).nColor );
        }
        break;
        case text::textfield::Type::EXTENDED_TIME:
        {
            const SvxExtTimeField* pField = static_cast<const SvxExtTimeField*>(pFieldData);
            if (mpDoc)
                aRet = pField->GetFormatted(*mpDoc->GetFormatTable(), ScGlobal::eLnge);
        }
        break;
        case text::textfield::Type::DATE:
        {
            Date aDate(Date::SYSTEM);
            aRet = ScGlobal::pLocaleData->getDate(aDate);
        }
        break;
        case text::textfield::Type::DOCINFO_TITLE:
        {
            SfxObjectShell* pDocShell = mpDoc->GetDocumentShell();
            aRet = pDocShell->getDocProperties()->getTitle();
            if (aRet.isEmpty())
                aRet = pDocShell->GetTitle();
        }
        break;
        case text::textfield::Type::TABLE:
        {
            const SvxTableField* pField = static_cast<const SvxTableField*>(pFieldData);
            SCTAB nTab = pField->GetTab();
            rtl::OUString aName;
            if (mpDoc->GetName(nTab, aName))
                aRet = aName;
            else
                aRet = "?";
        }
        break;
        default:
            aRet = "?";
    }

    if (aRet.isEmpty())        // leer ist baeh
        aRet = " ";         // Space ist Default der Editengine

    return aRet;
}

void ScFieldEditEngine::FieldClicked( const SvxFieldItem& rField, sal_uInt16, sal_uInt16 )
{
    const SvxFieldData* pFld = rField.GetField();

    if ( pFld && pFld->ISA( SvxURLField ) && bExecuteURL )
    {
        const SvxURLField* pURLField = (const SvxURLField*) pFld;
        ScGlobal::OpenURL( pURLField->GetURL(), pURLField->GetTargetFrame() );
    }
}

//------------------------------------------------------------------------

ScNoteEditEngine::ScNoteEditEngine( SfxItemPool* pEnginePoolP,
            SfxItemPool* pTextObjectPool, sal_Bool bDeleteEnginePoolP ) :
    ScEditEngineDefaulter( pEnginePoolP, bDeleteEnginePoolP )
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    SetControlWord( (GetControlWord() | EE_CNTRL_MARKFIELDS) & ~EE_CNTRL_RTFSTYLESHEETS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
