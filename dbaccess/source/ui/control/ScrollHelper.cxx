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

#include "ScrollHelper.hxx"
#include <tools/debug.hxx>

#define LISTBOX_SCROLLING_AREA  12
namespace dbaui
{
DBG_NAME(OScrollHelper)

    OScrollHelper::OScrollHelper()
    {
        DBG_CTOR(OScrollHelper,NULL);
    }
    OScrollHelper::~OScrollHelper()
    {

        DBG_DTOR(OScrollHelper,NULL);
    }
    void OScrollHelper::scroll(const Point& _rPoint, const Size& _rOutputSize)
    {
        // Scrolling Areas
        Rectangle aScrollArea( Point(0, _rOutputSize.Height() - LISTBOX_SCROLLING_AREA),
                                     Size(_rOutputSize.Width(), LISTBOX_SCROLLING_AREA) );

        Link aToCall;
        // if pointer in bottom area begin scroll
        if( aScrollArea.IsInside(_rPoint) )
            aToCall = m_aUpScroll;
        else
        {
            aScrollArea.SetPos(Point(0,0));
            // if pointer in top area begin scroll
            if( aScrollArea.IsInside(_rPoint) )
                aToCall = m_aDownScroll;
        }
        if ( aToCall.IsSet() )
            aToCall.Call( NULL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
