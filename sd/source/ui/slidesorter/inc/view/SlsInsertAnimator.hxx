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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSINSERTANIMATOR_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSINSERTANIMATOR_HXX

#include "controller/SlsAnimator.hxx"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace sd { namespace slidesorter { namespace view {

class InsertPosition;

/** Animate the positions of page objects to make room at the insert
    position while a move or copy operation takes place.
*/
class InsertAnimator
    : private ::boost::noncopyable
{
public:
    InsertAnimator (SlideSorter& rSlideSorter);

    /** Set the position at which we have to make room for the display of an
        icon.
    */
    void SetInsertPosition (const InsertPosition& rInsertPosition);

    enum ResetMode { RM_Normal, RM_AbortAnimations };
    /** Restore the normal position of all page objects.
        @param eMode
            This flag controls whether to start an animation that ends in the
            normal positions of all slides (AM_Animated) or to restore the
            normal positions immediately (AM_Immediate).
    */
    void Reset (const controller::Animator::AnimationMode eMode);

private:
    class Implementation;
    ::boost::shared_ptr<Implementation> mpImplementation;
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
