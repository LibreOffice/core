/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellsh.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:47:13 $
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

#ifndef SC_CELLSH_HXX
#define SC_CELLSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#include "shellids.hxx"
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef SC_FORMATSH_HXX
#include "formatsh.hxx"
#endif

class SvxClipboardFmtItem;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractScLinkedAreaDlg;

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
    BOOL            bPastePossible;

    void        GetPossibleClipboardFormats( SvxClipboardFmtItem& rFormats );
    void        ExecuteExternalSource(
                    const String& _rFile, const String& _rFilter, const String& _rOptions,
                    const String& _rSource, ULONG _nRefresh, SfxRequest& _rRequest );

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
    DECL_LINK( DialogClosed, AbstractScLinkedAreaDlg* );

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_CELL_SHELL);

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

};

#endif
