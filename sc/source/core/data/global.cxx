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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include <vcl/svapp.hxx>
#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/brshitem.hxx>
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
#include <tools/rcid.h>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <numeric>


#include <i18npool/mslangid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/processfactory.hxx>
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

// -----------------------------------------------------------------------

#define CLIPST_AVAILABLE    0
#define CLIPST_CAPTURED     1
#define CLIPST_DELETE       2
#define CLIPST_DRAW         3

ScDocShellRef*  ScGlobal::pDrawClipDocShellRef = NULL;
SvxSearchItem*  ScGlobal::pSearchItem = NULL;
ScAutoFormat*   ScGlobal::pAutoFormat = NULL;
FuncCollection* ScGlobal::pFuncCollection = NULL;
ScUnoAddInCollection* ScGlobal::pAddInCollection = NULL;
ScUserList*     ScGlobal::pUserList = NULL;
String**        ScGlobal::ppRscString = NULL;
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
String*         ScGlobal::pStrClipDocName = NULL;

SvxBrushItem*   ScGlobal::pEmptyBrushItem = NULL;
SvxBrushItem*   ScGlobal::pButtonBrushItem = NULL;
SvxBrushItem*   ScGlobal::pEmbeddedBrushItem = NULL;
SvxBrushItem*   ScGlobal::pProtectedBrushItem = NULL;

ImageList*      ScGlobal::pOutlineBitmaps = NULL;
ImageList*      ScGlobal::pOutlineBitmapsHC = NULL;

ScFunctionList* ScGlobal::pStarCalcFunctionList = NULL;
ScFunctionMgr*  ScGlobal::pStarCalcFunctionMgr  = NULL;

ScUnitConverter* ScGlobal::pUnitConverter = NULL;
SvNumberFormatter* ScGlobal::pEnglishFormatter = NULL;

double          ScGlobal::nScreenPPTX           = 96.0;
double          ScGlobal::nScreenPPTY           = 96.0;

sal_uInt16          ScGlobal::nDefFontHeight        = 240;
sal_uInt16          ScGlobal::nStdRowHeight         = 257;

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
    sal_Bool                bInvalidate = sal_False;
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

sal_uLong ScGlobal::GetStandardFormat( double fNumber, SvNumberFormatter& rFormatter,
        sal_uLong nFormat, short nType )
{
    const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
    if ( pFormat )
        return rFormatter.GetStandardFormat( fNumber, nFormat, nType,
            pFormat->GetLanguage() );
    return rFormatter.GetStandardFormat( nType, eLnge );
}


// static
SvNumberFormatter* ScGlobal::GetEnglishFormatter()
{
    if ( !pEnglishFormatter )
    {
        pEnglishFormatter = new SvNumberFormatter(
            ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
        pEnglishFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
    }
    return pEnglishFormatter;
}


//------------------------------------------------------------------------

sal_Bool ScGlobal::CheckWidthInvalidate( sal_Bool& bNumFormatChanged,
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

const String& ScGlobal::GetRscString( sal_uInt16 nIndex )
{
    DBG_ASSERT( nIndex < STR_COUNT, "ScGlobal::GetRscString - invalid string index");
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
            ppRscString[ nIndex ] = new String(
                    ScCompiler::GetNativeSymbol( eOp));
        else
            ppRscString[ nIndex ] = new String(
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

        default          : sResStr = GetRscString(STR_ERROR_STR);
                           sResStr += String::CreateFromInt32( nErrNumber );
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

ImageList* ScGlobal::GetOutlineSymbols( bool bHC )
{
    ImageList*& rpImageList = bHC ? pOutlineBitmapsHC : pOutlineBitmaps;
    if( !rpImageList )
        rpImageList = new ImageList( ScResId( bHC ? RID_OUTLINEBITMAPS_H : RID_OUTLINEBITMAPS ) );
    return rpImageList;
}

void ScGlobal::Init()
{
    pEmptyString = new String;

    //  Die Default-Sprache fuer Zahlenformate (ScGlobal::eLnge)
    //  muss immer LANGUAGE_SYSTEM sein
    //! Dann kann auch die Variable raus
    eLnge = LANGUAGE_SYSTEM;

    //! Wenn Sortierung etc. von der Sprache der installierten Offfice-Version
    //! abhaengen sollen, hier "Application::GetSettings().GetUILanguage()"
    pSysLocale = new SvtSysLocale;
    pCharClass = pSysLocale->GetCharClassPtr();
    pLocaleData = pSysLocale->GetLocaleDataPtr();

    ppRscString = new String *[ STR_COUNT ];
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
        DBG_ERROR("kein Pool bei ScGlobal::InitTextHeight");
        return;
    }

    const ScPatternAttr* pPattern = (const ScPatternAttr*)&pPool->GetDefaultItem(ATTR_PATTERN);
    if (!pPattern)
    {
        DBG_ERROR("kein Default-Pattern bei ScGlobal::InitTextHeight");
        return;
    }

//  String aTestString('X');
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    VirtualDevice aVirtWindow( *pDefaultDev );
    aVirtWindow.SetMapMode(MAP_PIXEL);
    Font aDefFont;
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, &aVirtWindow);        // font color doesn't matter here
    aVirtWindow.SetFont(aDefFont);
    nDefFontHeight = (sal_uInt16) aVirtWindow.PixelToLogic(Size(0, aVirtWindow.GetTextHeight()),
                                MAP_TWIP).Height();

    const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);

    nStdRowHeight = (sal_uInt16) ( nDefFontHeight +
                                pMargin->GetTopMargin() + pMargin->GetBottomMargin()
                                - STD_ROWHEIGHT_DIFF );
}

void ScGlobal::Clear()
{
    // asyncs _vor_ ExitExternalFunc zerstoeren!
    theAddInAsyncTbl.DeleteAndDestroy( 0, theAddInAsyncTbl.Count() );
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
    DELETEZ(pOutlineBitmapsHC);
//  DELETEZ(pAnchorBitmap);
//  DELETEZ(pGrayAnchorBitmap);
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

    ScDocumentPool::DeleteVersionMaps();

    DELETEZ(pEmptyString);
}

//------------------------------------------------------------------------

// static
CharSet ScGlobal::GetCharsetValue( const String& rCharSet )
{
    // new TextEncoding values
    if ( CharClass::isAsciiNumeric( rCharSet ) )
    {
        sal_Int32 nVal = rCharSet.ToInt32();
        if ( !nVal || nVal == RTL_TEXTENCODING_DONTKNOW )
            return gsl_getSystemTextEncoding();
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
//  else if (rCharSet.EqualsIgnoreCaseAscii("SYSTEM")   ) return gsl_getSystemTextEncoding();
    else return gsl_getSystemTextEncoding();
}

//------------------------------------------------------------------------

// static
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
            return String::CreateFromInt32( eVal );
    }
    return String::CreateFromAscii(pChar);
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

// static
ScUnitConverter* ScGlobal::GetUnitConverter()
{
    if ( !pUnitConverter )
        pUnitConverter = new ScUnitConverter;

    return pUnitConverter;
}


//------------------------------------------------------------------------

// static
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

void ScGlobal::AddToken( String& rTokenList, const String& rToken, sal_Unicode cSep, xub_StrLen nSepCount, bool bForceSep )
{
    if( bForceSep || (rToken.Len() && rTokenList.Len()) )
        rTokenList.Expand( rTokenList.Len() + nSepCount, cSep );
    rTokenList.Append( rToken );
}

bool ScGlobal::IsQuoted( const String& rString, sal_Unicode cQuote )
{
    return (rString.Len() >= 2) && (rString.GetChar( 0 ) == cQuote) && (rString.GetChar( rString.Len() - 1 ) == cQuote);
}

void ScGlobal::AddQuotes( String& rString, sal_Unicode cQuote, bool bEscapeEmbedded )
{
    if (bEscapeEmbedded)
    {
        sal_Unicode pQ[3];
        pQ[0] = pQ[1] = cQuote;
        pQ[2] = 0;
        String aQuotes( pQ );
        rString.SearchAndReplaceAll( cQuote, aQuotes);
    }
    rString.Insert( cQuote, 0 ).Append( cQuote );
}

void ScGlobal::EraseQuotes( String& rString, sal_Unicode cQuote, bool bUnescapeEmbedded )
{
    if ( IsQuoted( rString, cQuote ) )
    {
        rString.Erase( rString.Len() - 1 ).Erase( 0, 1 );
        if (bUnescapeEmbedded)
        {
            sal_Unicode pQ[3];
            pQ[0] = pQ[1] = cQuote;
            pQ[2] = 0;
            String aQuotes( pQ );
            rString.SearchAndReplaceAll( aQuotes, cQuote);
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
        sal_uInt16 nParCount = pObj1->GetParagraphCount();
        if ( nParCount != pObj2->GetParagraphCount() )
            return sal_False;
        for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
            if ( pObj1->GetText(nPar) != pObj2->GetText(nPar) )
                return sal_False;

        SvMemoryStream  aStream1;
        SvMemoryStream  aStream2;
        pObj1->Store( aStream1 );
        pObj2->Store( aStream2 );
        sal_uLong nSize = aStream1.Tell();
        if ( aStream2.Tell() == nSize )
            if ( !memcmp( aStream1.GetData(), aStream2.GetData(), (sal_uInt16) nSize ) )
                return sal_True;
    }

    return sal_False;
}

void ScGlobal::OpenURL( const String& rURL, const String& rTarget )
{
    //  OpenURL wird immer ueber irgendwelche Umwege durch Mausklicks im GridWindow
    //  aufgerufen, darum stimmen pScActiveViewShell und nScClickMouseModifier.

    SfxStringItem aUrl( SID_FILE_NAME, rURL );
    SfxStringItem aTarget( SID_TARGETNAME, rTarget );

    if ( nScClickMouseModifier & KEY_MOD1 )     // control-click -> into new window
        aTarget.SetValue(
            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("_blank")) );

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

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_False );
    SfxBoolItem aBrowsing( SID_BROWSE, sal_True );

    //  kein SID_SILENT mehr wegen Bug #42525# (war angeblich sowieso falsch)

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
    return MsLangId::isRightToLeft( Application::GetSettings().GetLanguage() );
}

sal_uInt8 ScGlobal::GetDefaultScriptType()
{
    //  Used when text contains only WEAK characters.
    //  Script type of office language is used then (same as GetEditDefaultLanguage,
    //  to get consistent behavior of text in simple cells and EditEngine,
    //  also same as GetAppLanguage() in Writer)

    return (sal_uInt8) SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
}

LanguageType ScGlobal::GetEditDefaultLanguage()
{
    //  used for EditEngine::SetDefaultLanguage

    return Application::GetSettings().GetLanguage();
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
    DBG_ASSERT( rSet.GetItemState( ATTR_LANGUAGE_FORMAT, sal_False ) == SFX_ITEM_DEFAULT,
        "ScGlobal::AddLanguage - language already added");

    const SfxPoolItem* pHardItem;
    if ( rSet.GetItemState( ATTR_VALUE_FORMAT, sal_False, &pHardItem ) == SFX_ITEM_SET )
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





//===================================================================
// class ScFunctionList:
//===================================================================

//===================================================================
//      class ScFuncRes
// fuer temporaere Objekte zum Holen der Resourcen

class ScFuncRes : public Resource
{
public:
    ScFuncRes( ResId&, ScFuncDesc*, bool & rbSuppressed );

private:
    sal_uInt16 GetNum();
};

//--------------------------------------------------------------------

ScFuncRes::ScFuncRes( ResId &aRes, ScFuncDesc* pDesc, bool & rbSuppressed )
 : Resource(aRes)
{
    rbSuppressed = (bool)GetNum();
    pDesc->nCategory = GetNum();
    pDesc->sHelpId = ReadByteStringRes();       //! Hack, see scfuncs.src
    pDesc->nArgCount = GetNum();
    sal_uInt16 nArgs = pDesc->nArgCount;
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    if (nArgs)
    {
        pDesc->pDefArgFlags = new ScFuncDesc::ParameterFlags[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; i++)
        {
            pDesc->pDefArgFlags[i].bOptional = (bool)GetNum();
        }
    }
    // Need to read the value from the resource even if nArgs==0 to advance the
    // resource position pointer, so this can't be in the if(nArgs) block above.
    sal_uInt16 nSuppressed = GetNum();
    if (nSuppressed)
    {
        if (nSuppressed > nArgs)
        {
            DBG_ERROR3( "ScFuncRes: suppressed parameters count mismatch on OpCode %u: suppressed %d > params %d",
                    aRes.GetId(), (int)nSuppressed, (int)nArgs);
            nSuppressed = nArgs;    // sanitize
        }
        for (sal_uInt16 i=0; i < nSuppressed; ++i)
        {
            sal_uInt16 nParam = GetNum();
            if (nParam < nArgs)
            {
                if (pDesc->nArgCount >= VAR_ARGS && nParam == nArgs-1)
                {
                    DBG_ERROR3( "ScFuncRes: VAR_ARGS parameters can't be suppressed, on OpCode %u: param %d == arg %d-1",
                            aRes.GetId(), (int)nParam, (int)nArgs);
                }
                else
                {
                    pDesc->pDefArgFlags[nParam].bSuppress = true;
                    pDesc->bHasSuppressedArgs = true;
                }
            }
            else
            {
                DBG_ERROR3( "ScFuncRes: suppressed parameter exceeds count on OpCode %u: param %d >= args %d",
                        aRes.GetId(), (int)nParam, (int)nArgs);
            }
        }
    }

    pDesc->pFuncName = new String( ScCompiler::GetNativeSymbol( static_cast<OpCode>( aRes.GetId())));
    pDesc->pFuncDesc = new String(ScResId(1));

    if (nArgs)
    {
        pDesc->ppDefArgNames = new String*[nArgs];
        pDesc->ppDefArgDescs = new String*[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; i++)
        {
            pDesc->ppDefArgNames[i] = new String(ScResId(2*(i+1)  ));
            pDesc->ppDefArgDescs[i] = new String(ScResId(2*(i+1)+1));
        }
    }

    FreeResource();
}

//------------------------------------------------------------------------

sal_uInt16 ScFuncRes::GetNum()
{
    return ReadShortRes();
}

//=========================================================================

// um an die protected von Resource ranzukommen
class ScResourcePublisher : public Resource
{
private:
    void            FreeResource() { Resource::FreeResource(); }
public:
        ScResourcePublisher( const ScResId& rId ) : Resource( rId ) {}
        ~ScResourcePublisher() { FreeResource(); }
    sal_Bool            IsAvailableRes( const ResId& rId ) const
                        { return Resource::IsAvailableRes( rId ); }

};


ScFunctionList::ScFunctionList() :
        nMaxFuncNameLen ( 0 )
{
    ScFuncDesc*     pDesc   = NULL;
    xub_StrLen      nStrLen = 0;
    FuncCollection* pFuncColl;
    sal_uInt16 i,j;
    sal_uInt16 nDescBlock[] =
    {
        RID_SC_FUNCTION_DESCRIPTIONS1,
        RID_SC_FUNCTION_DESCRIPTIONS2
    };
    const sal_uInt16 nBlocks = sizeof(nDescBlock) / sizeof(sal_uInt16);

    aFunctionList.Clear();

    for ( sal_uInt16 k = 0; k < nBlocks; k++ )
    {
        ::std::auto_ptr<ScResourcePublisher> pBlock( new ScResourcePublisher( ScResId( nDescBlock[k] ) ) );
        // Browse for all possible OpCodes. This is not the fastest method, but
        // otherwise the sub resources within the resource blocks and the
        // resource blocks themselfs would had to be ordered according to
        // OpCodes, which is utopian..
        for (i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; i++)
        {
            ScResId aRes(i);
            aRes.SetRT(RSC_RESOURCE);
            // Sub resource of OpCode available?
            if (pBlock->IsAvailableRes(aRes))
            {
                pDesc = new ScFuncDesc;
                bool bSuppressed = false;
                ScFuncRes aSubRes( aRes, pDesc, bSuppressed);
                // Instead of dealing with this exceptional case at 1001 places
                // we simply don't add an entirely suppressed function to the
                // list and delete it.
                if (bSuppressed)
                    delete pDesc;
                else
                {
                    pDesc->nFIndex = i;
                    aFunctionList.Insert( pDesc, LIST_APPEND );

                    nStrLen = (*(pDesc->pFuncName)).Len();
                    if (nStrLen > nMaxFuncNameLen)
                        nMaxFuncNameLen = nStrLen;
                }
            }
        }
    }

    sal_uInt16 nNextId = SC_OPCODE_LAST_OPCODE_ID + 1;      // FuncID for AddIn functions

    // Auswertung AddIn-Liste
    String aDefArgNameValue(RTL_CONSTASCII_STRINGPARAM("value"));
    String aDefArgNameString(RTL_CONSTASCII_STRINGPARAM("string"));
    String aDefArgNameValues(RTL_CONSTASCII_STRINGPARAM("values"));
    String aDefArgNameStrings(RTL_CONSTASCII_STRINGPARAM("strings"));
    String aDefArgNameCells(RTL_CONSTASCII_STRINGPARAM("cells"));
    String aDefArgNameNone(RTL_CONSTASCII_STRINGPARAM("none"));
    String aDefArgDescValue(RTL_CONSTASCII_STRINGPARAM("a value"));
    String aDefArgDescString(RTL_CONSTASCII_STRINGPARAM("a string"));
    String aDefArgDescValues(RTL_CONSTASCII_STRINGPARAM("array of values"));
    String aDefArgDescStrings(RTL_CONSTASCII_STRINGPARAM("array of strings"));
    String aDefArgDescCells(RTL_CONSTASCII_STRINGPARAM("range of cells"));
    String aDefArgDescNone(RTL_CONSTASCII_STRINGPARAM("none"));
    String aArgName, aArgDesc;
    pFuncColl = ScGlobal::GetFuncCollection();
    for (i = 0; i < pFuncColl->GetCount(); i++)
    {
        pDesc = new ScFuncDesc;
        FuncData *pAddInFuncData = (FuncData*)pFuncColl->At(i);
        sal_uInt16 nArgs = pAddInFuncData->GetParamCount() - 1;
        pAddInFuncData->GetParamDesc( aArgName, aArgDesc, 0 );
          pDesc->nFIndex     = nNextId++;               //  ??? OpCode vergeben
          pDesc->nCategory   = ID_FUNCTION_GRP_ADDINS;
          pDesc->pFuncName   = new String(pAddInFuncData->GetInternalName());
          pDesc->pFuncName->ToUpperAscii();
          pDesc->pFuncDesc   = new String( aArgDesc );
        *(pDesc->pFuncDesc) += '\n';
          pDesc->pFuncDesc->AppendAscii(RTL_CONSTASCII_STRINGPARAM( "( AddIn: " ));
        *(pDesc->pFuncDesc) += pAddInFuncData->GetModuleName();
          pDesc->pFuncDesc->AppendAscii(RTL_CONSTASCII_STRINGPARAM( " )" ));
          pDesc->nArgCount   = nArgs;
        if (nArgs)
        {
            pDesc->pDefArgFlags  = new ScFuncDesc::ParameterFlags[nArgs];
            pDesc->ppDefArgNames = new String*[nArgs];
            pDesc->ppDefArgDescs = new String*[nArgs];
            for (j = 0; j < nArgs; j++)
            {
                pDesc->pDefArgFlags[j].bOptional = false;
                pDesc->pDefArgFlags[j].bSuppress = false;
                pAddInFuncData->GetParamDesc( aArgName, aArgDesc, j+1 );
                if ( aArgName.Len() )
                    pDesc->ppDefArgNames[j] = new String( aArgName );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameValue );
                            break;
                        case PTR_STRING:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameCells );
                            break;
                        default:
                            pDesc->ppDefArgNames[j] = new String( aDefArgNameNone );
                            break;
                    }
                }
                if ( aArgDesc.Len() )
                    pDesc->ppDefArgDescs[j] = new String( aArgDesc );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescValue );
                            break;
                        case PTR_STRING:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescCells );
                            break;
                        default:
                            pDesc->ppDefArgDescs[j] = new String( aDefArgDescNone );
                            break;
                    }
                }
            }
        }
//      pDesc->nHelpId    = 0;

        aFunctionList.Insert(pDesc, LIST_APPEND);
        nStrLen = (*(pDesc->pFuncName)).Len();
        if ( nStrLen > nMaxFuncNameLen)
            nMaxFuncNameLen = nStrLen;
    }

    //  StarOne AddIns

    ScUnoAddInCollection* pUnoAddIns = ScGlobal::GetAddInCollection();
    long nUnoCount = pUnoAddIns->GetFuncCount();
    for (long nFunc=0; nFunc<nUnoCount; nFunc++)
    {
        pDesc = new ScFuncDesc;
        pDesc->nFIndex = nNextId++;

        if ( pUnoAddIns->FillFunctionDesc( nFunc, *pDesc ) )
        {
            aFunctionList.Insert(pDesc, LIST_APPEND);
            nStrLen = (*(pDesc->pFuncName)).Len();
            if (nStrLen > nMaxFuncNameLen)
                nMaxFuncNameLen = nStrLen;
        }
        else
            delete pDesc;
    }
}

//------------------------------------------------------------------------

ScFunctionList::~ScFunctionList()
{
    const ScFuncDesc* pDesc = First();
    while (pDesc)
    {
        delete pDesc;
        pDesc = Next();
    }
}


//========================================================================
// class ScFuncDesc:

ScFuncDesc::ScFuncDesc() :
        pFuncName       (NULL),
        pFuncDesc       (NULL),
        ppDefArgNames   (NULL),
        ppDefArgDescs   (NULL),
        pDefArgFlags    (NULL),
        nFIndex         (0),
        nCategory       (0),
        nArgCount       (0),
        bIncomplete     (false),
        bHasSuppressedArgs(false)
{}

//------------------------------------------------------------------------

ScFuncDesc::~ScFuncDesc()
{
    Clear();
}

//------------------------------------------------------------------------

void ScFuncDesc::Clear()
{
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= VAR_ARGS) nArgs -= VAR_ARGS-1;
    if (nArgs)
    {
        for (sal_uInt16 i=0; i<nArgs; i++ )
        {
            delete ppDefArgNames[i];
            delete ppDefArgDescs[i];
        }
        delete [] ppDefArgNames;
        delete [] ppDefArgDescs;
        delete [] pDefArgFlags;
    }
    nArgCount = 0;
    ppDefArgNames = NULL;
    ppDefArgDescs = NULL;
    pDefArgFlags = NULL;

    delete pFuncName;
    pFuncName = NULL;

    delete pFuncDesc;
    pFuncDesc = NULL;

    nFIndex = 0;
    nCategory = 0;
    sHelpId = "";
    bIncomplete = false;
    bHasSuppressedArgs = false;
}

//------------------------------------------------------------------------

String ScFuncDesc::GetParamList() const
{
    const String& sep = ScCompiler::GetNativeSymbol(ocSep);

    String aSig;

    if ( nArgCount > 0 )
    {
        if ( nArgCount < VAR_ARGS )
        {
            sal_uInt16 nLastSuppressed = nArgCount;
            sal_uInt16 nLastAdded = nArgCount;
            for ( sal_uInt16 i=0; i<nArgCount; i++ )
            {
                if (pDefArgFlags[i].bSuppress)
                    nLastSuppressed = i;
                else
                {
                    nLastAdded = i;
                    aSig += *(ppDefArgNames[i]);
                    if ( i != nArgCount-1 )
                    {
                        aSig.Append(sep);
                        aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " " ));
                    }
                }
            }
            // If only suppressed parameters follow the last added parameter,
            // remove one "; "
            if (nLastSuppressed < nArgCount && nLastAdded < nLastSuppressed &&
                    aSig.Len() >= 2)
                aSig.Erase( aSig.Len() - 2 );
        }
        else
        {
            sal_uInt16 nFix = nArgCount - VAR_ARGS;
            for ( sal_uInt16 nArg = 0; nArg < nFix; nArg++ )
            {
                if (!pDefArgFlags[nArg].bSuppress)
                {
                    aSig += *(ppDefArgNames[nArg]);
                    aSig.Append(sep);
                    aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " " ));
                }
            }
            /* NOTE: Currently there are no suppressed var args parameters. If
             * there were, we'd have to cope with it here and above for the fix
             * parameters. For now parameters are always added, so no special
             * treatment of a trailing "; " necessary. */
            aSig += *(ppDefArgNames[nFix]);
            aSig += '1';
            aSig.Append(sep);
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " " ));
            aSig += *(ppDefArgNames[nFix]);
            aSig += '2';
            aSig.Append(sep);
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ... " ));
        }
    }

    return aSig;
}

//------------------------------------------------------------------------

String ScFuncDesc::GetSignature() const
{
    String aSig;

    if(pFuncName)
    {
        aSig = *pFuncName;

        String aParamList( GetParamList() );
        if( aParamList.Len() )
        {
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "( " ));
            aSig.Append( aParamList );
            // U+00A0 (NBSP) prevents automatic line break
            aSig.Append( static_cast< sal_Unicode >(0xA0) ).Append( ')' );
        }
        else
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
    }
    return aSig;
}

//------------------------------------------------------------------------

::rtl::OUString ScFuncDesc::getFormula( const ::std::vector< ::rtl::OUString >& _aArguments ) const
{
    const String& sep = ScCompiler::GetNativeSymbol(ocSep);

    ::rtl::OUStringBuffer aFormula;

    if(pFuncName)
    {
        aFormula.append( *pFuncName );

        aFormula.appendAscii( "(" );
        ::std::vector< ::rtl::OUString >::const_iterator aIter = _aArguments.begin();
        ::std::vector< ::rtl::OUString >::const_iterator aEnd = _aArguments.end();

        if ( nArgCount > 0 && aIter != aEnd )
        {
            sal_Bool bLastArg = ( aIter->getLength() == 0 );

            while( aIter != aEnd && !bLastArg )
            {
                aFormula.append( *(aIter) );
                if ( aIter != (aEnd-1) )
                {
                    bLastArg = !( (aIter+1)->getLength() > 0 );
                    if ( !bLastArg )
                        aFormula.append( sep );
                }

                ++aIter;
            }
        }

        aFormula.appendAscii( ")" );
    }
    return aFormula.makeStringAndClear();
}

//------------------------------------------------------------------------

sal_uInt16 ScFuncDesc::GetSuppressedArgCount() const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
        return nArgCount;

    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    sal_uInt16 nCount = nArgs;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (pDefArgFlags[i].bSuppress)
            --nCount;
    }
    if (nArgCount >= VAR_ARGS)
        nCount += VAR_ARGS - 1;
    return nCount;
}

//------------------------------------------------------------------------

::rtl::OUString ScFuncDesc::getFunctionName() const
{
    ::rtl::OUString sRet;
    if ( pFuncName )
        sRet = *pFuncName;
    return sRet;
}
// -----------------------------------------------------------------------------
const formula::IFunctionCategory* ScFuncDesc::getCategory() const
{
    return ScGlobal::GetStarCalcFunctionMgr()->getCategory(nCategory);
}
// -----------------------------------------------------------------------------
::rtl::OUString ScFuncDesc::getDescription() const
{
    ::rtl::OUString sRet;
    if ( pFuncDesc )
        sRet = *pFuncDesc;
    return sRet;
}
// -----------------------------------------------------------------------------
// GetSuppressedArgCount
xub_StrLen ScFuncDesc::getSuppressedArgumentCount() const
{
    return GetSuppressedArgCount();
}
// -----------------------------------------------------------------------------
//
void ScFuncDesc::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
    {
        _rArguments.resize( nArgCount);
        ::std::iota( _rArguments.begin(), _rArguments.end(), 0);
    }

    _rArguments.reserve( nArgCount);
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (!pDefArgFlags[i].bSuppress)
            _rArguments.push_back(i);
    }
}
// -----------------------------------------------------------------------------
void ScFuncDesc::initArgumentInfo()  const
{
    // get the full argument description
    // (add-in has to be instantiated to get the type information)

    if ( bIncomplete && pFuncName )
    {
        ScUnoAddInCollection& rAddIns = *ScGlobal::GetAddInCollection();
        String aIntName = rAddIns.FindFunction( *pFuncName, sal_True );         // pFuncName is upper-case

        if ( aIntName.Len() )
        {
            // GetFuncData with bComplete=true loads the component and updates
            // the global function list if needed.

            rAddIns.GetFuncData( aIntName, true );
        }

        if ( bIncomplete )
        {
            DBG_ERRORFILE( "couldn't initialize add-in function" );
            const_cast<ScFuncDesc*>(this)->bIncomplete = sal_False;         // even if there was an error, don't try again
        }
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString ScFuncDesc::getSignature() const
{
    return GetSignature();
}
// -----------------------------------------------------------------------------
rtl::OString ScFuncDesc::getHelpId() const
{
    return sHelpId;
}
// -----------------------------------------------------------------------------

// parameter
sal_uInt32 ScFuncDesc::getParameterCount() const
{
    return nArgCount;
}
// -----------------------------------------------------------------------------
::rtl::OUString ScFuncDesc::getParameterName(sal_uInt32 _nPos) const
{
    return *(ppDefArgNames[_nPos]);
}
// -----------------------------------------------------------------------------
::rtl::OUString ScFuncDesc::getParameterDescription(sal_uInt32 _nPos) const
{
    return *(ppDefArgDescs[_nPos]);
}
// -----------------------------------------------------------------------------
bool ScFuncDesc::isParameterOptional(sal_uInt32 _nPos) const
{
    return pDefArgFlags[_nPos].bOptional;
}
// -----------------------------------------------------------------------------
//========================================================================
// class ScFunctionMgr:

ScFunctionMgr::ScFunctionMgr()
    :   pFuncList   ( ScGlobal::GetStarCalcFunctionList() ),
        pCurCatList ( NULL )
{
    DBG_ASSERT( pFuncList, "Funktionsliste nicht gefunden." );
    sal_uLong       nCount  = pFuncList->GetCount();
    const ScFuncDesc*   pDesc;
    List*       pRootList;
    sal_uLong       n;

    for ( sal_uInt16 i=0; i<MAX_FUNCCAT; i++ )                  // Kategorie-Listen erstellen
        aCatLists[i] = new List;

    pRootList = aCatLists[0];                               // Gesamtliste ("Alle") erstellen
    CollatorWrapper* pCaseCollator = ScGlobal::GetCaseCollator();
    for ( n=0; n<nCount; n++ )
    {
        sal_uLong nTmpCnt=0;
        pDesc = pFuncList->GetFunction(n);
        for (nTmpCnt = 0; nTmpCnt < n; nTmpCnt++)
        {
            // ist zwar case-sensitiv, aber Umlaute muessen richtig einsortiert werden

            const ScFuncDesc*   pTmpDesc = (const ScFuncDesc*)pRootList->GetObject(nTmpCnt);
            if ( pCaseCollator->compareString(*pDesc->pFuncName, *pTmpDesc->pFuncName ) == COMPARE_LESS )
                break;
        }
        pRootList->Insert((void*)pDesc, nTmpCnt);                   // Einsortieren
    }

    for ( n=0; n<nCount; n++ )                              // in Gruppenlisten kopieren
    {
        pDesc = (const ScFuncDesc*)pRootList->GetObject(n);
        DBG_ASSERT((pDesc->nCategory) < MAX_FUNCCAT, "Unbekannte Kategorie");
        if ((pDesc->nCategory) < MAX_FUNCCAT)
            aCatLists[pDesc->nCategory]->Insert((void*)pDesc, LIST_APPEND);
    }
}

//------------------------------------------------------------------------

ScFunctionMgr::~ScFunctionMgr()
{
    for (sal_uInt16 i = 0; i < MAX_FUNCCAT; i++)
        delete aCatLists[i];
//  delete pFuncList;       // Macht spaeter die App
}

//------------------------------------------------------------------------

const ScFuncDesc* ScFunctionMgr::Get( const String& rFName ) const
{
    const ScFuncDesc*   pDesc = NULL;
    if (rFName.Len() <= pFuncList->GetMaxFuncNameLen())
        for (pDesc = First(0); pDesc; pDesc = Next())
            if (rFName.EqualsIgnoreCaseAscii(*(pDesc->pFuncName)))
                break;
    return pDesc;
}

//------------------------------------------------------------------------

const ScFuncDesc* ScFunctionMgr::Get( sal_uInt16 nFIndex ) const
{
    const ScFuncDesc*   pDesc;
    for (pDesc = First(0); pDesc; pDesc = Next())
        if (pDesc->nFIndex == nFIndex)
            break;
    return pDesc;
}

//------------------------------------------------------------------------

const ScFuncDesc*   ScFunctionMgr::First( sal_uInt16 nCategory ) const
{
    DBG_ASSERT( nCategory < MAX_FUNCCAT, "Unbekannte Kategorie" );

    if ( nCategory < MAX_FUNCCAT )
    {
        pCurCatList = aCatLists[nCategory];
        return (const ScFuncDesc*)pCurCatList->First();
    }
    else
    {
        pCurCatList = NULL;
        return NULL;
    }
}

//------------------------------------------------------------------------

const ScFuncDesc* ScFunctionMgr::Next() const
{
    if ( pCurCatList )
        return (const ScFuncDesc*)pCurCatList->Next();
    else
        return NULL;
}
sal_uInt32 ScFunctionMgr::getCount() const
{
    return MAX_FUNCCAT - 1;
}
const formula::IFunctionCategory* ScFunctionMgr::getCategory(sal_uInt32 nCategory) const
{
    formula::IFunctionCategory* pRet = NULL;
    if ( nCategory < (MAX_FUNCCAT-1) )
    {
         pRet = new ScFunctionCategory(const_cast<ScFunctionMgr*>(this),aCatLists[nCategory+1],nCategory); // aCatLists[0] is "all"
    }
    return pRet;
}
// -----------------------------------------------------------------------------
const formula::IFunctionDescription* ScFunctionMgr::getFunctionByName(const ::rtl::OUString& _sFunctionName) const
{
    return Get(_sFunctionName);
}
// -----------------------------------------------------------------------------
void ScFunctionMgr::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const
{
#define LRU_MAX 10

    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = Min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
    sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();

    if ( pLRUListIds )
    {
        for ( sal_uInt16 i=0; i<nLRUFuncCount; i++ )
            _rLastRUFunctions.push_back( Get( pLRUListIds[i] ) );
    }
}
// -----------------------------------------------------------------------------
String ScFunctionMgr::GetCategoryName(sal_uInt32 _nCategoryNumber )
{
    if ( _nCategoryNumber > SC_FUNCGROUP_COUNT )
    {
        DBG_ERROR("Invalid category number!");
        return String();
    } // if ( _nCategoryNumber >= SC_FUNCGROUP_COUNT )

    ::std::auto_ptr<ScResourcePublisher> pCategories( new ScResourcePublisher( ScResId( RID_FUNCTION_CATEGORIES ) ) );
    return String(ScResId((sal_uInt16)_nCategoryNumber));
}
sal_Unicode ScFunctionMgr::getSingleToken(const formula::IFunctionManager::EToken _eToken) const
{
    switch(_eToken)
    {
        case eOk:
            return ScCompiler::GetNativeSymbol(ocOpen).GetChar(0);
        case eClose:
            return ScCompiler::GetNativeSymbol(ocClose).GetChar(0);
        case eSep:
            return ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
        case eArrayOpen:
            return ScCompiler::GetNativeSymbol(ocArrayOpen).GetChar(0);
        case eArrayClose:
            return ScCompiler::GetNativeSymbol(ocArrayClose).GetChar(0);
    } // switch(_eToken)
    return 0;
}
// -----------------------------------------------------------------------------
sal_uInt32 ScFunctionCategory::getCount() const
{
    return m_pCategory->Count();
}
// -----------------------------------------------------------------------------
const formula::IFunctionManager* ScFunctionCategory::getFunctionManager() const
{
    return m_pMgr;
}
// -----------------------------------------------------------------------------
::rtl::OUString ScFunctionCategory::getName() const
{
    if ( !m_sName.getLength() )
        m_sName = ScFunctionMgr::GetCategoryName(m_nCategory+1);
    return m_sName;
}
// -----------------------------------------------------------------------------
const formula::IFunctionDescription* ScFunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    const ScFuncDesc*   pDesc = NULL;
    sal_uInt32 i = 0;
    for (pDesc = (const ScFuncDesc*)m_pCategory->First(); i < _nPos &&  pDesc; pDesc = (const ScFuncDesc*)m_pCategory->Next(),++i)
        ;
    return pDesc;
}
// -----------------------------------------------------------------------------
sal_uInt32 ScFunctionCategory::getNumber() const
{
    return m_nCategory;
}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------

utl::TransliterationWrapper* ScGlobal::GetpTransliteration() //add by CHINA001
{
    if ( !pTransliteration )
    {
        const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
        pTransliteration = new ::utl::TransliterationWrapper(
            ::comphelper::getProcessServiceFactory(), SC_TRANSLITERATION_IGNORECASE );
        pTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    }
    DBG_ASSERT(
        pTransliteration,
        "ScGlobal::GetpTransliteration() called before ScGlobal::Init()");
    return pTransliteration;
}

const LocaleDataWrapper* ScGlobal::GetpLocaleData()
{
    DBG_ASSERT(
        pLocaleData,
        "ScGlobal::GetpLocaleData() called before ScGlobal::Init()");
    return pLocaleData;
}
CalendarWrapper*     ScGlobal::GetCalendar()
{
    if ( !pCalendar )
    {
        pCalendar = new CalendarWrapper( ::comphelper::getProcessServiceFactory() );
        pCalendar->loadDefaultCalendar( *GetLocale() );
    }
    return pCalendar;
}
CollatorWrapper*        ScGlobal::GetCollator()
{
    if ( !pCollator )
    {
        pCollator = new CollatorWrapper( ::comphelper::getProcessServiceFactory() );
        pCollator->loadDefaultCollator( *GetLocale(), SC_COLLATOR_IGNORES );
    } // if ( !pCollator )
    return pCollator;
}
CollatorWrapper*        ScGlobal::GetCaseCollator()
{
    if ( !pCaseCollator )
    {
        pCaseCollator = new CollatorWrapper( ::comphelper::getProcessServiceFactory() );
        pCaseCollator->loadDefaultCollator( *GetLocale(), 0 );
    } // if ( !pCaseCollator )
    return pCaseCollator;
}
::utl::TransliterationWrapper* ScGlobal::GetCaseTransliteration()
{
    if ( !pCaseTransliteration )
    {
        const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
        pCaseTransliteration = new ::utl::TransliterationWrapper(::comphelper::getProcessServiceFactory(), SC_TRANSLITERATION_CASESENSE );
        pCaseTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    } // if ( !pCaseTransliteration )
    return pCaseTransliteration;
}
IntlWrapper*         ScGlobal::GetScIntlWrapper()
{
    if ( !pScIntlWrapper )
    {
        pScIntlWrapper = new IntlWrapper( ::comphelper::getProcessServiceFactory(), *GetLocale() );
    }
    return pScIntlWrapper;
}
::com::sun::star::lang::Locale*     ScGlobal::GetLocale()
{
    if ( !pLocale )
    {
        pLocale = new ::com::sun::star::lang::Locale( Application::GetSettings().GetLocale());
    }
    return pLocale;
}

