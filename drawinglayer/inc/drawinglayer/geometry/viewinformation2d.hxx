/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:41:57 $
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
#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace {
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
        class ViewInformation2D
        {
        private:
            // impl pointer
            ImpViewInformation2D*                   mpViewInformation2D;

        public:
            // constructor to easily build a ViewInformation2D when all view
            // information is available as basegfx implementation classes. The
            // ExtendedParameters allows adding extra parameters besides ViewTransformation,
            // Viewport and ViewTime and should not itself include one of these.
            ViewInformation2D(
                const basegfx::B2DHomMatrix& rViewTransformation,
                const basegfx::B2DRange& rViewport,
                const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                double fViewTime,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rExtendedParameters);

            // constructor to create a ViewInformation2D based on API information only. The
            // ViewParameters can contain ViewTransformation, Viewport and ViewTime but also
            // other parameters which will be preserved in the ExtendedInformation. The three
            // named information will be extracted locally for faster access.
            ViewInformation2D(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters);

            // copy constructor
            ViewInformation2D(const ViewInformation2D& rCandidate);

            // destructor
            ~ViewInformation2D();

            // assignment operator
            ViewInformation2D& operator=(const ViewInformation2D& rCandidate);

            // data access
            const basegfx::B2DHomMatrix& getViewTransformation() const;
            const basegfx::B2DRange& getViewport() const;
            double getViewTime() const;

            // get the DrawPage which is visualized. Tjhis is needed e.g. for the
            // view-dependent decomposition of PageNumber Fields in Texts
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& ViewInformation2D::getVisualizedPage() const;

            // data access with on-demand preparations
            const basegfx::B2DHomMatrix& getInverseViewTransformation() const;
            const basegfx::B2DRange& getDiscreteViewport() const;

            // get the uno::Sequence< beans::PropertyValue > which contains all information. When
            // constructed using the API constructor, You will get back Your input. If not, the
            // needed sequence will be constructed including the extended informations.
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getViewInformationSequence() const;

            // get the uno::Sequence< beans::PropertyValue > which contains only extra information. This means
            // information different from ViewTransformation, Viewport and ViewTime.
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getExtendedInformationSequence() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

// eof
