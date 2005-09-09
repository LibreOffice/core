/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BreakDlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:02:12 $
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

#ifndef SD_BREAK_DLG_HXX
#define SD_BREAK_DLG_HXX

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SVX_DLG_CTRL_HXX //autogen
#include <svx/dlgctrl.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <svtools/prgsbar.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

class SvdProgressInfo;
class SfxProgress;

namespace sd {

class DrawDocShell;
class DrawView;

/*************************************************************************
|*
|* Dialog zum aufbrechen von Metafiles
|*
\************************************************************************/
class BreakDlg
    : public SfxModalDialog
{
public:
    BreakDlg (
        ::Window* pWindow,
        DrawView* pDrView,
        DrawDocShell* pShell,
        ULONG nSumActionCount,
        ULONG nObjCount);
    virtual ~BreakDlg();

    short Execute();

private:
    FixedText       aFtObjInfo;
    FixedText       aFtActInfo;
    FixedText       aFtInsInfo;

    FixedInfo       aFiObjInfo;
    FixedInfo       aFiActInfo;
    FixedInfo       aFiInsInfo;

    CancelButton    aBtnCancel;
    DrawView*   pDrView;

    BOOL            bCancel;

    Timer           aTimer;
    SvdProgressInfo *pProgrInfo;
    Link            aLink;
    SfxProgress     *mpProgress;

    DECL_LINK( CancelButtonHdl, void* );
    DECL_LINK( UpDate, void* );
    DECL_LINK( InitialUpdate, Timer* );
};

} // end of namespace sd

#endif
