/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdpreslt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:52:22 $
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
    ::sd::DrawDocShell*     pDocSh;
    ::sd::ViewShell* pViewSh;
    FixedText           aFtLayout;
    ValueSet            aVS;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    CheckBox            aCbxMasterPage;
    CheckBox            aCbxCheckMasters;
    PushButton          aBtnLoad;

    const SfxItemSet&   rOutAttrs;

    List*               pLayoutNames;

    String              aName;          // Layoutname oder Dateiname
    long                nLayoutCount;   // Anzahl, der im Dokument vorhandenen MasterPages
    String              aStrNone;

    void                FillValueSet();
    void                Reset();
};

#endif

