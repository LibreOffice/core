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

#include <dispatch/dispatchinformationprovider.hxx>
#include <dispatch/closedispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <stdtypes.h>
#include <services.h>

#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/AppDispatchProvider.hpp>


#include <comphelper/sequenceasvector.hxx>

namespace framework{

//_________________________________________________________________________________________________________________
DispatchInformationProvider::DispatchInformationProvider(const css::uno::Reference< css::uno::XComponentContext >& xContext ,
                                                         const css::uno::Reference< css::frame::XFrame >&          xFrame)
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_xContext    (xContext                     )
    , m_xFrame      (xFrame                       )
{
}

//_________________________________________________________________________________________________________________
DispatchInformationProvider::~DispatchInformationProvider()
{
}

//_________________________________________________________________________________________________________________
css::uno::Sequence< sal_Int16 > SAL_CALL DispatchInformationProvider::getSupportedCommandGroups()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > lProvider = implts_getAllSubProvider();
    sal_Int32                                                                             c1        = lProvider.getLength();
    sal_Int32                                                                             i1        = 0;

    ::comphelper::SequenceAsVector< sal_Int16 > lGroups;

    for (i1=0; i1<c1; ++i1)
    {
        // ignore controller, which doesnt implement the right interface
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider = lProvider[i1];
        if (!xProvider.is())
            continue;

        const css::uno::Sequence< sal_Int16 > lProviderGroups = xProvider->getSupportedCommandGroups();
              sal_Int32                       c2              = lProviderGroups.getLength();
              sal_Int32                       i2              = 0;
        for (i2=0; i2<c2; ++i2)
        {
            const sal_Int16&                                                  rGroup = lProviderGroups[i2];
                  ::comphelper::SequenceAsVector< sal_Int16 >::const_iterator pGroup = ::std::find(lGroups.begin(), lGroups.end(), rGroup);
            if (pGroup == lGroups.end())
                lGroups.push_back(rGroup);
        }
    }

    return lGroups.getAsConstList();
}

//_________________________________________________________________________________________________________________
css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL DispatchInformationProvider::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > lProvider = implts_getAllSubProvider();
    sal_Int32                                                                             c1        = lProvider.getLength();
    sal_Int32                                                                             i1        = 0;

    BaseHash< css::frame::DispatchInformation > lInfos;

    for (i1=0; i1<c1; ++i1)
    {
        try
        {
            // ignore controller, which doesnt implement the right interface
            css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider = lProvider[i1];
            if (!xProvider.is())
                continue;

            const css::uno::Sequence< css::frame::DispatchInformation > lProviderInfos = xProvider->getConfigurableDispatchInformation(nCommandGroup);
                  sal_Int32                                             c2             = lProviderInfos.getLength();
                  sal_Int32                                             i2             = 0;
            for (i2=0; i2<c2; ++i2)
            {
                const css::frame::DispatchInformation&                            rInfo = lProviderInfos[i2];
                      BaseHash< css::frame::DispatchInformation >::const_iterator pInfo = lInfos.find(rInfo.Command);
                if (pInfo == lInfos.end())
                    lInfos[rInfo.Command] = rInfo;
            }
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    c1 = (sal_Int32)lInfos.size();
    i1 = 0;

    css::uno::Sequence< css::frame::DispatchInformation >       lReturn(c1);
    BaseHash< css::frame::DispatchInformation >::const_iterator pStepp ;
    for (  pStepp  = lInfos.begin()          ;
           pStepp != lInfos.end  () && i1<c1 ;
         ++pStepp, ++i1                      )
    {
        lReturn[i1] = pStepp->second;
    }
    return lReturn;
}

//_________________________________________________________________________________________________________________
css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > DispatchInformationProvider::implts_getAllSubProvider()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    css::uno::Reference< css::frame::XFrame >          xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (!xFrame.is())
        return css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > >();

    CloseDispatcher* pCloser = new CloseDispatcher(xContext, xFrame, OUString("_self")); // explicit "_self" ... not "" ... see implementation of close dispatcher itself!
    css::uno::Reference< css::uno::XInterface > xCloser(static_cast< css::frame::XDispatch* >(pCloser), css::uno::UNO_QUERY);

    css::uno::Reference< css::frame::XDispatchInformationProvider > xCloseDispatch(xCloser                                                      , css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchInformationProvider > xController   (xFrame->getController()                                      , css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchInformationProvider > xAppDispatcher = css::frame::AppDispatchProvider::create(xContext);
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > lProvider(3);
    lProvider[0] = xController   ;
    lProvider[1] = xCloseDispatch;
    lProvider[2] = xAppDispatcher;

    return lProvider;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
