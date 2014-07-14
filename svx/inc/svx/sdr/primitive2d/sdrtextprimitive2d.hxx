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

            // remember last VisualizingPage for which a decomposition was made. If the new target
            // is not given or different, the decomposition needs to be potentially removed
            // for supporting e.g. page number change on MasterPage objects or the different
            // field renderings in SubGeometry and MasterPage mnode
            com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > mxLastVisualizingPage;

            // remember last PageNumber for which a decomposition was made. This is only used
            // when mbContainsPageField is true, else it is 0
            sal_uInt32                              mnLastPageNumber;

            // remember last PageCount for which a decomposition was made. This is only used
            // when mbContainsPageCountField is true, else it is 0
            sal_uInt32                              mnLastPageCount;

            // #i101443# remember last TextBackgroundColor to decide if a new decomposition is
            // needed because of background color change
            Color                                   maLastTextBackgroundColor;

            // bitfield
            // is there a PageNumber, Header, Footer or DateTimeField used? Evaluated at construction
            bool                                    mbContainsPageField : 1;
            bool                                    mbContainsPageCountField : 1;
            bool                                    mbContainsOtherFields : 1;

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
            bool                                    mbFixedCellHeight : 1;
            bool                                    mbUnlimitedPage : 1;    // force layout with no text break
            bool                                    mbCellText : 1;         // this is a cell text as block text
            bool                                    mbWordWrap : 1;         // for CustomShapes text layout
            bool                                    mbClipOnBounds : 1;     // for CustomShapes text layout

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
            bool                                    mbFixedCellHeight : 1;

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

            // transformed clone operator
            virtual SdrTextPrimitive2D* createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX

// eof
