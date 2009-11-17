/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backgroundcolorprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence EpsPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const GDIMetaFile& rSubstituteContent = getMetaFile();

            if(rSubstituteContent.GetActionCount())
            {
                // the default decomposition will use the Metafile replacement visualisation.
                // To really use the Eps data, a renderer has to know and interpret this primitive
                // directly.
                xRetval.realloc(1);

                xRetval[0] = Primitive2DReference(
                    new MetafilePrimitive2D(
                        getEpsTransform(),
                        rSubstituteContent));
            }

            return xRetval;
        }

        EpsPrimitive2D::EpsPrimitive2D(
            const basegfx::B2DHomMatrix& rEpsTransform,
            const GfxLink& rGfxLink,
            const GDIMetaFile& rMetaFile)
        :   BufferedDecompositionPrimitive2D(),
            maEpsTransform(rEpsTransform),
            maGfxLink(rGfxLink),
            maMetaFile(rMetaFile)
        {
        }

        bool EpsPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const EpsPrimitive2D& rCompare = (EpsPrimitive2D&)rPrimitive;

                return (getEpsTransform() == rCompare.getEpsTransform()
                    && getGfxLink().IsEqual(rCompare.getGfxLink())
                    && getMetaFile() == rCompare.getMetaFile());
            }

            return false;
        }

        basegfx::B2DRange EpsPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // use own implementation to quickly answer the getB2DRange question.
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getEpsTransform());

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(EpsPrimitive2D, PRIMITIVE2D_ID_EPSPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
