/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation2d.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#define INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

// the solaris compiler defines 'sun' as '1'. To avoid that (and to allow
// pre-declarations of com/sun/star namespace), include sal/config.h here
// where sun is redefined as 'sun' (so i guess the problem is known).
#include <sal/config.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace geometry {
    class ImpViewInformation2D;
}}

namespace basegfx {
    class B2DHomMatrix;
    class B2DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        /** ViewInformation2D class

            This class holds all view-relevant information for a 2d geometry. It works
            together with UNO API definitions and supports holding a sequence of PropertyValues.
            The most used data is for convenience offered directly using basegfx tooling classes.
            It is an implementation to support the sequence of PropertyValues used in a
            ::com::sun::star::graphic::XPrimitive2D for C++ implementations working with those
        */
        class ViewInformation2D
        {
        private:
            /// pointer to private implementation class
            ImpViewInformation2D*                   mpViewInformation2D;

        public:
            /** Constructor: Create a ViewInformation2D

                @param rObjectTransformation
                The Transformation from Object to World coordinates (normally logic coordinates).

                @param rViewTransformation
                The Transformation from World to View coordinates (normally logic coordinates
                to discrete units, e.g. pixels).

                @param rViewport
                The visible part of the view in World coordinates. If empty (getViewport().isEmpty())
                everything is visible. The data is in World coordinates.

                @param rxDrawPage
                The currently displaqyed page. This information is needed e.g. due to existing PageNumber
                fields which need to be interpreted.

                @param fViewTime
                The time the view is defined for. Default is 0.0. This parameter is used e.g. for
                animated objects

                @param rExtendedParameters
                A sequence of property values which allows holding various other parameters besides
                the obvious and needed ones above. For this constructor none of the other parameters
                should be added as data. The constructor will parse the given parameters and if
                data for the other parameters is given, the value in rExtendedParameters will
                be preferred and overwrite the given parameter
            */
            ViewInformation2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const basegfx::B2DHomMatrix& rViewTransformation,
                const basegfx::B2DRange& rViewport,
                const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                double fViewTime,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rExtendedParameters);

            /** Constructor: Create a ViewInformation2D

                @param rViewParameters
                A sequence of property values which allows holding any combination of local and various
                other parameters. This constructor is feeded completely with a sequence of PropertyValues
                which will be parsed to be able to offer the most used ones in a convenient way.
            */
            ViewInformation2D(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters);

            /// copy constructor
            ViewInformation2D(const ViewInformation2D& rCandidate);

            /// destructor
            ~ViewInformation2D();

            /// assignment operator
            ViewInformation2D& operator=(const ViewInformation2D& rCandidate);

            /// compare operators
            bool operator==(const ViewInformation2D& rCandidate) const;
            bool operator!=(const ViewInformation2D& rCandidate) const { return !operator==(rCandidate); }

            /// data access
            const basegfx::B2DHomMatrix& getObjectTransformation() const;
            const basegfx::B2DHomMatrix& getViewTransformation() const;
            const basegfx::B2DRange& getViewport() const;
            double getViewTime() const;
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& getVisualizedPage() const;

            /// On-demand prepared Object to View transformation and it's inerse for convenience
            const basegfx::B2DHomMatrix& getObjectToViewTransformation() const;
            const basegfx::B2DHomMatrix& getInverseObjectToViewTransformation() const;

            /// On-demand prepared Viewport in discrete units for convenience
            const basegfx::B2DRange& getDiscreteViewport() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains all ViewInformation

                Use this call if You need to extract all contained ViewInformation. The ones
                directly supported for convenience will be added to the ones only available
                as PropertyValues. This set completely describes this ViewInformation2D and
                can be used for complete information transport over UNO API.
            */
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getViewInformationSequence() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains only ViewInformation
                not offered directly

                Use this call if You only need ViewInformation which is not offered conveniently,
                but only exists as PropertyValue. This is e.g. used to create partially updated
                incarnations of ViewInformation2D without losing the only with PropertyValues
                defined data. It does not contain a complete description.
            */
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getExtendedInformationSequence() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

// eof
