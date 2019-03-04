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
#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONAUDIONODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONAUDIONODE_HXX

#include <basecontainernode.hxx>
#include <soundplayer.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAudio.hpp>

namespace slideshow {
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
        css::uno::Reference<css::animations::XAnimationNode> const& xNode,
        BaseContainerNodeSharedPtr const& pParent,
        NodeContext const& rContext );

protected:
    virtual void dispose() override;

private:
    virtual void activate_st() override;
    virtual void deactivate_st( NodeState eDestState ) override;
    virtual bool hasPendingAnimation() const override;

    /// overridden, because we need to deal with STOPAUDIO commands
    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode ) override;

private:
    css::uno::Reference<css::animations::XAudio >  mxAudioNode;
    OUString                                       maSoundURL;
    mutable SoundPlayerSharedPtr                   mpPlayer;

    void createPlayer() const;
    void resetPlayer() const;
    void checkPlayingStatus();
};

} // namespace internal
} // namespace slideshow

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONAUDIONODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
