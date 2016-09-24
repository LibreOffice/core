/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_UNDOCONVERT_HXX
#define INCLUDED_SC_UNDOCONVERT_HXX

#include <undobase.hxx>
#include <cellvalues.hxx>

namespace sc {

class UndoFormulaToValue : public ScSimpleUndo
{
    TableValues maUndoValues;

public:
    UndoFormulaToValue( ScDocShell* pDocSh, TableValues& rUndoValues );

    virtual OUString GetComment() const override;
    virtual void Undo() override;
    virtual void Redo() override;

private:
    void Execute();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
