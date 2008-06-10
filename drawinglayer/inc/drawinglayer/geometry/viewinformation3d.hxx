/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:20 $
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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX
#define INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX

// the solaris compiler defines 'sun' as '1'. To avoid that (and to allow
// pre-declarations of com/sun/star namespace), include sal/config.h here
// where sun is redefined as 'sun' (so i guess the problem is known).
#include <sal/config.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace {
    class ImpViewInformation3D;
}}

namespace basegfx {
    class B3DHomMatrix;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        class ViewInformation3D
        {
        private:
            // impl pointer
            ImpViewInformation3D*                   mpViewInformation3D;

        public:
            // constructor to easily build a ViewInformation3D when all view
            // information is available as basegfx implementation classes. The
            // ExtendedParameters allows adding extra parameters besides the simple ones
            // and should not itself include one of these.
            ViewInformation3D(
                const basegfx::B3DHomMatrix& rTransformation,
                const basegfx::B3DHomMatrix& rOrientation,
                const basegfx::B3DHomMatrix& rProjection,
                const basegfx::B3DHomMatrix& rDeviceToView,
                double fViewTime,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rExtendedParameters);

            // constructor to create a ViewInformation3D based on API information only. The
            // ViewParameters can contain ViewTransformation, Viewport and ViewTime but also
            // other parameters which will be preserved in the ExtendedInformation. The three
            // named information will be extracted locally for faster access.
            ViewInformation3D(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters);

            // copy constructor
            ViewInformation3D(const ViewInformation3D& rCandidate);

            // destructor
            ~ViewInformation3D();

            // assignment operator
            ViewInformation3D& operator=(const ViewInformation3D& rCandidate);

            // compare operator
            bool operator==(const ViewInformation3D& rCandidate) const;

            // data access
            // the four transformations defining the 3D view pipeline complately
            const basegfx::B3DHomMatrix& getTransformation() const;
            const basegfx::B3DHomMatrix& getOrientation() const;
            const basegfx::B3DHomMatrix& getProjection() const;
            const basegfx::B3DHomMatrix& getDeviceToView() const;

            // for convenience, the linear combination of the above four transformations
            const basegfx::B3DHomMatrix& getObjectToView() const;

            // time at which the transformation is to be used. This may be used from animated objects
            double getViewTime() const;

            // get the uno::Sequence< beans::PropertyValue > which contains all information. When
            // constructed using the API constructor, You will get back Your input. If not, the
            // needed sequence will be constructed including the extended informations.
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getViewInformationSequence() const;

            // get the uno::Sequence< beans::PropertyValue > which contains only extra information. This means
            // information different from the four transformations.
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getExtendedInformationSequence() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX

// eof
