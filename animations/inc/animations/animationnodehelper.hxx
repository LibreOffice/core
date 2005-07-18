/*************************************************************************
 *
 *  $RCSfile: animationnodehelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-18 14:35:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
