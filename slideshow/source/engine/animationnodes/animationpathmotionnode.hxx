/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationpathmotionnode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:41:42 $
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

#ifndef _SLIDESHOW_ANIMATIONPATHMOTIONNODE_HXX
#define _SLIDESHOW_ANIMATIONPATHMOTIONNODE_HXX

#include <activityanimationbasenode.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATEMOTION_HPP_
#include <com/sun/star/animations/XAnimateMotion.hpp>
#endif


namespace presentation
{
    namespace internal
    {
        class AnimationPathMotionNode : public ActivityAnimationBaseNode
        {
        public:
            AnimationPathMotionNode( const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::animations::XAnimationNode >&    xNode,
                                     const BaseContainerNodeSharedPtr&                  rParent,
                                     const NodeContext&                                 rContext );

            virtual void dispose();
            virtual bool init();

#if defined(VERBOSE) && defined(DBG_UTIL)
            virtual const char* getDescription() const;
#endif

        private:
            AnimationActivitySharedPtr createPathMotionActivity();

            ::com::sun::star::uno::Reference<
                ::com::sun::star::animations::XAnimateMotion >  mxPathMotionNode;
        };

    }
}

#endif /* _SLIDESHOW_ANIMATIONPATHMOTIONNODE_HXX */
