/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <editeng/outlobj.hxx>
#include <tools/color.hxx>
#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <tools/weakbase.hxx>
#include <svx/sdtaitm.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrText;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrTextPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            // The text model data; this sould later just be the OutlinerParaObject or
            // something equal
            ::tools::WeakReference< SdrText >       mrSdrText;

            // #i97628#
            // The text content; now as local OutlinerParaObject copy (internally RefCounted and
            // COW) and in exclusive, local form as needed in a primitive
            const OutlinerParaObject                maOutlinerParaObject;

            // remeber last VisualizingPage for which a decomposition was made. If the new target
            // is not given or different, the decomposition needs to be potentially removed
            // for supporting e.g. page number change on MasterPage objects or the different
            // field renderings in SubGeometry and MasterPage mnode
            com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > mxLastVisualizingPage;

            // remember last PageNumber for which a decomposition was made. This is only used
            // when mbContainsPageField is true, else it is 0
            sal_Int16                               mnLastPageNumber;

            // remember last PageCount for which a decomposition was made. This is only used
            // when mbContainsPageCountField is true, else it is 0
            sal_Int16                               mnLastPageCount;

            // #i101443# remember last TextBackgroundColor to decide if a new decomposition is
            // needed because of background color change
            Color                                   maLastTextBackgroundColor;

            // bitfield
            // is there a PageNumber, Header, Footer or DateTimeField used? Evaluated at construction
            unsigned                                mbContainsPageField : 1;
            unsigned                                mbContainsPageCountField : 1;
            unsigned                                mbContainsOtherFields : 1;

        protected:
            // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
            Primitive2DSequence encapsulateWithTextHierarchyBlockPrimitive2D(const Primitive2DSequence& rCandidate) const;

        public:
            SdrTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr);

            // get data
            const SdrText* getSdrText() const { return mrSdrText.get(); }
            const OutlinerParaObject& getOutlinerParaObject() const { return maOutlinerParaObject; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // own get2DDecomposition to take aspect of decomposition with or without spell checker
            // into account
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const = 0;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrContourTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            // unit contour polygon (scaled to [0.0 .. 1.0])
            basegfx::B2DPolyPolygon             maUnitPolyPolygon;

            // complete contour polygon transform (scale, rotate, shear, translate)
            basegfx::B2DHomMatrix               maObjectTransform;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrContourTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
                const basegfx::B2DHomMatrix& rObjectTransform);

            // get data
            const basegfx::B2DPolyPolygon& getUnitPolyPolygon() const { return maUnitPolyPolygon; }
            const basegfx::B2DHomMatrix& getObjectTransform() const { return maObjectTransform; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrPathTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            // the path to use. Each paragraph will use one Polygon.
            basegfx::B2DPolyPolygon             maPathPolyPolygon;

            // the Fontwork parameters
            attribute::SdrFormTextAttribute     maSdrFormTextAttribute;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrPathTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const basegfx::B2DPolyPolygon& rPathPolyPolygon,
                const attribute::SdrFormTextAttribute& rSdrFormTextAttribute);

            // get data
            const basegfx::B2DPolyPolygon& getPathPolyPolygon() const { return maPathPolyPolygon; }
            const attribute::SdrFormTextAttribute& getSdrFormTextAttribute() const { return maSdrFormTextAttribute; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrBlockTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            // text range transformation from unit range ([0.0 .. 1.0]) to text range
            basegfx::B2DHomMatrix                   maTextRangeTransform;

            // text alignments
            SdrTextHorzAdjust                       maSdrTextHorzAdjust;
            SdrTextVertAdjust                       maSdrTextVertAdjust;

            // bitfield
            unsigned                                mbFixedCellHeight : 1;
            unsigned                                mbUnlimitedPage : 1;    // force layout with no text break
            unsigned                                mbCellText : 1;         // this is a cell text as block text
            unsigned                                mbWordWrap : 1;         // for CustomShapes text layout
            unsigned                                mbClipOnBounds : 1;     // for CustomShapes text layout

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrBlockTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const basegfx::B2DHomMatrix& rTextRangeTransform,
                SdrTextHorzAdjust aSdrTextHorzAdjust,
                SdrTextVertAdjust aSdrTextVertAdjust,
                bool bFixedCellHeight,
                bool bUnlimitedPage,
                bool bCellText,
                bool bWordWrap,
                bool bClipOnBounds);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            SdrTextHorzAdjust getSdrTextHorzAdjust() const { return maSdrTextHorzAdjust; }
            SdrTextVertAdjust getSdrTextVertAdjust() const { return maSdrTextVertAdjust; }
            bool isFixedCellHeight() const { return mbFixedCellHeight; }
            bool getUnlimitedPage() const { return mbUnlimitedPage; }
            bool getCellText() const { return mbCellText; }
            bool getWordWrap() const { return mbWordWrap; }
            bool getClipOnBounds() const { return mbClipOnBounds; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrStretchTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            // text range transformation from unit range ([0.0 .. 1.0]) to text range
            basegfx::B2DHomMatrix                   maTextRangeTransform;

            // bitfield
            unsigned                                mbFixedCellHeight : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrStretchTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const basegfx::B2DHomMatrix& rTextRangeTransform,
                bool bFixedCellHeight);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            bool isFixedCellHeight() const { return mbFixedCellHeight; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrAutoFitTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            ::basegfx::B2DHomMatrix                 maTextRangeTransform;   // text range transformation from unit range ([0.0 .. 1.0]) to text range

            // bitfield
            unsigned                                mbWordWrap : 1;         // for CustomShapes text layout

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrAutoFitTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const ::basegfx::B2DHomMatrix& rTextRangeTransform,
                bool bWordWrap);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            bool getWordWrap() const { return mbWordWrap; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
