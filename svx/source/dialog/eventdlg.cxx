/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventdlg.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-14 07:17:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// include ***************************************************************

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#include <comphelper/processfactory.hxx>
#ifndef _UNOTOOLS_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#include <rtl/ustring.hxx>

#include "eventdlg.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/macrconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
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
#include "eventdlg.hrc"
#include "helpid.hrc"
#include "selector.hxx"
#include "cfg.hxx"


using ::rtl::OUString;
using namespace ::com::sun::star;
// -----------------------------------------------------------------------

SvxEventConfigPage::SvxEventConfigPage( Window* pParent, const SfxItemSet& rSet ) :

    _SvxMacroTabPage( pParent, SVX_RES(RID_SVXPAGE_EVENTS), rSet ),
    aSaveInText( this, ResId( TXT_SAVEIN ) ),
    aSaveInListBox( this, ResId( LB_SAVEIN ) ),
    bAppConfig  ( TRUE )
{
    mpImpl->pStrEvent           = new String( ResId( STR_EVENT ));
    mpImpl->pAssignedMacro      = new String( ResId( STR_ASSMACRO ));
    mpImpl->pEventLB            = new _HeaderTabListBox( this, ResId( LB_EVENT ));
    mpImpl->pAssignFT           = new FixedText( this,  ResId( FT_ASSIGN ));
    mpImpl->pAssignPB           = new PushButton( this, ResId( PB_ASSIGN ));
    mpImpl->pDeletePB           = new PushButton( this, ResId( PB_DELETE ));
    mpImpl->pMacroImg           = new Image( ResId( IMG_MACRO) );
    mpImpl->pComponentImg       = new Image( ResId( IMG_COMPONENT) );
    mpImpl->pMacroImg_h         = new Image( ResId( IMG_MACRO_H) );
    mpImpl->pComponentImg_h     = new Image( ResId( IMG_COMPONENT_H) );

    FreeResource();

    // must be done after FreeResource is called
    InitResources();

    mpImpl->pEventLB->GetListBox().SetHelpId( HID_SVX_MACRO_LB_EVENT );

    aSaveInListBox.SetSelectHdl( LINK( this, SvxEventConfigPage,
                SelectHdl_Impl ) );

    uno::Reference< document::XEventsSupplier > xSupplier;

//    xSupplier = uno::Reference< document::XEventsSupplier >( new GlobalEventConfig());
    xSupplier = uno::Reference< document::XEventsSupplier > (
        ::comphelper::getProcessServiceFactory()->createInstance(
            OUString::createFromAscii(
                "com.sun.star.frame.GlobalEventBroadcaster" ) ),
        uno::UNO_QUERY );

    uno::Reference< container::XNameReplace > xEvents_app, xEvents_doc;
    USHORT nPos;
    if ( xSupplier.is() )
    {
        xEvents_app = xSupplier->getEvents();
        OUString label;
        utl::ConfigManager::GetDirectConfigProperty(
            utl::ConfigManager::PRODUCTNAME ) >>= label;
        nPos = aSaveInListBox.InsertEntry( label );
        aSaveInListBox.SetEntryData( nPos, new bool(true) );
        aSaveInListBox.SelectEntryPos( nPos, TRUE );
    }

    uno::Reference< frame::XFramesSupplier > xFramesSupplier(
        ::comphelper::getProcessServiceFactory()->createInstance(
            OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
        uno::UNO_QUERY );

    uno::Reference< frame::XModel > xModel;
    uno::Reference< frame::XFrame > xFrame =
        xFramesSupplier->getActiveFrame();

    if ( xFrame.is() )
    {
        // first establish if this type of application module
        // supports document configuration
        uno::Reference< ::com::sun::star::frame::XModuleManager >
            xModuleManager(
            ::comphelper::getProcessServiceFactory()->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.frame.ModuleManager" ) ) ),
            uno::UNO_QUERY );

        OUString aModuleId;
        try{
            aModuleId = xModuleManager->identify( xFrame );
        } catch(const uno::Exception&)
            { aModuleId = ::rtl::OUString(); }

        if ( SvxConfigPage::CanConfig( aModuleId ) )
        {
            uno::Reference< frame::XController > xController =
                xFrame->getController();

            if ( xController.is() )
            {
                xModel = xController->getModel();
            }
        }
    }

    uno::Reference< util::XModifiable > xModifiable_doc;
    if ( xModel.is() )
    {
        xSupplier = uno::Reference< document::XEventsSupplier >(
            xModel, uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            xEvents_doc = xSupplier->getEvents();
            xModifiable_doc =
                uno::Reference< util::XModifiable >( xModel, uno::UNO_QUERY );

            OUString aTitle;
            SvxScriptSelectorDialog::GetDocTitle( xModel, aTitle );
            nPos = aSaveInListBox.InsertEntry( aTitle );

            aSaveInListBox.SetEntryData( nPos, new bool(false) );
            aSaveInListBox.SelectEntryPos( nPos, TRUE );

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
    (void)pBox;

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
        bool isReadonly = FALSE;

        uno::Reference< frame::XFramesSupplier > xFramesSupplier(
            ::comphelper::getProcessServiceFactory()->createInstance(
                OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
            uno::UNO_QUERY );

        uno::Reference< frame::XFrame > xFrame =
            xFramesSupplier->getActiveFrame();

        if ( xFrame.is() )
        {
            uno::Reference< frame::XController > xController =
                xFrame->getController();

            if ( xController.is() )
            {
                uno::Reference< frame::XStorable > xStorable(
                    xController->getModel(), uno::UNO_QUERY );
                isReadonly = xStorable->isReadonly();
            }
        }

        SetReadOnly( isReadonly );
        _SvxMacroTabPage::DisplayAppEvents( false );
    }

    mpImpl->pEventLB->SetUpdateMode( TRUE );
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SvxEventConfigPage::FillItemSet( SfxItemSet& rSet )
{
    return _SvxMacroTabPage::FillItemSet( rSet );
}

// -----------------------------------------------------------------------

void SvxEventConfigPage::Reset( const SfxItemSet& )
{
    _SvxMacroTabPage::Reset();
}

