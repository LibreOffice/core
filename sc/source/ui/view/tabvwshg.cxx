/*************************************************************************
 *
 *  $RCSfile: tabvwshg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:06:51 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

//#define SI_VCDRAWOBJ

#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

using namespace com::sun::star;

#include "tabvwsh.hxx"
#include "document.hxx"
#include "drawview.hxx"
#include "globstr.hrc"
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif

//------------------------------------------------------------------------

void ScTabViewShell::InsertURLButton( const String& rName, const String& rURL,
                                        const String& rTarget,
                                        const Point* pInsPos )
{
    //  Tabelle geschuetzt ?

    ScViewData* pViewData = GetViewData();
    if ( pViewData->GetDocument()->IsTabProtected(pViewData->GetTabNo()) )
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
                               pDrView->GetPageViewPvNum(0)->GetPage(), pModel);
    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);

    uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
    DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
    uno::Any aAny;

    aAny <<= rtl::OUString(rName);
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "Label" ), aAny );

    aAny <<= rtl::OUString(INetURLObject::RelToAbs(rURL));
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "TargetURL" ), aAny );

    if( rTarget.Len() )
    {
        aAny <<= rtl::OUString(rTarget);
        xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "TargetFrame" ), aAny );
    }

    form::FormButtonType eButtonType = form::FormButtonType_URL;
    aAny <<= eButtonType;
    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "ButtonType" ), aAny );

    if ( Sound::IsSoundFile( rURL ) )
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

    pObj->SetLogicRect(Rectangle(aPos, aSize));
//  pObj->Resize(Point(), Fraction(1, 1), Fraction(1, 1));

    //  am alten VC-Button musste die Position/Groesse nochmal explizit
    //  gesetzt werden - das scheint mit UnoControls nicht noetig zu sein

    //  nicht markieren wenn Ole
    pDrView->InsertObjectSafe( pObj, *pDrView->GetPageViewPvNum(0) );
}




