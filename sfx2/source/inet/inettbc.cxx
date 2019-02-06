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


#include <inettbc.hxx>

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <rtl/ustring.hxx>

#include <svl/itemset.hxx>
#include <svl/urihelper.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/inettbc.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfxtypes.hxx>
#include <helper.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;


// SfxURLToolBoxControl_Impl


SFX_IMPL_TOOLBOX_CONTROL(SfxURLToolBoxControl_Impl,SfxStringItem)

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
{
    addStatusListener( ".uno:CurrentURL");
}

SfxURLToolBoxControl_Impl::~SfxURLToolBoxControl_Impl()
{
}

SvtURLBox* SfxURLToolBoxControl_Impl::GetURLBox() const
{
    return static_cast<SvtURLBox*>(GetToolBox().GetItemWindow( GetId() ));
}


void SfxURLToolBoxControl_Impl::OpenURL( const OUString& rName ) const
{
    OUString aName;
    OUString aFilter;

    INetURLObject aObj( rName );
    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aBaseURL = GetURLBox()->GetBaseURL();
        aName = SvtURLBox::ParseSmart( rName, aBaseURL );
    }
    else
        aName = rName;

    if ( aName.isEmpty() )
        return;

    Reference< XDispatchProvider > xDispatchProvider( getFrameInterface(), UNO_QUERY );
    if ( !xDispatchProvider.is() )
        return;

    URL             aTargetURL;
    aTargetURL.Complete = aName;

    getURLTransformer()->parseStrict( aTargetURL );
    Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, "_default", 0 );
    if ( !xDispatch.is() )
        return;

    Sequence< PropertyValue > aArgs( 2 );
    aArgs[0].Name = "Referer";
    aArgs[0].Value <<= OUString( "private:user" );
    aArgs[1].Name = "FileName";
    aArgs[1].Value <<= aName;

    if ( !aFilter.isEmpty() )
    {
        aArgs.realloc( 4 );
        aArgs[2].Name = "FilterOptions";
        aArgs[2].Value <<= OUString();
        aArgs[3].Name = "FilterName";
        aArgs[3].Value <<= aFilter;
    }

    SfxURLToolBoxControl_Impl::ExecuteInfo* pExecuteInfo = new SfxURLToolBoxControl_Impl::ExecuteInfo;
    pExecuteInfo->xDispatch     = xDispatch;
    pExecuteInfo->aTargetURL    = aTargetURL;
    pExecuteInfo->aArgs         = aArgs;
    Application::PostUserEvent( LINK( nullptr, SfxURLToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
}


IMPL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
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
}


VclPtr<vcl::Window> SfxURLToolBoxControl_Impl::CreateItemWindow( vcl::Window* pParent )
{
    VclPtrInstance<SvtURLBox> pURLBox( pParent );
    pURLBox->SetOpenHdl( LINK( this, SfxURLToolBoxControl_Impl, OpenHdl ) );
    pURLBox->SetSelectHdl( LINK( this, SfxURLToolBoxControl_Impl, SelectHdl ) );
    return pURLBox.get();
}

IMPL_LINK_NOARG(SfxURLToolBoxControl_Impl, SelectHdl, ComboBox&, void)
{
    SvtURLBox* pURLBox = GetURLBox();
    OUString aName( pURLBox->GetURL() );

    if ( !pURLBox->IsTravelSelect() && !aName.isEmpty() )
        OpenURL( aName );
}

IMPL_LINK_NOARG(SfxURLToolBoxControl_Impl, OpenHdl, SvtURLBox*, void)
{
    SvtURLBox* pURLBox = GetURLBox();
    OpenURL( pURLBox->GetURL() );

    Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );
    Reference< XFrame > xFrame( xDesktop->getActiveFrame(), UNO_QUERY );
    if ( xFrame.is() )
    {
        VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        if ( pWin )
        {
            pWin->GrabFocus();
            pWin->ToTop( ToTopFlags::RestoreWhenMin );
        }
    }
}


void SfxURLToolBoxControl_Impl::StateChanged
(
    sal_uInt16              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( nSID == SID_OPENURL )
    {
        // Disable URL box if command is disabled
        GetURLBox()->Enable( SfxItemState::DISABLED != eState );
    }

    if ( !GetURLBox()->IsEnabled() )
        return;

    if( nSID == SID_FOCUSURLBOX )
    {
        if ( GetURLBox()->IsVisible() )
            GetURLBox()->GrabFocus();
    }
    else if ( !GetURLBox()->IsModified() && SfxItemState::DEFAULT == eState )
    {
        SvtURLBox* pURLBox = GetURLBox();
        pURLBox->Clear();

        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > lList = SvtHistoryOptions().GetList(ePICKLIST);
        for (sal_Int32 i=0; i<lList.getLength(); ++i)
        {
            css::uno::Sequence< css::beans::PropertyValue > lProps = lList[i];
            for (sal_Int32 p=0; p<lProps.getLength(); ++p)
            {
                if (lProps[p].Name != HISTORY_PROPERTYNAME_URL)
                    continue;

                OUString sURL;
                if (!(lProps[p].Value>>=sURL) || sURL.isEmpty())
                    continue;

                INetURLObject aURL    ( sURL );
                OUString      sMainURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::WithCharset ) );
                OUString      sFile;

                if (osl::FileBase::getSystemPathFromFileURL(sMainURL, sFile) == osl::FileBase::E_None)
                    pURLBox->InsertEntry(sFile);
                else
                    pURLBox->InsertEntry(sMainURL);
            }
        }

        const SfxStringItem *pURL = dynamic_cast< const SfxStringItem* >(pState);
        INetURLObject aURL( pURL->GetValue() );
        INetProtocol eProt = aURL.GetProtocol();
        if ( eProt == INetProtocol::File )
        {
            pURLBox->SetText( aURL.PathToFileName() );
        }
        else
            pURLBox->SetText( aURL.GetURLNoPass() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
