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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <editeng/outlobj.hxx>
#include <tools/color.hxx>
#include <sdr/attribute/sdrformtextattribute.hxx>
#include <tools/weakbase.h>
#include <svx/sdtaitm.hxx>
#include <rtl/ref.hxx>


// predefines
class SdrText;


namespace drawinglayer::primitive2d
    {
        class SdrTextPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            // The text model data; this should later just be the OutlinerParaObject or
            // something equal
            ::tools::WeakReference< SdrText >       mrSdrText;

            // #i97628#
            // The text content; now as local OutlinerParaObject copy (internally RefCounted and
            // COW) and in exclusive, local form as needed in a primitive
            const OutlinerParaObject                maOutlinerParaObject;

            // remember last VisualizingPage for which a decomposition was made. If the new target
            // is not given or different, the decomposition needs to be potentially removed
            // for supporting e.g. page number change on MasterPage objects or the different
            // field renderings in SubGeometry and MasterPage node
            css::uno::Reference< css::drawing::XDrawPage > mxLastVisualizingPage;

            // remember last PageNumber for which a decomposition was made. This is only used
            // when mbContainsPageField is true, else it is 0
            sal_Int16                               mnLastPageNumber;

            // remember last PageCount for which a decomposition was made. This is only used
            // when mbContainsPageCountField is true, else it is 0
            sal_Int16                               mnLastPageCount;

            // #i101443# remember last TextBackgroundColor to decide if a new decomposition is
            // needed because of background color change
            Color                                   maLastTextBackgroundColor;

            // is there a PageNumber, Header, Footer or DateTimeField used? Evaluated at construction
            bool                                    mbContainsPageField : 1;
            bool                                    mbContainsPageCountField : 1;
            bool                                    mbContainsOtherFields : 1;

        protected:
            // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
            static void encapsulateWithTextHierarchyBlockPrimitive2D(Primitive2DContainer& rContainer, const Primitive2DContainer& rCandidate);

        public:
            SdrTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr);

            // get data
            const SdrText* getSdrText() const;
            const OutlinerParaObject& getOutlinerParaObject() const { return maOutlinerParaObject; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // own get2DDecomposition to take aspect of decomposition with or without spell checker
            // into account
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const = 0;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrContourTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            // unit contour polygon (scaled to [0.0 .. 1.0])
            basegfx::B2DPolyPolygon             maUnitPolyPolygon;

            // complete contour polygon transform (scale, rotate, shear, translate)
            basegfx::B2DHomMatrix               maObjectTransform;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrPathTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            // the path to use. Each paragraph will use one Polygon.
            basegfx::B2DPolyPolygon             maPathPolyPolygon;

            // the Fontwork parameters
            attribute::SdrFormTextAttribute     maSdrFormTextAttribute;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrBlockTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            // text range transformation from unit range ([0.0 .. 1.0]) to text range
            basegfx::B2DHomMatrix                   maTextRangeTransform;

            // text alignments
            SdrTextHorzAdjust                       maSdrTextHorzAdjust;
            SdrTextVertAdjust                       maSdrTextVertAdjust;

            bool                                    mbFixedCellHeight : 1;
            bool                                    mbUnlimitedPage : 1;    // force layout with no text break
            bool                                    mbCellText : 1;         // this is a cell text as block text
            bool                                    mbWordWrap : 1;         // for CustomShapes text layout

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

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
                bool bWordWrap);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            SdrTextHorzAdjust getSdrTextHorzAdjust() const { return maSdrTextHorzAdjust; }
            SdrTextVertAdjust getSdrTextVertAdjust() const { return maSdrTextVertAdjust; }
            bool isFixedCellHeight() const { return mbFixedCellHeight; }
            bool getUnlimitedPage() const { return mbUnlimitedPage; }
            bool getCellText() const { return mbCellText; }
            bool getWordWrap() const { return mbWordWrap; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrStretchTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            // text range transformation from unit range ([0.0 .. 1.0]) to text range
            basegfx::B2DHomMatrix                   maTextRangeTransform;

            bool                                    mbFixedCellHeight : 1;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrAutoFitTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            ::basegfx::B2DHomMatrix                 maTextRangeTransform;   // text range transformation from unit range ([0.0 .. 1.0]) to text range

            bool                                    mbWordWrap : 1;         // for CustomShapes text layout

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d

namespace drawinglayer::primitive2d
    {
        class SdrChainedTextPrimitive2D final : public SdrTextPrimitive2D
        {
        private:
            // XXX: might have position of overflowing text

            ::basegfx::B2DHomMatrix maTextRangeTransform;   // text range transformation from unit range ([0.0 .. 1.0]) to text range

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrChainedTextPrimitive2D(
                const SdrText* pSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtrs,
                const ::basegfx::B2DHomMatrix& rTextRangeTransform);

            // get data
            const basegfx::B2DHomMatrix& getTextRangeTransform() const { return maTextRangeTransform; }
            //bool getWordWrap() const { return true; } // XXX: Hack! Should have a proper implementation//

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // transformed clone operator
            virtual rtl::Reference<SdrTextPrimitive2D> createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRTEXTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
