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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "inettbc.hxx"

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/historyoptions.hxx>
#include <svl/folderrestriction.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/unohlp.hxx>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <svl/itemset.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/inettbc.hxx>

#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include "referers.hxx"
#include "sfxtypes.hxx"
#include "helper.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

//***************************************************************************
// SfxURLToolBoxControl_Impl
//***************************************************************************

SFX_IMPL_TOOLBOX_CONTROL(SfxURLToolBoxControl_Impl,SfxStringItem)

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox ),
    pAccExec( 0 )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CurrentURL" )));
}

SfxURLToolBoxControl_Impl::~SfxURLToolBoxControl_Impl()
{
    delete pAccExec;
}

SvtURLBox* SfxURLToolBoxControl_Impl::GetURLBox() const
{
    return (SvtURLBox*)GetToolBox().GetItemWindow( GetId() );
}

//***************************************************************************

void SfxURLToolBoxControl_Impl::OpenURL( const String& rName, BOOL /*bNew*/ ) const
{
    String aName;
    String aFilter;
    String aOptions;

    INetURLObject aObj( rName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aBaseURL = GetURLBox()->GetBaseURL();
        aName = SvtURLBox::ParseSmart( rName, aBaseURL, SvtPathOptions().GetWorkPath() );
    }
    else
        aName = rName;

    if ( !aName.Len() )
        return;

    Reference< XDispatchProvider > xDispatchProvider( getFrameInterface(), UNO_QUERY );
    if ( xDispatchProvider.is() && m_xServiceManager.is() )
    {
        URL             aTargetURL;
        ::rtl::OUString aTarget( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default")));

        aTargetURL.Complete = aName;

        getURLTransformer()->parseStrict( aTargetURL );
        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTarget, 0 );
        if ( xDispatch.is() )
        {
            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer"));
            aArgs[0].Value = makeAny( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER )));
            aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ));
            aArgs[1].Value = makeAny( ::rtl::OUString( aName ));

            if ( aFilter.Len() )
            {
                aArgs.realloc( 4 );
                aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterOptions"));
                aArgs[2].Value = makeAny( ::rtl::OUString( aOptions ));
                aArgs[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
                aArgs[3].Value = makeAny( ::rtl::OUString( aFilter ));
            }

            SfxURLToolBoxControl_Impl::ExecuteInfo* pExecuteInfo = new SfxURLToolBoxControl_Impl::ExecuteInfo;
            pExecuteInfo->xDispatch     = xDispatch;
            pExecuteInfo->aTargetURL    = aTargetURL;
            pExecuteInfo->aArgs         = aArgs;
            Application::PostUserEvent( STATIC_LINK( 0, SfxURLToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
        }
    }
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch ( Exception& )
    {
    }

    delete pExecuteInfo;
    return 0;
}


Window* SfxURLToolBoxControl_Impl::CreateItemWindow( Window* pParent )
{
    SvtURLBox* pURLBox = new SvtURLBox( pParent );
    pURLBox->SetOpenHdl( LINK( this, SfxURLToolBoxControl_Impl, OpenHdl ) );
    pURLBox->SetSelectHdl( LINK( this, SfxURLToolBoxControl_Impl, SelectHdl ) );

    return pURLBox;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, SelectHdl, void*, EMPTYARG )
{
    SvtURLBox* pURLBox = GetURLBox();
    String aName( pURLBox->GetURL() );

    if ( !pURLBox->IsTravelSelect() && aName.Len() )
        OpenURL( aName, FALSE );

    return 1L;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, OpenHdl, void*, EMPTYARG )
{
    SvtURLBox* pURLBox = GetURLBox();
    OpenURL( pURLBox->GetURL(), pURLBox->IsCtrlOpen() );

    if ( m_xServiceManager.is() )
    {
        Reference< XFramesSupplier > xDesktop( m_xServiceManager->createInstance(
                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))),
                                             UNO_QUERY );
        Reference< XFrame > xFrame( xDesktop->getActiveFrame(), UNO_QUERY );
        if ( xFrame.is() )
        {
            Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
            if ( pWin )
            {
                pWin->GrabFocus();
                pWin->ToTop( TOTOP_RESTOREWHENMIN );
            }
        }
    }

    return 1L;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, WindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( pAccExec &&
         pEvent &&
         pEvent->ISA( VclWindowEvent ) &&
         ( pEvent->GetId() == VCLEVENT_WINDOW_KEYINPUT ))
    {
        VclWindowEvent* pWinEvent = static_cast< VclWindowEvent* >( pEvent );
        KeyEvent* pKeyEvent = static_cast< KeyEvent* >( pWinEvent->GetData() );

        pAccExec->execute( pKeyEvent->GetKeyCode() );
    }

    return 1;
}

//***************************************************************************

void SfxURLToolBoxControl_Impl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( nSID == SID_OPENURL )
    {
        // Disable URL box if command is disabled
        GetURLBox()->Enable( SFX_ITEM_DISABLED != eState );
    }

    if ( GetURLBox()->IsEnabled() )
    {
        if( nSID == SID_FOCUSURLBOX )
        {
            if ( GetURLBox()->IsVisible() )
                GetURLBox()->GrabFocus();
        }
        else if ( !GetURLBox()->IsModified() && SFX_ITEM_AVAILABLE == eState )
        {
            SvtURLBox* pURLBox = GetURLBox();
            pURLBox->Clear();

            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > lList = SvtHistoryOptions().GetList(eHISTORY);
            for (sal_Int32 i=0; i<lList.getLength(); ++i)
            {
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lProps = lList[i];
                for (sal_Int32 p=0; p<lProps.getLength(); ++p)
                {
                    if (lProps[p].Name != HISTORY_PROPERTYNAME_URL)
                        continue;

                    ::rtl::OUString sURL;
                    if (!(lProps[p].Value>>=sURL) || !sURL.getLength())
                        continue;

                    INetURLObject aURL    ( sURL );
                    String        sMainURL( aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET ) );
                    String        sFile;

                    if (::utl::LocalFileHelper::ConvertURLToSystemPath(sMainURL,sFile))
                        pURLBox->InsertEntry(sFile);
                    else
                        pURLBox->InsertEntry(sMainURL);
                }
            }

            const SfxStringItem *pURL = PTR_CAST(SfxStringItem,pState);
            String aRep( pURL->GetValue() );
            INetURLObject aURL( aRep );
            INetProtocol eProt = aURL.GetProtocol();
            if ( eProt == INET_PROT_FILE )
            {
                pURLBox->SetText( aURL.PathToFileName() );
            }
            else
                pURLBox->SetText( aURL.GetURLNoPass() );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
