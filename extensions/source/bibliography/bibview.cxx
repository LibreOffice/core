/*************************************************************************
 *
 *  $RCSfile: bibview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 12:58:00 $
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

#include "bib.hrc"
#include "bibcont.hxx"
#include "bibbeam.hxx"
#include "bibmod.hxx"
#include "general.hxx"
#include "bibview.hxx"
#include "datman.hxx"
#include "bibresid.hxx"
#include "bibmod.hxx"
#include "sections.hrc"


#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#define DEFAULT_SIZE 500
using namespace ::com::sun::star;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)

rtl::OUString   gSectionSizes(C2U("TheSectionSizes"));
rtl::OUString   gSectionNames(C2U("TheSectionNames"));
rtl::OUString   gSectionExpander(C2U("TheSectionExpander"));
rtl::OUString   gSectionPos(C2U("TheSectionPosition"));
// -----------------------------------------------------------------------
BibView::BibView(Window* pParent, BibDataManager* pDM, WinBits nStyle):
        Window(pParent,nStyle),
        pDatMan(pDM),
        xDatMan(pDM),
        pGeneralPage(NULL),
        nDefNameCount(0)
{
        String aDefName(BibResId(RID_BIB_STR_SECTION));
        aDefaultName=aDefName;
        UpdatePages();
}

BibView::~BibView()
{
    pGeneralPage->CommitActiveControl();
    uno::Reference< XForm >  xForm = pDatMan->getDatabaseForm();
    uno::Reference< XPropertySet >  xPrSet(xForm, UNO_QUERY);
    if(xPrSet.is())
    {
        uno::Any aProp = xPrSet->getPropertyValue(C2U("IsModified"));
        if(*(sal_Bool*)aProp.getValue())
        {
            uno::Any aNew = xPrSet->getPropertyValue(C2U("IsNew"));
            uno::Reference< sdbc::XResultSetUpdate >  xResUpd(xForm, UNO_QUERY);
            if(*(sal_Bool*)aNew.getValue())
                xResUpd->insertRow();
            else
                xResUpd->updateRow();
        }
    }
     //delete pGeneralPage;
     pGeneralPage->RemoveListeners();
     xGeneralPage = 0;

}
/* -----------------16.11.99 13:13-------------------

 --------------------------------------------------*/
void BibView::UpdatePages()
{
    BOOL bResize = FALSE;
    if(pGeneralPage)
    {
        bResize = TRUE;
        pGeneralPage->Hide();
//      delete pGeneralPage;
        pGeneralPage->RemoveListeners();
        xGeneralPage = 0;
    }
//  pGeneralPage = new BibGeneralPage(this, pDatMan);
    xGeneralPage = pGeneralPage = new BibGeneralPage(this, pDatMan);
    if(bResize)
        Resize();
    String sErrorString(pGeneralPage->GetErrorString());
    if(sErrorString.Len())
    {
        sErrorString += '\n';
        sErrorString += String(BibResId(RID_MAP_QUESTION));
        QueryBox aQuery( this, WB_YES_NO, sErrorString );
        if(RET_YES == aQuery.Execute())
        {
            Application::PostUserEvent( STATIC_LINK(
                    this, BibView, CallMappingHdl ) );

        }
    }

}
/* -----------------------------02.02.00 16:49--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_STATIC_LINK(BibView, CallMappingHdl, BibView*, EMPTYARG)
{
    pThis->pDatMan->CreateMappingDialog(pThis);
    return 0;
}
/* -----------------------------13.04.00 16:12--------------------------------

 ---------------------------------------------------------------------------*/
void BibView::Resize()
{
    if(pGeneralPage)
    {
        Size aSz(GetOutputSizePixel());
        pGeneralPage->SetSizePixel(aSz);
    }
    Window::Resize();
}




