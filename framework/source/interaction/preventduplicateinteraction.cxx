/*************************************************************************
 *
 *  $RCSfile: preventduplicateinteraction.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 12:05:36 $
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

#include "interaction/preventduplicateinteraction.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#define IMPLEMENTATIONNAME_UIINTERACTIONHANDLER                 ::rtl::OUString::createFromAscii("com.sun.star.comp.uui.UUIInteractionHandler")

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________

PreventDuplicateInteraction::PreventDuplicateInteraction(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : m_xSMGR(xSMGR)
{
}

//_________________________________________________________________________________________________________________

PreventDuplicateInteraction::~PreventDuplicateInteraction()
{
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::setHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::useDefaultUUIHandler()
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aLock.clear();
    // <- SAFE

    css::uno::Reference< css::task::XInteractionHandler > xHandler(
                            xSMGR->createInstance(IMPLEMENTATIONNAME_UIINTERACTIONHANDLER),
                            css::uno::UNO_QUERY_THROW);

    // SAFE ->
    aLock.reset();
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

void SAL_CALL PreventDuplicateInteraction::handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
    throw(css::uno::RuntimeException)
{
    css::uno::Any aRequest  = xRequest->getRequest();
    sal_Bool      bHandleIt = sal_True;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;

        if (aRequest.isExtractableTo(rInfo.m_aInteraction))
        {
            ++rInfo.m_nCallCount;
            rInfo.m_xRequest = xRequest;
            bHandleIt = (rInfo.m_nCallCount <= rInfo.m_nMaxCount);
            break;
        }
    }

    css::uno::Reference< css::task::XInteractionHandler > xHandler = m_xHandler;

    aLock.clear();
    // <- SAFE

    if (
        (bHandleIt    ) &&
        (xHandler.is())
       )
    {
        xHandler->handle(xRequest);
    }
    else
    {
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
        sal_Int32 c = lContinuations.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            css::uno::Reference< css::task::XInteractionAbort > xAbort(lContinuations[i], css::uno::UNO_QUERY);
            if (xAbort.is())
            {
                xAbort->select();
                break;
            }
        }
    }
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::addInteractionRule(const PreventDuplicateInteraction::InteractionInfo& aInteractionInfo)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteractionInfo.m_aInteraction)
        {
            rInfo.m_nMaxCount  = aInteractionInfo.m_nMaxCount ;
            rInfo.m_nCallCount = aInteractionInfo.m_nCallCount;
            return;
        }
    }

    m_lInteractionRules.push_back(aInteractionInfo);

    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

sal_Bool PreventDuplicateInteraction::getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                               PreventDuplicateInteraction::InteractionInfo* pReturn     ) const
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    PreventDuplicateInteraction::InteractionList::const_iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        const PreventDuplicateInteraction::InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteraction)
        {
            *pReturn = rInfo;
            return sal_True;
        }
    }

    aLock.clear();
    // <- SAFE

    return sal_False;
}

} // namespace framework
