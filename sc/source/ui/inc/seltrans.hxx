/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seltrans.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:49:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_SELTRANS_HXX
#define SC_SELTRANS_HXX

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

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
    BOOL        StillValid();
    ScTabView*  GetView() const     { return pView; }

    ScTransferObj*      GetCellData();
    ScDrawTransferObj*  GetDrawData();

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();
};

#endif

