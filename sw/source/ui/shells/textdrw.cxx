/*************************************************************************
 *
 *  $RCSfile: textdrw.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:52:22 $
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


#pragma hdrstop

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

#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
using namespace ::com::sun::star;
using namespace ::rtl;
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
    rSh.StartUndo( UIUNDO_INSERT_URLBTN );
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
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
            uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

            ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);


            uno::Any aTmp;

            aTmp <<= OUString(rTxt);
            xPropSet->setPropertyValue( C2U("Label"), aTmp );

            aTmp <<= OUString(INetURLObject::RelToAbs(rURL));
            xPropSet->setPropertyValue( C2U("TargetURL"), aTmp );

            if( rTarget.Len() )
            {
                aTmp <<= OUString(rTarget);
                xPropSet->setPropertyValue( C2U("TargetFrame"), aTmp );
            }


            form::FormButtonType eButtonType = form::FormButtonType_URL;
            aTmp.setValue( &eButtonType, ::getCppuType((const form::FormButtonType*)0));
            xPropSet->setPropertyValue( C2U("ButtonType"), aTmp );

            if ( Sound::IsSoundFile( rURL ) )
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
    rSh.EndUndo( UIUNDO_INSERT_URLBTN );
    rSh.EndAction();
}


