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
class Rectangle;
class Gradient;
class SvtGraphicFill;
class SvtGraphicStroke;

namespace drawinglayer { namespace attribute {
    class FillGradientAttribute;
    class LineAttribute;
    class StrokeAttribute;
    class LineStartEndAttribute;
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
            Rectangle impDumpToMetaFile(
                const primitive2d::Primitive2DSequence& rContent,
                GDIMetaFile& o_rContentMetafile);
            void impConvertFillGradientAttributeToVCLGradient(
                Gradient& o_rVCLGradient,
                const attribute::FillGradientAttribute& rFiGrAtt,
                bool bIsTransparenceGradient);
            void impStartSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill);
            void impEndSvtGraphicFill(SvtGraphicFill* pSvtGraphicFill);
            SvtGraphicStroke* impTryToCreateSvtGraphicStroke(
                const basegfx::B2DPolygon& rB2DPolygon,
                const basegfx::BColor* pColor,
                const attribute::LineAttribute* pLineAttribute,
                const attribute::StrokeAttribute* pStrokeAttribute,
                const attribute::LineStartEndAttribute* pStart,
                const attribute::LineStartEndAttribute* pEnd);
            void impStartSvtGraphicStroke(SvtGraphicStroke* pSvtGraphicStroke);
            void impEndSvtGraphicStroke(SvtGraphicStroke* pSvtGraphicStroke);

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

        protected:
            /*  the local processor for BasePrinitive2D-Implementation based primitives,
                called from the common process()-implementation
             */
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

        public:
            /// constructor/destructor
            VclMetafileProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclMetafileProcessor2D();
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer



#endif // INCLUDED_DRAWINGLAYER_SOURCE_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
