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

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "globals.hxx"

//--> TRA
#include <com/sun/star/datatransfer/XTransferable.hpp>

// used as shortcut when drag-source and drop-target are the same
css::uno::Reference< css::datatransfer::XTransferable > g_XTransferable;

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
    else if ( grfKeyState & MK_RBUTTON ||
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
