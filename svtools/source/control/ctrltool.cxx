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

#include <string.h>

#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sal/macros.h>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/ctrltool.hxx>

// =======================================================================

// Standard Fontgroessen fuer scalierbare Fonts
static const sal_IntPtr aStdSizeAry[] =
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

class ImplFontListFontInfo : public vcl::FontInfo
{
    friend class FontList;

private:
    OutputDevice*           mpDevice;
    ImplFontListFontInfo*   mpNext;

public:
                            ImplFontListFontInfo( const vcl::FontInfo& rInfo,
                                                  OutputDevice* pDev ) :
                                vcl::FontInfo( rInfo ), mpNext(NULL)
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
    OUString                maSearchName;
    ImplFontListFontInfo*   mpFirst;
    sal_uInt16              mnType;

    ImplFontListNameInfo(const OUString& rSearchName)
        : maSearchName( rSearchName )
        , mnType(0)
    {
    }
};

//sort normal to the start
static int sortWeightValue(FontWeight eWeight)
{
    if (eWeight < WEIGHT_NORMAL)
        return eWeight + 1;
    if (eWeight > WEIGHT_NORMAL)
        return eWeight - 1;
    return 0; // eWeight == WEIGHT_NORMAL
}

static sal_Int32 ImplCompareFontInfo( ImplFontListFontInfo* pInfo1,
                                          ImplFontListFontInfo* pInfo2 )
{
    //Sort non italic before italics
    if ( pInfo1->GetItalic() < pInfo2->GetItalic() )
        return -1;
    else if ( pInfo1->GetItalic() > pInfo2->GetItalic() )
        return 1;

    //Sort normal weight to the start, followed by lightest to heaviest weights
    int nWeight1 = sortWeightValue(pInfo1->GetWeight());
    int nWeight2 = sortWeightValue(pInfo2->GetWeight());

    if ( nWeight1 < nWeight2 )
        return -1;
    else if ( nWeight1 > nWeight2 )
        return 1;

    return pInfo1->GetStyleName().compareTo( pInfo2->GetStyleName() );
}

// =======================================================================

static OUString ImplMakeSearchString(const OUString& rStr)
{
    return rStr.toAsciiLowerCase();
}

// -----------------------------------------------------------------------

static OUString ImplMakeSearchStringFromName(const OUString& rStr)
{
    // check for features before alternate font separator
    sal_Int32 nColon = rStr.indexOf(':');
    sal_Int32 nSemiColon = rStr.indexOf(';');
    if (nColon != -1 && (nSemiColon == -1 || nColon < nSemiColon))
        return ImplMakeSearchString(rStr.getToken( 0, ':' ));
    return ImplMakeSearchString(rStr.getToken( 0, ';' ));
}

// -----------------------------------------------------------------------

ImplFontListNameInfo* FontList::ImplFind(const OUString& rSearchName, sal_uLong* pIndex) const
{
    // Wenn kein Eintrag in der Liste oder der Eintrag groesser ist als
    // der Letzte, dann hinten dranhaengen. Wir vergleichen erst mit dem
    // letzten Eintrag, da die Liste von VCL auch sortiert zurueckkommt
    // und somit die Wahrscheinlichkeit das hinten angehaengt werden muss
    // sehr gross ist.
    sal_uLong nCnt = maEntries.size();
    if ( !nCnt )
    {
        if ( pIndex )
            *pIndex = ULONG_MAX;
        return NULL;
    }
    else
    {
        const ImplFontListNameInfo* pCmpData = &maEntries[nCnt-1];
        sal_Int32 nComp = rSearchName.compareTo( pCmpData->maSearchName );
        if (nComp > 0)
        {
            if ( pIndex )
                *pIndex = ULONG_MAX;
            return NULL;
        }
        else if (nComp == 0)
            return const_cast<ImplFontListNameInfo*>(pCmpData);
    }

    // Fonts in der Liste suchen
    const ImplFontListNameInfo* pCompareData;
    const ImplFontListNameInfo* pFoundData = NULL;
    sal_uLong                   nLow = 0;
    sal_uLong                   nHigh = nCnt-1;
    sal_uLong                   nMid;

    do
    {
        nMid = (nLow + nHigh) / 2;
        pCompareData = &maEntries[nMid];
        sal_Int32 nComp = rSearchName.compareTo(pCompareData->maSearchName);
        if (nComp < 0)
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if (nComp > 0)
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
        sal_Int32 nComp = rSearchName.compareTo(pCompareData->maSearchName);
        if (nComp > 0)
            *pIndex = (nMid+1);
        else
            *pIndex = nMid;
    }

    return const_cast<ImplFontListNameInfo*>(pFoundData);
}

ImplFontListNameInfo* FontList::ImplFindByName(const OUString& rStr) const
{
    OUString aSearchName = ImplMakeSearchStringFromName(rStr);
    return ImplFind( aSearchName, NULL );
}

void FontList::ImplInsertFonts( OutputDevice* pDevice, sal_Bool bAll,
                                sal_Bool bInsertData )
{
    rtl_TextEncoding eSystemEncoding = osl_getThreadTextEncoding();

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
        vcl::FontInfo aFontInfo = pDevice->GetDevFont( i );

        // Wenn keine Raster-Schriften angezeigt werden sollen,
        // dann diese ignorieren
        if ( !bAll && (aFontInfo.GetType() == TYPE_RASTER) )
            continue;

        OUString aSearchName(aFontInfo.GetName());
        ImplFontListNameInfo*   pData;
        sal_uLong                   nIndex;
        aSearchName = ImplMakeSearchString(aSearchName);
        pData = ImplFind( aSearchName, &nIndex );

        if ( !pData )
        {
            if ( bInsertData )
            {
                ImplFontListFontInfo* pNewInfo = new ImplFontListFontInfo( aFontInfo, pDevice );
                pData = new ImplFontListNameInfo( aSearchName );
                pData->mpFirst      = pNewInfo;
                pNewInfo->mpNext    = NULL;

                if (nIndex < maEntries.size())
                    maEntries.insert(maEntries.begin()+nIndex,pData);
                else
                    maEntries.push_back(pData);
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
                    sal_Int32 eComp = ImplCompareFontInfo( pNewInfo, pTemp );
                    if ( eComp <= 0 )
                    {
                        if ( eComp == 0 )
                        {
                            // Overwrite charset, because charset should match
                            // with the system charset
                            if ( (pTemp->GetCharSet() != eSystemEncoding) &&
                                 (pNewInfo->GetCharSet() == eSystemEncoding) )
                            {
                                ImplFontListFontInfo* pTemp2 = pTemp->mpNext;
                                *((vcl::FontInfo*)pTemp) = *((vcl::FontInfo*)pNewInfo);
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
            pData->mnType |= nType;
    }
}

// =======================================================================

FontList::FontList( OutputDevice* pDevice, OutputDevice* pDevice2, sal_Bool bAll )
{
    // Variablen initialisieren
    mpDev = pDevice;
    mpDev2 = pDevice2;
    mpSizeAry = NULL;

    // Stylenamen festlegen
    maLight         = SVT_RESSTR(STR_SVT_STYLE_LIGHT);
    maLightItalic   = SVT_RESSTR(STR_SVT_STYLE_LIGHT_ITALIC);
    maNormal        = SVT_RESSTR(STR_SVT_STYLE_NORMAL);
    maNormalItalic  = SVT_RESSTR(STR_SVT_STYLE_NORMAL_ITALIC);
    maBold          = SVT_RESSTR(STR_SVT_STYLE_BOLD);
    maBoldItalic    = SVT_RESSTR(STR_SVT_STYLE_BOLD_ITALIC);
    maBlack         = SVT_RESSTR(STR_SVT_STYLE_BLACK);
    maBlackItalic   = SVT_RESSTR(STR_SVT_STYLE_BLACK_ITALIC);

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
    delete[] mpSizeAry;

    // FontInfos loeschen
    ImplFontListFontInfo *pTemp, *pInfo;
    boost::ptr_vector<ImplFontListNameInfo>::iterator it;
    for (it = maEntries.begin(); it != maEntries.end(); ++it)
    {
        pInfo = it->mpFirst;
        while ( pInfo )
        {
            pTemp = pInfo->mpNext;
            delete pInfo;
            pInfo = pTemp;
        }
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

const OUString& FontList::GetStyleName(FontWeight eWeight, FontItalic eItalic) const
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

OUString FontList::GetStyleName(const vcl::FontInfo& rInfo) const
{
    OUString aStyleName = rInfo.GetStyleName();
    FontWeight eWeight = rInfo.GetWeight();
    FontItalic eItalic = rInfo.GetItalic();

    // Nur wenn kein StyleName gesetzt ist, geben wir einen syntetischen
    // Namen zurueck
    if (aStyleName.isEmpty())
        aStyleName = GetStyleName(eWeight, eItalic);
    else
    {
        // Translate StyleName to localized name
        OUString aCompareStyleName = aStyleName.toAsciiLowerCase();
        aCompareStyleName = comphelper::string::remove(aCompareStyleName, ' ');
        if (aCompareStyleName == "bold")
            aStyleName = maBold;
        else if (aCompareStyleName == "bolditalic")
            aStyleName = maBoldItalic;
        else if (aCompareStyleName == "italic")
            aStyleName = maNormalItalic;
        else if (aCompareStyleName == "standard")
            aStyleName = maNormal;
        else if (aCompareStyleName == "regular")
            aStyleName = maNormal;
        else if (aCompareStyleName == "medium")
            aStyleName = maNormal;
        else if (aCompareStyleName == "light")
            aStyleName = maLight;
        else if (aCompareStyleName == "lightitalic")
            aStyleName = maLightItalic;
        else if (aCompareStyleName == "black")
            aStyleName = maBlack;
        else if (aCompareStyleName == "blackitalic")
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

OUString FontList::GetFontMapText( const vcl::FontInfo& rInfo ) const
{
    if ( rInfo.GetName().isEmpty() )
    {
        return OUString();
    }

    // Search Fontname
    ImplFontListNameInfo* pData = ImplFindByName( rInfo.GetName() );
    if ( !pData )
    {
        if (maMapNotAvailable.isEmpty())
            maMapNotAvailable = SVT_RESSTR(STR_SVT_FONTMAP_NOTAVAILABLE);
        return maMapNotAvailable;
    }

    // search for synthetic style
    sal_uInt16              nType       = pData->mnType;
    const OUString& rStyleName  = rInfo.GetStyleName();
    if (!rStyleName.isEmpty())
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
            return OUString();
        }
        else if ( !bNotSynthetic )
        {
            if (maMapStyleNotAvailable.isEmpty())
                ((FontList*)this)->maMapStyleNotAvailable = SVT_RESSTR(STR_SVT_FONTMAP_STYLENOTAVAILABLE);
            return maMapStyleNotAvailable;
        }
    }

    // Only Printer-Font?
    if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_PRINTER )
    {
        if (maMapPrinterOnly.isEmpty())
            ((FontList*)this)->maMapPrinterOnly = SVT_RESSTR(STR_SVT_FONTMAP_PRINTERONLY);
        return maMapPrinterOnly;
    }
    // Only Screen-Font?
    else if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_SCREEN
            && rInfo.GetType() == TYPE_RASTER )
    {
        if (maMapScreenOnly.isEmpty())
            ((FontList*)this)->maMapScreenOnly = SVT_RESSTR(STR_SVT_FONTMAP_SCREENONLY);
        return maMapScreenOnly;
    }
    else
    {
        if (maMapBoth.isEmpty())
            ((FontList*)this)->maMapBoth = SVT_RESSTR(STR_SVT_FONTMAP_BOTH);
        return maMapBoth;
    }
}

namespace
{
    vcl::FontInfo makeMissing(ImplFontListFontInfo* pFontNameInfo, const OUString &rName,
        FontWeight eWeight, FontItalic eItalic)
    {
        vcl::FontInfo aInfo;
        // if the fontname matches, we copy as much as possible
        if (pFontNameInfo)
        {
            aInfo = *pFontNameInfo;
            aInfo.SetStyleName(OUString());
        }

        aInfo.SetWeight(eWeight);
        aInfo.SetItalic(eItalic);

        //If this is a known but uninstalled symbol font which we can remap to
        //OpenSymbol then toggle its charset to be a symbol font
        if (ConvertChar::GetRecodeData(rName, OUString("OpenSymbol")))
            aInfo.SetCharSet(RTL_TEXTENCODING_SYMBOL);

        return aInfo;
    }
}

vcl::FontInfo FontList::Get(const OUString& rName, const OUString& rStyleName) const
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
            if (rStyleName.equalsIgnoreAsciiCase(GetStyleName(*pSearchInfo)))
            {
                pFontInfo = pSearchInfo;
                break;
            }

            pSearchInfo = pSearchInfo->mpNext;
        }
    }

    // reproduce attributes if data could not be found
    vcl::FontInfo aInfo;
    if ( !pFontInfo )
    {
        FontWeight eWeight = WEIGHT_DONTKNOW;
        FontItalic eItalic = ITALIC_NONE;

        if ( rStyleName == maNormal )
        {
            eItalic = ITALIC_NONE;
            eWeight = WEIGHT_NORMAL;
        }
        else if ( rStyleName == maNormalItalic )
        {
            eItalic = ITALIC_NORMAL;
            eWeight = WEIGHT_NORMAL;
        }
        else if ( rStyleName == maBold )
        {
            eItalic = ITALIC_NONE;
            eWeight = WEIGHT_BOLD;
        }
        else if ( rStyleName == maBoldItalic )
        {
            eItalic = ITALIC_NORMAL;
            eWeight = WEIGHT_BOLD;
        }
        else if ( rStyleName == maLight )
        {
            eItalic = ITALIC_NONE;
            eWeight = WEIGHT_LIGHT;
        }
        else if ( rStyleName == maLightItalic )
        {
            eItalic = ITALIC_NORMAL;
            eWeight = WEIGHT_LIGHT;
        }
        else if ( rStyleName == maBlack )
        {
            eItalic = ITALIC_NONE;
            eWeight = WEIGHT_BLACK;
        }
        else if ( rStyleName == maBlackItalic )
        {
            eItalic = ITALIC_NORMAL;
            eWeight = WEIGHT_BLACK;
        }
        aInfo = makeMissing(pFontNameInfo, rName, eWeight, eItalic);
    }
    else
        aInfo = *pFontInfo;

    // set Fontname to keep FontAlias
    aInfo.SetName( rName );
    aInfo.SetStyleName( rStyleName );

    return aInfo;
}

vcl::FontInfo FontList::Get(const OUString& rName,
                        FontWeight eWeight, FontItalic eItalic) const
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

    // reproduce attributes if data could not be found
    vcl::FontInfo aInfo;
    if ( !pFontInfo )
        aInfo = makeMissing(pFontNameInfo, rName, eWeight, eItalic);
    else
        aInfo = *pFontInfo;

    // set Fontname to keep FontAlias
    aInfo.SetName( rName );

    return aInfo;
}

// -----------------------------------------------------------------------

sal_Bool FontList::IsAvailable(const OUString& rName) const
{
    return (ImplFindByName( rName ) != 0);
}

const vcl::FontInfo& FontList::GetFontName( sal_uInt16 nFont ) const
{
    DBG_ASSERT( nFont < GetFontNameCount(), "FontList::GetFontName(): nFont >= Count" );

    return *(maEntries[nFont].mpFirst);
}

// -----------------------------------------------------------------------

sal_Handle FontList::GetFirstFontInfo(const OUString& rName) const
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

const vcl::FontInfo& FontList::GetFontInfo( sal_Handle hFontInfo ) const
{
    ImplFontListFontInfo* pInfo = (ImplFontListFontInfo*)(void*)hFontInfo;
    return *pInfo;
}

const sal_IntPtr* FontList::GetSizeAry( const vcl::FontInfo& rInfo ) const
{
    // Size-Array vorher loeschen
    if ( mpSizeAry )
    {
        delete[] ((FontList*)this)->mpSizeAry;
        ((FontList*)this)->mpSizeAry = NULL;
    }

    // Falls kein Name, dann Standardgroessen
    if ( rInfo.GetName().isEmpty() )
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
    ((FontList*)this)->mpSizeAry = new sal_IntPtr[nDevSizeCount+1];
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

const sal_IntPtr* FontList::GetStdSizeAry()
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

static const ImplFSNameItem aImplSimplifiedChinese[] =
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

//------------------------------------------------------------------------

FontSizeNames::FontSizeNames( LanguageType eLanguage )
{
    if ( eLanguage == LANGUAGE_DONTKNOW )
        eLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
    if ( eLanguage == LANGUAGE_SYSTEM )
        eLanguage = MsLangId::getSystemUILanguage();

    if (MsLangId::isSimplifiedChinese(eLanguage))
    {
        // equivalent for traditional chinese disabled by popular request, #i89077#
        mpArray = aImplSimplifiedChinese;
        mnElem = SAL_N_ELEMENTS(aImplSimplifiedChinese);
    }
    else
    {
        mpArray = NULL;
        mnElem = 0;
    }
}

//------------------------------------------------------------------------

long FontSizeNames::Name2Size( const OUString& rName ) const
{
    if ( mnElem )
    {
        OString aName(OUStringToOString(rName,
            RTL_TEXTENCODING_UTF8));

        // linear search is sufficient for this rare case
        for( long i = mnElem; --i >= 0; )
            if ( aName.equals(mpArray[i].mszUtf8Name) )
                return mpArray[i].mnSize;
    }

    return 0;
}

//------------------------------------------------------------------------

OUString FontSizeNames::Size2Name( long nValue ) const
{
    OUString aStr;

    // binary search
    for( long lower = 0, upper = mnElem - 1; lower <= upper; )
    {
        long mid = (upper + lower) >> 1;
        if ( nValue == mpArray[mid].mnSize )
        {
            aStr = OUString( mpArray[mid].mszUtf8Name, strlen(mpArray[mid].mszUtf8Name), RTL_TEXTENCODING_UTF8 );
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

OUString FontSizeNames::GetIndexName( sal_uLong nIndex ) const
{
    OUString aStr;

    if ( nIndex < mnElem )
        aStr = OUString( mpArray[nIndex].mszUtf8Name, strlen(mpArray[nIndex].mszUtf8Name), RTL_TEXTENCODING_UTF8 );

    return aStr;
}

//------------------------------------------------------------------------

long FontSizeNames::GetIndexSize( sal_uLong nIndex ) const
{
    if ( nIndex >= mnElem )
        return 0;
    return mpArray[nIndex].mnSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
