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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SELTRANS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SELTRANS_HXX

#include <svtools/transfer.hxx>

class ScTabView;
class ScTransferObj;
class ScDrawTransferObj;

enum ScSelectionTransferMode
{
    SC_SELTRANS_INVALID,
    SC_SELTRANS_CELL,
    SC_SELTRANS_CELLS,
    SC_SELTRANS_DRAW_BITMAP,
    SC_SELTRANS_DRAW_GRAPHIC,
    SC_SELTRANS_DRAW_BOOKMARK,
    SC_SELTRANS_DRAW_OLE,
    SC_SELTRANS_DRAW_OTHER
};

class ScSelectionTransferObj : public TransferableHelper
{
private:
    ScTabView*              pView;
    ScSelectionTransferMode eMode;
    ScTransferObj*          pCellData;
    ScDrawTransferObj*      pDrawData;

                ScSelectionTransferObj( ScTabView* pSource, ScSelectionTransferMode eNewMode );
    void        CreateCellData();
    void        CreateDrawData();

public:
            // creates an object if the view has a valid selection,
            // returns NULL otherwise
    static ScSelectionTransferObj* CreateFromView( ScTabView* pSource );

    virtual     ~ScSelectionTransferObj();

    void        ForgetView();
    ScTabView*  GetView() const     { return pView; }

    ScTransferObj*      GetCellData();
    ScDrawTransferObj*  GetDrawData();

    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual void        ObjectReleased() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
