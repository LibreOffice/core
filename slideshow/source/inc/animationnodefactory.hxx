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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODEFACTORY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODEFACTORY_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>

#include "subsettableshapemanager.hxx"
#include "animationnode.hxx"
#include "slideshowcontext.hxx"
#include "eventqueue.hxx"
#include "activitiesqueue.hxx"
#include "usereventqueue.hxx"

#include <boost/noncopyable.hpp>
#include <vector>


namespace slideshow
{
    namespace internal
    {
        /* Definition of AnimationNodeFactory class */

        class AnimationNodeFactory : private boost::noncopyable
        {
        public:
            /** Create an AnimatioNode for the given XAnimationNode
             */
            static AnimationNodeSharedPtr createAnimationNode( const ::com::sun::star::uno::Reference<
                                                                       ::com::sun::star::animations::XAnimationNode >& xNode,
                                                               const ::basegfx::B2DVector&                          rSlideSize,
                                                               const SlideShowContext&                              rContext );


#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
            static void showTree( AnimationNodeSharedPtr& pRootNode );
# define SHOW_NODE_TREE(a) AnimationNodeFactory::showTree(a)
#else
# define SHOW_NODE_TREE(a)
#endif

        private:
            // default: constructor/destructor disabled
            AnimationNodeFactory();
            ~AnimationNodeFactory();
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
