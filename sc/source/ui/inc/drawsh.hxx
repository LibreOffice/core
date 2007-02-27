/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawsh.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:21:34 $
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

#ifndef SC_DRAWSH_HXX
#define SC_DRAWSH_HXX

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

class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class ScViewData;


class ScDrawShell : public SfxShell
{
    ScViewData* pViewData;

    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );

protected:
    ScViewData* GetViewData()   { return pViewData; }

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_DRAW_SHELL)

                    ScDrawShell(ScViewData* pData);
                    ~ScDrawShell();

    void    StateDisableItems( SfxItemSet &rSet );

    void    ExecDrawAttr(SfxRequest& rReq);
    void    GetDrawAttrState(SfxItemSet &rSet);
    void    GetAttrFuncState(SfxItemSet &rSet);

    void    ExecDrawFunc(SfxRequest& rReq);
    void    GetDrawFuncState(SfxItemSet &rSet);
    void    GetState(SfxItemSet &rSet);

    void    ExecFormText(SfxRequest& rReq);     // StarFontWork
    void    GetFormTextState(SfxItemSet& rSet);

    void    ExecuteHLink(SfxRequest& rReq);     // Hyperlink
    void    GetHLinkState(SfxItemSet& rSet);

    void    ExecFormatPaintbrush(SfxRequest& rReq);
    void    StateFormatPaintbrush(SfxItemSet& rSet);

    void    ExecuteLineDlg( SfxRequest& rReq, USHORT nTabPage = 0xffff );
    void    ExecuteAreaDlg( SfxRequest& rReq, USHORT nTabPage = 0xffff );
    void    ExecuteTextAttrDlg( SfxRequest& rReq, USHORT nTabPage = 0xffff );

    BOOL    AreAllObjectsOnLayer(USHORT nLayerNo,const SdrMarkList& rMark);
};



#endif


