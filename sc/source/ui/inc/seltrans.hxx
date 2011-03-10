/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SC_SELTRANS_HXX
#define SC_SELTRANS_HXX

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
    sal_Bool        StillValid();
    ScTabView*  GetView() const     { return pView; }

    ScTransferObj*      GetCellData();
    ScDrawTransferObj*  GetDrawData();

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
