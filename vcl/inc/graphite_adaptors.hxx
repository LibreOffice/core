/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
