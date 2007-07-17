/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: view.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:18:59 $
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

#ifndef INCLUDED_SLIDESHOW_VIEW_HXX
#define INCLUDED_SLIDESHOW_VIEW_HXX

#include "viewlayer.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>


namespace basegfx { class B2DRange; class B2DVector; }


/* Definition of View interface */

namespace slideshow
{
    namespace internal
    {
        class View : public ViewLayer
        {
        public:
            /** Create a new view layer for this view

                @param rLayerBounds
                Specifies the bound rect of the layer relative to the
                user view coordinate system.

                This method sets the bounds of the view layer in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
             */
            virtual ViewLayerSharedPtr createViewLayer( const basegfx::B2DRange& rLayerBounds ) const = 0;

            /** Update screen representation from backbuffer
             */
            virtual bool updateScreen() const = 0;

            /** Paint screen content unconditionally from backbuffer
             */
            virtual bool paintScreen() const = 0;

            /** Set the size of the user view coordinate system.

                This method sets the width and height of the view in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
            */
            virtual void setViewSize( const ::basegfx::B2DVector& ) = 0;

            /** Change the view's mouse cursor.

                @param nPointerShape
                One of the ::com::sun::star::awt::SystemPointer
                constant group members.
             */
            virtual void setCursorShape( sal_Int16 nPointerShape ) = 0;
        };

        typedef ::boost::shared_ptr< View >     ViewSharedPtr;
        typedef ::std::vector< ViewSharedPtr >  ViewVector;
    }
}

#endif /* INCLUDED_SLIDESHOW_VIEW_HXX */
