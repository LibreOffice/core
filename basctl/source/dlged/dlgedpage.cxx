/*************************************************************************
 *
 *  $RCSfile: dlgedpage.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-26 10:48:07 $
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

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXVIEW_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SVDIO_HXX
#include <svx/svdio.hxx>
#endif

#include <vcl/color.hxx>

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif

#pragma hdrstop

#include <svx/svdobj.hxx>

#include <svx/svxids.hrc>

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif


TYPEINIT1( DlgPage, SdrPage );

//----------------------------------------------------------------------------

DlgPage::DlgPage( DlgEdModel& rModel, StarBASIC* pBas, FASTBOOL bMasterPage ) :
    SdrPage( rModel, bMasterPage ),
    pBasic(pBas)
{
    if( pBasic )
        pBasic->AddRef();
    //pPool = rModel.pVCItemPool; // GetVCItemPool() wuerde ggf. einen erzeugen
}

//----------------------------------------------------------------------------

DlgPage::~DlgPage()
{
    //if( pBasic && pSbxForm && pSbxForm->Type() == TYPE( VCSbxForm ) )
    //  pBasic->Remove( pSbxForm );

    Clear();

    //if( pVCManager )
    //  delete pVCManager;

    if( pBasic )
        pBasic->ReleaseRef();

}

//----------------------------------------------------------------------------

void DlgPage::SetName( const String& rName )
{
    aPageName = rName;
    //if( pSbxForm )
    //  pSbxForm->SetName( aPageName );
}

//----------------------------------------------------------------------------

void DlgPage::SetBasic( StarBASIC* pBas )
{
    if( pBas == pBasic )
        return;

    StarBASIC* pOldBasic = pBasic;
    pBasic = pBas;

    if( pBasic )
        pBasic->AddRef();
    /*
    if( pSbxForm )
    {
        if( pOldBasic )
            pOldBasic->Remove( pSbxForm );
        if( pBasic )
            pBasic->Insert( pSbxForm );
    }
    */
    if( pOldBasic )
        pOldBasic->ReleaseRef();
}

//----------------------------------------------------------------------------

BOOL DlgPage::RequestHelp( Window* pWindow, SdrView* pView,
                              const HelpEvent& rEvt )
{
    if( !pView->IsAction() )
    {
        Point aPos = rEvt.GetMousePosPixel();
        aPos = pWindow->ScreenToOutputPixel( aPos );
        aPos = pWindow->PixelToLogic( aPos );

        SdrObject* pObj;
        SdrPageView* pPV;
        if( pView->PickObj( aPos, 0, pObj, pPV ) )
        {
            /*
            // Ein Object getroffen
            if( pObj->IsA( TYPE( VCSbxDrawObject ) ) )
            {
                SfxItemSet* pItemSet = ((VCSbxDrawObject*)pObj)->GetItemSet();
                String aHelpText = ((SfxStringItem&)pItemSet->Get( VCATTR_HELPTEXT )).GetValue();
                if( !aHelpText.Len() )
                {
                    // keonnen wir eine URL anzeigen?
                    VCSbxDrawObject* pControl = (VCSbxDrawObject*)pObj;
                    if( pControl->GetControl()->IsA(TYPE(VCSbxURLButton)))
                    {
                        VCURLButton* pBtn = (VCURLButton*)(pControl->GetControl()->GetVCControl());
                        aHelpText = pBtn->GetURL();
                        INetURLObject aUrl( aHelpText );
                        aHelpText = aUrl.GetURLNoPass();
                    }
                }
                if( aHelpText.Len())
                {
                    // Hilfe anzeigen
                    Rectangle aItemRect = pObj->GetBoundRect();
                    aItemRect = pWindow->LogicToPixel( aItemRect );
                    Point aPt = pWindow->OutputToScreenPixel( aItemRect.TopLeft() );
                    aItemRect.Left()   = aPt.X();
                    aItemRect.Top()    = aPt.Y();
                    aPt = pWindow->OutputToScreenPixel( aItemRect.BottomRight() );
                    aItemRect.Right()  = aPt.X();
                    aItemRect.Bottom() = aPt.Y();
                    if( rEvt.GetMode() == HELPMODE_BALLOON )
                        Help::ShowBalloon( pWindow, aItemRect.Center(), aItemRect, aHelpText);
                    else
                        Help::ShowQuickHelp( pWindow, aItemRect, aHelpText );


                }
                return TRUE;
            }
            */
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------

void DlgPage::RequestBasic()
{
}

//----------------------------------------------------------------------------

void DlgPage::WriteData( SvStream& rOut ) const
{

    /*
    CharSet eOldCharSet = rOut.GetStreamCharSet( );
    rOut.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );

    if( pSbxForm )
    {
        SdrDownCompat aCompat( rOut, STREAM_WRITE);
        rOut << (UINT16)1;
        UINT32 nMyVer = 0x13081961;
        rOut << nMyVer;
        rOut << *pSbxForm;
    }
    else
    {
        SdrDownCompat aCompat( rOut, STREAM_WRITE);
        rOut << (UINT16)0;
    }

    SdrPage::WriteData( rOut );

    SdrDownCompat aCompat( rOut, STREAM_WRITE);
    rOut.WriteByteString( GetName() );

    // Magic-Number schreiben, da Carsten vergessen hat,
    // eine (erhoehbare!) Versionsnummer einzubauen
    rOut << (ULONG)0x11051967;
    rOut << (ULONG)0x19670511;

    USHORT nVer = 1;
    rOut << nVer; // Versionsnummer innerhalb meines Blockes
    {
        SdrDownCompat aCompat( rOut, STREAM_WRITE);
        // VCForms speichern
        ULONG nFormCount = 0;
        if( pVCManager )
        {
            nFormCount = pVCManager->FormCount();
            rOut << nFormCount;
            for( ULONG nCurForm=0; nCurForm < nFormCount; nCurForm++ )
            {
                rOut << *(pVCManager->GetFormByPos(nCurForm));
            }
        }
        else
            rOut << nFormCount;
    }
    rOut.SetStreamCharSet( eOldCharSet );
    */
}

//----------------------------------------------------------------------------

void DlgPage::ReadData( const SdrIOHeader& rHead, SvStream& rIn )
{
    /*
    // macht JOE jetzt
    CharSet eOldCharSet = rIn.GetStreamCharSet( );
    rIn.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );

    {
        SdrDownCompat aCompat( rIn, STREAM_READ );
        UINT16 n;
        rIn >> n;
        if( n )
        {
            GetSbxForm();

            UINT32 nMyVer;
            rIn >> nMyVer;
            if( nMyVer == 0x13081961 )
                rIn >> *pSbxForm;
        }
    }

    // Joe liest jetzt auch die Dok-Controls ein!
    SdrPage::ReadData( rHead, rIn );

    {
        SdrDownCompat aCompat( rIn, STREAM_READ );
        String aName;
        rIn.ReadByteString( aName );
        SetName( aName );

        ULONG nMagic1=0, nMagic2=0;
        ULONG nFPos = rIn.Tell();
        // Magic-Number lesen
        rIn >> nMagic1; rIn >> nMagic2;
        if( nMagic1 == 0x11051967 && nMagic2 == 0x19670511 )
        {
            // es ist noch was da
            USHORT nVer;
            rIn >> nVer; // Versionsnummer innerhalb meines Blockes
            {
                SdrDownCompat aCompat( rIn, STREAM_READ);
                // VCForms laden
                ULONG nFormCount;
                rIn >> nFormCount;
                if( nFormCount )
                {
                    // diese Dummy-Referenz  sorgt dafuer, dass ein
                    // VCManager angelegt wird, falls noch keiner da ist
                    VCSbxFormRef aDummyRef = DlgPage::GetSbxForm();
                    DBG_ASSERT(pVCManager,"No VCManager!");
                    for( ULONG nCurForm=0; nCurForm < nFormCount; nCurForm++ )
                    {
                        VCForm* pForm = new VCForm;
                        rIn >> *pForm;
                        pVCManager->InsertForm( pForm );
                    }
                }
            }
        }
        else
            rIn.Seek( nFPos );

        // Ueber alle Controls gehen und die Dinger in ihre Forms einfuegen.
        // Das kann ich nicht im VCSbxDrawObject::ReadData machen, weil
        // die Forms dann noch nicht geladen sind!
        if( pVCManager )
        {
            ULONG nCtrlCount = pVCManager->GetChildCount();
            for( ULONG nCur=0; nCur < nCtrlCount; nCur++ )
            {
                VCControl* pVCControl = pVCManager->GetChildByPos(nCur);
                String aFormName( pVCControl->GetFormName() );
                if( aFormName.Len() )
                {
                    // in eine neue Form packen, diese ggf. erzeugen
                    VCForm* pForm = pVCManager->GetFormByName( aFormName );
                    DBG_ASSERT(pForm,"No Form!");
                    pVCControl->SetVCForm( pForm );
                }
            }
        }
    }
    rIn.SetStreamCharSet( eOldCharSet );
    */
}

//----------------------------------------------------------------------------

void DlgPage::SetModel(SdrModel* pNewModel)
{
    DBG_ASSERT(pNewModel,"DlgPage::SetModel: pNewModel==0");
    SdrModel* pOldModel = GetModel();
    SdrPage::SetModel( pNewModel );
    /*
    if( pOldModel != pNewModel )
    {
        VCItemPool* pNewPool = ((VCDrawModel*)pNewModel)->GetVCItemPool();
        if( pVCManager )
        {
            ULONG nCount = pVCManager->GetChildCount();
            for( ULONG nCur = 0; nCur < nCount; nCur++ )
            {
                VCControl* pCtrl = pVCManager->GetChildByPos( nCur );
                pCtrl->SetPool( pNewPool );
            }
        }
        if( pSbxForm && pSbxForm->Type() == TYPE(VCSbxForm) )
            pSbxForm->SetPool( pNewPool, FALSE );
    }
    */
}

//----------------------------------------------------------------------------

SdrPage* DlgPage::Clone() const
{
    return new DlgPage( *this );
}

//----------------------------------------------------------------------------

DlgPage::DlgPage( const DlgPage& rPage )
    : SdrPage( rPage ),
    pBasic(0)
{
    //pPool = ((DlgEdModel*)GetModel())->pVCItemPool; // GetVCItemPool() wuerde ggf. einen erzeugen
}

//----------------------------------------------------------------------------


