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

#pragma once

#include <svx/sdrundomanager.hxx>

namespace sd
{

class UndoManager final : public SdrUndoManager
{
public:
    UndoManager();

    virtual void            EnterListAction(const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId, ViewShellId nViewShellId) override;

    virtual void            AddUndoAction( std::unique_ptr<SfxUndoAction> pAction, bool bTryMerg=false ) override;

    /** Set or reset the undo manager linked with the called undo manager.
    */
    void SetLinkedUndoManager (SfxUndoManager* pLinkedUndoManager);

private:
    using SdrUndoManager::Undo;
    using SdrUndoManager::Redo;

    /** Used when the outline view is visible as a last resort to
        synchronize the undo managers.
    */
    SfxUndoManager* mpLinkedUndoManager;

    /** Call ClearRedo() at the linked undo manager, when present.

        It should not be necessary to call ClearRedo() explicitly, but the
        synchronization between the under managers of the document and the
        outline view seems to have a bug.  Therefore this method is called
        whenever a new undo action is added.
    */
    void ClearLinkedRedoActions();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
