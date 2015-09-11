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
#ifndef INCLUDED_DTRANS_SOURCE_WIN32_DND_GLOBALS_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_DND_GLOBALS_HXX

#include <osl/mutex.hxx>

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

#define DNDTARGET_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDropTarget"
#define DNDTARGET_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"

// This maps key states as occur as parameter, e.g. in IDropTarget::DragEnter,
// IDropSource::QueryContinueDrag, to actions as are declared in
// css::datatransfer::dnd::DNDConstants ( ACTION_MOVE etc).
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
// ( css::datatransfer::dnd::DNDConstants).
// The argument can be a combination of different DROPEFFECTS,
// In that case the return value is also a combination of the
// appropriate actions.
sal_Int8 dndOleDropEffectsToActions( DWORD dwEffect);

// The function maps actions ( css::datatransfer::dnd::DNDConstants)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
