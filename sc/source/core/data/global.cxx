/*************************************************************************
 *
 *  $RCSfile: global.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:55:34 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editobj.hxx>
#include <svx/scripttypeitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/langitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/stritem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <tools/rcid.h>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>


#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif
#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/intlwrapper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

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
#include "globstr.hrc"
#include "scfuncs.hrc"
#include "sc.hrc"

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

USHORT          ScGlobal::nDefFontHeight        = 240;
USHORT          ScGlobal::nStdRowHeight         = 257;

long            ScGlobal::nLastRowHeightExtra   = 0;
long            ScGlobal::nLastColWidthExtra    = STD_EXTRA_WIDTH;

static USHORT nPPTZoom = 0;     // ScreenZoom used to determine nScreenPPTX/Y


// ... oder so?

BOOL bOderSo;

class SfxViewShell;
SfxViewShell* pScActiveViewShell = NULL;            //! als Member !!!!!
USHORT nScClickMouseModifier = 0;                   //! dito
USHORT nScFillModeMouseModifier = 0;                //! dito

// Hack: ScGlobal::GetUserList() muss InitAppOptions in der UI aufrufen,
//       damit UserList aus Cfg geladen wird

void global_InitAppOptions();

//========================================================================
//
//      statische Funktionen
//
//========================================================================

BOOL ScGlobal::HasAttrChanged( const SfxItemSet&  rNewAttrs,
                               const SfxItemSet&  rOldAttrs,
                               const USHORT       nWhich )
{
    BOOL                bInvalidate = FALSE;
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

        bInvalidate = (rNewItem != rOldItem);
    }

    return bInvalidate;
}

ULONG ScGlobal::GetStandardFormat( SvNumberFormatter& rFormatter,
        ULONG nFormat, short nType )
{
    const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
    if ( pFormat )
        return rFormatter.GetStandardFormat( nFormat, nType, pFormat->GetLanguage() );
    return rFormatter.GetStandardFormat( nType, eLnge );
}

ULONG ScGlobal::GetStandardFormat( double fNumber, SvNumberFormatter& rFormatter,
        ULONG nFormat, short nType )
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

BOOL ScGlobal::CheckWidthInvalidate( BOOL& bNumFormatChanged,
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

const String& ScGlobal::GetRscString( USHORT nIndex )
{
    DBG_ASSERT( nIndex <= STR_COUNT,
        "-ScGlobal::GetRscString(): Index zu gross!");
    if( !ppRscString[ nIndex ] )
    {
        ppRscString[ nIndex ] =
            new String( ScRscStrLoader( RID_GLOBSTR, nIndex ).GetString() );
    }
    return *ppRscString[ nIndex ];
}

String ScGlobal::GetErrorString(USHORT nErrNumber)
{
    String sResStr;
    switch (nErrNumber)
    {
        case NOVALUE     : nErrNumber = STR_NV_STR; break;
        case errNoRef    : nErrNumber = STR_NO_REF_TABLE; break;
        case errNoName   : nErrNumber = STR_NO_NAME_REF; break;
        case errNoAddin  : nErrNumber = STR_NO_ADDIN; break;
        case errNoMacro  : nErrNumber = STR_NO_MACRO; break;
        case errDoubleRef:
        case errNoValue  : nErrNumber = STR_NO_VALUE; break;

        default          : sResStr = GetRscString(STR_ERROR_STR);
                           sResStr += String::CreateFromInt32( nErrNumber );
                           nErrNumber = 0;
                           break;
    }
    if( nErrNumber )
        sResStr = GetRscString( nErrNumber );
    return sResStr;
}

String ScGlobal::GetLongErrorString(USHORT nErrNumber)
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
        case errNoValue:
            nErrNumber = STR_LONG_ERR_NO_VALUE;
        break;
        case NOVALUE:
            nErrNumber = STR_LONG_ERR_NV;
        break;
        default:
            nErrNumber = STR_ERROR_STR;
        break;
    }
    String aRes( GetRscString( nErrNumber ) );
    if( bOderSo )
    {
        String aOderSo( GetRscString( STR_ODER_SO ) );
        aOderSo.SearchAndReplace( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("%s")), aRes );
        aRes = aOderSo;
    }
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
    String aLanguage, aCountry;
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
    ConvertLanguageToIsoNames( eOfficeLanguage, aLanguage, aCountry );
    pLocale = new ::com::sun::star::lang::Locale( aLanguage, aCountry, EMPTY_STRING );
    pSysLocale = new SvtSysLocale;
    pCharClass = pSysLocale->GetCharClassPtr();
    pLocaleData = pSysLocale->GetLocaleDataPtr();
    pCalendar = new CalendarWrapper( ::comphelper::getProcessServiceFactory() );
    pCalendar->loadDefaultCalendar( *pLocale );
    pCollator = new CollatorWrapper( ::comphelper::getProcessServiceFactory() );
    pCollator->loadDefaultCollator( *pLocale, SC_COLLATOR_IGNORES );
    pCaseCollator = new CollatorWrapper( ::comphelper::getProcessServiceFactory() );
    pCaseCollator->loadDefaultCollator( *pLocale, 0 );
    pTransliteration = new ::utl::TransliterationWrapper(
        ::comphelper::getProcessServiceFactory(), SC_TRANSLITERATION_IGNORECASE );
    pTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    pCaseTransliteration = new ::utl::TransliterationWrapper(
        ::comphelper::getProcessServiceFactory(), SC_TRANSLITERATION_CASESENSE );
    pCaseTransliteration->loadModuleIfNeeded( eOfficeLanguage );
    pScIntlWrapper = new IntlWrapper( ::comphelper::getProcessServiceFactory(), *pLocale );

    ppRscString = new String *[ STR_COUNT+1 ];
    for( USHORT nC = 0 ; nC <= STR_COUNT ; nC++ ) ppRscString[ nC ] = NULL;

    pEmptyBrushItem = new SvxBrushItem( Color( COL_TRANSPARENT ) );
    pButtonBrushItem = new SvxBrushItem( Color() );
    pEmbeddedBrushItem = new SvxBrushItem( Color( COL_LIGHTCYAN ) );
    pProtectedBrushItem = new SvxBrushItem( Color( COL_LIGHTGRAY ) );

    UpdatePPT(NULL);
    ScCompiler::Init();
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
    USHORT nCurrentZoom = Application::GetSettings().GetStyleSettings().GetScreenZoom();
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
    nDefFontHeight = (USHORT) aVirtWindow.PixelToLogic(Size(0, aVirtWindow.GetTextHeight()),
                                MAP_TWIP).Height();

    const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);

    nStdRowHeight = (USHORT) ( nDefFontHeight +
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

    for( USHORT nC = 0 ; nC < STR_COUNT ; nC++ )
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

void ScGlobal::AddQuotes( String& rString, sal_Unicode cQuote )
{
    rString.Insert( cQuote, 0 ).Append( cQuote );
}

void ScGlobal::EraseQuotes( String& rString, sal_Unicode cQuote )
{
    if( IsQuoted( rString ) )
        rString.Erase( rString.Len() - 1 ).Erase( 0, 1 );
}

//------------------------------------------------------------------------

BOOL ScGlobal::EETextObjEqual( const EditTextObject* pObj1,
                               const EditTextObject* pObj2 )
{
    if ( pObj1 == pObj2 )               // both empty or the same object
        return TRUE;

    if ( pObj1 && pObj2 )
    {
        //  first test for equal text content
        USHORT nParCount = pObj1->GetParagraphCount();
        if ( nParCount != pObj2->GetParagraphCount() )
            return FALSE;
        for (USHORT nPar=0; nPar<nParCount; nPar++)
            if ( pObj1->GetText(nPar) != pObj2->GetText(nPar) )
                return FALSE;

        SvMemoryStream  aStream1;
        SvMemoryStream  aStream2;
        pObj1->Store( aStream1 );
        pObj2->Store( aStream2 );
        ULONG nSize = aStream1.Tell();
        if ( aStream2.Tell() == nSize )
            if ( !memcmp( aStream1.GetData(), aStream2.GetData(), (USHORT) nSize ) )
                return TRUE;
    }

    return FALSE;
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

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, FALSE );
    SfxBoolItem aBrowsing( SID_BROWSE, TRUE );

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

BOOL ScGlobal::IsSystemRTL()
{
    BOOL bRet = FALSE;

    switch ( Application::GetSettings().GetLanguage() )
    {
        // same languages as in GetDefaultFrameDirection in Writer (poolfmt.cxx)
        case LANGUAGE_ARABIC:
        case LANGUAGE_ARABIC_SAUDI_ARABIA:
        case LANGUAGE_ARABIC_IRAQ:
        case LANGUAGE_ARABIC_EGYPT:
        case LANGUAGE_ARABIC_LIBYA:
        case LANGUAGE_ARABIC_ALGERIA:
        case LANGUAGE_ARABIC_MOROCCO:
        case LANGUAGE_ARABIC_TUNISIA:
        case LANGUAGE_ARABIC_OMAN:
        case LANGUAGE_ARABIC_YEMEN:
        case LANGUAGE_ARABIC_SYRIA:
        case LANGUAGE_ARABIC_JORDAN:
        case LANGUAGE_ARABIC_LEBANON:
        case LANGUAGE_ARABIC_KUWAIT:
        case LANGUAGE_ARABIC_UAE:
        case LANGUAGE_ARABIC_BAHRAIN:
        case LANGUAGE_ARABIC_QATAR:
        case LANGUAGE_HEBREW:
        case LANGUAGE_URDU:
            bRet = TRUE;
            break;
    }

    return bRet;
}

BYTE ScGlobal::GetDefaultScriptType()
{
    //  Used when text contains only WEAK characters.
    //  Script type of office language is used then (same as GetEditDefaultLanguage,
    //  to get consistent behavior of text in simple cells and EditEngine,
    //  also same as GetAppLanguage() in Writer)

    return (BYTE) SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
}

LanguageType ScGlobal::GetEditDefaultLanguage()
{
    //  used for EditEngine::SetDefaultLanguage

    return Application::GetSettings().GetLanguage();
}

USHORT ScGlobal::GetScriptedWhichID( BYTE nScriptType, USHORT nWhich )
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
    DBG_ASSERT( rSet.GetItemState( ATTR_LANGUAGE_FORMAT, FALSE ) == SFX_ITEM_DEFAULT,
        "ScGlobal::AddLanguage - language already added");

    const SfxPoolItem* pHardItem;
    if ( rSet.GetItemState( ATTR_VALUE_FORMAT, FALSE, &pHardItem ) == SFX_ITEM_SET )
    {
        const SvNumberformat* pHardFormat = rFormatter.GetEntry(
            ((const SfxUInt32Item*)pHardItem)->GetValue() );

        ULONG nParentFmt = 0;   // pool default
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
//  class ScFormulaUtil - statische Methoden
//===================================================================

ScFuncDesc aDefaultFuncDesc;

const ScFuncDesc* ScFormulaUtil::GetDefaultFuncDesc()
{
    return &aDefaultFuncDesc;
}

BOOL ScFormulaUtil::GetNextFunc( const String&  rFormula,
                                 BOOL           bBack,
                                 xub_StrLen&    rFStart,   // Ein- und Ausgabe
                                 xub_StrLen*    pFEnd,     // = NULL
                                 const ScFuncDesc** ppFDesc,   // = NULL
                                 String***      pppArgs )  // = NULL
{
    BOOL        bFound = FALSE;
    xub_StrLen  nOldStart = rFStart;
    String      aFname;

    rFStart = GetFunctionStart( rFormula, rFStart, bBack, ppFDesc ? &aFname : NULL );
    bFound  = ( rFStart != FUNC_NOTFOUND );

    if ( bFound )
    {
        if ( pFEnd )
            *pFEnd = GetFunctionEnd( rFormula, rFStart );

        if ( ppFDesc )
        {
            ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
            *ppFDesc = pFuncMgr->Get( aFname );
            if ( *ppFDesc )
            {
                if (pppArgs)
                    *pppArgs = GetArgStrings( rFormula, rFStart, (*ppFDesc)->nArgCount );
            }
            else
            {
                *ppFDesc = &aDefaultFuncDesc;
            }
        }
    }
    else
        rFStart = nOldStart;

    return bFound;
}

//------------------------------------------------------------------------

void ScFormulaUtil::FillArgStrings( const String&   rFormula,
                                    xub_StrLen      nFuncPos,
                                    USHORT          nArgs,
                                    String**        aArgArr )
{
    if ( !aArgArr ) return;

    xub_StrLen  nStart  = 0;
    xub_StrLen  nEnd    = 0;
    USHORT      i;
    BOOL        bLast   = FALSE;

    for ( i=0; i<nArgs && !bLast; i++ )
    {
        nStart = GetArgStart( rFormula, nFuncPos, i );

        if ( i+1<nArgs ) // letztes Argument?
        {
            nEnd = GetArgStart( rFormula, nFuncPos, i+1 );

            if ( nEnd != nStart )
                aArgArr[i] = new String( rFormula.Copy( nStart, nEnd-1-nStart ) );
            else
                aArgArr[i] = new String, bLast = TRUE;
        }
        else
        {
            nEnd = GetFunctionEnd( rFormula, nFuncPos )-1;
            if ( nStart < nEnd )
                aArgArr[i] = new String( rFormula.Copy( nStart, nEnd-nStart ) );
            else
                aArgArr[i] = new String;
        }
    }

    if ( bLast )
        for ( ; i<nArgs; i++ )
            aArgArr[i] = new String;
}

//------------------------------------------------------------------------

String** ScFormulaUtil::GetArgStrings( const String& rFormula,
                                       xub_StrLen nFuncPos,
                                       USHORT nArgs )
{
    String** aArgArr = NULL;
    if (nArgs)
    {
        aArgArr = new String*[nArgs];
        FillArgStrings( rFormula, nFuncPos, nArgs, aArgArr );
    }
    return aArgArr;
}

//------------------------------------------------------------------------

inline BOOL IsFormulaText( const String& rStr, xub_StrLen nPos )
{
    if( ScGlobal::pCharClass->isLetterNumeric( rStr, nPos ) )
        return TRUE;
    else
    {   // In internationalized versions function names may contain a dot
        //  and in every version also an underscore... ;-)
        sal_Unicode c = rStr.GetChar(nPos);
        return c == '.' || c == '_';
    }

}

xub_StrLen ScFormulaUtil::GetFunctionStart( const String&   rFormula,
                                        xub_StrLen      nStart,
                                        BOOL            bBack,
                                        String*         pFuncName )
{
    xub_StrLen nStrLen = rFormula.Len();

    if ( nStrLen < nStart )
        return nStart;

    xub_StrLen  nFStart = FUNC_NOTFOUND;
    xub_StrLen  nParPos = nStart;

    BOOL bRepeat, bFound;
    do
    {
        bFound  = FALSE;
        bRepeat = FALSE;

        if ( bBack )
        {
            while ( !bFound && (nParPos > 0) )
            {
                if ( rFormula.GetChar(nParPos) == '"' )
                {
                    nParPos--;
                    while ( (nParPos > 0) && rFormula.GetChar(nParPos) != '"' )
                        nParPos--;
                    if (nParPos > 0)
                        nParPos--;
                }
                else if ( !(bFound = ( rFormula.GetChar(nParPos) == '(' ) ) )
                    nParPos--;
            }
        }
        else
        {
            while ( !bFound && (nParPos < nStrLen) )
            {
                if ( rFormula.GetChar(nParPos) == '"' )
                {
                    nParPos++;
                    while ( (nParPos < nStrLen) && rFormula.GetChar(nParPos) != '"' )
                        nParPos++;
                    nParPos++;
                }
                else if ( !(bFound = ( rFormula.GetChar(nParPos) == '(' ) ) )
                    nParPos++;
            }
        }

        if ( bFound && (nParPos > 0) )
        {
            nFStart = nParPos-1;

            while ( (nFStart > 0) && IsFormulaText( rFormula, nFStart ))
                nFStart--;
        }

        nFStart++;

        if ( bFound )
        {
            if ( IsFormulaText( rFormula, nFStart ) )
            {
                                    //  Funktion gefunden
                if ( pFuncName )
                    *pFuncName = rFormula.Copy( nFStart, nParPos-nFStart );
            }
            else                    // Klammern ohne Funktion -> weitersuchen
            {
                bRepeat = TRUE;
                if ( !bBack )
                    nParPos++;
                else if (nParPos > 0)
                    nParPos--;
                else
                    bRepeat = FALSE;
            }
        }
        else                        // keine Klammern gefunden
        {
            nFStart = FUNC_NOTFOUND;
            if ( pFuncName )
                pFuncName->Erase();
        }
    }
    while(bRepeat);

    return nFStart;
}

//------------------------------------------------------------------------

xub_StrLen  ScFormulaUtil::GetFunctionEnd( const String& rStr, xub_StrLen nStart )
{
    xub_StrLen nStrLen = rStr.Len();

    if ( nStrLen < nStart )
        return nStart;

    short   nParCount = 0;
    BOOL    bFound = FALSE;

    while ( !bFound && (nStart < nStrLen) )
    {
        sal_Unicode c = rStr.GetChar(nStart);

        if ( c == '"' )
        {
            nStart++;
            while ( (nStart < nStrLen) && rStr.GetChar(nStart) != '"' )
                nStart++;
        }
        else if ( c == '(' )
            nParCount++;
        else if ( c == ')' )
        {
            nParCount--;
            if ( nParCount == 0 )
                bFound = TRUE;
            else if ( nParCount < 0 )
            {
                bFound = TRUE;
                nStart--;   // einen zu weit gelesen
            }
        }
        else if ( c == ';' )
        {
            if ( nParCount == 0 )
            {
                bFound = TRUE;
                nStart--;   // einen zu weit gelesen
            }
        }
        nStart++; // hinter gefundene Position stellen
    }

    return nStart;
}

//------------------------------------------------------------------

xub_StrLen ScFormulaUtil::GetArgStart( const String& rStr, xub_StrLen nStart, USHORT nArg )
{
    xub_StrLen nStrLen = rStr.Len();

    if ( nStrLen < nStart )
        return nStart;

    short   nParCount   = 0;
    short   nSemiCount  = 0;
    BOOL    bFound      = FALSE;

    while ( !bFound && (nStart < nStrLen) )
    {
        sal_Unicode c = rStr.GetChar(nStart);

        if ( c == '"' )
        {
            nStart++;
            while ( (nStart < nStrLen) && rStr.GetChar(nStart) != '"' )
                nStart++;
        }
        else if ( c == '(' )
        {
            bFound = ( nArg == 0 );
            nParCount++;
        }
        else if ( c == ')' )
        {
            nParCount--;
            bFound = ( nParCount == 0 );
        }
        else if ( c == ';' )
        {
            if ( nParCount == 1 )
            {
                nArg--;
                bFound = ( nArg == 0  );
            }
        }
        nStart++;
    }

    return nStart;
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
    ScFuncRes( ResId&, ScFuncDesc* );

private:
    USHORT GetNum();
};

//--------------------------------------------------------------------

ScFuncRes::ScFuncRes( ResId &aRes, ScFuncDesc* pDesc )
 : Resource(aRes)
{
    USHORT      nArgs;

    pDesc->nCategory = 1;
    pDesc->nCategory = GetNum();
    pDesc->nHelpId = GetNum() + 32768;      //! Hack, siehe ScFuncs.src
    pDesc->nArgCount = GetNum();
    nArgs = pDesc->nArgCount;
    if (nArgs >= VAR_ARGS) nArgs = nArgs-VAR_ARGS+1;
    if (nArgs)
    {
        pDesc->aDefArgOpt = new BOOL[nArgs];
        for (USHORT i = 0; i < nArgs; i++)
            pDesc->aDefArgOpt[i] = (BOOL)GetNum();
    }

    pDesc->pFuncName = new String( ScCompiler::pSymbolTableNative[aRes.GetId()] );
    pDesc->pFuncDesc = new String(ScResId(1));

    if (nArgs)
    {
        pDesc->aDefArgNames = new String*[nArgs];
        pDesc->aDefArgDescs = new String*[nArgs];
        for (USHORT i = 0; i < nArgs; i++)
        {
            pDesc->aDefArgNames[i] = new String(ScResId(2*(i+1)  ));
            pDesc->aDefArgDescs[i] = new String(ScResId(2*(i+1)+1));
        }
    }

    FreeResource();
}

//------------------------------------------------------------------------

USHORT ScFuncRes::GetNum()
{
    return ReadShortRes();
}

//=========================================================================

// um an die protected von Resource ranzukommen
class ScResourcePublisher : public Resource
{
public:
        ScResourcePublisher( const ScResId& rId ) : Resource( rId ) {}
    BOOL            IsAvailableRes( const ResId& rId ) const
                        { return Resource::IsAvailableRes( rId ); }
    void            FreeResource() { Resource::FreeResource(); }
};


ScFunctionList::ScFunctionList() :
        nMaxFuncNameLen ( 0 )
{
    ScFuncDesc*     pDesc   = NULL;
    xub_StrLen      nStrLen = 0;
    FuncCollection* pFuncColl;
    USHORT i,j;
    USHORT nDescBlock[] =
    {
        RID_SC_FUNCTION_DESCRIPTIONS1,
        RID_SC_FUNCTION_DESCRIPTIONS2
    };
    const USHORT nBlocks = sizeof(nDescBlock) / sizeof(USHORT);

    aFunctionList.Clear();

    for ( USHORT k = 0; k < nBlocks; k++ )
    {
        ScResourcePublisher* pBlock =
            new ScResourcePublisher( ScResId( nDescBlock[k] ) );
        for (i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; i++)
        {   // Alle moeglichen OpCodes abgrasen.
            // Das ist zwar nicht das schnellste, aber sonst muessten
            // die Sub-Ressources innerhalb der Ressource-Bloecke und die
            // Ressource-Bloecke selber nach OpCodes geordnet sein,
            // was wohl eher utopisch ist..
            ScResId aRes(i);
            aRes.SetRT(RSC_RESOURCE);
            if (pBlock->IsAvailableRes(aRes))
            {   // Subresource fuer OpCode vorhanden
                pDesc = new ScFuncDesc;
                ScFuncRes aSubRes(aRes, pDesc);
                pDesc->nFIndex = i;
                aFunctionList.Insert( pDesc, LIST_APPEND );

                nStrLen = (*(pDesc->pFuncName)).Len();
                if (nStrLen > nMaxFuncNameLen)
                    nMaxFuncNameLen = nStrLen;
            }
        }
        pBlock->FreeResource();
        delete pBlock;
    }

    USHORT nNextId = SC_OPCODE_LAST_OPCODE_ID + 1;      // FuncID for AddIn functions

    // Auswertung AddIn-Liste
    String aDefArgNameValue =   String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("value"));
    String aDefArgNameString =  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("string"));
    String aDefArgNameValues =  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("values"));
    String aDefArgNameStrings = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("strings"));
    String aDefArgNameCells =   String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("cells"));
    String aDefArgNameNone =    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("none"));
    String aDefArgDescValue =   String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("a value"));
    String aDefArgDescString =  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("a string"));
    String aDefArgDescValues =  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("array of values"));
    String aDefArgDescStrings = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("array of strings"));
    String aDefArgDescCells =   String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("range of cells"));
    String aDefArgDescNone =    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("none"));
    String aArgName, aArgDesc;
    pFuncColl = ScGlobal::GetFuncCollection();
    for (i = 0; i < pFuncColl->GetCount(); i++)
    {
        pDesc = new ScFuncDesc;
        FuncData *pAddInFuncData = (FuncData*)pFuncColl->At(i);
        USHORT nArgs = pAddInFuncData->GetParamCount() - 1;
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
            pDesc->aDefArgOpt   = new BOOL[nArgs];
            pDesc->aDefArgNames = new String*[nArgs];
            pDesc->aDefArgDescs = new String*[nArgs];
            for (j = 0; j < nArgs; j++)
            {
                pDesc->aDefArgOpt[j] = FALSE;
                pAddInFuncData->GetParamDesc( aArgName, aArgDesc, j+1 );
                if ( aArgName.Len() )
                    pDesc->aDefArgNames[j] = new String( aArgName );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameValue );
                            break;
                        case PTR_STRING:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameCells );
                            break;
                        default:
                            pDesc->aDefArgNames[j] = new String( aDefArgNameNone );
                            break;
                    }
                }
                if ( aArgDesc.Len() )
                    pDesc->aDefArgDescs[j] = new String( aArgDesc );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescValue );
                            break;
                        case PTR_STRING:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescCells );
                            break;
                        default:
                            pDesc->aDefArgDescs[j] = new String( aDefArgDescNone );
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

ScFuncDesc::ScFuncDesc()
    :   nFIndex         (0),
        nCategory       (0),
        pFuncName       (NULL),
        pFuncDesc       (NULL),
        nArgCount       (0),
        aDefArgNames    (NULL),
        aDefArgDescs    (NULL),
        aDefArgOpt      (NULL),
        nHelpId         (0)
{}

//------------------------------------------------------------------------

ScFuncDesc::~ScFuncDesc()
{
    USHORT      nArgs;

    nArgs = nArgCount;
    if (nArgs >= VAR_ARGS) nArgs -= VAR_ARGS-1;
    if (nArgs)
    {
        for (USHORT i=0; i<nArgs; i++ )
        {
            delete aDefArgNames[i];
            delete aDefArgDescs[i];
        }
        delete [] aDefArgNames;
        delete [] aDefArgDescs;
        delete [] aDefArgOpt;
    }
    if(pFuncName)
        delete pFuncName;

    if(pFuncDesc)
        delete pFuncDesc;
}

//------------------------------------------------------------------------

String ScFuncDesc::GetParamList() const
{
    String aSig;

    if ( nArgCount > 0 )
    {
        if ( nArgCount < VAR_ARGS )
        {
            for ( USHORT i=0; i<nArgCount; i++ )
            {
                aSig += *(aDefArgNames[i]);
                if ( i != nArgCount-1 )
                    aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "; " ));
            }
        }
        else
        {
            USHORT nFix = nArgCount - VAR_ARGS;
            for ( USHORT nArg = 0; nArg < nFix; nArg++ )
            {
                aSig += *(aDefArgNames[nArg]);
                aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "; " ));
            }
            aSig += *(aDefArgNames[nFix]);
            aSig += '1';
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "; " ));
            aSig += *(aDefArgNames[nFix]);
            aSig += '2';
            aSig.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "; ... " ));
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

String ScFuncDesc::GetFormulaString( String** aArgArr ) const
{
    String aFormula;

    if(pFuncName)
    {
        aFormula= *pFuncName;

        aFormula += '(';

        if ( nArgCount > 0 && aArgArr )
        {
            BOOL bLastArg = ( aArgArr[0]->Len() == 0 );

            if ( !bLastArg )
            {
                for ( USHORT i=0; i<nArgCount && !bLastArg; i++ )
                {
                    aFormula += *(aArgArr[i]);

                    if ( i < (nArgCount-1) )
                    {
                        bLastArg = !( aArgArr[i+1]->Len() > 0 );
                        if ( !bLastArg )
                            aFormula += ';';
                    }
                }
            }
        }

        aFormula += ')';
    }
    return aFormula;
}

//========================================================================
// class ScFunctionMgr:

ScFunctionMgr::ScFunctionMgr()
    :   pFuncList   ( ScGlobal::GetStarCalcFunctionList() ),
        pCurCatList ( NULL )
{
    DBG_ASSERT( pFuncList, "Funktionsliste nicht gefunden." );
    ULONG       nCount  = pFuncList->GetCount();
    const ScFuncDesc*   pDesc;
    List*       pRootList;
    ULONG       n;

    for ( USHORT i=0; i<MAX_FUNCCAT; i++ )                  // Kategorie-Listen erstellen
        aCatLists[i] = new List;

    pRootList = aCatLists[0];                               // Gesamtliste ("Alle") erstellen
    for ( n=0; n<nCount; n++ )
    {
        ULONG nTmpCnt=0;
        pDesc = pFuncList->GetFunction(n);
        for (nTmpCnt = 0; nTmpCnt < n; nTmpCnt++)
        {
            // ist zwar case-sensitiv, aber Umlaute muessen richtig einsortiert werden

            const ScFuncDesc*   pTmpDesc = (const ScFuncDesc*)pRootList->GetObject(nTmpCnt);
            if ( ScGlobal::pCaseCollator->compareString(
                        *pDesc->pFuncName, *pTmpDesc->pFuncName ) == COMPARE_LESS )
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
    for (USHORT i = 0; i < MAX_FUNCCAT; i++)
        delete aCatLists[i];
//  delete pFuncList;       // Macht später die App
}

//------------------------------------------------------------------------

const ScFuncDesc* ScFunctionMgr::Get( const String& rFName )
{
    const ScFuncDesc*   pDesc = NULL;
    if (rFName.Len() <= pFuncList->GetMaxFuncNameLen())
        for (pDesc = First(0); pDesc; pDesc = Next())
            if (rFName.EqualsIgnoreCaseAscii(*(pDesc->pFuncName)))
                break;
    return pDesc;
}

//------------------------------------------------------------------------

const ScFuncDesc* ScFunctionMgr::Get( USHORT nFIndex )
{
    const ScFuncDesc*   pDesc;
    for (pDesc = First(0); pDesc; pDesc = Next())
        if (pDesc->nFIndex == nFIndex)
            break;
    return pDesc;
}

//------------------------------------------------------------------------

const ScFuncDesc*   ScFunctionMgr::First( USHORT nCategory )
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

//------------------------------------------------------------------------

::utl::TransliterationWrapper* GetScGlobalpTransliteration()//add by CHINA001
{
    return ScGlobal::pTransliteration;
}

const LocaleDataWrapper* GetScGlobalpLocaleData()
{
    return ScGlobal::pLocaleData;
}
