/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdpreslt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:47:47 $
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

#ifndef SD_PRES_LAYOUT_DLG_HXX
#define SD_PRES_LAYOUT_DLG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif

class SfxItemSet;

namespace sd {
class DrawDocShell;
class ViewShell;
}



class SdPresLayoutDlg
    : public ModalDialog
{
public:
    SdPresLayoutDlg(
        ::sd::DrawDocShell* pDocShell,
        ::sd::ViewShell* pViewShell,
        ::Window* pWindow,
        const SfxItemSet& rInAttrs);

    virtual ~SdPresLayoutDlg (void);

    void                GetAttr(SfxItemSet& rOutAttrs);

    DECL_LINK(ClickLayoutHdl, void *);
    DECL_LINK(ClickLoadHdl, void *);

private:
    ::sd::DrawDocShell* mpDocSh;
    ::sd::ViewShell*    mpViewSh;
    FixedText           maFtLayout;
    ValueSet            maVS;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    CheckBox            maCbxMasterPage;
    CheckBox            maCbxCheckMasters;
    PushButton          maBtnLoad;

    const SfxItemSet&   mrOutAttrs;

    List*               mpLayoutNames;

    String              maName;          // Layoutname oder Dateiname
    long                mnLayoutCount;  // Anzahl, der im Dokument vorhandenen MasterPages
    const String        maStrNone;

    void                FillValueSet();
    void                Reset();
};

#endif

