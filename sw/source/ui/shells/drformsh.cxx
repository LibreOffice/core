/*************************************************************************
 *
 *  $RCSfile: drformsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#include <hintids.hxx>
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
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
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#include "viewopt.hxx"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "drwbassh.hxx"
#include "drformsh.hxx"
#include "view.hxx"

#define SwDrawFormShell
#include "itemdef.hxx"
#include "swslots.hxx"

#define C2U(cChar) OUString::createFromAscii(cChar)

using namespace ::com::sun::star;
using namespace ::rtl;

SFX_IMPL_INTERFACE(SwDrawFormShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAWFORM))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAWFORM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAWFORM_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_DRAWFORM));
}


TYPEINIT1(SwDrawFormShell, SwDrawBaseShell)


void SwDrawFormShell::Execute(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxPoolItem* pItem = 0;
    const SfxItemSet *pArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_HYPERLINK_SETLINK:
        {
        if(pArgs)
            pArgs->GetItemState(SID_HYPERLINK_SETLINK, sal_False, &pItem);
        if(pItem)
        {
            SdrView *pSdrView = rSh.GetDrawView();
            const SvxHyperlinkItem& rHLinkItem = *(const SvxHyperlinkItem *)pItem;
            const SdrMarkList& rMarkList = pSdrView->GetMarkList();
            if (rMarkList.GetMark(0))
            {
                SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
                if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                {
                    uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                    ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                    if( !xControlModel.is() )
                        return;

                    uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                    uno::Any aTmp;

                    // Darf man eine URL an dem Objekt setzen?
                    uno::Reference< beans::XPropertySetInfo >  xPropInfoSet = xPropSet->getPropertySetInfo();
                    beans::Property aProp = xPropInfoSet->getPropertyByName( C2U("TargetURL" ));
                    if (aProp.Name.getLength())
                    {
                        // Ja!
                        aTmp <<= OUString(rHLinkItem.GetName());
                        xPropSet->setPropertyValue(C2U("Label"), aTmp );

                        aTmp <<=  OUString(INetURLObject::RelToAbs(rHLinkItem.GetURL()));
                        xPropSet->setPropertyValue( C2U("TargetURL"), aTmp );

                        if( rHLinkItem.GetTargetFrame().Len() )
                        {
                            aTmp <<=  OUString(rHLinkItem.GetTargetFrame());
                            xPropSet->setPropertyValue( C2U("TargetFrame"), aTmp );
                        }


                         form::FormButtonType eButtonType = form::FormButtonType_URL;
                        aTmp.setValue( &eButtonType, ::getCppuType((const form::FormButtonType*)0));
                        xPropSet->setPropertyValue( C2U("ButtonType"), aTmp );
                    }
                }
            }
        }
        }
        break;

        default:
            DBG_ASSERT(!this, "falscher Dispatcher");
            return;
    }
}

void SwDrawFormShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_HYPERLINK_GETLINK:
            {
                SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
                const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                SvxHyperlinkItem aHLinkItem;
                if (rMarkList.GetMark(0))
                {
                    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
                    if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                    {
                        uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                        ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                        if( !xControlModel.is() )
                            return;

                        uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                        uno::Any aTmp;
                        uno::Reference< beans::XPropertySetInfo >  xInfo = xPropSet->getPropertySetInfo();
                        if(xInfo->hasPropertyByName(C2U("ButtonType" )))
                        {
                             form::FormButtonType eButtonType = form::FormButtonType_URL;
                            aTmp = xPropSet->getPropertyValue( C2U("ButtonType") );
                            if( aTmp.getValueType() == ::getCppuType((form::FormButtonType*)0)
                                &&  eButtonType == *( form::FormButtonType*)aTmp.getValue())
                            {
                                // Label
                                if(xInfo->hasPropertyByName( C2U("Label") ))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("Label") );
                                    if( aTmp.getValueType() == ::getCppuType((const OUString*)0) )
                                    {
                                        OUString sTmp = *(OUString*)aTmp.getValue();
                                        if(sTmp.getLength())
                                            aHLinkItem.SetName(sTmp);
                                    }
                                }

                                // URL
                                if(xInfo->hasPropertyByName( C2U("TargetURL" )))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                                    if( aTmp.getValueType() == ::getCppuType((const OUString*)0))
                                    {
                                        OUString sTmp = *(OUString*)aTmp.getValue();
                                        if(sTmp.getLength())
                                            aHLinkItem.SetURL(sTmp);
                                    }
                                }

                                // Target
                                if(xInfo->hasPropertyByName( C2U("TargetFrame") ))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("TargetFrame") );
                                    if( aTmp.getValueType() == ::getCppuType((const OUString*)0))
                                    {
                                        OUString sTmp = *(OUString*) aTmp.getValue();
                                        if(sTmp.getLength())
                                            aHLinkItem.SetTargetFrame(sTmp);
                                    }
                                }
                                aHLinkItem.SetInsertMode(HLINK_BUTTON);
                            }
                        }
                    }
                }
                sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                    ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                rSet.Put(aHLinkItem);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


SwDrawFormShell::SwDrawFormShell(SwView &rView) :
    SwDrawBaseShell(rView)
{
    SetHelpId(SW_DRAWFORMSHELL);
    GetShell().NoEdit(sal_True);
    SetName(String::CreateFromAscii("DrawForm"));
}

SwDrawFormShell::~SwDrawFormShell()
{
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.19  2000/09/18 16:06:03  willem.vandorp
      OpenOffice header added.

      Revision 1.18  2000/05/26 07:21:32  os
      old SW Basic API Slots removed

      Revision 1.17  2000/05/16 09:15:13  os
      project usr removed

      Revision 1.16  2000/05/10 11:53:01  os
      Basic API removed

      Revision 1.15  2000/04/18 14:58:23  os
      UNICODE

      Revision 1.14  2000/03/23 07:49:14  os
      UNO III

      Revision 1.13  2000/02/11 14:57:20  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.12  1999/11/19 16:40:24  os
      modules renamed

      Revision 1.11  1999/09/24 14:38:32  os
      hlnkitem.hxx now in SVX

      Revision 1.10  1999/03/29 08:31:06  OS
      #64017# vor getPropertyValue immer hasPropertyByName rufen


      Rev 1.9   29 Mar 1999 10:31:06   OS
   #64017# vor getPropertyValue immer hasPropertyByName rufen

      Rev 1.8   26 Jan 1999 14:48:26   OS
   #56371# TF_ONE51

      Rev 1.7   26 Nov 1998 15:19:44   MA
   #59896# Raster usw jetzt an der Application

      Rev 1.6   02 Nov 1998 09:22:10   MA
   #58294# Slots fuer Raster

      Rev 1.5   07 Oct 1998 10:58:48   JP
   Bug #57153#: in allen GetStates einer DrawShell die MarkListe aktualisieren

      Rev 1.4   07 Jul 1998 14:03:54   OM
   #51853# URL-Button wiederbelebt

      Rev 1.3   06 Jul 1998 14:18:44   OM
   #51853# URL-Button einfuegen

      Rev 1.2   19 May 1998 12:45:50   OM
   SvxMacro-Umstellung

      Rev 1.1   08 Apr 1998 13:46:58   OM
   #42505 Keine URL-Buttons in Html-Dokumenten

      Rev 1.0   11 Mar 1998 17:06:02   OM
   Initial revision.

*************************************************************************/


