/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdview.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/srchitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <svx/htmlmode.hxx>
#include <tools/urlobj.hxx>

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
#include <svl/urihelper.hxx>
#include <view.hxx>
#include <sfx2/docfile.hxx>
#include <docsh.hxx>

#define SwDrawFormShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

#include <unomid.h>


using namespace ::com::sun::star;
using ::rtl::OUString;

SFX_IMPL_INTERFACE(SwDrawFormShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAWFORM))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAWFORM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
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
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            if (rMarkList.GetMark(0))
            {
                SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
                if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                {
                    if(bConvertToText)
                    {
                        //remove object -> results in destruction of this!
                        SwView& rTempView = GetView();
                        rTempView.GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SFX_CALLMODE_SYNCHRON );
                        rTempView.StopShellTimer();
                        //issue a new command to insert the link
                        rTempView.GetViewFrame()->GetDispatcher()->Execute(
                                SID_HYPERLINK_SETLINK, SFX_CALLMODE_ASYNCHRON, &rHLinkItem, 0);
                    }
                    else
                    {
                        uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                        OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
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
                                ::rtl::OUString sLabel(C2U("Label"));
                                if( xPropInfoSet->hasPropertyByName(sLabel) )
                                {
                                    aTmp <<= OUString(rHLinkItem.GetName());
                                    xPropSet->setPropertyValue(sLabel, aTmp );
                                }

                                SfxMedium* pMedium = GetView().GetDocShell()->GetMedium();
                                INetURLObject aAbs;
                                if( pMedium )
                                    aAbs = pMedium->GetURLObject();
                                aTmp <<=  OUString(URIHelper::SmartRel2Abs(aAbs, rHLinkItem.GetURL()));
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
            OSL_ENSURE(!this, "wrong dispatcher");
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
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                SvxHyperlinkItem aHLinkItem;
                if (rMarkList.GetMark(0))
                {
                    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
                    if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                    {
                        uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                        OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
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


SwDrawFormShell::SwDrawFormShell(SwView &_rView) :
    SwDrawBaseShell(_rView)
{
    SetHelpId(SW_DRAWFORMSHELL);
    GetShell().NoEdit(sal_True);
    SetName(String::CreateFromAscii("DrawForm"));
}

SwDrawFormShell::~SwDrawFormShell()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
