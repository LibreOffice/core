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
#include "precompiled_vcl.hxx"

#include "vcl/window.hxx"
#include "vcl/waitobj.hxx"
#include "vcl/button.hxx"

// -----------------------------------------------------------------------

WaitObject::~WaitObject()
{
    if ( mpWindow )
        mpWindow->LeaveWait();
}

// -----------------------------------------------------------------------

Size Window::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return Size();
    case WINDOWSIZE_PREFERRED:
        return GetOptimalSize( WINDOWSIZE_MINIMUM );
    case WINDOWSIZE_MAXIMUM:
    default:
        return Size( LONG_MAX, LONG_MAX );
    }
}

// -----------------------------------------------------------------------

void Window::ImplAdjustNWFSizes()
{
    switch( GetType() )
    {
    case WINDOW_CHECKBOX:
        ((CheckBox*)this)->ImplSetMinimumNWFSize();
        break;
    case WINDOW_RADIOBUTTON:
        ((RadioButton*)this)->ImplSetMinimumNWFSize();
        break;
    default:
        {
            // iterate over children
            Window* pWin = GetWindow( WINDOW_FIRSTCHILD );
            while( pWin )
            {
                pWin->ImplAdjustNWFSizes();
                pWin = pWin->GetWindow( WINDOW_NEXT );
            }
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
