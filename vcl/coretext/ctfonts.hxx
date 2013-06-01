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

#include "coretext/salgdi2.h"
#include "sallayout.hxx"

#include <ApplicationServices/ApplicationServices.h>

// =======================================================================

class CTTextStyle
:   public ImplMacTextStyle
{
public:
    explicit    CTTextStyle( const ImplFontSelectData& );
    virtual     ~CTTextStyle( void );

    virtual SalLayout* GetTextLayout( void ) const;

    virtual void    GetFontMetric( float fDPIY, ImplFontMetricData& ) const;
    virtual bool    GetGlyphBoundRect( sal_GlyphId, Rectangle& ) const;
    virtual bool    GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) const;

    virtual void    SetTextColor( const RGBAColor& );

private:
    /// CoreText text style object
    CFMutableDictionaryRef  mpStyleDict;

    friend class CTLayout;
    CFMutableDictionaryRef  GetStyleDict( void ) const { return mpStyleDict; }
};

// =======================================================================

#ifndef DISABLE_CORETEXT_DYNLOAD
// the CoreText symbols may need to be loaded dynamically
// since platform targets like OSX 10.4 do not provide all required symbols
// TODO: avoid the dlsym stuff if the target platform is >= OSX10.5

class DynCoreTextSyms
{
public:
    // dynamic symbols to access the CoreText API
    uint32_t    (*GetCoreTextVersion)(void);
    CTFontCollectionRef (*FontCollectionCreateFromAvailableFonts)(CFDictionaryRef);
    CFArrayRef  (*FontCollectionCreateMatchingFontDescriptors)(CTFontCollectionRef);
    CGPathRef   (*FontCreatePathForGlyph)(CTFontRef,CGGlyph,const CGAffineTransform*);
    CGRect          (*FontGetBoundingRectsForGlyphs)(CTFontRef,CTFontOrientation,CGGlyph*,CGRect*,CFIndex);
    CTLineRef   (*LineCreateJustifiedLine)(CTLineRef,CGFloat,double);
    double      (*LineGetTrailingWhitespaceWidth)(CTLineRef);
    CGFloat     (*LineGetOffsetForStringIndex)(CTLineRef,CFIndex,CGFloat*);
    CFArrayRef  (*LineGetGlyphRuns)(CTLineRef);
    CFIndex     (*RunGetGlyphCount)(CTRunRef);
    const CGGlyph*  (*RunGetGlyphsPtr)(CTRunRef);
    const CGPoint*  (*RunGetPositionsPtr)(CTRunRef);
    const CGSize*   (*RunGetAdvancesPtr)(CTRunRef);
    const CFIndex * (*RunGetStringIndicesPtr)(CTRunRef);

    // singleton helpers
    static const DynCoreTextSyms& get( void );
    bool        IsActive( void ) const { return mbIsActive; }

private:
    explicit    DynCoreTextSyms( void );
    bool        mbIsActive;
};

#endif // DISABLE_CORETEXT_DYNLOAD

// =======================================================================

