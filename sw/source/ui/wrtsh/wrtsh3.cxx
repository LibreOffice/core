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

#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sfx2/htmlmode.hxx>
#include "wrtsh.hxx"
#include "view.hxx"
#include "IMark.hxx"
#include "doc.hxx"
#include "wrtsh.hrc"

#include <unomid.h>

using namespace ::com::sun::star;

extern bool bNoInterrupt;       // in mainwn.cxx

sal_Bool SwWrtShell::MoveBookMark( BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark)
{
    addCurrentPosition();
    (this->*fnKillSel)( 0, sal_False );

    sal_Bool bRet = sal_True;
    switch(eFuncId)
    {
        case BOOKMARK_INDEX:bRet = SwCrsrShell::GotoMark( pMark );break;
        case BOOKMARK_NEXT: bRet = SwCrsrShell::GoNextBookmark();break;
        case BOOKMARK_PREV: bRet = SwCrsrShell::GoPrevBookmark();break;
        default:;//prevent warning
    }

    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }
    return bRet;
}

sal_Bool SwWrtShell::GotoField( const SwFmtFld& rFld )
{
    (this->*fnKillSel)( 0, sal_False );

    sal_Bool bRet = SwCrsrShell::GotoFld( rFld );
    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }

    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }

    return bRet;
}

bool SwWrtShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark)
{
    (this->*fnKillSel)( 0, sal_False );
    bool bRet = SwCrsrShell::GotoFieldmark(pMark);
    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }
    return bRet;
}

// Invalidate FontWork-Slots

void SwWrtShell::DrawSelChanged( )
{
    static sal_uInt16 const aInval[] =
    {
        SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, SID_ATTR_LINE_STYLE,
        SID_ATTR_LINE_WIDTH, SID_ATTR_LINE_COLOR,
        /*AF: these may be needed for the sidebar.
        SID_SVX_AREA_TRANSPARENCY, SID_SVX_AREA_TRANSP_GRADIENT,
        SID_SVX_AREA_TRANS_TYPE,
        */
        0
    };

    GetView().GetViewFrame()->GetBindings().Invalidate(aInval);

    bool bOldVal = bNoInterrupt;
    bNoInterrupt = true;    // Trick to run AttrChangedNotify by timer.
    GetView().AttrChangedNotify(this);
    bNoInterrupt = bOldVal;
}

sal_Bool SwWrtShell::GotoMark( const OUString& rName )
{
    IDocumentMarkAccess::const_iterator_t ppMark = getIDocumentMarkAccess()->findMark( rName );
    if(ppMark == getIDocumentMarkAccess()->getMarksEnd()) return false;
    return MoveBookMark( BOOKMARK_INDEX, ppMark->get() );
}

sal_Bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark )
{
    return MoveBookMark( BOOKMARK_INDEX, pMark );
}

sal_Bool SwWrtShell::GoNextBookmark()
{
    return MoveBookMark( BOOKMARK_NEXT );
}

sal_Bool SwWrtShell::GoPrevBookmark()
{
    return MoveBookMark( BOOKMARK_PREV );
}

void SwWrtShell::ExecMacro( const SvxMacro& rMacro, OUString* pRet, SbxArray* pArgs )
{
    // OD 11.02.2003 #100556# - execute macro, if it is allowed.
    if ( IsMacroExecAllowed() )
    {
        GetDoc()->ExecMacro( rMacro, pRet, pArgs );
    }
}


sal_uInt16 SwWrtShell::CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                                bool bChkPtr, SbxArray* pArgs,
                                const Link* pCallBack )
{
    return GetDoc()->CallEvent( nEvent, rCallEvent, bChkPtr, pArgs, pCallBack );
}


    // If a util::URL-Button is selected, return its util::URL
    // otherwise an emtpy string.
bool SwWrtShell::GetURLFromButton( String& rURL, String& rDescr ) const
{
    bool bRet = false;
    const SdrView *pDView = GetDrawView();
    if( pDView )
    {
        // A fly is precisely achievable if it is selected.
        const SdrMarkList &rMarkList = pDView->GetMarkedObjectList();

        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
                if( !xControlModel.is() )
                    return bRet;

                uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                uno::Any aTmp;

                form::FormButtonType eButtonType = form::FormButtonType_URL;
                uno::Reference< beans::XPropertySetInfo >   xInfo = xPropSet->getPropertySetInfo();
                if(xInfo->hasPropertyByName( "ButtonType" ))
                {
                    aTmp = xPropSet->getPropertyValue( "ButtonType" );
                    form::FormButtonType eTmpButtonType;
                    aTmp >>= eTmpButtonType;
                    if( eButtonType == eTmpButtonType)
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( "Label" );
                        OUString uTmp;
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rDescr = String(uTmp);
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( "TargetURL" );
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rURL = String(uTmp);
                        }
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
