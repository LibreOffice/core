/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <svx/svdlegacy.hxx>

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
    const basegfx::B2DPoint* pInsPos )
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

//  ScTabView*  pView   = pViewData->GetView();
//  SdrView*    pDrView = pView->GetSdrView();
//  ScDrawView* pDrView = pView->GetScDrawView();
//  SdrModel*   pModel  = pDrView->getSdrModelFromSdrView();

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pViewData->GetView()->GetSdrView()->getSdrModelFromSdrView(),
        SdrObjectCreationInfo(OBJ_FM_BUTTON, FmFormInventor));
    SdrUnoObj* pUnoCtrl = dynamic_cast< SdrUnoObj* >( pObj);

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

    basegfx::B2DPoint aPos;

    if (pInsPos)
        aPos = *pInsPos;
    else
        aPos = GetInsertPos();

    // Groesse wie in 3.1:
    const basegfx::B2DVector aScale(GetActiveWin()->GetInverseViewTransformation() * basegfx::B2DVector(140.0, 20.0));

    if ( pDoc->IsNegativePage(nTab) )
    {
        aPos.setX(aPos.getX() - aScale.getX());
    }

    sdr::legacy::SetLogicRange(*pObj, basegfx::B2DRange(aPos, aPos + aScale));
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    pDrView->InsertObjectSafe( *pObj );
}




