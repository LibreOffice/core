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
#include <svx/htmlmode.hxx>
#include "wrtsh.hxx"
#include "view.hxx"
#include "IMark.hxx"
#include "doc.hxx"
#include "wrtsh.hrc"

#include <unomid.h>


using namespace ::com::sun::star;
using ::rtl::OUString;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

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

/*--------------------------------------------------------------------
    Beschreibung: FontWork-Slots invalidieren
 --------------------------------------------------------------------*/


void SwWrtShell::DrawSelChanged( )
{
    static sal_uInt16 const aInval[] =
    {
        SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, SID_ATTR_LINE_STYLE,
        SID_ATTR_LINE_WIDTH, SID_ATTR_LINE_COLOR, 0
    };

    GetView().GetViewFrame()->GetBindings().Invalidate(aInval);

    sal_Bool bOldVal = bNoInterrupt;
    bNoInterrupt = sal_True;    // Trick, um AttrChangedNotify ueber Timer auszufuehren
    GetView().AttrChangedNotify(this);
    bNoInterrupt = bOldVal;
}

sal_Bool SwWrtShell::GotoMark( const ::rtl::OUString& rName )
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


void SwWrtShell::ExecMacro( const SvxMacro& rMacro, String* pRet, SbxArray* pArgs )
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


    // fall ein util::URL-Button selektiert ist, dessen util::URL returnen, ansonsten
    // einen LeerString
sal_Bool SwWrtShell::GetURLFromButton( String& rURL, String& rDescr ) const
{
    sal_Bool bRet = sal_False;
    const SdrView *pDView = GetDrawView();
    if( pDView )
    {
        // Ein Fly ist genau dann erreichbar, wenn er selektiert ist.
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
                if(xInfo->hasPropertyByName( C2U("ButtonType") ))
                {
                    aTmp = xPropSet->getPropertyValue( C2U("ButtonType") );
                    form::FormButtonType eTmpButtonType;
                    aTmp >>= eTmpButtonType;
                    if( eButtonType == eTmpButtonType)
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( C2U("Label") );
                        OUString uTmp;
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rDescr = String(uTmp);
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rURL = String(uTmp);
                        }
                        bRet = sal_True;
                    }
                }
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
