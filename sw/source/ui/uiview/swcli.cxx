/*************************************************************************
 *
 *  $RCSfile: swcli.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-01 12:31:34 $
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

#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SWCLI_HXX
#include <swcli.hxx>
#endif


SwOleClient::SwOleClient( SwView *pView, SwEditWin *pWin ) :
    SfxInPlaceClient( pView, pWin ), bInDoVerb( FALSE ),
    bOldCheckForOLEInCaption( pView->GetWrtShell().IsCheckForOLEInCaption() )
{
}


void SwOleClient::RequestObjAreaPixel( const Rectangle & rObjRect )
{
    if ( !GetEnv() )
        return;

    //Der Server moechte die Clientgrosse verandern.
    //Wir stecken die Wunschgroesse in die Core. Die Attribute des Rahmens
    //werden auf den Wunschwert eingestellt. Dieser Wert wird also auch an
    //den InPlaceClient weitergegeben.
    //Die Core aktzeptiert bzw. formatiert die eingestellten Werte nicht
    //zwangslaeufig. Wenn der Ole-Frm formatiert wurde wird das CalcAndSetScale()
    //der WrtShell gerufen. Dort wird ggf. die Scalierung des SwOleClient
    //eingestellt.

    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
    Window     *pWin = rSh.GetWin();

    SvContainerEnvironment *pEnv  = GetEnv();
    SvInPlaceEnvironment * pIPEnv = pEnv->GetIPEnv();
    ASSERT( pIPEnv, "RequestObjAreaPixel, Environment not found" );

    SvInPlaceObject * pIPObj = pIPEnv->GetIPObj();
    // Falls der Server nicht mit dem Maástab des Containers syncronisiert
    // ist, wird durch das Setzen und Abfragen der VisArea die Server
    // Einstellung ermittelt.
    // Niemals Koordinatentransformationen mit dem Rectangle vornehmen!!!
    Rectangle aLogRect( pEnv->PixelObjVisAreaToLogic( rObjRect ) );
    if ( pEnv->GetObjAreaPixel().GetSize() != rObjRect.GetSize() )
        // sichtbaren Ausschnitt setzen und abfragen
        aLogRect = pIPObj->SetGetVisArea( aLogRect );

    Size aBla( aLogRect.GetSize() );
    aBla.Width() = Fraction( aBla.Width()  ) * GetEnv()->GetScaleWidth();
    aBla.Height()= Fraction( aBla.Height() ) * GetEnv()->GetScaleHeight();
    aLogRect.SetSize( aBla );

    const MapMode aTmp( pIPObj->GetMapUnit() );
    aLogRect.SetSize( pWin->LogicToLogic( aLogRect.GetSize(), aTmp, MAP_TWIP ) );

    //#52207# Hat sich die Position wirklich geaendert (Umrechnungsfehler vermeiden)?
    if ( GetClientData()->GetObjAreaPixel().TopLeft() != rObjRect.TopLeft() )
        aLogRect.SetPos ( pWin->PixelToLogic( rObjRect.TopLeft()));
    else
        aLogRect.SetPos( Point( LONG_MIN, LONG_MIN ) );

    rSh.StartAllAction();
    rSh.RequestObjectResize( SwRect( aLogRect ), GetIPObj());
    rSh.EndAllAction();


    SwRect aFrm( rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,     0, GetIPObj() )),
           aPrt( rSh.GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, GetIPObj() ));
    Size  aSz ( aPrt.SSize() );
    Point aPos( aPrt.Pos() + aFrm.Pos() );
    aSz  = pWin->LogicToPixel( aSz );
    aPos = pWin->LogicToPixel( aPos);

    Rectangle aPixRect( aPos, aSz );
    SfxInPlaceClient::RequestObjAreaPixel( aPixRect );

    if ( !aFrm.IsOver( rSh.VisArea() ) )
        rSh.MakeVisible( aFrm );
}


void SwOleClient::ViewChanged( USHORT nAspect )
{
    SfxInPlaceClient::ViewChanged( nAspect );
    if ( !GetEnv() || bInDoVerb )
        return;

    const Rectangle aOldArea( GetEnv()->GetObjArea() );
    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
    Window     *pWin = rSh.GetWin();

    //Einstellen der Groesse des Objektes in der Core. Die Scalierung muss
    //beruecksichtigt werden. Rueckwirkung auf das Objekt werden von
    //CalcAndSetScale() der WrtShell beruecksichtig, wenn die Groesse/Pos des
    //Rahmens in der Core sich veraendert.
    SvEmbeddedObject *pObj = GetEmbedObj();
    Size aVisSize( pObj->GetVisArea().GetSize() );

    // Bug 24833: solange keine vernuenftige Size vom Object kommt,
    //              kann nichts skaliert werden
    if( !aVisSize.Width() || !aVisSize.Height() )
        return;

    aVisSize.Width() = Fraction( aVisSize.Width()  ) * GetEnv()->GetScaleWidth();
    aVisSize.Height()= Fraction( aVisSize.Height() ) * GetEnv()->GetScaleHeight();
    const MapMode aMyMap ( MAP_TWIP );
    const MapMode aObjMap( pObj->GetMapUnit() );
    aVisSize = OutputDevice::LogicToLogic( aVisSize, aObjMap, aMyMap );

    SwRect aRect( Point( LONG_MIN, LONG_MIN ), aVisSize );
    rSh.LockView( TRUE );   //Scrollen im EndAction verhindern
    rSh.StartAllAction();
    rSh.RequestObjectResize( aRect, GetIPObj() );
    rSh.EndAllAction();
    rSh.LockView( FALSE );
}


void SwOleClient::MakeVisible()
{
    SfxInPlaceClient::MakeVisible();

    const SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
    SvEmbeddedObject *pObj = GetEmbedObj();
    rSh.MakeObjVisible( pObj );
}


void SwOleClient::MakeViewData()
{
    SfxInPlaceClient::MakeViewData();

    SvClientData *pCD = GetClientData();
    if ( pCD )
    {
        SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
        SvEmbeddedObjectRef xObj = GetEmbedObj();
        rSh.CalcAndSetScale( xObj );
    }
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:48  hr
    initial import

    Revision 1.28  2000/09/18 16:06:11  willem.vandorp
    OpenOffice header added.

    Revision 1.27  2000/02/11 14:59:15  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.26  1999/05/11 19:50:16  JP
    Task #66127#: Methoden rund ums Lineal verbessert und Schnittstellen veraendert/erweitert


      Rev 1.25   11 May 1999 21:50:16   JP
   Task #66127#: Methoden rund ums Lineal verbessert und Schnittstellen veraendert/erweitert

      Rev 1.24   18 Dec 1998 17:56:52   JP
   Bug #60270#: im SwOleClient Flag fuers abklemmen der ViewChgMsg beim Aktivieren

      Rev 1.23   06 Jul 1998 15:19:08   MA
   #52207# Pixelfehler vermeiden

      Rev 1.22   21 Nov 1997 15:00:28   MA
   includes

      Rev 1.21   12 Nov 1997 12:31:08   TJ
   includes vertauscht wegen internal compiler error!

      Rev 1.20   03 Nov 1997 13:58:30   MA
   precomp entfernt

      Rev 1.19   04 Sep 1997 17:14:42   MA
   includes

      Rev 1.18   26 Nov 1996 17:35:02   MA
   Entschlackt

      Rev 1.17   29 Jul 1996 15:47:14   MA
   includes

      Rev 1.16   19 Apr 1996 14:04:10   HJS
   const entfernt

      Rev 1.15   18 Apr 1996 12:58:18   MA
   Umstellung 313

      Rev 1.14   18 Apr 1996 12:05:08   MA
   Umstellung 313

      Rev 1.13   18 Apr 1996 11:53:28   MA
   new: MakeVisible

      Rev 1.12   27 Mar 1996 12:27:30   MA
   fix: Request..(), ggf. MakeVisible rufen

      Rev 1.11   05 Mar 1996 15:51:04   MA
   chg: ein paar SS mit IPObj

      Rev 1.10   01 Feb 1996 19:11:30   JP
   Bug #24833#: Ohne VisSize den Rahmen nicht anzeigen

      Rev 1.9   19 Jan 1996 08:17:02   MA
   chg: EqualSize entfernt

      Rev 1.8   18 Jan 1996 13:20:30   MA
   Umstellung OLE

      Rev 1.7   09 Jan 1996 12:42:02   MA
   fix: ViewChanged, View locken damit nicht gescrollt wird.

      Rev 1.6   24 Nov 1995 16:58:02   OM
   PCH->PRECOMPILED

      Rev 1.5   13 Nov 1995 10:59:48   OM
   static entfernt

      Rev 1.4   20 Sep 1995 13:20:26   JP
   RequestObjectAreaPixel: teil auskommentiert

      Rev 1.3   14 Sep 1995 16:28:04   ER
   add: soipenv.hxx

      Rev 1.2   01 Sep 1995 16:11:14   SWG
   Umstellung 255

      Rev 1.1   14 Jul 1995 15:27:24   MA
   fix: SEXPORT's nachgeruestet.

      Rev 1.0   14 Jul 1995 11:57:52   MA
   new: auf eigenen OleClient umgestellt

------------------------------------------------------------------------*/


