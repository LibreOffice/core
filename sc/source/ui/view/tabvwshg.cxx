/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabvwshg.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 17:00:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

//#define SI_VCDRAWOBJ

#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

using namespace com::sun::star;

#include "tabvwsh.hxx"
#include "document.hxx"
#include "drawview.hxx"
#include "globstr.hrc"
#include <avmedia/mediawindow.hxx>

//------------------------------------------------------------------------

void ScTabViewShell::InsertURLButton( const String& rName, const String& rURL,
                                        const String& rTarget,
                                        const Point* pInsPos )
{
    //  Tabelle geschuetzt ?

    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if ( pDoc->IsTabProtected(nTab) )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    MakeDrawLayer();

    ScTabView*  pView   = pViewData->GetView();
//  SdrView*    pDrView = pView->GetSdrView();
    ScDrawView* pDrView = pView->GetScDrawView();
    SdrModel*   pModel  = pDrView->GetModel();

    SdrObject* pObj = SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                               pDrView->GetSdrPageView()->GetPage(), pModel);
    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);

    uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
    DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
    uno::Any aAny;

    aAny <<= rtl::OUString(rName);
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "Label" ), aAny );

    ::rtl::OUString aTmp = INetURLObject::GetAbsURL( pDoc->GetDocumentShell()->GetMedium()->GetBaseURL(), rURL );
    aAny <<= aTmp;
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "TargetURL" ), aAny );

    if( rTarget.Len() )
    {
        aAny <<= rtl::OUString(rTarget);
        xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "TargetFrame" ), aAny );
    }

    form::FormButtonType eButtonType = form::FormButtonType_URL;
    aAny <<= eButtonType;
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "ButtonType" ), aAny );

        if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
    {
        // #105638# OJ
        aAny <<= sal_True;
        xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), aAny );
    }

    Point aPos;
    if (pInsPos)
        aPos = *pInsPos;
    else
        aPos = GetInsertPos();

    // Groesse wie in 3.1:
    Size aSize = GetActiveWin()->PixelToLogic(Size(140, 20));

    if ( pDoc->IsNegativePage(nTab) )
        aPos.X() -= aSize.Width();

    pObj->SetLogicRect(Rectangle(aPos, aSize));
//  pObj->Resize(Point(), Fraction(1, 1), Fraction(1, 1));

    //  am alten VC-Button musste die Position/Groesse nochmal explizit
    //  gesetzt werden - das scheint mit UnoControls nicht noetig zu sein

    //  nicht markieren wenn Ole
    pDrView->InsertObjectSafe( pObj, *pDrView->GetSdrPageView() );
}




