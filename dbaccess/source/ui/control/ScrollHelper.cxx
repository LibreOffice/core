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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "ScrollHelper.hxx"
#include <tools/debug.hxx>

#define LISTBOX_SCROLLING_AREA  12
namespace dbaui
{
DBG_NAME(OScrollHelper)

// -----------------------------------------------------------------------------


    OScrollHelper::OScrollHelper()
    {
        DBG_CTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
    OScrollHelper::~OScrollHelper()
    {

        DBG_DTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
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
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
