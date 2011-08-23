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


#ifndef OOX_PPT_CONDITIONCONTEXT
#define OOX_PPT_CONDITIONCONTEXT

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/animations/Event.hpp>

#include "oox/core/fragmenthandler.hxx"
#include "oox/ppt/timenode.hxx"
#include "oox/ppt/timenodelistcontext.hxx"
#include "oox/ppt/animationspersist.hxx"

namespace oox { namespace ppt {


    /** CT_TLTimeCondition */
    class CondContext
        : public TimeNodeContext
    {
    public:
        CondContext( ::oox::core::ContextHandler& rParent,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
                     const TimeNodePtr & pNode, AnimationCondition & aCond );
        ~CondContext( ) throw( );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    private:
//		::com::sun::star::uno::Any &         maCond;
        ::com::sun::star::animations::Event  maEvent;
//		AnimTargetElementPtr         mpTarget;
        AnimationCondition &                 maCond;
    };



    /** CT_TLTimeConditionList */
    class CondListContext
        : public TimeNodeContext
    {
    public:
        CondListContext( ::oox::core::ContextHandler& rParent,
             sal_Int32  aElement,
             const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
             const TimeNodePtr & pNode, AnimationConditionList & aCondList );
        ~CondListContext( ) throw( );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& /*xAttribs*/ ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    private:
        AnimationConditionList     & maConditions;
    };


} }


#endif
