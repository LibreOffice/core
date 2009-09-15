/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:31:09 $
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

#include <drawinglayer/processor3d/baseprocessor3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        void BaseProcessor3D::processBasePrimitive3D(const primitive3d::BasePrimitive3D& /*rCandidate*/)
        {
        }

        BaseProcessor3D::BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation)
        :   maViewInformation3D(rViewInformation)
        {
        }

        BaseProcessor3D::~BaseProcessor3D()
        {
        }

        void BaseProcessor3D::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BufDecPrimitive3D implementation
                        const primitive3d::BufDecPrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BufDecPrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            processBasePrimitive3D(*pBasePrimitive);
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue >& rViewParameters(getViewInformation3D().getViewInformationSequence());
                            process(xReference->getDecomposition(rViewParameters));
                        }
                    }
                }
            }
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        CollectingProcessor3D::CollectingProcessor3D(const geometry::ViewInformation3D& rViewInformation)
        :   BaseProcessor3D(rViewInformation),
            maPrimitive3DSequence()
        {
        }

        CollectingProcessor3D::~CollectingProcessor3D()
        {
        }

        void CollectingProcessor3D::process(const primitive3d::Primitive3DSequence& rSource)
        {
            // accept everything
            primitive3d::appendPrimitive3DSequenceToPrimitive3DSequence(maPrimitive3DSequence, rSource);
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
