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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_XPDFWRAPPER_PDFIOUTDEV_GPL_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_XPDFWRAPPER_PDFIOUTDEV_GPL_HXX

#include <sal/types.h>
#include <config_poppler.h>

#if defined __GNUC__ || defined __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wundef"
# pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4310) // cast truncates constant value
#endif

#include <GfxState.h>
#include <GfxFont.h>
#include <UnicodeMap.h>
#include <Link.h>
#include <Object.h>
#include <OutputDev.h>
#include <GlobalParams.h>
#include <PDFDoc.h>

#if defined __GNUC__ || defined __clang__
# pragma GCC diagnostic pop
#elif defined _MSC_VER
#pragma warning(pop)
#endif

#include <unordered_map>
#include <vector>
#include <memory>

class GfxPath;
class GfxFont;
class PDFDoc;
#if HAVE_POPPLER_VERSION_H
#include <cpp/poppler-version.h>
#else
#define POPPLER_VERSION "0.12.3"
#define POPPLER_VERSION_MAJOR 0
#define POPPLER_VERSION_MINOR 12
#define POPPLER_VERSION_MICRO 3
#endif
#define POPPLER_CHECK_VERSION(major,minor,micro) \
  (POPPLER_VERSION_MAJOR > (major) || \
   (POPPLER_VERSION_MAJOR == (major) && POPPLER_VERSION_MINOR > (minor)) || \
   (POPPLER_VERSION_MAJOR == (major) && POPPLER_VERSION_MINOR == (minor) && POPPLER_VERSION_MICRO >= (micro)))

namespace pdfi
{
    struct FontAttributes
    {
        FontAttributes() :
            familyName(),
            isEmbedded(false),
            isBold(false),
            isItalic(false),
            isUnderline(false),
            size(0.0)
        {}

        // xpdf goo stuff is so totally borked...
        // ...need to hand-code assignment
        FontAttributes( const FontAttributes& rSrc ) :
            familyName(),
            isEmbedded(rSrc.isEmbedded),
            isBold(rSrc.isBold),
            isItalic(rSrc.isItalic),
            isUnderline(rSrc.isUnderline),
            size(rSrc.size)
        {
            familyName.append(&rSrc.getFamilyName());
        }

        FontAttributes& operator=( const FontAttributes& rSrc )
        {
            familyName.clear();
            familyName.append(&rSrc.getFamilyName());

            isEmbedded  = rSrc.isEmbedded;
            isBold      = rSrc.isBold;
            isItalic    = rSrc.isItalic;
            isUnderline = rSrc.isUnderline;
            size        = rSrc.size;

            return *this;
        }

        bool operator==(const FontAttributes& rFont) const
        {
            return getFamilyName().cmp(&rFont.getFamilyName())==0 &&
                isEmbedded == rFont.isEmbedded &&
                isBold == rFont.isBold &&
                isItalic == rFont.isItalic &&
                isUnderline == rFont.isUnderline &&
                size == rFont.size;
        }

        GooString     familyName;
        bool        isEmbedded;
        bool        isBold;
        bool        isItalic;
        bool        isUnderline;
        double      size;

    private:
        // Work around const-ness issues in the GooString API:
        GooString & getFamilyName() const
        { return const_cast<GooString &>(familyName); }
    };

    // Versions before 0.15 defined GBool as int; 0.15 redefined it as bool; 0.71 dropped GBool
#if POPPLER_VERSION_MAJOR == 0 && POPPLER_VERSION_MINOR < 71
    typedef GBool poppler_bool;
#else
    typedef bool poppler_bool;
#endif

    class PDFOutDev : public OutputDev
    {
        // not owned by this class
        PDFDoc*                                 m_pDoc;
        mutable std::unordered_map< long long,
                               FontAttributes > m_aFontMap;
        std::unique_ptr<UnicodeMap>             m_pUtf8Map;
        bool                                    m_bSkipImages;

        int  parseFont( long long nNewId, GfxFont* pFont, GfxState* state ) const;
        void writeFontFile( GfxFont* gfxFont ) const;
        static void printPath( GfxPath* pPath );

    public:
        explicit PDFOutDev( PDFDoc* pDoc );
        virtual ~PDFOutDev() override;

        //----- get info about output device

        // Does this device use upside-down coordinates?
        // (Upside-down means (0,0) is the top left corner of the page.)
        virtual poppler_bool upsideDown() override { return true; }

        // Does this device use drawChar() or drawString()?
        virtual poppler_bool useDrawChar() override { return true; }

        // Does this device use beginType3Char/endType3Char?  Otherwise,
        // text in Type 3 fonts will be drawn with drawChar/drawString.
        virtual poppler_bool interpretType3Chars() override { return false; }

        // Does this device need non-text content?
        virtual poppler_bool needNonText() override { return true; }

        //----- initialization and control

        // Set default transform matrix.
#if POPPLER_CHECK_VERSION(0, 71, 0)
        virtual void setDefaultCTM(const double *ctm) override;
#else
        virtual void setDefaultCTM(double *ctm) override;
#endif

        // Start a page.
        virtual void startPage(int pageNum, GfxState *state
#if POPPLER_CHECK_VERSION(0, 23, 0) || POPPLER_CHECK_VERSION(0, 24, 0)
                               , XRef *xref
#endif
        ) override;

        // End a page.
        virtual void endPage() override;

        //----- link borders
    #if POPPLER_CHECK_VERSION(0, 19, 0)
        virtual void processLink(AnnotLink *link) override;
    #elif POPPLER_CHECK_VERSION(0, 17, 0)
        virtual void processLink(AnnotLink *link, Catalog *catalog) override;
    #else
        virtual void processLink(Link *link, Catalog *catalog) override;
    #endif

        //----- save/restore graphics state
        virtual void saveState(GfxState *state) override;
        virtual void restoreState(GfxState *state) override;

        //----- update graphics state
        virtual void updateCTM(GfxState *state, double m11, double m12,
                               double m21, double m22, double m31, double m32) override;
        virtual void updateLineDash(GfxState *state) override;
        virtual void updateFlatness(GfxState *state) override;
        virtual void updateLineJoin(GfxState *state) override;
        virtual void updateLineCap(GfxState *state) override;
        virtual void updateMiterLimit(GfxState *state) override;
        virtual void updateLineWidth(GfxState *state) override;
        virtual void updateFillColor(GfxState *state) override;
        virtual void updateStrokeColor(GfxState *state) override;
        virtual void updateFillOpacity(GfxState *state) override;
        virtual void updateStrokeOpacity(GfxState *state) override;
        virtual void updateBlendMode(GfxState *state) override;

        //----- update text state
        virtual void updateFont(GfxState *state) override;
        virtual void updateRender(GfxState *state) override;

        //----- path painting
        virtual void stroke(GfxState *state) override;
        virtual void fill(GfxState *state) override;
        virtual void eoFill(GfxState *state) override;

        //----- path clipping
        virtual void clip(GfxState *state) override;
        virtual void eoClip(GfxState *state) override;

        //----- text drawing
        virtual void drawChar(GfxState *state, double x, double y,
                              double dx, double dy,
                              double originX, double originY,
                              CharCode code, int nBytes, Unicode *u, int uLen) override;
#if POPPLER_CHECK_VERSION(0, 64, 0)
        virtual void drawString(GfxState *state, const GooString *s) override;
#else
        virtual void drawString(GfxState *state, GooString *s) override;
#endif
        virtual void endTextObject(GfxState *state) override;

        //----- image drawing
        virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
                                   int width, int height, poppler_bool invert,
                                   poppler_bool interpolate,
                                   poppler_bool inlineImg) override;
        virtual void drawImage(GfxState *state, Object *ref, Stream *str,
                               int width, int height, GfxImageColorMap *colorMap,
                               poppler_bool interpolate,
                               int* maskColors, poppler_bool inlineImg) override;
        virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
                                     int width, int height,
                                     GfxImageColorMap *colorMap,
                                     poppler_bool interpolate,
                                     Stream *maskStr, int maskWidth, int maskHeight,
                                     poppler_bool maskInvert,
                                     poppler_bool maskInterpolate
                                    ) override;
        virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
                                         int width, int height,
                                         GfxImageColorMap *colorMap,
                                         poppler_bool interpolate,
                                         Stream *maskStr,
                                         int maskWidth, int maskHeight,
                                         GfxImageColorMap *maskColorMap
                                         , poppler_bool maskInterpolate
                                        ) override;

        static void setPageNum( int nNumPages );
        void setSkipImages ( bool bSkipImages );
    };
}

extern FILE* g_binary_out;

// note: if you ever change Output_t, please keep in mind that the current code
// relies on it being of 8 bit size
typedef unsigned char Output_t;
typedef std::vector< Output_t > OutputBuffer;

#if !POPPLER_CHECK_VERSION(0, 73, 0)
static_assert(std::is_same_v<Guchar, unsigned char>, "unexpected typedef");
#endif

#endif // INCLUDED_SDEXT_SOURCE_PDFIMPORT_XPDFWRAPPER_PDFIOUTDEV_GPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
