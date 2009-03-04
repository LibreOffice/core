/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextprimitive2d.hxx,v $
 *
 * $Revision: 1.2.18.1 $
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
#include <boost/shared_ptr.hpp>
#include <svx/outlobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrText;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrTextPrimitive2D : public BasePrimitive2D
        {
        private:
            // The text model data; this sould later just be the OutlinerParaObject or
            // something equal
            const SdrText&                          mrSdrText;              // text model data

            // #i97628#
            // The text content; now as OutlinerParaObject* and in exclusive, local, cloned
            // form as needed in a primitive
            const OutlinerParaObject                maOutlinerParaObject;

            // remeber last VisualizingPage for which a decomposition was made. If the new target
            // is not given or different, the decomposition needs to be potentially removed
            // for supporting e.g. page number change on MasterPage objects or the different
            // field renderings in SubGeometry and MasterPage mnode
            com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > mxLastVisualizingPage;

            // bitfield
            // remember if last decomposition was with or without spell checker. In this special
            // case the get2DDecomposition implementation has to take care of this aspect. This is
            // needed since different views do different text decompositons regarding spell checking.
            unsigned                                mbLastSpellCheck : 1;

            // is there a PageNumber, Header, Footer or DateTimeField used? Evaluated at construction
            unsigned                                mbContainsPageField : 1;

        protected:
            // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
            Primitive2DSequence encapsulateWithTextHierarchyBlockPrimitive2D(const Primitive2DSequence& rCandidate) const;

            bool getLastSpellCheck() const { return (bool)mbLastSpellCheck; }
            void setLastSpellCheck(bool bNew) { mbLastSpellCheck = bNew; }

        public:
            SdrTextPrimitive2D(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr);

            // get data
            const SdrText& getSdrText() const { return mrSdrText; }
            const OutlinerParaObject& getOutlinerParaObject() const { return maOutlinerParaObject; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // own get2DDecomposition to take aspect of decomposition with or without spell checker
            // into account
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const = 0;
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
            ::basegfx::B2DPolyPolygon               maUnitPolyPolygon;      // unit contour polygon (scaled to [0.0 .. 1.0])
            ::basegfx::B2DHomMatrix                 maObjectTransform;      // complete contour polygon transform (scale, rotate, shear, translate)

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrContourTextPrimitive2D(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const ::basegfx::B2DPolyPolygon& rUnitPolyPolygon,
                const ::basegfx::B2DHomMatrix& rObjectTransform);

            // get data
            const ::basegfx::B2DPolyPolygon& getUnitPolyPolygon() const { return maUnitPolyPolygon; }
            const ::basegfx::B2DHomMatrix& getObjectTransform() const { return maObjectTransform; }

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

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrPathTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            ::basegfx::B2DPolyPolygon               maPathPolyPolygon;      // the path to use. Each paragraph will use one Polygon.

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrPathTextPrimitive2D(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const ::basegfx::B2DPolyPolygon& rPathPolyPolygon);

            // get data
            const ::basegfx::B2DPolyPolygon& getPathPolyPolygon() const { return maPathPolyPolygon; }

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

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrBlockTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            ::basegfx::B2DHomMatrix                 maTextRangeTransform;   // text range transformation from unit range ([0.0 .. 1.0]) to text range

            // bitfield
            unsigned                                mbUnlimitedPage : 1;    // force layout with no text break
            unsigned                                mbCellText : 1;         // this is a cell text as block text
            unsigned                                mbWordWrap : 1;         // for CustomShapes text layout

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrBlockTextPrimitive2D(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const ::basegfx::B2DHomMatrix& rTextRangeTransform,
                bool bUnlimitedPage,
                bool bCellText,
                bool bWordWrap);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            bool getUnlimitedPage() const { return mbUnlimitedPage; }
            bool getCellText() const { return mbCellText; }
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

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrStretchTextPrimitive2D : public SdrTextPrimitive2D
        {
        private:
            ::basegfx::B2DHomMatrix                 maTextRangeTransform;   // text range transformation from unit range ([0.0 .. 1.0]) to text range

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrStretchTextPrimitive2D(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                const ::basegfx::B2DHomMatrix& rTextRangeTransform);

            // get data
            const ::basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }

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

// eof
