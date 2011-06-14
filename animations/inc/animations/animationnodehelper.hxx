/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX
#define INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#include <vector>

/* Declaration and definition of AnimationNode helper */

namespace anim
{
    // TODO(Q1): this could possibly be implemented with a somewhat
    // more lightweight template, by having the actual worker receive
    // only a function pointer, and a thin templated wrapper around
    // that which converts member functions into that.

    /** Apply given functor to every animation node child.

        @param xNode
        Parent node

        @param rFunctor
        Functor to apply. The functor must have an appropriate
        operator()( const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode >& ) member.

        @return true, if the functor was successfully applied to
        all children, false otherwise.
    */
    template< typename Functor > inline bool for_each_childNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >&    xNode,
                                                          Functor&                                                                                  rFunctor )
    {
        try
        {
            // get an XEnumerationAccess to the children
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
                  xEnumerationAccess( xNode,
                                      ::com::sun::star::uno::UNO_QUERY_THROW );
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                  xEnumeration( xEnumerationAccess->createEnumeration(),
                                ::com::sun::star::uno::UNO_QUERY_THROW );

            while( xEnumeration->hasMoreElements() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >
                      xChildNode( xEnumeration->nextElement(),
                                  ::com::sun::star::uno::UNO_QUERY_THROW );

                rFunctor( xChildNode );
            }

            return true;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            return false;
        }
    }


    /** pushes the given node to the given vector and recursivly calls itself for each child node.
    */
    inline void create_deep_vector( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                                std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > >& rVector )
    {
        rVector.push_back( xNode );

        try
        {
            // get an XEnumerationAccess to the children
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
                  xEnumerationAccess( xNode,
                                      ::com::sun::star::uno::UNO_QUERY );

            if( xEnumerationAccess.is() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                      xEnumeration( xEnumerationAccess->createEnumeration(),
                                    ::com::sun::star::uno::UNO_QUERY );

                if( xEnumeration.is() )
                {
                    while( xEnumeration->hasMoreElements() )
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >
                            xChildNode( xEnumeration->nextElement(),
                                        ::com::sun::star::uno::UNO_QUERY_THROW );

                        create_deep_vector( xChildNode, rVector );
                    }
                }
            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }
}

#endif /* INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
