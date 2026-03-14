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
#include <SheetView.hxx>
#include <SheetViewManager.hxx>

namespace sc {

class UndoSort : public ScSimpleUndo
{
    ReorderParam maParam;

    // For sorting on a sheet view.
    SheetViewID mnSheetViewID = -1;
    SCTAB mnDefaultViewTab = -1;
    std::shared_ptr<SheetViewSortData> mpSortDataBefore;
    std::shared_ptr<SheetViewSortData> mpSortDataAfter;

    // For sorting on a default view tab:
    bool mbDefaultViewSort = false;
    std::shared_ptr<DefaultViewSortData> mpDefaultViewSortDataBefore;
    std::shared_ptr<DefaultViewSortData> mpDefaultViewSortDataAfter;

public:
    UndoSort( ScDocShell& rDocSh, ReorderParam aParam );

    /** Set sheet view context for sorts on a sheet view. */
    void setSheetViewContext(SCTAB nDefaultViewTab, SheetViewID nSheetViewID,
                             std::shared_ptr<SheetViewSortData> pSortDataBefore,
                             std::shared_ptr<SheetViewSortData> pSortDataAfter);

    /** Set default view context for sorts on a default view tab that has sheet views. */
    void setDefaultViewContext(SCTAB nDefaultViewTab,
                               std::shared_ptr<DefaultViewSortData> pDefaultViewSortDataBefore,
                               std::shared_ptr<DefaultViewSortData> pDefaultViewSortDataAfter);

    virtual OUString GetComment() const override;
    virtual void Undo() override;
    virtual void Redo() override;

private:
    void Execute( bool bUndo );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
