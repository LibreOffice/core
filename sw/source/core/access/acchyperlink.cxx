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

#include <comphelper/accessiblekeybindinghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <swurl.hxx>
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>
#include "accpara.hxx"
#include "acchyperlink.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::lang::IndexOutOfBoundsException;

SwAccessibleHyperlink::SwAccessibleHyperlink(const SwTextAttr & rTextAttr,
        SwAccessibleParagraph & rAccPara,
        sal_Int32 const nStt, sal_Int32 const nEnd)
    : m_pHyperlink(const_cast<SwFormatINetFormat*>(&rTextAttr.GetINetFormat()))
    , m_xParagraph(&rAccPara)
    , m_nStartIndex( nStt )
    , m_nEndIndex( nEnd )
{
    StartListening(m_pHyperlink->GetNotifier());
}

SwAccessibleHyperlink::~SwAccessibleHyperlink()
{
    Invalidate(); // with SolarMutex!
}

// when the pool item dies, invalidate! this is the only reason for Listener...
void SwAccessibleHyperlink::Notify(SfxHint const& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        Invalidate();
    }
}

// both the parent SwAccessibleParagraph and the pool-item must be valid
const SwFormatINetFormat *SwAccessibleHyperlink::GetTextAttr() const
{
    return (m_xParagraph.is() && m_xParagraph->GetMap())
        ? m_pHyperlink
        : nullptr;
}

// XAccessibleAction
sal_Int32 SAL_CALL SwAccessibleHyperlink::getAccessibleActionCount()
{
     return isValid() ? 1 : 0;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    bool bRet = false;

    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    SwFormatINetFormat const*const pINetFormat = GetTextAttr();
    if (pINetFormat && !pINetFormat->GetValue().isEmpty())
    {
        SwViewShell *pVSh = m_xParagraph->GetShell();
        if (pVSh)
        {
            LoadURL(*pVSh, pINetFormat->GetValue(), LoadUrlFlags::NONE,
                     pINetFormat->GetTargetFrame());
            const SwTextINetFormat *const pTextAttr = pINetFormat->GetTextINetFormat();
            if (pTextAttr)
            {
                const_cast<SwTextINetFormat*>(pTextAttr)->SetVisited(true);
                const_cast<SwTextINetFormat*>(pTextAttr)->SetVisitedValid(true);
            }
            bRet = true;
        }
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleHyperlink::getAccessibleActionDescription(
        sal_Int32 nIndex )
{
    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();

    SolarMutexGuard g;
    if (SwFormatINetFormat const*const pINetFormat = GetTextAttr())
    {
        return pINetFormat->GetValue();
    }

    return OUString();
}

uno::Reference< XAccessibleKeyBinding > SAL_CALL
    SwAccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 )
{
    uno::Reference< XAccessibleKeyBinding > xKeyBinding;

    if( isValid() )
    {
        ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper =
               new ::comphelper::OAccessibleKeyBindingHelper();
        xKeyBinding = pKeyBindingHelper;

        awt::KeyStroke aKeyStroke;
        aKeyStroke.Modifiers = 0;
        aKeyStroke.KeyCode = KEY_RETURN;
        aKeyStroke.KeyChar = 0;
        aKeyStroke.KeyFunc = 0;
        pKeyBindingHelper->AddKeyBinding( aKeyStroke );
    }

    return xKeyBinding;
}

// XAccessibleHyperlink
uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionAnchor(
        sal_Int32 nIndex)
{
    SolarMutexGuard g;

    uno::Any aRet;
    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    OUString text( m_xParagraph->GetString() );
    OUString retText =  text.copy(m_nStartIndex, m_nEndIndex - m_nStartIndex);
    aRet <<= retText;
    return aRet;
}

uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionObject(
            sal_Int32 nIndex )
{
    SolarMutexGuard g;

    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    OUString retText;
    if (SwFormatINetFormat const*const pINetFormat = GetTextAttr())
    {
        retText = pINetFormat->GetValue();
    }
    uno::Any aRet;
    aRet <<= retText;
    return aRet;
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getStartIndex()
{
    return m_nStartIndex;
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getEndIndex()
{
    return m_nEndIndex;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::isValid(  )
{
    SolarMutexGuard aGuard;
    if (m_xParagraph.is())
    {
        if (SwFormatINetFormat const*const pINetFormat = GetTextAttr())
        {
            OUString const sText(pINetFormat->GetValue());
            OUString sToken = "#";
            sal_Int32 nPos = sText.indexOf(sToken);
            if (nPos==0)//document link
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                if( ! xFactory.is() )
                    return false;
                uno::Reference< css::frame::XDesktop > xDesktop( xFactory->createInstance( "com.sun.star.frame.Desktop" ),
                    uno::UNO_QUERY );
                if( !xDesktop.is() )
                    return false;
                uno::Reference< lang::XComponent > xComp;
                xComp = xDesktop->getCurrentComponent();
                if( !xComp.is() )
                    return false;
                uno::Reference< css::document::XLinkTargetSupplier >  xLTS(xComp, uno::UNO_QUERY);
                if ( !xLTS.is())
                    return false;

                uno::Reference< css::container::XNameAccess > xLinks = xLTS->getLinks();
                uno::Reference< css::container::XNameAccess > xSubLinks;
                const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
                const sal_uLong nLinks = aNames.getLength();
                const OUString* pNames = aNames.getConstArray();

                for( sal_uLong i = 0; i < nLinks; i++ )
                {
                    uno::Any aAny;
                    OUString aLink( *pNames++ );
                    aAny = xLinks->getByName( aLink );
                    aAny >>= xSubLinks;
                    if (xSubLinks->hasByName(sText.copy(1)) )
                        return true;
                }
            }
            else//internet
                return true;
        }
    }//xpara valid
    return false;
}

void SwAccessibleHyperlink::Invalidate()
{
    SolarMutexGuard aGuard;
    m_xParagraph = nullptr;
    m_pHyperlink = nullptr;
    EndListeningAll();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
