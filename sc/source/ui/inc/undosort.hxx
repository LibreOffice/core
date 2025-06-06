/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "undobase.hxx"
#include <sortparam.hxx>

namespace sc {

class UndoSort : public ScSimpleUndo
{
    ReorderParam maParam;

public:
    UndoSort( ScDocShell& rDocSh, ReorderParam aParam );

    virtual OUString GetComment() const override;
    virtual void Undo() override;
    virtual void Redo() override;

private:
    void Execute( bool bUndo );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
