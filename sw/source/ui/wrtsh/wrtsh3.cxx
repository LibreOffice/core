/*************************************************************************
 *
 *  $RCSfile: wrtsh3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
    GetDoc()->ExecMacro( rMacro, pRet, pArgs );
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
                    if( eButtonType == *((form::FormButtonType*)aTmp.getValue()))
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( C2U("Label") );

                        if( aTmp.getValueType() == ::getCppuType((OUString*)0))
                        {
                            OUString uTmp(*(OUString*)aTmp.getValue());
                            if(uTmp.getLength())
                                rDescr = String(uTmp);
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                        if( aTmp.getValueType() == ::getCppuType((const OUString*)0))
                        {
                            OUString uTmp(*(OUString*)aTmp.getValue());
                            if(uTmp.getLength())
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
                if (aProp.Name.len())
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


/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.65  2000/09/18 16:06:27  willem.vandorp
    OpenOffice header added.

    Revision 1.64  2000/09/07 15:59:36  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.63  2000/05/16 09:15:15  os
    project usr removed

    Revision 1.62  2000/04/19 11:18:25  os
    UNICODE

    Revision 1.61  2000/03/23 07:51:11  os
    UNO III

    Revision 1.60  2000/02/11 15:03:40  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.59  1999/11/19 16:40:25  os
    modules renamed

    Revision 1.58  1999/02/17 06:46:48  OS
    #61990# Property vor dem Aufruf auf Existenz pruefen


      Rev 1.57   17 Feb 1999 07:46:48   OS
   #61990# Property vor dem Aufruf auf Existenz pruefen

      Rev 1.56   27 Jan 1999 09:59:54   OS
   #56371# TF_ONE51

      Rev 1.55   24 Nov 1998 17:01:30   OM
   #59654# GPF durch Assertion behoben

      Rev 1.54   18 Nov 1998 16:43:22   OM
   #58898# Kein Statusupdate im SFX-Update

      Rev 1.53   02 Oct 1998 16:17:12   OM
   #57349# Shellwechsel nach Controlselektion durch Formularnavigator

      Rev 1.52   10 Jul 1998 15:31:28   OM
   Clipboard-Anbindung fuer util::URL-Button

      Rev 1.51   09 Jun 1998 15:34:36   OM
   VC-Controls entfernt

      Rev 1.50   04 May 1998 18:00:18   JP
   Bug #49993#: IsSelOver darf nicht immer den Cursor verschieben; GotoBookmark mit ReturntWert

      Rev 1.49   29 Nov 1997 15:15:06   MA
   includes

      Rev 1.48   24 Nov 1997 14:35:08   MA
   includes

      Rev 1.47   01 Sep 1997 13:22:00   OS
   DLL-Umstellung

      Rev 1.46   29 Jul 1997 17:09:48   TJ
   include VCURLButton

      Rev 1.45   02 May 1997 20:45:48   NF
   includes...

      Rev 1.44   09 Apr 1997 08:30:52   NF
   includes...

      Rev 1.43   07 Apr 1997 17:02:30   JP
   CallEvent: Callback als weiteren Parameter

      Rev 1.42   04 Apr 1997 16:51:34   HJS
   includes

      Rev 1.41   18 Feb 1997 17:08:08   JP
   CallEvent/ExecMacro: zusaetzlich Parameter fuers Basic

      Rev 1.40   22 Jan 1997 11:55:56   MA
   opt: bSelection entfernt

      Rev 1.39   09 Dec 1996 15:05:18   JP
   CallEvent: Parameter geaendert

      Rev 1.38   28 Nov 1996 20:28:44   JP
   Bug #33859#: CallEvent - Object muss ggfs. ueberprueft werden

      Rev 1.37   23 Nov 1996 16:44:38   JP
   neu: CallEvent - Event ans Basic oder script::JavaScript weitergeben

      Rev 1.36   21 Nov 1996 18:22:08   JP
   ExecMacro aus der WrtShell ins SwDoc verschoben

      Rev 1.35   19 Nov 1996 16:06:00   NF
   defines...

      Rev 1.34   08 Nov 1996 12:29:38   HJS
   include w.g. positivdefine

      Rev 1.33   10 Oct 1996 15:17:42   HJS
   clooks

      Rev 1.32   13 Sep 1996 18:10:48   JP
   neu: SetURLToButton - util::URL am util::URL-Button setzen

      Rev 1.31   29 Aug 1996 09:25:54   OS
   includes

      Rev 1.30   07 Aug 1996 18:24:20   HJS
   clooks

      Rev 1.29   27 Jun 1996 11:14:10   NF
   CLOOKS

      Rev 1.28   17 Jun 1996 13:34:00   TRI
   CLOOKS

      Rev 1.27   13 Jun 1996 15:43:34   MA
   split sihxx

      Rev 1.26   06 Jun 1996 15:40:14   HJS
   alte defs raus, neue rein...

      Rev 1.25   05 Jun 1996 15:33:00   JP
   ExecMacro: optional einen ErgebnisString erfragen

      Rev 1.24   05 Jun 1996 15:24:46   OS
   clook-defines hinter hdrstop

      Rev 1.23   05 Jun 1996 12:01:44   OV
   VCBrowser Umstellung

      Rev 1.22   10 May 1996 15:55:32   OS
   GotoBookmark markiert jetzt immer bug #27690#

      Rev 1.21   08 May 1996 12:59:40   OS
   MoveBookmark mit Start-Flag

 -------------------------------------------------------------------------*/
