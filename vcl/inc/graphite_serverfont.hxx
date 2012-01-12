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



#ifndef _SV_GRAPHITESERVERFONT_HXX
#define _SV_GRAPHITESERVERFONT_HXX

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

#ifndef MSC
#include <graphite_layout.hxx>
#include <graphite_adaptors.hxx>

// Modules

class VCL_PLUGIN_PUBLIC GraphiteLayoutImpl : public GraphiteLayout
{
public:
    GraphiteLayoutImpl(const gr::Font & font, const grutils::GrFeatureParser * features, GraphiteFontAdaptor * pFont) throw()
    : GraphiteLayout(font, features), mpFont(pFont) {};
    virtual ~GraphiteLayoutImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & width);
private:
    GraphiteFontAdaptor * mpFont;
};

// This class implments the server font specific parts.
// @author tse
//
class VCL_PLUGIN_PUBLIC GraphiteServerFontLayout : public ServerFontLayout
{
private:
        mutable GraphiteFontAdaptor * mpFont;
        // mutable so that the DrawOffset/DrawBase can be set
        mutable GraphiteLayoutImpl maImpl;
public:
        explicit GraphiteServerFontLayout( GraphiteFontAdaptor* font ) throw();

        virtual bool  LayoutText( ImplLayoutArgs& rArgs) { SalLayout::AdjustLayout(rArgs); return maImpl.LayoutText(rArgs); };    // first step of layout
        virtual void  AdjustLayout( ImplLayoutArgs& rArgs)
        {
            SalLayout::AdjustLayout(rArgs);
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            maImpl.AdjustLayout(rArgs);
        };
        virtual long    GetTextWidth() const                           { return maImpl.GetTextWidth(); }
        virtual long    FillDXArray( sal_Int32* dxa ) const                 { return maImpl.FillDXArray(dxa); }
        virtual int     GetTextBreak( long mw, long ce, int f ) const  { return maImpl.GetTextBreak(mw, ce, f); }
        virtual void    GetCaretPositions( int as, sal_Int32* cxa ) const   { maImpl.GetCaretPositions(as, cxa); }

        // used by display layers
        virtual int     GetNextGlyphs( int l, sal_GlyphId* gia, Point& p, int& s,
                        sal_Int32* gaa = NULL, int* cpa = NULL ) const
        {
            maImpl.DrawBase() = maDrawBase;
            maImpl.DrawOffset() = maDrawOffset;
            return maImpl.GetNextGlyphs(l, gia, p, s, gaa, cpa);
        }

        virtual void    MoveGlyph( int nStart, long nNewXPos ) { maImpl.MoveGlyph(nStart, nNewXPos); };
        virtual void    DropGlyph( int nStart ) { maImpl.DropGlyph(nStart); };
        virtual void    Simplify( bool bIsBase ) { maImpl.Simplify(bIsBase); };

        virtual ~GraphiteServerFontLayout() throw();

// For use with PspGraphics
        const sal_Unicode* getTextPtr() const;
        int getMinCharPos() const { return mnMinCharPos; }
        int getMaxCharPos() const { return mnEndCharPos; }
};

#endif
#endif //_SV_GRAPHITESERVERFONT_HXX
