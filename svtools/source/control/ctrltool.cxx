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
#include "precompiled_svtools.hxx"

#define CTRLTOOL_CXX

#include <string.h>

#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtdata.hxx>
#include <svtools/ctrltool.hxx>

// =======================================================================

// Standard Fontgroessen fuer scalierbare Fonts
static long aStdSizeAry[] =
{
     60,
     70,
     80,
     90,
    100,
    105,
    110,
    120,
    130,
    140,
    150,
    160,
    180,
    200,
    220,
    240,
    260,
    280,
    320,
    360,
    400,
    440,
    480,
    540,
    600,
    660,
    720,
    800,
    880,
    960,
    0
};

// =======================================================================

// -----------------------------
// - class ImplFontListFonInfo -
// -----------------------------

class ImplFontListFontInfo : public FontInfo
{
    friend class FontList;

private:
    OutputDevice*           mpDevice;
    ImplFontListFontInfo*   mpNext;

public:
                            ImplFontListFontInfo( const FontInfo& rInfo,
                                                  OutputDevice* pDev ) :
                                FontInfo( rInfo )
                            {
                                mpDevice = pDev;
                            }

    OutputDevice*           GetDevice() const { return mpDevice; }
};

// ------------------------------
// - class ImplFontListNameInfo -
// ------------------------------

class ImplFontListNameInfo
{
    friend class FontList;

private:
    XubString               maSearchName;
    ImplFontListFontInfo*   mpFirst;
    sal_uInt16                  mnType;

                            ImplFontListNameInfo( const XubString& rSearchName ) :
                                maSearchName( rSearchName )
                            {}

    const XubString&        GetSearchName() const { return maSearchName; }
};

// =======================================================================

static StringCompare ImplCompareFontInfo( ImplFontListFontInfo* pInfo1,
                                          ImplFontListFontInfo* pInfo2 )
{
    if ( pInfo1->GetWeight() < pInfo2->GetWeight() )
        return COMPARE_LESS;
    else if ( pInfo1->GetWeight() > pInfo2->GetWeight() )
        return COMPARE_GREATER;

    if ( pInfo1->GetItalic() < pInfo2->GetItalic() )
        return COMPARE_LESS;
    else if ( pInfo1->GetItalic() > pInfo2->GetItalic() )
        return COMPARE_GREATER;

    return pInfo1->GetStyleName().CompareTo( pInfo2->GetStyleName() );
}

// =======================================================================

static void ImplMakeSearchString( XubString& rStr )
{
    rStr.ToLowerAscii();
}

// -----------------------------------------------------------------------

static void ImplMakeSearchStringFromName( XubString& rStr )
{
    // check for features before alternate font separator
    if (rStr.Search(':') < rStr.Search(';'))
        rStr = rStr.GetToken( 0, ':' );
    else
        rStr = rStr.GetToken( 0, ';' );
    ImplMakeSearchString( rStr );
}

// -----------------------------------------------------------------------

ImplFontListNameInfo* FontList::ImplFind( const XubString& rSearchName, sal_uLong* pIndex ) const
{
    // Wenn kein Eintrag in der Liste oder der Eintrag groesser ist als
    // der Letzte, dann hinten dranhaengen. Wir vergleichen erst mit dem
    // letzten Eintrag, da die Liste von VCL auch sortiert zurueckkommt
    // und somit die Wahrscheinlichkeit das hinten angehaengt werden muss
    // sehr gross ist.
    StringCompare eComp;
    sal_uLong nCnt = Count();
    if ( !nCnt )
    {
        if ( pIndex )
            *pIndex = LIST_APPEND;
        return NULL;
    }
    else
    {
        ImplFontListNameInfo* pCmpData = (ImplFontListNameInfo*)List::GetObject( nCnt-1 );
        eComp = rSearchName.CompareTo( pCmpData->maSearchName );
        if ( eComp == COMPARE_GREATER )
        {
            if ( pIndex )
                *pIndex = LIST_APPEND;
            return NULL;
        }
        else if ( eComp == COMPARE_EQUAL )
            return pCmpData;
    }

    // Fonts in der Liste suchen
    ImplFontListNameInfo*   pCompareData;
    ImplFontListNameInfo*   pFoundData = NULL;
    sal_uLong                   nLow = 0;
    sal_uLong                   nHigh = nCnt-1;
    sal_uLong                   nMid;

    do
    {
        nMid = (nLow + nHigh) / 2;
        pCompareData = (ImplFontListNameInfo*)List::GetObject( nMid );
        eComp = rSearchName.CompareTo( pCompareData->maSearchName );
        if ( eComp == COMPARE_LESS )
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if ( eComp == COMPARE_GREATER )
                nLow = nMid + 1;
            else
            {
                pFoundData = pCompareData;
                break;
            }
        }
    }
    while ( nLow <= nHigh );

    if ( pIndex )
    {
        eComp = rSearchName.CompareTo( pCompareData->maSearchName );
        if ( eComp == COMPARE_GREATER )
            *pIndex = (nMid+1);
        else
            *pIndex = nMid;
    }

    return pFoundData;
}

// -----------------------------------------------------------------------

ImplFontListNameInfo* FontList::ImplFindByName( const XubString& rStr ) const
{
    XubString aSearchName = rStr;
    ImplMakeSearchStringFromName( aSearchName );
    return ImplFind( aSearchName, NULL );
}

// -----------------------------------------------------------------------

void FontList::ImplInsertFonts( OutputDevice* pDevice, sal_Bool bAll,
                                sal_Bool bInsertData )
{
    rtl_TextEncoding eSystemEncoding = gsl_getSystemTextEncoding();

    sal_uInt16 nType;
    if ( pDevice->GetOutDevType() != OUTDEV_PRINTER )
        nType = FONTLIST_FONTNAMETYPE_SCREEN;
    else
        nType = FONTLIST_FONTNAMETYPE_PRINTER;

    // Alle Fonts vom Device abfragen
    int n = pDevice->GetDevFontCount();
    sal_uInt16  i;
    for( i = 0; i < n; i++ )
    {
        FontInfo aFontInfo = pDevice->GetDevFont( i );

        // Wenn keine Raster-Schriften angezeigt werden sollen,
        // dann diese ignorieren
        if ( !bAll && (aFontInfo.GetType() == TYPE_RASTER) )
            continue;

        XubString               aSearchName = aFontInfo.GetName();
        ImplFontListNameInfo*   pData;
        sal_uLong                   nIndex;
        ImplMakeSearchString( aSearchName );
        pData = ImplFind( aSearchName, &nIndex );

        if ( !pData )
        {
            if ( bInsertData )
            {
                ImplFontListFontInfo* pNewInfo = new ImplFontListFontInfo( aFontInfo, pDevice );
                pData = new ImplFontListNameInfo( aSearchName );
                pData->mpFirst      = pNewInfo;
                pNewInfo->mpNext    = NULL;
                pData->mnType       = 0;
                Insert( (void*)pData, nIndex );
            }
        }
        else
        {
            if ( bInsertData )
            {
                sal_Bool                    bInsert = sal_True;
                ImplFontListFontInfo*   pPrev = NULL;
                ImplFontListFontInfo*   pTemp = pData->mpFirst;
                ImplFontListFontInfo*   pNewInfo = new ImplFontListFontInfo( aFontInfo, pDevice );
                while ( pTemp )
                {
                    StringCompare eComp = ImplCompareFontInfo( pNewInfo, pTemp );
                    if ( (eComp == COMPARE_LESS) || (eComp == COMPARE_EQUAL) )
                    {
                        if ( eComp == COMPARE_EQUAL )
                        {
                            // Overwrite charset, because charset should match
                            // with the system charset
                            if ( (pTemp->GetCharSet() != eSystemEncoding) &&
                                 (pNewInfo->GetCharSet() == eSystemEncoding) )
                            {
                                ImplFontListFontInfo* pTemp2 = pTemp->mpNext;
                                *((FontInfo*)pTemp) = *((FontInfo*)pNewInfo);
                                pTemp->mpNext = pTemp2;
                            }
                            delete pNewInfo;
                            bInsert = sal_False;
                        }

                        break;
                    }

                    pPrev = pTemp;
                    pTemp = pTemp->mpNext;
                }

                if ( bInsert )
                {
                    pNewInfo->mpNext = pTemp;
                    if ( pPrev )
                        pPrev->mpNext = pNewInfo;
                    else
                        pData->mpFirst = pNewInfo;
                }
            }
        }

        if ( pData )
        {
            pData->mnType |= nType;
            if ( aFontInfo.GetType() != TYPE_RASTER )
                pData->mnType |= FONTLIST_FONTNAMETYPE_SCALABLE;
        }
    }
}

// =======================================================================

FontList::FontList( OutputDevice* pDevice, OutputDevice* pDevice2, sal_Bool bAll ) :
    List( 4096, sal::static_int_cast< sal_uInt16 >(pDevice->GetDevFontCount()), 32 )
{
    // Variablen initialisieren
    mpDev = pDevice;
    mpDev2 = pDevice2;
    mpSizeAry = NULL;

    // Stylenamen festlegen
    maLight         = XubString( SvtResId( STR_SVT_STYLE_LIGHT ) );
    maLightItalic   = XubString( SvtResId( STR_SVT_STYLE_LIGHT_ITALIC ) );
    maNormal        = XubString( SvtResId( STR_SVT_STYLE_NORMAL ) );
    maNormalItalic  = XubString( SvtResId( STR_SVT_STYLE_NORMAL_ITALIC ) );
    maBold          = XubString( SvtResId( STR_SVT_STYLE_BOLD ) );
    maBoldItalic    = XubString( SvtResId( STR_SVT_STYLE_BOLD_ITALIC ) );
    maBlack         = XubString( SvtResId( STR_SVT_STYLE_BLACK ) );
    maBlackItalic   = XubString( SvtResId( STR_SVT_STYLE_BLACK_ITALIC ) );

    ImplInsertFonts( pDevice, bAll, sal_True );

    // Gegebenenfalls muessen wir mit den Bildschirmfonts vergleichen,
    // damit dort die eigentlich doppelten auf Equal mappen koennen
    sal_Bool bCompareWindow = sal_False;
    if ( !pDevice2 && (pDevice->GetOutDevType() == OUTDEV_PRINTER) )
    {
        bCompareWindow = sal_True;
        pDevice2 = Application::GetDefaultDevice();
    }

    if ( pDevice2 &&
         (pDevice2->GetOutDevType() != pDevice->GetOutDevType()) )
        ImplInsertFonts( pDevice2, bAll, !bCompareWindow );
}

// -----------------------------------------------------------------------

FontList::~FontList()
{
    // Gegebenenfalls SizeArray loeschen
    if ( mpSizeAry )
        delete[] mpSizeAry;

    // FontInfos loeschen
    ImplFontListNameInfo* pData = (ImplFontListNameInfo*)First();
    while ( pData )
    {
        ImplFontListFontInfo* pTemp;
        ImplFontListFontInfo* pInfo = pData->mpFirst;
        while ( pInfo )
        {
            pTemp = pInfo->mpNext;
            delete pInfo;
            pInfo = pTemp;
        }
        ImplFontListNameInfo* pNext = (ImplFontListNameInfo*)Next();
        delete pData;
        pData = pNext;
    }
}
// -----------------------------------------------------------------------
FontList* FontList::Clone() const
{
    FontList* pReturn = new FontList(
            mpDev, mpDev2, GetFontNameCount() == mpDev->GetDevFontCount());
    return pReturn;
}

// -----------------------------------------------------------------------

const XubString& FontList::GetStyleName( FontWeight eWeight, FontItalic eItalic ) const
{
    if ( eWeight > WEIGHT_BOLD )
    {
        if ( eItalic > ITALIC_NONE )
            return maBlackItalic;
        else
            return maBlack;
    }
    else if ( eWeight > WEIGHT_MEDIUM )
    {
        if ( eItalic > ITALIC_NONE )
            return maBoldItalic;
        else
            return maBold;
    }
    else if ( eWeight > WEIGHT_LIGHT )
    {
        if ( eItalic > ITALIC_NONE )
            return maNormalItalic;
        else
            return maNormal;
    }
    else if ( eWeight != WEIGHT_DONTKNOW )
    {
        if ( eItalic > ITALIC_NONE )
            return maLightItalic;
        else
            return maLight;
    }
    else
    {
        if ( eItalic > ITALIC_NONE )
            return maNormalItalic;
        else
            return maNormal;
    }
}

// -----------------------------------------------------------------------

XubString FontList::GetStyleName( const FontInfo& rInfo ) const
{
    XubString   aStyleName = rInfo.GetStyleName();
    FontWeight  eWeight = rInfo.GetWeight();
    FontItalic  eItalic = rInfo.GetItalic();

    // Nur wenn kein StyleName gesetzt ist, geben wir einen syntetischen
    // Namen zurueck
    if ( !aStyleName.Len() )
        aStyleName = GetStyleName( eWeight, eItalic );
    else
    {
        // Translate StyleName to localized name
        XubString aCompareStyleName = aStyleName;
        aCompareStyleName.ToLowerAscii();
        aCompareStyleName.EraseAllChars( ' ' );
        if ( aCompareStyleName.EqualsAscii( "bold" ) )
            aStyleName = maBold;
        else if ( aCompareStyleName.EqualsAscii( "bolditalic" ) )
            aStyleName = maBoldItalic;
        else if ( aCompareStyleName.EqualsAscii( "italic" ) )
            aStyleName = maNormalItalic;
        else if ( aCompareStyleName.EqualsAscii( "standard" ) )
            aStyleName = maNormal;
        else if ( aCompareStyleName.EqualsAscii( "regular" ) )
            aStyleName = maNormal;
        else if ( aCompareStyleName.EqualsAscii( "medium" ) )
            aStyleName = maNormal;
        else if ( aCompareStyleName.EqualsAscii( "light" ) )
            aStyleName = maLight;
        else if ( aCompareStyleName.EqualsAscii( "lightitalic" ) )
            aStyleName = maLightItalic;
        else if ( aCompareStyleName.EqualsAscii( "black" ) )
            aStyleName = maBlack;
        else if ( aCompareStyleName.EqualsAscii( "blackitalic" ) )
            aStyleName = maBlackItalic;

        // fix up StyleName, because the PS Printer driver from
        // W2000 returns wrong StyleNames (e.g. Bold instead of Bold Italic
        // for Helvetica)
        if ( eItalic > ITALIC_NONE )
        {
            if ( (aStyleName == maNormal) ||
                 (aStyleName == maBold) ||
                 (aStyleName == maLight) ||
                 (aStyleName == maBlack) )
                aStyleName = GetStyleName( eWeight, eItalic );
        }
    }

    return aStyleName;
}

// -----------------------------------------------------------------------

XubString FontList::GetFontMapText( const FontInfo& rInfo ) const
{
    if ( !rInfo.GetName().Len() )
    {
        XubString aEmptryStr;
        return aEmptryStr;
    }

    // Search Fontname
    ImplFontListNameInfo* pData = ImplFindByName( rInfo.GetName() );
    if ( !pData )
    {
        if ( !maMapNotAvailable.Len() )
            ((FontList*)this)->maMapNotAvailable = XubString( SvtResId( STR_SVT_FONTMAP_NOTAVAILABLE ) );
        return maMapNotAvailable;
    }

    // search for synthetic style
    sal_uInt16              nType       = pData->mnType;
    const XubString&    rStyleName  = rInfo.GetStyleName();
    if ( rStyleName.Len() )
    {
        sal_Bool                    bNotSynthetic = sal_False;
        sal_Bool                    bNoneAvailable = sal_False;
        FontWeight              eWeight = rInfo.GetWeight();
        FontItalic              eItalic = rInfo.GetItalic();
        ImplFontListFontInfo*   pFontInfo = pData->mpFirst;
        while ( pFontInfo )
        {
            if ( (eWeight == pFontInfo->GetWeight()) &&
                 (eItalic == pFontInfo->GetItalic()) )
            {
                bNotSynthetic = sal_True;
                break;
            }

            pFontInfo = pFontInfo->mpNext;
        }

        if ( bNoneAvailable )
        {
            XubString aEmptryStr;
            return aEmptryStr;
        }
        else if ( !bNotSynthetic )
        {
            if ( !maMapStyleNotAvailable.Len() )
                ((FontList*)this)->maMapStyleNotAvailable = XubString( SvtResId( STR_SVT_FONTMAP_STYLENOTAVAILABLE ) );
            return maMapStyleNotAvailable;
        }
    }

    /* Size not available not implemented yet
    if ( !(nType & FONTLIST_FONTNAMETYPE_SCALABLE) )
    {
        ...
        {
            if ( !maMapSizeNotAvailable.Len() )
                 ((FontList*)this)->maMapSizeNotAvailable = XubString( SvtResId( STR_SVT_FONTMAP_SIZENOTAVAILABLE ) );
            return maMapSizeNotAvailable;
        }
    }
    */

    // Only Printer-Font?
    if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_PRINTER )
    {
        if ( !maMapPrinterOnly.Len() )
            ((FontList*)this)->maMapPrinterOnly = XubString( SvtResId( STR_SVT_FONTMAP_PRINTERONLY ) );
        return maMapPrinterOnly;
    }
    // Only Screen-Font?
    else if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_SCREEN
            && rInfo.GetType() == TYPE_RASTER )
    {
        if ( !maMapScreenOnly.Len() )
            ((FontList*)this)->maMapScreenOnly = XubString( SvtResId( STR_SVT_FONTMAP_SCREENONLY ) );
        return maMapScreenOnly;
    }
    else
    {
        if ( !maMapBoth.Len() )
            ((FontList*)this)->maMapBoth = XubString( SvtResId( STR_SVT_FONTMAP_BOTH ) );
        return maMapBoth;
    }
}

// -----------------------------------------------------------------------

sal_uInt16 FontList::GetFontNameType( const XubString& rFontName ) const
{
    ImplFontListNameInfo* pData = ImplFindByName( rFontName );
    if ( pData )
        return pData->mnType;
    else
        return 0;
}

// -----------------------------------------------------------------------

FontInfo FontList::Get( const XubString& rName, const XubString& rStyleName ) const
{
    ImplFontListNameInfo* pData = ImplFindByName( rName );
    ImplFontListFontInfo* pFontInfo = NULL;
    ImplFontListFontInfo* pFontNameInfo = NULL;
    if ( pData )
    {
        ImplFontListFontInfo* pSearchInfo = pData->mpFirst;
        pFontNameInfo = pSearchInfo;
        pSearchInfo = pData->mpFirst;
        while ( pSearchInfo )
        {
            if ( rStyleName.EqualsIgnoreCaseAscii( GetStyleName( *pSearchInfo ) ) )
            {
                pFontInfo = pSearchInfo;
                break;
            }

            pSearchInfo = pSearchInfo->mpNext;
        }
    }

    // Konnten die Daten nicht gefunden werden, dann muessen bestimmte
    // Attribute nachgebildet werden
    FontInfo aInfo;
    if ( !pFontInfo )
    {
        if ( pFontNameInfo )
            aInfo = *pFontNameInfo;

        if ( rStyleName == maNormal )
        {
            aInfo.SetItalic( ITALIC_NONE );
            aInfo.SetWeight( WEIGHT_NORMAL );
        }
        else if ( rStyleName == maNormalItalic )
        {
            aInfo.SetItalic( ITALIC_NORMAL );
            aInfo.SetWeight( WEIGHT_NORMAL );
        }
        else if ( rStyleName == maBold )
        {
            aInfo.SetItalic( ITALIC_NONE );
            aInfo.SetWeight( WEIGHT_BOLD );
        }
        else if ( rStyleName == maBoldItalic )
        {
            aInfo.SetItalic( ITALIC_NORMAL );
            aInfo.SetWeight( WEIGHT_BOLD );
        }
        else if ( rStyleName == maLight )
        {
            aInfo.SetItalic( ITALIC_NONE );
            aInfo.SetWeight( WEIGHT_LIGHT );
        }
        else if ( rStyleName == maLightItalic )
        {
            aInfo.SetItalic( ITALIC_NORMAL );
            aInfo.SetWeight( WEIGHT_LIGHT );
        }
        else if ( rStyleName == maBlack )
        {
            aInfo.SetItalic( ITALIC_NONE );
            aInfo.SetWeight( WEIGHT_BLACK );
        }
        else if ( rStyleName == maBlackItalic )
        {
            aInfo.SetItalic( ITALIC_NORMAL );
            aInfo.SetWeight( WEIGHT_BLACK );
        }
        else
        {
            aInfo.SetItalic( ITALIC_NONE );
            aInfo.SetWeight( WEIGHT_DONTKNOW );
        }
    }
    else
        aInfo = *pFontInfo;

    // set Fontname to keep FontAlias
    aInfo.SetName( rName );
    aInfo.SetStyleName( rStyleName );

    return aInfo;
}

// -----------------------------------------------------------------------

FontInfo FontList::Get( const XubString& rName,
                        FontWeight eWeight, FontItalic eItalic ) const
{
    ImplFontListNameInfo* pData = ImplFindByName( rName );
    ImplFontListFontInfo* pFontInfo = NULL;
    ImplFontListFontInfo* pFontNameInfo = NULL;
    if ( pData )
    {
        ImplFontListFontInfo* pSearchInfo = pData->mpFirst;
        pFontNameInfo = pSearchInfo;
        while ( pSearchInfo )
        {
            if ( (eWeight == pSearchInfo->GetWeight()) &&
                 (eItalic == pSearchInfo->GetItalic()) )
            {
                pFontInfo = pSearchInfo;
                break;
            }

            pSearchInfo = pSearchInfo->mpNext;
        }
    }

    // Konnten die Daten nicht gefunden werden, dann muessen bestimmte
    // Attribute nachgebildet werden
    FontInfo aInfo;
    if ( !pFontInfo )
    {
        // Falls der Fontname stimmt, uebernehmen wir soviel wie moeglich
        if ( pFontNameInfo )
        {
            aInfo = *pFontNameInfo;
            aInfo.SetStyleName( XubString() );
        }

        aInfo.SetWeight( eWeight );
        aInfo.SetItalic( eItalic );
    }
    else
        aInfo = *pFontInfo;

    // set Fontname to keep FontAlias
    aInfo.SetName( rName );

    return aInfo;
}

// -----------------------------------------------------------------------

sal_Bool FontList::IsAvailable( const XubString& rName ) const
{
    return (ImplFindByName( rName ) != 0);
}

// -----------------------------------------------------------------------

const FontInfo& FontList::GetFontName( sal_uInt16 nFont ) const
{
    DBG_ASSERT( nFont < GetFontNameCount(), "FontList::GetFontName(): nFont >= Count" );

    ImplFontListNameInfo* pData = (ImplFontListNameInfo*)List::GetObject( nFont );
    return *(pData->mpFirst);
}

// -----------------------------------------------------------------------

sal_uInt16 FontList::GetFontNameType( sal_uInt16 nFont ) const
{
    DBG_ASSERT( nFont < GetFontNameCount(), "FontList::GetFontNameType(): nFont >= Count" );

    ImplFontListNameInfo* pData = (ImplFontListNameInfo*)List::GetObject( nFont );
    return pData->mnType;
}

// -----------------------------------------------------------------------

sal_Handle FontList::GetFirstFontInfo( const XubString& rName ) const
{
    ImplFontListNameInfo* pData = ImplFindByName( rName );
    if ( !pData )
        return (sal_Handle)NULL;
    else
        return (sal_Handle)pData->mpFirst;
}

// -----------------------------------------------------------------------

sal_Handle FontList::GetNextFontInfo( sal_Handle hFontInfo ) const
{
    ImplFontListFontInfo* pInfo = (ImplFontListFontInfo*)(void*)hFontInfo;
    return (sal_Handle)(pInfo->mpNext);
}

// -----------------------------------------------------------------------

const FontInfo& FontList::GetFontInfo( sal_Handle hFontInfo ) const
{
    ImplFontListFontInfo* pInfo = (ImplFontListFontInfo*)(void*)hFontInfo;
    return *pInfo;
}

// -----------------------------------------------------------------------

const long* FontList::GetSizeAry( const FontInfo& rInfo ) const
{
    // Size-Array vorher loeschen
    if ( mpSizeAry )
    {
        delete[] ((FontList*)this)->mpSizeAry;
        ((FontList*)this)->mpSizeAry = NULL;
    }

    // Falls kein Name, dann Standardgroessen
    if ( !rInfo.GetName().Len() )
        return aStdSizeAry;

    // Zuerst nach dem Fontnamen suchen um das Device dann von dem
    // entsprechenden Font zu nehmen
    OutputDevice*           pDevice = mpDev;
    ImplFontListNameInfo*   pData = ImplFindByName( rInfo.GetName() );
    if ( pData )
        pDevice = pData->mpFirst->GetDevice();

    int nDevSizeCount = pDevice->GetDevFontSizeCount( rInfo );
    if ( !nDevSizeCount ||
         (pDevice->GetDevFontSize( rInfo, 0 ).Height() == 0) )
        return aStdSizeAry;

    MapMode aOldMapMode = pDevice->GetMapMode();
    MapMode aMap( MAP_10TH_INCH, Point(), Fraction( 1, 72 ), Fraction( 1, 72 ) );
    pDevice->SetMapMode( aMap );

    sal_uInt16  i;
    sal_uInt16  nRealCount = 0;
    long    nOldHeight = 0;
    ((FontList*)this)->mpSizeAry = new long[nDevSizeCount+1];
    for ( i = 0; i < nDevSizeCount; i++ )
    {
        Size aSize = pDevice->GetDevFontSize( rInfo, i );
        if ( aSize.Height() != nOldHeight )
        {
            nOldHeight = aSize.Height();
            ((FontList*)this)->mpSizeAry[nRealCount] = nOldHeight;
            nRealCount++;
        }
    }
    ((FontList*)this)->mpSizeAry[nRealCount] = 0;

    pDevice->SetMapMode( aOldMapMode );
    return mpSizeAry;
}

// -----------------------------------------------------------------------

const long* FontList::GetStdSizeAry()
{
    return aStdSizeAry;
}

// =======================================================================

// ---------------------------------
// - FontSizeNames & FsizeNameItem -
// ---------------------------------

struct ImplFSNameItem
{
    long        mnSize;
    const char* mszUtf8Name;
};

//------------------------------------------------------------------------

static ImplFSNameItem aImplSimplifiedChinese[] =
{
    {  50, "\xe5\x85\xab\xe5\x8f\xb7" },
    {  55, "\xe4\xb8\x83\xe5\x8f\xb7" },
    {  65, "\xe5\xb0\x8f\xe5\x85\xad" },
    {  75, "\xe5\x85\xad\xe5\x8f\xb7" },
    {  90, "\xe5\xb0\x8f\xe4\xba\x94" },
    { 105, "\xe4\xba\x94\xe5\x8f\xb7" },
    { 120, "\xe5\xb0\x8f\xe5\x9b\x9b" },
    { 140, "\xe5\x9b\x9b\xe5\x8f\xb7" },
    { 150, "\xe5\xb0\x8f\xe4\xb8\x89" },
    { 160, "\xe4\xb8\x89\xe5\x8f\xb7" },
    { 180, "\xe5\xb0\x8f\xe4\xba\x8c" },
    { 220, "\xe4\xba\x8c\xe5\x8f\xb7" },
    { 240, "\xe5\xb0\x8f\xe4\xb8\x80" },
    { 260, "\xe4\xb8\x80\xe5\x8f\xb7" },
    { 360, "\xe5\xb0\x8f\xe5\x88\x9d" },
    { 420, "\xe5\x88\x9d\xe5\x8f\xb7" }
};

// -----------------------------------------------------------------------

#if 0 // #i89077# disabled by popular request
static ImplFSNameItem aImplTraditionalChinese[] =
{
    {  50, "\xe5\x85\xab\xe8\x99\x9f" },
    {  55, "\xe4\xb8\x83\xe8\x99\x9f" },
    {  65, "\xe5\xb0\x8f\xe5\x85\xad" },
    {  75, "\xe5\x85\xad\xe8\x99\x9f" },
    {  90, "\xe5\xb0\x8f\xe4\xba\x94" },
    { 105, "\xe4\xba\x94\xe8\x99\x9f" },
    { 120, "\xe5\xb0\x8f\xe5\x9b\x9b" },
    { 140, "\xe5\x9b\x9b\xe8\x99\x9f" },
    { 150, "\xe5\xb0\x8f\xe4\xb8\x89" },
    { 160, "\xe4\xb8\x89\xe8\x99\x9f" },
    { 180, "\xe5\xb0\x8f\xe4\xba\x8c" },
    { 220, "\xe4\xba\x8c\xe8\x99\x9f" },
    { 240, "\xe5\xb0\x8f\xe4\xb8\x80" },
    { 260, "\xe4\xb8\x80\xe8\x99\x9f" },
    { 360, "\xe5\xb0\x8f\xe5\x88\x9d" },
    { 420, "\xe5\x88\x9d\xe8\x99\x9f" }
};
#endif

//------------------------------------------------------------------------

FontSizeNames::FontSizeNames( LanguageType eLanguage )
{
    if ( eLanguage == LANGUAGE_DONTKNOW )
        eLanguage = Application::GetSettings().GetUILanguage();
    if ( eLanguage == LANGUAGE_SYSTEM )
        eLanguage = MsLangId::getSystemUILanguage();

    switch( eLanguage )
    {
        case LANGUAGE_CHINESE:
        case LANGUAGE_CHINESE_SIMPLIFIED:
            mpArray = aImplSimplifiedChinese;
            mnElem = sizeof(aImplSimplifiedChinese) / sizeof(aImplSimplifiedChinese[0]);
            break;

#if 0 // #i89077# disabled by popular request
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_SINGAPORE:
        case LANGUAGE_CHINESE_MACAU:
        case LANGUAGE_CHINESE_TRADITIONAL:
            mpArray = aImplTraditionalChinese;
            mnElem = sizeof(aImplTraditionalChinese) / sizeof(aImplTraditionalChinese[0]);
            break;
#endif

        default:
            mpArray = NULL;
            mnElem = 0;
            break;
    };
}

//------------------------------------------------------------------------

long FontSizeNames::Name2Size( const String& rName ) const
{
    if ( mnElem )
    {
        ByteString aName( rName, RTL_TEXTENCODING_UTF8 );

        // linear search is sufficient for this rare case
        for( long i = mnElem; --i >= 0; )
            if ( aName == mpArray[i].mszUtf8Name )
                return mpArray[i].mnSize;
    }

    return 0;
}

//------------------------------------------------------------------------

String FontSizeNames::Size2Name( long nValue ) const
{
    String aStr;

    // binary search
    for( long lower = 0, upper = mnElem - 1; lower <= upper; )
    {
        long mid = (upper + lower) >> 1;
        if ( nValue == mpArray[mid].mnSize )
        {
            aStr = String( mpArray[mid].mszUtf8Name, RTL_TEXTENCODING_UTF8 );
            break;
        }
        else if ( nValue < mpArray[mid].mnSize )
            upper = mid - 1;
        else /* ( nValue > mpArray[mid].mnSize ) */
            lower = mid + 1;
    }

    return aStr;
}

//------------------------------------------------------------------------

String FontSizeNames::GetIndexName( sal_uLong nIndex ) const
{
    String aStr;

    if ( nIndex < mnElem )
        aStr = String( mpArray[nIndex].mszUtf8Name, RTL_TEXTENCODING_UTF8 );

    return aStr;
}

//------------------------------------------------------------------------

long FontSizeNames::GetIndexSize( sal_uLong nIndex ) const
{
    if ( nIndex >= mnElem )
        return 0;
    return mpArray[nIndex].mnSize;
}
