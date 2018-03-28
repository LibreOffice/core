/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "globals.hxx"
#include "DragSource.hxx"
#include "DropTarget.hxx"

using namespace com::sun::star::datatransfer::dnd::DNDConstants;

//
// Convert office drag actions as defined in
//   <type>com::sun::star::datatransfer::dnd::DNDConstants</type>
//   into system conform drag actions.
//
MRESULT OfficeToSystemDragActions( sal_Int8 dragActions)
{
    MRESULT actions = MRFROM2SHORT( DOR_NODROP, 0);

    if (dragActions & ACTION_COPY)
    {
        actions = MRFROM2SHORT( DOR_DROP, DO_COPY);
    }

    if (dragActions & ACTION_MOVE)
    {
        actions = MRFROM2SHORT( DOR_DROP, DO_MOVE);
    }

    if (dragActions & ACTION_LINK)
    {
        actions = MRFROM2SHORT( DOR_DROP, DO_LINK);
    }

    debug_printf("OfficeToSystemDragActions %d->0x%x", dragActions, actions);
    return actions;
}

//
// Convert system conform drag actions into office conform
// drag actions as defined in
// <type>com::sun::star::datatransfer::dnd::DNDConstants</type>.
//
sal_Int8 SystemToOfficeDragActions( USHORT usOperation)
{
    sal_Int8 actions = ACTION_NONE;

    switch( usOperation) {
    case DO_UNKNOWN:
        break;
    case DO_DEFAULT:
        actions |= ACTION_MOVE;
        break;
    case DO_MOVE:
        actions |= ACTION_MOVE;
        break;
    case DO_COPY:
        actions |= ACTION_COPY;
        break;
    case DO_LINK:
        actions |= ACTION_LINK;
        break;
    }

    debug_printf("SystemToOfficeDragActions 0x%x->%d", usOperation, actions);
    return actions;
}


//
// functions used by dnd.dll to get access to Window class data fields
// in the subclassed window procedure
//

// Store DropTarget object reference
void SetWindowDropTargetPtr( HWND hWnd, void* dropTarget )
{
    WinSetWindowULong( hWnd, SAL_FRAME_DROPTARGET, (ULONG)dropTarget);
}

// Get DropTarget object reference
void* GetWindowDropTargetPtr( HWND hWnd )
{
    return (void*)WinQueryWindowULong( hWnd, SAL_FRAME_DROPTARGET);
}

// Store DragSource object reference
void SetWindowDragSourcePtr( HWND hWnd, void* dragSource )
{
    WinSetWindowULong( hWnd, SAL_FRAME_DRAGSOURCE, (ULONG)dragSource);
}

// Get DropTarget object reference
void* GetWindowDragSourcePtr( HWND hWnd )
{
    return (void*)WinQueryWindowULong( hWnd, SAL_FRAME_DRAGSOURCE);
}

// map desktop coordinates of mouse pointer to local window
// inverting also the y axis
void MapWindowPoint( HWND hwnd, PDRAGINFO dragInfo, PPOINTL ptlMouse)
{
    RECTL   rclClient;
    ptlMouse->x = dragInfo->xDrop;
    ptlMouse->y = dragInfo->yDrop;
    WinMapWindowPoints( HWND_DESKTOP, hwnd, ptlMouse, 1);
    // invert y-coordinate
    WinQueryWindowRect( hwnd, &rclClient);
    ptlMouse->y = rclClient.yTop - ptlMouse->y;
}


//
// subclassed frame window procedure: used to intercept DM_* messages
// without accessing default window procedure inside VCL private code
//
extern "C" MRESULT EXPENTRY dndFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
#if 0
    debug_printf("dndFrameProc hwnd %x, msg %x", hwnd, msg);
#endif
    DropTarget* dt = (DropTarget*) GetWindowDropTargetPtr( hwnd);
    DragSource* ds = NULL;
    MRESULT mr;

    switch( msg) {

    case DM_DRAGOVER:
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_DRAGOVER", hwnd, dt);
        return dt->dragOver( (PDRAGINFO) mp1);
        break;

    case DM_DRAGLEAVE:
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_DRAGLEAVE", hwnd, dt);
        return dt->dragLeave( (PDRAGINFO) mp1);
        break;

    case DM_DROP:
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_DROP", hwnd, dt);
        mr = dt->drop( (PDRAGINFO) mp1);
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_DROP mr=%x", hwnd, dt, mr);
        return mr;
        break;

    case DM_RENDERCOMPLETE:
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_RENDERCOMPLETE", hwnd, dt);
        mr = dt->renderComplete( (PDRAGTRANSFER) mp1);
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_RENDERCOMPLETE mr=0x%x", hwnd, dt, mr);
        return mr;
        break;

    case DM_RENDERPREPARE:
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_RENDERPREPARE", hwnd, dt);
        break;

    case DM_RENDER:
        ds = (DragSource*) GetWindowDragSourcePtr( hwnd);
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_RENDER", hwnd, ds);
        mr = ds->render( (PDRAGTRANSFER) mp1);
        return mr;
        break;

        // sent from target window to source window after rendering
    case DM_ENDCONVERSATION:
        // sent from AOO to DropSource to notify end of dragging
    case DM_AOO_ENDCONVERSATION:
        ds = (DragSource*) GetWindowDragSourcePtr( hwnd);
        debug_printf("dndFrameProc hwnd %x, dt %x, DM_ENDCONVERSATION", hwnd, ds);
        mr = ds->endConversation( (ULONG) mp1, (ULONG) mp2);
        return mr;
        break;

    }

    // forward to VCL frame proc
    return dt->defWndProc(hwnd, msg, mp1, mp2);
}
