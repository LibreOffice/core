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

#include <config_folders.h>

#include <comphelper/lok.hxx>
#include <i18nutil/unicode.hxx>
#include <tools/stream.hxx>
#include <vcl/builder.hxx>
#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <unotools/fontoptions.hxx>
#include <unotools/localedatawrapper.hxx>

#include <svtools/borderline.hxx>
#include <svtools/sampletext.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>
#include <svtools/valueset.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <editeng/borderline.hxx>

#include <rtl/bootstrap.hxx>

#include <boost/property_tree/ptree.hpp>

#include <borderline.hrc>

#include <stdio.h>

#define IMGOUTERTEXTSPACE 5
#define EXTRAFONTSIZE 5
#define GAPTOEXTRAPREVIEW 10
#define MINGAPWIDTH 2

#define FONTNAMEBOXMRUENTRIESFILE "/user/config/fontnameboxmruentries"


BorderWidthImpl::BorderWidthImpl( BorderWidthImplFlags nFlags, double nRate1, double nRate2, double nRateGap ):
    m_nFlags( nFlags ),
    m_nRate1( nRate1 ),
    m_nRate2( nRate2 ),
    m_nRateGap( nRateGap )
{
}

bool BorderWidthImpl::operator== ( const BorderWidthImpl& r ) const
{
    return ( m_nFlags == r.m_nFlags ) &&
           ( m_nRate1 == r.m_nRate1 ) &&
           ( m_nRate2 == r.m_nRate2 ) &&
           ( m_nRateGap == r.m_nRateGap );
}

long BorderWidthImpl::GetLine1( long nWidth ) const
{
    long result = static_cast<long>(m_nRate1);
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_LINE1 )
    {
        long const nConstant2 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE2) ? 0 : m_nRate2;
        long const nConstantD = (m_nFlags & BorderWidthImplFlags::CHANGE_DIST ) ? 0 : m_nRateGap;
        result = std::max<long>(0,
                    static_cast<long>((m_nRate1 * nWidth) + 0.5)
                        - (nConstant2 + nConstantD));
        if (result == 0 && m_nRate1 > 0.0 && nWidth > 0)
        {   // fdo#51777: hack to essentially treat 1 twip DOUBLE border
            result = 1;  // as 1 twip SINGLE border
        }
    }
    return result;
}

long BorderWidthImpl::GetLine2( long nWidth ) const
{
    long result = static_cast<long>(m_nRate2);
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_LINE2)
    {
        long const nConstant1 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstantD = (m_nFlags & BorderWidthImplFlags::CHANGE_DIST ) ? 0 : m_nRateGap;
        result = std::max<long>(0,
                    static_cast<long>((m_nRate2 * nWidth) + 0.5)
                        - (nConstant1 + nConstantD));
    }
    return result;
}

long BorderWidthImpl::GetGap( long nWidth ) const
{
    long result = static_cast<long>(m_nRateGap);
    if ( m_nFlags & BorderWidthImplFlags::CHANGE_DIST )
    {
        long const nConstant1 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE1) ? 0 : m_nRate1;
        long const nConstant2 = (m_nFlags & BorderWidthImplFlags::CHANGE_LINE2) ? 0 : m_nRate2;
        result = std::max<long>(0,
                    static_cast<long>((m_nRateGap * nWidth) + 0.5)
                        - (nConstant1 + nConstant2));
    }

    // Avoid having too small distances (less than 0.1pt)
    if ( result < MINGAPWIDTH && m_nRate1 > 0 && m_nRate2 > 0 )
        result = MINGAPWIDTH;

    return result;
}

static double lcl_getGuessedWidth( long nTested, double nRate, bool bChanging )
{
    double nWidth = -1.0;
    if ( bChanging )
        nWidth = double( nTested ) / nRate;
    else
    {
        if ( rtl::math::approxEqual(double( nTested ), nRate) )
            nWidth = nRate;
    }

    return nWidth;
}

long BorderWidthImpl::GuessWidth( long nLine1, long nLine2, long nGap )
{
    std::vector< double > aToCompare;
    bool bInvalid = false;

    bool bLine1Change = bool( m_nFlags & BorderWidthImplFlags::CHANGE_LINE1 );
    double nWidth1 = lcl_getGuessedWidth( nLine1, m_nRate1, bLine1Change );
    if ( bLine1Change )
        aToCompare.push_back( nWidth1 );
    else if (nWidth1 < 0)
        bInvalid = true;

    bool bLine2Change = bool( m_nFlags & BorderWidthImplFlags::CHANGE_LINE2 );
    double nWidth2 = lcl_getGuessedWidth( nLine2, m_nRate2, bLine2Change );
    if ( bLine2Change )
        aToCompare.push_back( nWidth2 );
    else if (nWidth2 < 0)
        bInvalid = true;

    bool bGapChange = bool( m_nFlags & BorderWidthImplFlags::CHANGE_DIST );
    double nWidthGap = lcl_getGuessedWidth( nGap, m_nRateGap, bGapChange );
    if ( bGapChange && nGap >= MINGAPWIDTH )
        aToCompare.push_back( nWidthGap );
    else if ( !bGapChange && nWidthGap < 0 )
        bInvalid = true;

    // non-constant line width factors must sum to 1
    assert((((bLine1Change) ? m_nRate1 : 0) +
            ((bLine2Change) ? m_nRate2 : 0) +
            ((bGapChange) ? m_nRateGap : 0)) - 1.0 < 0.00001 );

    double nWidth = 0.0;
    if ( (!bInvalid) && (!aToCompare.empty()) )
    {
        nWidth = *aToCompare.begin();
        for (auto const& elem : aToCompare)
        {
            bInvalid = ( nWidth != elem );
            if (bInvalid)
                break;
        }
        nWidth = bInvalid ?  0.0 : nLine1 + nLine2 + nGap;
    }

    return nWidth;
}

static void lclDrawPolygon( OutputDevice& rDev, const basegfx::B2DPolygon& rPolygon, long nWidth, SvxBorderLineStyle nDashing )
{
    AntialiasingFlags nOldAA = rDev.GetAntialiasing();
    rDev.SetAntialiasing( nOldAA & ~AntialiasingFlags::EnableB2dDraw );

    long nPix = rDev.PixelToLogic(Size(1, 1)).Width();
    basegfx::B2DPolyPolygon aPolygons = svtools::ApplyLineDashing(rPolygon, nDashing, nPix);

    // Handle problems of width 1px in Pixel mode: 0.5px gives a 1px line
    if (rDev.GetMapMode().GetMapUnit() == MapUnit::MapPixel && nWidth == nPix)
        nWidth = 0;

    for ( sal_uInt32 i = 0; i < aPolygons.count( ); i++ )
    {
        const basegfx::B2DPolygon& aDash = aPolygons.getB2DPolygon( i );
        basegfx::B2DPoint aStart = aDash.getB2DPoint( 0 );
        basegfx::B2DPoint aEnd = aDash.getB2DPoint( aDash.count() - 1 );

        basegfx::B2DVector aVector( aEnd - aStart );
        aVector.normalize( );
        const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

        const basegfx::B2DVector aWidthOffset( double( nWidth ) / 2 * aPerpendicular);
        basegfx::B2DPolygon aDashPolygon;
        aDashPolygon.append( aStart + aWidthOffset );
        aDashPolygon.append( aEnd + aWidthOffset );
        aDashPolygon.append( aEnd - aWidthOffset );
        aDashPolygon.append( aStart - aWidthOffset );
        aDashPolygon.setClosed( true );

        rDev.DrawPolygon( aDashPolygon );
    }

    rDev.SetAntialiasing( nOldAA );
}

namespace svtools {

/**
 * Dashing array must start with a line width and end with a blank width.
 */
static std::vector<double> GetDashing( SvxBorderLineStyle nDashing )
{
    std::vector<double> aPattern;
    switch (nDashing)
    {
        case SvxBorderLineStyle::DOTTED:
            aPattern.push_back( 1.0 ); // line
            aPattern.push_back( 2.0 ); // blank
        break;
        case SvxBorderLineStyle::DASHED:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
        break;
        case SvxBorderLineStyle::FINE_DASHED:
            aPattern.push_back( 6.0 ); // line
            aPattern.push_back( 2.0 ); // blank
        break;
        case SvxBorderLineStyle::DASH_DOT:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
        break;
        case SvxBorderLineStyle::DASH_DOT_DOT:
            aPattern.push_back( 16.0 ); // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
            aPattern.push_back( 5.0 );  // line
            aPattern.push_back( 5.0 );  // blank
        break;
        default:
            ;
    }

    return aPattern;
}

namespace {

class ApplyScale
{
    double mfScale;
public:
    explicit ApplyScale( double fScale ) : mfScale(fScale) {}
    void operator() ( double& rVal )
    {
        rVal *= mfScale;
    }
};

}

std::vector<double> GetLineDashing( SvxBorderLineStyle nDashing, double fScale )
{
    std::vector<double> aPattern = GetDashing(nDashing);
    std::for_each(aPattern.begin(), aPattern.end(), ApplyScale(fScale));
    return aPattern;
}

basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon, SvxBorderLineStyle nDashing, double fScale )
{
    std::vector<double> aPattern = GetDashing(nDashing);
    std::for_each(aPattern.begin(), aPattern.end(), ApplyScale(fScale));

    basegfx::B2DPolyPolygon aPolygons;

    if (aPattern.empty())
        aPolygons.append(rPolygon);
    else
        basegfx::utils::applyLineDashing(rPolygon, aPattern, &aPolygons);

    return aPolygons;
}

void DrawLine( OutputDevice& rDev, const Point& rP1, const Point& rP2,
    sal_uInt32 nWidth, SvxBorderLineStyle nDashing )
{
    DrawLine( rDev, basegfx::B2DPoint( rP1.X(), rP1.Y() ),
            basegfx::B2DPoint( rP2.X(), rP2.Y( ) ), nWidth, nDashing );
}

void DrawLine( OutputDevice& rDev, const basegfx::B2DPoint& rP1, const basegfx::B2DPoint& rP2,
    sal_uInt32 nWidth, SvxBorderLineStyle nDashing )
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append( rP1 );
    aPolygon.append( rP2 );
    lclDrawPolygon( rDev, aPolygon, nWidth, nDashing );
}

}

static Size gUserItemSz;
static int gFontNameBoxes;
static size_t gPreviewsPerDevice;
static std::vector<VclPtr<VirtualDevice>> gFontPreviewVirDevs;
static std::vector<OUString> gRenderedFontNames;

FontNameBox::FontNameBox(std::unique_ptr<weld::ComboBox> p)
    : m_xComboBox(std::move(p))
    , mnPreviewProgress(0)
    , mbWYSIWYG(false)
    , maUpdateIdle("FontNameBox Preview Update")
{
    ++gFontNameBoxes;
    InitFontMRUEntriesFile();

    maUpdateIdle.SetPriority(TaskPriority::LOWEST);
    maUpdateIdle.SetInvokeHandler(LINK(this, FontNameBox, UpdateHdl));
}

FontNameBox::~FontNameBox()
{
    if (mpFontList)
    {
        SaveMRUEntries (maFontMRUEntriesFile);
        ImplDestroyFontList();
    }
    --gFontNameBoxes;
    if (!gFontNameBoxes)
    {
        gFontPreviewVirDevs.clear();
        gRenderedFontNames.clear();
    }
}

void FontNameBox::SaveMRUEntries(const OUString& aFontMRUEntriesFile) const
{
    OString aEntries(OUStringToOString(m_xComboBox->get_mru_entries(),
        RTL_TEXTENCODING_UTF8));

    if (aEntries.isEmpty() || aFontMRUEntriesFile.isEmpty())
        return;

    SvFileStream aStream;
    aStream.Open( aFontMRUEntriesFile, StreamMode::WRITE | StreamMode::TRUNC );
    if( ! (aStream.IsOpen() && aStream.IsWritable()) )
    {
        SAL_INFO("svtools.control", "FontNameBox::SaveMRUEntries: opening mru entries file " << aFontMRUEntriesFile << " failed");
        return;
    }

    aStream.SetLineDelimiter( LINEEND_LF );
    aStream.WriteLine( aEntries );
    aStream.WriteLine( OString() );
}

void FontNameBox::LoadMRUEntries( const OUString& aFontMRUEntriesFile )
{
    if (aFontMRUEntriesFile.isEmpty())
        return;

    SvtFontOptions aFontOpt;
    if (!aFontOpt.IsFontHistoryEnabled())
        return;

    SvFileStream aStream( aFontMRUEntriesFile, StreamMode::READ );
    if( ! aStream.IsOpen() )
    {
        SAL_INFO("svtools.control", "FontNameBox::LoadMRUEntries: opening mru entries file " << aFontMRUEntriesFile << " failed");
        return;
    }

    OString aLine;
    aStream.ReadLine( aLine );
    OUString aEntries = OStringToOUString(aLine,
        RTL_TEXTENCODING_UTF8);
    m_xComboBox->set_mru_entries(aEntries);
}

void FontNameBox::InitFontMRUEntriesFile()
{
    OUString sUserConfigDir("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}");
    rtl::Bootstrap::expandMacros(sUserConfigDir);

    maFontMRUEntriesFile = sUserConfigDir;
    if( !maFontMRUEntriesFile.isEmpty() )
    {
        maFontMRUEntriesFile += FONTNAMEBOXMRUENTRIESFILE;
    }
}

void FontNameBox::ImplDestroyFontList()
{
    mpFontList.reset();
    mnPreviewProgress = 0;
    maUpdateIdle.Stop();
}

void FontNameBox::Fill( const FontList* pList )
{
    // store old text and clear box
    OUString aOldText = m_xComboBox->get_active_text();
    OUString rEntries = m_xComboBox->get_mru_entries();
    bool bLoadFromFile = rEntries.isEmpty();
    m_xComboBox->freeze();
    m_xComboBox->clear();

    ImplDestroyFontList();
    mpFontList.reset(new ImplFontList);

    // insert fonts
    size_t nFontCount = pList->GetFontNameCount();
    for (size_t i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = pList->GetFontName(i);
        m_xComboBox->append(OUString::number(i), rFontMetric.GetFamilyName());
        mpFontList->push_back(rFontMetric);
    }

    if (bLoadFromFile)
        LoadMRUEntries(maFontMRUEntriesFile);
    else
        m_xComboBox->set_mru_entries(rEntries);

    m_xComboBox->thaw();

    if (mbWYSIWYG && nFontCount)
    {
        assert(mnPreviewProgress == 0 && "ImplDestroyFontList wasn't called");
        maUpdateIdle.Start();
    }

    // restore text
    if (!aOldText.isEmpty())
        set_active_or_entry_text(aOldText);
}

void FontNameBox::EnableWYSIWYG()
{
    if (mbWYSIWYG || comphelper::LibreOfficeKit::isActive())
        return;
    mbWYSIWYG = true;

    static bool bGlobalsInited;
    if (!bGlobalsInited)
    {
        gUserItemSz = Size(m_xComboBox->get_approximate_digit_width() * 52, m_xComboBox->get_text_height());
        gUserItemSz.setHeight(gUserItemSz.Height() * 16);
        gUserItemSz.setHeight(gUserItemSz.Height() / 10);

        size_t nMaxDeviceHeight = SAL_MAX_INT16 / 2; // see limitXCreatePixmap
        gPreviewsPerDevice = nMaxDeviceHeight / gUserItemSz.Height();

        bGlobalsInited = true;
    }

    m_xComboBox->connect_custom_get_size(LINK(this, FontNameBox, CustomGetSizeHdl));
    m_xComboBox->connect_custom_render(LINK(this, FontNameBox, CustomRenderHdl));
    m_xComboBox->set_custom_renderer();

    mbWYSIWYG = true;
}

IMPL_STATIC_LINK_NOARG(FontNameBox, CustomGetSizeHdl, OutputDevice&, Size)
{
    return gUserItemSz;
}

namespace
{
    long shrinkFontToFit(OUString const &rSampleText, long nH, vcl::Font &rFont, OutputDevice &rDevice, tools::Rectangle &rTextRect)
    {
        long nWidth = 0;

        Size aSize( rFont.GetFontSize() );

        //Make sure it fits in the available height
        while (aSize.Height() > 0)
        {
            if (!rDevice.GetTextBoundRect(rTextRect, rSampleText))
                break;
            if (rTextRect.GetHeight() <= nH)
            {
                nWidth = rTextRect.GetWidth();
                break;
            }

            aSize.AdjustHeight( -(EXTRAFONTSIZE) );
            rFont.SetFontSize(aSize);
            rDevice.SetFont(rFont);
        }

        return nWidth;
    }
}

IMPL_LINK_NOARG(FontNameBox, UpdateHdl, Timer*, void)
{
    CachePreview(mnPreviewProgress++, nullptr);
    // tdf#132536 limit to ~25 pre-rendered for now. The font caches look
    // b0rked, the massive charmaps are ~never swapped out, and don't count
    // towards the size of a font in the font cache and if the freetype font
    // cache size is set experimentally very low then we crash, so there's an
    // awful lot to consider there.
    if (mnPreviewProgress < std::min<size_t>(25, mpFontList->size()))
        maUpdateIdle.Start();
}

static void DrawPreview(const FontMetric& rFontMetric, const Point& rTopLeft, OutputDevice& rDevice, bool bSelected)
{
    rDevice.Push(PushFlags::TEXTCOLOR);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rDevice.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rDevice.SetTextColor(rStyleSettings.GetDialogTextColor());

    long nX = rTopLeft.X();
    long nH = gUserItemSz.Height();

    nX += IMGOUTERTEXTSPACE;

    const bool bSymbolFont = isSymbolFont(rFontMetric);

    vcl::Font aOldFont(rDevice.GetFont());
    Size aSize( aOldFont.GetFontSize() );
    aSize.AdjustHeight(EXTRAFONTSIZE );
    vcl::Font aFont( rFontMetric );
    aFont.SetFontSize( aSize );
    rDevice.SetFont(aFont);

    bool bUsingCorrectFont = true;
    tools::Rectangle aTextRect;

    // Preview the font name
    const OUString& sFontName = rFontMetric.GetFamilyName();

    //If it shouldn't or can't draw its own name because it doesn't have the glyphs
    if (!canRenderNameOfSelectedFont(rDevice))
        bUsingCorrectFont = false;
    else
    {
        //Make sure it fits in the available height, shrinking the font if necessary
        bUsingCorrectFont = shrinkFontToFit(sFontName, nH, aFont, rDevice, aTextRect) != 0;
    }

    if (!bUsingCorrectFont)
    {
        rDevice.SetFont(aOldFont);
        rDevice.GetTextBoundRect(aTextRect, sFontName);
    }

    long nTextHeight = aTextRect.GetHeight();
    long nDesiredGap = (nH-nTextHeight)/2;
    long nVertAdjust = nDesiredGap - aTextRect.Top();
    Point aPos( nX, rTopLeft.Y() + nVertAdjust );
    rDevice.DrawText(aPos, sFontName);
    long nTextX = aPos.X() + aTextRect.GetWidth() + GAPTOEXTRAPREVIEW;

    if (!bUsingCorrectFont)
        rDevice.SetFont(aFont);

    OUString sSampleText;

    if (!bSymbolFont)
    {
        const bool bNameBeginsWithLatinText = rFontMetric.GetFamilyName()[0] <= 'z';

        if (bNameBeginsWithLatinText || !bUsingCorrectFont)
            sSampleText = makeShortRepresentativeTextForSelectedFont(rDevice);
    }

    //If we're not a symbol font, but could neither render our own name and
    //we can't determine what script it would like to render, then try a
    //few well known scripts
    if (sSampleText.isEmpty() && !bUsingCorrectFont)
    {
        static const UScriptCode aScripts[] =
        {
            USCRIPT_ARABIC,
            USCRIPT_HEBREW,

            USCRIPT_BENGALI,
            USCRIPT_GURMUKHI,
            USCRIPT_GUJARATI,
            USCRIPT_ORIYA,
            USCRIPT_TAMIL,
            USCRIPT_TELUGU,
            USCRIPT_KANNADA,
            USCRIPT_MALAYALAM,
            USCRIPT_SINHALA,
            USCRIPT_DEVANAGARI,

            USCRIPT_THAI,
            USCRIPT_LAO,
            USCRIPT_GEORGIAN,
            USCRIPT_TIBETAN,
            USCRIPT_SYRIAC,
            USCRIPT_MYANMAR,
            USCRIPT_ETHIOPIC,
            USCRIPT_KHMER,
            USCRIPT_MONGOLIAN,

            USCRIPT_KOREAN,
            USCRIPT_JAPANESE,
            USCRIPT_HAN,
            USCRIPT_SIMPLIFIED_HAN,
            USCRIPT_TRADITIONAL_HAN,

            USCRIPT_GREEK
        };

        for (const UScriptCode& rScript : aScripts)
        {
            OUString sText = makeShortRepresentativeTextForScript(rScript);
            if (!sText.isEmpty())
            {
                bool bHasSampleTextGlyphs = (-1 == rDevice.HasGlyphs(aFont, sText));
                if (bHasSampleTextGlyphs)
                {
                    sSampleText = sText;
                    break;
                }
            }
        }

        static const UScriptCode aMinimalScripts[] =
        {
            USCRIPT_HEBREW, //e.g. biblical hebrew
            USCRIPT_GREEK
        };

        for (const UScriptCode& rMinimalScript : aMinimalScripts)
        {
            OUString sText = makeShortMinimalTextForScript(rMinimalScript);
            if (!sText.isEmpty())
            {
                bool bHasSampleTextGlyphs = (-1 == rDevice.HasGlyphs(aFont, sText));
                if (bHasSampleTextGlyphs)
                {
                    sSampleText = sText;
                    break;
                }
            }
        }
    }

    //If we're a symbol font, or for some reason the font still couldn't
    //render something representative of what it would like to render then
    //make up some semi-random text that it *can* display
    if (bSymbolFont || (!bUsingCorrectFont && sSampleText.isEmpty()))
        sSampleText = makeShortRepresentativeSymbolTextForSelectedFont(rDevice);

    if (!sSampleText.isEmpty())
    {
        const Size &rItemSize = gUserItemSz;

        //leave a little border at the edge
        long nSpace = rItemSize.Width() - nTextX - IMGOUTERTEXTSPACE;
        if (nSpace >= 0)
        {
            //Make sure it fits in the available height, and get how wide that would be
            long nWidth = shrinkFontToFit(sSampleText, nH, aFont, rDevice, aTextRect);
            //Chop letters off until it fits in the available width
            while (nWidth > nSpace || nWidth > gUserItemSz.Width())
            {
                sSampleText = sSampleText.copy(0, sSampleText.getLength()-1);
                nWidth = rDevice.GetTextBoundRect(aTextRect, sSampleText) ?
                         aTextRect.GetWidth() : 0;
            }

            //center the text on the line
            if (!sSampleText.isEmpty() && nWidth)
            {
                nTextHeight = aTextRect.GetHeight();
                nDesiredGap = (nH-nTextHeight)/2;
                nVertAdjust = nDesiredGap - aTextRect.Top();
                aPos = Point(nTextX + nSpace - nWidth, rTopLeft.Y() + nVertAdjust);
                rDevice.DrawText(aPos, sSampleText);
            }
        }
    }

    rDevice.SetFont(aOldFont);
    rDevice.Pop();
}

OutputDevice& FontNameBox::CachePreview(size_t nIndex, Point* pTopLeft)
{
    SolarMutexGuard aGuard;
    const FontMetric& rFontMetric = (*mpFontList)[nIndex];
    const OUString& rFontName = rFontMetric.GetFamilyName();

    size_t nPreviewIndex;
    auto xFind = std::find(gRenderedFontNames.begin(), gRenderedFontNames.end(), rFontName);
    bool bPreviewAvailable = xFind != gRenderedFontNames.end();
    if (!bPreviewAvailable)
    {
        nPreviewIndex = gRenderedFontNames.size();
        gRenderedFontNames.push_back(rFontName);
    }
    else
        nPreviewIndex = std::distance(gRenderedFontNames.begin(), xFind);

    size_t nPage = nPreviewIndex / gPreviewsPerDevice;
    size_t nIndexInPage = nPreviewIndex - (nPage * gPreviewsPerDevice);

    Point aTopLeft(0, gUserItemSz.Height() * nIndexInPage);

    if (!bPreviewAvailable)
    {
        if (nPage >= gFontPreviewVirDevs.size())
        {
            gFontPreviewVirDevs.emplace_back(m_xComboBox->create_render_virtual_device());
            VirtualDevice& rDevice = *gFontPreviewVirDevs.back();
            rDevice.SetOutputSizePixel(Size(gUserItemSz.Width(), gUserItemSz.Height() * gPreviewsPerDevice));
            if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
                pDefaultDevice->SetPointFont(rDevice, m_xComboBox->get_font());
            assert(gFontPreviewVirDevs.size() == nPage + 1);
        }

        DrawPreview(rFontMetric, aTopLeft, *gFontPreviewVirDevs.back(), false);
    }

    if (pTopLeft)
        *pTopLeft = aTopLeft;

    return *gFontPreviewVirDevs[nPage];
}

IMPL_LINK(FontNameBox, CustomRenderHdl, weld::ComboBox::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    sal_uInt32 nIndex = rId.toUInt32();

    Point aDestPoint(rRect.TopLeft());
    auto nMargin = (rRect.GetHeight() - gUserItemSz.Height()) / 2;
    aDestPoint.AdjustY(nMargin);

    if (bSelected)
    {
        const FontMetric& rFontMetric = (*mpFontList)[nIndex];
        DrawPreview(rFontMetric, aDestPoint, rRenderContext, true);
    }
    else
    {
        // use cache of unselected entries
        Point aTopLeft;
        OutputDevice& rDevice = CachePreview(nIndex, &aTopLeft);

        rRenderContext.DrawOutDev(aDestPoint, gUserItemSz,
                                  aTopLeft, gUserItemSz,
                                  rDevice);
    }
}

void FontNameBox::set_active_or_entry_text(const OUString& rText)
{
    const int nFound = m_xComboBox->find_text(rText);
    if (nFound != -1)
        m_xComboBox->set_active(nFound);
    else
        m_xComboBox->set_entry_text(rText);
}

FontStyleBox::FontStyleBox(std::unique_ptr<weld::ComboBox> p)
    : m_xComboBox(std::move(p))
{
    //Use the standard texts to get an optimal size and stick to that size.
    //That should stop the character dialog dancing around.
    auto nMaxLen = m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_LIGHT)).Width();
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_LIGHT_ITALIC)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_NORMAL)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_NORMAL_ITALIC)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_BOLD)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_BOLD_ITALIC)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_BLACK)).Width());
    nMaxLen = std::max(nMaxLen, m_xComboBox->get_pixel_size(SvtResId(STR_SVT_STYLE_BLACK_ITALIC)).Width());
    m_xComboBox->set_entry_width_chars(std::ceil(nMaxLen / m_xComboBox->get_approximate_digit_width()));
}

void FontStyleBox::Fill( const OUString& rName, const FontList* pList )
{
    m_xComboBox->freeze();
    OUString aOldText = m_xComboBox->get_active_text();
    int nPos = m_xComboBox->get_active();
    m_xComboBox->clear();

    // does a font with this name already exist?
    sal_Handle hFontMetric = pList->GetFirstFontMetric( rName );
    if ( hFontMetric )
    {
        OUString aStyleText;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        bool        bNormal = false;
        bool        bItalic = false;
        bool        bBold = false;
        bool        bBoldItalic = false;
        bool        bInsert = false;
        FontMetric    aFontMetric;
        while ( hFontMetric )
        {
            aFontMetric = FontList::GetFontMetric( hFontMetric );

            FontWeight  eWeight = aFontMetric.GetWeight();
            FontItalic  eItalic = aFontMetric.GetItalic();
            FontWidth   eWidth = aFontMetric.GetWidthType();
            // Only if the attributes are different, we insert the
            // Font to avoid double Entries in different languages
            if ( (eWeight != eLastWeight) || (eItalic != eLastItalic) ||
                 (eWidth != eLastWidth) )
            {
                if ( bInsert )
                    m_xComboBox->append_text(aStyleText);

                if ( eWeight <= WEIGHT_NORMAL )
                {
                    if ( eItalic != ITALIC_NONE )
                        bItalic = true;
                    else
                        bNormal = true;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = true;
                    else
                        bBold = true;
                }

                // For wrong StyleNames we replace this with the correct once
                aStyleText = pList->GetStyleName( aFontMetric );
                bInsert = m_xComboBox->find_text(aStyleText) == -1;
                if ( !bInsert )
                {
                    aStyleText = pList->GetStyleName( eWeight, eItalic );
                    bInsert = m_xComboBox->find_text(aStyleText) == -1;
                }

                eLastWeight = eWeight;
                eLastItalic = eItalic;
                eLastWidth = eWidth;
            }
            else
            {
                if ( bInsert )
                {
                    // If we have two names for the same attributes
                    // we prefer the translated standard names
                    const OUString& rAttrStyleText = pList->GetStyleName( eWeight, eItalic );
                    if (rAttrStyleText != aStyleText)
                    {
                        OUString aTempStyleText = pList->GetStyleName( aFontMetric );
                        if (rAttrStyleText == aTempStyleText)
                            aStyleText = rAttrStyleText;
                        bInsert = m_xComboBox->find_text(aStyleText) == -1;
                    }
                }
            }

            if ( !bItalic && (aStyleText == pList->GetItalicStr()) )
                bItalic = true;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = true;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = true;

            hFontMetric = FontList::GetNextFontMetric( hFontMetric );
        }

        if ( bInsert )
            m_xComboBox->append_text(aStyleText);

        // certain style as copy
        if ( bNormal )
        {
            if ( !bItalic )
                m_xComboBox->append_text(pList->GetItalicStr());
            if ( !bBold )
                m_xComboBox->append_text(pList->GetBoldStr());
        }
        if ( !bBoldItalic )
        {
            if ( bNormal || bItalic || bBold )
                m_xComboBox->append_text(pList->GetBoldItalicStr());
        }
        if (!aOldText.isEmpty())
        {
            int nFound = m_xComboBox->find_text(aOldText);
            if (nFound != -1)
                m_xComboBox->set_active(nFound);
            else
            {
                if (nPos >= m_xComboBox->get_count())
                    m_xComboBox->set_active(0);
                else
                    m_xComboBox->set_active(nPos);
            }
        }
    }
    else
    {
        // insert standard styles if no font
        m_xComboBox->append_text(pList->GetNormalStr());
        m_xComboBox->append_text(pList->GetItalicStr());
        m_xComboBox->append_text(pList->GetBoldStr());
        m_xComboBox->append_text(pList->GetBoldItalicStr());
        if (!aOldText.isEmpty())
        {
            if (nPos >= m_xComboBox->get_count())
                m_xComboBox->set_active(0);
            else
                m_xComboBox->set_active(nPos);
        }
    }
    m_xComboBox->thaw();
}

FontSizeBox::FontSizeBox(std::unique_ptr<weld::ComboBox> p)
    : pFontList(nullptr)
    , nSavedValue(0)
    , nMin(20)
    , nMax(9999)
    , eUnit(FieldUnit::POINT)
    , nDecimalDigits(1)
    , nRelMin(0)
    , nRelMax(0)
    , nRelStep(0)
    , nPtRelMin(0)
    , nPtRelMax(0)
    , nPtRelStep(0)
    , bRelativeMode(false)
    , bRelative(false)
    , bPtRelative(false)
    , bStdSize(false)
    , m_xComboBox(std::move(p))
{
    m_xComboBox->set_entry_width_chars(std::ceil(m_xComboBox->get_pixel_size(format_number(105)).Width() /
                                                 m_xComboBox->get_approximate_digit_width()));
    m_xComboBox->connect_focus_out(LINK(this, FontSizeBox, ReformatHdl));
    m_xComboBox->connect_changed(LINK(this, FontSizeBox, ModifyHdl));
}

void FontSizeBox::set_active_or_entry_text(const OUString& rText)
{
    const int nFound = m_xComboBox->find_text(rText);
    if (nFound != -1)
        m_xComboBox->set_active(nFound);
    else
        m_xComboBox->set_entry_text(rText);
}

IMPL_LINK(FontSizeBox, ReformatHdl, weld::Widget&, rWidget, void)
{
    FontSizeNames aFontSizeNames(Application::GetSettings().GetUILanguageTag().getLanguageType());
    if (!bRelativeMode || !aFontSizeNames.IsEmpty())
    {
        if (aFontSizeNames.Name2Size(m_xComboBox->get_active_text()) != 0)
            return;
    }

    set_value(get_value());

    m_aFocusOutHdl.Call(rWidget);
}

IMPL_LINK(FontSizeBox, ModifyHdl, weld::ComboBox&, rBox, void)
{
    if (bRelativeMode)
    {
        OUString aStr = comphelper::string::stripStart(rBox.get_active_text(), ' ');

        bool bNewMode = bRelative;
        bool bOldPtRelMode = bPtRelative;

        if ( bRelative )
        {
            bPtRelative = false;
            const sal_Unicode* pStr = aStr.getStr();
            while ( *pStr )
            {
                if ( ((*pStr < '0') || (*pStr > '9')) && (*pStr != '%') && !unicode::isSpace(*pStr) )
                {
                    if ( ('-' == *pStr || '+' == *pStr) && !bPtRelative )
                        bPtRelative = true;
                    else if ( bPtRelative && 'p' == *pStr && 't' == *++pStr )
                        ;
                    else
                    {
                        bNewMode = false;
                        break;
                    }
                }
                pStr++;
            }
        }
        else if (!aStr.isEmpty())
        {
            if ( -1 != aStr.indexOf('%') )
            {
                bNewMode = true;
                bPtRelative = false;
            }

            if ( '-' == aStr[0] || '+' == aStr[0] )
            {
                bNewMode = true;
                bPtRelative = true;
            }
        }

        if ( bNewMode != bRelative || bPtRelative != bOldPtRelMode )
            SetRelative( bNewMode );
    }
    m_aChangeHdl.Call(rBox);
}

void FontSizeBox::Fill( const FontMetric* pFontMetric, const FontList* pList )
{
    // remember for relative mode
    pFontList = pList;

    // no font sizes need to be set for relative mode
    if ( bRelative )
        return;

    // query font sizes
    const sal_IntPtr* pTempAry;
    const sal_IntPtr* pAry = nullptr;

    if( pFontMetric )
    {
        aFontMetric = *pFontMetric;
        pAry = pList->GetSizeAry( *pFontMetric );
    }
    else
    {
        pAry = FontList::GetStdSizeAry();
    }

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( Application::GetSettings().GetUILanguageTag().getLanguageType() );
    if ( pAry == FontList::GetStdSizeAry() )
    {
        // for standard sizes we don't need to bother
        if (bStdSize && m_xComboBox->get_count() && aFontSizeNames.IsEmpty())
            return;
        bStdSize = true;
    }
    else
        bStdSize = false;

    int nSelectionStart, nSelectionEnd;
    m_xComboBox->get_entry_selection_bounds(nSelectionStart, nSelectionEnd);
    OUString aStr = m_xComboBox->get_active_text();

    m_xComboBox->freeze();
    m_xComboBox->clear();
    int nPos = 0;

    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == FontList::GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            sal_uLong nCount = aFontSizeNames.Count();
            for( sal_uLong i = 0; i < nCount; i++ )
            {
                OUString    aSizeName = aFontSizeNames.GetIndexName( i );
                sal_IntPtr  nSize = aFontSizeNames.GetIndexSize( i );
                OUString sId(OUString::number(-nSize)); // mark as special
                m_xComboBox->insert(nPos, aSizeName, &sId, nullptr, nullptr);
                nPos++;
            }
        }
        else
        {
            // for fixed size fonts only selectable font size names
            pTempAry = pAry;
            while ( *pTempAry )
            {
                OUString aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                if ( !aSizeName.isEmpty() )
                {
                    OUString sId(OUString::number(-(*pTempAry))); // mark as special
                    m_xComboBox->insert(nPos, aSizeName, &sId, nullptr, nullptr);
                    nPos++;
                }
                pTempAry++;
            }
        }
    }

    // then insert numerical font size values
    pTempAry = pAry;
    while (*pTempAry)
    {
        InsertValue(*pTempAry);
        ++pTempAry;
    }

    set_active_or_entry_text(aStr);
    m_xComboBox->select_entry_region(nSelectionStart, nSelectionEnd);
    m_xComboBox->thaw();
}

void FontSizeBox::EnableRelativeMode( sal_uInt16 nNewMin, sal_uInt16 nNewMax, sal_uInt16 nStep )
{
    bRelativeMode = true;
    nRelMin       = nNewMin;
    nRelMax       = nNewMax;
    nRelStep      = nStep;
    SetUnit(FieldUnit::POINT);
}

void FontSizeBox::EnablePtRelativeMode( short nNewMin, short nNewMax, short nStep )
{
    bRelativeMode = true;
    nPtRelMin     = nNewMin;
    nPtRelMax     = nNewMax;
    nPtRelStep    = nStep;
    SetUnit(FieldUnit::POINT);
}

void FontSizeBox::InsertValue(int i)
{
    OUString sNumber(OUString::number(i));
    m_xComboBox->append(sNumber, format_number(i));
}

void FontSizeBox::SetRelative( bool bNewRelative )
{
    if ( !bRelativeMode )
        return;

    int nSelectionStart, nSelectionEnd;
    m_xComboBox->get_entry_selection_bounds(nSelectionStart, nSelectionEnd);
    OUString aStr = comphelper::string::stripStart(m_xComboBox->get_active_text(), ' ');

    if (bNewRelative)
    {
        bRelative = true;
        bStdSize = false;

        m_xComboBox->clear();

        if (bPtRelative)
        {
            SetDecimalDigits( 1 );
            SetRange(nPtRelMin, nPtRelMax);
            SetUnit(FieldUnit::POINT);

            short i = nPtRelMin, n = 0;
            // JP 30.06.98: more than 100 values are not useful
            while ( i <= nPtRelMax && n++ < 100 )
            {
                InsertValue( i );
                i = i + nPtRelStep;
            }
        }
        else
        {
            SetDecimalDigits(0);
            SetRange(nRelMin, nRelMax);
            SetUnit(FieldUnit::PERCENT);

            sal_uInt16 i = nRelMin;
            while ( i <= nRelMax )
            {
                InsertValue( i );
                i = i + nRelStep;
            }
        }
    }
    else
    {
        if (pFontList)
            m_xComboBox->clear();
        bRelative = bPtRelative = false;
        SetDecimalDigits(1);
        SetRange(20, 9999);
        SetUnit(FieldUnit::POINT);
        if ( pFontList)
            Fill( &aFontMetric, pFontList );
    }

    set_active_or_entry_text(aStr);
    m_xComboBox->select_entry_region(nSelectionStart, nSelectionEnd);
}

OUString FontSizeBox::format_number(int nValue) const
{
    OUString sRet;

    //pawn percent off to icu to decide whether percent is separated from its number for this locale
    if (eUnit == FieldUnit::PERCENT)
    {
        double fValue = nValue;
        fValue /= weld::SpinButton::Power10(nDecimalDigits);
        sRet = unicode::formatPercent(fValue, Application::GetSettings().GetUILanguageTag());
    }
    else
    {
        const SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
        sRet = rLocaleData.getNum(nValue, nDecimalDigits, true, false);
        if (eUnit != FieldUnit::NONE && eUnit != FieldUnit::DEGREE)
            sRet += " ";
        assert(eUnit != FieldUnit::PERCENT);
        sRet += weld::MetricSpinButton::MetricToString(eUnit);
    }

    if (bRelativeMode && bPtRelative && (0 <= nValue) && !sRet.isEmpty())
        sRet = "+" + sRet;

    return sRet;
}

void FontSizeBox::SetValue(int nNewValue, FieldUnit eInUnit)
{
    auto nTempValue = vcl::ConvertValue(nNewValue, 0, GetDecimalDigits(), eInUnit, GetUnit());
    if (nTempValue < nMin)
        nTempValue = nMin;
    else if (nTempValue > nMax)
        nTempValue = nMax;
    if (!bRelative)
    {
        FontSizeNames aFontSizeNames(Application::GetSettings().GetUILanguageTag().getLanguageType());
        // conversion loses precision; however font sizes should
        // never have a problem with that
        OUString aName = aFontSizeNames.Size2Name(nTempValue);
        if (!aName.isEmpty() && m_xComboBox->find_text(aName) != -1)
        {
            m_xComboBox->set_active_text(aName);
            return;
        }
    }
    OUString aResult = format_number(nTempValue);
    set_active_or_entry_text(aResult);
}

void FontSizeBox::set_value(int nNewValue)
{
    SetValue(nNewValue, eUnit);
}

int FontSizeBox::get_value() const
{
    OUString aStr = m_xComboBox->get_active_text();
    if (!bRelative)
    {
        FontSizeNames aFontSizeNames(Application::GetSettings().GetUILanguageTag().getLanguageType());
        auto nValue = aFontSizeNames.Name2Size(aStr);
        if (nValue)
            return vcl::ConvertValue(nValue, 0, GetDecimalDigits(), GetUnit(), GetUnit());
    }

    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    double fResult(0.0);
    (void)vcl::TextToValue(aStr, fResult, 0, GetDecimalDigits(), rLocaleData, GetUnit());
    if (!aStr.isEmpty())
    {
        if (fResult < nMin)
            fResult = nMin;
        else if (fResult > nMax)
            fResult = nMax;
    }
    return fResult;
}

SvxBorderLineStyle SvtLineListBox::GetSelectEntryStyle() const
{
    if (m_xLineSet->IsNoSelection())
        return SvxBorderLineStyle::NONE;
    auto nId = m_xLineSet->GetSelectedItemId();
    return static_cast<SvxBorderLineStyle>(nId - 1);
}

namespace
{
    Size getPreviewSize(const weld::Widget& rControl)
    {
        return Size(rControl.get_approximate_digit_width() * 15, rControl.get_text_height());
    }
}

void SvtLineListBox::ImpGetLine( long nLine1, long nLine2, long nDistance,
                            Color aColor1, Color aColor2, Color aColorDist,
                            SvxBorderLineStyle nStyle, BitmapEx& rBmp )
{
    Size aSize(getPreviewSize(*m_xControl));

    // SourceUnit to Twips
    if ( eSourceUnit == FieldUnit::POINT )
    {
        nLine1      /= 5;
        nLine2      /= 5;
        nDistance   /= 5;
    }

    // Paint the lines
    aSize = aVirDev->PixelToLogic( aSize );
    long nPix = aVirDev->PixelToLogic( Size( 0, 1 ) ).Height();
    sal_uInt32 n1 = nLine1;
    sal_uInt32 n2 = nLine2;
    long nDist  = nDistance;
    n1 += nPix-1;
    n1 -= n1%nPix;
    if ( n2 )
    {
        nDist += nPix-1;
        nDist -= nDist%nPix;
        n2    += nPix-1;
        n2    -= n2%nPix;
    }
    long nVirHeight = n1+nDist+n2;
    if ( nVirHeight > aSize.Height() )
        aSize.setHeight( nVirHeight );
    // negative width should not be drawn
    if ( aSize.Width() <= 0 )
        return;

    Size aVirSize = aVirDev->LogicToPixel( aSize );
    if ( aVirDev->GetOutputSizePixel() != aVirSize )
        aVirDev->SetOutputSizePixel( aVirSize );
    aVirDev->SetFillColor( aColorDist );
    aVirDev->DrawRect( tools::Rectangle( Point(), aSize ) );

    aVirDev->SetFillColor( aColor1 );

    double y1 = double( n1 ) / 2;
    svtools::DrawLine( *aVirDev, basegfx::B2DPoint( 0, y1 ), basegfx::B2DPoint( aSize.Width( ), y1 ), n1, nStyle );

    if ( n2 )
    {
        double y2 =  n1 + nDist + double( n2 ) / 2;
        aVirDev->SetFillColor( aColor2 );
        svtools::DrawLine( *aVirDev, basegfx::B2DPoint( 0, y2 ), basegfx::B2DPoint( aSize.Width(), y2 ), n2, SvxBorderLineStyle::SOLID );
    }
    rBmp = aVirDev->GetBitmapEx( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
}

namespace
{
    OUString GetLineStyleName(SvxBorderLineStyle eStyle)
    {
        OUString sRet;
        for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(RID_SVXSTR_BORDERLINE); ++i)
        {
            if (eStyle == RID_SVXSTR_BORDERLINE[i].second)
            {
                sRet = SvtResId(RID_SVXSTR_BORDERLINE[i].first);
                break;
            }
        }
        return sRet;
    }
}

SvtLineListBox::SvtLineListBox(std::unique_ptr<weld::MenuButton> pControl)
    : m_xControl(std::move(pControl))
    , m_xBuilder(Application::CreateBuilder(m_xControl.get(), "svt/ui/linewindow.ui"))
    , m_xTopLevel(m_xBuilder->weld_widget("line_popup_window"))
    , m_xNoneButton(m_xBuilder->weld_button("none_line_button"))
    , m_xLineSet(new ValueSet(nullptr))
    , m_xLineSetWin(new weld::CustomWeld(*m_xBuilder, "lineset", *m_xLineSet))
    , m_nWidth( 5 )
    , aVirDev(VclPtr<VirtualDevice>::Create())
    , aColor(COL_BLACK)
    , maPaintCol(COL_BLACK)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    m_xLineSet->SetStyle(WinBits(WB_FLATVALUESET | WB_NO_DIRECTSELECT | WB_TABSTOP));
    m_xLineSet->SetItemHeight(rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1);
    m_xLineSet->SetColCount(1);
    m_xLineSet->SetSelectHdl(LINK(this, SvtLineListBox, ValueSelectHdl));

    m_xNoneButton->connect_clicked(LINK(this, SvtLineListBox, NoneHdl));

    m_xTopLevel->connect_focus_in(LINK(this, SvtLineListBox, FocusHdl));
    m_xControl->set_popover(m_xTopLevel.get());
    m_xControl->connect_toggled(LINK(this, SvtLineListBox, ToggleHdl));

    // lock size to these maxes height/width so it doesn't jump around in size
    m_xControl->set_label(GetLineStyleName(SvxBorderLineStyle::NONE));
    Size aNonePrefSize = m_xControl->get_preferred_size();
    m_xControl->set_label("");
    aVirDev->SetOutputSizePixel(getPreviewSize(*m_xControl));
    m_xControl->set_image(aVirDev);
    Size aSolidPrefSize = m_xControl->get_preferred_size();
    m_xControl->set_size_request(std::max(aNonePrefSize.Width(), aSolidPrefSize.Width()),
                                 std::max(aNonePrefSize.Height(), aSolidPrefSize.Height()));

    eSourceUnit = FieldUnit::POINT;

    aVirDev->SetLineColor();
    aVirDev->SetMapMode(MapMode(MapUnit::MapTwip));

    UpdatePaintLineColor();
}

IMPL_LINK_NOARG(SvtLineListBox, FocusHdl, weld::Widget&, void)
{
    if (GetSelectEntryStyle() == SvxBorderLineStyle::NONE)
        m_xNoneButton->grab_focus();
    else
        m_xLineSet->GrabFocus();
}

IMPL_LINK(SvtLineListBox, ToggleHdl, weld::ToggleButton&, rButton, void)
{
    if (rButton.get_active())
        FocusHdl(*m_xTopLevel);
}

IMPL_LINK_NOARG(SvtLineListBox, NoneHdl, weld::Button&, void)
{
    SelectEntry(SvxBorderLineStyle::NONE);
    ValueSelectHdl(nullptr);
}

SvtLineListBox::~SvtLineListBox()
{
}

sal_Int32 SvtLineListBox::GetStylePos( sal_Int32 nListPos )
{
    sal_Int32 nPos = -1;
    --nListPos;

    sal_Int32 n = 0;
    size_t i = 0;
    size_t nCount = m_vLineList.size();
    while ( nPos == -1 && i < nCount )
    {
        if ( nListPos == n )
            nPos = static_cast<sal_Int32>(i);
        n++;
        i++;
    }

    return nPos;
}

void SvtLineListBox::SelectEntry(SvxBorderLineStyle nStyle)
{
    if (nStyle == SvxBorderLineStyle::NONE)
        m_xLineSet->SetNoSelection();
    else
        m_xLineSet->SelectItem(static_cast<sal_Int16>(nStyle) + 1);
    UpdatePreview();
}

void SvtLineListBox::InsertEntry(
    const BorderWidthImpl& rWidthImpl, SvxBorderLineStyle nStyle, long nMinWidth,
    ColorFunc pColor1Fn, ColorFunc pColor2Fn, ColorDistFunc pColorDistFn )
{
    m_vLineList.emplace_back(new ImpLineListData(
        rWidthImpl, nStyle, nMinWidth, pColor1Fn, pColor2Fn, pColorDistFn));
}

void SvtLineListBox::UpdatePaintLineColor()
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Color aNewCol(rSettings.GetWindowColor().IsDark() ? rSettings.GetLabelTextColor() : aColor);

    bool bRet = aNewCol != maPaintCol;

    if( bRet )
        maPaintCol = aNewCol;
}

void SvtLineListBox::UpdateEntries()
{
    UpdatePaintLineColor( );

    SvxBorderLineStyle eSelected = GetSelectEntryStyle();

    // Remove the old entries
    m_xLineSet->Clear();

    // Add the new entries based on the defined width

    sal_uInt16 n = 0;
    sal_uInt16 nCount = m_vLineList.size( );
    while ( n < nCount )
    {
        auto& pData = m_vLineList[ n ];
        BitmapEx aBmp;
        ImpGetLine( pData->GetLine1ForWidth( m_nWidth ),
                pData->GetLine2ForWidth( m_nWidth ),
                pData->GetDistForWidth( m_nWidth ),
                GetColorLine1(m_xLineSet->GetItemCount()),
                GetColorLine2(m_xLineSet->GetItemCount()),
                GetColorDist(m_xLineSet->GetItemCount()),
                pData->GetStyle(), aBmp );
        sal_Int16 nItemId = static_cast<sal_Int16>(pData->GetStyle()) + 1;
        m_xLineSet->InsertItem(nItemId, Image(aBmp), GetLineStyleName(pData->GetStyle()));
        if (pData->GetStyle() == eSelected)
            m_xLineSet->SelectItem(nItemId);
        n++;
    }

    m_xLineSet->SetOptimalSize();
}

Color SvtLineListBox::GetColorLine1( sal_Int32 nPos )
{
    sal_Int32 nStyle = GetStylePos( nPos );
    if (nStyle == -1)
        return GetPaintColor( );
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorLine1( GetColor( ) );
}

Color SvtLineListBox::GetColorLine2( sal_Int32 nPos )
{
    sal_Int32 nStyle = GetStylePos(nPos);
    if (nStyle == -1)
        return GetPaintColor( );
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorLine2( GetColor( ) );
}

Color SvtLineListBox::GetColorDist( sal_Int32 nPos )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Color rResult = rSettings.GetFieldColor();

    sal_Int32 nStyle = GetStylePos( nPos );
    if (nStyle == -1)
        return rResult;
    auto& pData = m_vLineList[ nStyle ];
    return pData->GetColorDist( GetColor( ), rResult );
}

IMPL_LINK_NOARG(SvtLineListBox, ValueSelectHdl, ValueSet*, void)
{
    maSelectHdl.Call(*this);
    UpdatePreview();
    if (m_xControl->get_active())
        m_xControl->set_active(false);
}

void SvtLineListBox::UpdatePreview()
{
    SvxBorderLineStyle eStyle = GetSelectEntryStyle();
    for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(RID_SVXSTR_BORDERLINE); ++i)
    {
        if (eStyle == RID_SVXSTR_BORDERLINE[i].second)
        {
            m_xControl->set_label(SvtResId(RID_SVXSTR_BORDERLINE[i].first));
            break;
        }
    }

    if (eStyle == SvxBorderLineStyle::NONE)
    {
        m_xControl->set_image(nullptr);
        m_xControl->set_label(GetLineStyleName(SvxBorderLineStyle::NONE));
    }
    else
    {
        Image aImage(m_xLineSet->GetItemImage(m_xLineSet->GetSelectedItemId()));
        m_xControl->set_label("");
        const auto nPos = (aVirDev->GetOutputSizePixel().Height() - aImage.GetSizePixel().Height()) / 2;
        aVirDev->Push(PushFlags::MAPMODE);
        aVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
        aVirDev->Erase();
        aVirDev->DrawImage(Point(0, nPos), aImage);
        m_xControl->set_image(aVirDev.get());
        aVirDev->Pop();
    }
}

SvtCalendarBox::SvtCalendarBox(std::unique_ptr<weld::MenuButton> pControl)
    : m_xControl(std::move(pControl))
    , m_xBuilder(Application::CreateBuilder(m_xControl.get(), "svt/ui/datewindow.ui"))
    , m_xTopLevel(m_xBuilder->weld_widget("date_popup_window"))
    , m_xCalendar(m_xBuilder->weld_calendar("date"))
{
    m_xControl->set_popover(m_xTopLevel.get());
    m_xCalendar->connect_selected(LINK(this, SvtCalendarBox, SelectHdl));
    m_xCalendar->connect_activated(LINK(this, SvtCalendarBox, ActivateHdl));
}

void SvtCalendarBox::set_date(const Date& rDate)
{
    m_xCalendar->set_date(rDate);
    set_label_from_date();
}

void SvtCalendarBox::set_label_from_date()
{
    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
    m_xControl->set_label(rLocaleData.getDate(m_xCalendar->get_date()));
}

IMPL_LINK_NOARG(SvtCalendarBox, SelectHdl, weld::Calendar&, void)
{
    set_label_from_date();
    m_aSelectHdl.Call(*this);
}

IMPL_LINK_NOARG(SvtCalendarBox, ActivateHdl, weld::Calendar&, void)
{
    if (m_xControl->get_active())
        m_xControl->set_active(false);
    m_aActivatedHdl.Call(*this);
}

SvtCalendarBox::~SvtCalendarBox()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
