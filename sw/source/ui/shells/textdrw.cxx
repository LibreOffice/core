/*************************************************************************
 *
 *  $RCSfile: textdrw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
using namespace ::com::sun::star;
using namespace ::rtl;
/*---------------------------------------------------------------------------
    Beschreibung:
 ----------------------------------------------------------------------------*/

void SwTextShell::InsertURLButton(const String& rURL, const String& rTarget, const String& rTxt)
{
    SwWrtShell& rSh = GetShell();

    if (!rSh.HasDrawView())
        rSh.MakeDrawView();
    SdrView *pSdrView = rSh.GetDrawView();

    // OBJ_FM_BUTTON
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

        const SdrMarkList& rMarkList = pSdrView->GetMarkList();
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.36  2000/09/18 16:06:05  willem.vandorp
    OpenOffice header added.

    Revision 1.35  2000/05/16 09:15:14  os
    project usr removed

    Revision 1.34  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.33  2000/03/23 07:49:14  os
    UNO III

    Revision 1.32  2000/02/11 14:57:44  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.31  1999/11/19 16:40:24  os
    modules renamed

    Revision 1.30  1999/01/26 13:48:26  OS
    #56371# TF_ONE51


      Rev 1.29   26 Jan 1999 14:48:26   OS
   #56371# TF_ONE51

      Rev 1.28   21 Jan 1999 23:59:00   JP
   Bug #61062#: InsertURL mit eigener UndoId

      Rev 1.27   07 Jul 1998 14:03:54   OM
   #51853# util::URL-Button wiederbelebt

      Rev 1.26   06 Jul 1998 14:18:46   OM
   #51853# util::URL-Button einfuegen

      Rev 1.25   09 Jun 1998 15:35:58   OM
   VC-Controls entfernt

      Rev 1.24   29 Nov 1997 15:52:54   MA
   includes

      Rev 1.23   24 Nov 1997 09:47:08   MA
   includes

      Rev 1.22   03 Nov 1997 13:55:46   MA
   precomp entfernt

      Rev 1.21   05 Sep 1997 12:08:16   MH
   chg: header

      Rev 1.20   29 Jul 1997 16:00:16   TJ
   include VCURLButton

      Rev 1.19   04 Apr 1997 16:41:46   HJS
   includes

      Rev 1.18   08 Nov 1996 11:59:22   HJS
   include w.g. positivdefine

      Rev 1.17   28 Aug 1996 15:55:04   OS
   includes

      Rev 1.16   15 Aug 1996 13:44:24   JP
   svdraw.hxx entfernt

      Rev 1.15   11 Jul 1996 12:15:44   OM
   #29283# Buttons zeichengebunden einfuegen

      Rev 1.14   26 Jun 1996 15:21:00   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.13   25 Jun 1996 22:07:12   JP
   SmartRelToAbs: Flag fuer CutMark hat jetzt inverse Bedeutung

      Rev 1.12   18 Jun 1996 12:29:36   JP
   SmartRelToAbs: text::Bookmark soll beibehalten werden

      Rev 1.11   13 Jun 1996 15:55:36   MA
   splitt si.hxx

      Rev 1.10   12 Jun 1996 15:09:18   OM
   Target am util::URL-Button setzen

      Rev 1.9   12 Jun 1996 14:56:48   OM
   Target aus Hyperlinkleiste uebernehmen

      Rev 1.8   31 May 1996 11:17:14   NF
   CLOOKS

      Rev 1.7   11 Apr 1996 17:22:40   JP
   Umstellung - RelToAbs/AbsToRel

------------------------------------------------------------------------*/


