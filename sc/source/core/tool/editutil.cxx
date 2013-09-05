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

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/editobj.hxx>
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

// STATIC DATA
//  Delimiters zusaetzlich zu EditEngine-Default:

const sal_Char ScEditUtil::pCalcDelimiters[] = "=()+-*/^&<>";

OUString ScEditUtil::ModifyDelimiters( const OUString& rOld )
{
    // underscore is used in function argument names
    OUString aRet = OUString( comphelper::string::remove(rOld, '_') ) +
        OUString::createFromAscii( pCalcDelimiters ) +
        ScCompiler::GetNativeSymbol(ocSep); // argument separator is localized.
    return aRet;
}

static OUString lcl_GetDelimitedString( const EditEngine& rEngine, const sal_Char c )
{
    sal_Int32 nParCount = rEngine.GetParagraphCount();
    OUStringBuffer aRet( nParCount * 80 );
    for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
    {
        if (nPar > 0)
            aRet.append(c);
        aRet.append( rEngine.GetText( nPar ));
    }
    return aRet.makeStringAndClear();
}

static OUString lcl_GetDelimitedString( const EditTextObject& rEdit, const sal_Char c )
{
    sal_Int32 nParCount = rEdit.GetParagraphCount();
    OUStringBuffer aRet( nParCount * 80 );
    for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
    {
        if (nPar > 0)
            aRet.append(c);
        aRet.append( rEdit.GetText( nPar ));
    }
    return aRet.makeStringAndClear();
}

OUString ScEditUtil::GetSpaceDelimitedString( const EditEngine& rEngine )
{
    return lcl_GetDelimitedString(rEngine, ' ');
}

OUString ScEditUtil::GetSpaceDelimitedString( const EditTextObject& rEdit )
{
    return lcl_GetDelimitedString(rEdit, ' ');
}

OUString ScEditUtil::GetMultilineString( const EditEngine& rEngine )
{
    return lcl_GetDelimitedString(rEngine, '\n');
}

OUString ScEditUtil::GetMultilineString( const EditTextObject& rEdit )
{
    return lcl_GetDelimitedString(rEdit, '\n');
}

OUString ScEditUtil::GetString( const EditTextObject& rEditText, const ScDocument* pDoc )
{
    // ScFieldEditEngine is needed to resolve field contents.
    if (pDoc)
    {
        /* TODO: make ScDocument::GetEditEngine() const? Most likely it's only
         * not const because of the pointer assignment, make that mutable, and
         * then remove the ugly const_cast here. */
        EditEngine& rEE = const_cast<ScDocument*>(pDoc)->GetEditEngine();
        rEE.SetText( rEditText);
        return GetMultilineString( rEE);
    }
    else
    {
        static osl::Mutex aMutex;
        osl::MutexGuard aGuard( aMutex);
        EditEngine& rEE = ScGlobal::GetStaticFieldEditEngine();
        rEE.SetText( rEditText);
        return GetMultilineString( rEE);
    }
}

EditTextObject* ScEditUtil::CreateURLObjectFromURL( ScDocument& rDoc, const OUString& rURL, const OUString& rText )
{
    SvxURLField aUrlField( rURL, rText, SVXURLFORMAT_APPDEFAULT);
    EditEngine& rEE = rDoc.GetEditEngine();
    rEE.SetText( EMPTY_OUSTRING );
    rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ),
            ESelection( EE_PARA_MAX_COUNT, EE_TEXTPOS_MAX_COUNT ) );

    return rEE.CreateTextObject();
}

void ScEditUtil::RemoveCharAttribs( EditTextObject& rEditText, const ScPatternAttr& rAttr )
{
    const struct {
        sal_uInt16 nAttrType;
        sal_uInt16 nCharType;
    } AttrTypeMap[] = {
        { ATTR_FONT,        EE_CHAR_FONTINFO },
        { ATTR_FONT_HEIGHT, EE_CHAR_FONTHEIGHT },
        { ATTR_FONT_WEIGHT, EE_CHAR_WEIGHT },
        { ATTR_FONT_COLOR,  EE_CHAR_COLOR }
    };
    sal_uInt16 nMapCount = sizeof (AttrTypeMap) / sizeof (AttrTypeMap[0]);

    const SfxItemSet& rSet = rAttr.GetItemSet();
    const SfxPoolItem* pItem;
    for (sal_uInt16 i = 0; i < nMapCount; ++i)
    {
        if ( rSet.GetItemState(AttrTypeMap[i].nAttrType, false, &pItem) == SFX_ITEM_SET )
            rEditText.RemoveCharAttribs(AttrTypeMap[i].nCharType);
    }
}

EditTextObject* ScEditUtil::Clone( const EditTextObject& rObj, ScDocument& rDestDoc )
{
    EditTextObject* pNew = NULL;

    EditEngine& rEngine = rDestDoc.GetEditEngine();
    if (rObj.HasOnlineSpellErrors())
    {
        sal_uLong nControl = rEngine.GetControlWord();
        const sal_uLong nSpellControl = EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS;
        bool bNewControl = ( (nControl & nSpellControl) != nSpellControl );
        if (bNewControl)
            rEngine.SetControlWord(nControl | nSpellControl);
        rEngine.SetText(rObj);
        pNew = rEngine.CreateTextObject();
        if (bNewControl)
            rEngine.SetControlWord(nControl);
    }
    else
    {
        rEngine.SetText(rObj);
        pNew = rEngine.CreateTextObject();
    }

    return pNew;
}

OUString ScEditUtil::GetCellFieldValue(
    const SvxFieldData& rFieldData, const ScDocument* pDoc, Color** ppTextColor )
{
    OUString aRet;
    switch (rFieldData.GetClassId())
    {
        case text::textfield::Type::URL:
        {
            const SvxURLField& rField = static_cast<const SvxURLField&>(rFieldData);
            OUString aURL = rField.GetURL();

            switch (rField.GetFormat())
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
                case SVXURLFORMAT_REPR:
                    aRet = rField.GetRepresentation();
                break;
                case SVXURLFORMAT_URL:
                    aRet = aURL;
                break;
                default:
                    ;
            }

            svtools::ColorConfigEntry eEntry =
                INetURLHistory::GetOrCreate()->QueryUrl(aURL) ? svtools::LINKSVISITED : svtools::LINKS;

            if (ppTextColor)
                *ppTextColor = new Color( SC_MOD()->GetColorConfig().GetColorValue(eEntry).nColor );
        }
        break;
        case text::textfield::Type::EXTENDED_TIME:
        {
            const SvxExtTimeField& rField = static_cast<const SvxExtTimeField&>(rFieldData);
            if (pDoc)
                aRet = rField.GetFormatted(*pDoc->GetFormatTable(), ScGlobal::eLnge);
            else
            {
                /* TODO: quite expensive, we could have a global formatter? */
                SvNumberFormatter aFormatter( comphelper::getProcessComponentContext(), ScGlobal::eLnge );
                aRet = rField.GetFormatted(aFormatter, ScGlobal::eLnge);
            }
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
            if (pDoc)
            {
                SfxObjectShell* pDocShell = pDoc->GetDocumentShell();
                if (pDocShell)
                {
                    aRet = pDocShell->getDocProperties()->getTitle();
                    if (aRet.isEmpty())
                        aRet = pDocShell->GetTitle();
                }
            }
            if (aRet.isEmpty())
                aRet = "?";
        }
        break;
        case text::textfield::Type::TABLE:
        {
            const SvxTableField& rField = static_cast<const SvxTableField&>(rFieldData);
            SCTAB nTab = rField.GetTab();
            OUString aName;
            if (pDoc && pDoc->GetName(nTab, aName))
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
    sal_Int32 nPara = GetParagraphCount();
    for ( sal_Int32 j=0; j<nPara; j++ )
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

void ScEditEngineDefaulter::SetText( const OUString& rText )
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
        sal_Int32 nPara = GetParagraphCount();
        for ( sal_Int32 j=0; j<nPara; j++ )
            SetParaAttribs( j, *pDefaults );
    }
}

void ScEditEngineDefaulter::RemoveParaAttribs()
{
    SfxItemSet* pCharItems = NULL;
    sal_Bool bUpdateMode = GetUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( false );
    sal_Int32 nParCount = GetParagraphCount();
    for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
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

//      Feldbefehle fuer Kopf- und Fusszeilen

//
//      Zahlen aus \sw\source\core\doc\numbers.cxx
//

static String lcl_GetCharStr( sal_Int32 nNo )
{
    OSL_ENSURE( nNo, "0 is an invalid number !!" );
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

static OUString lcl_GetNumStr(sal_Int32 nNo, SvxNumType eType)
{
    OUString aTmpStr('0');
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
                aTmpStr = OUString();
            break;

        case SVX_NUMBER_NONE:
            aTmpStr = OUString();
            break;

//      CHAR_SPECIAL:
//          ????

//      case ARABIC:    ist jetzt default
        default:
            aTmpStr = OUString::number(nNo);
            break;
        }

        if( SVX_CHARS_UPPER_LETTER == eType )
            aTmpStr = aTmpStr.toAsciiUpperCase();
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

OUString ScHeaderEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    sal_Int32 /* nPara */, sal_uInt16 /* nPos */,
                                    Color*& /* rTxtColor */, Color*& /* rFldColor */ )
{
    const SvxFieldData* pFieldData = rField.GetField();
    if (!pFieldData)
        return OUString("?");

    OUString aRet;
    sal_Int32 nClsId = pFieldData->GetClassId();
    switch (nClsId)
    {
        case text::textfield::Type::PAGE:
            aRet = lcl_GetNumStr( aData.nPageNo,aData.eNumType );
        break;
        case text::textfield::Type::PAGES:
            aRet = lcl_GetNumStr( aData.nTotalPages,aData.eNumType );
        break;
        case text::textfield::Type::EXTENDED_TIME:
        case text::textfield::Type::TIME:
            // For now, time field in the header / footer is always dynamic.
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

//
//                          Feld-Daten
//

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

OUString ScFieldEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    sal_Int32 /* nPara */, sal_uInt16 /* nPos */,
                                    Color*& rTxtColor, Color*& /* rFldColor */ )
{
    const SvxFieldData* pFieldData = rField.GetField();

    if (!pFieldData)
        return OUString(" ");

    return ScEditUtil::GetCellFieldValue(*pFieldData, mpDoc, &rTxtColor);
}

void ScFieldEditEngine::FieldClicked( const SvxFieldItem& rField, sal_Int32, sal_uInt16 )
{
    const SvxFieldData* pFld = rField.GetField();

    if ( pFld && pFld->ISA( SvxURLField ) && bExecuteURL )
    {
        const SvxURLField* pURLField = (const SvxURLField*) pFld;
        ScGlobal::OpenURL( pURLField->GetURL(), pURLField->GetTargetFrame() );
    }
}

ScNoteEditEngine::ScNoteEditEngine( SfxItemPool* pEnginePoolP,
            SfxItemPool* pTextObjectPool, sal_Bool bDeleteEnginePoolP ) :
    ScEditEngineDefaulter( pEnginePoolP, bDeleteEnginePoolP )
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    SetControlWord( (GetControlWord() | EE_CNTRL_MARKFIELDS) & ~EE_CNTRL_RTFSTYLESHEETS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
