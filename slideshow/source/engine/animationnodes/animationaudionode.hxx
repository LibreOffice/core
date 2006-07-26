/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationaudionode.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:29:16 $
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
#ifndef INCLUDED_SLIDESHOW_ANIMATIONAUDIONODE_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONAUDIONODE_HXX

#include "basecontainernode.hxx"
#include "soundplayer.hxx"
#include "com/sun/star/animations/XAnimationNode.hpp"
#include "com/sun/star/animations/XAudio.hpp"

namespace presentation {
namespace internal {

/** Audio node.

    This animation node contains an audio effect. Duration and
    start/stop behaviour is affected by the referenced audio
    file.
*/
class AnimationAudioNode : public BaseNode, public AnimationEventHandler
{
public:
    AnimationAudioNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext );

protected:
    virtual void dispose();

private:
    virtual void activate_st();
    virtual void deactivate_st( NodeState eDestState, bool );
    virtual bool hasPendingAnimation() const;

    /// overriden, because we need to deal with STOPAUDIO commands
    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode );

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAudio >  mxAudioNode;
    ::rtl::OUString                             maSoundURL;
    mutable SoundPlayerSharedPtr                mpPlayer;

    void createPlayer() const;
    void resetPlayer() const;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_ANIMATIONAUDIONODE_HXX */
