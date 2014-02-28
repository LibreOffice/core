/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
