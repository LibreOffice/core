/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequentialtimecontainer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:37:02 $
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
#ifndef INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX

#include "basecontainernode.hxx"

namespace presentation {
namespace internal {

/** This class implements sequential node containers

    All children of this node are played sequentially
*/
class SequentialTimeContainer : public BaseContainerNode
{
public:
    SequentialTimeContainer(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        BaseContainerNodeSharedPtr const& pParent,
        NodeContext const& rContext )
        : BaseContainerNode( xNode, pParent, rContext ) {}

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "SequentialTimeContainer"; }
#endif

protected:
    virtual void dispose();

private:
    virtual void activate_st();
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& rNotifier );

    void skipEffect( AnimationNodeSharedPtr const& pChildNode );
    void rewindEffect( AnimationNodeSharedPtr const& pChildNode );

private:
    bool resolveChild( AnimationNodeSharedPtr const& pChildNode );

    EventSharedPtr mpCurrentSkipEvent;
    EventSharedPtr mpCurrentRewindEvent;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX */

