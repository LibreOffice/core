/*************************************************************************
 *
 *  $RCSfile: headerfooterdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-13 16:32:48 $
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
#ifndef _SD_HEADERFOOTER_DIALOG_HXX
#define _SD_HEADERFOOTER_DIALOG_HXX

#ifndef _SV_TABDLG_HXX //autogen
#include <vcl/tabdlg.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_TABCTRL_HXX //autogen
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif

#include "headerfooterdlg.hrc"
#include "sdpage.hxx"

class SfxObjectShell;
class SdUndoGroup;
class ::sd::ViewShell;

namespace sd
{

class HeaderFooterTabPage;

class HeaderFooterDialog : public TabDialog
{
private:
    DECL_LINK( ActivatePageHdl, TabControl * );
    DECL_LINK( DeactivatePageHdl, TabControl * );

    TabControl      maTabCtrl;

    HeaderFooterTabPage*    mpSlideTabPage;
    HeaderFooterTabPage*    mpNotesHandoutsTabPage;

    HeaderFooterSettings    maSlideSettings;
    HeaderFooterSettings    maNotesHandoutSettings;
    bool                    mbNotOnTitle;

    SdDrawDocument*         mpDoc;
    SdPage*                 mpCurrentPage;
    ViewShell*              mpViewShell;

    void initTabPages();
    void apply( bool bToAll, bool bForceSlides );
    void change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings );

public:
    HeaderFooterDialog( ViewShell* pViewShell, ::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~HeaderFooterDialog();

    void ApplyToAll( TabPage* pPage );
    void Apply( TabPage* pPage );
    void Cancel( TabPage* pPage );

    virtual short Execute();
};

}

#endif

