/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoview.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:05:47 $
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

#ifndef _SLIDESHOW_UNOVIEW_HXX
#define _SLIDESHOW_UNOVIEW_HXX

#include "view.hxx"

#include <vector>

namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }


/* Definition of UnoView interface */

namespace slideshow
{
    namespace internal
    {
        /** Extend View with UNO interface retrieval.

            This interface extends View with an UNO interface
            retrieval, to be used for Views which are set from
            external API.
         */
        class UnoView : public View
        {
        public:
            /** Retrieve the underlying UNO slide view.
             */
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowView > getUnoView() const = 0;

            /** Dispose view

                This needs to be different from Disposable interface,
                as the UNO XComponent also provides a dispose() (only
                with a different calling convention under Windows).
             */
            virtual void _dispose() = 0;
        };

        typedef ::boost::shared_ptr< UnoView >      UnoViewSharedPtr;
        typedef ::std::vector< UnoViewSharedPtr >   UnoViewVector;
    }
}

#endif /* _SLIDESHOW_UNOVIEW_HXX */
