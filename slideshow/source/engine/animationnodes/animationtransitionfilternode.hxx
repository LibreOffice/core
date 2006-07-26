/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtransitionfilternode.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:33:32 $
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

#ifndef INCLUDED_SLIDESHOW_ANIMATIONTRANSITIONFILTERNODE_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONTRANSITIONFILTERNODE_HXX

#include "animationbasenode.hxx"
#include "com/sun/star/animations/XTransitionFilter.hpp"

namespace presentation {
namespace internal {

class AnimationTransitionFilterNode : public AnimationBaseNode
{
public:
    AnimationTransitionFilterNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext )
        : AnimationBaseNode( xNode, pParent, rContext ),
          mxTransitionFilterNode( xNode, ::com::sun::star::uno::UNO_QUERY_THROW)
        {}

#if defined(VERBOSE)
    virtual const char* getDescription() const
        { return "AnimationTransitionFilterNode"; }
#endif

protected:
    virtual void dispose();

private:
    virtual AnimationActivitySharedPtr createActivity() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XTransitionFilter> mxTransitionFilterNode;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_ANIMATIONTRANSITIONFILTERNODE_HXX */
