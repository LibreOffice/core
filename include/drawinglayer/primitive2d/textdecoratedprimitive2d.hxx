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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx { namespace tools {
    class B2DHomMatrixBufferedOnDemandDecompose;
}}

namespace com { namespace sun { namespace star { namespace i18n {
    struct Boundary;
}}}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextDecoratedPortionPrimitive2D class

            This primitive expands the TextSimplePortionPrimitive2D by common
            decorations used in the office. It can be decomposed and will create
            a TextSimplePortionPrimitive2D and all the contained decorations (if used)
            as geometry.
         */
        class DRAWINGLAYER_DLLPUBLIC TextDecoratedPortionPrimitive2D : public TextSimplePortionPrimitive2D
        {
        private:
            /// decoration definitions
            basegfx::BColor                             maOverlineColor;
            basegfx::BColor                             maTextlineColor;
            TextLine                                    meFontOverline;
            TextLine                                    meFontUnderline;
            TextStrikeout                               meTextStrikeout;
            TextEmphasisMark                            meTextEmphasisMark;
            TextRelief                                  meTextRelief;

            /// bitfield
            unsigned                                    mbUnderlineAbove : 1;
            unsigned                                    mbWordLineMode : 1;
            unsigned                                    mbEmphasisMarkAbove : 1;
            unsigned                                    mbEmphasisMarkBelow : 1;
            unsigned                                    mbShadow : 1;

            /// helper methods
            void impCreateGeometryContent(
                std::vector< Primitive2DReference >& rTarget,
                basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans,
                const OUString& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const attribute::FontAttribute& rFontAttribute) const;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            TextDecoratedPortionPrimitive2D(

                /// TextSimplePortionPrimitive2D parameters
                const basegfx::B2DHomMatrix& rNewTransform,
                const OUString& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const attribute::FontAttribute& rFontAttribute,
                const ::com::sun::star::lang::Locale& rLocale,
                const basegfx::BColor& rFontColor,

                /// local parameters
                const basegfx::BColor& rOverlineColor,
                const basegfx::BColor& rTextlineColor,
                TextLine eFontOverline = TEXT_LINE_NONE,
                TextLine eFontUnderline = TEXT_LINE_NONE,
                bool bUnderlineAbove = false,
                TextStrikeout eTextStrikeout = TEXT_STRIKEOUT_NONE,
                bool bWordLineMode = false,
                TextEmphasisMark eTextEmphasisMark = TEXT_EMPHASISMARK_NONE,
                bool bEmphasisMarkAbove = true,
                bool bEmphasisMarkBelow = false,
                TextRelief eTextRelief = TEXT_RELIEF_NONE,
                bool bShadow = false);

            /// data read access
            TextLine getFontOverline() const { return meFontOverline; }
            TextLine getFontUnderline() const { return meFontUnderline; }
            TextStrikeout getTextStrikeout() const { return meTextStrikeout; }
            TextEmphasisMark getTextEmphasisMark() const { return meTextEmphasisMark; }
            TextRelief getTextRelief() const { return meTextRelief; }
            const basegfx::BColor& getOverlineColor() const { return maOverlineColor; }
            const basegfx::BColor& getTextlineColor() const { return maTextlineColor; }
            bool getUnderlineAbove() const { return mbUnderlineAbove; }
            bool getWordLineMode() const { return mbWordLineMode; }
            bool getEmphasisMarkAbove() const { return mbEmphasisMarkAbove; }
            bool getEmphasisMarkBelow() const { return mbEmphasisMarkBelow; }
            bool getShadow() const { return mbShadow; }

            /// check if this needs to be a TextDecoratedPortionPrimitive2D or
            /// if a TextSimplePortionPrimitive2D would be suficcient
            bool decoratedIsNeeded() const;

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
