/*************************************************************************
 *
 *  $RCSfile: drawsh5.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:56 $
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

//------------------------------------------------------------------

#include <sfx2/request.hxx>
#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdouno.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------

void ScDrawShell::GetHLinkState( SfxItemSet& rSet )             //  Hyperlink
{
    ScDrawView* pView = pViewData->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

        //  Hyperlink

    SvxHyperlinkItem aHLinkItem;

    if ( nMarkCount == 1 )              // URL-Button markiert ?
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);
        if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
        {
            uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
            DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

            rtl::OUString sPropButtonType  = rtl::OUString::createFromAscii( "ButtonType" );
            rtl::OUString sPropTargetURL   = rtl::OUString::createFromAscii( "TargetURL" );
            rtl::OUString sPropTargetFrame = rtl::OUString::createFromAscii( "TargetFrame" );
            rtl::OUString sPropLabel       = rtl::OUString::createFromAscii( "Label" );

            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                {
                    rtl::OUString sTmp;
                    // Label
                    if(xInfo->hasPropertyByName( sPropLabel ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropLabel );
                        if ( (aAny >>= sTmp) && sTmp.len() )
                        {
                            aHLinkItem.SetName(sTmp);
                        }
                    }
                    // URL
                    if(xInfo->hasPropertyByName( sPropTargetURL ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetURL );
                        if ( (aAny >>= sTmp) && sTmp.len() )
                        {
                            aHLinkItem.SetURL(sTmp);
                        }
                    }
                    // Target
                    if(xInfo->hasPropertyByName( sPropTargetFrame ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetFrame );
                        if ( (aAny >>= sTmp) && sTmp.len() )
                        {
                            aHLinkItem.SetTargetFrame(sTmp);
                        }
                    }
                    aHLinkItem.SetInsertMode(HLINK_BUTTON);
                }
            }
        }
    }

    rSet.Put(aHLinkItem);
}

void ScDrawShell::ExecuteHLink( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName     = pHyper->GetName();
                    const String& rURL      = pHyper->GetURL();
                    const String& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    BOOL bDone = FALSE;
                    if ( eMode == HLINK_DEFAULT || eMode == HLINK_BUTTON )
                    {
                        ScDrawView* pView = pViewData->GetScDrawView();
                        const SdrMarkList& rMarkList = pView->GetMarkList();
                        if ( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
                            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                            {
                                uno::Reference<awt::XControlModel> xControlModel =
                                                        pUnoCtrl->GetUnoControlModel();
                                DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                                if( !xControlModel.is() )
                                    return;

                                uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                                uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

                                rtl::OUString sPropTargetURL =
                                    rtl::OUString::createFromAscii( "TargetURL" );

                                // Darf man eine URL an dem Objekt setzen?
                                if (xInfo->hasPropertyByName( sPropTargetURL ))
                                {
                                    // Ja!

                                    rtl::OUString sPropButtonType =
                                        rtl::OUString::createFromAscii( "ButtonType" );
                                    rtl::OUString sPropTargetFrame =
                                        rtl::OUString::createFromAscii( "TargetFrame" );
                                    rtl::OUString sPropLabel =
                                        rtl::OUString::createFromAscii( "Label" );

                                    uno::Any aAny;
                                    aAny <<= rtl::OUString(rName);
                                    xPropSet->setPropertyValue( sPropLabel, aAny );

                                    aAny <<= rtl::OUString(INetURLObject::RelToAbs(rURL));
                                    xPropSet->setPropertyValue( sPropTargetURL, aAny );

                                    if( rTarget.Len() )
                                    {
                                        aAny <<= rtl::OUString(rTarget);
                                        xPropSet->setPropertyValue( sPropTargetFrame, aAny );
                                    }

                                    form::FormButtonType eButtonType = form::FormButtonType_URL;
                                    aAny <<= eButtonType;
                                    xPropSet->setPropertyValue( sPropButtonType, aAny );

                                    //! Undo ???
                                    pViewData->GetDocShell()->SetDocumentModified();
                                    bDone = TRUE;
                                }
                            }
                        }
                    }

                    if (!bDone)
                        pViewData->GetViewShell()->
                            InsertURL( rName, rURL, rTarget, (USHORT) eMode );

                    //  InsertURL an der ViewShell schaltet bei "Text" die DrawShell ab !!!
                }
            }
            break;
        default:
            DBG_ERROR("falscher Slot");
    }
}



