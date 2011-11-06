/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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
    static sal_Int16 ooxToOdpSideDirections( ::sal_Int32 nOoxType );
    static sal_Bool  ooxToOdpSideDirectionsDirectionNormal( ::sal_Int32 nOoxType );

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
