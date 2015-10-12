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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONCOLORNODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONCOLORNODE_HXX

#include "animationbasenode.hxx"
#include "com/sun/star/animations/XAnimateColor.hpp"

namespace slideshow {
namespace internal {

class AnimationColorNode : public AnimationBaseNode
{
public:
    AnimationColorNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext )
        : AnimationBaseNode( xNode, pParent, rContext ),
          mxColorNode( xNode, ::com::sun::star::uno::UNO_QUERY_THROW ) {}

#if defined(DBG_UTIL)
    virtual const char* getDescription() const override { return "AnimationColorNode"; }
#endif

private:
    virtual AnimationActivitySharedPtr createActivity() const override;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimateColor > mxColorNode;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_ANIMATIONCOLORNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
