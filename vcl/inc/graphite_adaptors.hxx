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

#ifndef _SV_GRAPHITEADAPTORS_HXX
#define _SV_GRAPHITEADAPTORS_HXX

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Standard Library
#include <stdexcept>
// Platform

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

// Module
#include "vcl/dllapi.h"

// Libraries
#include <preextstl.h>
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/ITextSource.h>
#include <postextstl.h>

// Module type definitions and forward declarations.
//
#ifndef MSC
// SAL/VCL types
class ServerFont;
class FreetypeServerFont;

// Graphite types

struct FontProperties : gr::FontProps
{
    FontProperties(const FreetypeServerFont & font) throw();
};

namespace grutils
{
    class GrFeatureParser;
}

// This class adapts the Sal font and graphics services to form required by
// the Graphite engine.
// @author tse
//
class VCL_PLUGIN_PUBLIC GraphiteFontAdaptor : public gr::Font
{
    typedef std::map<const gr::gid16, std::pair<gr::Rect, gr::Point> > GlyphMetricMap;
    friend class GrFontHasher;
public:
    static bool    IsGraphiteEnabledFont(ServerFont &) throw();

    GraphiteFontAdaptor(ServerFont & font, const sal_Int32 dpi_x, const sal_Int32 dpi_y);
    GraphiteFontAdaptor(const GraphiteFontAdaptor &) throw();
    ~GraphiteFontAdaptor() throw();

     gr::Font    * copyThis();

    // Basic attribute accessors.
    virtual float        ascent();
    virtual float        descent();
    virtual bool        bold();
    virtual bool        italic();
    virtual float        height();
    virtual unsigned int    getDPIx();
    virtual unsigned int    getDPIy();

    // Font access methods.
    virtual const void    * getTable(gr::fontTableId32 tableID, size_t * pcbSize);
    virtual void          getFontMetrics(float * ascent_out, float * descent_out = 0, float * em_square_out = 0);

    // Glyph metrics.
    virtual void      getGlyphMetrics(gr::gid16 glyphID, gr::Rect & boundingBox, gr::Point & advances);

    // Adaptor attributes.
    const FontProperties    & fontProperties() const throw();
    FreetypeServerFont        & font() const throw();
    const grutils::GrFeatureParser * features() const { return mpFeatures; };

private:
    virtual void UniqueCacheInfo(ext_std::wstring &, bool &, bool &);

    FreetypeServerFont& mrFont;
    FontProperties        maFontProperties;
    const unsigned int    mnDpiX, mnDpiY;
    const float           mfAscent,
                    mfDescent,
                    mfEmUnits;
    grutils::GrFeatureParser * mpFeatures;
    GlyphMetricMap maGlyphMetricMap;
};

// Partial implementation of class GraphiteFontAdaptor.
//
inline const FontProperties & GraphiteFontAdaptor::fontProperties() const throw() {
    return maFontProperties;
}

inline FreetypeServerFont & GraphiteFontAdaptor::font() const throw() {
    return mrFont;
}
#endif // not MFC

// Partial implementation of class TextSourceAdaptor.
//
//inline const ImplLayoutArgs & TextSourceAdaptor::layoutArgs() const throw() {
//  return _layout_args;
//}


#endif // _SV_GRAPHITEADAPTORS_HXX
