/*************************************************************************
 *
 *  $RCSfile: sdpreslt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SD_SDPRESLT_HXX_
#define _SD_SDPRESLT_HXX_

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
class SdViewShell;
class SdDrawDocShell;



class SdPresLayoutDlg : public ModalDialog
{
private:
    SdDrawDocShell*     pDocSh;
    SdViewShell*        pViewSh;
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

public:
                        SdPresLayoutDlg( SdDrawDocShell* pDocShell, SdViewShell* pViewShell, Window* pWindow,
                                        const SfxItemSet& rInAttrs);

                        ~SdPresLayoutDlg();

    void                GetAttr(SfxItemSet& rOutAttrs);

    DECL_LINK(ClickLayoutHdl, void *);
    DECL_LINK(ClickLoadHdl, void *);
};

#endif // _SD_PRESLT_HXX_

