/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationnodefactory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:02:58 $
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

#ifndef INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX

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


#if defined(VERBOSE) && defined(DBG_UTIL)
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

#endif /* INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX */
