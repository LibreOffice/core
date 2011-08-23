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

#ifndef SC_CELLSH_HXX
#define SC_CELLSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <bf_sfx2/shell.hxx>
#endif
#include "shellids.hxx"
#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <bf_svx/svdmark.hxx>
#endif

#ifndef _LINK_HXX 
#include <tools/link.hxx>
#endif

#ifndef SC_FORMATSH_HXX
#include "formatsh.hxx"
#endif
class TransferableDataHelper;
class TransferableClipboardListener;
namespace binfilter {

class SvxClipboardFmtItem;


class ScCellShell: public ScFormatShell
{
private:
    TransferableClipboardListener* pClipEvtLstnr;
    BOOL		bPastePossible;

    void		GetPossibleClipboardFormats( SvxClipboardFmtItem& rFormats );

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_CELL_SHELL);

                ScCellShell(ScViewData* pData);
    virtual		~ScCellShell();

    void		Execute(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		Execute(SfxRequest &);
    void		GetState(SfxItemSet &);

    void		ExecuteEdit( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteEdit( SfxRequest& rReq );
    void		ExecuteTrans( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteTrans( SfxRequest& rReq );

    void		GetBlockState( SfxItemSet& rSet );
    void		GetCellState( SfxItemSet& rSet );

    void		ExecuteDB( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteDB( SfxRequest& rReq );
    void		GetDBState( SfxItemSet& rSet );

    void		ExecImageMap(SfxRequest& rReq);		// ImageMap
    void		GetImageMapState(SfxItemSet& rSet);

    void		GetClipState( SfxItemSet& rSet );
    void		GetHLinkState( SfxItemSet& rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		GetHLinkState( SfxItemSet& rSet );

    void		ExecuteCursor( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteCursor( SfxRequest& rReq );
    void		ExecuteCursorSel( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteCursorSel( SfxRequest& rReq );
    void		ExecutePage( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecutePage( SfxRequest& rReq );
    void		ExecutePageSel( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecutePageSel( SfxRequest& rReq );
    void		ExecuteMove( SfxRequest& rReq ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		ExecuteMove( SfxRequest& rReq );
    void		GetStateCursor( SfxItemSet& rSet ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void		GetStateCursor( SfxItemSet& rSet );

};

} //namespace binfilter
#endif
