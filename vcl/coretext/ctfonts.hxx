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

#include "coretext/salgdi2.h"
#include "sallayout.hxx"

#include <ApplicationServices/ApplicationServices.h>

// =======================================================================

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
