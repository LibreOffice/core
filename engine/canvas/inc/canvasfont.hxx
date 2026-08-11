/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/StringContext.hpp>

#include <vcl/font.hxx>

#include "vclwrapper.hxx"


/* Definition of CanvasFont class */

namespace vclcanvas
{
    class Canvas;
    class TextLayout;

    /** This interface provides access to a specific, XCanvas-dependent
        font incarnation. This font is not universally usable, but belongs
        to the XCanvas it was queried from.
     */
    class CanvasFont : public salhelper::SimpleReferenceObject
    {
    public:
        /// make noncopyable
        CanvasFont(const CanvasFont&) = delete;
        const CanvasFont& operator=(const CanvasFont&) = delete;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    FontEmphasisMark                                                       eMark,
                    const css::geometry::Matrix2D&                                         rFontMatrix,
                    OutputDevice&                                                          rOutDevProvider );

        /** Create a text layout interface.<p>

          Create a text layout interface for the given string, using
          this font to generate the glyphs from.<p>

          @param aText
          The text to layout.

          @param nDirection
          Main text direction for the string specified. The main text
          direction is e.g. important for characters that are not
          strong, i.e. that change affinity according to the current
          writing direction. Make sure that across text portions and
          lines, the direction is set consistently.

          @param nRandomSeed
          Optional random seed for OpenType glyph variations.
        */
        rtl::Reference< vclcanvas::TextLayout > createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed );

        /** Query the FontRequest that was used to generate this object.
         */
        css::rendering::FontRequest getFontRequest(  );

        vcl::Font const & getVCLFont() const;
      
        const css::geometry::Matrix2D& getFontMatrix() const;

    private:
        ::canvas::vcltools::VCLObject<vcl::Font>                          maFont;
        css::rendering::FontRequest                                       maFontRequest;
        VclPtr<OutputDevice>                                              mxOutDev;
        css::geometry::Matrix2D                                           maFontMatrix;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
