/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paralleltimecontainer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:36:13 $
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
#ifndef INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX

#include "basecontainernode.hxx"

namespace presentation {
namespace internal {

/** This class implements parallel node containers

    All children of this node are played in parallel
*/
class ParallelTimeContainer : public BaseContainerNode
{
public:
    ParallelTimeContainer(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode >& xNode,
        const BaseContainerNodeSharedPtr&               rParent,
        const NodeContext&                              rContext )
        : BaseContainerNode( xNode, rParent, rContext ) {}

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "ParallelTimeContainer"; }
#endif

private:
    virtual void activate_st();
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& pChildNode );
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX */

