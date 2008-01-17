/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidetransition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:47 $
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


#ifndef OOX_PPT_SLIDETRANSITION
#define OOX_PPT_SLIDETRANSITION

#include <rtl/ustring.hxx>

#include <com/sun/star/animations/XTransitionFilter.hpp>

namespace oox { class PropertyMap; }

namespace oox { namespace ppt {

    class SlideTransition
    {
    public:
        SlideTransition();
        explicit SlideTransition(const ::rtl::OUString & );

        void setSlideProperties( PropertyMap& props );
        void setTransitionFilterProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTransitionFilter > & xFilter );

        void setOoxTransitionSpeed( sal_Int32 nToken );
        void setFadeColor( sal_Int32 nColor )
            { mnFadeColor = nColor; }
        void setMode( sal_Bool bMode )
            { mbMode = bMode; }

    static sal_Int16 ooxToOdpDirection( ::sal_Int32 nOoxType );
    static sal_Int16 ooxToOdpEightDirections( ::sal_Int32 nOoxType );
    static sal_Int16 ooxToOdpCornerDirections( ::sal_Int32 nOoxType );
    static sal_Int16 ooxToOdpBorderDirections( ::sal_Int32 nOoxType );

        void setOoxTransitionType( ::sal_Int32 OoxType,
                                                             ::sal_Int32 param1, ::sal_Int32 param2 );
    private:
        ::sal_Int16 mnTransitionType;
        ::sal_Int16 mnTransitionSubType;
        ::sal_Bool  mbTransitionDirectionNormal;
        ::sal_Int16 mnAnimationSpeed;
        ::sal_Int32 mnFadeColor;
        ::sal_Bool  mbMode; /**< http://api.openoffice.org/docs/common/ref/com/sun/star/animations/XTransitionFilter.html Mode property */
    };

} }

#endif
