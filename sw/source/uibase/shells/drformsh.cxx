/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/hlnkitem.hxx>
#include <svx/svdview.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <sfx2/htmlmode.hxx>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>

#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <drwbassh.hxx>
#include <drformsh.hxx>
#include <svl/urihelper.hxx>
#include <view.hxx>
#include <sfx2/docfile.hxx>
#include <docsh.hxx>

#define ShellClass_SwDrawFormShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

using namespace ::com::sun::star;

SFX_IMPL_INTERFACE(SwDrawFormShell, SwDrawBaseShell)

void SwDrawFormShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(u"form"_ustr);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Text_Toolbox_Sw);
}


void SwDrawFormShell::Execute(SfxRequest const &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxPoolItem* pItem = nullptr;
    const SfxItemSet *pArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_HYPERLINK_SETLINK:
        {
        if(pArgs)
            pArgs->GetItemState(SID_HYPERLINK_SETLINK, false, &pItem);
        if(pItem)
        {
            SdrView *pSdrView = rSh.GetDrawView();
            const SvxHyperlinkItem& rHLinkItem = *static_cast<const SvxHyperlinkItem *>(pItem);
            bool bConvertToText = rHLinkItem.GetInsertMode() == HLINK_DEFAULT ||
                            rHLinkItem.GetInsertMode() == HLINK_FIELD;
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            if (rMarkList.GetMark(0))
            {
                SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
                {
                    if(bConvertToText)
                    {
                        //remove object -> results in destruction of this!
                        SwView& rTempView = GetView();
                        rTempView.GetViewFrame().GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON );
                        rTempView.StopShellTimer();
                        //issue a new command to insert the link
                        rTempView.GetViewFrame().GetDispatcher()->ExecuteList(
                                SID_HYPERLINK_SETLINK, SfxCallMode::ASYNCHRON,
                                { &rHLinkItem });
                    }
                    else
                    {
                        const uno::Reference< awt::XControlModel >&  xControlModel = pUnoCtrl->GetUnoControlModel();

                        OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
                        if( !xControlModel.is() )
                            return;

                        uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                        // Can we set a URL to the object?
                        OUString sTargetURL( u"TargetURL"_ustr );
                        uno::Reference< beans::XPropertySetInfo >  xPropInfoSet = xPropSet->getPropertySetInfo();
                        if( xPropInfoSet->hasPropertyByName( sTargetURL ))
                        {
                            beans::Property aProp = xPropInfoSet->getPropertyByName( sTargetURL );
                            if( !aProp.Name.isEmpty() )
                            {
                                uno::Any aTmp;
                                // Yes!
                                OUString sLabel(u"Label"_ustr);
                                if( xPropInfoSet->hasPropertyByName(sLabel) )
                                {
                                    aTmp <<= rHLinkItem.GetName();
                                    xPropSet->setPropertyValue(sLabel, aTmp );
                                }

                                SfxMedium* pMedium = GetView().GetDocShell()->GetMedium();
                                INetURLObject aAbs;
                                if( pMedium )
                                    aAbs = pMedium->GetURLObject();
                                aTmp <<= URIHelper::SmartRel2Abs(aAbs, rHLinkItem.GetURL());
                                xPropSet->setPropertyValue( sTargetURL, aTmp );

                                if( !rHLinkItem.GetTargetFrame().isEmpty() )
                                {
                                    aTmp <<= rHLinkItem.GetTargetFrame();
                                    xPropSet->setPropertyValue( u"TargetFrame"_ustr, aTmp );
                                }

                                aTmp <<= form::FormButtonType_URL;
                                xPropSet->setPropertyValue( u"ButtonType"_ustr, aTmp );
                            }
                        }
                    }
                }
            }
        }
        }
        break;

        default:
            OSL_ENSURE(false, "wrong dispatcher");
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
                    SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                    if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
                    {
                        const uno::Reference< awt::XControlModel >&  xControlModel = pUnoCtrl->GetUnoControlModel();

                        OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
                        if( !xControlModel.is() )
                            return;

                        uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                        uno::Any aTmp;
                        uno::Reference< beans::XPropertySetInfo >  xInfo = xPropSet->getPropertySetInfo();
                        if(xInfo->hasPropertyByName( u"ButtonType"_ustr ))
                        {
                            form::FormButtonType eButtonType = form::FormButtonType_URL;
                            aTmp = xPropSet->getPropertyValue( u"ButtonType"_ustr );
                            if( aTmp >>= eButtonType )
                            {
                                // Label
                                if(xInfo->hasPropertyByName( u"Label"_ustr ))
                                {
                                    aTmp = xPropSet->getPropertyValue( u"Label"_ustr );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && !sTmp.isEmpty())
                                    {
                                        aHLinkItem.SetName(sTmp);
                                    }
                                }

                                // URL
                                if(xInfo->hasPropertyByName( u"TargetURL"_ustr ))
                                {
                                    aTmp = xPropSet->getPropertyValue( u"TargetURL"_ustr );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && !sTmp.isEmpty())
                                    {
                                        aHLinkItem.SetURL(sTmp);
                                    }
                                }

                                // Target
                                if(xInfo->hasPropertyByName( u"TargetFrame"_ustr ))
                                {
                                    aTmp = xPropSet->getPropertyValue( u"TargetFrame"_ustr );
                                    OUString sTmp;
                                    if( (aTmp >>= sTmp) && !sTmp.isEmpty())
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
                aHLinkItem.SetInsertMode(static_cast<SvxLinkInsertMode>(aHLinkItem.GetInsertMode() |
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
    GetShell().NoEdit();
    SetName(u"DrawForm"_ustr);
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Form));
}

SwDrawFormShell::~SwDrawFormShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
