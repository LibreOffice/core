/*************************************************************************
 *
 *  $RCSfile: eventdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:11:49 $
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

// include ***************************************************************

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#ifndef GCC
#pragma hdrstop
#endif

#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>

#include "eventdlg.hxx"

#include "eventdlg.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/macrconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include "cfg.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/eventcfg.hxx>


#ifndef _HEADERTABLISTBOX_HXX
#include "headertablistbox.hxx"
#endif
#ifndef _MACROPG_IMPL_HXX
#include "macropg_impl.hxx"
#endif

#include "dialmgr.hxx"
#include "dialogs.hrc"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
// -----------------------------------------------------------------------

SvxEventConfigPage::SvxEventConfigPage( Window* pParent, const SfxItemSet& rSet ) :

    _SvxMacroTabPage( pParent, SVX_RES(RID_SVXPAGE_EVENTS), rSet ),
    aSaveInText( this, ResId( TXT_SAVEIN ) ),
    aSaveInListBox( this, ResId( LB_SAVEIN ) ),

    bAppConfig  ( TRUE )
{
    mpImpl->pStrEvent           = new String( SVX_RES( STR_EVENT ) );
    mpImpl->pAssignedMacro      = new String( SVX_RES( STR_ASSMACRO ) );
    mpImpl->pEventLB            = new _HeaderTabListBox( this, SVX_RES( LB_EVENT ) );
    mpImpl->pAssignPB           = new PushButton( this, SVX_RES( PB_ASSIGN ) );
    mpImpl->pDeletePB           = new PushButton( this, SVX_RES( PB_DELETE ) );

    FreeResource();

    aSaveInListBox.SetSelectHdl( LINK( this, SvxEventConfigPage,
                SelectHdl_Impl ) );

    Reference< document::XEventsSupplier > xSupplier;

//    xSupplier = Reference< document::XEventsSupplier >( new GlobalEventConfig());
      xSupplier = Reference< document::XEventsSupplier >
                ( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster" )), UNO_QUERY );

    Reference< container::XNameReplace > xEvents_app, xEvents_doc;
    Reference< util::XModifiable > xModifiable_doc;
    USHORT nPos;
    if ( xSupplier.is() )
    {
        xEvents_app = xSupplier->getEvents();
        if(!xEvents_app.is())
        {
            OSL_TRACE("==============globalevents impl xNameReplace broken :-(");
        }
        nPos = aSaveInListBox.InsertEntry( SFX_APP()->GetName() );
        aSaveInListBox.SetEntryData( nPos, new bool(true) );
        aSaveInListBox.SelectEntryPos( nPos, TRUE );
    }
    else
    {
        OSL_TRACE("==============globalevents impl broken :-(");
    }

    SfxObjectShell* pDoc = SfxObjectShell::Current();
    if ( pDoc )
    {
        xSupplier = Reference< document::XEventsSupplier >( pDoc->GetModel(), UNO_QUERY );
        if ( xSupplier.is() )
        {
            xEvents_doc = xSupplier->getEvents();
            nPos = aSaveInListBox.InsertEntry(
                    pDoc->GetTitle() );
            aSaveInListBox.SetEntryData( nPos, new bool(false) );
            aSaveInListBox.SelectEntryPos( nPos, TRUE );
            xModifiable_doc = Reference< util::XModifiable >( pDoc->GetModel(), UNO_QUERY );
            bAppConfig = false;
        }
    }
    InitAndSetHandler( xEvents_app, xEvents_doc, xModifiable_doc );

       SelectHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

SvxEventConfigPage::~SvxEventConfigPage()
{
    //DF Do I need to delete bools?
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxEventConfigPage, SelectHdl_Impl, ListBox *, pBox )
{
    OSL_TRACE("In SvxEventConfigPage::SelectHdl");
    bool* bApp = (bool*) aSaveInListBox.GetEntryData(
            aSaveInListBox.GetSelectEntryPos());

    mpImpl->pEventLB->SetUpdateMode( FALSE );
    bAppConfig = *bApp;
    if ( *bApp )
    {
        SetReadOnly( FALSE );
        _SvxMacroTabPage::DisplayAppEvents( true );
    }
    else
    {
        SetReadOnly( SfxObjectShell::Current()->IsReadOnly() );
        _SvxMacroTabPage::DisplayAppEvents( false );
    }

    mpImpl->pEventLB->SetUpdateMode( TRUE );
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SvxEventConfigPage::FillItemSet( SfxItemSet& rSet )
{
    OSL_TRACE("EventConfigPage:FillItemSet");
    return _SvxMacroTabPage::FillItemSet( rSet );
}

// -----------------------------------------------------------------------

void SvxEventConfigPage::Reset( const SfxItemSet& rSet )
{
    OSL_TRACE("SvxEventConfigPage::Reset");
    _SvxMacroTabPage::Reset();
}

