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



#ifndef SC_CELLSH_HXX
#define SC_CELLSH_HXX

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <tools/link.hxx>
#include "formatsh.hxx"

class SvxClipboardFmtItem;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractScLinkedAreaDlg;
class ScTabViewShell;

struct CellShell_Impl
{
    TransferableClipboardListener*  m_pClipEvtLstnr;
    AbstractScLinkedAreaDlg*        m_pLinkedDlg;
    SfxRequest*                     m_pRequest;

    CellShell_Impl() :
        m_pClipEvtLstnr( NULL ),
        m_pLinkedDlg( NULL ),
        m_pRequest( NULL ) {}
};

class ScCellShell: public ScFormatShell
{
private:
    CellShell_Impl* pImpl;
    sal_Bool            bPastePossible;

    void        GetPossibleClipboardFormats( SvxClipboardFmtItem& rFormats );
    void        ExecuteExternalSource(
                    const String& _rFile, const String& _rFilter, const String& _rOptions,
                    const String& _rSource, sal_uLong _nRefresh, SfxRequest& _rRequest );

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
    DECL_LINK( DialogClosed, AbstractScLinkedAreaDlg* );

public:

    SFX_DECL_INTERFACE(SCID_CELL_SHELL)

                ScCellShell(ScViewData* pData);
    virtual     ~ScCellShell();

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);

    void        ExecuteEdit( SfxRequest& rReq );
    void        ExecuteTrans( SfxRequest& rReq );

    void        GetBlockState( SfxItemSet& rSet );
    void        GetCellState( SfxItemSet& rSet );

    void        ExecuteDB( SfxRequest& rReq );
    void        GetDBState( SfxItemSet& rSet );

    void        ExecImageMap(SfxRequest& rReq);     // ImageMap
    void        GetImageMapState(SfxItemSet& rSet);

    void        GetClipState( SfxItemSet& rSet );
    void        GetHLinkState( SfxItemSet& rSet );

    void        ExecuteCursor( SfxRequest& rReq );
    void        ExecuteCursorSel( SfxRequest& rReq );
    void        ExecutePage( SfxRequest& rReq );
    void        ExecutePageSel( SfxRequest& rReq );
    void        ExecuteMove( SfxRequest& rReq );
    void        GetStateCursor( SfxItemSet& rSet );

    static void PasteFromClipboard( ScViewData* pViewData, ScTabViewShell* pTabViewShell, bool bShowDialog );
};

#endif
