/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationaudionode.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:39:11 $
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

#ifndef _SLIDESHOW_ANIMATIONAUDIONODE_HXX
#define _SLIDESHOW_ANIMATIONAUDIONODE_HXX

#include <basecontainernode.hxx>
#include <soundplayer.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAUDIO_HPP_
#include <com/sun/star/animations/XAudio.hpp>
#endif


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        /** Audio node.

            This animation node contains an audio effect. Duration and
            start/stop behaviour is affected by the referenced audio
            file.
        */
        class AnimationAudioNode : public BaseNode, public AnimationEventHandler
        {
        public:
            AnimationAudioNode( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::animations::XAnimationNode >& xNode,
                                const BaseContainerNodeSharedPtr&                   rParent,
                                const NodeContext&                                  rContext );

            virtual void dispose();
            virtual bool activate();
            virtual void deactivate();

            /** Overridden, because the sound duration normally
                determines effect duration.
            */
            virtual void scheduleDeactivationEvent() const;

            /// overridden, because NO-OP for all leaf nodes (which typically don't register nowhere)
            virtual void notifyDeactivating( const AnimationNodeSharedPtr& rNotifier );

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

    }
}

#endif /* _SLIDESHOW_ANIMATIONAUDIONODE_HXX */
