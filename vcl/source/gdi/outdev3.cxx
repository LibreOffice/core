/*************************************************************************
 *
 *  $RCSfile: outdev3.cxx,v $
 *
 *  $Revision: 1.159 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:34:00 $
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

#include <math.h>
#include <string.h>


#define _SV_OUTDEV_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif // REMOTE_APPSERVER

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif // _SV_SALLAYOUT_HXX

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <metric.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_FONTCVT_HXX
#include <fontcvt.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif
#ifndef _VCL_FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _OSL_FILE_H
#include <osl/file.h>
#endif

#ifndef _SV_GLYPHCACHE_HXX
#include <glyphcache.hxx>
#endif

#include <unohelp.hxx>
#ifndef _VCL_PDFWRITER_IMPL_HXX
#include <pdfwriter_impl.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <controllayout.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HDL_
#include <com/sun/star/beans/PropertyValues.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#if defined UNX
#define GLYPH_FONT_HEIGHT   128
#elif defined OS2
#define GLYPH_FONT_HEIGHT   176
#else
#define GLYPH_FONT_HEIGHT   256
#endif

#if defined(WIN32)
#include <malloc.h>
#define alloca _alloca
#elif defined(SOLARIS) || defined(IRIX)
#include <alloca.h>
#endif

#include <memory>

// =======================================================================

DBG_NAMEEX( OutputDevice );
DBG_NAMEEX( Font );

// =======================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::vcl;

// =======================================================================

#define TEXT_DRAW_ELLIPSIS  (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS)

// =======================================================================

#define UNDERLINE_LAST      UNDERLINE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

// =======================================================================

static void ImplRotatePos( long nOriginX, long nOriginY, long& rX, long& rY,
                           int nOrientation )
{
    if ( (nOrientation >= 0) && !(nOrientation % 900) )
    {
        if ( (nOrientation >= 3600) )
            nOrientation %= 3600;

        if ( nOrientation )
        {
            rX -= nOriginX;
            rY -= nOriginY;

            if ( nOrientation == 900 )
            {
                long nTemp = rX;
                rX = rY;
                rY = -nTemp;
            }
            else if ( nOrientation == 1800 )
            {
                rX = -rX;
                rY = -rY;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = rX;
                rX = -rY;
                rY = nTemp;
            }

            rX += nOriginX;
            rY += nOriginY;
        }
    }
    else
    {
        double nRealOrientation = nOrientation*F_PI1800;
        double nCos = cos( nRealOrientation );
        double nSin = sin( nRealOrientation );

        // Translation...
        long nX = rX-nOriginX;
        long nY = rY-nOriginY;

        // Rotation...
        rX = +((long)(nCos*nX + nSin*nY)) + nOriginX;
        rY = -((long)(nSin*nX - nCos*nY)) + nOriginY;
    }
}

// =======================================================================

void OutputDevice::ImplUpdateFontData( BOOL bNewFontLists )
{
    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }
    if ( bNewFontLists )
    {
        if ( mpGetDevFontList )
        {
            delete mpGetDevFontList;
            mpGetDevFontList = NULL;
        }
        if ( mpGetDevSizeList )
        {
            delete mpGetDevSizeList;
            mpGetDevSizeList = NULL;
        }
    }

    if ( GetOutDevType() == OUTDEV_PRINTER || mpPDFWriter )
    {
        ImplSVData* pSVData = ImplGetSVData();

        if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
            mpFontCache->Clear();

        if ( bNewFontLists )
        {
            // we need a graphics
#ifndef REMOTE_APPSERVER
            if ( ImplGetGraphics() )
#else
            if ( ImplGetServerGraphics() )
#endif
            {
                if( mpFontList && mpFontList != pSVData->maGDIData.mpScreenFontList )
                    mpFontList->Clear();

                if( mpPDFWriter )
                {
                    if( mpFontList && mpFontList != pSVData->maGDIData.mpScreenFontList )
                        delete mpFontList;
                    if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
                        delete mpFontCache;
                    mpFontList = mpPDFWriter->filterDevFontList( pSVData->maGDIData.mpScreenFontList );
                    mpFontCache = new ImplFontCache( FALSE );
                }
                else
                {
                    if( mpOutDevData )
                    {
                        ImplFontSubstEntry* pEntry = mpOutDevData->mpFirstFontSubstEntry;
                        while( pEntry )
                        {
                            ImplFontSubstEntry* pNext = pEntry->mpNext;
                            delete pEntry;
                            pEntry = pNext;
                        }
                        mpOutDevData->mpFirstFontSubstEntry = NULL;
                    }
                    mpGraphics->GetDevFontList( mpFontList );
                    mpGraphics->GetDevFontSubstList( this );
                }
            }
        }
    }

    mbInitFont = TRUE;
    mbNewFont = TRUE;

    // also update child windows if needed
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pChild = ((Window*)this)->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplUpdateFontData( TRUE );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplUpdateAllFontData( BOOL bNewFontLists )
{
    ImplSVData* pSVData = ImplGetSVData();

    // update all windows
    Window* pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->ImplUpdateFontData( bNewFontLists );

        Window* pSysWin = pFrame->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->ImplUpdateFontData( bNewFontLists );
            pSysWin = pSysWin->mpNextOverlap;
        }

        pFrame = pFrame->mpFrameData->mpNextFrame;
    }

    // update all virtual devices
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while ( pVirDev )
    {
        pVirDev->ImplUpdateFontData( bNewFontLists );
        pVirDev = pVirDev->mpNext;
    }

    // update all printers
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while ( pPrinter )
    {
        pPrinter->ImplUpdateFontData( bNewFontLists );
        pPrinter = pPrinter->mpNext;
    }

    // clear global font lists to have them updated
    pSVData->maGDIData.mpScreenFontCache->Clear();
    if ( bNewFontLists )
    {
        pSVData->maGDIData.mpScreenFontList->Clear();
        pFrame = pSVData->maWinData.mpFirstFrame;
        if ( pFrame )
        {
#ifndef REMOTE_APPSERVER
            if ( pFrame->ImplGetGraphics() )
#endif
                pFrame->mpGraphics->GetDevFontList( pFrame->mpFrameData->mpFontList );
        }
    }
}

// =======================================================================

struct ImplLocaliziedFontName
{
    const char*         mpEnglishName;
    const sal_Unicode*  mpLocaliziedNames;
};

static sal_Unicode const aBatang[] = { 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aBatangChe[] = { 0xBC14, 0xD0D5, 0xCCB4, 0, 0 };
static sal_Unicode const aGungsuh[] = { 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aGungsuhChe[] = { 0xAD81, 0xC11C, 0xCCB4, 0, 0 };
static sal_Unicode const aGulim[] = { 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aGulimChe[] = { 0xAD74, 0xB9BC, 0xCCB4, 0, 0 };
static sal_Unicode const aDotum[] = { 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aDotumChe[] = { 0xB3CB, 0xC6C0, 0xCCB4, 0, 0 };
static sal_Unicode const aSimSun[] = { 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aNSimSun[] = { 0x65B0, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aSimHei[] = { 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aSimKai[] = { 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const azycjkSun[] = { 0x4E2D, 0x6613, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const azycjkHei[] = { 0x4E2D, 0x6613, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const azycjkKai[] = { 0x4E2D, 0x6613, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZHei[] = { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aFZKai[] = { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZSongYI[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4E00, 0, 0 };
static sal_Unicode const aFZShuSong[] = { 0x65B9, 0x6B63, 0x4E66, 0x5B8B, 0, 0 };
static sal_Unicode const aFZFangSong[] = { 0x65B9, 0x6B63, 0x4EFF, 0x5B8B, 0, 0 };
// Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
// First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
static sal_Unicode const aMHei[] = { 'm', 0x7B80, 0x9ED1, 0, 'm', 0x6F60, 0x7AAA, 0, 0 };
static sal_Unicode const aMKai[] = { 'm', 0x7B80, 0x6977, 0x566C, 0, 'm', 0x6F60, 0x7FF1, 0x628E, 0, 0 };
static sal_Unicode const aMSong[] = { 'm', 0x7B80, 0x5B8B, 0, 'm', 0x6F60, 0x51BC, 0, 0 };
static sal_Unicode const aCFangSong[] = { 'm', 0x7B80, 0x592B, 0x5B8B, 0, 'm', 0x6F60, 0x6E98, 0x51BC, 0, 0 };
static sal_Unicode const aMingLiU[] = { 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aPMingLiU[] = { 0x65B0, 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aHei[] = { 0x6865, 0, 0 };
static sal_Unicode const aKai[] = { 0x6B61, 0, 0 };
static sal_Unicode const aMing[] = { 0x6D69, 0x6E67, 0, 0 };
static sal_Unicode const aMSGothic[] = { 0xFF2D, 0xFF33, ' ', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSPGothic[] = { 0xFF2D, 0xFF33, ' ', 0xFF30, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSMincho[] = { 0xFF2D, 0xFF33, ' ', 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aMSPMincho[] = { 0xFF2D, 0xFF33, ' ', 0xFF30, 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aHGMinchoL[] = { 'h', 'g', 0x660E, 0x671D, 'l', 0, 0 };
static sal_Unicode const aHGGothicB[] = { 'h', 'g', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0, 0 };
static sal_Unicode const aHGHeiseiMin[] = { 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 0, 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 'w', '3', 'x', '1', '2', 0, 0 };
static sal_Unicode const aSunDotum[] = { 0xC36C, 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aSunBatang[] = { 0xC36C, 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aFzHeiTi[] = { 0x65B9, 0x6B63, 0x9ED1, 0x9AD4, 0, 0 };
static sal_Unicode const aFzMingTi[] = { 0x65B9, 0x6B63, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aFzSongTi[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4F53, 0, 0 };

static sal_Unicode const aHYMyeongJoExtra[]         = { 'h', 'y', 0xACAC, 0xBA85, 0xC870, 0, 0 };
static sal_Unicode const aHYSinMyeongJoMedium[]     = { 'h', 'y', 0xC2E0, 0xBA85, 0xC870, 0, 0 };
static sal_Unicode const aHYGothicMedium[]          = { 'h', 'y', 0xC911, 0xACE0, 0xB515, 0, 0 };
static sal_Unicode const aHYGraphicMedium[]         = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 'm', 0, 0 };
static sal_Unicode const aHYGraphic[]               = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 0, 0 };
static sal_Unicode const aNewGulim[]                = { 0xC0C8, 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aSunGungseo[]              = { 0xC36C, 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aHYGungSoBold[]            = { 'h','y', 0xAD81, 0xC11C, 'b', 0, 0 };
static sal_Unicode const aHYGungSo[]                 = { 'h','y', 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aSunHeadLine[]             = { 0xC36C, 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0, 0 };
static sal_Unicode const aHYHeadLineMedium[]        = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 'm', 0, 0 };
static sal_Unicode const aHYHeadLine[]              = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0, 0 };
static sal_Unicode const aYetR[]                    = { 0xD734, 0xBA3C, 0xC61B, 0xCCB4, 0, 0 };
static sal_Unicode const aHYGothicExtra[]           = { 'h', 'y', 0xACAC, 0xACE0, 0xB515, 0, 0 };
static sal_Unicode const aSunMokPan[]               = { 0xC36C, 0xBAA9, 0xD310, 0, 0 };
static sal_Unicode const aSunYeopseo[]              = { 0xC36C, 0xC5FD, 0xC11C, 0, 0 };
static sal_Unicode const aSunBaekSong[]              = { 0xC36C, 0xBC31, 0xC1A1, 0, 0 };
static sal_Unicode const aHYPostLight[]             = { 'h', 'y', 0xC5FD, 0xC11C, 'l', 0, 0 };
static sal_Unicode const aHYPost[]                  = { 'h', 'y', 0xC5FD, 0xC11C, 0, 0 };
static sal_Unicode const aMagicR[]                  = { 0xD734, 0xBA3C, 0xB9E4, 0xC9C1, 0xCCB4, 0, 0 };
static sal_Unicode const aSunCrystal[]              = { 0xC36C, 0xD06C, 0xB9AC, 0xC2A4, 0xD0C8, 0, 0 };
static sal_Unicode const aSunSaemmul[]              = { 0xC36C, 0xC0D8, 0xBB3C, 0, 0 };
static sal_Unicode const aHYShortSamulMedium[]      = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 'm', 0, 0 };
static sal_Unicode const aHYShortSamul[]            = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 0, 0 };
static sal_Unicode const aHaansoftBatang[]          = { 0xD55C, 0xCEF4, 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aHaansoftDotum[]           = { 0xD55C, 0xCEF4, 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aHyhaeseo[]                = { 0xD55C, 0xC591, 0xD574, 0xC11C, 0, 0 };
static sal_Unicode const aMDSol[]                   = { 'm', 'd', 0xC194, 0xCCB4, 0, 0 };
static sal_Unicode const aMDGaesung[]               = { 'm', 'd', 0xAC1C, 0xC131, 0xCCB4, 0, 0 };
static sal_Unicode const aMDArt[]                   = { 'm', 'd', 0xC544, 0xD2B8, 0xCCB4, 0, 0 };
static sal_Unicode const aMDAlong[]                 = { 'm', 'd', 0xC544, 0xB871, 0xCCB4, 0, 0 };
static sal_Unicode const aMDEasop[]                 = { 'm', 'd', 0xC774, 0xC19D, 0xCCB4, 0, 0 };
static sal_Unicode const HYShortSamulMedium[]       = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 'm', 0, 0 };
static sal_Unicode const HYShortSamul[]             = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 0, 0 };


static ImplLocaliziedFontName const aImplLocaliziedNamesList[] =
{
{   "batang",               aBatang },
{   "batangche",            aBatangChe },
{   "gungshu",              aGungsuh },
{   "gungshuche",           aGungsuhChe },
{   "gulim",                aGulim },
{   "gulimche",             aGulimChe },
{   "dotum",                aDotum },
{   "dotumche",             aDotumChe },
{   "simsun",               aSimSun },
{   "nsimsun",              aNSimSun },
{   "simhei",               aSimHei },
{   "simkai",               aSimKai },
{   "zycjksun",             azycjkSun },
{   "zycjkhei",             azycjkHei },
{   "zycjkkai",             azycjkKai },
{   "fzhei",                aFZHei },
{   "fzkai",                aFZKai },
{   "fzsong",               aFZSongYI },
{   "fzshusong",            aFZShuSong },
{   "fzfangsong",           aFZFangSong },
{   "mhei",                 aMHei },
{   "mkai",                 aMKai },
{   "msong",                aMSong },
{   "cfangsong",            aCFangSong },
{   "mingliu",              aMingLiU },
{   "pmingliu",             aPMingLiU },
{   "hei",                  aHei },
{   "kai",                  aKai },
{   "ming",                 aMing },
{   "msgothic",             aMSGothic },
{   "mspgothic",            aMSPGothic },
{   "msmincho",             aMSMincho },
{   "mspmincho",            aMSPMincho },
{   "hgminchol",            aHGMinchoL },
{   "hggothicb",            aHGGothicB },
{   "hgheiseimin",          aHGHeiseiMin },
{   "sundotum",             aSunDotum },
{   "sungulim",             aSunGulim },
{   "sunbatang",            aSunBatang },
{   "fzheiti",              aFzHeiTi },
{   "fzmingti",             aFzMingTi },
{   "fzsongti",             aFzSongTi },
{   "hymyeongjoextra",      aHYMyeongJoExtra },
{   "hysinmyeongjomedium",  aHYSinMyeongJoMedium },
{   "hygothicmedium",       aHYGothicMedium },
{   "hygraphicmedium",      aHYGraphicMedium },
{   "hygraphic",            aHYGraphic },
{   "newgulim",             aNewGulim },
{   "sungungseo",           aSunGungseo },
{   "hygungsobold",         aHYGungSoBold },
{   "hygungso",             aHYGungSo },
{   "sunheadline",          aSunHeadLine },
{   "hyheadlinemedium",     aHYHeadLineMedium },
{   "hyheadline",           aHYHeadLine },
{   "yetr",                 aYetR },
{   "hygothicextra",        aHYGothicExtra },
{   "sunmokpan",            aSunMokPan },
{   "sunyeopseo",           aSunYeopseo },
{   "sunbaeksong",          aSunBaekSong },
{   "hypostlight",          aHYPostLight },
{   "hypost",               aHYPost },
{   "magicr",               aMagicR },
{   "suncrystal",           aSunCrystal },
{   "sunsaemmul",           aSunSaemmul },
{   "hyshortsamulmedium",   aHYShortSamulMedium },
{   "hyshortsamul",         aHYShortSamul },
{   "haansoftbatang",       aHaansoftBatang },
{   "haansoftdotum",        aHaansoftDotum },
{   "hyhaeseo",             aHyhaeseo },
{   "mdsol",                aMDSol },
{   "mdgaesung",            aMDGaesung },
{   "mdart",                aMDArt },
{   "mdalong",              aMDAlong },
{   "mdeasop",              aMDEasop },
{   "hyshortsamulmedium",   HYShortSamulMedium },
{   "hyshortsamul",         HYShortSamul },
{   NULL,                   NULL },
};

// -----------------------------------------------------------------------

void ImplGetEnglishSearchFontName( String& rName )
{
    BOOL        bTranslate = FALSE;
    xub_StrLen  i;
    xub_StrLen  nLen = rName.Len();

    // Remove trailing whitespaces
    i = nLen;
    while ( i && (rName.GetChar( i-1 ) < 32) )
        i--;
    if ( i != nLen )
        rName.Erase( i );

    // Remove Script at the end
    // Scriptname must be the last part of the fontname and
    // looks like "fontname (scriptname)". So there can only be a
    // script name at the and of the fontname, when the last char is ')'
    if ( (nLen >= 3) && rName.GetChar( nLen-1 ) == ')' )
    {
        int nOpen = 1;
        xub_StrLen nTempLen = nLen-2;
        while ( nTempLen )
        {
            if ( rName.GetChar( nTempLen ) == '(' )
            {
                nOpen--;
                if ( !nOpen )
                {
                    // Remove Space at the end
                    if ( nTempLen && (rName.GetChar( nTempLen-1 ) == ' ') )
                        nTempLen--;
                    rName.Erase( nTempLen );
                    nLen = nTempLen;
                    break;
                }
            }
            if ( rName.GetChar( nTempLen ) == ')' )
                nOpen++;
            nTempLen--;
        }
    }

    // This function removes all whitespaces, convert to Lowercase-ASCII
    i = 0;
    while ( i < nLen )
    {
        sal_Unicode c = rName.GetChar( i );
        if ( c > 127 )
        {
            // Translate to Lowercase-ASCII
            // FullWidth-ASCII to half ASCII
            if ( (c >= 0xFF00) && (c <= 0xFF5E) )
            {
                c -= 0xFF00-0x0020;
                // Upper to Lower
                if ( (c >= 'A') && (c <= 'Z') )
                    c += 0x0020;
                rName.SetChar( i, c );
            }
            else
            {
                // Only Fontnames with None-Ascii-Characters must be translated
                bTranslate = TRUE;
            }
        }
        // not lowercase Ascii
        else if ( !((c >= 'a') && (c <= 'z')) )
        {
            // To Lowercase-Ascii
            if ( (c >= 'A') && (c <= 'Z') )
            {
                c += 0x0020;
                rName.SetChar( i, c );
            }
            else if( ((c < '0') || (c > '9')) && (c != ';') ) // not 0-9 or semicolon
            {
                // Remove white spaces and special characters
                rName.Erase( i, 1 );
                nLen--;
                continue;
            }
        }

        i++;
    }

    // Translate localizied name to English ASCII name
    const ImplLocaliziedFontName* pTranslateNames = aImplLocaliziedNamesList;
    while ( bTranslate && pTranslateNames->mpEnglishName )
    {
        const sal_Unicode* pLocaliziedName = pTranslateNames->mpLocaliziedNames;
        while ( *pLocaliziedName )
        {
            if ( rName.Equals( pLocaliziedName ) )
            {
                rName.AssignAscii( pTranslateNames->mpEnglishName );
                bTranslate = FALSE;
                break;
            }

            // Run to the end of the Token (\0\0 is the end mark)
            while ( *pLocaliziedName )
                pLocaliziedName++;
            pLocaliziedName++;
        }

        pTranslateNames++;
    }
}

// -----------------------------------------------------------------------

String GetFontToken( const String& rStr, xub_StrLen nToken, xub_StrLen& rIndex )
{
    const sal_Unicode*  pStr            = rStr.GetBuffer();
    xub_StrLen          nLen            = (xub_StrLen)rStr.Len();
    xub_StrLen          nTok            = 0;
    xub_StrLen          nFirstChar      = rIndex;
    xub_StrLen          i               = nFirstChar;

    // get the token position and it's length
    pStr += i;
    while ( i < nLen )
    {
        // increase TokCount when the token matches
        if ( (*pStr == ';') || (*pStr == ',') )
        {
            nTok++;

            if ( nTok == nToken )
                nFirstChar = i+1;
            else
            {
                if ( nTok > nToken )
                    break;
            }
        }

        pStr++;
        i++;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = STRING_NOTFOUND;
        return String( rStr, nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = STRING_NOTFOUND;
        return String();
    }
}

// -----------------------------------------------------------------------

static int ImplStrMatchCompare( const String& rStr1, const char* pStr2 )
{
    const sal_Unicode* pStr1 = rStr1.GetBuffer();
    while ( (*pStr1 == (xub_Unicode)(unsigned char)*pStr2) && *pStr1 )
    {
        pStr1++;
        pStr2++;
    }

    if ( !(*pStr1) )
        return 0;
    else
        return *pStr1-((xub_Unicode)(unsigned char)*pStr2);
}

// =======================================================================

void ImplFreeOutDevFontData()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    while( pEntry )
    {
        ImplFontSubstEntry* pNext = pEntry->mpNext;
        delete pEntry;
        pEntry = pNext;
    }
}

// =======================================================================

void OutputDevice::BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::EndFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maGDIData.mbFontSubChanged )
    {
        ImplUpdateAllFontData( FALSE );

        Application* pApp = GetpApp();
        DataChangedEvent aDCEvt( DATACHANGED_FONTSUBSTITUTION );
        pApp->DataChanged( aDCEvt );
        pApp->NotifyAllWindows( aDCEvt );
        pSVData->maGDIData.mbFontSubChanged = FALSE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::AddFontSubstitute( const XubString& rFontName,
                                      const XubString& rReplaceFontName,
                                      USHORT nFlags )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = new ImplFontSubstEntry;

    pEntry->maName              = rFontName;
    pEntry->maReplaceName       = rReplaceFontName;
    pEntry->maSearchName        = rFontName;
    pEntry->maSearchReplaceName = rReplaceFontName;
    pEntry->mnFlags             = nFlags;
    pEntry->mpNext              = pSVData->maGDIData.mpFirstFontSubst;
    ImplGetEnglishSearchFontName( pEntry->maSearchName );
    ImplGetEnglishSearchFontName( pEntry->maSearchReplaceName );

    pSVData->maGDIData.mpFirstFontSubst = pEntry;
    pSVData->maGDIData.mbFontSubChanged = TRUE;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplAddDevFontSubstitute( const XubString& rFontName,
                                             const XubString& rReplaceFontName,
                                             USHORT nFlags )
{
    ImplInitOutDevData();

    ImplFontSubstEntry* pEntry = new ImplFontSubstEntry;

    pEntry->maName              = rFontName;
    pEntry->maReplaceName       = rReplaceFontName;
    pEntry->maSearchName        = rFontName;
    pEntry->maSearchReplaceName = rReplaceFontName;
    pEntry->mnFlags             = nFlags;
    pEntry->mpNext              = mpOutDevData->mpFirstFontSubstEntry;
    ImplGetEnglishSearchFontName( pEntry->maSearchName );
    ImplGetEnglishSearchFontName( pEntry->maSearchReplaceName );

    mpOutDevData->mpFirstFontSubstEntry = pEntry;
}

// -----------------------------------------------------------------------

void OutputDevice::RemoveFontSubstitute( USHORT n )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    ImplFontSubstEntry* pPrev = NULL;
    USHORT              nCount = 0;

    while ( pEntry )
    {
        if ( nCount == n )
        {
            pSVData->maGDIData.mbFontSubChanged = TRUE;
            if ( pPrev )
                pPrev->mpNext = pEntry->mpNext;
            else
                pSVData->maGDIData.mpFirstFontSubst = pEntry->mpNext;
            delete pEntry;
            break;
        }

        nCount++;
        pPrev = pEntry;
        pEntry = pEntry->mpNext;
    }
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetFontSubstituteCount()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    USHORT              nCount = 0;
    while ( pEntry )
    {
        nCount++;
        pEntry = pEntry->mpNext;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void OutputDevice::GetFontSubstitute( USHORT n,
                                      XubString& rFontName,
                                      XubString& rReplaceFontName,
                                      USHORT& rFlags )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    USHORT              nCount = 0;
    while ( pEntry )
    {
        if ( nCount == n )
        {
            rFontName           = pEntry->maName;
            rReplaceFontName    = pEntry->maReplaceName;
            rFlags              = pEntry->mnFlags;
            break;
        }

        nCount++;
        pEntry = pEntry->mpNext;
    }
}

// -----------------------------------------------------------------------

static BOOL ImplFontSubstitute( XubString& rFontName,
                                USHORT nFlags1, USHORT nFlags2, ImplFontSubstEntry* pDevSpecific )
{
#ifdef DBG_UTIL
    String aTempName = rFontName;
    ImplGetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplFontSubstitute() called without a searchname" );
#endif

    // apply font replacement (eg, from the list in Tools->Options)
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    while ( pEntry )
    {
        if ( ((pEntry->mnFlags & nFlags1) == nFlags2) &&
             (pEntry->maSearchName == rFontName) )
        {
            rFontName = pEntry->maSearchReplaceName;
            return TRUE;
        }

        pEntry = pEntry->mpNext;
    }
    if( pDevSpecific )
    {
        pEntry = pDevSpecific;
        while ( pEntry )
        {
            if ( ((pEntry->mnFlags & nFlags1) == nFlags2) &&
                 (pEntry->maSearchName == rFontName) )
            {
                rFontName = pEntry->maSearchReplaceName;
                return TRUE;
            }
            pEntry = pEntry->mpNext;
        }
    }

    return FALSE;
}

// =======================================================================

static BOOL ImplIsFontToken( const String& rName, const String& rToken )
{
    BOOL        bRet = FALSE;
    String      aTempName;
    xub_StrLen  nIndex = 0;
    do
    {
        aTempName = GetFontToken( rName, 0, nIndex );
        if ( rToken == aTempName )
        {
            bRet = TRUE;
            break;
        }
    }
    while ( nIndex != STRING_NOTFOUND );

    return bRet;
}

// -----------------------------------------------------------------------

static void ImplAppendFontToken( String& rName, const String rNewToken )
{
    if ( rName.Len() )
    {
        rName.Append( ';' );
        rName.Append( rNewToken );
    }
    else
        rName = rNewToken;
}

// -----------------------------------------------------------------------

static void ImplAppendFontToken( String& rName, const char* pNewToken )
{
    if ( rName.Len() )
    {
        rName.Append( ';' );
        rName.AppendAscii( pNewToken );
    }
    else
        rName.AssignAscii( pNewToken );
}

// -----------------------------------------------------------------------

static void ImplAddTokenFontName( String& rName, const String& rNewToken )
{
    if ( !ImplIsFontToken( rName, rNewToken ) )
        ImplAppendFontToken( rName, rNewToken );
}

// -----------------------------------------------------------------------

static void ImplAddTokenFontNames( String& rName, const OUString& rFontNames )
{
    sal_Int32 nOuterIndex = 0;
    String aName;
    do
    {
        aName = rFontNames.getToken( 0, ';', nOuterIndex );
        String      aTempName;
        xub_StrLen  nIndex = 0;
        do
        {
            aTempName = GetFontToken( rName, 0, nIndex );
            if ( aName == aTempName )
            {
                aName.Erase();
                break;
            }
        }
        while ( nIndex != STRING_NOTFOUND );

        if ( aName.Len() )
            ImplAppendFontToken( rName, aName );
    }
    while ( nOuterIndex != -1 );
}

// -----------------------------------------------------------------------

Font OutputDevice::GetDefaultFont( USHORT nType, LanguageType eLang,
                                   ULONG nFlags, const OutputDevice* pOutDev )
{
    if( eLang == LANGUAGE_NONE || eLang == LANGUAGE_SYSTEM || eLang == LANGUAGE_DONTKNOW )
    {
        eLang = Application::GetSettings().GetUILanguage();
    }

    DefaultFontConfigItem* pDefaults = DefaultFontConfigItem::get();
    String aSearch = pDefaults->getUserInterfaceFont( eLang ); // ensure a fallback
    String aDefault = pDefaults->getDefaultFont( eLang, nType );
    if( aDefault.Len() )
        aSearch = aDefault;

    Font aFont;
    switch ( nType )
    {
        case DEFAULTFONT_SANS_UNICODE:
        case DEFAULTFONT_UI_SANS:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SANS:
        case DEFAULTFONT_LATIN_HEADING:
        case DEFAULTFONT_LATIN_SPREADSHEET:
        case DEFAULTFONT_LATIN_DISPLAY:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SERIF:
        case DEFAULTFONT_LATIN_TEXT:
        case DEFAULTFONT_LATIN_PRESENTATION:
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case DEFAULTFONT_FIXED:
        case DEFAULTFONT_LATIN_FIXED:
        case DEFAULTFONT_UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            break;

        case DEFAULTFONT_SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            break;

        case DEFAULTFONT_CJK_TEXT:
        case DEFAULTFONT_CJK_PRESENTATION:
        case DEFAULTFONT_CJK_SPREADSHEET:
        case DEFAULTFONT_CJK_HEADING:
        case DEFAULTFONT_CJK_DISPLAY:
            break;

        case DEFAULTFONT_CTL_TEXT:
        case DEFAULTFONT_CTL_PRESENTATION:
        case DEFAULTFONT_CTL_SPREADSHEET:
        case DEFAULTFONT_CTL_HEADING:
        case DEFAULTFONT_CTL_DISPLAY:
            break;
    }

    if ( aSearch.Len() )
    {
        aFont.SetSize( Size( 0, 12 ) );
        aFont.SetWeight( WEIGHT_NORMAL );
        if ( aFont.GetPitch() == PITCH_DONTKNOW )
            aFont.SetPitch ( PITCH_VARIABLE );
        if ( aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW )
            aFont.SetCharSet( gsl_getSystemTextEncoding() );

        // Should we only return available fonts on the given device
        if ( pOutDev )
        {
            pOutDev->ImplInitFontList();

            // Search Font in the FontList
            String      aName;
            String      aTempName;
            xub_StrLen  nIndex = 0;
            do
            {
                aTempName = GetFontToken( aSearch, 0, nIndex );
                ImplGetEnglishSearchFontName( aTempName );
                ImplDevFontListData* pFoundData = pOutDev->mpFontList->ImplFind( aTempName );
                if ( pFoundData )
                {
                    ImplAddTokenFontName( aName, pFoundData->mpFirst->maName );
                    if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
                        break;
                }
            }
            while ( nIndex != STRING_NOTFOUND );
            aFont.SetName( aName );
        }

        // No Name, than set all names
        if ( !aFont.GetName().Len() )
        {
            xub_StrLen nIndex = 0;
            if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
            {
                //aFont.SetName( aSearch.GetToken( 0, ';', nIndex ) );
                if( !pOutDev )
                    pOutDev = (const OutputDevice *)ImplGetSVData()->mpDefaultWin;
                if( !pOutDev )
                    aFont.SetName( aSearch.GetToken( 0, ';', nIndex ) );
                else
                {
                    pOutDev->ImplInitFontList();

                    aFont.SetName( aSearch );

                    // convert to pixel height
                    Size aSize = pOutDev->ImplLogicToDevicePixel( aFont.GetSize() );
                    if ( !aSize.Height() )
                    {
                        // use default pixel height only when logical height is zero
                        if ( aFont.GetSize().Height() )
                            aSize.Height() = 1;
                        else
                            aSize.Height() = (12*pOutDev->mnDPIY)/72;
                    }

                    // use default width only when logical width is zero
                    if( (0 == aSize.Width()) && (0 != aFont.GetSize().Width()) )
                        aSize.Width() = 1;

                    ImplFontEntry* pEntry = pOutDev->mpFontCache->Get( pOutDev->mpFontList, aFont, aSize, pOutDev->mpOutDevData ? pOutDev->mpOutDevData->mpFirstFontSubstEntry : NULL );
                    aFont.SetName( pEntry->maFontSelData.maFoundName ); // returns a single name
                }
            }
            else
                aFont.SetName( aSearch );
        }
    }
    return aFont;
}

// =======================================================================

String GetSubsFontName( const String& rName, ULONG nFlags )
{
    String aName;
    String aOrgName = GetFontToken( rName, 0 );
    ImplGetEnglishSearchFontName( aOrgName );

    // #93662# do not try to replace StarSymbol with MS only font
    if( nFlags == (SUBSFONT_MS|SUBSFONT_ONLYONE)
    &&  ( aOrgName.EqualsAscii( "starsymbol" )
      ||  aOrgName.EqualsAscii( "opensymbol" ) ) )
        return aName;

    const FontSubstConfigItem::FontNameAttr* pAttr = FontSubstConfigItem::get()->getSubstInfo( aOrgName );
    if ( pAttr )
    {
        for( int i = 0; i < 3; i++ )
        {
            const ::std::vector< String >* pVector = NULL;
            switch( i )
            {
                case 0:
                    if( nFlags & SUBSFONT_MS  &&  pAttr->MSSubstitutions.size() )
                        pVector = &pAttr->MSSubstitutions;
                    break;
                case 1:
                    if( nFlags & SUBSFONT_PS  &&  pAttr->PSSubstitutions.size() )
                        pVector = &pAttr->PSSubstitutions;
                    break;
                case 2:
                    if( nFlags & SUBSFONT_HTML  &&  pAttr->HTMLSubstitutions.size() )
                        pVector = &pAttr->HTMLSubstitutions;
                    break;
            }
            if( ! pVector )
                continue;
            for( ::std::vector< String >::const_iterator it = pVector->begin(); it != pVector->end(); ++it )
                if( ! ImplIsFontToken( rName, *it ) )
                {
                    ImplAppendFontToken( aName, *it );
                    if( nFlags & SUBSFONT_ONLYONE )
                    {
                        i = 4;
                        break;
                    }
                }
        }
    }

    return aName;
}

// =======================================================================

static unsigned ImplIsCJKFont( const String& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    const sal_Unicode* pStr = rFontName.GetBuffer();
    while ( *pStr )
    {
        // japanese
        if ( ((*pStr >= 0x3040) && (*pStr <= 0x30FF)) ||
             ((*pStr >= 0x3190) && (*pStr <= 0x319F)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_JP;

        // korean
        if ( ((*pStr >= 0xAC00) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0x3130) && (*pStr <= 0x318F)) ||
             ((*pStr >= 0x1100) && (*pStr <= 0x11FF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_KR;

        // chinese
        if ( ((*pStr >= 0x3400) && (*pStr <= 0x9FFF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_TC|IMPL_FONT_ATTR_CJK_SC;

        // cjk
        if ( ((*pStr >= 0x3000) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0xFF00) && (*pStr <= 0xFFEE)) )
            return IMPL_FONT_ATTR_CJK;

        pStr++;
    }

    return 0;
}

// -----------------------------------------------------------------------

static void ImplCalcType( ULONG& rType, FontWeight& rWeight, FontWidth& rWidth,
                          FontFamily eFamily, const FontSubstConfigItem::FontNameAttr* pFontAttr )
{
    if ( eFamily != FAMILY_DONTKNOW )
    {
        if ( eFamily == FAMILY_SWISS )
            rType |= IMPL_FONT_ATTR_SANSSERIF;
        else if ( eFamily == FAMILY_ROMAN )
            rType |= IMPL_FONT_ATTR_SERIF;
        else if ( eFamily == FAMILY_SCRIPT )
            rType |= IMPL_FONT_ATTR_SCRIPT;
        else if ( eFamily == FAMILY_MODERN )
            rType |= IMPL_FONT_ATTR_FIXED;
        else if ( eFamily == FAMILY_DECORATIVE )
            rType |= IMPL_FONT_ATTR_DECORATIVE;
    }

    if ( pFontAttr )
    {
        rType |= pFontAttr->Type;

        if ( ((rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL)) &&
             (pFontAttr->Weight != WEIGHT_DONTKNOW) )
            rWeight = pFontAttr->Weight;
        if ( ((rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL)) &&
             (pFontAttr->Width != WIDTH_DONTKNOW) )
            rWidth = pFontAttr->Width;
    }
}

// =======================================================================

ImplDevFontList::ImplDevFontList() : List( CONTAINER_MAXBLOCKSIZE, 96, 32 )
{
    mbMatchData = FALSE;
    mbMapNames  = FALSE;
    mpFallbackList  = NULL;
    mnFallbackCount = -1;
}

// -----------------------------------------------------------------------

ImplDevFontList::~ImplDevFontList()
{
    ImplClear();
}

// -----------------------------------------------------------------------

void ImplDevFontList::ImplClear()
{
    // clear all entries in the device font list
    ImplFontSelectData aIFSD;
    for( ImplDevFontListData* pEntry = First(); pEntry; pEntry = Next() )
    {
        for( ImplFontData* pFontData = pEntry->mpFirst; pFontData; )
        {
            ImplFontData* pNextFD = pFontData->mpNext;

            // tell lower layers about the imminent death
#ifdef UNX
            GlyphCache::GetInstance().RemoveFont( pFontData );
#endif
            delete pFontData;

            pFontData = pNextFD;
        }

        delete pEntry;
    }

    // remove fallback lists
    delete[] mpFallbackList;
    mpFallbackList = NULL;
    mnFallbackCount = -1;

    // match data must be recalculated too
    mbMatchData = FALSE;
}

// -----------------------------------------------------------------------

void ImplDevFontList::Clear()
{
    ImplClear();
    List::Clear();
}

// -----------------------------------------------------------------------

bool ImplDevFontList::HasFallbacks() const
{
    return (mnFallbackCount >= 0);
}

// -----------------------------------------------------------------------

void ImplDevFontList::SetFallbacks( ImplDevFontListData** pList, int nCount )
{
    mnFallbackCount = nCount;
    mpFallbackList  = pList;
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::GetFallback( int nIndex ) const
{
    if( nIndex < mnFallbackCount )
        return mpFallbackList[ nIndex ];
    return NULL;
}

// -----------------------------------------------------------------------

static StringCompare ImplCompareFontDataWithoutSize( const ImplFontData* pEntry1,
                                                     const ImplFontData* pEntry2 )
{
    // compare their width, weight, italic and style name
    if ( pEntry1->meWidthType < pEntry2->meWidthType )
        return COMPARE_LESS;
    else if ( pEntry1->meWidthType > pEntry2->meWidthType )
        return COMPARE_GREATER;

    if ( pEntry1->meWeight < pEntry2->meWeight )
        return COMPARE_LESS;
    else if ( pEntry1->meWeight > pEntry2->meWeight )
        return COMPARE_GREATER;

    if ( pEntry1->meItalic < pEntry2->meItalic )
        return COMPARE_LESS;
    else if ( pEntry1->meItalic > pEntry2->meItalic )
        return COMPARE_GREATER;

    StringCompare eCompare = pEntry1->maName.CompareTo( pEntry2->maName );
    if ( eCompare == COMPARE_EQUAL )
        eCompare = pEntry1->maStyleName.CompareTo( pEntry2->maStyleName );

    return eCompare;
}


// -----------------------------------------------------------------------

static StringCompare ImplCompareFontData( const ImplFontData* pEntry1,
                                          const ImplFontData* pEntry2 )
{
    StringCompare eComp = ImplCompareFontDataWithoutSize( pEntry1, pEntry2 );
    if ( eComp != COMPARE_EQUAL )
        return eComp;

    if ( pEntry1->mnHeight < pEntry2->mnHeight )
        return COMPARE_LESS;
    else if ( pEntry1->mnHeight > pEntry2->mnHeight )
        return COMPARE_GREATER;

    if ( pEntry1->mnWidth < pEntry2->mnWidth )
        return COMPARE_LESS;
    else if ( pEntry1->mnWidth > pEntry2->mnWidth )
        return COMPARE_GREATER;

    return COMPARE_EQUAL;
}

// -----------------------------------------------------------------------

void ImplDevFontList::Add( ImplFontData* pNewData )
{
    XubString aSearchName = pNewData->maName;
    ImplGetEnglishSearchFontName( aSearchName );

    // add font
    ULONG                   nIndex;
    ImplDevFontListData*    pFoundData = ImplFind( aSearchName, &nIndex );
    BOOL                    bInsert = TRUE;

    if ( !pFoundData )
    {
        pFoundData                  = new ImplDevFontListData;
        pFoundData->maName          = pNewData->maName;
        pFoundData->maSearchName    = aSearchName;
        pFoundData->mpFirst         = pNewData;
        pFoundData->meFamily        = pNewData->meFamily;
        pFoundData->mePitch         = pNewData->mePitch;
        pFoundData->mnTypeFaces     = 0;
        pFoundData->meMatchWeight   = WEIGHT_DONTKNOW;
        pFoundData->meMatchWidth    = WIDTH_DONTKNOW;
        pFoundData->mnMatchType     = 0;
        pNewData->mpNext            = NULL;
        Insert( pFoundData, nIndex );
        bInsert = FALSE;
    }
    else
    {
        if ( pFoundData->meFamily == FAMILY_DONTKNOW )
            pFoundData->meFamily = pNewData->meFamily;
        if ( pFoundData->mePitch == PITCH_DONTKNOW )
            pFoundData->mePitch = pNewData->mePitch;
    }

    // set match data
    if ( (pNewData->meType == TYPE_SCALABLE) && (pNewData->mnHeight == 0) )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_SCALABLE;
    if ( pNewData->meCharSet == RTL_TEXTENCODING_SYMBOL )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_SYMBOL;
    else
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_NONESYMBOL;
    if ( pNewData->meWeight != WEIGHT_DONTKNOW )
    {
        if ( pNewData->meWeight >= WEIGHT_SEMIBOLD )
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_BOLD;
        else if ( pNewData->meWeight <= WEIGHT_SEMILIGHT )
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_LIGHT;
        else
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_NORMAL;
    }
    if ( pNewData->meItalic == ITALIC_NONE )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_NONEITALIC;
    else if ( (pNewData->meItalic == ITALIC_NORMAL) ||
              (pNewData->meItalic == ITALIC_OBLIQUE) )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_ITALIC;

    // TODO: is it cheaper to calc match data now or later?
    if( (pFoundData->meMatchWeight == WEIGHT_DONTKNOW)
    ||  (pFoundData->meMatchWidth  == WIDTH_DONTKNOW)
    ||  (pFoundData->mnMatchType   == 0) )
        mbMatchData = FALSE;

    // add map/alias names
    if ( pNewData->maMapNames.Len() )
    {
        String      aName;
        xub_StrLen  nIndex = 0;
        do
        {
            aName = GetFontToken( pNewData->maMapNames, 0, nIndex );
            ImplGetEnglishSearchFontName( aName );
            if ( aName != aSearchName )
            {
                ImplAddTokenFontName( pFoundData->maMapNames, aName );
                mbMapNames = TRUE;
            }
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    if ( bInsert )
    {
        // reassign name (sharing saves memory)
        if ( pNewData->maName == pFoundData->maName )
            pNewData->maName = pFoundData->maName;

        ImplFontData*   pPrev = NULL;
        ImplFontData*   pTemp = pFoundData->mpFirst;
        do
        {
            StringCompare eComp = ImplCompareFontData( pNewData, pTemp );
            if ( eComp != COMPARE_GREATER )
            {
                // prefer device font, else remove duplicate
                if ( eComp == COMPARE_EQUAL )
                {
                    // prefer font with better quality
                    // else prefer device font
                    if ( (pNewData->mnQuality > pTemp->mnQuality) ||
                         ((pNewData->mnQuality == pTemp->mnQuality) &&
                          (pNewData->mbDevice && !pTemp->mbDevice)) )
                    {
                        pNewData->mpNext = pTemp->mpNext;
                        if ( pPrev )
                            pPrev->mpNext = pNewData;
                        else
                            pFoundData->mpFirst = pNewData;
                        delete pTemp;
                    }
                    else
                        delete pNewData;

                    bInsert = FALSE;
                }
                break;
            }

            pPrev = pTemp;
            pTemp = pTemp->mpNext;
        }
        while ( pTemp );

        if ( bInsert )
        {
            pNewData->mpNext = pTemp;
            if ( pPrev )
                pPrev->mpNext = pNewData;
            else
                pFoundData->mpFirst = pNewData;
        }
    }
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::ImplFind( const XubString& rFontName, ULONG* pIndex ) const
{
#ifdef DBG_UTIL
    String aTempName = rFontName;
    ImplGetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplDevFontList::ImplFind() called without a searchname" );
#endif

    ULONG nCount = Count();
    if ( !nCount )
    {
        if ( pIndex )
            *pIndex = LIST_APPEND;
        return NULL;
    }

    // find fonts in font list
    ImplDevFontListData*    pCompareData;
    ImplDevFontListData*    pFoundData = NULL;
    ULONG                   nLow = 0;
    ULONG                   nHigh = nCount-1;
    ULONG                   nMid;
    StringCompare           eCompare;

    do
    {
        nMid = (nLow + nHigh) / 2;
        pCompareData = Get( nMid );
        eCompare = rFontName.CompareTo( pCompareData->maSearchName );
        if ( eCompare == COMPARE_LESS )
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if ( eCompare == COMPARE_GREATER )
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
        eCompare = rFontName.CompareTo( pCompareData->maSearchName );
        if ( eCompare == COMPARE_GREATER )
            *pIndex = (nMid+1);
        else
            *pIndex = nMid;
    }

    return pFoundData;
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::FindFont( const XubString& rFontName ) const
{
    XubString aName = rFontName;
    ImplGetEnglishSearchFontName( aName );
    return ImplFind( aName );
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::ImplFindFontFromToken( const String& rStr ) const
{
    xub_StrLen nIndex = 0;
    while( nIndex != STRING_NOTFOUND )
    {
        String aName( rStr.GetToken( 0, ';', nIndex ) );
        if ( aName.Len() )
        {
            ImplDevFontListData* pData = ImplFind( aName );
            if ( pData )
                return pData;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------

void ImplDevFontList::InitMatchData()
{
    if ( mbMatchData )
        return;

    // Calculate MatchData for all Entries
    ImplDevFontListData* pEntry = First();
    const FontSubstConfigItem* pFontSubst = FontSubstConfigItem::get();
    while ( pEntry )
    {
        // Get all information about the matching font
        const FontSubstConfigItem::FontNameAttr* pTempFontAttr;
        String                  aTempShortName;
        pFontSubst->getMapName( pEntry->maSearchName, aTempShortName, pEntry->maMatchFamilyName,
                                pEntry->meMatchWeight, pEntry->meMatchWidth, pEntry->mnMatchType );
        pTempFontAttr = FontSubstConfigItem::get()->getSubstInfo( pEntry->maSearchName );
        if ( !pTempFontAttr && (aTempShortName != pEntry->maSearchName) )
            pTempFontAttr = pFontSubst->getSubstInfo( aTempShortName );
        ImplCalcType( pEntry->mnMatchType, pEntry->meMatchWeight, pEntry->meMatchWidth,
                      pEntry->meFamily, pTempFontAttr );
        pEntry->mnMatchType |= ImplIsCJKFont( pEntry->maName );

        pEntry = Next();
    }

    mbMatchData = TRUE;
}

// =======================================================================

void ImplGetDevSizeList::Add( long nNewHeight )
{
    ULONG n = Count();
    if ( !n || (nNewHeight > Get( n-1 )) )
        Insert( (void*)nNewHeight, LIST_APPEND );
    else
    {
        for ( ULONG i=0 ; i < n; i++ )
        {
            long nHeight = Get( i );

            if ( nNewHeight <= nHeight )
            {
                if ( nNewHeight != nHeight )
                    Insert( (void*)nNewHeight, i );
                break;
            }
        }
    }
}

// =======================================================================

ImplFontEntry::~ImplFontEntry()
{
    delete[] mpKernPairs;
}

// =======================================================================

ImplFontCache::ImplFontCache( BOOL bPrinter )
{
    mpFirstEntry    = NULL;
    mnRef0Count     = 0;
    mbPrinter       = bPrinter;
}

// -----------------------------------------------------------------------

ImplFontCache::~ImplFontCache()
{
    for( ImplFontEntry* pEntry = mpFirstEntry; pEntry; )
    {
        ImplFontEntry* pNext = pEntry->mpNext;
        delete pEntry;
        pEntry = pNext;
    }
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplFontCache::Get( ImplDevFontList* pFontList,
                                   const Font& rFont, const Size& rSize, ImplFontSubstEntry* pDevSpecific )
{
    String aName                = rFont.GetName();
    const XubString& rStyleName = rFont.GetStyleName();
    long nWidth                 = rSize.Width();
    long nHeight                = rSize.Height();
    FontFamily eFamily          = rFont.GetFamily();
    CharSet eCharSet            = rFont.GetCharSet();
    LanguageType eLanguage      = rFont.GetLanguage();
    FontWeight eWeight          = rFont.GetWeight();
    FontItalic eItalic          = rFont.GetItalic();
    FontPitch ePitch            = rFont.GetPitch();
    short nOrientation          = rFont.GetOrientation();
    BOOL bVertical              = rFont.IsVertical();

    // normalize orientation between 0 and 3600
    if ( nOrientation )
    {
        while( nOrientation < 0 )
            nOrientation += 3600;
        nOrientation %= 3600;
    }

    // correct size
    if ( nHeight < 0 )
        nHeight = -nHeight;
    if ( nWidth < 0 )
        nWidth = -nWidth;

    // find entry
    ImplFontEntry* pPrevEntry = NULL;
    ImplFontEntry* pEntry = mpFirstEntry;
    while ( pEntry )
    {
        ImplFontSelectData* pFontSelData = &(pEntry->maFontSelData);
        if ( !(pEntry->mnSetFontFlags & SAL_SETFONT_BADFONT)    // avoid known bad fonts
        &&   (nHeight       == pFontSelData->mnHeight)    &&
             (eWeight       == pFontSelData->meWeight)    &&
             (eItalic       == pFontSelData->meItalic)    &&
             ((aName == pFontSelData->maName) || (aName == pFontSelData->maFoundName)) &&
             (rStyleName    == pFontSelData->maStyleName) &&
             (eFamily       == pFontSelData->meFamily)    &&
             (ePitch        == pFontSelData->mePitch)     &&
             (nWidth        == pFontSelData->mnWidth)     &&
             (eCharSet      == pFontSelData->meCharSet)   &&
             (eLanguage     == pFontSelData->meLanguage)  &&
             (bVertical     == pFontSelData->mbVertical)  &&
             (nOrientation  == pFontSelData->mnOrientation) )
        {
            if ( !pEntry->mnRefCount++ )
                mnRef0Count--;

            // entry becomes head of list
            if ( pPrevEntry )
            {
                pPrevEntry->mpNext = pEntry->mpNext;
                pEntry->mpNext = mpFirstEntry;
                mpFirstEntry = pEntry;
            }

            return pEntry;
        }

        pPrevEntry = pEntry;
        pEntry = pEntry->mpNext;
    }

    const ImplCvtChar*      pConvertFontTab = NULL;
    ImplFontData*           pFontData = NULL;

    ImplDevFontListData*    pFoundData;
    String                  aSearchName;
    USHORT                  nSubstFlags1 = FONT_SUBSTITUTE_ALWAYS;
    USHORT                  nSubstFlags2 = FONT_SUBSTITUTE_ALWAYS;
    xub_StrLen              nFirstNameIndex = 0;
    xub_StrLen              nIndex = 0;
    int                     nToken = 0;
    ULONG                   i;

    if ( mbPrinter )
        nSubstFlags1 |= FONT_SUBSTITUTE_SCREENONLY;

    // Test if one Font in the name list is available
    do
    {
        nToken++;
        String aToken = GetFontToken( aName, 0, nIndex );
        aSearchName = aToken;
        ImplGetEnglishSearchFontName( aSearchName );
        ImplFontSubstitute( aSearchName, nSubstFlags1, nSubstFlags2, pDevSpecific );
        pFoundData = pFontList->ImplFind( aSearchName );
        if( pFoundData )
        {
            aName = aToken;
            break;
        }
    }
    while ( nIndex != STRING_NOTFOUND );

    // Danach versuchen wir es nocheinmal unter Beruecksichtigung
    // der gloablen Fontersetzungstabelle, wobei wir jetzt auch
    // die Fonts nehmen, die ersetzt werden sollen, wenn sie
    // nicht vorhanden sind
    if ( !pFoundData )
    {
        nSubstFlags1 &= ~FONT_SUBSTITUTE_ALWAYS;
        nSubstFlags2 &= ~FONT_SUBSTITUTE_ALWAYS;
        nIndex = 0;
        do
        {
            if ( nToken > 1 )
            {
                aSearchName = GetFontToken( aName, 0, nIndex );
                ImplGetEnglishSearchFontName( aSearchName );
            }
            else
                nIndex = STRING_NOTFOUND;
            ImplFontSubstitute( aSearchName, nSubstFlags1, nSubstFlags2, pDevSpecific );
            pFoundData = pFontList->ImplFind( aSearchName );
            if( pFoundData )
                break;
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    ULONG nFontCount = pFontList->Count();
    if ( !pFoundData && nFontCount )
    {
        // Wenn kein Font mit dem entsprechenden Namen existiert, versuchen
        // wir ueber den Namen und die Attribute einen passenden Font zu
        // finden - wir nehmen dazu das erste Token
        if ( nToken > 1 )
        {
            nIndex = 0;
            aSearchName = GetFontToken( aName, 0, nIndex );
            ImplGetEnglishSearchFontName( aSearchName );
        }

        const FontSubstConfigItem::FontNameAttr* pFontAttr = NULL;
        const FontSubstConfigItem* pFontSubst = FontSubstConfigItem::get();
        String                  aSearchShortName;
        String                  aSearchFamilyName;
        ULONG                   nSearchType = 0;
        FontWeight              eSearchWeight = eWeight;
        FontWidth               eSearchWidth = rFont.GetWidthType();
        BOOL                    bSymbolEncoding = (eCharSet == RTL_TEXTENCODING_SYMBOL);
        FontSubstConfigItem::getMapName( aSearchName, aSearchShortName, aSearchFamilyName,
                                         eSearchWeight, eSearchWidth, nSearchType );

        // note: the search name was already translated to english (if possible)

        // Search, if ShortName is available
        if ( aSearchShortName != aSearchName )
        {
#ifdef UNX
            /* #96738# don't use mincho as an replacement for "MS Mincho" on X11: Mincho is
               a korean bitmap font that is not suitable here. Use the font replacement table,
               that automatically leads to the desired "HG Mincho Light J". Same story for
               MS Gothic, there are thai and korean "Gothic" fonts, so we even prefer Andale */
            static String aMS_Mincho = String(RTL_CONSTASCII_USTRINGPARAM("msmincho"));
            static String aMS_Gothic = String(RTL_CONSTASCII_USTRINGPARAM("msgothic"));
            if ((aSearchName != aMS_Mincho) && (aSearchName != aMS_Gothic))
#endif
            pFoundData = pFontList->ImplFind( aSearchShortName );
        }
        if ( !pFoundData && aSearchName.Len() )
        {
            // look for substitution tables for the name, shortname and family name in that order
            pFontAttr = pFontSubst->getSubstInfo( aSearchName );
            if ( !pFontAttr && (aSearchShortName != aSearchName) )
                pFontAttr = pFontSubst->getSubstInfo( aSearchShortName );
            if ( !pFontAttr && (aSearchFamilyName != aSearchShortName) )
                pFontAttr = pFontSubst->getSubstInfo( aSearchFamilyName );

            // Try Substitution
            if ( pFontAttr )
            {
                for( ::std::vector< String >::const_iterator it = pFontAttr->Substitutions.begin();
                     ! pFoundData && it != pFontAttr->Substitutions.end(); ++it )
                {
                    String aFontname( *it );
                    // list may contain localized names -> translate
                    ImplGetEnglishSearchFontName( aFontname );
                    pFoundData = pFontList->ImplFind( aFontname );
                }
            }
        }

        if ( !pFoundData && bSymbolEncoding )
        {
            String aFontname = DefaultFontConfigItem::get()->getDefaultFont( LANGUAGE_ENGLISH, DEFAULTFONT_SYMBOL );
            ImplGetEnglishSearchFontName( aFontname );
            pFoundData = pFontList->ImplFindFontFromToken( aFontname );
        }

        // If we haven't found a font, we try this with the other Font Token names, if availble
        if ( !pFoundData && (nToken > 1) )
        {
            while ( nIndex != STRING_NOTFOUND )
            {
                const FontSubstConfigItem::FontNameAttr* pTempFontAttr;
                String                  aTempName = GetFontToken( aName, 0, nIndex );
                String                  aTempShortName;
                String                  aTempFamilyName;
                ULONG                   nTempType = 0;
                FontWeight              eTempWeight = eWeight;
                FontWidth               eTempWidth = WIDTH_DONTKNOW;
                ImplGetEnglishSearchFontName( aTempName );
                FontSubstConfigItem::getMapName( aTempName, aTempShortName, aTempFamilyName,
                                                 eTempWeight, eTempWidth, nTempType );

                // Temp, if ShortName is available
                if ( aTempShortName != aTempName )
                    pFoundData = pFontList->ImplFind( aTempShortName );

                if ( !pFoundData && aTempName.Len() )
                {
                    // look for substitution tables for the name, shortname and family name in that order
                    pTempFontAttr = pFontSubst->getSubstInfo( aTempName );
                    if ( !pTempFontAttr && (aTempShortName != aTempName) )
                        pTempFontAttr = pFontSubst->getSubstInfo( aTempShortName );
                    if ( !pTempFontAttr && (aTempFamilyName != aTempShortName) )
                        pTempFontAttr = pFontSubst->getSubstInfo( aTempFamilyName );

                    // Try Substitution
                    if ( pTempFontAttr )
                    {
                        for( ::std::vector< String >::const_iterator it = pTempFontAttr->Substitutions.begin();
                             ! pFoundData && it != pTempFontAttr->Substitutions.end(); ++it )
                        {
                            String aFontname( *it );
                            // list may contain localized names -> translate
                            ImplGetEnglishSearchFontName( aFontname );
                            pFoundData = pFontList->ImplFind( aFontname );
                        }
                    }
                }
            }
        }

        // Try to find the font over the alias names
        if ( !pFoundData && pFontList->AreMapNamesAvailable() && aSearchName.Len() )
        {
            i = 0;
            do
            {
                ImplDevFontListData* pData = pFontList->Get( i );
                i++;

                // Test mapping names
                // If match one matching name, than this is our font!
                if ( pData->maMapNames.Len() )
                {
                    String      aTempName;
                    xub_StrLen  nIndex = 0;
                    do
                    {
                        aTempName = GetFontToken( pData->maMapNames, 0, nIndex );
                        // Test, if the Font name match with one of the mapping names
                        if ( (aTempName == aSearchName) || (aTempName == aSearchShortName) )
                        {
                            // Found - we use this font
                            pFoundData = pData;
                            break;
                        }
                    }
                    while ( nIndex != STRING_NOTFOUND );
                }
            }
            while ( !pFoundData && (i < nFontCount) );
        }

        // If we haven't found a font over the name, we try to find the best match over the attributes
        if ( !pFoundData && aSearchName.Len() )
        {
            ImplCalcType( nSearchType, eSearchWeight, eSearchWidth,
                          eFamily, pFontAttr );
            if ( (eItalic != ITALIC_NONE) && (eItalic != ITALIC_DONTKNOW) )
                nSearchType |= IMPL_FONT_ATTR_ITALIC;
            LanguageType eLang = rFont.GetLanguage();
            if ( (eLang == LANGUAGE_CHINESE) ||
                 (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC;
            else if ( (eLang == LANGUAGE_CHINESE_TRADITIONAL) ||
                      (eLang == LANGUAGE_CHINESE_HONGKONG) ||
                      (eLang == LANGUAGE_CHINESE_MACAU) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC;
            else if ( (eLang == LANGUAGE_KOREAN) ||
                      (eLang == LANGUAGE_KOREAN_JOHAB) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR;
            else if ( eLang == LANGUAGE_JAPANESE )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP;
            else
                nSearchType |= ImplIsCJKFont( aName );
            if ( bSymbolEncoding )
                nSearchType |= IMPL_FONT_ATTR_SYMBOL;

            // We must only match, if we have something to match
            if ( nSearchType ||
                 ((eSearchWeight != WEIGHT_DONTKNOW) && (eSearchWeight != WEIGHT_NORMAL)) ||
                 ((eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL)) )
            {
                pFontList->InitMatchData();

                long    nTestMatch;
                long    nBestMatch = 40000;
                ULONG   nBestType = 0;
                for ( ULONG i = 0; i < nFontCount; i++ )
                {
                    ImplDevFontListData* pData = pFontList->Get( i );

                    // Get all information about the matching font
                    ULONG       nMatchType = pData->mnMatchType;
                    FontWeight  eMatchWeight = pData->meMatchWeight;
                    FontWidth   eMatchWidth = pData->meMatchWidth;

                    // Calculate Match Value
                    // 1000000000
                    //  100000000
                    //   10000000   CJK, CTL, None-Latin, Symbol
                    //    1000000   FamilyName, Script, Fixed, -Special, -Decorative,
                    //              Titling, Capitals, Outline, Shadow
                    //     100000   Match FamilyName, Serif, SansSerif, Italic,
                    //              Width, Weight
                    //      10000   Scalable, Standard, Default,
                    //              full, Normal, Knownfont,
                    //              Otherstyle, +Special, +Decorative,
                    //       1000   Typewriter, Rounded, Gothic, Schollbook
                    //        100
                    nTestMatch = 0;

                    // Test, if the choosen font should be CJK, CTL, None-Latin
                    if ( nSearchType & IMPL_FONT_ATTR_CJK )
                    {
                        // Matching language
                        if ( (nSearchType & IMPL_FONT_ATTR_CJK_ALLLANG) ==
                             (nMatchType & IMPL_FONT_ATTR_CJK_ALLLANG) )
                            nTestMatch += 10000000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_CJK )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_CJK )
                        nTestMatch -= 10000000;

                    if ( nSearchType & IMPL_FONT_ATTR_CTL )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_CTL )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_CTL )
                        nTestMatch -= 10000000;

                    if ( nSearchType & IMPL_FONT_ATTR_NONELATIN )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_NONELATIN )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_SYMBOL )
                    {
                        // prefer some special known symbol fonts
                        if ( pData->maSearchName.EqualsAscii( "starsymbol" ) )
                            nTestMatch += 10000000*6+(10000*3);
                        else if ( pData->maSearchName.EqualsAscii( "opensymbol" ) )
                            nTestMatch += 10000000*6;
                        else if ( pData->maSearchName.EqualsAscii( "starbats" ) ||
                                  pData->maSearchName.EqualsAscii( "wingdings" ) ||
                                  pData->maSearchName.EqualsAscii( "monotypesorts" ) ||
                                  pData->maSearchName.EqualsAscii( "dingbats" ) ||
                                  pData->maSearchName.EqualsAscii( "zapfdingbats" ) )
                            nTestMatch += 10000000*5;
                        else if ( pData->mnTypeFaces & IMPL_DEVFONT_SYMBOL )
                            nTestMatch += 10000000*4;
                        else
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                                nTestMatch += 10000000*2;
                            if ( nMatchType & IMPL_FONT_ATTR_FULL )
                                nTestMatch += 10000000;
                        }
                    }
                    else if ( (pData->mnTypeFaces & (IMPL_DEVFONT_SYMBOL | IMPL_DEVFONT_NONESYMBOL)) == IMPL_DEVFONT_SYMBOL )
                        nTestMatch -= 10000000;
                    else if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                        nTestMatch -= 10000;

                    if ( (aSearchFamilyName == pData->maMatchFamilyName) &&
                         aSearchFamilyName.Len() )
                        nTestMatch += 1000000*3;

                    if ( nSearchType & IMPL_FONT_ATTR_ALLSCRIPT )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
                        {
                            nTestMatch += 1000000*2;
                            if ( nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT )
                            {
                                if ( (nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT) ==
                                     (nMatchType & IMPL_FONT_ATTR_ALLSUBSCRIPT) )
                                    nTestMatch += 1000000*2;
                                if ( (nSearchType & IMPL_FONT_ATTR_BRUSHSCRIPT) &&
                                     !(nMatchType & IMPL_FONT_ATTR_BRUSHSCRIPT) )
                                    nTestMatch -= 1000000;
                            }
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
                        nTestMatch -= 1000000;

                    if ( nSearchType & IMPL_FONT_ATTR_FIXED )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_FIXED )
                        {
                            nTestMatch += 1000000*2;
                            // Typewriter has now a higher prio
                            if ( (nSearchType & IMPL_FONT_ATTR_TYPEWRITER) &&
                                 (nMatchType & IMPL_FONT_ATTR_TYPEWRITER) )
                                 nTestMatch += 10000*2;
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_FIXED )
                        nTestMatch -= 1000000;

                    if ( nSearchType & IMPL_FONT_ATTR_SPECIAL )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SPECIAL )
                            nTestMatch += 10000;
                        else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                                nTestMatch += 1000*2;
                            else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                                nTestMatch += 1000;
                        }
                    }
                    else if ( (nMatchType & IMPL_FONT_ATTR_SPECIAL) &&
                              !(nSearchType & IMPL_FONT_ATTR_SYMBOL) )
                        nTestMatch -= 1000000;
                    if ( nSearchType & IMPL_FONT_ATTR_DECORATIVE )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
                            nTestMatch += 10000;
                        else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                                nTestMatch += 1000*2;
                            else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                                nTestMatch += 1000;
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
                        nTestMatch -= 1000000;

                    if ( nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                    {
                        if ( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                            nTestMatch += 1000000*2;
                        if ( (nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) ==
                             (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) )
                            nTestMatch += 1000000;
                        else if ( (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) &&
                                  (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                            nTestMatch += 1000000;
                    }
                    else if ( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                        nTestMatch -= 1000000;
                    if ( nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                    {
                        if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                            nTestMatch += 1000000*2;
                        if ( (nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) ==
                             (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) )
                            nTestMatch += 1000000;
                        else if ( (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) &&
                                  (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                            nTestMatch += 1000000;
                    }
                    else if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                        nTestMatch -= 1000000;

                    if ( (aSearchFamilyName.Len() && pData->maMatchFamilyName.Len()) &&
                         ((aSearchFamilyName.Search( pData->maMatchFamilyName ) != STRING_NOTFOUND) ||
                          (pData->maMatchFamilyName.Search( aSearchFamilyName ) != STRING_NOTFOUND)) )
                        nTestMatch += 100000*2;

                    if ( nSearchType & IMPL_FONT_ATTR_SERIF )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch += 1000000*2;
                        else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch -= 1000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_SANSSERIF )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch += 1000000;
                        else if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch -= 1000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_ITALIC )
                    {
                        if ( pData->mnTypeFaces & IMPL_DEVFONT_ITALIC )
                            nTestMatch += 1000000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_ITALIC )
                            nTestMatch += 1000000;
                    }
                    else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSCRIPT) &&
                              ((nMatchType & IMPL_FONT_ATTR_ITALIC) ||
                               !(pData->mnTypeFaces & IMPL_DEVFONT_NONEITALIC)) )
                        nTestMatch -= 1000000*2;

                    if ( (eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL) )
                    {
                        if ( eSearchWidth < WIDTH_NORMAL )
                        {
                            if ( eSearchWidth == eMatchWidth )
                                nTestMatch += 1000000*3;
                            else if ( (eMatchWidth < WIDTH_NORMAL) && (eMatchWidth != WIDTH_DONTKNOW) )
                                nTestMatch += 1000000;
                        }
                        else
                        {
                            if ( eSearchWidth == eMatchWidth )
                                nTestMatch += 1000000*3;
                            else if ( eMatchWidth > WIDTH_NORMAL )
                                nTestMatch += 1000000;
                        }
                    }
                    else if ( (eMatchWidth != WIDTH_DONTKNOW) && (eMatchWidth != WIDTH_NORMAL) )
                        nTestMatch -= 1000000;

                    if ( (eSearchWeight != WEIGHT_DONTKNOW) && (eSearchWeight != WEIGHT_NORMAL) && (eSearchWeight != WEIGHT_MEDIUM) )
                    {
                        if ( eSearchWeight < WEIGHT_NORMAL )
                        {
                            if ( pData->mnTypeFaces & IMPL_DEVFONT_LIGHT )
                                nTestMatch += 1000000;
                            if ( (eMatchWeight < WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_DONTKNOW) )
                                nTestMatch += 1000000;
                        }
                        else
                        {
                            if ( pData->mnTypeFaces & IMPL_DEVFONT_BOLD )
                                nTestMatch += 1000000;
                            if ( eMatchWeight > WEIGHT_BOLD )
                                nTestMatch += 1000000;
                        }
                    }
                    else if ( ((eMatchWeight != WEIGHT_DONTKNOW) && (eMatchWeight != WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_MEDIUM)) ||
                              !(pData->mnTypeFaces & IMPL_DEVFONT_NORMAL) )
                        nTestMatch -= 1000000;

                    // prefer scalable fonts
                    if ( pData->mnTypeFaces & IMPL_DEVFONT_SCALABLE )
                        nTestMatch += 10000*4;
                    else
                        nTestMatch -= 10000*4;
                    if ( nMatchType & IMPL_FONT_ATTR_STANDARD )
                        nTestMatch += 10000*2;
                    if ( nMatchType & IMPL_FONT_ATTR_DEFAULT )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_FULL )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_NORMAL )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_OTHERSTYLE )
                    {
                        if ( !(nMatchType & IMPL_FONT_ATTR_OTHERSTYLE) )
                            nTestMatch -= 10000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_OTHERSTYLE )
                        nTestMatch -= 10000;

                    if ( (nSearchType & IMPL_FONT_ATTR_ROUNDED) ==
                         (nMatchType & IMPL_FONT_ATTR_ROUNDED) )
                         nTestMatch += 1000;
                    if ( (nSearchType & IMPL_FONT_ATTR_TYPEWRITER) ==
                         (nMatchType & IMPL_FONT_ATTR_TYPEWRITER) )
                         nTestMatch += 1000;
                    if ( nSearchType & IMPL_FONT_ATTR_GOTHIC )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_GOTHIC )
                            nTestMatch += 1000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch += 1000*2;
                    }
                    if ( nSearchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                            nTestMatch += 1000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch += 1000*2;
                    }

                    if ( nTestMatch > nBestMatch )
                    {
                        pFoundData  = pData;
                        nBestMatch  = nTestMatch;
                        nBestType   = nMatchType;
                    }
                    else if ( nTestMatch == nBestMatch )
                    {
                        // some fonts are more suitable defaults
                        if ( nMatchType & IMPL_FONT_ATTR_DEFAULT )
                        {
                            pFoundData  = pData;
                            nBestType   = nMatchType;
                        }
                        else if ( (nMatchType & IMPL_FONT_ATTR_STANDARD) &&
                                  !(nBestType & IMPL_FONT_ATTR_DEFAULT) )
                        {
                            pFoundData  = pData;
                            nBestType   = nMatchType;
                        }
                    }
                }

                // Overwrite Attributes
                if ( pFoundData )
                {
                    if ( (eSearchWeight >= WEIGHT_BOLD) &&
                         (eSearchWeight > eWeight) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_BOLD) )
                        eWeight = eSearchWeight;
                    if ( (eSearchWeight < WEIGHT_NORMAL) &&
                         (eSearchWeight < eWeight) &&
                         (eSearchWeight != WEIGHT_DONTKNOW) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_LIGHT) )
                        eWeight = eSearchWeight;
                    if ( (nSearchType & IMPL_FONT_ATTR_ITALIC) &&
                         ((eItalic == ITALIC_DONTKNOW) || (eItalic == ITALIC_NONE)) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_ITALIC) )
                        eItalic = ITALIC_NORMAL;
                }
            }
        }

        if ( !pFoundData )
        {
            // Try to use a Standard Unicode or a Standard Font to get
            // as max as possible characters
            DefaultFontConfigItem* pDefaults = DefaultFontConfigItem::get();
            String aFontname = pDefaults->getDefaultFont( LANGUAGE_ENGLISH, DEFAULTFONT_SANS_UNICODE );
            ImplGetEnglishSearchFontName( aFontname );

            pFoundData = pFontList->ImplFindFontFromToken( aFontname );
            if ( !pFoundData )
            {
                aFontname = pDefaults->getDefaultFont( LANGUAGE_ENGLISH, DEFAULTFONT_SANS );
                ImplGetEnglishSearchFontName( aFontname );
                pFoundData = pFontList->ImplFindFontFromToken( aFontname );
                if ( !pFoundData )
                {
                    aFontname = pDefaults->getDefaultFont( LANGUAGE_ENGLISH, DEFAULTFONT_SERIF );
                    ImplGetEnglishSearchFontName( aFontname );
                    pFoundData = pFontList->ImplFindFontFromToken( aFontname );
                    if ( !pFoundData )
                    {
                        aFontname = pDefaults->getDefaultFont( LANGUAGE_ENGLISH, DEFAULTFONT_FIXED );
                        ImplGetEnglishSearchFontName( aFontname );
                        pFoundData = pFontList->ImplFindFontFromToken( aFontname );
                    }
                }
            }

            // if nothing helps, we try to use a reasonable non-symbol font
            if ( !pFoundData )
            {
                pFontList->InitMatchData();
                for( i = 0; i < nFontCount; ++i )
                {
                    ImplDevFontListData* pData = pFontList->Get( i );
                    if( pData->mnMatchType & IMPL_FONT_ATTR_SYMBOL )
                        continue;
                    pFoundData = pData;
                    if( pData->mnMatchType & (IMPL_FONT_ATTR_DEFAULT|IMPL_FONT_ATTR_STANDARD) )
                        break;
                }
                // if there was no reasonable font just use the first in the font list
                if( i >= nFontCount )
                    pFoundData = pFontList->Get( 0 );
            }
        }
    }

    // we found a useable Font, than we look which font maps best
    // with the requested attributes
    if ( pFoundData )
    {
        ULONG           nBestMatch = 0;         // Der groessere Wert ist der bessere
        ULONG           nBestHeightMatch = 0;   // Der kleinere Wert ist der bessere
        ULONG           nBestWidthMatch = 0;    // Der kleinere Wert ist der bessere
        ULONG           nMatch;
        ULONG           nHeightMatch;
        ULONG           nWidthMatch;
        ImplFontData*   pCurFontData;

        // FontName+StyleName should map to FamilyName+StyleName
        const xub_Unicode* pCompareStyleName = NULL;
        if ( (aSearchName.Len() > pFoundData->maSearchName.Len()) &&
             aSearchName.Equals( pFoundData->maSearchName, 0, pFoundData->maSearchName.Len() ) )
            pCompareStyleName = aSearchName.GetBuffer()+pFoundData->maSearchName.Len()+1;

        pCurFontData = pFoundData->mpFirst;
        while ( pCurFontData )
        {
            nMatch = 0;
            nHeightMatch = 0;
            nWidthMatch = 0;

            if ( (aName == pCurFontData->maName) ||
                 aName.EqualsIgnoreCaseAscii( pCurFontData->maName ) )
                nMatch += 240000;

            if ( pCompareStyleName &&
                 pCurFontData->maStyleName.EqualsIgnoreCaseAscii( pCompareStyleName ) )
                nMatch += 120000;

            if ( (ePitch != PITCH_DONTKNOW) && (ePitch == pCurFontData->mePitch) )
                nMatch += 60000;

            if ( (eFamily != FAMILY_DONTKNOW) && (eFamily == pCurFontData->meFamily) )
                nMatch += 30000;

            // prefer NORMAL font width
            // TODO: change when the upper layers can tell their preference
            if ( pCurFontData->meWidthType == WIDTH_NORMAL )
                nMatch += 15000;

            if ( eWeight != WEIGHT_DONTKNOW )
            {
                USHORT nReqWeight;
                USHORT nGivenWeight;
                USHORT nWeightDiff;
                // schmale Fonts werden bei nicht Bold vor fetten
                // Fonts bevorzugt
                if ( eWeight > WEIGHT_MEDIUM )
                    nReqWeight = ((USHORT)eWeight)+100;
                else
                    nReqWeight = (USHORT)eWeight;
                if ( pCurFontData->meWeight > WEIGHT_MEDIUM )
                    nGivenWeight = ((USHORT)pCurFontData->meWeight)+100;
                else
                    nGivenWeight = (USHORT)pCurFontData->meWeight;
                if ( nReqWeight > nGivenWeight )
                    nWeightDiff = nReqWeight-nGivenWeight;
                else
                    nWeightDiff = nGivenWeight-nReqWeight;

                if ( nWeightDiff == 0 )
                    nMatch += 1000;
                else if ( nWeightDiff == 1 )
                    nMatch += 700;
                else if ( nWeightDiff < 50 )
                    nMatch += 200;
            }
            if ( eItalic == ITALIC_NONE )
            {
                if ( pCurFontData->meItalic == ITALIC_NONE )
                    nMatch += 900;
            }
            else
            {
                if ( eItalic == pCurFontData->meItalic )
                    nMatch += 900;
                else if ( pCurFontData->meItalic != ITALIC_NONE )
                    nMatch += 600;
            }

            if ( pCurFontData->mbDevice )
                nMatch += 40;
            if ( pCurFontData->meType == TYPE_SCALABLE )
            {
                if ( nOrientation )
                    nMatch += 80;
                else
                {
                    if ( nWidth )
                        nMatch += 25;
                    else
                        nMatch += 5;
                }
            }
            else
            {
                if ( nHeight == pCurFontData->mnHeight )
                {
                    nMatch += 20;
                    if ( nWidth == pCurFontData->mnWidth )
                        nMatch += 10;
                }
                else
                {
                    // Dann kommt die Size-Abweichung in die
                    // Bewertung rein. Hier bevorzugen wir
                    // nach Moeglichkeit den kleineren Font
                    if ( nHeight < pCurFontData->mnHeight )
                        nHeightMatch += pCurFontData->mnHeight-nHeight;
                    else
                        nHeightMatch += (nHeight-pCurFontData->mnHeight-nHeight)+10000;
                    if ( nWidth && pCurFontData->mnWidth && (nWidth != pCurFontData->mnWidth) )
                    {
                        if ( nWidth < pCurFontData->mnWidth )
                            nWidthMatch += pCurFontData->mnWidth-nWidth;
                        else
                            nWidthMatch += nWidth-pCurFontData->mnWidth-nWidth;
                    }
                }
            }

            if ( nMatch > nBestMatch )
            {
                pFontData           = pCurFontData;
                nBestMatch          = nMatch;
                nBestHeightMatch    = nHeightMatch;
                nBestWidthMatch     = nWidthMatch;
            }
            else if ( nMatch == nBestMatch )
            {
                // when two fonts are still competing prefer the
                // one with the closest height
                if ( nHeightMatch < nBestHeightMatch )
                {
                    pFontData           = pCurFontData;
                    nBestHeightMatch    = nHeightMatch;
                    nBestWidthMatch     = nWidthMatch;
                }
                else if ( nHeightMatch == nBestHeightMatch )
                {
                    if ( nWidthMatch < nBestWidthMatch )
                    {
                        pFontData       = pCurFontData;
                        nBestWidthMatch = nWidthMatch;
                    }
                }
            }

            pCurFontData = pCurFontData->mpNext;
        }

        // we have not found the font => try to use a font-conversion table
        if ( aSearchName != pFoundData->maSearchName )
            pConvertFontTab = ImplGetRecodeData( aSearchName, pFoundData->maSearchName );
    }

    // add a new entry into the cache
    pEntry                          = new ImplFontEntry;
    pEntry->mpNext                  = mpFirstEntry;
    pEntry->mpConversion            = pConvertFontTab;
    pEntry->mnWidthInit             = 0;
    pEntry->mpKernPairs             = NULL;
    pEntry->mnOwnOrientation        = 0;
    pEntry->mnOrientation           = 0;
    pEntry->mbInit                  = FALSE;
    pEntry->mnSetFontFlags          = 0;
    pEntry->mnRefCount              = 1;
    mpFirstEntry                    = pEntry;

    // initialize font selection data
    ImplFontSelectData& rFontSelData = pEntry->maFontSelData;
    rFontSelData.mpFontData        = pFontData;
    rFontSelData.maName            = rFont.GetName();
    rFontSelData.maFoundName       = aName;
    rFontSelData.maStyleName       = rStyleName;
    rFontSelData.mnWidth           = nWidth;
    rFontSelData.mnHeight          = nHeight;
    rFontSelData.meFamily          = eFamily;
    rFontSelData.meCharSet         = eCharSet;
    rFontSelData.meLanguage        = eLanguage;
    rFontSelData.meWidthType       = WIDTH_DONTKNOW;
    rFontSelData.meWeight          = eWeight;
    rFontSelData.meItalic          = eItalic;
    rFontSelData.mePitch           = ePitch;
    rFontSelData.mnOrientation     = nOrientation;
    rFontSelData.mbVertical        = bVertical;
    rFontSelData.mbNonAntialiased  = FALSE;

    return pEntry;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplFontCache::GetFallback( ImplDevFontList* pFontList,
    const Font& rOrigFont, const Size& rSize, int nLevel, ImplFontSubstEntry* pDevSpecific )
{
    // make sure the fontlist knows it's fallbacks
    if( !pFontList->HasFallbacks() )
    {
        // TODO: implement dynamic lists or improve static lists
        #define FALLBACKFONT_NAMELIST \
            "Arial Unicode MS;Andale Sans UI;Cyberbit;StarSymbol;Lucida TypeWriter;"  \
            "FZMingTi;SunBatang;SunDotum;"                                           \
            "HGMinchoLightJ;MSungLightSC;MSungLightTC;HYMyeongJoLight K;" \
            "Lucida Sans;Tahoma;"                       \
            "Shree;Mangal;Raavi;Shruti;Tunga;Latha;"    \
            "Shayyal MT;Nask MT;David;"                 \
            "Norasi;AngsanaUPC;"                                   \
            "Gulim;Batang;Dotum;MS Mincho"
        String aNameList( RTL_CONSTASCII_USTRINGPARAM(FALLBACKFONT_NAMELIST) );
        int nMaxLevel = 0;
        ImplDevFontListData** pFallbackList = NULL;
        for( xub_StrLen nTokenPos = 0; nTokenPos != STRING_NOTFOUND; )
        {
            String aTokenName = GetFontToken( aNameList, 0, nTokenPos );
            ImplGetEnglishSearchFontName( aTokenName );
            //ImplFontSubstitute( aSearchName, nSubstFlags1, nSubstFlags2 );
            ImplDevFontListData* pFoundData = pFontList->ImplFind( aTokenName );
            // TODO: check FontCharset and reject if it is already covered
            if( pFoundData && (pFoundData->mpFirst->meType == TYPE_SCALABLE) )
            {
                if( !pFallbackList )
                    pFallbackList = new ImplDevFontListData*[ MAX_FALLBACK ];
                pFallbackList[ nMaxLevel ] = pFoundData;
                if( ++nMaxLevel >= MAX_FALLBACK )
                    break;
            }
        }

        pFontList->SetFallbacks( pFallbackList, nMaxLevel );
    }

    // nLevel>=1 for fallback => remove +1 offset
    ImplDevFontListData* pFallbackData = pFontList->GetFallback( nLevel-1 );
    if( !pFallbackData )
        return NULL;

    // TODO: use pFallbackData directly
    Font aFallbackFont = rOrigFont;
    aFallbackFont.SetName( pFallbackData->maName );
    ImplFontEntry* pFallbackFont = Get( pFontList, aFallbackFont, rSize, pDevSpecific );

    if( pFallbackFont && !pFallbackFont->mbInit )
    {
        // HACK: maMetrics are irrelevant for fallback fonts, but
        // to prevent trouble at cleanup we need to set some members
        pFallbackFont->maMetric.maName      =
        pFallbackFont->maMetric.maStyleName = pFallbackData->maName;
    }

    return pFallbackFont;
}

// -----------------------------------------------------------------------

void ImplFontCache::Release( ImplFontEntry* pEntry )
{
    static const int FONTCACHE_MIN = 5;
    static const int FONTCACHE_MAX = 50;

    if( --pEntry->mnRefCount > 0 )
        return;

    if( ++mnRef0Count < FONTCACHE_MAX )
        return;

    // delete least-recently-used unreferenced entries
    int mnRef0Index = 0;
    for( ImplFontEntry** pNextPtr = &mpFirstEntry; pEntry = *pNextPtr; )
    {
        if( (pEntry->mnRefCount > 0) || (++mnRef0Index < FONTCACHE_MAX) )
            pNextPtr = &pEntry->mpNext;
        else
        {
            *pNextPtr = pEntry->mpNext;
            delete pEntry;
            --mnRef0Count;
        }
    }

    DBG_ASSERT( (mnRef0Index<=FONTCACHE_MAX), "ImplFontCache::Release() - mismatch" );
}

// -----------------------------------------------------------------------

void ImplFontCache::Clear()
{
    // delete unreferenced entries
    ImplFontEntry** pNextPtr = &mpFirstEntry;
    while( ImplFontEntry* pEntry = *pNextPtr )
    {
        if( pEntry->mnRefCount > 0 )
            pNextPtr = &pEntry->mpNext;
        else
        {
            *pNextPtr = pEntry->mpNext;
            delete pEntry;
            --mnRef0Count;
        }
    }

    DBG_ASSERT( (mnRef0Count==0), "ImplFontCache::Clear() - mnRef0Count non-zero" );
}

// =======================================================================

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
    mpLines = new PImplTextLineInfo[MULTITEXTLINEINFO_RESIZE];
    mnLines = 0;
    mnSize  = MULTITEXTLINEINFO_RESIZE;
}


ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
    for ( xub_StrLen i = 0; i < mnLines; i++ )
        delete mpLines[i];
    delete [] mpLines;
}

void ImplMultiTextLineInfo::AddLine( ImplTextLineInfo* pLine )
{
    if ( mnSize == mnLines )
    {
        mnSize += MULTITEXTLINEINFO_RESIZE;
        PImplTextLineInfo* pNewLines = new PImplTextLineInfo[mnSize];
        memcpy( pNewLines, mpLines, mnLines*sizeof(PImplTextLineInfo) );
        mpLines = pNewLines;
    }

    mpLines[mnLines] = pLine;
    mnLines++;
}

void ImplMultiTextLineInfo::Clear()
{
    for ( xub_StrLen i = 0; i < mnLines; i++ )
        delete mpLines[i];
    mnLines = 0;
}

// =======================================================================

FontEmphasisMark OutputDevice::ImplGetEmphasisMarkStyle( const Font& rFont )
{
    FontEmphasisMark nEmphasisMark = rFont.GetEmphasisMark();

    // If no Position is set, then calculate the default position, which
    // depends on the language
    if ( !(nEmphasisMark & (EMPHASISMARK_POS_ABOVE | EMPHASISMARK_POS_BELOW)) )
    {
        LanguageType eLang = rFont.GetLanguage();
        // In Chinese Simplified the EmphasisMarks are below/left
        if ( (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
             (eLang == LANGUAGE_CHINESE_SINGAPORE) )
            nEmphasisMark |= EMPHASISMARK_POS_BELOW;
        else
        {
            eLang = rFont.GetCJKContextLanguage();
            // In Chinese Simplified the EmphasisMarks are below/left
            if ( (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                nEmphasisMark |= EMPHASISMARK_POS_BELOW;
            else
                nEmphasisMark |= EMPHASISMARK_POS_ABOVE;
        }
    }

    return nEmphasisMark;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::ImplIsUnderlineAbove( const Font& rFont )
{
    if ( !rFont.IsVertical() )
        return FALSE;

    if( (LANGUAGE_JAPANESE == rFont.GetLanguage())
    ||  (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()) )
        // the underline is right for Japanese only
        return TRUE;

    return FALSE;
}

// =======================================================================

void OutputDevice::ImplInitFontList() const
{
    if( ! mpFontList->Count() )
    {
        if( mpGraphics || ImplGetGraphics() )
        {
            mpGraphics->GetDevFontList( mpFontList );
            if( !mpFontList->Count() )
                // dummy mode, no fonts at all
                // at least we should not crash or loop
                return;
        }
    }
}

// =======================================================================

void OutputDevice::ImplInitFont()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitFont )
    {
        if ( meOutDevType != OUTDEV_PRINTER )
        {
            // decide if antialiasing is appropriate
            bool bNonAntialiased = (GetAntialiasing() & ANTIALIASING_DISABLE_TEXT) != 0;
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            bNonAntialiased |= ((rStyleSettings.GetDisplayOptions() & DISPLAY_OPTION_AA_DISABLE) != 0);
            bNonAntialiased |= (rStyleSettings.GetAntialiasingMinPixelHeight() > mpFontEntry->maFontSelData.mnHeight);
            mpFontEntry->maFontSelData.mbNonAntialiased = (BOOL)bNonAntialiased;
        }

        if( !mpPDFWriter || !mpPDFWriter->isBuiltinFont( mpFontEntry->maFontSelData.mpFontData ) )
        {
            // Select Font
            mpFontEntry->mnSetFontFlags = mpGraphics->SetFont( &(mpFontEntry->maFontSelData), 0 );
        }
        mbInitFont = FALSE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitTextColor )
    {
#ifndef REMOTE_APPSERVER
        mpGraphics->SetTextColor( ImplColorToSal( GetTextColor() ) );
#else
        mpGraphics->SetTextColor( GetTextColor() );
#endif
        mbInitTextColor = FALSE;
    }
}

// -----------------------------------------------------------------------

int OutputDevice::ImplNewFont()
{
    DBG_TESTSOLARMUTEX();

    // get correct font list on the PDF writer if necessary
    ImplSVData* pSVData = ImplGetSVData();
    if( ( mpFontList == pSVData->maGDIData.mpScreenFontList ||
        mpFontCache == pSVData->maGDIData.mpScreenFontCache  )
        && mpPDFWriter )
    {
        ImplUpdateFontData( TRUE );
    }

    if ( !mbNewFont )
        return TRUE;

    mbNewFont = FALSE;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics && !ImplGetGraphics() )
        return FALSE;
    SalGraphics* pGraphics = mpGraphics;
#else
    // due to clipping we may get NULL, so don't use return value
    ImplGetServerGraphics();
    ImplServerGraphics* pGraphics = mpGraphics;
#endif
    ImplInitFontList();

    // convert to pixel height
    Size aSize = ImplLogicToDevicePixel( maFont.GetSize() );
    if ( !aSize.Height() )
    {
        // use default pixel height only when logical height is zero
        if ( maFont.GetSize().Height() )
            aSize.Height() = 1;
        else
            aSize.Height() = (12*mnDPIY)/72;
    }

    // use default width only when logical width is zero
    if( (0 == aSize.Width()) && (0 != maFont.GetSize().Width()) )
        aSize.Width() = 1;

    // get font entry
    ImplFontEntry* pOldEntry = mpFontEntry;
    mpFontEntry = mpFontCache->Get( mpFontList, maFont, aSize, mpOutDevData ? mpOutDevData->mpFirstFontSubstEntry : NULL );
    ImplFontEntry* pFontEntry = mpFontEntry;

    // do lower layers need notification
    if ( pFontEntry != pOldEntry )
        mbInitFont = TRUE;

    // these two may be filled in remote version
    ImplKernPairData* pKernPairs = NULL;
    long nKernPairs = 0;
    // select font when it has not been initialized yet
    if ( !pFontEntry->mbInit )
    {
        ImplInitFont();

        // get font data
        if ( pGraphics )
        {
            pFontEntry->mbInit = TRUE;

            pFontEntry->maMetric.mnAscent       = 0;
            pFontEntry->maMetric.mnDescent      = 0;
            pFontEntry->maMetric.mnIntLeading   = 0;
            pFontEntry->maMetric.mnExtLeading   = 0;
            pFontEntry->maMetric.mnSlant        = 0;
            pFontEntry->maMetric.mnFirstChar    = 0;
            pFontEntry->maMetric.mnLastChar     = 0;
            pFontEntry->maMetric.mnWidth        = pFontEntry->maFontSelData.mnWidth;
            pFontEntry->maMetric.meFamily       = pFontEntry->maFontSelData.meFamily;
            pFontEntry->maMetric.meCharSet      = pFontEntry->maFontSelData.meCharSet;
            pFontEntry->maMetric.meWeight       = pFontEntry->maFontSelData.meWeight;
            pFontEntry->maMetric.meItalic       = pFontEntry->maFontSelData.meItalic;
            pFontEntry->maMetric.mePitch        = pFontEntry->maFontSelData.mePitch;
            pFontEntry->maMetric.mnOrientation  = pFontEntry->maFontSelData.mnOrientation;
            if ( pFontEntry->maFontSelData.mpFontData )
            {
                pFontEntry->maMetric.meType     = pFontEntry->maFontSelData.mpFontData->meType;
                pFontEntry->maMetric.maName     = pFontEntry->maFontSelData.mpFontData->maName;
                pFontEntry->maMetric.maStyleName= pFontEntry->maFontSelData.mpFontData->maStyleName;
                pFontEntry->maMetric.mbDevice   = pFontEntry->maFontSelData.mpFontData->mbDevice;
            }
            else
            {
                pFontEntry->maMetric.meType     = TYPE_DONTKNOW;
                pFontEntry->maMetric.maName     = GetFontToken( pFontEntry->maFontSelData.maName, 0 );
                pFontEntry->maMetric.maStyleName= pFontEntry->maFontSelData.maStyleName;
                pFontEntry->maMetric.mbDevice   = FALSE;
            }
            pFontEntry->maMetric.mnUnderlineSize            = 0;
            pFontEntry->maMetric.mnUnderlineOffset          = 0;
            pFontEntry->maMetric.mnBUnderlineSize           = 0;
            pFontEntry->maMetric.mnBUnderlineOffset         = 0;
            pFontEntry->maMetric.mnDUnderlineSize           = 0;
            pFontEntry->maMetric.mnDUnderlineOffset1        = 0;
            pFontEntry->maMetric.mnDUnderlineOffset2        = 0;
            pFontEntry->maMetric.mnWUnderlineSize           = 0;
            pFontEntry->maMetric.mnWUnderlineOffset         = 0;
            pFontEntry->maMetric.mnAboveUnderlineSize       = 0;
            pFontEntry->maMetric.mnAboveUnderlineOffset     = 0;
            pFontEntry->maMetric.mnAboveBUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveBUnderlineOffset    = 0;
            pFontEntry->maMetric.mnAboveDUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveDUnderlineOffset1   = 0;
            pFontEntry->maMetric.mnAboveDUnderlineOffset2   = 0;
            pFontEntry->maMetric.mnAboveWUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveWUnderlineOffset    = 0;
            pFontEntry->maMetric.mnStrikeoutSize            = 0;
            pFontEntry->maMetric.mnStrikeoutOffset          = 0;
            pFontEntry->maMetric.mnBStrikeoutSize           = 0;
            pFontEntry->maMetric.mnBStrikeoutOffset         = 0;
            pFontEntry->maMetric.mnDStrikeoutSize           = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset1        = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset2        = 0;

            if( mpPDFWriter && mpPDFWriter->isBuiltinFont( pFontEntry->maFontSelData.mpFontData ) )
                mpPDFWriter->getFontMetric( &pFontEntry->maFontSelData, &(pFontEntry->maMetric) );
            else
            {
#ifndef REMOTE_APPSERVER
                pGraphics->GetFontMetric( &(pFontEntry->maMetric) );
#else
                long nFactor = 0;
                pGraphics->GetFontMetric(
                                         pFontEntry->maMetric, nFactor,
                                         0x21, 0x20, NULL,
                                         (maFont.GetKerning() & KERNING_FONTSPECIFIC) != 0,
                                         &pKernPairs, nKernPairs );
#endif
            }

            pFontEntry->mbFixedFont     = pFontEntry->maMetric.mePitch == PITCH_FIXED;
            pFontEntry->mnLineHeight    = pFontEntry->maMetric.mnAscent + pFontEntry->maMetric.mnDescent;
            pFontEntry->mbInitKernPairs = FALSE;
            pFontEntry->mnKernPairs     = nKernPairs;

            if( pFontEntry->maFontSelData.mnOrientation
            && !pFontEntry->maMetric.mnOrientation
            && (meOutDevType != OUTDEV_PRINTER) )
            {
                pFontEntry->mnOwnOrientation = pFontEntry->maFontSelData.mnOrientation;
                pFontEntry->mnOrientation = pFontEntry->mnOwnOrientation;
            }
            else
                pFontEntry->mnOrientation = pFontEntry->maMetric.mnOrientation;
        }
    }

    // get kerning array if requested
    if ( maFont.GetKerning() & KERNING_FONTSPECIFIC )
    {
        ImplInitKerningPairs( pKernPairs, nKernPairs );
        mbKerning = (pFontEntry->mnKernPairs) != 0;
    }
    else
        mbKerning = FALSE;
    if ( maFont.GetKerning() & KERNING_ASIAN )
        mbKerning = TRUE;

    // calculate EmphasisArea
    mnEmphasisAscent = 0;
    mnEmphasisDescent = 0;
    if ( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
        long                nEmphasisHeight = (pFontEntry->mnLineHeight*250)/1000;
        if ( nEmphasisHeight < 1 )
            nEmphasisHeight = 1;
        if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
            mnEmphasisDescent = nEmphasisHeight;
        else
            mnEmphasisAscent = nEmphasisHeight;
    }

    // calculate text offset depending on TextAlignment
    TextAlign eAlign = maFont.GetAlign();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        mnTextOffX = 0;
        mnTextOffY = +pFontEntry->maMetric.mnAscent + mnEmphasisAscent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontEntry->maMetric.mnDescent + mnEmphasisDescent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }

    mbTextLines     = ((maFont.GetUnderline() != UNDERLINE_NONE) && (maFont.GetUnderline() != UNDERLINE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline() ||
                      (maFont.GetRelief() != RELIEF_NONE);

    if ( pOldEntry )
        mpFontCache->Release( pOldEntry );

    // #95414# fix for OLE objects which use scale factors very creatively
    if( mbMap && !aSize.Width() )
    {
        int nOrigWidth = pFontEntry->maMetric.mnWidth;
        float fStretch = (float)maMapRes.mnMapScNumX * maMapRes.mnMapScDenomY;
        fStretch /= (float)maMapRes.mnMapScNumY * maMapRes.mnMapScDenomX;
        int nNewWidth = (int)(nOrigWidth * fStretch + 0.5);
        if( (nNewWidth != nOrigWidth) && (nNewWidth != 0) )
        {
            Size aOrigSize = maFont.GetSize();
            maFont.SetSize( Size( nNewWidth, aSize.Height() ) );
            mbMap = FALSE;
            mbNewFont = TRUE;
            ImplNewFont();  // recurse once using stretched width
            mbMap = TRUE;
            maFont.SetSize( aOrigSize );
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------

inline bool CmpKernData( const ImplKernPairData& a, const ImplKernPairData& b )
{
    return (*(sal_uInt32*)&a) > (*(sal_uInt32*)&b);
}

static void ImplSortKernPairs( ImplKernPairData* pKernPairs, long l, long r )
{
#if 1 // TODO: use STL's insertion sort
    long                i = l;

    long                j = r;
    ImplKernPairData*   pComp = pKernPairs + ((l+r) >> 1);
    sal_uInt32          nComp = *((sal_uInt32*)pComp);

    do
    {
        while ( *((sal_uInt32*)(pKernPairs+i)) < nComp )
            i++;
        while ( nComp < *((sal_uInt32*)(pKernPairs+j)) )
            j--;
        if ( i <= j )
        {
            ImplKernPairData aTemp = *(pKernPairs+i);
            *(pKernPairs+i) = *(pKernPairs+j);
            *(pKernPairs+j) = aTemp;
            i++;
            j--;
        }
    }
    while ( i <= j );

    if ( l < j )
        ImplSortKernPairs( pKernPairs, l, j );
    if ( i < r )
        ImplSortKernPairs( pKernPairs, i, r );
#else
    std::sort( pKernPairs+l, pKernPairs+r, CmpKernData );
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitKerningPairs( ImplKernPairData* pKernPairs, long nKernPairs )
{
    if( mbNewFont && !ImplNewFont() )
        return;

    ImplFontEntry* pFontEntry = mpFontEntry;
    if ( !pFontEntry->mbInitKernPairs )
    {
        if ( mbInitFont )
            ImplInitFont();
        pFontEntry->mbInitKernPairs = TRUE;
        if( mpPDFWriter && mpPDFWriter->isBuiltinFont( mpFontEntry->maFontSelData.mpFontData ) )
        {
            pFontEntry->mnKernPairs = 0;
            pFontEntry->mpKernPairs = NULL;
            return;
        }
#ifndef REMOTE_APPSERVER
        pFontEntry->mnKernPairs = mpGraphics->GetKernPairs( 0, NULL );
        if ( pFontEntry->mnKernPairs )
        {
            pKernPairs = new ImplKernPairData[pFontEntry->mnKernPairs];
            memset( pKernPairs, 0, sizeof(ImplKernPairData)*pFontEntry->mnKernPairs );
            pFontEntry->mnKernPairs = mpGraphics->GetKernPairs( pFontEntry->mnKernPairs, pKernPairs );
            pFontEntry->mpKernPairs = pKernPairs;
        }
#else
        if ( !pKernPairs )
            nKernPairs = mpGraphics->GetKernPairs( &pKernPairs );
        if ( nKernPairs )
            pFontEntry->mpKernPairs = pKernPairs;
#endif

        // Sort Kerning Pairs
        if ( pFontEntry->mpKernPairs )
            ImplSortKernPairs( pFontEntry->mpKernPairs, 0, (long)pFontEntry->mnKernPairs-1 );
    }
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetTextWidth( const SalLayout& rSalLayout ) const
{
    long nWidth = rSalLayout.GetTextWidth();
    nWidth /= rSalLayout.GetUnitsPerPixel();
    return nWidth;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextRect( long nBaseX, long nBaseY,
                                     long nX, long nY, long nWidth, long nHeight )
{
    short nOrientation = mpFontEntry->mnOrientation;
    if ( nOrientation )
    {
        // Rotate rect without rounding problems for 90 degree rotations
        if ( !(nOrientation % 900) )
        {
            nX -= nBaseX;
            nY -= nBaseY;

            if ( nOrientation == 900 )
            {
                long nTemp = nX;
                nX = nY;
                nY = -nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nY -= nHeight;
            }
            else if ( nOrientation == 1800 )
            {
                nX = -nX;
                nY = -nY;
                nX -= nWidth;
                nY -= nHeight;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = nX;
                nX = -nY;
                nY = nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nX -= nWidth;
            }

            nX += nBaseX;
            nY += nBaseY;
        }
        else
        {
            // inflate because polygons are drawn smaller
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
#ifndef REMOTE_APPSERVER
            ImplDrawPolygon( aPoly );
#else
            mpGraphics->DrawPolygon( aPoly );
#endif
            return;
        }
    }

#ifndef REMOTE_APPSERVER
    mpGraphics->DrawRect( nX, nY, nWidth, nHeight, this );
#else
    Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
    mpGraphics->DrawRect( aRect );
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextBackground( const SalLayout& rSalLayout )
{
    long nWidth = rSalLayout.GetTextWidth();
    Point aBase = rSalLayout.GetDrawPosition();
    long nX = aBase.X();
    long nY = aBase.Y();

#ifndef REMOTE_APPSERVER
    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = TRUE;
    }
    mpGraphics->SetFillColor( ImplColorToSal( GetTextFillColor() ) );
    mbInitFillColor = TRUE;

    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent-mnEmphasisAscent,
                      nWidth,
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
#else
    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();
    SetLineColor();
    SetFillColor( GetTextFillColor() );
    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();
    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent-mnEmphasisAscent,
                      nWidth,
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
#endif
}

// -----------------------------------------------------------------------

Rectangle OutputDevice::ImplGetTextBoundRect( const SalLayout& rSalLayout )
{
    Point aPoint = rSalLayout.GetDrawPosition();
    long nX = aPoint.X();
    long nY = aPoint.Y();

    long nWidth = rSalLayout.GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    nY -= mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;

    if ( mpFontEntry->mnOrientation )
    {
        long nBaseX = nX, nBaseY = nY;
        if ( !(mpFontEntry->mnOrientation % 900) )
        {
            long nX2 = nX+nWidth;
            long nY2 = nY+nHeight;
            ImplRotatePos( nBaseX, nBaseY, nX, nY, mpFontEntry->mnOrientation );
            ImplRotatePos( nBaseX, nBaseY, nX2, nY2, mpFontEntry->mnOrientation );
            nWidth = nX2-nX;
            nHeight = nY2-nY;
        }
        else
        {
            // inflate by +1+1 because polygons are drawn smaller
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            return aPoly.GetBoundRect();
        }
    }

    return Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitTextLineSize()
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLineHeight2;
    long            nBLineHeight;
    long            nBLineHeight2;

    long            n2LineHeight;
    long            n2LineDY;
    long            n2LineDY2;
    long            nUnderlineOffset;
    long            nStrikeoutOffset;
    long            nDescent;

    nDescent = pFontEntry->maMetric.mnDescent;
    if ( !nDescent )
    {
        nDescent = pFontEntry->maMetric.mnAscent*100/1000;
        if ( !nDescent )
            nDescent = 1;
    }

    nLineHeight = ((nDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    nBLineHeight = ((nDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    n2LineHeight = ((nDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    n2LineDY = n2LineHeight;
    if ( n2LineDY <= 0 )
        n2LineDY = 1;
    n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    nUnderlineOffset = nDescent/2 + 1;
    nStrikeoutOffset = -((pFontEntry->maMetric.mnAscent-pFontEntry->maMetric.mnIntLeading)/3);

    if ( !pFontEntry->maMetric.mnUnderlineSize )
    {
        pFontEntry->maMetric.mnUnderlineSize        = nLineHeight;
        pFontEntry->maMetric.mnUnderlineOffset      = nUnderlineOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnBUnderlineSize )
    {
        pFontEntry->maMetric.mnBUnderlineSize       = nBLineHeight;
        pFontEntry->maMetric.mnBUnderlineOffset     = nUnderlineOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDUnderlineSize )
    {
        pFontEntry->maMetric.mnDUnderlineSize       = n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset1    = nUnderlineOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset2    = pFontEntry->maMetric.mnDUnderlineOffset1 + n2LineDY + n2LineHeight;
    }
    if ( !pFontEntry->maMetric.mnWUnderlineSize )
    {
        long nWCalcSize = pFontEntry->maMetric.mnDescent;
        if ( nWCalcSize < 6 )
        {
            if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
                pFontEntry->maMetric.mnWUnderlineSize = nWCalcSize;
            else
                pFontEntry->maMetric.mnWUnderlineSize = 3;
        }
        else
            pFontEntry->maMetric.mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;
         // #109280# the following line assures that wavelnes are never placed below the descent, however
         // for most fonts the waveline then is drawn into the text, so we better keep the old solution
         // pFontEntry->maMetric.mnWUnderlineOffset     = pFontEntry->maMetric.mnDescent + 1 - pFontEntry->maMetric.mnWUnderlineSize;
        pFontEntry->maMetric.mnWUnderlineOffset     = nUnderlineOffset;
    }

    if ( !pFontEntry->maMetric.mnStrikeoutSize )
    {
        pFontEntry->maMetric.mnStrikeoutSize        = nLineHeight;
        pFontEntry->maMetric.mnStrikeoutOffset      = nStrikeoutOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnBStrikeoutSize )
    {
        pFontEntry->maMetric.mnBStrikeoutSize       = nBLineHeight;
        pFontEntry->maMetric.mnBStrikeoutOffset     = nStrikeoutOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDStrikeoutSize )
    {
        pFontEntry->maMetric.mnDStrikeoutSize       = n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset1    = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset2    = pFontEntry->maMetric.mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitAboveTextLineSize()
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLineHeight2;
    long            nBLineHeight;
    long            nBLineHeight2;
    long            n2LineHeight;
    long            n2LineDY;
    long            n2LineDY2;
    long            nUnderlineOffset;

    long nIntLeading = pFontEntry->maMetric.mnIntLeading;
    // TODO: assess usage of nLeading below (changed in extleading CWS)
    if ( !nIntLeading )
    {
        // if no leading is available, we assume 15% of the ascent
        nIntLeading = pFontEntry->maMetric.mnAscent*150/1000;
        if ( !nIntLeading )
            nIntLeading = 1;
    }

    nLineHeight = ((nIntLeading*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    nBLineHeight = ((nIntLeading*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    n2LineHeight = ((nIntLeading*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    n2LineDY = n2LineHeight;
    if ( n2LineDY <= 0 )
        n2LineDY = 1;
    n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    nUnderlineOffset = -(pFontEntry->maMetric.mnAscent-((nIntLeading/2)-1));

    if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveUnderlineSize       = nLineHeight;
        pFontEntry->maMetric.mnAboveUnderlineOffset     = nUnderlineOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveBUnderlineSize      = nBLineHeight;
        pFontEntry->maMetric.mnAboveBUnderlineOffset    = nUnderlineOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveDUnderlineSize      = n2LineHeight;
        pFontEntry->maMetric.mnAboveDUnderlineOffset1   = nUnderlineOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnAboveDUnderlineOffset2   = pFontEntry->maMetric.mnAboveDUnderlineOffset1 + n2LineDY + n2LineHeight;
    }
    if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
    {
        long nWCalcSize = nIntLeading;
        if ( nWCalcSize < 6 )
        {
            if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
                pFontEntry->maMetric.mnAboveWUnderlineSize = nWCalcSize;
            else
                pFontEntry->maMetric.mnAboveWUnderlineSize = 3;
        }
        else
            pFontEntry->maMetric.mnAboveWUnderlineSize = ((nWCalcSize*50)+50) / 100;

        pFontEntry->maMetric.mnAboveWUnderlineOffset    = nUnderlineOffset;
    }
}

// -----------------------------------------------------------------------

static void ImplDrawWavePixel( long nOriginX, long nOriginY,
                               long nCurX, long nCurY,
                               short nOrientation,
#ifndef REMOTE_APPSERVER
                               SalGraphics* pGraphics,
#else
                               ImplServerGraphics* pGraphics,
#endif
                               OutputDevice* pOutDev,
                               BOOL bDrawPixAsRect,

                               long nPixWidth, long nPixHeight )
{
    if ( nOrientation )
        ImplRotatePos( nOriginX, nOriginY, nCurX, nCurY, nOrientation );

    if ( bDrawPixAsRect )
    {
#ifndef REMOTE_APPSERVER

        pGraphics->DrawRect( nCurX, nCurY, nPixWidth, nPixHeight, pOutDev );
#else
        Point       aPos( nCurX, nCurY );
        Size        aSize( nPixWidth, nPixHeight );
        Rectangle   aRect( aPos, aSize );
        pGraphics->DrawRect( aRect );
#endif
    }
    else
    {
#ifndef REMOTE_APPSERVER
        pGraphics->DrawPixel( nCurX, nCurY, pOutDev );
#else
        Point aPos( nCurX, nCurY );
        pGraphics->DrawPixel( aPos );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawWaveLine( long nBaseX, long nBaseY,
                                     long nStartX, long nStartY,
                                     long nWidth, long nHeight,
                                     long nLineWidth, short nOrientation,
                                     const Color& rColor )
{
    if ( !nHeight )
        return;

    // Bei Hoehe von 1 Pixel reicht es, eine Linie auszugeben
    if ( (nLineWidth == 1) && (nHeight == 1) )
    {
#ifndef REMOTE_APPSERVER
        mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
        mbInitLineColor = TRUE;
#else
        Color aOldLineColor = GetLineColor();
        SetLineColor( rColor );
        if ( mbInitLineColor )
            ImplInitLineColor();
#endif

        long nEndX = nStartX+nWidth;
        long nEndY = nStartY;
        if ( nOrientation )
        {
            ImplRotatePos( nBaseX, nBaseY, nStartX, nStartY, nOrientation );
            ImplRotatePos( nBaseX, nBaseY, nEndX, nEndY, nOrientation );
        }
#ifndef REMOTE_APPSERVER
        mpGraphics->DrawLine( nStartX, nStartY, nEndX, nEndY, this );
#else
        mpGraphics->DrawLine( Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
#endif

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
#endif
    }
    else
    {
        long    nCurX = nStartX;
        long    nCurY = nStartY;
        long    nDiffX = 2;
        long    nDiffY = nHeight-1;
        long    nCount = nWidth;
        long    nOffY = -1;
        long    nFreq;
        long    i;
        long    nPixWidth;
        long    nPixHeight;
        BOOL    bDrawPixAsRect;
#ifdef REMOTE_APPSERVER
        Color   aOldLineColor = GetLineColor();
        Color   aOldFillColor = GetFillColor();

#endif
        // Auf Druckern die Pixel per DrawRect() ausgeben
        if ( (GetOutDevType() == OUTDEV_PRINTER) || (nLineWidth > 1) )
        {
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( rColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( rColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif
            bDrawPixAsRect  = TRUE;
            nPixWidth       = nLineWidth;
            nPixHeight      = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        }
        else
        {
#ifndef REMOTE_APPSERVER
            mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
            mbInitLineColor = TRUE;
#else
            Color aOldLineColor = GetLineColor();
            SetLineColor( rColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
#endif
            nPixWidth       = 1;
            nPixHeight      = 1;
            bDrawPixAsRect  = FALSE;
        }

        if ( !nDiffY )
        {
            while ( nWidth )
            {
                ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                   mpGraphics, this,
                                   bDrawPixAsRect, nPixWidth, nPixHeight );
                nCurX++;
                nWidth--;
            }
        }
        else
        {
            nCurY += nDiffY;
            nFreq = nCount / (nDiffX+nDiffY);
            while ( nFreq-- )
            {
                for( i = nDiffY; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( i = nDiffX; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
                nOffY = -nOffY;
            }
            nFreq = nCount % (nDiffX+nDiffY);
            if ( nFreq )
            {
                for( i = nDiffY; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;

                }
                for( i = nDiffX; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
            }
        }

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextLine( long nBaseX,
                                     long nX, long nY, long nWidth,
                                     FontStrikeout eStrikeout,
                                     FontUnderline eUnderline,
                                     BOOL bUnderlineAbove )
{
    if ( !nWidth )
        return;

    ImplFontEntry*  pFontEntry = mpFontEntry;
    Color           aUnderlineColor = GetTextLineColor();
    Color           aStrikeoutColor = GetTextColor();
    long            nBaseY = nY;
    long            nLineHeight;
    long            nLinePos;
    long            nLinePos2;
    long            nLeft;
    BOOL            bNormalLines = TRUE;

    // TODO: fix rotated text
    if( ImplHasMirroredGraphics() && IsRTLEnabled() )
        // --- RTL --- mirror at basex
        nX = nBaseX - nWidth - (nX - nBaseX - 1);

    if ( !IsTextLineColor() )
        aUnderlineColor = GetTextColor();

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        if ( bUnderlineAbove )
        {
            if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
                ImplInitAboveTextLineSize();
            nLinePos = pFontEntry->maMetric.mnAboveWUnderlineOffset;
            nLineHeight = pFontEntry->maMetric.mnAboveWUnderlineSize;
        }
        else
        {
            if ( !pFontEntry->maMetric.mnWUnderlineSize )
                ImplInitTextLineSize();
            nLinePos = pFontEntry->maMetric.mnWUnderlineOffset;
            nLineHeight = pFontEntry->maMetric.mnWUnderlineSize;
        }
        if ( (eUnderline == UNDERLINE_SMALLWAVE) &&
             (nLineHeight > 3) )
            nLineHeight = 3;
        long nLineWidth = (mnDPIX/300);
        if ( !nLineWidth )
            nLineWidth = 1;
        if ( eUnderline == UNDERLINE_BOLDWAVE )
            nLineWidth *= 2;
        nLinePos += nY - (nLineHeight / 2);
        long nLineWidthHeight = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        if ( eUnderline == UNDERLINE_DOUBLEWAVE )
        {
            long nOrgLineHeight = nLineHeight;
            nLineHeight /= 3;
            if ( nLineHeight < 2 )
            {
                if ( nOrgLineHeight > 1 )
                    nLineHeight = 2;
                else
                    nLineHeight = 1;
            }
            long nLineDY = nOrgLineHeight-(nLineHeight*2);
            if ( nLineDY < nLineWidthHeight )
                nLineDY = nLineWidthHeight;
            long nLineDY2 = nLineDY/2;
            if ( !nLineDY2 )
                nLineDY2 = 1;

            nLinePos -= nLineWidthHeight-nLineDY2;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
            nLinePos += nLineWidthHeight+nLineDY;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
        }
        else
        {
            nLinePos -= nLineWidthHeight/2;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
        }


        if ( (eStrikeout == STRIKEOUT_NONE) ||
             (eStrikeout == STRIKEOUT_DONTKNOW) )
            bNormalLines = FALSE;
    }

    if ( bNormalLines &&
         ((eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X)) )
    {
        BOOL bOldMap = IsMapModeEnabled();
        EnableMapMode( FALSE );
        Color aOldColor = GetTextColor();
        SetTextColor( aStrikeoutColor );
        ImplInitTextColor();
        xub_Unicode pChars[5];
        if ( eStrikeout == STRIKEOUT_SLASH )
            pChars[0] = '/';
        else // ( eStrikeout == STRIKEOUT_X )
            pChars[0] = 'X';
        pChars[3]=pChars[2]=pChars[1]=pChars[0];
        // calculate approximation of strikeout atom size
        long nStrikeoutWidth = nWidth;
        String aStrikeoutTest( pChars, 4 );
        SalLayout* pSalLayout = ImplLayout( aStrikeoutTest, 0, 4 );
        if( pSalLayout )
        {
            nStrikeoutWidth = (pSalLayout->GetTextWidth() + 2) / 4;
            pSalLayout->Release();
        }
        // calculate acceptable strikeout length
        // allow the strikeout to be one pixel larger than the text it strikes out
        long nMaxWidth = nStrikeoutWidth/2;
        if ( nMaxWidth < 2 )
            nMaxWidth = 2;
        nMaxWidth += nWidth + 1;
        // build strikeout string
        long nFullStrikeoutWidth = 0;
        String aStrikeoutText( pChars, 0 );
        while( (nFullStrikeoutWidth+=nStrikeoutWidth) < nMaxWidth+1 )
            aStrikeoutText += pChars[0];
        // if the text width is smaller than the strikeout text, then do not
        // strike out at all. This case requires user interaction, e.g. adding
        // a space to the text
        if( (aStrikeoutText.Len() > 0)
        && !(mpPDFWriter && mpPDFWriter->isBuiltinFont(mpFontEntry->maFontSelData.mpFontData) ) )
        {
            if( mpFontEntry->mnOrientation )
                ImplRotatePos( nBaseX, nBaseY, nX, nY, mpFontEntry->mnOrientation );
            SalLayout* pSalLayout = ImplLayout( aStrikeoutText, 0, STRING_LEN, Point(nX,nY) );
            if( pSalLayout )
            {
                pSalLayout->DrawBase() = Point( nX+mnTextOffX, nY+mnTextOffY );
                pSalLayout->DrawText( *mpGraphics );
                pSalLayout->Release();
            }
        }

        SetTextColor( aOldColor );
        ImplInitTextColor();
        EnableMapMode( bOldMap );

        switch( eUnderline )
        {
            case UNDERLINE_NONE:
            case UNDERLINE_DONTKNOW:
            case UNDERLINE_SMALLWAVE:
            case UNDERLINE_WAVE:
            case UNDERLINE_DOUBLEWAVE:
            case UNDERLINE_BOLDWAVE:
                bNormalLines = FALSE;
        }
    }

    if ( bNormalLines )
    {
#ifdef REMOTE_APPSERVER
        Color aOldLineColor = GetLineColor();
        Color aOldFillColor = GetFillColor();
#endif

        if ( eUnderline > UNDERLINE_LAST )
            eUnderline = UNDERLINE_SINGLE;

        if ( (eUnderline == UNDERLINE_SINGLE) ||
             (eUnderline == UNDERLINE_DOTTED) ||
             (eUnderline == UNDERLINE_DASH) ||
             (eUnderline == UNDERLINE_LONGDASH) ||
             (eUnderline == UNDERLINE_DASHDOT) ||
             (eUnderline == UNDERLINE_DASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveUnderlineOffset;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnUnderlineOffset;
            }
        }
        else if ( (eUnderline == UNDERLINE_BOLD) ||
                  (eUnderline == UNDERLINE_BOLDDOTTED) ||
                  (eUnderline == UNDERLINE_BOLDDASH) ||

                  (eUnderline == UNDERLINE_BOLDLONGDASH) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOT) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveBUnderlineOffset;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnBUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnBUnderlineOffset;
            }
        }
        else if ( eUnderline == UNDERLINE_DOUBLE )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset2;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnDUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnDUnderlineOffset2;
            }
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( aUnderlineColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( aUnderlineColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif

            nLeft = nX;

            if ( (eUnderline == UNDERLINE_SINGLE) ||
                 (eUnderline == UNDERLINE_BOLD) )
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            else if ( eUnderline == UNDERLINE_DOUBLE )
            {
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            }
            else if ( (eUnderline == UNDERLINE_DOTTED) ||
                      (eUnderline == UNDERLINE_BOLDDOTTED) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nTempWidth = nDotWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDotWidth*2;

                }
            }
            else if ( (eUnderline == UNDERLINE_DASH) ||
                      (eUnderline == UNDERLINE_LONGDASH) ||
                      (eUnderline == UNDERLINE_BOLDDASH) ||
                      (eUnderline == UNDERLINE_BOLDLONGDASH) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nMinDashWidth;

                long nMinSpaceWidth;
                long nSpaceWidth;
                long nDashWidth;
                if ( (eUnderline == UNDERLINE_LONGDASH) ||
                     (eUnderline == UNDERLINE_BOLDLONGDASH) )
                {
                    nMinDashWidth = nDotWidth*6;
                    nMinSpaceWidth = nDotWidth*2;
                    nDashWidth = 200;
                    nSpaceWidth = 100;
                }
                else
                {
                    nMinDashWidth = nDotWidth*4;
                    nMinSpaceWidth = (nDotWidth*150)/100;
                    nDashWidth = 100;
                    nSpaceWidth = 50;
                }
                nDashWidth = ((nDashWidth*mnDPIX)+1270)/2540;
                nSpaceWidth = ((nSpaceWidth*mnDPIX)+1270)/2540;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                if ( nSpaceWidth < nMinSpaceWidth )
                    nSpaceWidth = nMinSpaceWidth;
                long nTempWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDashWidth+nSpaceWidth;
                }
            }
            else if ( (eUnderline == UNDERLINE_DASHDOT) ||
                      (eUnderline == UNDERLINE_BOLDDASHDOT) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
            else if ( (eUnderline == UNDERLINE_DASHDOTDOT) ||

                      (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;

                nDotWidth /= mnDPIY;
                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
        }

        if ( eStrikeout > STRIKEOUT_LAST )
            eStrikeout = STRIKEOUT_SINGLE;

        if ( eStrikeout == STRIKEOUT_SINGLE )
        {
            if ( !pFontEntry->maMetric.mnStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnStrikeoutSize;

            nLinePos    = nY + pFontEntry->maMetric.mnStrikeoutOffset;
        }
        else if ( eStrikeout == STRIKEOUT_BOLD )
        {
            if ( !pFontEntry->maMetric.mnBStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnBStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnBStrikeoutOffset;
        }
        else if ( eStrikeout == STRIKEOUT_DOUBLE )
        {
            if ( !pFontEntry->maMetric.mnDStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnDStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnDStrikeoutOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnDStrikeoutOffset2;
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( aStrikeoutColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( aStrikeoutColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif

            nLeft = nX;

            if ( (eStrikeout == STRIKEOUT_SINGLE) ||
                 (eStrikeout == STRIKEOUT_BOLD) )
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            else if ( eStrikeout == STRIKEOUT_DOUBLE )
            {
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            }
        }

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextLines( SalLayout& rSalLayout,
    FontStrikeout eStrikeout, FontUnderline eUnderline, BOOL bWordLine, BOOL bUnderlineAbove )
{
    if( bWordLine )
    {
        Point aPos, aStartPt;
        long nWidth = 0, nAdvance=0;
        for( int nStart = 0;;)
        {
            long nGlyphIndex;
            if( !rSalLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                break;

            if( !rSalLayout.IsSpacingGlyph( nGlyphIndex ) )
            {
                if( !nWidth )
                {
                    aStartPt = aPos;//rSalLayout.DrawBase() - (aPos - rSalLayout.DrawOffset());
                }

                nWidth += nAdvance;
            }
            else if( nWidth > 0 )
            {
                ImplDrawTextLine( rSalLayout.DrawBase().X(), aStartPt.X(), aStartPt.Y(), nWidth,
                    eStrikeout, eUnderline, bUnderlineAbove );
                nWidth = 0;
            }
        }

        if( nWidth > 0 )
        {
            ImplDrawTextLine( rSalLayout.DrawBase().X(), aStartPt.X(), aStartPt.Y(), nWidth,
                eStrikeout, eUnderline, bUnderlineAbove );
        }
    }
    else
    {
        Point aStartPt = rSalLayout.GetDrawPosition();
        int nWidth = rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel();
        ImplDrawTextLine( rSalLayout.DrawBase().X(), aStartPt.X(), aStartPt.Y(), nWidth,
            eStrikeout, eUnderline, bUnderlineAbove );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawMnemonicLine( long nX, long nY, long nWidth )
{
    long nBaseX = nX;
    if( ImplHasMirroredGraphics() && IsRTLEnabled() )
    {
        // --- RTL ---
        // add some strange offset
        nX += 2;
        // revert the hack that will be done later in ImplDrawTextLine
        nX = nBaseX - nWidth - (nX - nBaseX - 1);
    }

    ImplDrawTextLine( nBaseX, nX, nY, nWidth, STRIKEOUT_NONE, UNDERLINE_SINGLE, FALSE );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplGetEmphasisMark( PolyPolygon& rPolyPoly, BOOL& rPolyLine,
                                        Rectangle& rRect1, Rectangle& rRect2,
                                        long& rYOff, long& rWidth,
                                        FontEmphasisMark eEmphasis,
                                        long nHeight, short nOrient )
{
    static const BYTE aAccentPolyFlags[24] =
    {
        0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 0, 2, 0, 2, 2
    };

    static const long aAccentPos[48] =
    {
         78,      0,
        348,     79,
        599,    235,
        843,    469,
        938,    574,
        990,    669,
        990,    773,
        990,    843,
        964,    895,
        921,    947,
        886,    982,
        860,    999,
        825,    999,
        764,    999,
        721,    964,
        686,    895,
        625,    791,
        556,    660,
        469,    504,
        400,    400,
        261,    252,
         61,     61,
          0,     27,
          9,      0
    };

    rWidth      = 0;
    rYOff       = 0;
    rPolyLine   = FALSE;

    if ( !nHeight )
        return;

    FontEmphasisMark    nEmphasisStyle = eEmphasis & EMPHASISMARK_STYLE;
    long                nDotSize = 0;
    switch ( nEmphasisStyle )
    {
        case EMPHASISMARK_DOT:
            // Dot has 55% of the height
            nDotSize = (nHeight*550)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rYOff = ((nHeight*250)/1000)/2; // Center to the anthoer EmphasisMarks
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_CIRCLE:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
                // BorderWidth is 15%
                long nBorder = (nDotSize*150)/1000;
                if ( nBorder <= 1 )
                    rPolyLine = TRUE;
                else
                {
                    Polygon aPoly2( Point( nRad, nRad ),
                                    nRad-nBorder, nRad-nBorder );
                    rPolyPoly.Insert( aPoly2 );
                }
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_DISC:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_ACCENT:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
            {
                if ( nDotSize == 1 )
                {
                    rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
                    rWidth = nDotSize;
                }
                else
                {
                    rRect1 = Rectangle( Point(), Size( 1, 1 ) );
                    rRect2 = Rectangle( Point( 1, 1 ), Size( 1, 1 ) );
                }
            }
            else
            {
                Polygon aPoly( sizeof( aAccentPos ) / sizeof( long ) / 2,
                               (const Point*)aAccentPos,
                               aAccentPolyFlags );
                double dScale = ((double)nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                Polygon aTemp;
                aPoly.GetSimple( aTemp );
                Rectangle aBoundRect = aTemp.GetBoundRect();
                rWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();
                rPolyPoly.Insert( aTemp );
            }
            break;
    }

    // calculate position
    long nOffY = 1+(mnDPIY/300); // one visible pixel space
    long nSpaceY = nHeight-nDotSize;
    if ( nSpaceY >= nOffY*2 )
        rYOff += nOffY;
    if ( !(eEmphasis & EMPHASISMARK_POS_BELOW) )
        rYOff += nDotSize;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawEmphasisMark( long nBaseX, long nX, long nY,
                                         const PolyPolygon& rPolyPoly, BOOL bPolyLine,
                                         const Rectangle& rRect1, const Rectangle& rRect2 )
{
    // TODO: pass nWidth as width of this mark
    long nWidth = 0;

    if( ImplHasMirroredGraphics() && IsRTLEnabled() )
        // --- RTL --- mirror at basex
        nX = nBaseX - nWidth - (nX - nBaseX - 1);

    nX -= mnOutOffX;
    nY -= mnOutOffY;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            DrawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            DrawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        DrawRect( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        DrawRect( aRect );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawEmphasisMarks( SalLayout& rSalLayout )
{
    Color               aOldColor       = GetTextColor();
    Color               aOldLineColor   = GetLineColor();
    Color               aOldFillColor   = GetFillColor();
    BOOL                bOldMap         = mbMap;
    GDIMetaFile*        pOldMetaFile    = mpMetaFile;
    mpMetaFile = NULL;
    mbMap = FALSE;

    FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
    PolyPolygon         aPolyPoly;
    Rectangle           aRect1;
    Rectangle           aRect2;
    long                nEmphasisYOff;
    long                nEmphasisWidth;
    long                nEmphasisHeight;
    BOOL                bPolyLine;

    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        nEmphasisHeight = mnEmphasisDescent;
    else
        nEmphasisHeight = mnEmphasisAscent;

    ImplGetEmphasisMark( aPolyPoly, bPolyLine,
                         aRect1, aRect2,
                         nEmphasisYOff, nEmphasisWidth,
                         nEmphasisMark,
                         nEmphasisHeight, mpFontEntry->mnOrientation );

    if ( bPolyLine )
    {
        SetLineColor( GetTextColor() );
        SetFillColor();
    }
    else
    {
        SetLineColor();
        SetFillColor( GetTextColor() );
    }

    Point aOffset = Point(0,0);

    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        aOffset.Y() += mpFontEntry->maMetric.mnDescent + nEmphasisYOff;
    else
        aOffset.Y() -= mpFontEntry->maMetric.mnAscent + nEmphasisYOff;

    long nEmphasisWidth2  = nEmphasisWidth / 2;
    long nEmphasisHeight2 = nEmphasisHeight / 2;
    aOffset += Point( nEmphasisWidth2, nEmphasisHeight2 );

    Point aOutPoint;
    Rectangle aRectangle;
    for( int nStart = 0;;)
    {
        long nGlyphIndex;
        if( !rSalLayout.GetNextGlyphs( 1, &nGlyphIndex, aOutPoint, nStart ) )
            break;

        if( !mpGraphics->GetGlyphBoundRect( nGlyphIndex, aRectangle ) )
            continue;

        if( !rSalLayout.IsSpacingGlyph( nGlyphIndex ) )
        {
            Point aAdjPoint = aOffset;
            aAdjPoint.X() += aRectangle.Left() + (aRectangle.GetWidth() - nEmphasisWidth) / 2;
            if ( mpFontEntry->mnOrientation )
                ImplRotatePos( 0, 0, aAdjPoint.X(), aAdjPoint.Y(), mpFontEntry->mnOrientation );
            aOutPoint += aAdjPoint;
            aOutPoint -= Point( nEmphasisWidth2, nEmphasisHeight2 );
            ImplDrawEmphasisMark( rSalLayout.DrawBase().X(),
                                  aOutPoint.X(), aOutPoint.Y(),
                                  aPolyPoly, bPolyLine, aRect1, aRect2 );
        }
    }

    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

// -----------------------------------------------------------------------

bool OutputDevice::ImplDrawRotateText( SalLayout& rSalLayout )
{
    int nX = rSalLayout.DrawBase().X();
    int nY = rSalLayout.DrawBase().Y();

    Rectangle aBoundRect;
    rSalLayout.DrawBase() = Point( 0, 0 );
    rSalLayout.DrawOffset() = Point( 0, 0 );
    int nWidthFactor = rSalLayout.GetUnitsPerPixel();
    if( !rSalLayout.GetBoundRect( *mpGraphics, aBoundRect ) )
    {
        // guess vertical text extents if GetBoundRect failed
        int nRight = rSalLayout.GetTextWidth();
        int nTop = mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;
        long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
        aBoundRect = Rectangle( 0, -nTop, nRight, nHeight - nTop );
    }

    // cache virtual device for rotation
    if ( !mpOutDevData )
        ImplInitOutDevData();
    if ( !mpOutDevData->mpRotateDev )
        mpOutDevData->mpRotateDev = new VirtualDevice( *this, 1 );
    VirtualDevice* pVDev = mpOutDevData->mpRotateDev;

    // size it accordingly
    if( !pVDev->SetOutputSizePixel( aBoundRect.GetSize() ) )
        return false;

    Font aFont( GetFont() );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    pVDev->SetFont( aFont );
    pVDev->SetTextColor( Color( COL_BLACK ) );
    pVDev->SetTextFillColor();
    pVDev->ImplNewFont();
    pVDev->ImplInitFont();
    pVDev->ImplInitTextColor();

    // draw text into upper left corner
    rSalLayout.DrawBase() -= aBoundRect.TopLeft();
    rSalLayout.DrawText( *pVDev->mpGraphics );

    Bitmap aBmp = pVDev->GetBitmap( Point(), aBoundRect.GetSize() );
    if ( !aBmp || !aBmp.Rotate( mpFontEntry->mnOwnOrientation, COL_WHITE ) )
        return false;

    // calculate rotation offset
    Polygon aPoly( aBoundRect );
    aPoly.Rotate( Point(), mpFontEntry->mnOwnOrientation );
    Point aPoint = aPoly.GetBoundRect().TopLeft();
    aPoint += Point( nX, nY );

    // mask output with text colored bitmap
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    long nOldOffX = mnOutOffX;
    long nOldOffY = mnOutOffY;
    BOOL bOldMap = mbMap;

    mnOutOffX   = 0L;
    mnOutOffY   = 0L;
    mpMetaFile  = NULL;
    mbMap       = FALSE;

    DrawMask( aPoint, aBmp, GetTextColor() );

    mbMap       = bOldMap;
    mnOutOffX   = nOldOffX;
    mnOutOffY   = nOldOffY;
    mpMetaFile  = pOldMetaFile;

    return true;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextDirect( SalLayout& rSalLayout, BOOL bTextLines )
{
    if( mpFontEntry->mnOwnOrientation )
        if( ImplDrawRotateText( rSalLayout ) )
            return;

    long nOldX = rSalLayout.DrawBase().X();
    if( ! (mpPDFWriter && mpPDFWriter->isBuiltinFont(mpFontEntry->maFontSelData.mpFontData) ) )
    {
        if( ImplHasMirroredGraphics() )
        {
            long w = meOutDevType == OUTDEV_VIRDEV ? mnOutWidth : mpGraphics->GetGraphicsWidth();
            long x = rSalLayout.DrawBase().X();
               rSalLayout.DrawBase().X() = w - 1 - x;
            if( !IsRTLEnabled() )
            {
                // mirror this window back
                long devX = w-mnOutWidth-mnOutOffX;   // re-mirrored mnOutOffX
                rSalLayout.DrawBase().X() = devX + ( mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) ) ;
            }
        }

        rSalLayout.DrawText( *mpGraphics );
    }

    rSalLayout.DrawBase().X() = nOldX;

    if( bTextLines )
        ImplDrawTextLines( rSalLayout,
            maFont.GetStrikeout(), maFont.GetUnderline(),
            maFont.IsWordLineMode(), ImplIsUnderlineAbove( maFont ) );

    // emphasis marks
    if( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
        ImplDrawEmphasisMarks( rSalLayout );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawSpecialText( SalLayout& rSalLayout )
{
    Color       aOldColor           = GetTextColor();
    Color       aOldTextLineColor   = GetTextLineColor();
    FontRelief  eRelief             = maFont.GetRelief();

    Point aOrigPos = rSalLayout.DrawBase();
    if ( eRelief != RELIEF_NONE )
    {
        Color   aReliefColor( COL_LIGHTGRAY );
        Color   aTextColor( aOldColor );

        Color   aTextLineColor( aOldTextLineColor );

        // we don't have a automatic color, so black is always drawn on white
        if ( aTextColor.GetColor() == COL_BLACK )
            aTextColor = Color( COL_WHITE );
        if ( aTextLineColor.GetColor() == COL_BLACK )
            aTextLineColor = Color( COL_WHITE );

        // relief-color is black for white text, in all other cases
        // we set this to LightGray
        if ( aTextColor.GetColor() == COL_WHITE )
            aReliefColor = Color( COL_BLACK );
        SetTextLineColor( aReliefColor );
        SetTextColor( aReliefColor );
        ImplInitTextColor();

        // calculate offset - for high resolution printers the offset
        // should be greater so that the effect is visible
        long nOff = 1;
        nOff += mnDPIX/300;

        if ( eRelief == RELIEF_ENGRAVED )
            nOff = -nOff;
        rSalLayout.DrawOffset() += Point( nOff, nOff);
        ImplDrawTextDirect( rSalLayout, mbTextLines );
        rSalLayout.DrawOffset() -= Point( nOff, nOff);

        SetTextLineColor( aTextLineColor );
        SetTextColor( aTextColor );
        ImplInitTextColor();
        ImplDrawTextDirect( rSalLayout, mbTextLines );

        SetTextLineColor( aOldTextLineColor );

        if ( aTextColor != aOldColor )
        {
            SetTextColor( aOldColor );
            ImplInitTextColor();
        }
    }
    else
    {
        if ( maFont.IsShadow() )
        {
            long nOff = 1 + ((mpFontEntry->mnLineHeight-24)/24);
            if ( maFont.IsOutline() )
                nOff++;
            SetTextLineColor();
            if ( (GetTextColor().GetColor() == COL_BLACK)
            ||   (GetTextColor().GetLuminance() < 8) )
                SetTextColor( Color( COL_LIGHTGRAY ) );
            else
                SetTextColor( Color( COL_BLACK ) );
            ImplInitTextColor();
            rSalLayout.DrawBase() += Point( nOff, nOff );
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() -= Point( nOff, nOff );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            ImplInitTextColor();

            if ( !maFont.IsOutline() )
                ImplDrawTextDirect( rSalLayout, mbTextLines );
        }

        if ( maFont.IsOutline() )
        {
            rSalLayout.DrawBase() = aOrigPos + Point(-1,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(-1,+0);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(-1,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+0,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+0,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,+0);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos;

            SetTextColor( Color( COL_WHITE ) );
            SetTextLineColor( Color( COL_WHITE ) );
            ImplInitTextColor();
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            ImplInitTextColor();
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawText( SalLayout& rSalLayout )
{
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;
    if( mbInitTextColor )
        ImplInitTextColor();

    rSalLayout.DrawBase() += Point( mnTextOffX, mnTextOffY );

    if( IsTextFillColor() )
        ImplDrawTextBackground( rSalLayout );

    if( mbTextSpecial )
        ImplDrawSpecialText( rSalLayout );
    else
        ImplDrawTextDirect( rSalLayout, mbTextLines );
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo,
                                     long nWidth, const XubString& rStr,
                                     USHORT nStyle ) const
{
    DBG_ASSERTWARNING( nWidth >= 0, "ImplGetTextLines: nWidth <= 0!" );

    if ( nWidth <= 0 )
        nWidth = 1;

    long nMaxLineWidth  = 0;
    rLineInfo.Clear();
    if ( rStr.Len() && (nWidth > 0) )
    {
        ::rtl::OUString aText( rStr );
        uno::Reference < i18n::XBreakIterator > xBI;
        uno::Reference< linguistic2::XHyphenator > xHyph;
        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, uno::Sequence <beans::PropertyValue>(), 1 );
        i18n::LineBreakUserOptions aUserOptions;

        xub_StrLen nPos = 0;
        xub_StrLen nLen = rStr.Len();
        while ( nPos < nLen )
        {
            xub_StrLen nBreakPos = nPos;

            while ( ( nBreakPos < nLen ) && ( rStr.GetChar( nBreakPos ) != _CR ) && ( rStr.GetChar( nBreakPos ) != _LF ) )
                nBreakPos++;

            long nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
            if ( ( nLineWidth > nWidth ) && ( nStyle & TEXT_DRAW_WORDBREAK ) )
            {
                if ( !xBI.is() )
                    xBI = vcl::unohelper::CreateBreakIterator();

                if ( xBI.is() )
                {
                    static const com::sun::star::lang::Locale aDefLocale;
                    xub_StrLen nSoftBreak = GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
                    DBG_ASSERT( nSoftBreak < nBreakPos, "Break?!" );
                    i18n::LineBreakResults aLBR = xBI->getLineBreak( aText, nSoftBreak, aDefLocale, nPos, aHyphOptions, aUserOptions );
                    nBreakPos = (xub_StrLen)aLBR.breakIndex;
                    if ( nBreakPos <= nPos )
                        nBreakPos = nSoftBreak;
                    nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
                }
            }

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            rLineInfo.AddLine( new ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

            if ( nBreakPos == nPos )
                nBreakPos++;
            nPos = nBreakPos;

            if ( ( rStr.GetChar( nPos ) == _CR ) || ( rStr.GetChar( nPos ) == _LF ) )
            {
                nPos++;
                // CR/LF?
                if ( ( nPos < nLen ) && ( rStr.GetChar( nPos ) == _LF ) && ( rStr.GetChar( nPos-1 ) == _CR ) )
                    nPos++;
            }
        }
    }
#ifdef DBG_UTIL
    for ( USHORT nL = 0; nL < rLineInfo.Count(); nL++ )
    {
        ImplTextLineInfo* pLine = rLineInfo.GetLine( nL );
        String aLine( rStr, pLine->GetIndex(), pLine->GetLen() );
        DBG_ASSERT( aLine.Search( _CR ) == STRING_NOTFOUND, "ImplGetTextLines - Found CR!" );
        DBG_ASSERT( aLine.Search( _LF ) == STRING_NOTFOUND, "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

// =======================================================================

void OutputDevice::SetAntialiasing( USHORT nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetFont( const Font& rNewFont )
{
    DBG_TRACE( "OutputDevice::SetFont()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rNewFont, Font, NULL );

    Font aFont( rNewFont );
    if ( mnDrawMode & (DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT | DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT | DRAWMODE_SETTINGSTEXT |
                       DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
    {
        Color aTextColor( aFont.GetColor() );

        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aTextColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aTextColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aTextColor.GetLuminance();
            aTextColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aTextColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aTextColor = Color( (aTextColor.GetRed() >> 1 ) | 0x80,
                                (aTextColor.GetGreen() >> 1 ) | 0x80,
                                (aTextColor.GetBlue() >> 1 ) | 0x80 );
        }

        aFont.SetColor( aTextColor );

        BOOL bTransFill = aFont.IsTransparent();
        if ( !bTransFill )
        {
            Color aTextFillColor( aFont.GetFillColor() );

            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aTextFillColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aTextFillColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const UINT8 cLum = aTextFillColor.GetLuminance();
                aTextFillColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
                aTextFillColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aTextFillColor = Color( COL_TRANSPARENT );
                bTransFill = TRUE;
            }

            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aTextFillColor = Color( (aTextFillColor.GetRed() >> 1) | 0x80,
                                        (aTextFillColor.GetGreen() >> 1) | 0x80,
                                        (aTextFillColor.GetBlue() >> 1) | 0x80 );
            }

            aFont.SetFillColor( aTextFillColor );
        }
    }

    if ( mpMetaFile )
    {
        const Color& rTextFillColor = aFont.GetFillColor();

        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlign() ) );
        mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
        mpMetaFile->AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );
    }

    if ( !maFont.IsSameInstance( aFont ) )
    {
        // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
        // because SetTextColor() is used for this.
        if ( ( aFont.GetColor() != COL_TRANSPARENT ) && ( maFont.GetColor() != aFont.GetColor() ) )
        {
            maTextColor = aFont.GetColor();
            mbInitTextColor = TRUE;
        }
        maFont      = aFont;
        mbNewFont   = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetLayoutMode( ULONG nTextLayoutMode )
{
    DBG_TRACE( "OutputDevice::SetTextLayoutMode()" );

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLayoutModeAction( nTextLayoutMode ) );

    mnTextLayoutMode = nTextLayoutMode;
}

// -----------------------------------------------------------------------

void OutputDevice::SetDigitLanguage( LanguageType eTextLanguage )
{
    DBG_TRACE( "OutputDevice::SetTextLanguage()" );

#if 0   // TODO: evaluate if we need to record it into a meta file
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLanguageAction( eTextLanguage ) );
#endif

    if( eTextLanguage == LANGUAGE_SYSTEM )
        eTextLanguage = GetSystemLanguage();

    meTextLanguage = eTextLanguage;
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                        DRAWMODE_SETTINGSTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextColorAction( aColor ) );

    if ( maTextColor != aColor )
    {
        maTextColor = aColor;
        mbInitTextColor = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextFillColor()
{
    DBG_TRACE( "OutputDevice::SetTextFillColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), FALSE ) );

    if ( maFont.GetColor() != Color( COL_TRANSPARENT ) )
        maFont.SetFillColor( Color( COL_TRANSPARENT ) );
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( TRUE );
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextFillColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextFillColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );
    BOOL bTransFill = ImplIsColorTransparent( aColor );

    if ( !bTransFill )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                            DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                            DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
        {
            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const UINT8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aColor = Color( COL_TRANSPARENT );
                bTransFill = TRUE;
            }

            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80 );
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, TRUE ) );

    if ( maFont.GetFillColor() != aColor )
        maFont.SetFillColor( aColor );
    if ( maFont.IsTransparent() != bTransFill )
        maFont.SetTransparent( bTransFill );
}

// -----------------------------------------------------------------------

Color OutputDevice::GetTextFillColor() const
{
    if ( maFont.IsTransparent() )
        return Color( COL_TRANSPARENT );
    else
        return maFont.GetFillColor();
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextLineColor()
{
    DBG_TRACE( "OutputDevice::SetTextLineColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), FALSE ) );

    maTextLineColor = Color( COL_TRANSPARENT );
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextLineColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextLineColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                        DRAWMODE_SETTINGSTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, TRUE ) );

    maTextLineColor = aColor;
}

// -----------------------------------------------------------------------


void OutputDevice::SetTextAlign( TextAlign eAlign )
{
    DBG_TRACE( "OutputDevice::SetTextAlign()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlign() != eAlign )
    {
        maFont.SetAlign( eAlign );
        mbNewFont = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawTextLine( const Point& rPos, long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontUnderline eUnderline,
                                 BOOL bUnderlineAbove )
{
    DBG_TRACE( "OutputDevice::DrawTextLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline ) );

    if ( ((eUnderline == UNDERLINE_NONE) || (eUnderline == UNDERLINE_DONTKNOW)) &&
         ((eStrikeout == STRIKEOUT_NONE) || (eStrikeout == STRIKEOUT_DONTKNOW)) )
        return;

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;
#else
    if( !ImplGetServerGraphics() )
        return;
#endif

    // initialize font if needed to get text offsets
    // TODO: only needed for mnTextOff!=(0,0)
    if( mbNewFont )
        if( !ImplNewFont() )
            return;
    if( mbInitFont )
        ImplInitFont();

    Point aPos = ImplLogicToDevicePixel( rPos );
    nWidth = ImplLogicWidthToDevicePixel( nWidth );
    aPos += Point( mnTextOffX, mnTextOffY );
    ImplDrawTextLine( aPos.X(), aPos.X(), aPos.Y(), nWidth, eStrikeout, eUnderline, bUnderlineAbove );
}

// ------------------------------------------------------------------------

BOOL OutputDevice::IsTextUnderlineAbove( const Font& rFont )
{
    return ImplIsUnderlineAbove( rFont );
}

// ------------------------------------------------------------------------

void OutputDevice::DrawWaveLine( const Point& rStartPos, const Point& rEndPos,
                                 USHORT nStyle )
{
    DBG_TRACE( "OutputDevice::DrawWaveLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

#ifndef REMOTE_APPSERVER
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    Point   aStartPt = ImplLogicToDevicePixel( rStartPos );
    Point   aEndPt = ImplLogicToDevicePixel( rEndPos );
    long    nStartX = aStartPt.X();
    long    nStartY = aStartPt.Y();
    long    nEndX = aEndPt.X();
    long    nEndY = aEndPt.Y();
    short   nOrientation = 0;

    // when rotated
    if ( (nStartY != nEndY) || (nStartX > nEndX) )
    {
        long nDX = nEndX - nStartX;
        double nO = atan2( -nEndY + nStartY, ((nDX == 0L) ? 0.000000001 : nDX) );
        nO /= F_PI1800;
        nOrientation = (short)nO;
        ImplRotatePos( nStartX, nStartY, nEndX, nEndY, -nOrientation );
    }

    long nWaveHeight;
    if ( nStyle == WAVE_NORMAL )
    {
        nWaveHeight = 3;
        nStartY++;
        nEndY++;
    }
    else if( nStyle == WAVE_SMALL )
    {
        nWaveHeight = 2;
        nStartY++;
        nEndY++;
    }
    else // WAVE_FLAT
        nWaveHeight = 1;

     // #109280# make sure the waveline does not exceed the descent to avoid paint problems
     ImplFontEntry*  pFontEntry = mpFontEntry;
     if ( !pFontEntry->maMetric.mnWUnderlineSize )
         ImplInitTextLineSize();
     if( nWaveHeight > pFontEntry->maMetric.mnWUnderlineSize )
         nWaveHeight = pFontEntry->maMetric.mnWUnderlineSize;

     ImplDrawWaveLine( nStartX, nStartY, nStartX, nStartY,
                      nEndX-nStartX, nWaveHeight, 1,
                      nOrientation, GetLineColor() );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();

        Point aPos1 = ImplLogicToDevicePixel( rStartPos );
        Point aPos2 = ImplLogicToDevicePixel( rEndPos );
        pGraphics->DrawWaveLine( aPos1, aPos2, nStyle );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawText( const Point& rStartPt, const String& rStr,
                             xub_StrLen nIndex, xub_StrLen nLen,
                             MetricVector* pVector, String* pDisplayText
                             )
{
    if( mpOutDevData && mpOutDevData->mpRecordLayout )
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

    DBG_TRACE( "OutputDevice::DrawText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    if( pVector )
    {
        Region aClip( GetClipRegion() );
        if( meOutDevType == OUTDEV_WINDOW )
            aClip.Intersect( Rectangle( Point(), GetOutputSize() ) );
        if( mpOutDevData && mpOutDevData->mpRecordLayout )
        {
            mpOutDevData->mpRecordLayout->m_aLineIndices.push_back( mpOutDevData->mpRecordLayout->m_aDisplayText.Len() );
            aClip.Intersect( mpOutDevData->maRecordRect );
        }
        if( ! aClip.IsNull() )
        {
            MetricVector aTmp;
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, aTmp );

            bool bInserted = false;
            for( MetricVector::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it, nIndex++ )
            {
                bool bAppend = false;

                if( aClip.IsOver( *it ) )
                    bAppend = true;
                else if( rStr.GetChar( nIndex ) == ' ' && bInserted )
                {
                    MetricVector::const_iterator next = it;
                    ++next;
                    if( next != aTmp.end() && aClip.IsOver( *next ) )
                        bAppend = true;
                }

                if( bAppend )
                {
                    pVector->push_back( *it );
                    if( pDisplayText )
                        pDisplayText->Append( rStr.GetChar( nIndex ) );
                    bInserted = true;
                }
            }
        }
        else
        {
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, *pVector );
            if( pDisplayText )
                pDisplayText->Append( rStr.Copy( nIndex, nLen ) );
        }
    }

    if ( !IsDeviceOutputNecessary() || pVector )
        return;
#ifdef UNX
    String aStr( rStr );
    if( meOutDevType == OUTDEV_PRINTER )
    {
        if( !mpGraphics )
            if( !ImplGetGraphics() )
                return;
        // FIXME: make fax on unix work again
#if 0
        xub_StrLen nCutStart, nCutStop;
        aStr = mpGraphics->maGraphicsData.FaxPhoneComment( rStr, nIndex, nLen, nCutStart, nCutStop );
#endif
    }
    SalLayout* pSalLayout = ImplLayout( aStr, nIndex, nLen, rStartPt );
#else
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt );
#endif
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextWidth( const String& rStr,
                                 xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextWidth()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    long nWidth = GetTextArray( rStr, NULL, nIndex, nLen );
    return nWidth;
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextHeight() const
{
    DBG_TRACE( "OutputDevice::GetTextHeight()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( mbNewFont )
        if( !(const_cast<OutputDevice*>(this)->ImplNewFont()) )
            return 0;

    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    if ( mbMap )
        nHeight = ImplDevicePixelToLogicHeight( nHeight );

    return nHeight;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawTextArray( const Point& rStartPt, const String& rStr,
                                  const long* pDXAry,
                                  xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

#ifdef UNX
    String aStr( rStr );
    if( meOutDevType == OUTDEV_PRINTER )
    {
        if( !mpGraphics )
            if( !ImplGetGraphics() )
                return;
        // FIXME: make fax work again on UNX
#if 0
        xub_StrLen nCutStart, nCutStop, nOrgLen = nLen;
        aStr = mpGraphics->maGraphicsData.FaxPhoneComment( rStr, nIndex, nLen, nCutStart, nCutStop );
        if( nCutStop != nCutStart )
        {
            long* pAry = (long*)alloca(sizeof(long)*nLen );
            if( nCutStart > nIndex )
                memcpy( pAry, pDXAry, sizeof(long)*(nCutStart-nIndex) );
            memcpy( pAry+nCutStart-nIndex, pDXAry + nOrgLen - (nCutStop-nIndex), nLen - (nCutStop-nIndex) );
            pDXAry = pAry;
        }
#endif
    }
    SalLayout* pSalLayout = ImplLayout( aStr, nIndex, nLen, rStartPt, 0, pDXAry );
#else
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, 0, pDXAry );
#endif
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextArray( const String& rStr, long* pDXAry,
                                 xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( nIndex >= rStr.Len() )
        return 0;
    if( (ULONG)nIndex+nLen >= rStr.Len() )
        nLen = rStr.Len() - nIndex;

    // do layout
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    if( !pSalLayout )
        return 0;

    long nWidth = pSalLayout->FillDXArray( pDXAry );
    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->Release();

    // convert virtual char widths to virtual absolute positions
    if( pDXAry )
        for( int i = 1; i < nLen; ++i )
            pDXAry[ i ] += pDXAry[ i-1 ];

    // convert from font units to logical units
    if( mbMap )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] = ImplDevicePixelToLogicWidth( pDXAry[i] );
        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }

    if( nWidthFactor > 1 )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] /= nWidthFactor;
        nWidth /= nWidthFactor;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

bool OutputDevice::GetCaretPositions( const XubString& rStr, long* pCaretXArray,
    xub_StrLen nIndex, xub_StrLen nLen,
    long* pDXAry, long nLayoutWidth,
    BOOL bCellBreaking ) const
{
    DBG_TRACE( "OutputDevice::GetCaretPositions()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( nIndex >= rStr.Len() )
        return false;
    if( (ULONG)nIndex+nLen >= rStr.Len() )
        nLen = rStr.Len() - nIndex;

    // layout complex text
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen,
        Point(0,0), nLayoutWidth, pDXAry );
    if( !pSalLayout )
        return false;

    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->GetCaretPositions( 2*nLen, pCaretXArray );
    long nWidth = pSalLayout->GetTextWidth();
    pSalLayout->Release();

    // fixup unknown caret positions
    int i;
    for( i = 0; i < 2 * nLen; ++i )
        if( pCaretXArray[ i ] >= 0 )
            break;
    long nXPos = pCaretXArray[ i ];
    for( i = 0; i < 2 * nLen; ++i )
    {
        if( pCaretXArray[ i ] >= 0 )
            nXPos = pCaretXArray[ i ];
        else
            pCaretXArray[ i ] = nXPos;
    }

    // handle window mirroring
    if( ((OutputDevice*)this)->ImplHasMirroredGraphics() && IsRTLEnabled() )
    {
        for( i = 0; i < 2 * nLen; ++i )
            pCaretXArray[i] = nWidth - pCaretXArray[i] - 1;
    }

    // convert from font units to logical units
    if( mbMap )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] = ImplDevicePixelToLogicWidth( pCaretXArray[i] );
    }

    if( nWidthFactor != 1 )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] /= nWidthFactor;
    }

    // if requested move caret position to cell limits
    if( bCellBreaking )
    {
        ; // TODO
    }

    return true;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawStretchText( const Point& rStartPt, ULONG nWidth,
                                    const String& rStr,
                                    xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawStretchText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

#ifdef UNX
    String aStr( rStr );
    if( meOutDevType == OUTDEV_PRINTER )
    {
        if( !mpGraphics )
            if( !ImplGetGraphics() )
                return;
        // FIXME: make fax work again
#if 0
        xub_StrLen nCutStart, nCutStop;
        aStr = mpGraphics->maGraphicsData.FaxPhoneComment( rStr, nIndex, nLen, nCutStart, nCutStop );
#endif
    }
    SalLayout* pSalLayout = ImplLayout( aStr, nIndex, nLen, rStartPt, nWidth );
#else
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, nWidth );
#endif
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }
}

// -----------------------------------------------------------------------

SalLayout* OutputDevice::ImplLayout( const String& rOrigStr,
    xub_StrLen nMinIndex, xub_StrLen nLen,
    const Point& rLogicalPos, long nLogicalWidth, const long* pDXArray ) const
{
    SalLayout* pSalLayout = NULL;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return NULL;
#else
    // due to clipping we may get NULL, so don't use return value
    ImplGetServerGraphics();
#endif

    // initialize font if needed
    if( mbNewFont )
        if( !(const_cast<OutputDevice&>(*this).ImplNewFont()) )
            return NULL;
    if( mbInitFont )
        const_cast<OutputDevice&>(*this).ImplInitFont();

    // get string length for calculating extents
    xub_StrLen nEndIndex = rOrigStr.Len();
    if( (ULONG)nMinIndex + nLen < nEndIndex )
        nEndIndex = nMinIndex + nLen;

    // don't bother if there is nothing to do
    if( nMinIndex >= nEndIndex )
        return NULL;

    // recode string if needed
    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, 0, aStr.Len() );

    int nLayoutFlags = 0;
    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL )
        nLayoutFlags |= SAL_LAYOUT_BIDI_RTL;
    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_STRONG )
        nLayoutFlags |= SAL_LAYOUT_BIDI_STRONG;
    else if( 0 == (mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL) )
    {
        // disable Bidi if no RTL hint and no RTL codes used
        const xub_Unicode* pStr = aStr.GetBuffer() + nMinIndex;
        const xub_Unicode* pEnd = aStr.GetBuffer() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0580) && (*pStr < 0x0800))   // middle eastern scripts
            ||  ((*pStr >= 0xFB18) && (*pStr < 0xFE00))   // hebrew + arabic A presentation forms
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFF00)) ) // arabic presentation forms B
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SAL_LAYOUT_BIDI_STRONG;
    }

    if( mbKerning )
        nLayoutFlags |= SAL_LAYOUT_KERNING_PAIRS;
    if( maFont.GetKerning() & KERNING_ASIAN )
        nLayoutFlags |= SAL_LAYOUT_KERNING_ASIAN;
    if( maFont.IsVertical() )
        nLayoutFlags |= SAL_LAYOUT_VERTICAL;

    if( mnTextLayoutMode & TEXT_LAYOUT_ENABLE_LIGATURES )
        nLayoutFlags |= SAL_LAYOUT_ENABLE_LIGATURES;
    else if( mnTextLayoutMode & TEXT_LAYOUT_COMPLEX_DISABLED )
        nLayoutFlags |= SAL_LAYOUT_COMPLEX_DISABLED;
    else
    {
        // disable CTL for non-CTL text
        const sal_Unicode* pStr = aStr.GetBuffer() + nMinIndex;
        const sal_Unicode* pEnd = aStr.GetBuffer() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0590) && (*pStr < 0x10A0))
            ||  ((*pStr >= 0x1700) && (*pStr < 0x1900))
            ||  ((*pStr >= 0xFB1D) && (*pStr < 0xFE00))   // middle east presentation
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFF00)) ) // arabic presentation B
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SAL_LAYOUT_COMPLEX_DISABLED;
    }

    if( meTextLanguage ) //TODO: (mnTextLayoutMode & TEXT_LAYOUT_SUBSTITUTE_DIGITS)
    {
        // disable character localization when no digits used
        const sal_Unicode* pBase = aStr.GetBuffer();
        const sal_Unicode* pStr = pBase + nMinIndex;
        const sal_Unicode* pEnd = pBase + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
        {
            // TODO: are there non-digit localizations?
            if( (*pStr >= '0') && (*pStr <= '9') )
            {
                // translate characters to local preference
                sal_Unicode cChar = GetLocalizedChar( *pStr, meTextLanguage );
                if( cChar != *pStr )
                    aStr.SetChar( (pStr - pBase), cChar );
            }
        }
    }

    // right align for RTL text, DRAWPOS_REVERSED, RTL window style
    bool bRightAlign = ((mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL) != 0);
    if( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT )
        bRightAlign = false;
    else if ( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_RIGHT )
        bRightAlign = true;
    // SSA: hack for western office, ie text get right aligned
    //      for debugging purposes of mirrored UI
    //static const char* pEnv = getenv( "SAL_RTL_MIRRORTEXT" );
    bool bRTLWindow = (((OutputDevice*)this)->ImplHasMirroredGraphics() && IsRTLEnabled());
    bRightAlign ^= bRTLWindow;
    if( bRightAlign )
        nLayoutFlags |= SAL_LAYOUT_RIGHT_ALIGN;

    // set layout options
    ImplLayoutArgs aLayoutArgs( aStr.GetBuffer(), aStr.Len(), nMinIndex, nEndIndex, nLayoutFlags );

    int nOrientation = mpFontEntry ? mpFontEntry->mnOrientation : 0;
    aLayoutArgs.SetOrientation( nOrientation );

    Point aPixelPos = ImplLogicToDevicePixel( rLogicalPos );

    long nPixelWidth = 0;
    if( nLogicalWidth )
    {
        nPixelWidth = ImplLogicWidthToDevicePixel( nLogicalWidth );
        aLayoutArgs.SetLayoutWidth( nPixelWidth );
    }

    int nLength = nEndIndex - nMinIndex;
    if( pDXArray && mbMap )
    {
        // convert from logical units to font units using a temporary array
        long* pTempDXAry = (long*)alloca( nLength * sizeof(long) );
        // using base position for better rounding a.k.a. "dancing characters"
        int nPixelXOfs = ImplLogicWidthToDevicePixel( rLogicalPos.X() );
        for( int i = 0; i < nLength; ++i )
            pTempDXAry[i] = ImplLogicWidthToDevicePixel( rLogicalPos.X() + pDXArray[i] ) - nPixelXOfs;

        pDXArray = pTempDXAry;
    }
    aLayoutArgs.SetDXArray( pDXArray );

    // get matching layout object for base font
    if( mpPDFWriter )
        pSalLayout = mpPDFWriter->GetTextLayout( aLayoutArgs, &mpFontEntry->maFontSelData );

    if( !pSalLayout )
        pSalLayout = mpGraphics->GetTextLayout( aLayoutArgs, 0 );

    // layout text
    if( pSalLayout && !pSalLayout->LayoutText( aLayoutArgs ) )
    {
        pSalLayout->Release();
        pSalLayout = NULL;
    }

    ImplLayoutRuns aLayoutRuns = aLayoutArgs.maRuns;
    // do glyph fallback if needed
    // #105768# avoid fallback for very small font sizes
    if( mpFontEntry && (mpFontEntry->maFontSelData.mnHeight >= 6)
    && (pSalLayout && aLayoutArgs.PrepareFallback()) )
    {
        // prepare multi level glyph fallback
        MultiSalLayout* pMultiSalLayout = NULL;
        aLayoutArgs.mnFlags |= SAL_LAYOUT_FOR_FALLBACK;

        ImplFontSelectData aFontSelData = mpFontEntry->maFontSelData;
        Size aFontSize( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight );

        for( int nLevel = 1; nLevel < MAX_FALLBACK; ++nLevel )
        {
            // find font family suited for this fallback
            ImplFontEntry* pFallbackFont = mpFontCache->GetFallback( mpFontList,
                maFont, aFontSize, nLevel, mpOutDevData ? mpOutDevData->mpFirstFontSubstEntry : NULL );
            if( !pFallbackFont )
                break;

            // set up fallback font to match the original font
            // don't fallback to itself
            aFontSelData.mpFontData = pFallbackFont->maFontSelData.mpFontData;
            if( mpFontEntry
             && mpFontEntry->maFontSelData.mpFontData == aFontSelData.mpFontData )
            {
                mpFontCache->Release( pFallbackFont );
                continue;
            }

            pFallbackFont->mnSetFontFlags = mpGraphics->SetFont( &aFontSelData, nLevel );

            // create and add fallback layout to multilayout
            aLayoutArgs.ResetPos();
            SalLayout* pFallback = mpGraphics->GetTextLayout( aLayoutArgs, nLevel );
            if( pFallback )
            {
                if( pFallback->LayoutText( aLayoutArgs ) )
                {
                    if( !pMultiSalLayout )
                        pMultiSalLayout = new MultiSalLayout( *pSalLayout );
                    pMultiSalLayout->AddFallback( *pFallback,
                        aLayoutArgs.maRuns, aFontSelData.mpFontData );
                }
                else
                    pFallback->Release();
            }

            mpFontCache->Release( pFallbackFont );

            // break when this fallback was sufficient
            if( !aLayoutArgs.PrepareFallback() )
                break;
        }

        if( pMultiSalLayout && pMultiSalLayout->LayoutText( aLayoutArgs ) )
            pSalLayout = pMultiSalLayout;

        // restore orig font settings
        pSalLayout->InitFont();
    }

    if( pSalLayout )
    {
        // position, justify, etc. the layout
        aLayoutArgs.maRuns = aLayoutRuns;
        pSalLayout->AdjustLayout( aLayoutArgs );
        pSalLayout->DrawBase() = aPixelPos;
        // adjust to right alignment if necessary
        if( bRightAlign )
        {
            long nRTLOffset;
            if( pDXArray )
                nRTLOffset = pDXArray[ nLength-1 ];
            else if( nPixelWidth )
                nRTLOffset = nPixelWidth;
            else
                nRTLOffset = pSalLayout->GetTextWidth() / pSalLayout->GetUnitsPerPixel();
            pSalLayout->DrawOffset().X() = -nRTLOffset;
        }
    }

    return pSalLayout;
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const String& rStr, long nTextWidth,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra, BOOL /*TODO: bCellBreaking*/ ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    xub_StrLen nRetVal = STRING_LEN;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;
        nTextWidth *= nWidthFactor * nSubPixelFactor;
        long nTextPixelWidth = ImplLogicWidthToDevicePixel( nTextWidth );
        long nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = ImplLogicWidthToDevicePixel( nCharExtra );
        }
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        pSalLayout->Release();
    }

    return nRetVal;
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const String& rStr, long nTextWidth,
                                       sal_Unicode nHyphenatorChar, xub_StrLen& rHyphenatorPos,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    rHyphenatorPos = STRING_LEN;

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    if( !pSalLayout )
        return STRING_LEN;

    // convert logical widths into layout units
    // NOTE: be very careful to avoid rounding errors for nCharExtra case
    // problem with rounding errors especially for small nCharExtras
    // TODO: remove when layout units have subpixel granularity
    long nWidthFactor = pSalLayout->GetUnitsPerPixel();
    long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;

    nTextWidth *= nWidthFactor * nSubPixelFactor;
    long nTextPixelWidth = ImplLogicWidthToDevicePixel( nTextWidth );
    long nExtraPixelWidth = 0;
    if( nCharExtra != 0 )
    {
        nCharExtra *= nWidthFactor * nSubPixelFactor;
        nExtraPixelWidth = ImplLogicWidthToDevicePixel( nCharExtra );
    }

    // calculate un-hyphenated break position
    xub_StrLen nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

    // calculate hyphenated break position
    String aHyphenatorStr( &nHyphenatorChar, 1 );
    xub_StrLen nTempLen = 1;
    SalLayout* pHyphenatorLayout = ImplLayout( aHyphenatorStr, 0, nTempLen );
    if( pHyphenatorLayout )
    {
        // calculate subpixel width of hyphenation character
        long nHyphenatorPixelWidth = pHyphenatorLayout->GetTextWidth() * nSubPixelFactor;
        pHyphenatorLayout->Release();

        // calculate hyphenated break position
        nTextPixelWidth -= nHyphenatorPixelWidth;
        if( nExtraPixelWidth > 0 )
            nTextPixelWidth -= nExtraPixelWidth;

        rHyphenatorPos = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        if( rHyphenatorPos > nRetVal )
            rHyphenatorPos = nRetVal;
    }

    pSalLayout->Release();
    return nRetVal;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawText( const Rectangle& rRect,
                             const String& rOrigStr, USHORT nStyle,
                             MetricVector* pVector, String* pDisplayText )

{
    if( mpOutDevData && mpOutDevData->mpRecordLayout )
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

    DBG_TRACE( "OutputDevice::DrawText( const Rectangle& )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextRectAction( rRect, rOrigStr, nStyle ) );

    if ( ( !IsDeviceOutputNecessary() && ! pVector ) || !rOrigStr.Len() || rRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;
#else
    if( !ImplGetServerGraphics() )
        return;
#endif

    Color aOldTextColor;
    Color aOldTextFillColor;
    BOOL  bRestoreFillColor;
    if ( (nStyle & TEXT_DRAW_DISABLE) && ! pVector )
    {
        BOOL  bHighContrastBlack = FALSE;
        BOOL  bHighContrastWhite = FALSE;
        Color aCol;
        if( IsBackground() )
            aCol = GetBackground().GetColor();
        else
            // best guess is the face color here
            // but it may be totally wrong. the background color
            // was typically already reset
            aCol = GetSettings().GetStyleSettings().GetFaceColor();

        bHighContrastBlack = aCol.IsDark();
        bHighContrastWhite = aCol.IsBright() && GetSettings().GetStyleSettings().GetHighContrastMode();

        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = TRUE;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = FALSE;
        if( bHighContrastBlack )
            SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            SetTextColor( COL_LIGHTGREEN );
        else
        {
            SetTextColor( GetSettings().GetStyleSettings().GetLightColor() );
            Rectangle aRect = rRect;
            aRect.Move( 1, 1 );
            DrawText( aRect, rOrigStr, nStyle & ~TEXT_DRAW_DISABLE );
            SetTextColor( GetSettings().GetStyleSettings().GetShadowColor() );
        }
    }

    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & TEXT_DRAW_CLIP) )
        return;

    Point       aPos            = rRect.TopLeft();

    long        nTextHeight     = GetTextHeight();
    TextAlign   eAlign          = GetTextAlign();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    // Mehrzeiligen Text behandeln wir anders
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {

        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        long                    nMaxTextWidth;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            nMaxTextWidth = ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // Letzte Zeile zusammenbauen und kuerzen
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = aStr.Copy( pLineInfo->GetIndex() );
                    aLastLine.ConvertLineEnd( LINEEND_LF );
                    // Alle LineFeed's durch Spaces ersetzen
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == _LF )
                            aLastLine.SetChar( i, ' ' );
                    }
                    aLastLine = GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }
            else
            {
                if ( nMaxTextWidth <= nWidth )
                    nStyle &= ~TEXT_DRAW_CLIP;
            }

            // Muss in der Hoehe geclippt werden?
            if ( nFormatLines*nTextHeight > nHeight )
                nStyle |= TEXT_DRAW_CLIP;

            // Clipping setzen
            if ( nStyle & TEXT_DRAW_CLIP )
            {
                Push( PUSH_CLIPREGION );
                IntersectClipRegion( rRect );
            }

            // Vertikales Alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // Font Alignment
            if ( eAlign == ALIGN_BOTTOM )
                aPos.Y() += nTextHeight;
            else if ( eAlign == ALIGN_BASELINE )
                aPos.Y() += GetFontMetric().GetAscent();

            // Alle Zeilen ausgeben, bis auf die letzte
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
                DrawText( aPos, aStr, nIndex, nLineLen, pVector, pDisplayText );
                if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
                {
                    if ( (nMnemonicPos >= nIndex) && (nMnemonicPos < nIndex+nLineLen) )
                    {
                        long        nMnemonicX;
                        long        nMnemonicY;
                        long        nMnemonicWidth;

                        long *pCaretXArray = (long*) alloca( 2 * sizeof(long) * nLineLen );
                        BOOL bRet = GetCaretPositions( aStr, pCaretXArray,
                                                nIndex, nLineLen);
                        long lc_x1 = pCaretXArray[2*(nMnemonicPos - nIndex)];
                        long lc_x2 = pCaretXArray[2*(nMnemonicPos - nIndex)+1];
                        nMnemonicWidth = ::abs((int)(lc_x1 - lc_x2));

                        Point       aTempPos = LogicToPixel( aPos );
#if (_MSC_VER < 1300)
                        nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( std::min( lc_x1, lc_x2 ) );
#else
                        nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( min( lc_x1, lc_x2 ) );
#endif
                        nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
                        ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
                    }
                }
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }


            // Gibt es noch eine letzte Zeile, dann diese linksbuendig ausgeben,
            // da die Zeile gekuerzt wurde
            if ( aLastLine.Len() )
                DrawText( aPos, aLastLine, 0, STRING_LEN, pVector, pDisplayText );

            // Clipping zuruecksetzen
            if ( nStyle & TEXT_DRAW_CLIP )
                Pop();
        }
    }
    else
    {
        long nTextWidth = GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & TEXT_DRAW_ELLIPSIS )
            {
                aStr = GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = GetTextWidth( aStr );
            }
        }
        else
        {
            if ( nTextHeight <= nHeight )
                nStyle &= ~TEXT_DRAW_CLIP;
        }

        // Vertikales Alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        // Font Alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += nTextHeight;
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += GetFontMetric().GetAscent();

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        long        nMnemonicX;
        long        nMnemonicY;
        long        nMnemonicWidth;
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            long *pCaretXArray = (long*) alloca( 2 * sizeof(long) * aStr.Len() );
            BOOL bRet = GetCaretPositions( aStr, pCaretXArray, 0, aStr.Len() );
            long lc_x1 = pCaretXArray[2*(nMnemonicPos)];
            long lc_x2 = pCaretXArray[2*(nMnemonicPos)+1];
            nMnemonicWidth = ::abs((int)(lc_x1 - lc_x2));

            Point aTempPos = LogicToPixel( aPos );
#if (_MSC_VER < 1300)
            nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( std::min(lc_x1, lc_x2) );
#else
            nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( min(lc_x1, lc_x2) );
#endif
            nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
        }

        if ( nStyle & TEXT_DRAW_CLIP )
        {
            Push( PUSH_CLIPREGION );
            IntersectClipRegion( rRect );
            DrawText( aPos, aStr, 0, STRING_LEN, pVector, pDisplayText );
            if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
            Pop();
        }
        else
        {
            DrawText( aPos, aStr, 0, STRING_LEN, pVector, pDisplayText );
            if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE && !pVector )
    {
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
}

// -----------------------------------------------------------------------

Rectangle OutputDevice::GetTextRect( const Rectangle& rRect,
                                     const String& rOrigStr, USHORT nStyle,
                                     TextRectInfo* pInfo ) const
{
    DBG_TRACE( "OutputDevice::GetTextRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Rectangle           aRect = rRect;
    xub_StrLen          nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        xub_StrLen              nFormatLines;
        xub_StrLen              i;

        nMaxWidth = 0;
        ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
        nFormatLines = aMultiLineInfo.Count();
        if ( !nTextHeight )
            nTextHeight = 1;
        nLines = (USHORT)(aRect.GetHeight()/nTextHeight);
        if ( pInfo )
            pInfo->mnLineCount = nFormatLines;
        if ( !nLines )
            nLines = 1;
        if ( nFormatLines <= nLines )
            nLines = nFormatLines;
        else
        {
            if ( !(nStyle & TEXT_DRAW_ENDELLIPSIS) )
                nLines = nFormatLines;
            else
            {
                if ( pInfo )
                    pInfo->mbEllipsis = TRUE;
                nMaxWidth = nWidth;
            }
        }
        if ( pInfo )
        {
            BOOL bMaxWidth = nMaxWidth == 0;
            pInfo->mnMaxWidth = 0;
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( bMaxWidth && (pLineInfo->GetWidth() > nMaxWidth) )
                    nMaxWidth = pLineInfo->GetWidth();
                if ( pLineInfo->GetWidth() > pInfo->mnMaxWidth )
                    pInfo->mnMaxWidth = pLineInfo->GetWidth();
            }
        }
        else if ( !nMaxWidth )
        {
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( pLineInfo->GetWidth() > nMaxWidth )
                    nMaxWidth = pLineInfo->GetWidth();
            }
        }
    }
    else
    {
        nLines      = 1;
        nMaxWidth   = GetTextWidth( aStr );

        if ( pInfo )
        {
            pInfo->mnLineCount  = 1;
            pInfo->mnMaxWidth   = nMaxWidth;
        }

        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ELLIPSIS) )
        {
            if ( pInfo )
                pInfo->mbEllipsis = TRUE;
            nMaxWidth = nWidth;
        }
    }

    if ( nStyle & TEXT_DRAW_RIGHT )
        aRect.Left() = aRect.Right()-nMaxWidth+1;
    else if ( nStyle & TEXT_DRAW_CENTER )
    {
        aRect.Left() += (nWidth-nMaxWidth)/2;
        aRect.Right() = aRect.Left()+nMaxWidth-1;
    }
    else
        aRect.Right() = aRect.Left()+nMaxWidth-1;

    if ( nStyle & TEXT_DRAW_BOTTOM )
        aRect.Top() = aRect.Bottom()-(nTextHeight*nLines)+1;
    else if ( nStyle & TEXT_DRAW_VCENTER )
    {
        aRect.Top()   += (aRect.GetHeight()-(nTextHeight*nLines))/2;
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;
    }
    else
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;

    aRect.Right()++; // #99188# get rid of rounding problems when using this rect later
    return aRect;
}

// -----------------------------------------------------------------------

static BOOL ImplIsCharIn( xub_Unicode c, const sal_Char* pStr )
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return TRUE;
        pStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

String OutputDevice::GetEllipsisString( const String& rOrigStr, long nMaxWidth,
                                        USHORT nStyle ) const
{
    DBG_TRACE( "OutputDevice::GetEllipsisString()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    String aStr = rOrigStr;
    xub_StrLen nIndex = GetTextBreak( aStr, nMaxWidth );


    if ( nIndex != STRING_LEN )
    {
        if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
        {
            aStr.Erase( nIndex );
            if ( nIndex > 1 )
            {
                aStr.AppendAscii( "..." );
                while ( aStr.Len() && (GetTextWidth( aStr ) > nMaxWidth) )
                {
                    if ( (nIndex > 1) || (nIndex == aStr.Len()) )
                        nIndex--;
                    aStr.Erase( nIndex, 1 );
                }
            }

            if ( !aStr.Len() && (nStyle & TEXT_DRAW_CLIP) )
                aStr += rOrigStr.GetChar( 0 );
        }
        else if ( nStyle & TEXT_DRAW_PATHELLIPSIS )
        {
            rtl::OUString aPath( rOrigStr );
            rtl::OUString aAbbreviatedPath;
            osl_abbreviateSystemPath( aPath.pData, &aAbbreviatedPath.pData, nIndex, NULL );
            aStr = aAbbreviatedPath;
        }
        else if ( nStyle & TEXT_DRAW_NEWSELLIPSIS )
        {
            static sal_Char const   pSepChars[] = ".";
            // Letztes Teilstueck ermitteln
            xub_StrLen nLastContent = aStr.Len();
            while ( nLastContent )
            {
                nLastContent--;
                if ( ImplIsCharIn( aStr.GetChar( nLastContent ), pSepChars ) )
                    break;
            }
            while ( nLastContent &&
                    ImplIsCharIn( aStr.GetChar( nLastContent-1 ), pSepChars ) )
                nLastContent--;

            XubString aLastStr( aStr, nLastContent, aStr.Len() );
            XubString aTempLastStr( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            aTempLastStr += aLastStr;
            if ( GetTextWidth( aTempLastStr ) > nMaxWidth )
                aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
            else
            {
                USHORT nFirstContent = 0;
                while ( nFirstContent < nLastContent )
                {
                    nFirstContent++;
                    if ( ImplIsCharIn( aStr.GetChar( nFirstContent ), pSepChars ) )
                        break;
                }
                while ( (nFirstContent < nLastContent) &&
                        ImplIsCharIn( aStr.GetChar( nFirstContent ), pSepChars ) )
                    nFirstContent++;

                if ( nFirstContent >= nLastContent )
                    aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                else
                {
                    if ( nFirstContent > 4 )
                        nFirstContent = 4;
                    XubString aFirstStr( aStr, 0, nFirstContent );
                    aFirstStr.AppendAscii( "..." );
                    XubString aTempStr = aFirstStr;
                    aTempStr += aLastStr;
                    if ( GetTextWidth( aTempStr ) > nMaxWidth )
                        aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                    else
                    {
                        do
                        {
                            aStr = aTempStr;
                            if( nLastContent > aStr.Len() )
                                nLastContent = aStr.Len();
                            while ( nFirstContent < nLastContent )
                            {
                                nLastContent--;
                                if ( ImplIsCharIn( aStr.GetChar( nLastContent ), pSepChars ) )
                                    break;

                            }
                            while ( (nFirstContent < nLastContent) &&
                                    ImplIsCharIn( aStr.GetChar( nLastContent-1 ), pSepChars ) )
                                nLastContent--;

                            if ( nFirstContent < nLastContent )
                            {
                                XubString aTempLastStr( aStr, nLastContent, aStr.Len() );
                                aTempStr = aFirstStr;
                                aTempStr += aTempLastStr;
                                if ( GetTextWidth( aTempStr ) > nMaxWidth )
                                    break;
                            }
                        }
                        while ( nFirstContent < nLastContent );
                    }
                }
            }
        }
    }

    return aStr;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawCtrlText( const Point& rPos, const XubString& rStr,
                                 xub_StrLen nIndex, xub_StrLen nLen,
                                 USHORT nStyle, MetricVector* pVector, String* pDisplayText )
{
    DBG_TRACE( "OutputDevice::DrawCtrlText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !IsDeviceOutputNecessary() || (nIndex >= rStr.Len()) )
        return;

    // better get graphics here because ImplDrawMnemonicLine() will not
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if( !ImplGetServerGraphics() )
        return;
#endif

    if( nIndex >= rStr.Len() )
        return;
    if( (ULONG)nIndex+nLen >= rStr.Len() )
        nLen = rStr.Len() - nIndex;

    XubString   aStr = rStr;
    xub_StrLen  nMnemonicPos = STRING_NOTFOUND;

    long        nMnemonicX;
    long        nMnemonicY;
    long        nMnemonicWidth;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if( nMnemonicPos < nIndex )
                --nIndex;
            else if( nLen < STRING_LEN )
            {
                if( nMnemonicPos < (nIndex+nLen) )
                    --nLen;
                DBG_ASSERT( nMnemonicPos < (nIndex+nLen), "Mnemonic underline marker after last character" );
            }
            BOOL bInvalidPos = FALSE;

            if( nMnemonicPos >= nLen )
            {
                // #106952#
                // may occur in BiDi-Strings: the '~' is sometimes found behind the last char
                // due to some strange BiDi text editors
                // ->place the underline behind the string to indicate a failure
                bInvalidPos = TRUE;
                nMnemonicPos = nLen-1;
            }

            long *pCaretXArray = (long*)alloca( 2 * sizeof(long) * nLen );
            BOOL bRet = GetCaretPositions( aStr, pCaretXArray, nIndex, nLen );
            long lc_x1 = pCaretXArray[ 2*(nMnemonicPos - nIndex) ];
            long lc_x2 = pCaretXArray[ 2*(nMnemonicPos - nIndex)+1 ];
            nMnemonicWidth = ::abs((int)(lc_x1 - lc_x2));

#if (_MSC_VER < 1300)
            Point aTempPos( std::min(lc_x1,lc_x2), GetFontMetric().GetAscent() );
            if( bInvalidPos )  // #106952#, place behind the (last) character
                aTempPos = Point( std::max(lc_x1,lc_x2), GetFontMetric().GetAscent() );
#else
            Point aTempPos( min(lc_x1,lc_x2), GetFontMetric().GetAscent() );
            if( bInvalidPos )  // #106952#, place behind the (last) character
                aTempPos = Point( max(lc_x1,lc_x2), GetFontMetric().GetAscent() );
#endif
            aTempPos += rPos;
            aTempPos = LogicToPixel( aTempPos );
            nMnemonicX = mnOutOffX + aTempPos.X();
            nMnemonicY = mnOutOffY + aTempPos.Y();
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE && ! pVector )
    {
        Color aOldTextColor;
        Color aOldTextFillColor;
        BOOL  bRestoreFillColor;
        BOOL  bHighContrastBlack = FALSE;
        BOOL  bHighContrastWhite = FALSE;
        if( IsBackground() )
        {
            Wallpaper aWall = GetBackground();
            Color aCol = aWall.GetColor();
            bHighContrastBlack = aCol.IsDark();
            bHighContrastWhite = aCol.IsBright() && GetSettings().GetStyleSettings().GetHighContrastMode();
        }
        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = TRUE;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = FALSE;

        if( bHighContrastBlack )
            SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            SetTextColor( COL_LIGHTGREEN );
        else
        {
            SetTextColor( GetSettings().GetStyleSettings().GetLightColor() );
            DrawText( Point( rPos.X()+1, rPos.Y()+1 ), aStr, nIndex, nLen );
            if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    ImplDrawMnemonicLine( nMnemonicX+1, nMnemonicY+1, nMnemonicWidth );
            }
            SetTextColor( GetSettings().GetStyleSettings().GetShadowColor() );
        }

        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
        {
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
    else
    {
        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
        {
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }
}

// -----------------------------------------------------------------------

long OutputDevice::GetCtrlTextWidth( const String& rStr,
                                     xub_StrLen nIndex, xub_StrLen nLen,
                                     USHORT nStyle ) const
{
    DBG_TRACE( "OutputDevice::GetCtrlTextSize()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        xub_StrLen  nMnemonicPos;
        XubString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nLen < STRING_LEN) &&
                      (nMnemonicPos >= nIndex) && (nMnemonicPos < (ULONG)(nIndex+nLen)) )
                nLen--;
        }
        return GetTextWidth( aStr, nIndex, nLen );
    }
    else
        return GetTextWidth( rStr, nIndex, nLen );
}

// -----------------------------------------------------------------------

String OutputDevice::GetNonMnemonicString( const String& rStr, xub_StrLen& rMnemonicPos )
{
    String   aStr    = rStr;
    xub_StrLen  nLen    = aStr.Len();
    xub_StrLen  i       = 0;

    rMnemonicPos = STRING_NOTFOUND;
    while ( i < nLen )
    {
        if ( aStr.GetChar( i ) == '~' )
        {
            if ( aStr.GetChar( i+1 ) != '~' )
            {
                if ( rMnemonicPos == STRING_NOTFOUND )
                    rMnemonicPos = i;
                aStr.Erase( i, 1 );
                nLen--;
            }
            else
            {
                aStr.Erase( i, 1 );
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetDevFontCount() const
{
    DBG_TRACE( "OutputDevice::GetDevFontCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    // if we already have a font list we know enough
    if( mpGetDevFontList )
        return (USHORT)mpGetDevFontList->Count();

    ((OutputDevice*)this)->mpGetDevFontList = new ImplGetDevFontList;

    // fill font list
    ImplDevFontListData* pFontListData = mpFontList->First();
    while ( pFontListData )
    {
        ImplFontData*   pLastData = NULL;
        ImplFontData*   pData = pFontListData->mpFirst;
        while ( pData )
        {
            // Compare with the last font, because we wan't in the list
            // only fonts, that have different attributes, but not
            // different sizes
            if ( !pLastData ||
                 (ImplCompareFontDataWithoutSize( pLastData, pData ) != 0) )
                mpGetDevFontList->Add( pData );

            pLastData = pData;
            pData = pData->mpNext;
        }

        pFontListData = mpFontList->Next();
    }

    return (USHORT)mpGetDevFontList->Count();
}

// -----------------------------------------------------------------------

FontInfo OutputDevice::GetDevFont( USHORT nDevFont ) const
{
    DBG_TRACE( "OutputDevice::GetDevFont()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ImplInitFontList();

    FontInfo    aFontInfo;
    USHORT      nCount = GetDevFontCount();

    // Wertebereich ueberpruefen
    if ( nDevFont < nCount )
    {
        ImplFontData* pData = mpGetDevFontList->Get( nDevFont );
        aFontInfo.SetName( pData->maName );
        aFontInfo.SetStyleName( pData->maStyleName );
        aFontInfo.SetCharSet( pData->meCharSet );
        aFontInfo.SetFamily( pData->meFamily );
        aFontInfo.SetPitch( pData->mePitch );
        aFontInfo.SetWeight( pData->meWeight );
        aFontInfo.SetItalic( pData->meItalic );
        aFontInfo.mpImplMetric->meType = pData->meType;
        aFontInfo.mpImplMetric->mbDevice = pData->mbDevice;
    }

    return aFontInfo;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::AddTempDevFont( const String& rFileURL, const String& rFontName )
{
    DBG_TRACE( "OutputDevice::AddTempDevFont()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ImplInitFontList();

    if( !mpGraphics && !ImplGetGraphics() )
        return FALSE;

    ImplFontData* pFontData = mpGraphics->AddTempDevFont( rFileURL, rFontName );
    if( !pFontData )
        return FALSE;
    mpFontList->Add( pFontData );
    return TRUE;
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetDevFontSizeCount( const Font& rFont ) const
{
    DBG_TRACE( "OutputDevice::GetDevFontSizeCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    XubString aFontName = rFont.GetName();

    // create size list if it doesn't exists yet or the font name differs
    if ( mpGetDevSizeList )
    {
        if ( mpGetDevSizeList->GetFontName() == aFontName )
            return (USHORT)mpGetDevSizeList->Count();
        else
        {
            mpGetDevSizeList->Clear();
            mpGetDevSizeList->SetFontName( aFontName );
        }
    }
    else
        ((OutputDevice*)this)->mpGetDevSizeList = new ImplGetDevSizeList( aFontName );

    ImplInitFontList();
    // add fonts from the font list to the size list
    ImplDevFontListData* pFontListData = mpFontList->FindFont( aFontName );
    if ( pFontListData )
    {
        ImplFontData* pData = pFontListData->mpFirst;
        do
        {
            mpGetDevSizeList->Add( pData->mnHeight );
            pData = pData->mpNext;
        }
        while ( pData );
    }

    return (USHORT)mpGetDevSizeList->Count();
}

// -----------------------------------------------------------------------

Size OutputDevice::GetDevFontSize( const Font& rFont, USHORT nSize ) const
{
    DBG_TRACE( "OutputDevice::GetDevFontSize()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    USHORT nCount = GetDevFontSizeCount( rFont );

    // Wertebereich ueberpruefen
    if ( nSize >= nCount )
        return Size();

    // Wenn MapMode gesetzt ist, wird auf ,5-Points gerundet
    Size aSize( 0, mpGetDevSizeList->Get( nSize ) );
    if ( mbMap )
    {
        aSize.Height() *= 10;
        MapMode aMap( MAP_10TH_INCH, Point(), Fraction( 1, 72 ), Fraction( 1, 72 ) );
        aSize = PixelToLogic( aSize, aMap );
        aSize.Height() += 5;
        aSize.Height() /= 10;
        long nRound = aSize.Height() % 5;
        if ( nRound >= 3 )
            aSize.Height() += (5-nRound);
        else
            aSize.Height() -= nRound;
        aSize.Height() *= 10;
        aSize = LogicToPixel( aSize, aMap );
        aSize = PixelToLogic( aSize );
        aSize.Height() += 5;
        aSize.Height() /= 10;
    }
    return aSize;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::IsFontAvailable( const XubString& rFontName ) const
{
    DBG_TRACE( "OutputDevice::IsFontAvailable()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    return (mpFontList->FindFont( rFontName ) != 0);
}

// -----------------------------------------------------------------------

FontMetric OutputDevice::GetFontMetric() const
{
    DBG_TRACE( "OutputDevice::GetFontMetric()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    FontMetric aMetric;
    if( mbNewFont && !(const_cast<OutputDevice&>(*this).ImplNewFont()) )
        return aMetric;

    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);

    // prepare metric
    aMetric.Font::operator=( maFont );

    // set aMetric with info from font
    aMetric.SetName( maFont.GetName() );
    aMetric.SetStyleName( pMetric->maStyleName );
    aMetric.SetSize( PixelToLogic( Size( pMetric->mnWidth, pMetric->mnAscent+pMetric->mnDescent-pMetric->mnIntLeading ) ) );
    aMetric.SetCharSet( pMetric->meCharSet );
    aMetric.SetFamily( pMetric->meFamily );
    aMetric.SetPitch( pMetric->mePitch );
    aMetric.SetWeight( pMetric->meWeight );
    aMetric.SetItalic( pMetric->meItalic );
    if ( pEntry->mnOwnOrientation )
        aMetric.SetOrientation( pEntry->mnOwnOrientation );
    else
        aMetric.SetOrientation( pMetric->mnOrientation );
    if ( !pEntry->mnKernPairs )
        aMetric.SetKerning( aMetric.GetKerning() & ~KERNING_FONTSPECIFIC );

    // we want set correct family and pitch data, if we can't query the
    // data from the system
    if ( (aMetric.GetFamily() == FAMILY_DONTKNOW) ||
         (aMetric.GetPitch() == PITCH_DONTKNOW) )
    {
        const FontSubstConfigItem::FontNameAttr* pTempFontAttr;
        const FontSubstConfigItem* pFontSubst = FontSubstConfigItem::get();

        String                  aTempName = pMetric->maName;
        String                  aTempShortName;
        String                  aTempFamilyName;
        ULONG                   nTempType = 0;
        FontWeight              eTempWeight = WEIGHT_DONTKNOW;
        FontWidth               eTempWidth = WIDTH_DONTKNOW;
        ImplGetEnglishSearchFontName( aTempName );
        FontSubstConfigItem::getMapName( aTempName, aTempShortName, aTempFamilyName,
                                         eTempWeight, eTempWidth, nTempType );
        pTempFontAttr = pFontSubst->getSubstInfo( aTempName );
        if ( !pTempFontAttr && (aTempShortName != aTempName) )
            pTempFontAttr = pFontSubst->getSubstInfo( aTempShortName );
        if ( pTempFontAttr && pTempFontAttr->HTMLSubstitutions.size() )
        {
            // We use the HTML-Substitution string to overwrite these only
            // for standard fonts - other fonts should be the default, because
            // it's not easy to define the correct definition (for example
            // for none latin fonts or special fonts).
            if ( aMetric.GetFamily() == FAMILY_DONTKNOW )
            {
                if ( pTempFontAttr->Type & IMPL_FONT_ATTR_SERIF )
                    aMetric.SetFamily( FAMILY_ROMAN );
                else if ( pTempFontAttr->Type & IMPL_FONT_ATTR_SANSSERIF )
                    aMetric.SetFamily( FAMILY_SWISS );
                else if ( pTempFontAttr->Type & IMPL_FONT_ATTR_TYPEWRITER )
                    aMetric.SetFamily( FAMILY_MODERN );
                else if ( pTempFontAttr->Type & IMPL_FONT_ATTR_ITALIC )
                    aMetric.SetFamily( FAMILY_SCRIPT );
                else if ( pTempFontAttr->Type & IMPL_FONT_ATTR_DECORATIVE )
                    aMetric.SetFamily( FAMILY_DECORATIVE );
            }
            if ( aMetric.GetPitch() == PITCH_DONTKNOW )
            {
                if ( pTempFontAttr->Type & IMPL_FONT_ATTR_FIXED )
                    aMetric.SetPitch( PITCH_FIXED );
            }
        }
    }

    // set remaining metric fields
    aMetric.mpImplMetric->meType        = pMetric->meType;
    aMetric.mpImplMetric->mbDevice      = pMetric->mbDevice;
    aMetric.mpImplMetric->mnAscent      = ImplDevicePixelToLogicHeight( pMetric->mnAscent+mnEmphasisAscent );
    aMetric.mpImplMetric->mnDescent     = ImplDevicePixelToLogicHeight( pMetric->mnDescent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnIntLeading  = ImplDevicePixelToLogicHeight( pMetric->mnIntLeading+mnEmphasisAscent );
    aMetric.mpImplMetric->mnExtLeading  = ImplDevicePixelToLogicHeight( pMetric->mnExtLeading );
    aMetric.mpImplMetric->mnLineHeight  = ImplDevicePixelToLogicHeight( pMetric->mnAscent+pMetric->mnDescent+mnEmphasisAscent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnSlant       = ImplDevicePixelToLogicHeight( pMetric->mnSlant );
    aMetric.mpImplMetric->mnFirstChar   = pMetric->mnFirstChar;
    aMetric.mpImplMetric->mnLastChar    = pMetric->mnLastChar;

    return aMetric;
}

// -----------------------------------------------------------------------

FontMetric OutputDevice::GetFontMetric( const Font& rFont ) const
{
    // select font, query metrics, select original font again
    Font aOldFont = GetFont();
    const_cast<OutputDevice*>(this)->SetFont( rFont );
    FontMetric aMetric( GetFontMetric() );
    const_cast<OutputDevice*>(this)->SetFont( aOldFont );
    return aMetric;
}

// -----------------------------------------------------------------------

ULONG OutputDevice::GetKerningPairCount() const
{
    DBG_TRACE( "OutputDevice::GetKerningPairCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    const_cast<OutputDevice*>(this)->ImplInitKerningPairs();
    return mpFontEntry->mnKernPairs;
}

// -----------------------------------------------------------------------

// TODO: best is to get rid of this method completely
void OutputDevice::GetKerningPairs( ULONG nPairs, KerningPair* pKernPairs ) const
{
    DBG_TRACE( "OutputDevice::GetKerningPairs()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    // TODO: kerning pairs are no longer used by anybody else in this layer
    // TODO: remove it from FontEntry and from ImplInitFont()
    // NOTE: beware that OutputDevice owns the kern array
    const_cast<OutputDevice*>(this)->ImplInitKerningPairs();
    if ( nPairs > mpFontEntry->mnKernPairs )
        nPairs = mpFontEntry->mnKernPairs;
    if ( nPairs )
        memcpy( pKernPairs, mpFontEntry->mpKernPairs, nPairs*sizeof( KerningPair ) );
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetGlyphBoundRects( const Point& rOrigin, const String& rStr,
    int nIndex, int nLen, int nBase, MetricVector& rVector )
{
    DBG_TRACE( "OutputDevice::GetGlyphBoundRect_CTL()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    rVector.clear();

    if( nLen == STRING_LEN )
        nLen = rStr.Len() - nIndex;

    Rectangle aRect;
    for( int i = 0; i < nLen; i++ )
    {
        if( !GetTextBoundRect( aRect, rStr, nBase, nIndex+i, 1 ) )
            break;
        aRect.Move( rOrigin.X(), rOrigin.Y() );
        rVector.push_back( aRect );
    }

    return (nLen == (int)rVector.size());
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetTextBoundRect( Rectangle& rRect,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex,
    xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextBoundRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BOOL bRet = FALSE;
    rRect.SetEmpty();

    SalLayout* pSalLayout = NULL;
    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
#if (_MSC_VER < 1300)
        xub_StrLen nStart = std::min( nBase, nIndex );
        xub_StrLen nOfsLen = std::max( nBase, nIndex ) - nStart;
#else
        xub_StrLen nStart = min( nBase, nIndex );
        xub_StrLen nOfsLen = max( nBase, nIndex ) - nStart;
#endif
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            nXOffset /= pSalLayout->GetUnitsPerPixel();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase < nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen );
    Rectangle aPixelRect;
    if( pSalLayout )
    {
        bRet = pSalLayout->GetBoundRect( *mpGraphics, aPixelRect );

        if( bRet )
        {
            int nWidthFactor = pSalLayout->GetUnitsPerPixel();

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aPixelRect.Left()
                    = static_cast< long >(aPixelRect.Left() * fFactor);
                aPixelRect.Right()
                    = static_cast< long >(aPixelRect.Right() * fFactor);
                aPixelRect.Top()
                    = static_cast< long >(aPixelRect.Top() * fFactor);
                aPixelRect.Bottom()
                    = static_cast< long >(aPixelRect.Bottom() * fFactor);
            }

            Point aRotatedOfs( mnTextOffX, mnTextOffY );
            aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
            aPixelRect += aRotatedOfs;
            rRect = PixelToLogic( aPixelRect );
            if( mbMap )
                rRect += Point( maMapRes.mnMapOfsX, maMapRes.mnMapOfsY );
        }

        pSalLayout->Release();
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) )
        return bRet;

    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    VirtualDevice aVDev( 1 );
    Font aFont( GetFont() );
    aFont.SetShadow( FALSE );
    aFont.SetOutline( FALSE );
    aFont.SetRelief( RELIEF_NONE );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    aVDev.SetFont( aFont );
    aVDev.SetTextAlign( ALIGN_TOP );

    // layout the text on the virtual device
    pSalLayout = aVDev.ImplLayout( rStr, nIndex, nLen );
    if( !pSalLayout )
        return false;

    // make the bitmap big enough
    // TODO: use factors when it would get too big
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
    Point aOffset( nWidth/2, 8 );
    Size aSize( nWidth + 2*aOffset.X(), nHeight + 2*aOffset.Y() );
    if( !nWidth || !aVDev.SetOutputSizePixel( aSize ) )
        return false;

    // draw text in black
    pSalLayout->DrawBase() = aOffset;
    aVDev.SetTextColor( Color( COL_BLACK ) );
    aVDev.SetTextFillColor();
    aVDev.ImplInitTextColor();
    aVDev.ImplDrawText( *pSalLayout );
    pSalLayout->Release();

    // find extents using the bitmap
    Bitmap aBmp = aVDev.GetBitmap( Point(), aSize );
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();
    if( !pAcc )
        return FALSE;
    const BitmapColor aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
    const long nW = pAcc->Width();
    const long nH = pAcc->Height();
    long nLeft, nRight;

    // find top left point
    long nTop = 0;
    for(; nTop < nH; ++nTop )
    {
        for( nLeft = 0; nLeft < nW; ++nLeft )
            if( pAcc->GetPixel( nTop, nLeft ) == aBlack )
                break;
        if( nLeft < nW )
            break;
    }

    // find bottom right point
    long nBottom = nH;
    while( --nBottom >= nTop )
    {
        for( nRight = nW; --nRight >= 0; )
            if( pAcc->GetPixel( nBottom, nRight ) == aBlack )
                break;
        if( nRight >= 0 )
            break;
    }
    if( nRight < nLeft )
    {
        long nX = nRight;
        nRight = nLeft;
        nLeft  = nX;
    }

    for( long nY = nTop; nY <= nBottom; ++nY )
    {
        // find leftmost point
        long nX;
        for( nX = 0; nX < nLeft; ++nX )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nLeft = nX;

        // find rightmost point
        for( nX = nW; --nX > nRight; )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nRight = nX;
    }

    aBmp.ReleaseAccess( pAcc );

    if( nTop <= nBottom )
    {
        Size aSize( nRight - nLeft + 1, nBottom - nTop + 1 );
        Point aTopLeft( nLeft, nTop );
        aTopLeft -= aOffset;
        // adjust to text alignment
        aTopLeft.Y()+= mnTextOffY - (mpFontEntry->maMetric.mnAscent + mnEmphasisAscent);
        // convert to logical coordinates
        aSize = PixelToLogic( aSize );
        aTopLeft.X() = ImplDevicePixelToLogicWidth( aTopLeft.X() );
        aTopLeft.Y() = ImplDevicePixelToLogicHeight( aTopLeft.Y() );
        rRect = Rectangle( aTopLeft, aSize );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetTextOutline( PolyPolygon& rPolyPoly,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex, xub_StrLen nLen,
    BOOL bOptimize, const ULONG nTWidth, const long* pDXArray ) const
{
    rPolyPoly.Clear();
    PolyPolyVector aVector;
    if (!GetTextOutlines(aVector, rStr, nBase, nIndex, nLen, bOptimize, nTWidth, pDXArray ))
        return false;
    for (PolyPolyVector::iterator aIt(aVector.begin()); aIt != aVector.end();
         ++aIt)
        for (USHORT i = 0; i < aIt->Count(); ++i)
            rPolyPoly.Insert((*aIt)[i]);
    return true;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetTextOutlines( PolyPolyVector& rVector,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex,
    xub_StrLen nLen, BOOL bOptimize, const ULONG nTWidth, const long* pDXArray ) const
{
    BOOL bRet = FALSE;
    rVector.clear();
    if( nLen == STRING_LEN )
        nLen = rStr.Len() - nIndex;
    rVector.reserve( nLen );

    // we want to get the Rectangle in logical units, so to
    // avoid rounding errors we just size the font in logical units
    BOOL bOldMap = mbMap;
    if( bOldMap )
    {
        const_cast<OutputDevice&>(*this).mbMap = FALSE;
        const_cast<OutputDevice&>(*this).mbNewFont = TRUE;
    }

    SalLayout* pSalLayout = NULL;

    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
#if (_MSC_VER < 1300)
        xub_StrLen nStart = std::min( nBase, nIndex );
        xub_StrLen nOfsLen = std::max( nBase, nIndex ) - nStart;
#else
        xub_StrLen nStart = min( nBase, nIndex );
        xub_StrLen nOfsLen = max( nBase, nIndex ) - nStart;
#endif
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, Point( 0,0 ), nTWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point( 0,0 ), nTWidth, pDXArray );
    if( pSalLayout )
    {
        int nWidthFactor = pSalLayout->GetUnitsPerPixel();
        bRet = pSalLayout->GetOutline( *mpGraphics, rVector );
        pSalLayout->Release();

        if( bRet )
        {
            PolyPolyVector::iterator aIt;

            if( nXOffset | mnTextOffX | mnTextOffY )
            {
                Point aRotatedOfs( mnTextOffX*nWidthFactor, mnTextOffY*nWidthFactor );
                aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
                for( aIt = rVector.begin(); aIt != rVector.end(); ++aIt )
                    aIt->Move( aRotatedOfs.X(), aRotatedOfs.Y() );
            }

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                for( aIt = rVector.begin(); aIt != rVector.end(); ++aIt )
                    aIt->Scale( fFactor, fFactor );
            }
        }
    }

    if( bOldMap )
    {
        // restore original font size and map mode
        const_cast<OutputDevice&>(*this).mbMap = bOldMap;
        const_cast<OutputDevice&>(*this).mbNewFont = TRUE;
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) )
        return bRet;

    // fall back to bitmap conversion ------------------------------------------

    // Here, we can savely assume that the mapping between characters and glyphs
    // is one-to-one.

    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    pSalLayout = ImplLayout(rStr, nIndex, nLen, Point( 0,0 ), nTWidth, pDXArray );
    if (pSalLayout == 0)
        return false;
    long nOrgWidth = pSalLayout->GetTextWidth();
    long nOrgHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent
        + mnEmphasisDescent;
    pSalLayout->Release();

    VirtualDevice aVDev(1);

    Font aFont(GetFont());
    aFont.SetShadow(false);
    aFont.SetOutline(false);
    aFont.SetRelief(RELIEF_NONE);
    aFont.SetOrientation(0);
    if( bOptimize )
    {
        aFont.SetSize( Size( 0, GLYPH_FONT_HEIGHT ) );
        aVDev.SetMapMode( MAP_PIXEL );
    }
    aVDev.SetFont( aFont );
    aVDev.SetTextAlign( ALIGN_TOP );
    aVDev.SetTextColor( Color(COL_BLACK) );
    aVDev.SetTextFillColor();

    pSalLayout = aVDev.ImplLayout( rStr, nIndex, nLen, Point( 0,0 ), nTWidth, pDXArray );
    if (pSalLayout == 0)
        return false;
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = aVDev.mpFontEntry->mnLineHeight + aVDev.mnEmphasisAscent
        + aVDev.mnEmphasisDescent;
    pSalLayout->Release();

    if( !nWidth || !nHeight )
        return TRUE;
    double fScaleX = static_cast< double >(nOrgWidth) / nWidth;
    double fScaleY = static_cast< double >(nOrgHeight) / nHeight;

    // calculate offset when nBase!=nIndex
    // TODO: fix offset calculation for Bidi case
    nXOffset = 0;
    if( nBase != nIndex )
    {
        xub_StrLen nStart  = ((nBase < nIndex) ? nBase : nIndex);
        xub_StrLen nLength = ((nBase > nIndex) ? nBase : nIndex) - nStart;
        pSalLayout = aVDev.ImplLayout( rStr, nStart, nLength, Point( 0,0 ), nTWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    bRet = true;
    for (xub_StrLen i = nIndex; i < nIndex + nLen; ++i)
    {
        bool bSuccess = false;

        // draw character into virtual device
        pSalLayout = aVDev.ImplLayout(rStr, i, 1, Point( 0,0 ), nTWidth, pDXArray );
        if (pSalLayout == 0)
            return false;
        long nCharWidth = pSalLayout->GetTextWidth();

        Point aOffset(nCharWidth / 2, 8);
        Size aSize(nCharWidth + 2 * aOffset.X(), nHeight + 2 * aOffset.Y());
        bSuccess = (bool)aVDev.SetOutputSizePixel(aSize);
        if( bSuccess )
        {
            // draw glyph into virtual device
            aVDev.Erase();
            pSalLayout->DrawBase() += aOffset;
            pSalLayout->DrawBase() += Point( aVDev.mnTextOffX, aVDev.mnTextOffY );
            pSalLayout->DrawText( *aVDev.mpGraphics );
            pSalLayout->Release();

            // convert character image into outline
            Bitmap aBmp( aVDev.GetBitmap(Point(0, 0), aSize));

            PolyPolygon aPolyPoly;
            if( !aBmp.Vectorize(aPolyPoly, BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES) )
                bSuccess = false;
            else
            {
                // convert units to logical width
                for (USHORT j = 0; j < aPolyPoly.Count(); ++j)
                {
                    Polygon& rPoly = aPolyPoly[j];
                    for (USHORT k = 0; k < rPoly.GetSize(); ++k)
                    {
                        Point& rPt = rPoly[k];
                        rPt.X() = FRound(ImplDevicePixelToLogicWidth(
                                             nXOffset + rPt.X() - aOffset.X()
                                             - aVDev.mnTextOffX)
                                         * fScaleX);
                        rPt.Y() = FRound(ImplDevicePixelToLogicHeight(
                                             rPt.Y() - aOffset.Y()
                                             - aVDev.mnTextOffY)
                                         * fScaleY);
                    }
                }

                if (GetFont().GetOrientation() != 0)
                    aPolyPoly.Rotate(Point(0, 0),
                                     GetFont().GetOrientation());

                // Ignore "empty" glyphs:
                if (aPolyPoly.Count() > 0)
                    rVector.push_back(aPolyPoly);
            }
        }

        nXOffset += nCharWidth;
        bRet = bRet && bSuccess;
    }

    return bRet;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetFontCharMap( FontCharMap& rFontCharMap ) const
{
    rFontCharMap.ImplSetDefaultRanges();

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return FALSE;
#else
    ImplGetServerGraphics();
#endif

    if( mbNewFont )
        const_cast<OutputDevice&>(*this).ImplNewFont();
    if( mbInitFont )
        const_cast<OutputDevice&>(*this).ImplInitFont();
    if( !mpFontEntry )
        return FALSE;

    const ImplFontData* pFontData = mpFontEntry->maFontSelData.mpFontData;

    // a little font charmap cache helps considerably
    static const int NMAXITEMS = 16;
    static int nUsedItems = 0, nCurItem = 0;

    struct CharMapCacheItem { const ImplFontData* mpFontData; FontCharMap maCharMap; };
    static CharMapCacheItem aCache[ NMAXITEMS ];

    ULONG nPairs = 0;
    sal_UCS4* pPairs = NULL;

    int i;
    for( i = nUsedItems; --i >= 0; )
        if( pFontData == aCache[i].mpFontData )
            break;
    if( i >= 0 )    // found in cache
    {
        rFontCharMap = aCache[i].maCharMap;
    }
    else            // need to cache
    {
        // get the data
        nPairs = mpGraphics->GetFontCodeRanges( NULL );

        if( nPairs > 0 )
        {
            pPairs = new sal_UCS4[ 2 * nPairs ];
            mpGraphics->GetFontCodeRanges( pPairs );
            rFontCharMap.ImplSetRanges( nPairs, pPairs );
        }

        // manage cache round-robin and insert data
        CharMapCacheItem& rItem = aCache[ nCurItem ];
        rItem.mpFontData = pFontData;
        rItem.maCharMap = rFontCharMap;

        if( ++nCurItem >= NMAXITEMS )
            nCurItem = 0;

        if( ++nUsedItems >= NMAXITEMS )
            nUsedItems = NMAXITEMS;
    }

    if( rFontCharMap.IsDefaultMap() )
        return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::HasGlyphs( const Font& rTempFont, const String& rStr,
    xub_StrLen nIndex, xub_StrLen nLen ) const
{
    if( nIndex >= rStr.Len() )
        return nIndex;
    xub_StrLen nEnd = nIndex + nLen;
    if( (ULONG)nIndex+nLen > rStr.Len() )
        nEnd = rStr.Len();

    DBG_ASSERT( nIndex < nEnd, "StartPos >= EndPos?" );
    DBG_ASSERT( nEnd <= rStr.Len(), "String too short" );

    // to get the map temporarily set font
    const Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMap aFontCharMap;
    BOOL bRet = GetFontCharMap( aFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( bRet == FALSE )
        return nIndex;

    const sal_Unicode* pStr = rStr.GetBuffer();
    for( pStr += nIndex; nIndex < nEnd; ++pStr, ++nIndex )
        if( ! aFontCharMap.HasChar( *pStr ) )
            break;

    return nIndex;
}

// -----------------------------------------------------------------------

