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


#include "inettbc.hxx"

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox ),
    pAccExec( 0 )
{
    addStatusListener( OUString( ".uno:CurrentURL" ));
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

void SfxURLToolBoxControl_Impl::OpenURL( const OUString& rName, sal_Bool /*bNew*/ ) const
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
    if ( xDispatchProvider.is() )
    {
        URL             aTargetURL;
        OUString aTarget( OUString("_default"));

        aTargetURL.Complete = aName;

        getURLTransformer()->parseStrict( aTargetURL );
        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTarget, 0 );
        if ( xDispatch.is() )
        {
            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = OUString("Referer");
            aArgs[0].Value = makeAny( OUString(SFX_REFERER_USER ));
            aArgs[1].Name = OUString( "FileName" );
            aArgs[1].Value = makeAny( OUString( aName ));

            if ( aFilter.Len() )
            {
                aArgs.realloc( 4 );
                aArgs[2].Name = OUString("FilterOptions");
                aArgs[2].Value = makeAny( OUString( aOptions ));
                aArgs[3].Name = OUString("FilterName");
                aArgs[3].Value = makeAny( OUString( aFilter ));
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

IMPL_LINK_NOARG(SfxURLToolBoxControl_Impl, SelectHdl)
{
    SvtURLBox* pURLBox = GetURLBox();
    String aName( pURLBox->GetURL() );

    if ( !pURLBox->IsTravelSelect() && aName.Len() )
        OpenURL( aName, sal_False );

    return 1L;
}

IMPL_LINK_NOARG(SfxURLToolBoxControl_Impl, OpenHdl)
{
    SvtURLBox* pURLBox = GetURLBox();
    OpenURL( pURLBox->GetURL(), pURLBox->IsCtrlOpen() );

    Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );
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

    return 1L;
}

//***************************************************************************

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

                    OUString sURL;
                    if (!(lProps[p].Value>>=sURL) || sURL.isEmpty())
                        continue;

                    INetURLObject aURL    ( sURL );
                    OUString      sMainURL( aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET ) );
                    OUString      sFile;

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
