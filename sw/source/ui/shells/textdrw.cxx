/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdrw.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:54:45 $
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
#include "precompiled_sw.hxx"



#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
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
    pSdrView->SetDesignMode(TRUE);
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
//          rSh.ChgAnchor(FLY_AT_CNTNT);
            rSh.UnSelectFrm();
        }
    }
    rSh.EndUndo( UNDO_UI_INSERT_URLBTN );
    rSh.EndAction();
}


