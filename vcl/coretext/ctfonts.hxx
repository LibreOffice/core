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

#include "coretext/salgdi2.h"
#ifdef IOS
#include "headless/svpgdi.hxx"
#endif
#include "sallayout.hxx"

class CTTextStyle
:   public ImplMacTextStyle
{
public:
    explicit    CTTextStyle( const FontSelectPattern& );
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

// CoreText specific physically available font face
class CTFontData
:   public ImplMacFontData
{
public:
    explicit                CTFontData( const ImplDevFontAttributes&, sal_IntPtr nFontId );
    explicit                CTFontData( CTFontDescriptorRef pFontDesc );
    virtual                 ~CTFontData( void );
    virtual PhysicalFontFace*   Clone( void ) const;

    virtual ImplMacTextStyle*   CreateMacTextStyle( const FontSelectPattern& ) const;
    virtual ImplFontEntry*      CreateFontInstance( /*const*/ FontSelectPattern& ) const;
    virtual int                 GetFontTable( const char pTagName[5], unsigned char* ) const;
};

SystemFontList* GetCoretextFontList(void);
ImplDevFontAttributes DevFontFromCTFontDescriptor( CTFontDescriptorRef, bool* );

// =======================================================================
