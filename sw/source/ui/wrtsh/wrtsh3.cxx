/*************************************************************************
 *
 *  $RCSfile: wrtsh3.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:45:17 $
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

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#include "wrtsh.hxx"
#include "view.hxx"
#include "bookmrk.hxx"
#include "doc.hxx"
#include "wrtsh.hrc"

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

using namespace ::com::sun::star;
using namespace ::rtl;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

FASTBOOL SwWrtShell::MoveBookMark(  BookMarkMove eFuncId,
                                    sal_uInt16 nPos,
                                    sal_Bool bStart )
{
//JP 08.03.96: die Wizards brauchen die Selektion !!
//  EndSelect();
    (this->*fnKillSel)( 0, sal_False );

    FASTBOOL bRet = sal_True;
    switch(eFuncId)
    {
        case BOOKMARK_INDEX:bRet = SwCrsrShell::GotoBookmark( nPos );break;
        case BOOKMARK_NEXT: bRet = SwCrsrShell::GoNextBookmark();break;
        case BOOKMARK_PREV: bRet = SwCrsrShell::GoPrevBookmark();break;
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


/*--------------------------------------------------------------------
    Beschreibung: FontWork-Slots invalidieren
 --------------------------------------------------------------------*/


void SwWrtShell::DrawSelChanged(SdrView* pView)
{
    static sal_uInt16 __READONLY_DATA aInval[] =
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

FASTBOOL SwWrtShell::GotoBookmark( const String& rName )
{
    sal_uInt16 nPos = FindBookmark( rName );
    if( USHRT_MAX == nPos )
        return sal_False;

    return MoveBookMark( BOOKMARK_INDEX, nPos );
}


FASTBOOL SwWrtShell::GotoBookmark( sal_uInt16 nPos )
{
    return MoveBookMark( BOOKMARK_INDEX, nPos );
}


FASTBOOL SwWrtShell::GoNextBookmark()
{
    return MoveBookMark( BOOKMARK_NEXT );
}


FASTBOOL SwWrtShell::GoPrevBookmark()
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
                                sal_Bool bChkPtr, SbxArray* pArgs,
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
        const SdrMarkList &rMarkList = pDView->GetMarkList();

        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
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
                        if( (aTmp >>= uTmp) && uTmp.getLength())
                        {
                            rDescr = String(uTmp);
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                        if( (aTmp >>= uTmp) && uTmp.getLength())
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


    // falls ein util::URL-Button selektiert ist, dessen util::URL returnen, ansonsten
    // einen LeerString
sal_Bool SwWrtShell::SetURLToButton( const String& rURL, const String& rDescr )
{
    sal_Bool bRet = sal_False;
    const SdrView *pDView = GetDrawView();
    if( pDView )
    {
        // Ein Fly ist genau dann erreichbar, wenn er selektiert ist.
        const SdrMarkList &rMarkList = pDView->GetMarkList();
        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                if( !xControlModel.is() )
                    return bRet;

                uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                uno::Any aTmp;

                // Darf man eine util::URL an dem Objekt setzen?
                uno::Reference< beans::XPropertySetInfo >  xPropInfoSet = xPropSet->getPropertySetInfo();
                beans::Property aProp = xPropInfoSet->getPropertyByName( C2U("TargetURL") );
                if (aProp.Name.getLength())
                {
                    // Ja!
                    aTmp <<= OUString(rDescr);
                    xPropSet->setPropertyValue( C2U("Label"), aTmp );

                    aTmp <<= OUString(rURL);
                    xPropSet->setPropertyValue( C2U("TargetURL"), aTmp );


                    form::FormButtonType eButtonType = form::FormButtonType_URL;
                    aTmp.setValue( &eButtonType, ::getCppuType((form::FormButtonType*)0));
                    xPropSet->setPropertyValue( C2U("ButtonType"), aTmp );
                }
            }
        }
    }

    return bRet;
}



