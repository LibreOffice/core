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
#include <sal/macros.h>
#include <tools/rcid.h>
#include <unotools/charclass.hxx>
#include <unotools/securityoptions.hxx>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <numeric>


#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>

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
#include "random.hxx"
#include "editutil.hxx"

// -----------------------------------------------------------------------


ScDocShellRef*  ScGlobal::pDrawClipDocShellRef = NULL;
SvxSearchItem*  ScGlobal::pSearchItem = NULL;
ScAutoFormat*   ScGlobal::pAutoFormat = NULL;
FuncCollection* ScGlobal::pFuncCollection = NULL;
ScUnoAddInCollection* ScGlobal::pAddInCollection = NULL;
ScUserList*     ScGlobal::pUserList = NULL;
OUString**      ScGlobal::ppRscString = NULL;
LanguageType    ScGlobal::eLnge = LANGUAGE_SYSTEM;
::com::sun::star::lang::Locale*     ScGlobal::pLocale = NULL;
SvtSysLocale*   ScGlobal::pSysLocale = NULL;
const CharClass*  ScGlobal::pCharClass = NULL;
const LocaleDataWrapper*  ScGlobal::pLocaleData = NULL;
CalendarWrapper* ScGlobal::pCalendar = NULL;
CollatorWrapper* ScGlobal::pCollator = NULL;
CollatorWrapper* ScGlobal::pCaseCollator = NULL;
::utl::TransliterationWrapper* ScGlobal::pTransliteration = NULL;
::utl::TransliterationWrapper* ScGlobal::pCaseTransliteration = NULL;
::com::sun::star::uno::Reference< ::com::sun::star::i18n::XOrdinalSuffix> ScGlobal::xOrdinalSuffix = NULL;
IntlWrapper*    ScGlobal::pScIntlWrapper = NULL;
sal_Unicode     ScGlobal::cListDelimiter = ',';
String*         ScGlobal::pEmptyString = NULL;
OUString*       ScGlobal::pEmptyOUString = NULL;
String*         ScGlobal::pStrClipDocName = NULL;

SvxBrushItem*   ScGlobal::pEmptyBrushItem = NULL;
SvxBrushItem*   ScGlobal::pButtonBrushItem = NULL;
SvxBrushItem*   ScGlobal::pEmbeddedBrushItem = NULL;
SvxBrushItem*   ScGlobal::pProtectedBrushItem = NULL;

ImageList*      ScGlobal::pOutlineBitmaps = NULL;

ScFunctionList* ScGlobal::pStarCalcFunctionList = NULL;
ScFunctionMgr*  ScGlobal::pStarCalcFunctionMgr  = NULL;

ScUnitConverter* ScGlobal::pUnitConverter = NULL;
SvNumberFormatter* ScGlobal::pEnglishFormatter = NULL;
ScFieldEditEngine* ScGlobal::pFieldEditEngine = NULL;

double          ScGlobal::nScreenPPTX           = 96.0;
double          ScGlobal::nScreenPPTY           = 96.0;

sal_uInt16          ScGlobal::nDefFontHeight        = 225;
sal_uInt16          ScGlobal::nStdRowHeight         = 256;

long            ScGlobal::nLastRowHeightExtra   = 0;
long            ScGlobal::nLastColWidthExtra    = STD_EXTRA_WIDTH;

static sal_uInt16 nPPTZoom = 0;     // ScreenZoom used to determine nScreenPPTX/Y


class SfxViewShell;
SfxViewShell* pScActiveViewShell = NULL;            //! als Member !!!!!
sal_uInt16 nScClickMouseModifier = 0;                   //! dito
sal_uInt16 nScFillModeMouseModifier = 0;                //! dito

// Hack: ScGlobal::GetUserList() muss InitAppOptions in der UI aufrufen,
//       damit UserList aus Cfg geladen wird

void global_InitAppOptions();

//========================================================================
//
//      statische Funktionen
//
//========================================================================

sal_Bool ScGlobal::HasAttrChanged( const SfxItemSet&  rNewAttrs,
                               const SfxItemSet&  rOldAttrs,
                               const sal_uInt16       nWhich )
{
    sal_Bool                bInvalidate = false;
    const SfxItemState  eNewState   = rNewAttrs.GetItemState( nWhich );
    const SfxItemState  eOldState   = rOldAttrs.GetItemState( nWhich );

    //----------------------------------------------------------

    if ( eNewState == eOldState )
    {
        // beide Items gesetzt
        // PoolItems, d.h. Pointer-Vergleich zulaessig
        if ( SFX_ITEM_SET == eOldState )
            bInvalidate = (&rNewAttrs.Get( nWhich ) != &rOldAttrs.Get( nWhich ));
    }
    else
    {
        // ein Default-Item dabei
        // PoolItems, d.h. Item-Vergleich noetig

        const SfxPoolItem& rOldItem = ( SFX_ITEM_SET == eOldState )
                    ? rOldAttrs.Get( nWhich )
                    : rOldAttrs.GetPool()->GetDefaultItem( nWhich );

        const SfxPoolItem& rNewItem = ( SFX_ITEM_SET == eNewState )
                    ? rNewAttrs.Get( nWhich )
                    : rNewAttrs.GetPool()->GetDefaultItem( nWhich );

        bInvalidate = sal::static_int_cast<sal_Bool>(rNewItem != rOldItem);
    }

    return bInvalidate;
}

sal_uLong ScGlobal::GetStandardFormat( SvNumberFormatter& rFormatter,
        sal_uLong nFormat, short nType )
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

sal_uLong ScGlobal::GetStandardFormat( double fNumber, SvNumberFormatter& rFormatter,
        sal_uLong nFormat, short nType )
{
    const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
    if ( pFormat )
        return rFormatter.GetStandardFormat( fNumber, nFormat, nType,
            pFormat->GetLanguage() );
    return rFormatter.GetStandardFormat( nType, eLnge );
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


//------------------------------------------------------------------------

sal_Bool ScGlobal::CheckWidthInvalidate( bool& bNumFormatChanged,
                                     const SfxItemSet& rNewAttrs,
                                     const SfxItemSet& rOldAttrs )
{
    // Ueberpruefen, ob Attributaenderungen in rNewAttrs gegnueber
    // rOldAttrs die Textbreite an einer Zelle ungueltig machen

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
        pSearchItem->SetAppFlag( SVX_SEARCHAPP_CALC );
    }
    return *pSearchItem;
}

void ScGlobal::SetSearchItem( const SvxSearchItem& rNew )
{
    // Hier waere ein Zuweisungsoperator ganz nett:
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rNew.Clone();

    pSearchItem->SetWhich( SID_SEARCH_ITEM );
    pSearchItem->SetAppFlag( SVX_SEARCHAPP_CALC );
}

void ScGlobal::ClearAutoFormat()
{
    if (pAutoFormat!=NULL)
    {
        delete pAutoFormat;
        pAutoFormat=NULL;
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

FuncCollection* ScGlobal::GetFuncCollection()
{
    if (!pFuncCollection)
        pFuncCollection = new FuncCollection();
    return pFuncCollection;
}

ScUnoAddInCollection* ScGlobal::GetAddInCollection()
{
    if (!pAddInCollection)
        pAddInCollection = new ScUnoAddInCollection();
    return pAddInCollection;
}

ScUserList* ScGlobal::GetUserList()
{
    // Hack: Cfg-Item an der App ggF. laden

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
        pUserList = NULL;
    }
}

const OUString& ScGlobal::GetRscString( sal_uInt16 nIndex )
{
    OSL_ENSURE( nIndex < STR_COUNT, "ScGlobal::GetRscString - invalid string index");
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
            ppRscString[ nIndex ] = new OUString(
                    ScCompiler::GetNativeSymbol( eOp));
        else
            ppRscString[ nIndex ] = new OUString(
                    ScRscStrLoader( RID_GLOBSTR, nIndex ).GetString());
    }
    return *ppRscString[ nIndex ];
}

String ScGlobal::GetErrorString(sal_uInt16 nErrNumber)
{
    String sResStr;
    switch (nErrNumber)
    {
        case NOTAVAILABLE          : nErrNumber = STR_NV_STR; break;
        case errNoRef              : nErrNumber = STR_NO_REF_TABLE; break;
        case errNoName             : nErrNumber = STR_NO_NAME_REF; break;
        case errNoAddin            : nErrNumber = STR_NO_ADDIN; break;
        case errNoMacro            : nErrNumber = STR_NO_MACRO; break;
        case errDoubleRef          :
        case errNoValue            : nErrNumber = STR_NO_VALUE; break;
        case errNoCode             : nErrNumber = STR_NULL_ERROR; break;
        case errDivisionByZero     : nErrNumber = STR_DIV_ZERO; break;
        case errIllegalFPOperation : nErrNumber = STR_NUM_ERROR; break;

        default          : sResStr = GetRscString(STR_ERROR_STR) + OUString::number( nErrNumber );
                           nErrNumber = 0;
                           break;
    }
    if( nErrNumber )
        sResStr = GetRscString( nErrNumber );
    return sResStr;
}

String ScGlobal::GetLongErrorString(sal_uInt16 nErrNumber)
{
    switch (nErrNumber)
    {
        case 0:
            break;
        case 1:
        case errIllegalArgument:
            nErrNumber = STR_LONG_ERR_ILL_ARG;
        break;
        case 2:
        case 3:
        case 4:
        case 5:
        case errIllegalFPOperation:
            nErrNumber = STR_LONG_ERR_ILL_FPO;
        break;
        case errIllegalChar:
            nErrNumber = STR_LONG_ERR_ILL_CHAR;
        break;
        case errIllegalParameter:
            nErrNumber = STR_LONG_ERR_ILL_PAR;
        break;
        case errSeparator:
            nErrNumber = STR_LONG_ERR_ILL_SEP;
        break;
        case errPair:
        case errPairExpected:
            nErrNumber = STR_LONG_ERR_PAIR;
        break;
        case errOperatorExpected:
            nErrNumber = STR_LONG_ERR_OP_EXP;
        break;
        case errVariableExpected:
        case errParameterExpected:
            nErrNumber = STR_LONG_ERR_VAR_EXP;
        break;
        case errCodeOverflow:
            nErrNumber = STR_LONG_ERR_CODE_OVF;
        break;
        case errStringOverflow:
            nErrNumber = STR_LONG_ERR_STR_OVF;
        break;
        case errStackOverflow:
        case errInterpOverflow:
            nErrNumber = STR_LONG_ERR_STACK_OVF;
        break;
        case errIllegalJump:
        case errUnknownState:
        case errUnknownVariable:
        case errUnknownOpCode:
        case errUnknownStackVariable:
        case errUnknownToken:
        case errNoCode:
        case errDoubleRef:
            nErrNumber = STR_LONG_ERR_SYNTAX;
        break;
        case errCircularReference:
            nErrNumber = STR_LONG_ERR_CIRC_REF;
        break;
        case errNoConvergence:
            nErrNumber = STR_LONG_ERR_NO_CONV;
        break;
        case errNoRef:
            nErrNumber = STR_LONG_ERR_NO_REF;
        break;
        case errNoName:
            nErrNumber = STR_LONG_ERR_NO_NAME;
        break;
        case errNoAddin:
            nErrNumber = STR_LONG_ERR_NO_ADDIN;
        break;
        case errNoMacro:
            nErrNumber = STR_LONG_ERR_NO_MACRO;
        break;
        case errDivisionByZero:
            nErrNumber = STR_LONG_ERR_DIV_ZERO;
        break;
        case errNestedArray:
            nErrNumber = STR_ERR_LONG_NESTED_ARRAY;
        break;
        case errNoValue:
            nErrNumber = STR_LONG_ERR_NO_VALUE;
        break;
        case NOTAVAILABLE:
            nErrNumber = STR_LONG_ERR_NV;
        break;
        default:
            nErrNumber = STR_ERROR_STR;
        break;
    }
    String aRes( GetRscString( nErrNumber ) );
    return aRes;
}

SvxBrushItem* ScGlobal::GetButtonBrushItem()
{
    pButtonBrushItem->SetColor( Application::GetSettings().GetStyleSettings().GetFaceColor() );
    return pButtonBrushItem;
}

const String& ScGlobal::GetEmptyString()
{
    return *pEmptyString;
}

const OUString& ScGlobal::GetEmptyOUString()
{
    return *pEmptyOUString;
}

ImageList* ScGlobal::GetOutlineSymbols()
{
    ImageList*& rpImageList = pOutlineBitmaps;
    if( !rpImageList )
        rpImageList = new ImageList( ScResId( RID_OUTLINEBITMAPS ) );
    return rpImageList;
}

void ScGlobal::Init()
{
    pEmptyString = new String;
    pEmptyOUString = new OUString;

    //  Die Default-Sprache fuer Zahlenformate (ScGlobal::eLnge)
    //  muss immer LANGUAGE_SYSTEM sein
    //! Dann kann auch die Variable raus
    eLnge = LANGUAGE_SYSTEM;

    //! Wenn Sortierung etc. von der Sprache der installierten Offfice-Version
    //! abhaengen sollen, hier "Application::GetSettings().GetUILanguage()"
    pSysLocale = new SvtSysLocale;
    pCharClass = pSysLocale->GetCharClassPtr();
    pLocaleData = pSysLocale->GetLocaleDataPtr();

    ppRscString = new OUString *[ STR_COUNT ];
    for( sal_uInt16 nC = 0 ; nC < STR_COUNT ; nC++ ) ppRscString[ nC ] = NULL;

    pEmptyBrushItem = new SvxBrushItem( Color( COL_TRANSPARENT ), ATTR_BACKGROUND );
    pButtonBrushItem = new SvxBrushItem( Color(), ATTR_BACKGROUND );
    pEmbeddedBrushItem = new SvxBrushItem( Color( COL_LIGHTCYAN ), ATTR_BACKGROUND );
    pProtectedBrushItem = new SvxBrushItem( Color( COL_LIGHTGRAY ), ATTR_BACKGROUND );

    UpdatePPT(NULL);
    //ScCompiler::InitSymbolsNative();
    // ScParameterClassification _after_ Compiler, needs function resources if
    // arguments are to be merged in, which in turn need strings of function
    // names from the compiler.
    ScParameterClassification::Init();
    srand( (unsigned) time( NULL ) );       // Random Seed Init fuer Interpreter
    sc::rng::seed( time( NULL ) );          // seed for libc rand() replacement

    InitAddIns();

    pStrClipDocName = new String( ScResId( SCSTR_NONAME ) );
    *pStrClipDocName += '1';

    //  ScDocumentPool::InitVersionMaps() ist schon vorher gerufen worden
}

void ScGlobal::UpdatePPT( OutputDevice* pDev )
{
    sal_uInt16 nCurrentZoom = Application::GetSettings().GetStyleSettings().GetScreenZoom();
    if ( nCurrentZoom != nPPTZoom )
    {
        //  Screen PPT values must be updated when ScreenZoom has changed.
        //  If called from Window::DataChanged, the window is passed as pDev,
        //  to make sure LogicToPixel uses a device which already uses the new zoom.
        //  For the initial settings, NULL is passed and GetDefaultDevice used.

        if ( !pDev )
            pDev = Application::GetDefaultDevice();
        Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
        nScreenPPTX = aPix1000.X() / 1000.0;
        nScreenPPTY = aPix1000.Y() / 1000.0;
        nPPTZoom = nCurrentZoom;
    }
}

const String& ScGlobal::GetClipDocName()
{
    return *pStrClipDocName;
}

void ScGlobal::SetClipDocName( const String& rNew )
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

    const ScPatternAttr* pPattern = (const ScPatternAttr*)&pPool->GetDefaultItem(ATTR_PATTERN);
    if (!pPattern)
    {
        OSL_FAIL("ScGlobal::InitTextHeight: No default pattern");
        return;
    }

    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    VirtualDevice aVirtWindow( *pDefaultDev );
    aVirtWindow.SetMapMode(MAP_PIXEL);
    Font aDefFont;
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, &aVirtWindow);        // font color doesn't matter here
    aVirtWindow.SetFont(aDefFont);
    sal_uInt16 nTest = static_cast<sal_uInt16>(
        aVirtWindow.PixelToLogic(Size(0, aVirtWindow.GetTextHeight()), MAP_TWIP).Height());

    if (nTest > nDefFontHeight)
        nDefFontHeight = nTest;

    const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);

    nTest = static_cast<sal_uInt16>(
        nDefFontHeight + pMargin->GetTopMargin() + pMargin->GetBottomMargin() - STD_ROWHEIGHT_DIFF);

    if (nTest > nStdRowHeight)
        nStdRowHeight = nTest;
}

void ScGlobal::Clear()
{
    // asyncs _vor_ ExitExternalFunc zerstoeren!
    for( ScAddInAsyncs::iterator it = theAddInAsyncTbl.begin(); it != theAddInAsyncTbl.end(); ++it )
    {
        delete *it;
    }
    theAddInAsyncTbl.clear();
    ExitExternalFunc();
    DELETEZ(pAutoFormat);
    DELETEZ(pSearchItem);
    DELETEZ(pFuncCollection);
    DELETEZ(pAddInCollection);
    DELETEZ(pUserList);

    for( sal_uInt16 nC = 0 ; nC < STR_COUNT ; nC++ )
        if( ppRscString ) delete ppRscString[ nC ];
    delete[] ppRscString;
    ppRscString = NULL;

    DELETEZ(pStarCalcFunctionList);     // vor ResMgr zerstoeren!
    DELETEZ(pStarCalcFunctionMgr);
    ScParameterClassification::Exit();
    ScCompiler::DeInit();
    ScInterpreter::GlobalExit();            // statischen Stack loeschen

    DELETEZ(pEmptyBrushItem);
    DELETEZ(pButtonBrushItem);
    DELETEZ(pEmbeddedBrushItem);
    DELETEZ(pProtectedBrushItem);
    DELETEZ(pOutlineBitmaps);
    DELETEZ(pEnglishFormatter);
    DELETEZ(pCaseTransliteration);
    DELETEZ(pTransliteration);
    DELETEZ(pCaseCollator);
    DELETEZ(pCollator);
    DELETEZ(pCalendar);
    //! do NOT delete pCharClass since it is a pointer to the single SvtSysLocale instance
    pCharClass = NULL;
    //! do NOT delete pLocaleData since it is a pointer to the single SvtSysLocale instance
    pLocaleData = NULL;
    DELETEZ(pSysLocale);
    DELETEZ(pLocale);
    DELETEZ(pScIntlWrapper);
    DELETEZ(pStrClipDocName);

    DELETEZ(pUnitConverter);
    DELETEZ(pFieldEditEngine);

    ScDocumentPool::DeleteVersionMaps();

    DELETEZ(pEmptyString);
    DELETEZ(pEmptyOUString);
}

//------------------------------------------------------------------------

CharSet ScGlobal::GetCharsetValue( const String& rCharSet )
{
    // new TextEncoding values
    if ( CharClass::isAsciiNumeric( rCharSet ) )
    {
        sal_Int32 nVal = rCharSet.ToInt32();
        if ( !nVal || nVal == RTL_TEXTENCODING_DONTKNOW )
            return osl_getThreadTextEncoding();
        return (CharSet) nVal;
    }
    // old CharSet values for compatibility
    else if (rCharSet.EqualsIgnoreCaseAscii("ANSI")     ) return RTL_TEXTENCODING_MS_1252;
    else if (rCharSet.EqualsIgnoreCaseAscii("MAC")      ) return RTL_TEXTENCODING_APPLE_ROMAN;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC")    ) return RTL_TEXTENCODING_IBM_850;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_437")) return RTL_TEXTENCODING_IBM_437;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_850")) return RTL_TEXTENCODING_IBM_850;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_860")) return RTL_TEXTENCODING_IBM_860;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_861")) return RTL_TEXTENCODING_IBM_861;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_863")) return RTL_TEXTENCODING_IBM_863;
    else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_865")) return RTL_TEXTENCODING_IBM_865;
    else return osl_getThreadTextEncoding();
}

//------------------------------------------------------------------------

String ScGlobal::GetCharsetString( CharSet eVal )
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

//------------------------------------------------------------------------

bool ScGlobal::HasStarCalcFunctionList()
{
    return ( pStarCalcFunctionList != NULL );
}

ScFunctionList* ScGlobal::GetStarCalcFunctionList()
{
    if ( !pStarCalcFunctionList )
        pStarCalcFunctionList = new ScFunctionList;

    return pStarCalcFunctionList;
}

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

ScUnitConverter* ScGlobal::GetUnitConverter()
{
    if ( !pUnitConverter )
        pUnitConverter = new ScUnitConverter;

    return pUnitConverter;
}


//------------------------------------------------------------------------

const sal_Unicode* ScGlobal::UnicodeStrChr( const sal_Unicode* pStr,
            sal_Unicode c )
{
    if ( !pStr )
        return NULL;
    while ( *pStr )
    {
        if ( *pStr == c )
            return pStr;
        pStr++;
    }
    return NULL;
}

// ----------------------------------------------------------------------------

OUString ScGlobal::addToken(const OUString& rTokenList, const OUString& rToken,
    sal_Unicode cSep, sal_Int32 nSepCount, bool bForceSep)
{
    OUStringBuffer aBuf(rTokenList);
    if( bForceSep || (!rToken.isEmpty() && !rTokenList.isEmpty()) )
        comphelper::string::padToLength(aBuf, aBuf.getLength() + nSepCount, cSep);
    aBuf.append(rToken);
    return aBuf.makeStringAndClear();
}

bool ScGlobal::IsQuoted( const String& rString, sal_Unicode cQuote )
{
    return (rString.Len() >= 2) && (rString.GetChar( 0 ) == cQuote) && (rString.GetChar( rString.Len() - 1 ) == cQuote);
}

void ScGlobal::AddQuotes( OUString& rString, sal_Unicode cQuote, bool bEscapeEmbedded )
{
    if (bEscapeEmbedded)
    {
        sal_Unicode pQ[3];
        pQ[0] = pQ[1] = cQuote;
        pQ[2] = 0;
        OUString aQuotes( pQ );
        rString = rString.replaceAll( OUString(cQuote), aQuotes);
    }
    rString = OUString( cQuote ) + rString + OUString( cQuote );
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
            rString = rString.replaceAll( aQuotes, OUString(cQuote));
        }
    }
}

xub_StrLen ScGlobal::FindUnquoted( const String& rString, sal_Unicode cChar, xub_StrLen nStart, sal_Unicode cQuote )
{
    const sal_Unicode* const pStart = rString.GetBuffer();
    const sal_Unicode* const pStop = pStart + rString.Len();
    const sal_Unicode* p = pStart + nStart;
    bool bQuoted = false;
    while (p < pStop)
    {
        if (*p == cChar && !bQuoted)
            return sal::static_int_cast< xub_StrLen >( p - pStart );
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
    return STRING_NOTFOUND;
}

const sal_Unicode* ScGlobal::FindUnquoted( const sal_Unicode* pString, sal_Unicode cChar, sal_Unicode cQuote )
{
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
    return NULL;
}

//------------------------------------------------------------------------

sal_Bool ScGlobal::EETextObjEqual( const EditTextObject* pObj1,
                               const EditTextObject* pObj2 )
{
    if ( pObj1 == pObj2 )               // both empty or the same object
        return sal_True;

    if ( pObj1 && pObj2 )
    {
        //  first test for equal text content
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
        sal_uLong nSize = aStream1.Tell();
        if ( aStream2.Tell() == nSize )
            if ( !memcmp( aStream1.GetData(), aStream2.GetData(), (sal_uInt16) nSize ) )
                return sal_True;
    }

    return false;
}

void ScGlobal::OpenURL( const String& rURL, const String& rTarget )
{
    //  OpenURL wird immer ueber irgendwelche Umwege durch Mausklicks im GridWindow
    //  aufgerufen, darum stimmen pScActiveViewShell und nScClickMouseModifier.
    //SvtSecurityOptions to access Libreoffice global security parameters
    SvtSecurityOptions aSecOpt;
    bool bCtrlClickHappened = (nScClickMouseModifier & KEY_MOD1);
    bool bCtrlClickSecOption = aSecOpt.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
    if( bCtrlClickHappened && !( bCtrlClickSecOption ) )
    {
        //return since ctrl+click happened when the
        //ctrl+click security option was disabled, link should not open
        return;
    }
    else if( !( bCtrlClickHappened ) && bCtrlClickSecOption )
    {
        //ctrl+click did not happen; only click happened maybe with some
        //other key combo. and security option is set, so return
        return;
    }
    SfxStringItem aUrl( SID_FILE_NAME, rURL );
    SfxStringItem aTarget( SID_TARGETNAME, rTarget );
    aTarget.SetValue(OUString("_blank"));
    SfxViewFrame* pFrame = NULL;
    String aReferName;
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
    SfxBoolItem aBrowsing( SID_BROWSE, sal_True );

    //  kein SID_SILENT mehr

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetDispatcher()->Execute( SID_OPENDOC,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                    &aUrl, &aTarget,
                                    &aFrm, &aReferer,
                                    &aNewView, &aBrowsing,
                                    0L );
}

//------------------------------------------------------------------------

sal_Bool ScGlobal::IsSystemRTL()
{
    return MsLangId::isRightToLeft( Application::GetSettings().GetLanguageTag().getLanguageType() );
}

sal_uInt8 ScGlobal::GetDefaultScriptType()
{
    //  Used when text contains only WEAK characters.
    //  Script type of office language is used then (same as GetEditDefaultLanguage,
    //  to get consistent behavior of text in simple cells and EditEngine,
    //  also same as GetAppLanguage() in Writer)

    return (sal_uInt8) SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
}

LanguageType ScGlobal::GetEditDefaultLanguage()
{
    //  used for EditEngine::SetDefaultLanguage

    return Application::GetSettings().GetLanguageTag().getLanguageType();
}

sal_uInt16 ScGlobal::GetScriptedWhichID( sal_uInt8 nScriptType, sal_uInt16 nWhich )
{
    switch ( nScriptType )
    {
        case SCRIPTTYPE_LATIN:
        case SCRIPTTYPE_ASIAN:
        case SCRIPTTYPE_COMPLEX:
        break;      // take exact matches
        default:    // prefer one, first COMPLEX, then ASIAN
            if ( nScriptType & SCRIPTTYPE_COMPLEX )
                nScriptType = SCRIPTTYPE_COMPLEX;
            else if ( nScriptType & SCRIPTTYPE_ASIAN )
                nScriptType = SCRIPTTYPE_ASIAN;
    }
    switch ( nScriptType )
    {
        case SCRIPTTYPE_COMPLEX:
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
        case SCRIPTTYPE_ASIAN:
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

//------------------------------------------------------------------------

void ScGlobal::AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter )
{
    OSL_ENSURE( rSet.GetItemState( ATTR_LANGUAGE_FORMAT, false ) == SFX_ITEM_DEFAULT,
        "ScGlobal::AddLanguage - language already added");

    const SfxPoolItem* pHardItem;
    if ( rSet.GetItemState( ATTR_VALUE_FORMAT, false, &pHardItem ) == SFX_ITEM_SET )
    {
        const SvNumberformat* pHardFormat = rFormatter.GetEntry(
            ((const SfxUInt32Item*)pHardItem)->GetValue() );

        sal_uLong nParentFmt = 0;   // pool default
        const SfxItemSet* pParent = rSet.GetParent();
        if ( pParent )
            nParentFmt = ((const SfxUInt32Item&)pParent->Get( ATTR_VALUE_FORMAT )).GetValue();
        const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );

        if ( pHardFormat && pParFormat &&
                (pHardFormat->GetLanguage() != pParFormat->GetLanguage()) )
            rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
    }
}


//--------------------------------------------------------------------



//------------------------------------------------------------------------

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
::com::sun::star::lang::Locale*     ScGlobal::GetLocale()
{
    if ( !pLocale )
    {
        pLocale = new ::com::sun::star::lang::Locale( Application::GetSettings().GetLanguageTag().getLocale());
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
        pFieldEditEngine = new ScFieldEditEngine( NULL, NULL);
    }
    return *pFieldEditEngine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
