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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include "vclprocessor2d.hxx"
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>


// vcl::PDFExtOutDevData support

#include <vcl/pdfextoutdevdata.hxx>


// predefines

class GDIMetaFile;
namespace tools { class Rectangle; }
class Gradient;
class SvtGraphicFill;
class SvtGraphicStroke;

namespace drawinglayer { namespace attribute {
    class FillGradientAttribute;
    class LineAttribute;
    class StrokeAttribute;
    class LineStartEndAttribute;
}}

namespace drawinglayer { namespace primitive2d {
    class GraphicPrimitive2D;
    class ControlPrimitive2D;
    class TextHierarchyFieldPrimitive2D;
    class TextHierarchyLinePrimitive2D;
    class TextHierarchyBulletPrimitive2D;
    class TextHierarchyParagraphPrimitive2D;
    class TextHierarchyBlockPrimitive2D;
    class TextSimplePortionPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class PolygonStrokePrimitive2D;
    class PolygonStrokeArrowPrimitive2D;
    class PolyPolygonGraphicPrimitive2D;
    class PolyPolygonHatchPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonColorPrimitive2D;
    class MaskPrimitive2D;
    class UnifiedTransparencePrimitive2D;
    class TransparencePrimitive2D;
    class StructureTagPrimitive2D;
}}

namespace basegfx {
    class BColor;
}


namespace drawinglayer
{
    namespace processor2d
    {
        /** VclMetafileProcessor2D class

            This processor derived from VclProcessor2D is the base class for rendering
            all feeded primitives to a classical VCL-Metafile, including all over the
            time grown extra data in comments and PDF exception data creations. Also
            printing needs some exception stuff.

            All in all it is needed to emulate the old ::paint output from the old
            Drawinglayer as long as exporters and/or filters still use the Metafile
            and the extra-data added to it (which can be seen mostly as 'extensions'
            or simply as 'hacks').
         */
        class VclMetafileProcessor2D : public VclProcessor2D
        {
        private:
            /// local helper(s)
            ::tools::Rectangle impDumpToMetaFile(
                const primitive2d::Primitive2DContainer& rContent,
                GDIMetaFile& o_rContentMetafile);
            void impConvertFillGradientAttributeToVCLGradient(
                Gradient& o_rVCLGradient,
                const attribute::FillGradientAttribute& rFiGrAtt,
                bool bIsTransparenceGradient) const;
            void impStartSvtGraphicFill(SvtGraphicFill const * pSvtGraphicFill);
            void impEndSvtGraphicFill(SvtGraphicFill const * pSvtGraphicFill);
            std::unique_ptr<SvtGraphicStroke> impTryToCreateSvtGraphicStroke(
                const basegfx::B2DPolygon& rB2DPolygon,
                const basegfx::BColor* pColor,
                const attribute::LineAttribute* pLineAttribute,
                const attribute::StrokeAttribute* pStrokeAttribute,
                const attribute::LineStartEndAttribute* pStart,
                const attribute::LineStartEndAttribute* pEnd);
            void impStartSvtGraphicStroke(SvtGraphicStroke const * pSvtGraphicStroke);
            void impEndSvtGraphicStroke(SvtGraphicStroke const * pSvtGraphicStroke);

            void processGraphicPrimitive2D(const primitive2d::GraphicPrimitive2D& rGraphicPrimitive);
            void processControlPrimitive2D(const primitive2d::ControlPrimitive2D& rControlPrimitive);
            void processTextHierarchyFieldPrimitive2D(const primitive2d::TextHierarchyFieldPrimitive2D& rFieldPrimitive);
            void processTextHierarchyLinePrimitive2D(const primitive2d::TextHierarchyLinePrimitive2D& rLinePrimitive);
            void processTextHierarchyBulletPrimitive2D(const primitive2d::TextHierarchyBulletPrimitive2D& rBulletPrimitive);
            void processTextHierarchyParagraphPrimitive2D(const primitive2d::TextHierarchyParagraphPrimitive2D& rParagraphPrimitive);
            void processTextHierarchyBlockPrimitive2D(const primitive2d::TextHierarchyBlockPrimitive2D& rBlockPrimitive);
            void processTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate);
            void processPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rHairlinePrimitive);
            void processPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rStrokePrimitive);
            void processPolygonStrokeArrowPrimitive2D(const primitive2d::PolygonStrokeArrowPrimitive2D& rStrokeArrowPrimitive);
            void processPolyPolygonGraphicPrimitive2D(const primitive2d::PolyPolygonGraphicPrimitive2D& rBitmapCandidate);
            void processPolyPolygonHatchPrimitive2D(const primitive2d::PolyPolygonHatchPrimitive2D& rHatchCandidate);
            void processPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rGradientCandidate);
            void processPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate);
            void processMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate);
            void processUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate);
            void processTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransparenceCandidate);
            void processStructureTagPrimitive2D(const primitive2d::StructureTagPrimitive2D& rStructureTagCandidate);

            /// Convert the fWidth to the same space as its coordinates.
            double getTransformedLineWidth( double fWidth ) const;

            /// the current clipping tools::PolyPolygon from MaskPrimitive2D
            basegfx::B2DPolyPolygon             maClipPolyPolygon;

            /// the target MetaFile
            GDIMetaFile*                        mpMetaFile;

            /*  do not allow embedding SvtGraphicFills into each other,
                use a counter to prevent that
             */
            sal_uInt32                          mnSvtGraphicFillCount;

            /// same for SvtGraphicStroke
            sal_uInt32                          mnSvtGraphicStrokeCount;

            /*  hold the last unified transparence value to have it handy
                on SvtGraphicStroke creation
             */
            double                              mfCurrentUnifiedTransparence;

            /*  break iterator support
                made static so it only needs to be fetched once, even with many single
                constructed VclMetafileProcessor2D. It's still incarnated on demand,
                but exists for OOo runtime now by purpose.
             */
            static css::uno::Reference< css::i18n::XBreakIterator >   mxBreakIterator;

            /*  vcl::PDFExtOutDevData support
                For the first step, some extra actions at vcl::PDFExtOutDevData need to
                be emulated with the VclMetafileProcessor2D. These are potentially temporarily
                since PDF export may use PrimitiveSequences one day directly.
             */
            vcl::PDFExtOutDevData*              mpPDFExtOutDevData;

            // Remember the current OutlineLevel. This is used when tagged PDF export
            // is used to create/write valid structued list entries using PDF statements
            // like '/L', '/LI', 'LBody' instead of simple '/P' (Paragraph).
            // The value -1 means 'no OutlineLevel' and values >= 0 express the level.
            sal_Int16                           mnCurrentOutlineLevel;
            bool mbInListItem;
            bool mbBulletPresent;

        protected:
            /*  the local processor for BasePrimitive2D-Implementation based primitives,
                called from the common process()-implementation
             */
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

        public:
            /// constructor/destructor
            VclMetafileProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclMetafileProcessor2D() override;
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer


#endif // INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
