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
#include "precompiled_sw.hxx"



#include <cmdid.h>
#include <swtypes.hxx>
#include <popbox.hxx>
#include <navipi.hxx>

SwHelpToolBox::SwHelpToolBox( SwNavigationPI* pParent, const ResId& rResId )
    : ToolBox( pParent, rResId ),
    DropTargetHelper( this )
{
}

void SwHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    // If doubleclick is detected use doubleclick handler
    if(rEvt.GetButtons() == MOUSE_RIGHT &&
        0 == GetItemId(rEvt.GetPosPixel()))
    {
        aRightClickLink.Call((MouseEvent *)&rEvt);
    }
    else
        ToolBox::MouseButtonDown(rEvt);
}

long SwHelpToolBox::DoubleClick( ToolBox* pCaller )
{
    // No doubleclick on button
    if( 0 == pCaller->GetCurItemId() && aDoubleClickLink.Call(0) )
        return sal_True;
    return sal_False;
}

SwHelpToolBox::~SwHelpToolBox() {}

sal_Int8 SwHelpToolBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->AcceptDrop( rEvt );
}

sal_Int8 SwHelpToolBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->ExecuteDrop( rEvt );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
