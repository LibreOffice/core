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
#include "precompiled_sw.hxx"


#include <svx/svdview.hxx>
#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <view.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <swundo.hxx>
#include <basesh.hxx>

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#include <docsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <avmedia/mediawindow.hxx>

#include <unomid.h>


using namespace ::com::sun::star;
using ::rtl::OUString;
/*---------------------------------------------------------------------------
    Beschreibung:
 ----------------------------------------------------------------------------*/

void SwBaseShell::InsertURLButton(const String& rURL, const String& rTarget, const String& rTxt)
{
    SwWrtShell& rSh = GetShell();

    if (!rSh.HasDrawView())
        rSh.MakeDrawView();
    SdrView *pSdrView = rSh.GetDrawView();

    // OBJ_FM_BUTTON
    pSdrView->SetDesignMode(sal_True);
    pSdrView->SetCurrentObj(OBJ_FM_BUTTON);
    pSdrView->SetEditMode(sal_False);

    Point aStartPos(rSh.GetCharRect().Pos() + Point(0, 1));

    rSh.StartAction();
    rSh.StartUndo( UNDO_UI_INSERT_URLBTN );
    if (rSh.BeginCreate(OBJ_FM_BUTTON, FmFormInventor, aStartPos))
    {
        pSdrView->SetOrtho(sal_False);
         Size aSz(GetView().GetEditWin().PixelToLogic(Size(140, 20)));
        Point aEndPos(aSz.Width(), aSz.Height());

        rSh.MoveCreate(aStartPos + aEndPos);
        rSh.EndCreate(SDRCREATE_FORCEEND);

        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
            uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

            ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);


            uno::Any aTmp;

            aTmp <<= OUString(rTxt);
            xPropSet->setPropertyValue( C2U("Label"), aTmp );

            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();

            aTmp <<= OUString(URIHelper::SmartRel2Abs(aAbs, rURL));
            xPropSet->setPropertyValue( C2U("TargetURL"), aTmp );

            if( rTarget.Len() )
            {
                aTmp <<= OUString(rTarget);
                xPropSet->setPropertyValue( C2U("TargetFrame"), aTmp );
            }


            form::FormButtonType eButtonType = form::FormButtonType_URL;
            aTmp.setValue( &eButtonType, ::getCppuType((const form::FormButtonType*)0));
            xPropSet->setPropertyValue( C2U("ButtonType"), aTmp );

            if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
            {
                // #105638# OJ
                aTmp <<= sal_True;
                xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), aTmp );
            }
        }

        if (rSh.IsObjSelected())
        {
            rSh.UnSelectFrm();
        }
    }
    rSh.EndUndo( UNDO_UI_INSERT_URLBTN );
    rSh.EndAction();
}


