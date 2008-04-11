/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: globals.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_dtrans.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "globals.hxx"

//--> TRA
#include <com/sun/star/datatransfer/XTransferable.hpp>

// used as shortcut when drag-source and drop-target are the same
::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > g_XTransferable;

//<-- TRA

using namespace com::sun::star::datatransfer::dnd::DNDConstants;

sal_Int8 dndOleKeysToAction( DWORD grfKeyState, sal_Int8 nSourceActions)
{
    sal_Int8 ret= 0;

    // no MK_ALT, MK_CONTROL, MK_SHIFT
    if( !(grfKeyState & MK_CONTROL) &&
        !(grfKeyState & MK_ALT)    &&
        !(grfKeyState & MK_RBUTTON) &&
        !(grfKeyState & MK_SHIFT))
    {
        if( nSourceActions & ACTION_MOVE )
        {
            ret= ACTION_DEFAULT | ACTION_MOVE;
        }

        else if( nSourceActions & ACTION_COPY )
        {
            ret= ACTION_COPY;
        }

        else if( nSourceActions & ACTION_LINK )
        {
            ret= ACTION_LINK;
        }

        else
            ret = 0;
    }
    else if( grfKeyState & MK_SHIFT &&
            !(grfKeyState & MK_CONTROL))
    {
        ret= ACTION_MOVE;
    }
    else if ( grfKeyState & MK_CONTROL &&
              !(grfKeyState & MK_SHIFT) )
    {
        ret= ACTION_COPY;
    }
    else if ( grfKeyState & MK_CONTROL &&
              grfKeyState & MK_SHIFT)
    {
        ret= ACTION_LINK;
    }
    else if ( grfKeyState & MK_RBUTTON |
              grfKeyState & MK_ALT)
    {
        ret= ACTION_COPY_OR_MOVE | ACTION_LINK;
    }
    return ret;
}


sal_Int8 dndOleDropEffectsToActions( DWORD dwEffect)
{
    sal_Int8 ret= ACTION_NONE;
    if( dwEffect & DROPEFFECT_COPY)
        ret |= ACTION_COPY;
    if( dwEffect & DROPEFFECT_MOVE)
        ret |= ACTION_MOVE;
    if( dwEffect & DROPEFFECT_LINK)
        ret |= ACTION_LINK;

    return ret;
}

DWORD dndActionsToDropEffects( sal_Int8 actions)
{
    DWORD ret= DROPEFFECT_NONE;
    if( actions & ACTION_MOVE)
        ret |= DROPEFFECT_MOVE;
    if( actions & ACTION_COPY)
        ret |= DROPEFFECT_COPY;
    if( actions & ACTION_LINK)
        ret |= DROPEFFECT_LINK;
    if( actions & ACTION_DEFAULT)
        ret |= DROPEFFECT_COPY;
    return ret;
}

DWORD dndActionsToSingleDropEffect( sal_Int8 actions)
{
    DWORD effects= dndActionsToDropEffects( actions);

    sal_Int8 countEffect= 0;

    if( effects & DROPEFFECT_MOVE)
        countEffect++;
    if( effects & DROPEFFECT_COPY)
        countEffect++;
    if( effects & DROPEFFECT_LINK)
        countEffect++;

    // DROPEFFECT_MOVE is the default effect
    DWORD retVal= countEffect > 1 ? DROPEFFECT_MOVE : effects;
    return retVal;
}
