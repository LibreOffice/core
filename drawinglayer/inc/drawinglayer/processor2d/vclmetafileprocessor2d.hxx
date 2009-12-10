/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclmetafileprocessor2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX

#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////
// vcl::PDFExtOutDevData support

#include <vcl/pdfextoutdevdata.hxx>

//////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class VclMetafileProcessor2D : public VclProcessor2D
        {
        private:
            // local helper(s)
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

            // the current clipping PolyPolygon from MaskPrimitive2D
            basegfx::B2DPolyPolygon             maClipPolyPolygon;

            // the target MetaFile
            GDIMetaFile*                        mpMetaFile;

            // do not allow embedding SvtGraphicFills into each other,
            // use a counter to prevent that
            sal_uInt32                          mnSvtGraphicFillCount;

            // same for SvtGraphicStroke
            sal_uInt32                          mnSvtGraphicStrokeCount;

            // hold the last unified transparence value to have it handy
            // on SvtGraphicStroke creation
            double                              mfCurrentUnifiedTransparence;

            // break iterator support
            // made static so it only needs to be fetched once, even with many single
            // constructed VclMetafileProcessor2D. It's still incarnated on demand,
            // but exists for OOo runtime now by purpose.
            static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >   mxBreakIterator;

            // vcl::PDFExtOutDevData support
            // For the first step, some extra actions at vcl::PDFExtOutDevData need to
            // be emulated with the VclMetafileProcessor2D. These are potentially temporarily
            // since PDF export may use PrimitiveSequences one day directly.
            vcl::PDFExtOutDevData*              mpPDFExtOutDevData;

        protected:
            // the local processor for BasePrinitive2D-Implementation based primitives,
            // called from the common process()-implementation
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            // constructor/destructor
            VclMetafileProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclMetafileProcessor2D();
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLMETAFILEPROCESSOR2D_HXX

// eof
