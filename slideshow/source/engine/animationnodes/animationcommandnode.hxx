/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_SLIDESHOW_ANIMATIONCOMMANDNODE_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONCOMMANDNODE_HXX

#include "basecontainernode.hxx"
#include "externalmediashape.hxx"
#include "soundplayer.hxx"
#include "com/sun/star/animations/XCommand.hpp"

namespace slideshow {
namespace internal {

/** Command node.

    This animation node encapsulates a command. Not yet implemented:
    verb & custom.
*/
class AnimationCommandNode : public BaseNode
{
public:
    AnimationCommandNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext );

protected:
    virtual void dispose();

private:
    virtual void activate_st();
    virtual bool hasPendingAnimation() const;

private:
    ExternalMediaShapeSharedPtr mpShape;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XCommand > mxCommandNode;
    bool                                       mbIsPaused;
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_ANIMATIONAUDIONODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
