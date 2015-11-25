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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>

namespace objectpositioning
{
    class SwToLayoutAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position type TO_LAYOUT
            Point       maRelPos;

            // #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrameAnchorPos;

        public:
            SwToLayoutAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwToLayoutAnchoredObjectPosition();

            /** calculate position for object
            */
            virtual void CalcPosition() override;

            /** calculated relative position for object
            */
            const Point& GetRelPos() const { return maRelPos;}
    };
} // namespace objectpositioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
