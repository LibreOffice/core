/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: globals.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _GLOBALS_HXX_
#define _GLOBALS_HXX_

#ifndef _OSL_MUTEX_H_
#include <osl/mutex.hxx>
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <wtypes.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <sal/types.h>


#define DNDSOURCE_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDragSource"
#define DNDSOURCE_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"
#define DNDSOURCE_REGKEY_NAME  "/com.sun.star.comp.datatransfer.dnd.OleDragSource_V1/UNO/SERVICES/com.sun.star.datatransfer.dnd.OleDragSource"

#define DNDTARGET_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDropTarget"
#define DNDTARGET_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"
#define DNDTARGET_REGKEY_NAME  "/com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1/UNO/SERVICES/com.sun.star.datatransfer.dnd.OleDropTarget"

// This maps key states as occur as parameter, e.g. in IDropTarget::DragEnter,
// IDropSource::QueryContinueDrag, to actions as are declared in
// com::sun::star::datatransfer::dnd::DNDConstants ( ACTION_MOVE etc).
// If the grfKeyState indicates the ALt or right mousebutton then the returned
// values combines all possible actions. This is because those key and button are
// used when the user expect a menu to appear when he drops. The menu then
// contains entries, such as move, copy, link, cancel.
// An odd fact is that the argument grfKeyState in IDropTarget::Drop does not
// contain mouse buttons (winnt 4 SP6). That indicates that the right mouse button
// is not considered relevant in a drag operation. Contrarily the file explorer
// gives that button a special meaning: the user has to select the effect from
// a context menu on drop.
sal_Int8 dndOleKeysToAction( DWORD grfKeyState, sal_Int8 sourceActions);

// The function maps a windows DROPEFFECTs to actions
// ( com::sun::star::datatransfer::dnd::DNDConstants).
// The argument can be a combination of different DROPEFFECTS,
// In that case the return value is also a combination of the
// appropriate actions.
sal_Int8 dndOleDropEffectsToActions( DWORD dwEffect);

// The function maps actions ( com::sun::star::datatransfer::dnd::DNDConstants)
// to window DROPEFFECTs.
// The argument can be a combination of different actions
// In that case the return value is also a combination of the
// appropriate DROPEFFECTS.
DWORD dndActionsToDropEffects( sal_Int8 actions);

// If the argument constitutes only one action then it is mapped to the
// corresponding DROPEFFECT otherwise DROPEFFECT_MOVE is returned. This is
// why move is the default effect (no modifiers pressed, or right mouse button
// or Alt).
DWORD dndActionsToSingleDropEffect( sal_Int8 actions);



struct MutexDummy
{
    osl::Mutex m_mutex;
};


#endif
