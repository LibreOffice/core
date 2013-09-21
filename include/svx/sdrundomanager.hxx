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
#ifndef _SDR_UNDOMANAGER_HXX
#define _SDR_UNDOMANAGER_HXX

#include "svx/svxdllapi.h"
#include <sal/types.h>
#include <editeng/editund2.hxx>
#include <tools/link.hxx>

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoManager : public EditUndoManager
{
private:
    using EditUndoManager::Undo;
    using EditUndoManager::Redo;

    Link            maEndTextEditHdl;
    SfxUndoAction*  mpLastUndoActionBeforeTextEdit;
    bool            mbEndTextEditTriggeredFromUndo;

protected:
    // call to check for TextEdit active
    bool isTextEditActive() const;

public:
    SdrUndoManager(sal_uInt16 nMaxUndoActionCount = 20);
    virtual ~SdrUndoManager();

    /// react depending on edit mode and if no more undo is possible
    virtual bool Undo();
    virtual bool Redo();

    // Call for the view which starts the interactive text edit. Use link to
    // activate (start text edit) and empty link to reset (end text edit). On
    // reset all text edit actions will be removed from this undo manager to
    // restore the state before activation
    void SetEndTextEditHdl(const Link& rLink);

    // check from outside if we are inside a callback for ending text edit. This
    // is needed to detect inside end text edit if it is a regular one or triggered
    // by a last undo during text edit
    bool isEndTextEditTriggeredFromUndo() { return mbEndTextEditTriggeredFromUndo; }
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_UNDOMANAGER_HXX
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
