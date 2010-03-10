/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
