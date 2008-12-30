/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextprimitive2d.cxx,v $
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

#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/color/bcolor.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <svx/outlobj.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <unoapi.hxx>
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
        // for slideshow. This uses TextHierarchyBlockPrimitive2D to mark a text block.
        // ATM there is only one text block per SdrObject, this may get more in the future
        Primitive2DSequence SdrTextPrimitive2D::encapsulateWithTextHierarchyBlockPrimitive2D(const Primitive2DSequence& rCandidate) const
        {
            Primitive2DReference xReference(new TextHierarchyBlockPrimitive2D(rCandidate));
            Primitive2DSequence xRetval(&xReference, 1);

            return xRetval;
        }

        SdrTextPrimitive2D::SdrTextPrimitive2D(const SdrText& rSdrText)
        :   BasePrimitive2D(),
            mrSdrText(rSdrText),
            mxLastVisualizingPage(),
            mbLastSpellCheck(false),
            mbContainsPageField(false)
        {
            if(mrSdrText.GetOutlinerParaObject())
            {
                const EditTextObject& rETO = mrSdrText.GetOutlinerParaObject()->GetTextObject();
                mbContainsPageField = rETO.HasField(SvxPageField::StaticType())
                    || rETO.HasField(SvxHeaderField::StaticType())
                    || rETO.HasField(SvxFooterField::StaticType())
                    || rETO.HasField(SvxDateTimeField::StaticType());
            }
        }

        bool SdrTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                // since OutlinerParaObject has no compare operator, i do not clone it for this class and
                // use the address in the SdrText for comparison if it did change
                const SdrTextPrimitive2D& rCompare = (SdrTextPrimitive2D&)rPrimitive;
                const bool bOutlinerParaSameAddress(getSdrText().GetOutlinerParaObject() == rCompare.getSdrText().GetOutlinerParaObject());

                return bOutlinerParaSameAddress;
            }

            return false;
        }

        Primitive2DSequence SdrTextPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            const bool bCurrentSpellCheck(getSdrText().GetObject().impCheckSpellCheckForDecomposeTextPrimitive());
            uno::Reference< drawing::XDrawPage > xCurrentlyVisualizingPage;

            if(getLocalDecomposition().hasElements())
            {
                bool bDoDelete(getLastSpellCheck() != bCurrentSpellCheck);

                if(!bDoDelete && mbContainsPageField)
                {
                    xCurrentlyVisualizingPage = rViewInformation.getVisualizedPage();

                    if(xCurrentlyVisualizingPage != mxLastVisualizingPage)
                    {
                        bDoDelete = true;
                    }
                }

                if(bDoDelete)
                {
                    const_cast< SdrTextPrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
                }
            }

            if(!getLocalDecomposition().hasElements())
            {
                const_cast< SdrTextPrimitive2D* >(this)->setLastSpellCheck(bCurrentSpellCheck);
                const_cast< SdrTextPrimitive2D* >(this)->mxLastVisualizingPage = xCurrentlyVisualizingPage;
            }

            // call parent
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrContourTextPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            const bool bCurrentSpellCheck(getSdrText().GetObject().impDecomposeContourTextPrimitive(aRetval, *this, aViewInformation));

            if(getLastSpellCheck() != bCurrentSpellCheck)
            {
                // remember last spell check state; this is part of the decomposition source data definition
                const_cast< SdrContourTextPrimitive2D* >(this)->setLastSpellCheck(bCurrentSpellCheck);
            }

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
        }

        SdrContourTextPrimitive2D::SdrContourTextPrimitive2D(
            const SdrText& rSdrText,
            const ::basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const ::basegfx::B2DHomMatrix& rObjectTransform)
        :   SdrTextPrimitive2D(rSdrText),
            maUnitPolyPolygon(rUnitPolyPolygon),
            maObjectTransform(rObjectTransform)
        {
        }

        bool SdrContourTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrContourTextPrimitive2D& rCompare = (SdrContourTextPrimitive2D&)rPrimitive;

                return (maUnitPolyPolygon == rCompare.maUnitPolyPolygon
                    && maObjectTransform == rCompare.maObjectTransform);
            }

            return false;
        }

        SdrTextPrimitive2D* SdrContourTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrContourTextPrimitive2D(getSdrText(), maUnitPolyPolygon, rTransform * maObjectTransform);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrContourTextPrimitive2D, PRIMITIVE2D_ID_SDRCONTOURTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrPathTextPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            const bool bCurrentSpellCheck(getSdrText().GetObject().impDecomposePathTextPrimitive(aRetval, *this, aViewInformation));

            if(getLastSpellCheck() != bCurrentSpellCheck)
            {
                // remember last spell check state; this is part of the decomposition source data definition
                const_cast< SdrPathTextPrimitive2D* >(this)->setLastSpellCheck(bCurrentSpellCheck);
            }

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
        }

        SdrPathTextPrimitive2D::SdrPathTextPrimitive2D(
            const SdrText& rSdrText,
            const ::basegfx::B2DPolyPolygon& rPathPolyPolygon)
        :   SdrTextPrimitive2D(rSdrText),
            maPathPolyPolygon(rPathPolyPolygon)
        {
        }

        bool SdrPathTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrPathTextPrimitive2D& rCompare = (SdrPathTextPrimitive2D&)rPrimitive;

                return (maPathPolyPolygon == rCompare.maPathPolyPolygon);
            }

            return false;
        }

        SdrTextPrimitive2D* SdrPathTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
        {
            ::basegfx::B2DPolyPolygon aNewPolyPolygon(maPathPolyPolygon);
            aNewPolyPolygon.transform(rTransform);
            return new SdrPathTextPrimitive2D(getSdrText(), aNewPolyPolygon);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrPathTextPrimitive2D, PRIMITIVE2D_ID_SDRPATHTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrBlockTextPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            const bool bCurrentSpellCheck(getSdrText().GetObject().impDecomposeBlockTextPrimitive(aRetval, *this, aViewInformation));

            if(getLastSpellCheck() != bCurrentSpellCheck)
            {
                // remember last spell check state; this is part of the decomposition source data definition
                const_cast< SdrBlockTextPrimitive2D* >(this)->setLastSpellCheck(bCurrentSpellCheck);
            }

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
        }

        SdrBlockTextPrimitive2D::SdrBlockTextPrimitive2D(
            const SdrText& rSdrText,
            const ::basegfx::B2DHomMatrix& rTextRangeTransform,
            bool bUnlimitedPage,
            bool bCellText,
            bool bWordWrap)
        :   SdrTextPrimitive2D(rSdrText),
            maTextRangeTransform(rTextRangeTransform),
            mbUnlimitedPage(bUnlimitedPage),
            mbCellText(bCellText),
            mbWordWrap(bWordWrap)
        {
        }

        bool SdrBlockTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrBlockTextPrimitive2D& rCompare = (SdrBlockTextPrimitive2D&)rPrimitive;

                return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                    && getUnlimitedPage() == rCompare.getUnlimitedPage()
                    && getCellText() == rCompare.getCellText()
                    && getWordWrap() == rCompare.getWordWrap());
            }

            return false;
        }

        SdrTextPrimitive2D* SdrBlockTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrBlockTextPrimitive2D(getSdrText(), rTransform * getTextRangeTransform(), getUnlimitedPage(), getCellText(), getWordWrap());
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrBlockTextPrimitive2D, PRIMITIVE2D_ID_SDRBLOCKTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrStretchTextPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            const bool bCurrentSpellCheck(getSdrText().GetObject().impDecomposeStretchTextPrimitive(aRetval, *this, aViewInformation));

            if(getLastSpellCheck() != bCurrentSpellCheck)
            {
                // remember last spell check state; this is part of the decomposition source data definition
                const_cast< SdrStretchTextPrimitive2D* >(this)->setLastSpellCheck(bCurrentSpellCheck);
            }

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
        }

        SdrStretchTextPrimitive2D::SdrStretchTextPrimitive2D(
            const SdrText& rSdrText,
            const ::basegfx::B2DHomMatrix& rTextRangeTransform)
        :   SdrTextPrimitive2D(rSdrText),
            maTextRangeTransform(rTextRangeTransform)
        {
        }

        bool SdrStretchTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrStretchTextPrimitive2D& rCompare = (SdrStretchTextPrimitive2D&)rPrimitive;

                return (maTextRangeTransform == rCompare.maTextRangeTransform);
            }

            return false;
        }

        SdrTextPrimitive2D* SdrStretchTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrStretchTextPrimitive2D(getSdrText(), rTransform * maTextRangeTransform);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrStretchTextPrimitive2D, PRIMITIVE2D_ID_SDRSTRETCHTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
