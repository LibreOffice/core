/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drtxtob.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:22:13 $
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

#ifndef SC_DRTXTOB_HXX
#define SC_DRTXTOB_HXX

#ifndef _SFX_HXX
#endif

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#include "shellids.hxx"

USHORT ScGetFontWorkId();       // statt SvxFontWorkChildWindow::GetChildWindowId()

class ScViewData;
class TransferableDataHelper;
class TransferableClipboardListener;

class ScDrawTextObjectBar : public SfxShell
{
    ScViewData*         pViewData;
    TransferableClipboardListener* pClipEvtLstnr;
    BOOL                bPastePossible;

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_DRAW_TEXT_SHELL)

        ScDrawTextObjectBar(ScViewData* pData);
        ~ScDrawTextObjectBar();

    void StateDisableItems( SfxItemSet &rSet );

    void Execute( SfxRequest &rReq );
    void ExecuteTrans( SfxRequest& rReq );
    void GetState( SfxItemSet& rSet );
    void GetClipState( SfxItemSet& rSet );

    void ExecuteAttr( SfxRequest &rReq );
    void GetAttrState( SfxItemSet& rSet );
    void ExecuteToggle( SfxRequest &rReq );

    BOOL ExecuteCharDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet );
    BOOL ExecuteParaDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet );

    void ExecuteExtra( SfxRequest &rReq );
    void ExecFormText(SfxRequest& rReq);        // StarFontWork
    void GetFormTextState(SfxItemSet& rSet);

private:
    void ExecuteGlobal( SfxRequest &rReq );         // von Execute gerufen fuer ganze Objekte
    void GetGlobalClipState( SfxItemSet& rSet );
    void ExecutePasteContents( SfxRequest &rReq );
    BOOL IsNoteEdit();
};



#endif

