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

#include <sal/config.h>

#include <com/sun/star/frame/XAppDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/util/URL.hpp>

#include <basic/basmgr.hxx>
#include <basic/sbuno.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfx2/unoctitm.hxx>
#include <svl/intitem.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

namespace {

class SfxAppDispatchProvider : public ::cppu::WeakImplHelper2< css::frame::XAppDispatchProvider,
                                                               css::lang::XServiceInfo>
{
    css::uno::WeakReference < css::frame::XFrame > m_xFrame;
public:
    SfxAppDispatchProvider(const css::uno::Sequence< css::uno::Any >&aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference < css::frame::XDispatch > SAL_CALL queryDispatch(
            const css::util::URL& aURL, const OUString& sTargetFrameName,
            FrameSearchFlags eSearchFlags )
        throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< css::uno::Reference < css::frame::XDispatch > > SAL_CALL queryDispatches(
            const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescriptor )
        throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 )
        throw (css::uno::RuntimeException);
};

SfxAppDispatchProvider::SfxAppDispatchProvider(const css::uno::Sequence< css::uno::Any >& aArguments)
    throw (uno::Exception, uno::RuntimeException)
{
    Reference < XFrame > xFrame;
    if ( aArguments.getLength() )
    {
        aArguments[0] >>= xFrame;
        m_xFrame = xFrame;
    }
}

OUString SAL_CALL SfxAppDispatchProvider::getImplementationName() throw( css::uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.sfx2.AppDispatchProvider" );
}

sal_Bool SAL_CALL SfxAppDispatchProvider::supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SfxAppDispatchProvider::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    css::uno::Sequence< OUString > seqServiceNames( 2 );
    seqServiceNames.getArray()[0] = "com.sun.star.frame.DispatchProvider";
    seqServiceNames.getArray()[1] = "com.sun.star.frame.AppDispatchProvider";
    return seqServiceNames;
}

Reference < XDispatch > SAL_CALL SfxAppDispatchProvider::queryDispatch(
    const util::URL& aURL,
    const OUString& /*sTargetFrameName*/,
    FrameSearchFlags /*eSearchFlags*/ ) throw( RuntimeException )
{
    sal_uInt16                  nId( 0 );
    sal_Bool                bMasterCommand( sal_False );
    Reference < XDispatch > xDisp;
    const SfxSlot* pSlot = 0;
    SfxDispatcher* pAppDisp = SFX_APP()->GetAppDispatcher_Impl();
    if ( aURL.Protocol == "slot:" || aURL.Protocol == "commandId:" )
    {
        nId = (sal_uInt16) aURL.Path.toInt32();
        SfxShell* pShell;
        pAppDisp->GetShellAndSlot_Impl( nId, &pShell, &pSlot, sal_True, sal_True );
    }
    else if ( aURL.Protocol == ".uno:" )
    {
        // Support ".uno" commands. Map commands to slotid
        bMasterCommand = SfxOfficeDispatch::IsMasterUnoCommand( aURL );
        if ( bMasterCommand )
            pSlot = pAppDisp->GetSlot( SfxOfficeDispatch::GetMasterUnoCommand( aURL ) );
        else
            pSlot = pAppDisp->GetSlot( aURL.Main );
    }

    if ( pSlot )
    {
        SfxOfficeDispatch* pDispatch = new SfxOfficeDispatch( pAppDisp, pSlot, aURL ) ;
        pDispatch->SetFrame(m_xFrame);
        pDispatch->SetMasterUnoCommand( bMasterCommand );
        xDisp = pDispatch;
    }

    return xDisp;
}

Sequence< Reference < XDispatch > > SAL_CALL SfxAppDispatchProvider::queryDispatches( const Sequence < DispatchDescriptor >& seqDescriptor )
throw( RuntimeException )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    uno::Sequence< uno::Reference < frame::XDispatch > > lDispatcher(nCount);
    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = this->queryDispatch( seqDescriptor[i].FeatureURL,
                                              seqDescriptor[i].FrameName,
                                              seqDescriptor[i].SearchFlags );
    return lDispatcher;
}

Sequence< sal_Int16 > SAL_CALL SfxAppDispatchProvider::getSupportedCommandGroups()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    std::list< sal_Int16 > aGroupList;
    SfxSlotPool* pAppSlotPool = &SFX_APP()->GetAppSlotPool_Impl();

    const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

    // Gruppe anw"ahlen ( Gruppe 0 ist intern )
    for ( sal_uInt16 i=0; i<pAppSlotPool->GetGroupCount(); i++ )
    {
        pAppSlotPool->SeekGroup( i );
        const SfxSlot* pSfxSlot = pAppSlotPool->FirstSlot();
        while ( pSfxSlot )
        {
            if ( pSfxSlot->GetMode() & nMode )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                aGroupList.push_back( nCommandGroup );
                break;
            }
            pSfxSlot = pAppSlotPool->NextSlot();
        }
    }

    uno::Sequence< sal_Int16 > aSeq =
        comphelper::containerToSequence< sal_Int16, std::list< sal_Int16 > >( aGroupList );

    return aSeq;
}

Sequence< frame::DispatchInformation > SAL_CALL SfxAppDispatchProvider::getConfigurableDispatchInformation( sal_Int16 nCmdGroup )
throw (uno::RuntimeException)
{
    std::list< frame::DispatchInformation > aCmdList;

    SolarMutexGuard aGuard;
    SfxSlotPool* pAppSlotPool = &SFX_APP()->GetAppSlotPool_Impl();

    if ( pAppSlotPool )
    {
        const sal_uIntPtr   nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );
        OUString aCmdPrefix( ".uno:" );

        // Gruppe anw"ahlen ( Gruppe 0 ist intern )
        for ( sal_uInt16 i=0; i<pAppSlotPool->GetGroupCount(); i++ )
        {
            pAppSlotPool->SeekGroup( i );
            const SfxSlot* pSfxSlot = pAppSlotPool->FirstSlot();
            if ( pSfxSlot )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                if ( nCommandGroup == nCmdGroup )
                {
                    while ( pSfxSlot )
                    {
                        if ( pSfxSlot->GetMode() & nMode )
                        {
                            frame::DispatchInformation aCmdInfo;
                            OUStringBuffer aBuf( aCmdPrefix );
                            aBuf.appendAscii( pSfxSlot->GetUnoName() );
                            aCmdInfo.Command = aBuf.makeStringAndClear();
                            aCmdInfo.GroupId = nCommandGroup;
                            aCmdList.push_back( aCmdInfo );
                        }
                        pSfxSlot = pAppSlotPool->NextSlot();
                    }
                }
            }
        }
    }

    uno::Sequence< frame::DispatchInformation > aSeq =
        comphelper::containerToSequence< frame::DispatchInformation, std::list< frame::DispatchInformation > >( aCmdList );

    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_sfx2_AppDispatchProvider_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new SfxAppDispatchProvider(arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
