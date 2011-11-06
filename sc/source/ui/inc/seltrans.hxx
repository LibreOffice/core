/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

