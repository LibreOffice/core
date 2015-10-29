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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYER_HXX

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include "rgbcolor.hxx"

#include <stack>
#include <boost/shared_ptr.hpp>


namespace slideshow
{
    namespace internal
    {

        /** This interface represents a stateful object.

            The state ID returned by the getStateId() method
            abstractly encodes the object's state. When this ID
            changes, clients can assume that the object's state has
            changed.
         */
        class State
        {
        public:
            virtual ~State() {}

            /// Abstract, numerically encoded state ID
            typedef ::std::size_t StateId;

            /** This method returns a numerical state identifier.

                The state ID returned by this method abstractly
                encodes the object's state. When this ID changes,
                clients can assume that the object's state has
                changed.

                @return an abstract, numerical state ID.
             */
            ;
        };

        typedef ::boost::shared_ptr< State > StateSharedPtr;

        class ShapeAttributeLayer;

        typedef ::boost::shared_ptr< ShapeAttributeLayer > ShapeAttributeLayerSharedPtr;

        /** Encapsulates all modifiable attributes of a shape.

            This class holds all modifiable attributes of a shape, and
            at the same time provides means to layer attributes on top
            of each other..

            And yes, there's a reason why we even pass bools and ints
            by const reference. Namely, that makes the set* methods
            differ only in the value type, which greatly reduces
            template variability (e.g. in AnimationFactory).
         */
        class ShapeAttributeLayer
        {
        public:
            /** Create a ShapeAttributeLayer instance, with all
                attributes set to default.

                Furthermore, this constructor gets a pointer to a
                child layer, which is used as the fallback (or the
                base value) for all attributes

                @param rChildLayer
                Layer below this one

                @attention
                This method is only supposed to be called from Shape objects
             */
            explicit ShapeAttributeLayer( const ShapeAttributeLayerSharedPtr& rChildLayer );

            // Children management methods


            /** Revoke the given layer.

                This method revokes the given layer from this object
                or one of the children. That is, if this object does
                have children, and the given layer is no direct child,
                it is recursively passed to the children for removal.

                @return true, if removal was successful.

                @attention
                This method is only supposed to be called from Shape objects
             */
            bool revokeChildLayer( const ShapeAttributeLayerSharedPtr& rChildLayer );

            /** Query the child layer of this object.

                @attention
                This method is only supposed to be called from Shape objects
             */
            ShapeAttributeLayerSharedPtr getChildLayer() const;

            /** Set the additive mode for possible child attributes

                This method sets the additive mode for child
                attributes. That is the way underlying attribute
                layers are combined with this one (i.e. to overrule
                lower layers, or how to combine the values). The
                default is
                css::animations::AnimationAdditiveMode::BASE,
                which means, take the value of the underlying layers,
                or from the model shape itself.

                @param nMode
                Must be one of
                css::animations::AnimationAdditiveMode.
             */
            void setAdditiveMode( sal_Int16 nMode );

            // Attribute methods


            /** Query whether the width attribute is valid.
             */
            bool isWidthValid() const;
            /** Query the current width of the shape
             */
            double getWidth() const;
            /** Set the new width of the shape

                @param rNewWidth
                A negative width mirrors the shape.
             */
            void setWidth( const double& rNewWidth );

            /** Query whether the height attribute is valid.
             */
            bool isHeightValid() const;
            /** Query the current height of the shape
             */
            double getHeight() const;
            /** Set the new height of the shape

                @param rNewHeight
                A negative height mirrors the shape.
             */
            void setHeight( const double& rNewHeight );

            /** Set the new size of the shape

                @param rNewSize
                A negative size mirrors the shape.
             */
            void setSize( const ::basegfx::B2DSize& rNewSize );

            /** Query whether the x position attribute is valid
             */
            bool isPosXValid() const;
            /** Query the current x position of the shape.

                The current x position of the shape is always relative
                to the <em>center</em> of the shape (in contrast to
                the Shape::getBounds() and Shape::getUpdateArea()
                methods).
             */
            double getPosX() const;
            /** Set the new x position of the shape

                The current x position of the shape is always relative
                to the <em>center</em> of the shape (in contrast to
                the Shape::getBounds() and Shape::getUpdateArea()
                methods).
             */
            void setPosX( const double& rNewX );

            /** Query whether the y position attribute is valid
             */
            bool isPosYValid() const;
            /** Query the current y position of the shape

                The current y position of the shape is always relative
                to the <em>center</em> of the shape (in contrast to
                the Shape::getBounds() and Shape::getUpdateArea()
                methods).
             */
            double getPosY() const;
            /** Set the new y position of the shape

                The current y position of the shape is always relative
                to the <em>center</em> of the shape (in contrast to
                the Shape::getBounds() and Shape::getUpdateArea()
                methods).
             */
            void setPosY( const double& rNewY );

            /** Set the new position of the shape

                The current position of the shape is always relative
                to the <em>center</em> of the shape (in contrast to
                the Shape::getBounds() and Shape::getUpdateArea()
                methods).
             */
            void setPosition( const ::basegfx::B2DPoint& rNewPos );

            /** Query whether the rotation angle attribute is valid
             */
            bool isRotationAngleValid() const;
            /** Query the current rotation angle of the shape

                @return the rotation angle in degrees.
             */
            double getRotationAngle() const;
            /** Set the new rotation angle of the shape

                @param rNewAngle
                New rotation angle in degrees.
             */
            void setRotationAngle( const double& rNewAngle );

            /** Query whether the shear x angle attribute is valid
             */
            bool isShearXAngleValid() const;
            /** Query the current shear angle at the x axis of the shape

                @return the shear angle in degrees.
             */
            double getShearXAngle() const;
            /** Set the new shear angle at the x axis of the shape

                @param rNewAngle
                New shear angle in radians.
             */
            void setShearXAngle( const double& rNewAngle );

            /** Query whether the shear y angle attribute is valid
             */
            bool isShearYAngleValid() const;
            /** Query the current shear angle at the y axis of the shape

                @return the shear angle in degrees.
             */
            double getShearYAngle() const;
            /** Set the new shear angle at the y axis of the shape

                @param rNewAngle
                New shear angle in radians.
             */
            void setShearYAngle( const double& rNewAngle );

            /** Query whether the alpha attribute is valid
             */
            bool isAlphaValid() const;
            /** Query the current alpha value of the shape
             */
            double getAlpha() const;
            /** Set the new alpha value of the shape

                @param rNewValue
                New alpha value, must be in the [0,1] range
             */
            void setAlpha( const double& rNewValue );

            /** Query whether the clip attribute is valid
             */
            bool isClipValid() const;
            /** Query the current clip polygon of the shape
             */
            ::basegfx::B2DPolyPolygon getClip() const;
            /** Set the new clip polygon of the shape

                @param rNewClip
                New clip polygon, is interpreted in shape view coordinates, but
                relative to the shape (i.e. the origin of the shape coincides
                with the origin of the clip polygon).
             */
            void setClip( const ::basegfx::B2DPolyPolygon& rNewClip );

            /** Query whether the dim color attribute is valid

                The dim color globally 'dims' the shape towards that
                color
             */
            bool isDimColorValid() const;
            /** Get the dim color for the whole shape.
             */
            RGBColor getDimColor() const;
            /** Set the dim color globally for the whole shape.
             */
            void setDimColor( const RGBColor& nNewColor );

            /** Query whether the fill color attribute is valid
             */
            bool isFillColorValid() const;
            /** Get the fill color for the whole shape.

                If there's no unique fill color, the color from the
                first filled polygon is returned.
             */
            RGBColor getFillColor() const;
            /** Set the fill color globally for the whole shape.
             */
            void setFillColor( const RGBColor& nNewColor );

            /** Query whether the line color attribute is valid
             */
            bool isLineColorValid() const;
            /** Get the line color for the whole shape.

                If there's no unique line color, the color from the
                first line is returned.
             */
            RGBColor  getLineColor() const;
            /** Set the line color globally for the whole shape.
             */
            void setLineColor( const RGBColor& nNewColor );

            /** Query whether the fill mode attribute is valid
             */
            bool isFillStyleValid() const;
            /** Get the current fill mode for polygon fillings.

                @returns the current style
             */
            sal_Int16 getFillStyle() const;
            /** Changes polygon fillings.
             */
            void setFillStyle( const sal_Int16& rStyle );

            /** Query whether the line mode attribute is valid
             */
            bool isLineStyleValid() const;
            /** Get the current line mode for line drawing.

                @returns the current line style
             */
            sal_Int16 getLineStyle() const;
            /** Set line style for the whole shape
             */
            void setLineStyle( const sal_Int16& rStyle );

            /** Query whether the visibility state attribute is valid
             */
            bool isVisibilityValid() const;
            /** Get the current shape visibility.

                @returns true for visible, false for invisible.
             */
            bool getVisibility() const;
            /** Set the shape visibility
             */
            void setVisibility( const bool& bVisible );

            /** Query whether the char color attribute is valid
             */
            bool isCharColorValid() const;
            /** Get the text color for the whole shape.

                If there's no unique text color, the color from the
                first text drawn is returned.
             */
            RGBColor getCharColor() const;
            /** Set the text color globally for the whole shape.
             */
            void setCharColor( const RGBColor& nNewColor );

            /** Query whether the char rotation angle attribute is valid
             */
            bool isCharRotationAngleValid() const;
            /** Query the current text rotation angle of the shape

                @return the text rotation angle in degrees.
             */
            double getCharRotationAngle() const;
            /** Set the new text rotation angle of the shape

                @param rNewAngle
                New text rotation angle in degrees.
             */
            void setCharRotationAngle( const double& rNewAngle );

            /** Query whether the char weight attribute is valid
             */
            bool isCharWeightValid() const;
            /** Get the current char weight value for the whole shape.

                @returns the value for the char weight. The value must
                be out of the css::awt::FontWeight
                constant group.
             */
            double getCharWeight() const;
            /** Set the char weight globally for the whole shape.

                The value must be out of the
                css::awt::FontWeight constant group.
             */
            void setCharWeight( const double& rStyle );

            /** Query whether the underline mode attribute is valid
             */
            bool isUnderlineModeValid() const;
            /** Get the current text underline status for the whole shape.

                If there is no unique underline status, false is returned.

                @returns true for underlined text, false for normal.
             */
            sal_Int16 getUnderlineMode() const;
            /** Set the underline status globally for the whole shape
             */
            void setUnderlineMode( const sal_Int16& bUnderline );

            /** Query whether the font family attribute is valid
             */
            bool isFontFamilyValid() const;
            /** Get the current text font family for the whole shape.

                If there is no unique font family, the font family of
                the first text of the shape is returned.
             */
            OUString getFontFamily() const;
            /** Set the text font family name globally for the whole shape
             */
            void setFontFamily( const OUString& rName );

            /** Query whether the italic mode attribute is valid
             */
            bool isCharPostureValid() const;
            /** Get the current text italic style for the whole shape.

                @returns the italic style. The value returned is one
                of the css::awt::FontSlant enums
             */
            sal_Int16 getCharPosture() const;
            /** Set the italic style globally for the whole shape.

                The value must be one of the
                css::awt::FontSlant enums.
            */
            void setCharPosture( const sal_Int16& rStyle );

            /** Query whether the char scaling attribute is valid
             */
            bool isCharScaleValid() const;
            /** Query the current char scaling attribute globally for
                the shape.

                The char scaling changes the scale of the whole shape
                text (uniformely, i.e. both in x and in y direction).
             */
            double getCharScale() const;
            /** Set the new char scale globally for the shape

                @param rNewScale
                New char scale
             */
            void setCharScale( const double& rNewScale );

            // State change query methods


            State::StateId getTransformationState() const;
            State::StateId getClipState() const;
            State::StateId getAlphaState() const;
            State::StateId getPositionState() const;
            State::StateId getContentState() const;
            State::StateId getVisibilityState() const;

        private:
            // default copy/assignment operator is okay
            // ShapeAttributeLayer(const ShapeAttributeLayer&);
            // ShapeAttributeLayer& operator=( const ShapeAttributeLayer& );

            bool haveChild() const { return static_cast< bool >(mpChild); }
            void updateStateIds();

            template< typename T > T calcValue( const T&                   rCurrValue,
                                                bool                           bThisInstanceValid,
                                                bool (ShapeAttributeLayer::*pIsValid)() const,
                                                T   (ShapeAttributeLayer::*pGetValue)() const ) const;

            ShapeAttributeLayerSharedPtr                mpChild; // may be NULL

            ::basegfx::B2DSize                          maSize;
            ::basegfx::B2DPoint                         maPosition;
            ::basegfx::B2DPolyPolygon                   maClip;

            OUString                             maFontFamily;

            double                                      mnRotationAngle;
            double                                      mnShearXAngle;
            double                                      mnShearYAngle;
            double                                      mnAlpha;
            double                                      mnCharRotationAngle;
            double                                      mnCharScale;
            double                                      mnCharWeight;

            css::drawing::FillStyle        meFillStyle;
            css::drawing::LineStyle        meLineStyle;
            css::awt::FontSlant            meCharPosture;
            sal_Int16                                   mnUnderlineMode;

            RGBColor                                    maDimColor;
            RGBColor                                    maFillColor;
            RGBColor                                    maLineColor;
            RGBColor                                    maCharColor;

            State::StateId                              mnTransformationState;
            State::StateId                              mnClipState;
            State::StateId                              mnAlphaState;
            State::StateId                              mnPositionState;
            State::StateId                              mnContentState;
            State::StateId                              mnVisibilityState;

            sal_Int16                                   mnAdditiveMode;

            bool                                        mbVisibility            : 1;

            bool                                        mbWidthValid            : 1;
            bool                                        mbHeightValid           : 1;
            bool                                        mbPosXValid             : 1;
            bool                                        mbPosYValid             : 1;
            bool                                        mbClipValid             : 1;

            bool                                        mbFontFamilyValid       : 1;

            bool                                        mbRotationAngleValid    : 1;
            bool                                        mbShearXAngleValid      : 1;
            bool                                        mbShearYAngleValid      : 1;

            bool                                        mbAlphaValid            : 1;

            bool                                        mbCharRotationAngleValid: 1;
            bool                                        mbCharScaleValid        : 1;

            bool                                        mbDimColorValid         : 1;
            bool                                        mbFillColorValid        : 1;
            bool                                        mbLineColorValid        : 1;
            bool                                        mbCharColorValid        : 1;

            bool                                        mbFillStyleValid        : 1;
            bool                                        mbLineStyleValid        : 1;
            bool                                        mbCharWeightValid       : 1;
            bool                                        mbUnderlineModeValid    : 1;
            bool                                        mbCharPostureValid      : 1;
            bool                                        mbVisibilityValid       : 1;
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
