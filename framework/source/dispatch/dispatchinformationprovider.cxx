/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <dispatch/dispatchinformationprovider.hxx>
#include <dispatch/closedispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <stdtypes.h>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/CommandGroup.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <comphelper/sequenceasvector.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________
DEFINE_XINTERFACE_1(DispatchInformationProvider                               ,
                    OWeakObject                                               ,
                    DIRECT_INTERFACE(css::frame::XDispatchInformationProvider))

//_________________________________________________________________________________________________________________
DispatchInformationProvider::DispatchInformationProvider(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                         const css::uno::Reference< css::frame::XFrame >&              xFrame)
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_xSMGR       (xSMGR                        )
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
        // ignore controller, which doesn't implement the right interface
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
            // ignore controller, which doesn't implement the right interface
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
        catch(const css::uno::RuntimeException& exRun)
            { throw exRun; }
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
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    css::uno::Reference< css::frame::XFrame >              xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (!xFrame.is())
        return css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > >();

    CloseDispatcher* pCloser = new CloseDispatcher(xSMGR, xFrame, ::rtl::OUString::createFromAscii("_self")); // explicit "_self" ... not "" ... see implementation of close dispatcher itself!
    css::uno::Reference< css::uno::XInterface > xCloser(static_cast< css::frame::XDispatch* >(pCloser), css::uno::UNO_QUERY);

    css::uno::Reference< css::frame::XDispatchInformationProvider > xCloseDispatch(xCloser                                                      , css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchInformationProvider > xController   (xFrame->getController()                                      , css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchInformationProvider > xAppDispatcher(xSMGR->createInstance(IMPLEMENTATIONNAME_APPDISPATCHPROVIDER), css::uno::UNO_QUERY);

    css::uno::Sequence< css::uno::Reference< css::frame::XDispatchInformationProvider > > lProvider(3);
    lProvider[0] = xController   ;
    lProvider[1] = xCloseDispatch;
    lProvider[2] = xAppDispatcher;

    return lProvider;
}

} // namespace framework
