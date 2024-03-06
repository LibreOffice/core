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

#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editobj.hxx>
#include <svl/itempool.hxx>
#include <svl/srchitem.hxx>
#include <editeng/langitem.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <unotools/charclass.hxx>
#include <unotools/securityoptions.hxx>
#include <osl/diagnose.h>

#include <i18nlangtag/mslangid.hxx>
#include <comphelper/doublecheckedinit.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <comphelper/lok.hxx>

#include <global.hxx>
#include <scresid.hxx>
#include <autoform.hxx>
#include <patattr.hxx>
#include <addincol.hxx>
#include <adiasync.hxx>
#include <userlist.hxx>
#include <interpre.hxx>
#include <unitconv.hxx>
#include <compiler.hxx>
#include <parclass.hxx>
#include <funcdesc.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <scmod.hxx>
#include <editutil.hxx>
#include <docsh.hxx>
#include <sharedstringpoolpurge.hxx>
#include <formulaopt.hxx>

rtl::Reference<ScDocShell> ScGlobal::xDrawClipDocShellRef;
std::unique_ptr<SvxSearchItem> ScGlobal::xSearchItem;
std::unique_ptr<ScAutoFormat> ScGlobal::xAutoFormat;
std::atomic<LegacyFuncCollection*> ScGlobal::pLegacyFuncCollection(nullptr);
std::atomic<ScUnoAddInCollection*> ScGlobal::pAddInCollection(nullptr);
std::unique_ptr<ScUserList> ScGlobal::xUserList;
LanguageType    ScGlobal::eLnge = LANGUAGE_SYSTEM;
std::atomic<css::lang::Locale*> ScGlobal::pLocale(nullptr);
std::optional<SvtSysLocale>   ScGlobal::oSysLocale;
std::optional<CalendarWrapper> ScGlobal::oCalendar;
std::atomic<CollatorWrapper*> ScGlobal::pCollator(nullptr);
std::atomic<CollatorWrapper*> ScGlobal::pCaseCollator(nullptr);
std::atomic<::utl::TransliterationWrapper*> ScGlobal::pTransliteration(nullptr);
std::atomic<::utl::TransliterationWrapper*> ScGlobal::pCaseTransliteration(nullptr);
css::uno::Reference< css::i18n::XOrdinalSuffix> ScGlobal::xOrdinalSuffix;
OUString        ScGlobal::aStrClipDocName;

std::unique_ptr<SvxBrushItem> ScGlobal::xEmptyBrushItem;
std::unique_ptr<SvxBrushItem> ScGlobal::xButtonBrushItem;

std::unique_ptr<ScFunctionList> ScGlobal::xStarCalcFunctionList;
std::unique_ptr<ScFunctionMgr> ScGlobal::xStarCalcFunctionMgr;

std::atomic<ScUnitConverter*> ScGlobal::pUnitConverter(nullptr);
std::unique_ptr<SvNumberFormatter> ScGlobal::xEnglishFormatter;
std::unique_ptr<ScFieldEditEngine> ScGlobal::xFieldEditEngine;
std::atomic<sc::SharedStringPoolPurge*> ScGlobal::pSharedStringPoolPurge;

double          ScGlobal::nScreenPPTX           = 96.0;
double          ScGlobal::nScreenPPTY           = 96.0;

sal_uInt16          ScGlobal::nDefFontHeight        = 225;
sal_uInt16          ScGlobal::nStdRowHeight         = 256;

tools::Long            ScGlobal::nLastRowHeightExtra   = 0;
tools::Long            ScGlobal::nLastColWidthExtra    = STD_EXTRA_WIDTH;

SfxViewShell* pScActiveViewShell = nullptr; //FIXME: Make this a member
sal_uInt16 nScClickMouseModifier = 0;    //FIXME: This too
sal_uInt16 nScFillModeMouseModifier = 0; //FIXME: And this

bool ScGlobal::bThreadedGroupCalcInProgress = false;

InputHandlerFunctionNames ScGlobal::maInputHandlerFunctionNames;


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
            bInvalidate = !SfxPoolItem::areSame(pNewItem, pOldItem);
    }
    else
    {
        // Contains a Default Item
        // PoolItems, meaning Item comparison necessary
        if (!pOldItem)
            pOldItem = &rOldAttrs.GetPool()->GetUserOrPoolDefaultItem( nWhich );

        if (!pNewItem)
            pNewItem = &rNewAttrs.GetPool()->GetUserOrPoolDefaultItem( nWhich );

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
    if ( !xEnglishFormatter )
    {
        xEnglishFormatter.reset( new SvNumberFormatter(
            ::comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US ) );
        xEnglishFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
    }
    return xEnglishFormatter.get();
}

bool ScGlobal::CheckWidthInvalidate( bool& bNumFormatChanged,
                                     const SfxItemSet& rNewAttrs,
                                     const SfxItemSet& rOldAttrs )
{
    // Here ScPatternAttr::FastEqualPatternSets was used before. This implies that
    // the two given SfxItemSet are internal ones from ScPatternAttr, but there is
    // no guarantee here for that. Also that former method contained the comment
    //   "Actually test_tdf133629 from UITest_calc_tests9 somehow manages to have
    //   a different range (and I don't understand enough why), so better be safe and compare fully."
    // which may be based on this usage. I check for that already in
    // ScPatternAttr::operator==, seems not to be triggered there.
    // All in all: Better use SfxItemSet::operator== here, and not one specialized
    // on the SfxItemSets of ScPatternAttr
    if (rNewAttrs == rOldAttrs)
    {
        bNumFormatChanged = false;
        return false;
    }
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
    if (!xSearchItem)
    {
        xSearchItem.reset(new SvxSearchItem( SID_SEARCH_ITEM ));
        xSearchItem->SetAppFlag( SvxSearchApp::CALC );
    }
    return *xSearchItem;
}

void ScGlobal::SetSearchItem( const SvxSearchItem& rNew )
{
    assert(!bThreadedGroupCalcInProgress);
    // FIXME: An assignment operator would be nice here
    xSearchItem.reset(rNew.Clone());

    xSearchItem->SetWhich( SID_SEARCH_ITEM );
    xSearchItem->SetAppFlag( SvxSearchApp::CALC );
}

void ScGlobal::ClearAutoFormat()
{
    assert(!bThreadedGroupCalcInProgress);
    if (xAutoFormat)
    {
        //  When modified via StarOne then only the SaveLater flag is set and no saving is done.
        //  If the flag is set then save now.
        if (xAutoFormat->IsSaveLater())
            xAutoFormat->Save();
        xAutoFormat.reset();
    }
}

ScAutoFormat* ScGlobal::GetAutoFormat()
{
    return xAutoFormat.get();
}

ScAutoFormat* ScGlobal::GetOrCreateAutoFormat()
{
    assert(!bThreadedGroupCalcInProgress);
    if ( !xAutoFormat )
    {
        xAutoFormat.reset(new ScAutoFormat);
        xAutoFormat->Load();
    }

    return xAutoFormat.get();
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

    if (!xUserList)
        xUserList.reset(new ScUserList());
    return xUserList.get();
}

void ScGlobal::SetUserList( const ScUserList* pNewList )
{
    assert(!bThreadedGroupCalcInProgress);
    if ( pNewList )
    {
        if ( !xUserList )
            xUserList.reset( new ScUserList( *pNewList ) );
        else
            *xUserList = *pNewList;
    }
    else
    {
        xUserList.reset();
    }
}

OUString ScGlobal::GetErrorString(FormulaError nErr)
{
    TranslateId pErrNumber;
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
    TranslateId pErrNumber;
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
            pErrNumber = STR_LONG_ERR_SYNTAX;
        break;
        case FormulaError::NoCode:
            pErrNumber = STR_LONG_ERR_NO_CODE;
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
    }
    return ScResId(pErrNumber);
}

SvxBrushItem* ScGlobal::GetButtonBrushItem()
{
    assert(!bThreadedGroupCalcInProgress);
    xButtonBrushItem->SetColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    return xButtonBrushItem.get();
}

void ScGlobal::Init()
{
    // The default language for number formats (ScGlobal::eLnge) must
    // always be LANGUAGE_SYSTEM
    // FIXME: So remove this variable?
    eLnge = LANGUAGE_SYSTEM;

    oSysLocale.emplace();

    xEmptyBrushItem = std::make_unique<SvxBrushItem>( COL_TRANSPARENT, ATTR_BACKGROUND );
    xButtonBrushItem = std::make_unique<SvxBrushItem>( Color(), ATTR_BACKGROUND );

    InitPPT();
    //ScCompiler::InitSymbolsNative();
    // ScParameterClassification _after_ Compiler, needs function resources if
    // arguments are to be merged in, which in turn need strings of function
    // names from the compiler.
    ScParameterClassification::Init();

    InitAddIns();

    aStrClipDocName = ScResId( SCSTR_NONAME ) + "1";

    //  ScDocumentPool::InitVersionMaps() has been called earlier already
}

void ScGlobal::InitPPT()
{
    OutputDevice* pDev = Application::GetDefaultDevice();

    if (comphelper::LibreOfficeKit::isActive())
    {
        // LOK: the below limited precision is not enough for RowColumnHeader.
        nScreenPPTX = o3tl::convert<double>(pDev->GetDPIX(), o3tl::Length::twip, o3tl::Length::in);
        nScreenPPTY = o3tl::convert<double>(pDev->GetDPIY(), o3tl::Length::twip, o3tl::Length::in);
    }
    else
    {
        // Avoid cumulative placement errors by intentionally limiting
        // precision.
        Point aPix1000 = pDev->LogicToPixel(Point(1000, 1000), MapMode(MapUnit::MapTwip));
        nScreenPPTX = aPix1000.X() / 1000.0;
        nScreenPPTY = aPix1000.Y() / 1000.0;
    }
}

const OUString& ScGlobal::GetClipDocName()
{
    return aStrClipDocName;
}

void ScGlobal::SetClipDocName( const OUString& rNew )
{
    assert(!bThreadedGroupCalcInProgress);
    aStrClipDocName = rNew;
}

void ScGlobal::InitTextHeight(SfxItemPool& rPool)
{
    // this gets handed over the m_pMessagePool in ScModule::ScModule, so
    // the previously used item ScPatternAttr is unchanged. This allows to
    // just use an temporary incarnation of a CellAttributeHelper here
    const CellAttributeHelper aTempHelper(rPool);
    const ScPatternAttr& rDefaultCellAttribute(aTempHelper.getDefaultCellAttribute());

    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    ScopedVclPtrInstance< VirtualDevice > pVirtWindow( *pDefaultDev );
    pVirtWindow->SetMapMode(MapMode(MapUnit::MapPixel));
    vcl::Font aDefFont;
    rDefaultCellAttribute.fillFontOnly(aDefFont, pVirtWindow); // Font color doesn't matter here
    pVirtWindow->SetFont(aDefFont);
    sal_uInt16 nTest = static_cast<sal_uInt16>(
        pVirtWindow->PixelToLogic(Size(0, pVirtWindow->GetTextHeight()), MapMode(MapUnit::MapTwip)).Height());

    if (nTest > nDefFontHeight)
        nDefFontHeight = nTest;

    const SvxMarginItem& rMargin(rDefaultCellAttribute.GetItem(ATTR_MARGIN));

    nTest = static_cast<sal_uInt16>(nDefFontHeight + rMargin.GetTopMargin()
                                    + rMargin.GetBottomMargin() - STD_ROWHEIGHT_DIFF);

    if (nTest > nStdRowHeight)
        nStdRowHeight = nTest;
}

void ScGlobal::Clear()
{
    // Destroy asyncs _before_ ExitExternalFunc!
    theAddInAsyncTbl.clear();
    ExitExternalFunc();
    ClearAutoFormat();
    xSearchItem.reset();
    delete pLegacyFuncCollection.exchange(nullptr);
    delete pAddInCollection.exchange(nullptr);
    xUserList.reset();
    xStarCalcFunctionList.reset(); // Destroy before ResMgr!
    xStarCalcFunctionMgr.reset();
    ScParameterClassification::Exit();
    ScCompiler::DeInit();
    ScInterpreter::GlobalExit(); // Delete static Stack

    xEmptyBrushItem.reset();
    xButtonBrushItem.reset();
    xEnglishFormatter.reset();
    delete pCaseTransliteration.exchange(nullptr);
    delete pTransliteration.exchange(nullptr);
    delete pCaseCollator.exchange(nullptr);
    delete pCollator.exchange(nullptr);
    oCalendar.reset();
    oSysLocale.reset();
    delete pLocale.exchange(nullptr);

    delete pUnitConverter.exchange(nullptr);
    xFieldEditEngine.reset();
    delete pSharedStringPoolPurge.exchange(nullptr);

    xDrawClipDocShellRef.clear();
}

rtl_TextEncoding ScGlobal::GetCharsetValue( std::u16string_view rCharSet )
{
    // new TextEncoding values
    if ( CharClass::isAsciiNumeric( rCharSet ) )
    {
        sal_Int32 nVal = o3tl::toInt32(rCharSet);
        if ( nVal == RTL_TEXTENCODING_DONTKNOW )
            return osl_getThreadTextEncoding();
        return static_cast<rtl_TextEncoding>(nVal);
    }
    // old CharSet values for compatibility
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"ANSI")     ) return RTL_TEXTENCODING_MS_1252;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"MAC")      ) return RTL_TEXTENCODING_APPLE_ROMAN;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC")    ) return RTL_TEXTENCODING_IBM_850;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_437")) return RTL_TEXTENCODING_IBM_437;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_850")) return RTL_TEXTENCODING_IBM_850;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_860")) return RTL_TEXTENCODING_IBM_860;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_861")) return RTL_TEXTENCODING_IBM_861;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_863")) return RTL_TEXTENCODING_IBM_863;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"IBMPC_865")) return RTL_TEXTENCODING_IBM_865;
    // Some wrong "help" on the net mentions UTF8 and even unoconv uses it,
    // which worked accidentally if the system encoding is UTF-8 anyway, so
    // support it ;) but only when reading.
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"UTF8"))      return RTL_TEXTENCODING_UTF8;
    else if (o3tl::equalsIgnoreAsciiCase(rCharSet, u"UTF-8"))     return RTL_TEXTENCODING_UTF8;
    else return osl_getThreadTextEncoding();
}

OUString ScGlobal::GetCharsetString( rtl_TextEncoding eVal )
{
    const char* pChar;
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
    return bool(xStarCalcFunctionList);
}

ScFunctionList* ScGlobal::GetStarCalcFunctionList()
{
    assert(!bThreadedGroupCalcInProgress);
    if ( !xStarCalcFunctionList )
        xStarCalcFunctionList.reset( new ScFunctionList( SC_MOD()->GetFormulaOptions().GetUseEnglishFuncName()));

    return xStarCalcFunctionList.get();
}

ScFunctionMgr* ScGlobal::GetStarCalcFunctionMgr()
{
    assert(!bThreadedGroupCalcInProgress);
    if ( !xStarCalcFunctionMgr )
        xStarCalcFunctionMgr.reset(new ScFunctionMgr);

    return xStarCalcFunctionMgr.get();
}

void ScGlobal::ResetFunctionList()
{
    // FunctionMgr has pointers into FunctionList, must also be updated
    xStarCalcFunctionMgr.reset();
    xStarCalcFunctionList.reset();
    // Building new names also needs InputHandler data to be refreshed.
    maInputHandlerFunctionNames = InputHandlerFunctionNames();
}

const InputHandlerFunctionNames& ScGlobal::GetInputHandlerFunctionNames()
{
    if (maInputHandlerFunctionNames.maFunctionData.empty())
    {
        const OUString aParenthesesReplacement( cParenthesesReplacement);
        const ScFunctionList* pFuncList = GetStarCalcFunctionList();
        const sal_uInt32 nListCount = pFuncList->GetCount();
        const CharClass* pCharClass = (pFuncList->IsEnglishFunctionNames()
                ? ScCompiler::GetCharClassEnglish()
                : ScCompiler::GetCharClassLocalized());
        for (sal_uInt32 i=0; i < nListCount; ++i)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction( i );
            if ( pDesc->mxFuncName )
            {
                OUString aFuncName(pCharClass->uppercase(*(pDesc->mxFuncName)));
                // fdo#75264 fill maFormulaChar with all characters used in formula names
                for (sal_Int32 j = 0; j < aFuncName.getLength(); j++)
                    maInputHandlerFunctionNames.maFunctionChar.insert(aFuncName[j]);
                maInputHandlerFunctionNames.maFunctionData.insert(
                        ScTypedStrData(*(pDesc->mxFuncName) + aParenthesesReplacement, 0.0, 0.0,
                            ScTypedStrData::Standard));
                pDesc->initArgumentInfo();
                OUString aEntry = pDesc->getSignature();
                maInputHandlerFunctionNames.maFunctionDataPara.insert(
                        ScTypedStrData(aEntry, 0.0, 0.0, ScTypedStrData::Standard));
            }
        }
    }
    return maInputHandlerFunctionNames;
}

ScUnitConverter* ScGlobal::GetUnitConverter()
{
    return comphelper::doubleCheckedInit( pUnitConverter,
        []() { return new ScUnitConverter; });
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

OUString ScGlobal::addToken(std::u16string_view rTokenList, std::u16string_view rToken,
    sal_Unicode cSep, sal_Int32 nSepCount, bool bForceSep)
{
    OUStringBuffer aBuf(rTokenList);
    if( bForceSep || (!rToken.empty() && !rTokenList.empty()) )
        comphelper::string::padToLength(aBuf, aBuf.getLength() + nSepCount, cSep);
    aBuf.append(rToken);
    return aBuf.makeStringAndClear();
}

bool ScGlobal::IsQuoted( std::u16string_view rString, sal_Unicode cQuote )
{
    return (rString.size() >= 2) && (rString[0] == cQuote) && (rString[ rString.size() - 1 ] == cQuote);
}

void ScGlobal::AddQuotes( OUString& rString, sal_Unicode cQuote, bool bEscapeEmbedded )
{
    if (bEscapeEmbedded)
    {
        sal_Unicode pQ[3];
        pQ[0] = pQ[1] = cQuote;
        pQ[2] = 0;
        OUString aQuotes( pQ );
        rString = rString.replaceAll( OUStringChar(cQuote), aQuotes);
    }
    rString = OUStringChar( cQuote ) + rString + OUStringChar( cQuote );
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
            rString = rString.replaceAll( aQuotes, OUStringChar(cQuote));
        }
    }
}

sal_Int32 ScGlobal::FindUnquoted( const OUString& rString, sal_Unicode cChar, sal_Int32 nStart )
{
    assert(nStart >= 0);
    const sal_Unicode cQuote = '\'';
    const sal_Unicode* const pStart = rString.getStr();
    const sal_Unicode* const pStop = pStart + rString.getLength();
    const sal_Unicode* p = pStart + nStart;
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

void ScGlobal::OpenURL(const OUString& rURL, const OUString& rTarget, bool bIgnoreSettings)
{
    // OpenURL is always called in the GridWindow by mouse clicks in some way or another.
    // That's why pScActiveViewShell and nScClickMouseModifier are correct.

    // Fragments pointing into the current document should be always opened.
    if (!bIgnoreSettings && !(ShouldOpenURL() || rURL.startsWith("#")))
        return;

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    OUString aUrlName( rURL );
    SfxViewFrame* pFrame = nullptr;
    SfxObjectShell* pObjShell = nullptr;
    OUString aReferName;
    if ( pScActiveViewShell )
    {
        pFrame = &pScActiveViewShell->GetViewFrame();
        pObjShell = pFrame->GetObjectShell();
        const SfxMedium* pMed = pObjShell->GetMedium();
        if (pMed)
            aReferName = pMed->GetName();
    }

    // Don't fiddle with fragments pointing into current document.
    // Also don't mess around with a vnd.sun.star.script or service or other
    // internal "URI".
    if (!aUrlName.startsWith("#")
            && !aUrlName.startsWithIgnoreAsciiCase("vnd.sun.star.script:")
            && !aUrlName.startsWithIgnoreAsciiCase("macro:")
            && !aUrlName.startsWithIgnoreAsciiCase("slot:")
            && !aUrlName.startsWithIgnoreAsciiCase("service:")
            && !aUrlName.startsWithIgnoreAsciiCase(".uno:"))
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

    if (!SfxObjectShell::AllowedLinkProtocolFromDocument(aUrlName, pObjShell, pFrame ? pFrame->GetFrameWeld() : nullptr))
        return;

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

bool ScGlobal::ShouldOpenURL()
{
    bool bCtrlClickHappened = (nScClickMouseModifier & KEY_MOD1);
    bool bCtrlClickSecOption = SvtSecurityOptions::IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );
    if( bCtrlClickHappened && ! bCtrlClickSecOption )
    {
        // return since ctrl+click happened when the
        // ctrl+click security option was disabled, link should not open
        return false;
    }
    else if( ! bCtrlClickHappened && bCtrlClickSecOption )
    {
        // ctrl+click did not happen; only click happened maybe with some
        // other key combo. and security option is set, so return
        return false;
    }
    return true;
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

    const SfxUInt32Item* pHardItem = rSet.GetItemIfSet( ATTR_VALUE_FORMAT, false );
    if ( !pHardItem )
        return;

    const SvNumberformat* pHardFormat = rFormatter.GetEntry(
        pHardItem->GetValue() );

    sal_uInt32 nParentFmt = 0; // Pool default
    const SfxItemSet* pParent = rSet.GetParent();
    if ( pParent )
        nParentFmt = pParent->Get( ATTR_VALUE_FORMAT ).GetValue();
    const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );

    if ( pHardFormat && pParFormat &&
            (pHardFormat->GetLanguage() != pParFormat->GetLanguage()) )
        rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
}

utl::TransliterationWrapper& ScGlobal::GetTransliteration()
{
    return *comphelper::doubleCheckedInit( pTransliteration,
        []()
        {
            const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
            ::utl::TransliterationWrapper* p = new ::utl::TransliterationWrapper(
                ::comphelper::getProcessComponentContext(), TransliterationFlags::IGNORE_CASE );
            p->loadModuleIfNeeded( eOfficeLanguage );
            return p;
        });
}
::utl::TransliterationWrapper& ScGlobal::GetCaseTransliteration()
{
    return *comphelper::doubleCheckedInit( pCaseTransliteration,
        []()
        {
            const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
            ::utl::TransliterationWrapper* p = new ::utl::TransliterationWrapper(
                ::comphelper::getProcessComponentContext(), TransliterationFlags::NONE );
            p->loadModuleIfNeeded( eOfficeLanguage );
            return p;
        });
}
utl::TransliterationWrapper& ScGlobal::GetTransliteration(bool bCaseSensitive)
{
    return bCaseSensitive ? GetCaseTransliteration() : GetTransliteration();
}

const LocaleDataWrapper& ScGlobal::getLocaleData()
{
    OSL_ENSURE(
        oSysLocale,
        "ScGlobal::getLocaleDataPtr() called before ScGlobal::Init()");

    return oSysLocale->GetLocaleData();
}

const CharClass& ScGlobal::getCharClass()
{
    OSL_ENSURE(
        oSysLocale,
        "ScGlobal::getCharClassPtr() called before ScGlobal::Init()");

    return oSysLocale->GetCharClass();
}

CalendarWrapper& ScGlobal::GetCalendar()
{
    assert(!bThreadedGroupCalcInProgress);
    if ( !oCalendar )
    {
        oCalendar.emplace( ::comphelper::getProcessComponentContext() );
        oCalendar->loadDefaultCalendar( GetLocale() );
    }
    return *oCalendar;
}

namespace {

struct GetMutex {
    osl::Mutex * operator ()() {
        static osl::Mutex m;
        return &m;
    }
};

}

CollatorWrapper& ScGlobal::GetCollator()
{
    return *comphelper::doubleCheckedInit( pCollator,
        []()
        {
            CollatorWrapper* p = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
            p->loadDefaultCollator( GetLocale(), SC_COLLATOR_IGNORES );
            return p;
        },
        GetMutex());
}
CollatorWrapper& ScGlobal::GetCaseCollator()
{
    return *comphelper::doubleCheckedInit( pCaseCollator,
        []()
        {
            CollatorWrapper* p = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
            p->loadDefaultCollator( GetLocale(), 0 );
            return p;
        },
        GetMutex());
}
CollatorWrapper& ScGlobal::GetCollator(bool bCaseSensitive)
{
    return bCaseSensitive ? GetCaseCollator() : GetCollator();
}
css::lang::Locale& ScGlobal::GetLocale()
{
    return *comphelper::doubleCheckedInit( pLocale,
        []() { return new css::lang::Locale( Application::GetSettings().GetLanguageTag().getLocale()); });
}

ScFieldEditEngine& ScGlobal::GetStaticFieldEditEngine()
{
    assert(!bThreadedGroupCalcInProgress);
    if (!xFieldEditEngine)
    {
        // Creating a ScFieldEditEngine with pDocument=NULL leads to document
        // specific fields not being resolvable! See
        // ScFieldEditEngine::CalcFieldValue(). pEnginePool=NULL lets
        // EditEngine internally create and delete a default pool.
        xFieldEditEngine.reset(new ScFieldEditEngine( nullptr, nullptr));
    }
    return *xFieldEditEngine;
}

sc::SharedStringPoolPurge& ScGlobal::GetSharedStringPoolPurge()
{
    return *comphelper::doubleCheckedInit( pSharedStringPoolPurge,
        []() { return new sc::SharedStringPoolPurge; });
}

OUString ScGlobal::ReplaceOrAppend( const OUString& rString,
        std::u16string_view rPlaceholder, const OUString& rReplacement )
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
