/*************************************************************************
 *
 *  $RCSfile: drformsh.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hjs $ $Date: 2003-09-25 10:51:07 $
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
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
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
            bool bConvertToText = rHLinkItem.GetInsertMode() == HLINK_DEFAULT ||
                            rHLinkItem.GetInsertMode() == HLINK_FIELD;
            const SdrMarkList& rMarkList = pSdrView->GetMarkList();
            if (rMarkList.GetMark(0))
            {
                SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
                if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                {
                    if(bConvertToText)
                    {
                        SwView& rView = rSh.GetView();
                        //remove object -> results in destruction of this!
                        rView.GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SFX_CALLMODE_SYNCHRON );
                        rView.StopShellTimer();
                        //issue a new command to insert the link
                        rView.GetViewFrame()->GetDispatcher()->Execute(
                                SID_HYPERLINK_SETLINK, SFX_CALLMODE_ASYNCHRON, &rHLinkItem, 0);
                    }
                    else
                    {
                        uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                        ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                        if( !xControlModel.is() )
                            return;

                        uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                        // Darf man eine URL an dem Objekt setzen?
                        OUString sTargetURL( C2U( "TargetURL" ));
                        uno::Reference< beans::XPropertySetInfo >  xPropInfoSet = xPropSet->getPropertySetInfo();
                        if( xPropInfoSet->hasPropertyByName( sTargetURL ))
                        {
                            beans::Property aProp = xPropInfoSet->getPropertyByName( sTargetURL );
                            if( aProp.Name.getLength() )
                            {
                                uno::Any aTmp;
                                // Ja!
                                aTmp <<= OUString(rHLinkItem.GetName());
                                xPropSet->setPropertyValue(C2U("Label"), aTmp );

                                aTmp <<=  OUString(INetURLObject::RelToAbs(rHLinkItem.GetURL()));
                                xPropSet->setPropertyValue( sTargetURL, aTmp );

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
                            if( aTmp >>= eButtonType )
                            {
                                // Label
                                if(xInfo->hasPropertyByName( C2U("Label") ))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("Label") );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && sTmp.getLength())
                                    {
                                        aHLinkItem.SetName(sTmp);
                                    }
                                }

                                // URL
                                if(xInfo->hasPropertyByName( C2U("TargetURL" )))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && sTmp.getLength())
                                    {
                                        aHLinkItem.SetURL(sTmp);
                                    }
                                }

                                // Target
                                if(xInfo->hasPropertyByName( C2U("TargetFrame") ))
                                {
                                    aTmp = xPropSet->getPropertyValue( C2U("TargetFrame") );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && sTmp.getLength())
                                    {
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



