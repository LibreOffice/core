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
#include <scitems.hxx>
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
#include <unotools/charclass.hxx>
#include <unotools/securityoptions.hxx>
#include <stdlib.h>
#include <time.h>
#include <numeric>
#include <svx/svdmodel.hxx>
#include <svtools/colorcfg.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/doublecheckedinit.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include <global.hxx>
#include <scresid.hxx>
#include <autoform.hxx>
#include <document.hxx>
#include <patattr.hxx>
#include <addincol.hxx>
#include <adiasync.hxx>
#include <userlist.hxx>
#include <interpre.hxx>
#include <docpool.hxx>
#include <unitconv.hxx>
#include <compiler.hxx>
#include <parclass.hxx>
#include <funcdesc.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <scmod.hxx>
#include <appoptio.hxx>
#include <editutil.hxx>
#include <docsh.hxx>

tools::SvRef<ScDocShell>  ScGlobal::xDrawClipDocShellRef;
SvxSearchItem*  ScGlobal::pSearchItem = nullptr;
ScAutoFormat*   ScGlobal::pAutoFormat = nullptr;
std::atomic<LegacyFuncCollection*> ScGlobal::pLegacyFuncCollection(nullptr);
std::atomic<ScUnoAddInCollection*> ScGlobal::pAddInCollection(nullptr);
ScUserList*     ScGlobal::pUserList = nullptr;
LanguageType    ScGlobal::eLnge = LANGUAGE_SYSTEM;
std::atomic<css::lang::Locale*> ScGlobal::pLocale(nullptr);
SvtSysLocale*   ScGlobal::pSysLocale = nullptr;
const CharClass*  ScGlobal::pCharClass = nullptr;
const LocaleDataWrapper*  ScGlobal::pLocaleData = nullptr;
CalendarWrapper* ScGlobal::pCalendar = nullptr;
std::atomic<CollatorWrapper*> ScGlobal::pCollator(nullptr);
std::atomic<CollatorWrapper*> ScGlobal::pCaseCollator(nullptr);
std::atomic<::utl::TransliterationWrapper*> ScGlobal::pTransliteration(nullptr);
std::atomic<::utl::TransliterationWrapper*> ScGlobal::pCaseTransliteration(nullptr);
css::uno::Reference< css::i18n::XOrdinalSuffix> ScGlobal::xOrdinalSuffix = nullptr;
sal_Unicode     ScGlobal::cListDelimiter = ',';
OUString*       ScGlobal::pEmptyOUString = nullptr;
OUString*       ScGlobal::pStrClipDocName = nullptr;

SvxBrushItem*   ScGlobal::pEmptyBrushItem = nullptr;
SvxBrushItem*   ScGlobal::pButtonBrushItem = nullptr;
SvxBrushItem*   ScGlobal::pEmbeddedBrushItem = nullptr;

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

bool ScGlobal::bThreadedGroupCalcInProgress = false;

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
        sal_uInt32 nFormat, SvNumFormatType nType )
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
    assert(!bThreadedGroupCalcInProgress);
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
    assert(!bThreadedGroupCalcInProgress);
    if (!pSearchItem)
    {
        pSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );
        pSearchItem->SetAppFlag( SvxSearchApp::CALC );
    }
    return *pSearchItem;
}

void ScGlobal::SetSearchItem( const SvxSearchItem& rNew )
{
    assert(!bThreadedGroupCalcInProgress);
    // FIXME: An assignment operator would be nice here
    delete pSearchItem;
    pSearchItem = static_cast<SvxSearchItem*>(rNew.Clone());

    pSearchItem->SetWhich( SID_SEARCH_ITEM );
    pSearchItem->SetAppFlag( SvxSearchApp::CALC );
}

void ScGlobal::ClearAutoFormat()
{
    assert(!bThreadedGroupCalcInProgress);
    if (pAutoFormat)
    {
        //  When modified via StarOne then only the SaveLater flag is set and no saving is done.
        //  If the flag is set then save now.
        if (pAutoFormat->IsSaveLater())
            pAutoFormat->Save();
        delete pAutoFormat;
        pAutoFormat = nullptr;
    }
}

ScAutoFormat* ScGlobal::GetAutoFormat()
{
    return pAutoFormat;
}

ScAutoFormat* ScGlobal::GetOrCreateAutoFormat()
{
    assert(!bThreadedGroupCalcInProgress);
    if ( !pAutoFormat )
    {
        pAutoFormat = new ScAutoFormat;
        pAutoFormat->Load();
    }

    return pAutoFormat;
}

LegacyFuncCollection* ScGlobal::GetLegacyFuncCollection()
{
    return comphelper::doubleCheckedInit( pLegacyFuncCollection, []() { return new LegacyFuncCollection(); });
}

ScUnoAddInCollection* ScGlobal::GetAddInCollection()
{
    return comphelper::doubleCheckedInit( pAddInCollection, []() { return new ScUnoAddInCollection(); });
}

ScUserList* ScGlobal::GetUserList()
{
    assert(!bThreadedGroupCalcInProgress);
    // Hack: Load Cfg item at the App
    global_InitAppOptions();

    if (!pUserList)
        pUserList = new ScUserList();
    return pUserList;
}

void ScGlobal::SetUserList( const ScUserList* pNewList )
{
    assert(!bThreadedGroupCalcInProgress);
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

OUString ScGlobal::GetErrorString(FormulaError nErr)
{
    const char* pErrNumber;
    switch (nErr)
    {
        case FormulaError::NoRef:
            pErrNumber = STR_NO_REF_TABLE;
            break;
        case FormulaError::NoAddin:
            pErrNumber = STR_NO_ADDIN;
            break;
        case FormulaError::NoMacro:
            pErrNumber = STR_NO_MACRO;
            break;
        case FormulaError::NotAvailable:
            return ScCompiler::GetNativeSymbol(ocErrNA);
        case FormulaError::NoName:
            return ScCompiler::GetNativeSymbol(ocErrName);
        case FormulaError::NoValue:
            return ScCompiler::GetNativeSymbol(ocErrValue);
        case FormulaError::NoCode:
            return ScCompiler::GetNativeSymbol(ocErrNull);
        case FormulaError::DivisionByZero:
            return ScCompiler::GetNativeSymbol(ocErrDivZero);
        case FormulaError::IllegalFPOperation:
            return ScCompiler::GetNativeSymbol(ocErrNum);
        default:
            return ScResId(STR_ERROR_STR) + OUString::number( static_cast<int>(nErr) );
    }
    return ScResId(pErrNumber);
}

OUString ScGlobal::GetLongErrorString(FormulaError nErr)
{
    const char* pErrNumber;
    switch (nErr)
    {
        case FormulaError::NONE:
            return OUString();
        case FormulaError::IllegalArgument:
            pErrNumber = STR_LONG_ERR_ILL_ARG;
        break;
        case FormulaError::IllegalFPOperation:
            pErrNumber = STR_LONG_ERR_ILL_FPO;
        break;
        case FormulaError::IllegalChar:
            pErrNumber = STR_LONG_ERR_ILL_CHAR;
        break;
        case FormulaError::IllegalParameter:
            pErrNumber = STR_LONG_ERR_ILL_PAR;
        break;
        case FormulaError::Pair:
        case FormulaError::PairExpected:
            pErrNumber = STR_LONG_ERR_PAIR;
        break;
        case FormulaError::OperatorExpected:
            pErrNumber = STR_LONG_ERR_OP_EXP;
        break;
        case FormulaError::VariableExpected:
        case FormulaError::ParameterExpected:
            pErrNumber = STR_LONG_ERR_VAR_EXP;
        break;
        case FormulaError::CodeOverflow:
            pErrNumber = STR_LONG_ERR_CODE_OVF;
        break;
        case FormulaError::StringOverflow:
            pErrNumber = STR_LONG_ERR_STR_OVF;
        break;
        case FormulaError::StackOverflow:
            pErrNumber = STR_LONG_ERR_STACK_OVF;
        break;
        case FormulaError::MatrixSize:
            pErrNumber = STR_LONG_ERR_MATRIX_SIZE;
        break;
        case FormulaError::UnknownState:
        case FormulaError::UnknownVariable:
        case FormulaError::UnknownOpCode:
        case FormulaError::UnknownStackVariable:
        case FormulaError::UnknownToken:
        case FormulaError::NoCode:
            pErrNumber = STR_LONG_ERR_SYNTAX;
        break;
        case FormulaError::CircularReference:
            pErrNumber = STR_LONG_ERR_CIRC_REF;
        break;
        case FormulaError::NoConvergence:
            pErrNumber = STR_LONG_ERR_NO_CONV;
        break;
        case FormulaError::NoRef:
            pErrNumber = STR_LONG_ERR_NO_REF;
        break;
        case FormulaError::NoName:
            pErrNumber = STR_LONG_ERR_NO_NAME;
        break;
        case FormulaError::NoAddin:
            pErrNumber = STR_LONG_ERR_NO_ADDIN;
        break;
        case FormulaError::NoMacro:
            pErrNumber = STR_LONG_ERR_NO_MACRO;
        break;
        case FormulaError::DivisionByZero:
            pErrNumber = STR_LONG_ERR_DIV_ZERO;
        break;
        case FormulaError::NestedArray:
            pErrNumber = STR_ERR_LONG_NESTED_ARRAY;
        break;
        case FormulaError::BadArrayContent:
            pErrNumber = STR_ERR_LONG_BAD_ARRAY_CONTENT;
        break;
        case FormulaError::LinkFormulaNeedingCheck:
            pErrNumber = STR_ERR_LONG_LINK_FORMULA_NEEDING_CHECK;
        break;
        case FormulaError::NoValue:
            pErrNumber = STR_LONG_ERR_NO_VALUE;
        break;
        case FormulaError::NotAvailable:
            pErrNumber = STR_LONG_ERR_NV;
        break;
        default:
            return ScResId(STR_ERROR_STR) + OUString::number( static_cast<int>(nErr) );
        break;
    }
    return ScResId(pErrNumber);
}

SvxBrushItem* ScGlobal::GetButtonBrushItem()
{
    assert(!bThreadedGroupCalcInProgress);
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

    pEmptyBrushItem = new SvxBrushItem( COL_TRANSPARENT, ATTR_BACKGROUND );
    pButtonBrushItem = new SvxBrushItem( Color(), ATTR_BACKGROUND );
    pEmbeddedBrushItem = new SvxBrushItem( COL_LIGHTCYAN, ATTR_BACKGROUND );

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

    nScreenPPTX = double(pDev->GetDPIX()) / double(TWIPS_PER_INCH);
    nScreenPPTY = double(pDev->GetDPIY()) / double(TWIPS_PER_INCH);
}

const OUString& ScGlobal::GetClipDocName()
{
    return *pStrClipDocName;
}

void ScGlobal::SetClipDocName( const OUString& rNew )
{
    assert(!bThreadedGroupCalcInProgress);
    *pStrClipDocName = rNew;
}

void ScGlobal::InitTextHeight(const SfxItemPool* pPool)
{
    if (!pPool)
    {
        OSL_FAIL("ScGlobal::InitTextHeight: No Pool");
        return;
    }

    const ScPatternAttr* pPattern = &pPool->GetDefaultItem(ATTR_PATTERN);
    if (!pPattern)
    {
        OSL_FAIL("ScGlobal::InitTextHeight: No default pattern");
        return;
    }

    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    ScopedVclPtrInstance< VirtualDevice > pVirtWindow( *pDefaultDev );
    pVirtWindow->SetMapMode(MapMode(MapUnit::MapPixel));
    vcl::Font aDefFont;
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pVirtWindow); // Font color doesn't matter here
    pVirtWindow->SetFont(aDefFont);
    sal_uInt16 nTest = static_cast<sal_uInt16>(
        pVirtWindow->PixelToLogic(Size(0, pVirtWindow->GetTextHeight()), MapMode(MapUnit::MapTwip)).Height());

    if (nTest > nDefFontHeight)
        nDefFontHeight = nTest;

    const SvxMarginItem* pMargin = &pPattern->GetItem(ATTR_MARGIN);

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
    ClearAutoFormat();
    DELETEZ(pSearchItem);
    delete pLegacyFuncCollection.load(); pLegacyFuncCollection = nullptr;
    delete pAddInCollection.load(); pAddInCollection = nullptr;
    DELETEZ(pUserList);
    DELETEZ(pStarCalcFunctionList); // Destroy before ResMgr!
    DELETEZ(pStarCalcFunctionMgr);
    ScParameterClassification::Exit();
    ScCompiler::DeInit();
    ScInterpreter::GlobalExit(); // Delete static Stack

    DELETEZ(pEmptyBrushItem);
    DELETEZ(pButtonBrushItem);
    DELETEZ(pEmbeddedBrushItem);
    DELETEZ(pEnglishFormatter);
    delete pCaseTransliteration.load(); pCaseTransliteration = nullptr;
    delete pTransliteration.load(); pTransliteration = nullptr;
    delete pCaseCollator.load(); pCaseCollator = nullptr;
    delete pCollator.load(); pCollator = nullptr;
    DELETEZ(pCalendar);
    // Do NOT delete pCharClass since it is a pointer to the single SvtSysLocale instance !
    pCharClass = nullptr;
    // Do NOT delete pLocaleData since it is a pointer to the single SvtSysLocale instance !
    pLocaleData = nullptr;
    DELETEZ(pSysLocale);
    delete pLocale.load(); pLocale = nullptr;
    DELETEZ(pStrClipDocName);

    DELETEZ(pUnitConverter);
    DELETEZ(pFieldEditEngine);

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
        return static_cast<rtl_TextEncoding>(nVal);
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
    assert(!bThreadedGroupCalcInProgress);
    if ( !pStarCalcFunctionList )
        pStarCalcFunctionList = new ScFunctionList;

    return pStarCalcFunctionList;
}

ScFunctionMgr* ScGlobal::GetStarCalcFunctionMgr()
{
    assert(!bThreadedGroupCalcInProgress);
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
    assert(!bThreadedGroupCalcInProgress);
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
        return pObj1->Equals( *pObj2);

    return false;
}

void ScGlobal::OpenURL(const OUString& rURL, const OUString& rTarget)
{
    // OpenURL is always called in the GridWindow by mouse clicks in some way or another.
    // That's why pScActiveViewShell and nScClickMouseModifier are correct.
    // SvtSecurityOptions to access Libreoffice global security parameters
    SvtSecurityOptions aSecOpt;
    bool bCtrlClickHappened = (nScClickMouseModifier & KEY_MOD1);
    bool bCtrlClickSecOption = aSecOpt.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );
    if( bCtrlClickHappened && ! bCtrlClickSecOption )
    {
        // return since ctrl+click happened when the
        // ctrl+click security option was disabled, link should not open
        return;
    }
    else if( ! bCtrlClickHappened && bCtrlClickSecOption )
    {
        // ctrl+click did not happen; only click happened maybe with some
        // other key combo. and security option is set, so return
        return;
    }

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    OUString aUrlName( rURL );
    SfxViewFrame* pFrame = nullptr;
    const SfxObjectShell* pObjShell = nullptr;
    OUString aReferName;
    if ( pScActiveViewShell )
    {
        pFrame = pScActiveViewShell->GetViewFrame();
        pObjShell = pFrame->GetObjectShell();
        const SfxMedium* pMed = pObjShell->GetMedium();
        if (pMed)
            aReferName = pMed->GetName();
    }

    // Don't fiddle with fragments pointing into current document.
    if (!aUrlName.startsWith("#"))
    {
        // Any relative reference would fail with "not an absolute URL"
        // error, try to construct an absolute URI with the path relative
        // to the current document's path or work path, as usual for all
        // external references.
        // This then also, as ScGlobal::GetAbsDocName() uses
        // INetURLObject::smartRel2Abs(), supports "\\" UNC path names as
        // smb:// Samba shares and DOS path separators converted to proper
        // file:// URI.
        const OUString aNewUrlName( ScGlobal::GetAbsDocName( aUrlName, pObjShell));
        if (!aNewUrlName.isEmpty())
            aUrlName = aNewUrlName;
    }

    SfxStringItem aUrl( SID_FILE_NAME, aUrlName );
    SfxStringItem aTarget( SID_TARGETNAME, rTarget );
    if ( nScClickMouseModifier & KEY_SHIFT )     // control-click -> into new window
        aTarget.SetValue("_blank");

    SfxFrameItem aFrm( SID_DOCFRAME, pFrame );
    SfxStringItem aReferer( SID_REFERER, aReferName );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
    SfxBoolItem aBrowsing( SID_BROWSE, true );

    // No SID_SILENT anymore
    pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aUrl, &aTarget, &aFrm, &aReferer, &aNewView, &aBrowsing });
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

void ScGlobal::AddLanguage( SfxItemSet& rSet, const SvNumberFormatter& rFormatter )
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
            nParentFmt = pParent->Get( ATTR_VALUE_FORMAT ).GetValue();
        const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );

        if ( pHardFormat && pParFormat &&
                (pHardFormat->GetLanguage() != pParFormat->GetLanguage()) )
            rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
    }
}

utl::TransliterationWrapper* ScGlobal::GetpTransliteration()
{
    return comphelper::doubleCheckedInit( pTransliteration,
        []()
        {
            const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
            ::utl::TransliterationWrapper* p = new ::utl::TransliterationWrapper(
                ::comphelper::getProcessComponentContext(), TransliterationFlags::IGNORE_CASE );
            p->loadModuleIfNeeded( eOfficeLanguage );
            return p;
        });
}
::utl::TransliterationWrapper* ScGlobal::GetCaseTransliteration()
{
    return comphelper::doubleCheckedInit( pCaseTransliteration,
        []()
        {
            const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
            ::utl::TransliterationWrapper* p = new ::utl::TransliterationWrapper(
                ::comphelper::getProcessComponentContext(), TransliterationFlags::NONE );
            p->loadModuleIfNeeded( eOfficeLanguage );
            return p;
        });
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
    assert(!bThreadedGroupCalcInProgress);
    if ( !pCalendar )
    {
        pCalendar = new CalendarWrapper( ::comphelper::getProcessComponentContext() );
        pCalendar->loadDefaultCalendar( *GetLocale() );
    }
    return pCalendar;
}
CollatorWrapper*        ScGlobal::GetCollator()
{
    return comphelper::doubleCheckedInit( pCollator,
        []()
        {
            CollatorWrapper* p = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
            p->loadDefaultCollator( *GetLocale(), SC_COLLATOR_IGNORES );
            return p;
        });
}
CollatorWrapper*        ScGlobal::GetCaseCollator()
{
    return comphelper::doubleCheckedInit( pCaseCollator,
        []()
        {
            CollatorWrapper* p = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
            p->loadDefaultCollator( *GetLocale(), 0 );
            return p;
        });
}
css::lang::Locale*     ScGlobal::GetLocale()
{
    return comphelper::doubleCheckedInit( pLocale,
        []() { return new css::lang::Locale( Application::GetSettings().GetLanguageTag().getLocale()); });
}

ScFieldEditEngine& ScGlobal::GetStaticFieldEditEngine()
{
    assert(!bThreadedGroupCalcInProgress);
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
