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

#include <vcl/svapp.hxx>
#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <sal/macros.h>
#include <tools/rcid.h>
#include <unotools/charclass.hxx>
#include <unotools/securityoptions.hxx>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <numeric>
#include <svx/svdmodel.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <comphelper/string.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include "global.hxx"
#include "scresid.hxx"
#include "autoform.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "addincol.hxx"
#include "adiasync.hxx"
#include "userlist.hxx"
#include "interpre.hxx"
#include "strload.hxx"
#include "docpool.hxx"
#include "unitconv.hxx"
#include "compiler.hxx"
#include "parclass.hxx"
#include "funcdesc.hxx"
#include "globstr.hrc"
#include "scfuncs.hrc"
#include "sc.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "editutil.hxx"
#include "docsh.hxx"

tools::SvRef<ScDocShell>  ScGlobal::xDrawClipDocShellRef;
SvxSearchItem*  ScGlobal::pSearchItem = nullptr;
ScAutoFormat*   ScGlobal::pAutoFormat = nullptr;
LegacyFuncCollection* ScGlobal::pLegacyFuncCollection = nullptr;
ScUnoAddInCollection* ScGlobal::pAddInCollection = nullptr;
ScUserList*     ScGlobal::pUserList = nullptr;
OUString**      ScGlobal::ppRscString = nullptr;
LanguageType    ScGlobal::eLnge = LANGUAGE_SYSTEM;
css::lang::Locale*     ScGlobal::pLocale = nullptr;
SvtSysLocale*   ScGlobal::pSysLocale = nullptr;
const CharClass*  ScGlobal::pCharClass = nullptr;
const LocaleDataWrapper*  ScGlobal::pLocaleData = nullptr;
CalendarWrapper* ScGlobal::pCalendar = nullptr;
CollatorWrapper* ScGlobal::pCollator = nullptr;
CollatorWrapper* ScGlobal::pCaseCollator = nullptr;
::utl::TransliterationWrapper* ScGlobal::pTransliteration = nullptr;
::utl::TransliterationWrapper* ScGlobal::pCaseTransliteration = nullptr;
css::uno::Reference< css::i18n::XOrdinalSuffix> ScGlobal::xOrdinalSuffix = nullptr;
IntlWrapper*    ScGlobal::pScIntlWrapper = nullptr;
sal_Unicode     ScGlobal::cListDelimiter = ',';
OUString*       ScGlobal::pEmptyOUString = nullptr;
OUString*       ScGlobal::pStrClipDocName = nullptr;

SvxBrushItem*   ScGlobal::pEmptyBrushItem = nullptr;
SvxBrushItem*   ScGlobal::pButtonBrushItem = nullptr;
SvxBrushItem*   ScGlobal::pEmbeddedBrushItem = nullptr;
SvxBrushItem*   ScGlobal::pProtectedBrushItem = nullptr;

ScFunctionList* ScGlobal::pStarCalcFunctionList = nullptr;
ScFunctionMgr*  ScGlobal::pStarCalcFunctionMgr  = nullptr;

ScUnitConverter* ScGlobal::pUnitConverter = nullptr;
SvNumberFormatter* ScGlobal::pEnglishFormatter = nullptr;
ScFieldEditEngine* ScGlobal::pFieldEditEngine = nullptr;

double          ScGlobal::nScreenPPTX           = 96.0;
double          ScGlobal::nScreenPPTY           = 96.0;

sal_uInt16          ScGlobal::nDefFontHeight        = 225;
sal_uInt16          ScGlobal::nStdRowHeight         = 256;

long            ScGlobal::nLastRowHeightExtra   = 0;
long            ScGlobal::nLastColWidthExtra    = STD_EXTRA_WIDTH;

SfxViewShell* pScActiveViewShell = nullptr; //FIXME: Make this a member
sal_uInt16 nScClickMouseModifier = 0;    //FIXME: This too
sal_uInt16 nScFillModeMouseModifier = 0; //FIXME: And this

// Static functions

bool ScGlobal::HasAttrChanged( const SfxItemSet&  rNewAttrs,
                               const SfxItemSet&  rOldAttrs,
                               const sal_uInt16       nWhich )
{
    bool                bInvalidate = false;
    const SfxPoolItem*  pNewItem    = nullptr;
    const SfxItemState  eNewState   = rNewAttrs.GetItemState( nWhich, true, &pNewItem );
    const SfxPoolItem*  pOldItem    = nullptr;
    const SfxItemState  eOldState   = rOldAttrs.GetItemState( nWhich, true, &pOldItem );

    if ( eNewState == eOldState )
    {
        // Both Items set
        // PoolItems, meaning comparing pointers is valid
        if ( SfxItemState::SET == eOldState )
            bInvalidate = (pNewItem != pOldItem);
    }
    else
    {
        // Contains a Default Item
        // PoolItems, meaning Item comparison necessary
        if (!pOldItem)
            pOldItem = &rOldAttrs.GetPool()->GetDefaultItem( nWhich );

        if (!pNewItem)
            pNewItem = &rNewAttrs.GetPool()->GetDefaultItem( nWhich );

        bInvalidate = (*pNewItem != *pOldItem);
    }

    return bInvalidate;
}

sal_uInt32 ScGlobal::GetStandardFormat( SvNumberFormatter& rFormatter,
        sal_uInt32 nFormat, short nType )
{
    const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
    if ( pFormat )
        return rFormatter.GetStandardFormat( nFormat, nType, pFormat->GetLanguage() );
    return rFormatter.GetStandardFormat( nType, eLnge );
}

sal_uInt16 ScGlobal::GetStandardRowHeight()
{
    return nStdRowHeight;
}

SvNumberFormatter* ScGlobal::GetEnglishFormatter()
{
    if ( !pEnglishFormatter )
    {
        pEnglishFormatter = new SvNumberFormatter(
            ::comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US );
        pEnglishFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
    }
    return pEnglishFormatter;
}

bool ScGlobal::CheckWidthInvalidate( bool& bNumFormatChanged,
                                     const SfxItemSet& rNewAttrs,
                                     const SfxItemSet& rOldAttrs )
{
    // Check whether attribute changes in rNewAttrs compared to rOldAttrs render
    // the text width at a cell invalid
    bNumFormatChanged =
            HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_VALUE_FORMAT );
    return ( bNumFormatChanged
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_LANGUAGE_FORMAT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_HEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_HEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_HEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_WEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_WEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_WEIGHT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_POSTURE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_POSTURE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_POSTURE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_UNDERLINE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_OVERLINE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_CROSSEDOUT )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_CONTOUR )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_SHADOWED )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_STACKED )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_ROTATE_VALUE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_ROTATE_MODE )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_LINEBREAK )
        || HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_MARGIN )
        );
}

const SvxSearchItem& ScGlobal::GetSearchItem()
{
    if (!pSearchItem)
    {
        pSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );
        pSearchItem->SetAppFlag( SvxSearchApp::CALC );
    }
    return *pSearchItem;
}

void ScGlobal::SetSearchItem( const SvxSearchItem& rNew )
{
    // FIXME: An assignement operator would be nice here
    delete pSearchItem;
    pSearchItem = static_cast<SvxSearchItem*>(rNew.Clone());

    pSearchItem->SetWhich( SID_SEARCH_ITEM );
    pSearchItem->SetAppFlag( SvxSearchApp::CALC );
}

void ScGlobal::ClearAutoFormat()
{
    if (pAutoFormat!=nullptr)
    {
        delete pAutoFormat;
        pAutoFormat=nullptr;
    }
}

ScAutoFormat* ScGlobal::GetAutoFormat()
{
    return pAutoFormat;
}

ScAutoFormat* ScGlobal::GetOrCreateAutoFormat()
{
    if ( !pAutoFormat )
    {
        pAutoFormat = new ScAutoFormat;
        pAutoFormat->Load();
    }

    return pAutoFormat;
}

LegacyFuncCollection* ScGlobal::GetLegacyFuncCollection()
{
    if (!pLegacyFuncCollection)
        pLegacyFuncCollection = new LegacyFuncCollection();
    return pLegacyFuncCollection;
}

ScUnoAddInCollection* ScGlobal::GetAddInCollection()
{
    if (!pAddInCollection)
        pAddInCollection = new ScUnoAddInCollection();
    return pAddInCollection;
}

ScUserList* ScGlobal::GetUserList()
{
    // Hack: Load Cfg item at the App
    global_InitAppOptions();

    if (!pUserList)
        pUserList = new ScUserList();
    return pUserList;
}

void ScGlobal::SetUserList( const ScUserList* pNewList )
{
    if ( pNewList )
    {
        if ( !pUserList )
            pUserList = new ScUserList( *pNewList );
        else
            *pUserList = *pNewList;
    }
    else
    {
        delete pUserList;
        pUserList = nullptr;
    }
}

const OUString& ScGlobal::GetRscString( sal_uInt16 nIndex )
{
    assert( nIndex < SC_GLOBSTR_STR_COUNT);
    if( !ppRscString[ nIndex ] )
    {
        OpCode eOp = ocNone;
        // Map former globstr.src strings moved to compiler.src
        switch (nIndex)
        {
            case STR_NULL_ERROR:
                eOp = ocErrNull;
                break;
            case STR_DIV_ZERO:
                eOp = ocErrDivZero;
                break;
            case STR_NO_VALUE:
                eOp = ocErrValue;
                break;
            case STR_NOREF_STR:
                eOp = ocErrRef;
                break;
            case STR_NO_NAME_REF:
                eOp = ocErrName;
                break;
            case STR_NUM_ERROR:
                eOp = ocErrNum;
                break;
            case STR_NV_STR:
                eOp = ocErrNA;
                break;
            default:
                ;   // nothing
        }
        if (eOp != ocNone)
            ppRscString[ nIndex ] = new OUString( ScCompiler::GetNativeSymbol( eOp));
        else
            ppRscString[ nIndex ] = new OUString( SC_STRLOAD( RID_GLOBSTR, nIndex ));
    }
    return *ppRscString[ nIndex ];
}

OUString ScGlobal::GetErrorString(FormulaError nErr)
{
    sal_uInt16 nErrNumber;
    switch (nErr)
    {
        case FormulaError::NotAvailable       : nErrNumber = STR_NV_STR; break;
        case FormulaError::NoRef              : nErrNumber = STR_NO_REF_TABLE; break;
        case FormulaError::NoName             : nErrNumber = STR_NO_NAME_REF; break;
        case FormulaError::NoAddin            : nErrNumber = STR_NO_ADDIN; break;
        case FormulaError::NoMacro            : nErrNumber = STR_NO_MACRO; break;
        case FormulaError::DoubleRef          :
        case FormulaError::NoValue            : nErrNumber = STR_NO_VALUE; break;
        case FormulaError::NoCode             : nErrNumber = STR_NULL_ERROR; break;
        case FormulaError::DivisionByZero     : nErrNumber = STR_DIV_ZERO; break;
        case FormulaError::IllegalFPOperation : nErrNumber = STR_NUM_ERROR; break;

        default          : return GetRscString(STR_ERROR_STR) + OUString::number( (int)nErr );
    }
    return GetRscString( nErrNumber );
}

OUString ScGlobal::GetLongErrorString(FormulaError nErr)
{
    sal_uInt16 nErrNumber;
    switch (nErr)
    {
        case FormulaError::NONE:
            nErrNumber = 0;
            break;
        case FormulaError::IllegalArgument:
            nErrNumber = STR_LONG_ERR_ILL_ARG;
        break;
        case FormulaError::IllegalFPOperation:
            nErrNumber = STR_LONG_ERR_ILL_FPO;
        break;
        case FormulaError::IllegalChar:
            nErrNumber = STR_LONG_ERR_ILL_CHAR;
        break;
        case FormulaError::IllegalParameter:
            nErrNumber = STR_LONG_ERR_ILL_PAR;
        break;
        case FormulaError::Separator:
            nErrNumber = STR_LONG_ERR_ILL_SEP;
        break;
        case FormulaError::Pair:
        case FormulaError::PairExpected:
            nErrNumber = STR_LONG_ERR_PAIR;
        break;
        case FormulaError::OperatorExpected:
            nErrNumber = STR_LONG_ERR_OP_EXP;
        break;
        case FormulaError::VariableExpected:
        case FormulaError::ParameterExpected:
            nErrNumber = STR_LONG_ERR_VAR_EXP;
        break;
        case FormulaError::CodeOverflow:
            nErrNumber = STR_LONG_ERR_CODE_OVF;
        break;
        case FormulaError::StringOverflow:
            nErrNumber = STR_LONG_ERR_STR_OVF;
        break;
        case FormulaError::StackOverflow:
            nErrNumber = STR_LONG_ERR_STACK_OVF;
        break;
        case FormulaError::MatrixSize:
            nErrNumber = STR_LONG_ERR_MATRIX_SIZE;
        break;
        case FormulaError::IllegalJump:
        case FormulaError::UnknownState:
        case FormulaError::UnknownVariable:
        case FormulaError::UnknownOpCode:
        case FormulaError::UnknownStackVariable:
        case FormulaError::UnknownToken:
        case FormulaError::NoCode:
        case FormulaError::DoubleRef:
            nErrNumber = STR_LONG_ERR_SYNTAX;
        break;
        case FormulaError::CircularReference:
            nErrNumber = STR_LONG_ERR_CIRC_REF;
        break;
        case FormulaError::NoConvergence:
            nErrNumber = STR_LONG_ERR_NO_CONV;
        break;
        case FormulaError::NoRef:
            nErrNumber = STR_LONG_ERR_NO_REF;
        break;
        case FormulaError::NoName:
            nErrNumber = STR_LONG_ERR_NO_NAME;
        break;
        case FormulaError::NoAddin:
            nErrNumber = STR_LONG_ERR_NO_ADDIN;
        break;
        case FormulaError::NoMacro:
            nErrNumber = STR_LONG_ERR_NO_MACRO;
        break;
        case FormulaError::DivisionByZero:
            nErrNumber = STR_LONG_ERR_DIV_ZERO;
        break;
        case FormulaError::NestedArray:
            nErrNumber = STR_ERR_LONG_NESTED_ARRAY;
        break;
        case FormulaError::NoValue:
            nErrNumber = STR_LONG_ERR_NO_VALUE;
        break;
        case FormulaError::NotAvailable:
            nErrNumber = STR_LONG_ERR_NV;
        break;
        default:
            nErrNumber = STR_ERROR_STR;
        break;
    }
    OUString aRes( GetRscString( nErrNumber ) );
    return aRes;
}

SvxBrushItem* ScGlobal::GetButtonBrushItem()
{
    pButtonBrushItem->SetColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    return pButtonBrushItem;
}

const OUString& ScGlobal::GetEmptyOUString()
{
    return *pEmptyOUString;
}

void ScGlobal::Init()
{
    pEmptyOUString = new OUString;

    // The default language for number formats (ScGlobal::eLnge) must
    // always be LANGUAGE_SYSTEM
    // FIXME: So remove this variable?
    eLnge = LANGUAGE_SYSTEM;

    // FIXME: If the sort-order etc. should depend the installed Office version
    //        use Application::GetSettings().GetUILanguage() here
    pSysLocale = new SvtSysLocale;
    pCharClass = pSysLocale->GetCharClassPtr();
    pLocaleData = pSysLocale->GetLocaleDataPtr();

    ppRscString = new OUString *[ SC_GLOBSTR_STR_COUNT ];
    for( sal_uInt16 nC = 0 ; nC < SC_GLOBSTR_STR_COUNT ; nC++ ) ppRscString[ nC ] = nullptr;

    pEmptyBrushItem = new SvxBrushItem( Color( COL_TRANSPARENT ), ATTR_BACKGROUND );
    pButtonBrushItem = new SvxBrushItem( Color(), ATTR_BACKGROUND );
    pEmbeddedBrushItem = new SvxBrushItem( Color( COL_LIGHTCYAN ), ATTR_BACKGROUND );
    pProtectedBrushItem = new SvxBrushItem( Color( COL_LIGHTGRAY ), ATTR_BACKGROUND );

    InitPPT();
    //ScCompiler::InitSymbolsNative();
    // ScParameterClassification _after_ Compiler, needs function resources if
    // arguments are to be merged in, which in turn need strings of function
    // names from the compiler.
    ScParameterClassification::Init();

    InitAddIns();

    pStrClipDocName = new OUString( ScResId( SCSTR_NONAME ) );
    *pStrClipDocName += "1";

    //  ScDocumentPool::InitVersionMaps() has been called earlier already
}

void ScGlobal::InitPPT()
{
    OutputDevice* pDev = Application::GetDefaultDevice();
    Point aPix1000 = pDev->LogicToPixel( Point(100000,100000), MapUnit::MapTwip );
    nScreenPPTX = aPix1000.X() / 100000.0;
    nScreenPPTY = aPix1000.Y() / 100000.0;
}

const OUString& ScGlobal::GetClipDocName()
{
    return *pStrClipDocName;
}

void ScGlobal::SetClipDocName( const OUString& rNew )
{
    *pStrClipDocName = rNew;
}

void ScGlobal::InitTextHeight(SfxItemPool* pPool)
{
    if (!pPool)
    {
        OSL_FAIL("ScGlobal::InitTextHeight: No Pool");
        return;
    }

    const ScPatternAttr* pPattern = static_cast<const ScPatternAttr*>(&pPool->GetDefaultItem(ATTR_PATTERN));
    if (!pPattern)
    {
        OSL_FAIL("ScGlobal::InitTextHeight: No default pattern");
        return;
    }

    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    ScopedVclPtrInstance< VirtualDevice > pVirtWindow( *pDefaultDev );
    pVirtWindow->SetMapMode(MapUnit::MapPixel);
    vcl::Font aDefFont;
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pVirtWindow); // Font color doesn't matter here
    pVirtWindow->SetFont(aDefFont);
    sal_uInt16 nTest = static_cast<sal_uInt16>(
        pVirtWindow->PixelToLogic(Size(0, pVirtWindow->GetTextHeight()), MapUnit::MapTwip).Height());

    if (nTest > nDefFontHeight)
        nDefFontHeight = nTest;

    const SvxMarginItem* pMargin = static_cast<const SvxMarginItem*>(&pPattern->GetItem(ATTR_MARGIN));

    nTest = static_cast<sal_uInt16>(
        nDefFontHeight + pMargin->GetTopMargin() + pMargin->GetBottomMargin() - STD_ROWHEIGHT_DIFF);

    if (nTest > nStdRowHeight)
        nStdRowHeight = nTest;
}

void ScGlobal::Clear()
{
    // Destroy asyncs _before_ ExitExternalFunc!
    for( ScAddInAsyncs::iterator it = theAddInAsyncTbl.begin(); it != theAddInAsyncTbl.end(); ++it )
    {
        delete *it;
    }
    theAddInAsyncTbl.clear();
    ExitExternalFunc();
    DELETEZ(pAutoFormat);
    DELETEZ(pSearchItem);
    DELETEZ(pLegacyFuncCollection);
    DELETEZ(pAddInCollection);
    DELETEZ(pUserList);

    for( sal_uInt16 nC = 0 ; nC < SC_GLOBSTR_STR_COUNT ; nC++ )
        if( ppRscString ) delete ppRscString[ nC ];
    delete[] ppRscString;
    ppRscString = nullptr;

    DELETEZ(pStarCalcFunctionList); // Destroy before ResMgr!
    DELETEZ(pStarCalcFunctionMgr);
    ScParameterClassification::Exit();
    ScCompiler::DeInit();
    ScInterpreter::GlobalExit(); // Delete static Stack

    DELETEZ(pEmptyBrushItem);
    DELETEZ(pButtonBrushItem);
    DELETEZ(pEmbeddedBrushItem);
    DELETEZ(pProtectedBrushItem);
    DELETEZ(pEnglishFormatter);
    DELETEZ(pCaseTransliteration);
    DELETEZ(pTransliteration);
    DELETEZ(pCaseCollator);
    DELETEZ(pCollator);
    DELETEZ(pCalendar);
    // Do NOT delete pCharClass since it is a pointer to the single SvtSysLocale instance !
    pCharClass = nullptr;
    // Do NOT delete pLocaleData since it is a pointer to the single SvtSysLocale instance !
    pLocaleData = nullptr;
    DELETEZ(pSysLocale);
    DELETEZ(pLocale);
    DELETEZ(pScIntlWrapper);
    DELETEZ(pStrClipDocName);

    DELETEZ(pUnitConverter);
    DELETEZ(pFieldEditEngine);

    ScDocumentPool::DeleteVersionMaps();

    DELETEZ(pEmptyOUString);
    xDrawClipDocShellRef.clear();
}

rtl_TextEncoding ScGlobal::GetCharsetValue( const OUString& rCharSet )
{
    // new TextEncoding values
    if ( CharClass::isAsciiNumeric( rCharSet ) )
    {
        sal_Int32 nVal = rCharSet.toInt32();
        if ( nVal == RTL_TEXTENCODING_DONTKNOW )
            return osl_getThreadTextEncoding();
        return (rtl_TextEncoding) nVal;
    }
    // old CharSet values for compatibility
    else if (rCharSet.equalsIgnoreAsciiCase("ANSI")     ) return RTL_TEXTENCODING_MS_1252;
    else if (rCharSet.equalsIgnoreAsciiCase("MAC")      ) return RTL_TEXTENCODING_APPLE_ROMAN;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC")    ) return RTL_TEXTENCODING_IBM_850;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_437")) return RTL_TEXTENCODING_IBM_437;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_850")) return RTL_TEXTENCODING_IBM_850;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_860")) return RTL_TEXTENCODING_IBM_860;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_861")) return RTL_TEXTENCODING_IBM_861;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_863")) return RTL_TEXTENCODING_IBM_863;
    else if (rCharSet.equalsIgnoreAsciiCase("IBMPC_865")) return RTL_TEXTENCODING_IBM_865;
    // Some wrong "help" on the net mentions UTF8 and even unoconv uses it,
    // which worked accidentally if the system encoding is UTF-8 anyway, so
    // support it ;) but only when reading.
    else if (rCharSet.equalsIgnoreAsciiCase("UTF8"))      return RTL_TEXTENCODING_UTF8;
    else if (rCharSet.equalsIgnoreAsciiCase("UTF-8"))     return RTL_TEXTENCODING_UTF8;
    else return osl_getThreadTextEncoding();
}

OUString ScGlobal::GetCharsetString( rtl_TextEncoding eVal )
{
    const sal_Char* pChar;
    switch ( eVal )
    {
        // old CharSet strings for compatibility
        case RTL_TEXTENCODING_MS_1252:      pChar = "ANSI";         break;
        case RTL_TEXTENCODING_APPLE_ROMAN:  pChar = "MAC";          break;
        // IBMPC == IBMPC_850
        case RTL_TEXTENCODING_IBM_437:      pChar = "IBMPC_437";    break;
        case RTL_TEXTENCODING_IBM_850:      pChar = "IBMPC_850";    break;
        case RTL_TEXTENCODING_IBM_860:      pChar = "IBMPC_860";    break;
        case RTL_TEXTENCODING_IBM_861:      pChar = "IBMPC_861";    break;
        case RTL_TEXTENCODING_IBM_863:      pChar = "IBMPC_863";    break;
        case RTL_TEXTENCODING_IBM_865:      pChar = "IBMPC_865";    break;
        case RTL_TEXTENCODING_DONTKNOW:     pChar = "SYSTEM";       break;
        // new string of TextEncoding value
        default:
            return OUString::number( eVal );
    }
    return OUString::createFromAscii(pChar);
}

bool ScGlobal::HasStarCalcFunctionList()
{
    return ( pStarCalcFunctionList != nullptr );
}

ScFunctionList* ScGlobal::GetStarCalcFunctionList()
{
    if ( !pStarCalcFunctionList )
        pStarCalcFunctionList = new ScFunctionList;

    return pStarCalcFunctionList;
}

ScFunctionMgr* ScGlobal::GetStarCalcFunctionMgr()
{
    if ( !pStarCalcFunctionMgr )
        pStarCalcFunctionMgr = new ScFunctionMgr;

    return pStarCalcFunctionMgr;
}

void ScGlobal::ResetFunctionList()
{
    // FunctionMgr has pointers into FunctionList, must also be updated
    DELETEZ( pStarCalcFunctionMgr );
    DELETEZ( pStarCalcFunctionList );
}

ScUnitConverter* ScGlobal::GetUnitConverter()
{
    if ( !pUnitConverter )
        pUnitConverter = new ScUnitConverter;

    return pUnitConverter;
}

const sal_Unicode* ScGlobal::UnicodeStrChr( const sal_Unicode* pStr,
            sal_Unicode c )
{
    if ( !pStr )
        return nullptr;
    while ( *pStr )
    {
        if ( *pStr == c )
            return pStr;
        pStr++;
    }
    return nullptr;
}

OUString ScGlobal::addToken(const OUString& rTokenList, const OUString& rToken,
    sal_Unicode cSep, sal_Int32 nSepCount, bool bForceSep)
{
    OUStringBuffer aBuf(rTokenList);
    if( bForceSep || (!rToken.isEmpty() && !rTokenList.isEmpty()) )
        comphelper::string::padToLength(aBuf, aBuf.getLength() + nSepCount, cSep);
    aBuf.append(rToken);
    return aBuf.makeStringAndClear();
}

bool ScGlobal::IsQuoted( const OUString& rString, sal_Unicode cQuote )
{
    return (rString.getLength() >= 2) && (rString[0] == cQuote) && (rString[ rString.getLength() - 1 ] == cQuote);
}

void ScGlobal::AddQuotes( OUString& rString, sal_Unicode cQuote, bool bEscapeEmbedded )
{
    if (bEscapeEmbedded)
    {
        sal_Unicode pQ[3];
        pQ[0] = pQ[1] = cQuote;
        pQ[2] = 0;
        OUString aQuotes( pQ );
        rString = rString.replaceAll( OUStringLiteral1(cQuote), aQuotes);
    }
    rString = OUStringLiteral1( cQuote ) + rString + OUStringLiteral1( cQuote );
}

void ScGlobal::EraseQuotes( OUString& rString, sal_Unicode cQuote, bool bUnescapeEmbedded )
{
    if ( IsQuoted( rString, cQuote ) )
    {
        rString = rString.copy( 1, rString.getLength() - 2 );
        if (bUnescapeEmbedded)
        {
            sal_Unicode pQ[3];
            pQ[0] = pQ[1] = cQuote;
            pQ[2] = 0;
            OUString aQuotes( pQ );
            rString = rString.replaceAll( aQuotes, OUStringLiteral1(cQuote));
        }
    }
}

sal_Int32 ScGlobal::FindUnquoted( const OUString& rString, sal_Unicode cChar)
{
    const sal_Unicode cQuote = '\'';
    const sal_Unicode* const pStart = rString.getStr();
    const sal_Unicode* const pStop = pStart + rString.getLength();
    const sal_Unicode* p = pStart;
    bool bQuoted = false;
    while (p < pStop)
    {
        if (*p == cChar && !bQuoted)
            return sal::static_int_cast< sal_Int32 >( p - pStart );
        else if (*p == cQuote)
        {
            if (!bQuoted)
                bQuoted = true;
            else if (p < pStop-1 && *(p+1) == cQuote)
                ++p;
            else
                bQuoted = false;
        }
        ++p;
    }
    return -1;
}

const sal_Unicode* ScGlobal::FindUnquoted( const sal_Unicode* pString, sal_Unicode cChar )
{
    sal_Unicode cQuote = '\'';
    const sal_Unicode* p = pString;
    bool bQuoted = false;
    while (*p)
    {
        if (*p == cChar && !bQuoted)
            return p;
        else if (*p == cQuote)
        {
            if (!bQuoted)
                bQuoted = true;
            else if (*(p+1) == cQuote)
                ++p;
            else
                bQuoted = false;
        }
        ++p;
    }
    return nullptr;
}

bool ScGlobal::EETextObjEqual( const EditTextObject* pObj1,
                               const EditTextObject* pObj2 )
{
    if ( pObj1 == pObj2 ) // Both empty or the same object
        return true;

    if ( pObj1 && pObj2 )
    {
        // First test for equal text content
        sal_Int32 nParCount = pObj1->GetParagraphCount();
        if ( nParCount != pObj2->GetParagraphCount() )
            return false;
        for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
            if ( pObj1->GetText(nPar) != pObj2->GetText(nPar) )
                return false;

        SvMemoryStream  aStream1;
        SvMemoryStream  aStream2;
        pObj1->Store( aStream1 );
        pObj2->Store( aStream2 );
        const sal_uInt64 nSize = aStream1.Tell();
        if ( aStream2.Tell() == nSize )
            if ( !memcmp( aStream1.GetData(), aStream2.GetData(), (sal_uInt16) nSize ) )
                return true;
    }

    return false;
}

void ScGlobal::OpenURL(const OUString& rURL, const OUString& rTarget)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if(SfxViewShell* pViewShell = SfxViewShell::Current())
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED, rURL.toUtf8().getStr());
    }

    // OpenURL is always called in the GridWindow by mouse clicks in some way or another.
    // That's why pScActiveViewShell and nScClickMouseModifier are correct.
    // SvtSecurityOptions to access Libreoffice global security parameters
    SvtSecurityOptions aSecOpt;
    bool bCtrlClickHappened = (nScClickMouseModifier & KEY_MOD1);
    bool bCtrlClickSecOption = aSecOpt.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
    if( bCtrlClickHappened && !( bCtrlClickSecOption ) )
    {
        // return since ctrl+click happened when the
        // ctrl+click security option was disabled, link should not open
        return;
    }
    else if( !( bCtrlClickHappened ) && bCtrlClickSecOption )
    {
        // ctrl+click did not happen; only click happened maybe with some
        // other key combo. and security option is set, so return
        return;
    }
    SfxStringItem aUrl( SID_FILE_NAME, rURL );
    SfxStringItem aTarget( SID_TARGETNAME, rTarget );
    if ( nScClickMouseModifier & KEY_SHIFT )     // control-click -> into new window
        aTarget.SetValue("_blank");
    SfxViewFrame* pFrame = nullptr;
    OUString aReferName;
    if ( pScActiveViewShell )
    {
        pFrame = pScActiveViewShell->GetViewFrame();
        SfxMedium* pMed = pFrame->GetObjectShell()->GetMedium();
        if (pMed)
            aReferName = pMed->GetName();
    }

    SfxFrameItem aFrm( SID_DOCFRAME, pFrame );
    SfxStringItem aReferer( SID_REFERER, aReferName );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
    SfxBoolItem aBrowsing( SID_BROWSE, true );

    // No SID_SILENT anymore
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
    {
        pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                { &aUrl, &aTarget, &aFrm, &aReferer, &aNewView, &aBrowsing });
    }
}

bool ScGlobal::IsSystemRTL()
{
    return MsLangId::isRightToLeft( Application::GetSettings().GetLanguageTag().getLanguageType() );
}

SvtScriptType ScGlobal::GetDefaultScriptType()
{
    // Used when text contains only WEAK characters.
    // Script type of office language is used then (same as GetEditDefaultLanguage,
    // to get consistent behavior of text in simple cells and EditEngine,
    // also same as GetAppLanguage() in Writer)
    return SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
}

LanguageType ScGlobal::GetEditDefaultLanguage()
{
    // Used for EditEngine::SetDefaultLanguage
    return Application::GetSettings().GetLanguageTag().getLanguageType();
}

sal_uInt16 ScGlobal::GetScriptedWhichID( SvtScriptType nScriptType, sal_uInt16 nWhich )
{
    switch ( nScriptType )
    {
        case SvtScriptType::LATIN:
        case SvtScriptType::ASIAN:
        case SvtScriptType::COMPLEX:
        break;      // take exact matches
        default:    // prefer one, first COMPLEX, then ASIAN
            if ( nScriptType & SvtScriptType::COMPLEX )
                nScriptType = SvtScriptType::COMPLEX;
            else if ( nScriptType & SvtScriptType::ASIAN )
                nScriptType = SvtScriptType::ASIAN;
    }
    switch ( nScriptType )
    {
        case SvtScriptType::COMPLEX:
        {
            switch ( nWhich )
            {
                case ATTR_FONT:
                case ATTR_CJK_FONT:
                    nWhich = ATTR_CTL_FONT;
                break;
                case ATTR_FONT_HEIGHT:
                case ATTR_CJK_FONT_HEIGHT:
                    nWhich = ATTR_CTL_FONT_HEIGHT;
                break;
                case ATTR_FONT_WEIGHT:
                case ATTR_CJK_FONT_WEIGHT:
                    nWhich = ATTR_CTL_FONT_WEIGHT;
                break;
                case ATTR_FONT_POSTURE:
                case ATTR_CJK_FONT_POSTURE:
                    nWhich = ATTR_CTL_FONT_POSTURE;
                break;
            }
        }
        break;
        case SvtScriptType::ASIAN:
        {
            switch ( nWhich )
            {
                case ATTR_FONT:
                case ATTR_CTL_FONT:
                    nWhich = ATTR_CJK_FONT;
                break;
                case ATTR_FONT_HEIGHT:
                case ATTR_CTL_FONT_HEIGHT:
                    nWhich = ATTR_CJK_FONT_HEIGHT;
                break;
                case ATTR_FONT_WEIGHT:
                case ATTR_CTL_FONT_WEIGHT:
                    nWhich = ATTR_CJK_FONT_WEIGHT;
                break;
                case ATTR_FONT_POSTURE:
                case ATTR_CTL_FONT_POSTURE:
                    nWhich = ATTR_CJK_FONT_POSTURE;
                break;
            }
        }
        break;
        default:
        {
            switch ( nWhich )
            {
                case ATTR_CTL_FONT:
                case ATTR_CJK_FONT:
                    nWhich = ATTR_FONT;
                break;
                case ATTR_CTL_FONT_HEIGHT:
                case ATTR_CJK_FONT_HEIGHT:
                    nWhich = ATTR_FONT_HEIGHT;
                break;
                case ATTR_CTL_FONT_WEIGHT:
                case ATTR_CJK_FONT_WEIGHT:
                    nWhich = ATTR_FONT_WEIGHT;
                break;
                case ATTR_CTL_FONT_POSTURE:
                case ATTR_CJK_FONT_POSTURE:
                    nWhich = ATTR_FONT_POSTURE;
                break;
            }
        }
    }
    return nWhich;
}

void ScGlobal::AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter )
{
    OSL_ENSURE( rSet.GetItemState( ATTR_LANGUAGE_FORMAT, false ) == SfxItemState::DEFAULT,
        "ScGlobal::AddLanguage - language already added");

    const SfxPoolItem* pHardItem;
    if ( rSet.GetItemState( ATTR_VALUE_FORMAT, false, &pHardItem ) == SfxItemState::SET )
    {
        const SvNumberformat* pHardFormat = rFormatter.GetEntry(
            static_cast<const SfxUInt32Item*>(pHardItem)->GetValue() );

        sal_uInt32 nParentFmt = 0; // Pool default
        const SfxItemSet* pParent = rSet.GetParent();
        if ( pParent )
            nParentFmt = static_cast<const SfxUInt32Item&>(pParent->Get( ATTR_VALUE_FORMAT )).GetValue();
        const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );

        if ( pHardFormat && pParFormat &&
                (pHardFormat->GetLanguage() != pParFormat->GetLanguage()) )
            rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
    }
}

utl::TransliterationWrapper* ScGlobal::GetpTransliteration()
{
    if ( !pTransliteration )
    {
        const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
        pTransliteration = new ::utl::TransliterationWrapper(
            ::comphelper::getProcessComponentContext(), SC_TRANSLITERATION_IGNORECASE );
        pTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    }
    OSL_ENSURE(
        pTransliteration,
        "ScGlobal::GetpTransliteration() called before ScGlobal::Init()");
    return pTransliteration;
}

const LocaleDataWrapper* ScGlobal::GetpLocaleData()
{
    OSL_ENSURE(
        pLocaleData,
        "ScGlobal::GetpLocaleData() called before ScGlobal::Init()");
    return pLocaleData;
}
CalendarWrapper*     ScGlobal::GetCalendar()
{
    if ( !pCalendar )
    {
        pCalendar = new CalendarWrapper( ::comphelper::getProcessComponentContext() );
        pCalendar->loadDefaultCalendar( *GetLocale() );
    }
    return pCalendar;
}
CollatorWrapper*        ScGlobal::GetCollator()
{
    if ( !pCollator )
    {
        pCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
        pCollator->loadDefaultCollator( *GetLocale(), SC_COLLATOR_IGNORES );
    }
    return pCollator;
}
CollatorWrapper*        ScGlobal::GetCaseCollator()
{
    if ( !pCaseCollator )
    {
        pCaseCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
        pCaseCollator->loadDefaultCollator( *GetLocale(), 0 );
    }
    return pCaseCollator;
}
::utl::TransliterationWrapper* ScGlobal::GetCaseTransliteration()
{
    if ( !pCaseTransliteration )
    {
        const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
        pCaseTransliteration = new ::utl::TransliterationWrapper(::comphelper::getProcessComponentContext(), SC_TRANSLITERATION_CASESENSE );
        pCaseTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    }
    return pCaseTransliteration;
}
IntlWrapper*         ScGlobal::GetScIntlWrapper()
{
    if ( !pScIntlWrapper )
    {
        pScIntlWrapper = new IntlWrapper( LanguageTag( *GetLocale()) );
    }
    return pScIntlWrapper;
}
css::lang::Locale*     ScGlobal::GetLocale()
{
    if ( !pLocale )
    {
        pLocale = new css::lang::Locale( Application::GetSettings().GetLanguageTag().getLocale());
    }
    return pLocale;
}

ScFieldEditEngine& ScGlobal::GetStaticFieldEditEngine()
{
    if (!pFieldEditEngine)
    {
        // Creating a ScFieldEditEngine with pDocument=NULL leads to document
        // specific fields not being resolvable! See
        // ScFieldEditEngine::CalcFieldValue(). pEnginePool=NULL lets
        // EditEngine internally create and delete a default pool.
        pFieldEditEngine = new ScFieldEditEngine( nullptr, nullptr);
    }
    return *pFieldEditEngine;
}

OUString ScGlobal::ReplaceOrAppend( const OUString& rString,
        const OUString& rPlaceholder, const OUString& rReplacement )
{
    if (rString.isEmpty())
        return rReplacement;
    sal_Int32 nFound = rString.indexOf( rPlaceholder);
    if (nFound < 0)
    {
        if (rString[rString.getLength()-1] == ' ')
            return rString + rReplacement;
        return rString + " " + rReplacement;
    }
    return rString.replaceFirst( rPlaceholder, rReplacement, &nFound);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
