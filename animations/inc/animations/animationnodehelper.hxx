/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationnodehelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:45:47 $
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

#ifndef INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX
#define INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

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
