/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <comphelper/accessiblekeybindinghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <swurl.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>
#include <accpara.hxx>
#include <acchyperlink.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::lang::IndexOutOfBoundsException;

SwAccessibleHyperlink::SwAccessibleHyperlink( sal_uInt16 nHPos,
    SwAccessibleParagraph *p, sal_Int32 nStt, sal_Int32 nEnd ) :
    nHintPos( nHPos ),
    xPara( p ),
    nStartIdx( nStt ),
    nEndIdx( nEnd )
{
}

const SwTxtAttr *SwAccessibleHyperlink::GetTxtAttr() const
{
    const SwTxtAttr *pTxtAttr = 0;
    if( xPara.is() && xPara->GetMap() )
    {
        const SwTxtNode *pTxtNd = xPara->GetTxtNode();
        const SwpHints *pHints = pTxtNd->GetpSwpHints();
        if( pHints && nHintPos < pHints->Count() )
        {
            const SwTxtAttr *pHt = (*pHints)[nHintPos];
            if( RES_TXTATR_INETFMT == pHt->Which() )
                pTxtAttr = pHt;
        }
    }

    return pTxtAttr;
}


sal_Int32 SAL_CALL SwAccessibleHyperlink::getAccessibleActionCount()
        throw (uno::RuntimeException)
{
     return isValid() ? 1 : 0;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Bool bRet = sal_False;

    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        if( !rINetFmt.GetValue().isEmpty() )
        {
            SwViewShell *pVSh = xPara->GetShell();
            if( pVSh )
            {
                LoadURL(*pVSh, rINetFmt.GetValue(), URLLOAD_NOFILTER,
                         rINetFmt.GetTargetFrame());
                OSL_ENSURE( pTxtAttr == rINetFmt.GetTxtINetFmt(),
                         "lost my txt attr" );
                const SwTxtINetFmt* pTxtAttr2 = rINetFmt.GetTxtINetFmt();
                if( pTxtAttr2 )
                {
                    const_cast<SwTxtINetFmt*>(pTxtAttr2)->SetVisited(true);
                    const_cast<SwTxtINetFmt*>(pTxtAttr2)->SetVisitedValid(true);
                }
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleHyperlink::getAccessibleActionDescription(
        sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();

    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        return rINetFmt.GetValue();
    }

    return OUString();
}

uno::Reference< XAccessibleKeyBinding > SAL_CALL
    SwAccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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


uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionAnchor(
        sal_Int32 nIndex)
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard g;

    uno::Any aRet;
    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    OUString text( xPara->GetString() );
    OUString retText =  text.copy(nStartIdx, nEndIdx - nStartIdx);
    aRet <<= retText;
    return aRet;
}

uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionObject(
            sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard g;

    if(nIndex != 0)
        throw lang::IndexOutOfBoundsException();
    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    OUString retText;
    if( pTxtAttr )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        retText = OUString( rINetFmt.GetValue() );
    }
    uno::Any aRet;
    aRet <<= retText;
    return aRet;
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getStartIndex()
        throw (uno::RuntimeException)
{
    return nStartIdx;
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getEndIndex()
        throw (uno::RuntimeException)
{
    return nEndIdx;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::isValid(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (xPara.is())
    {
        const SwTxtAttr *pTxtAttr = GetTxtAttr();
        OUString sText;
        if( pTxtAttr )
        {
            const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
            sText = OUString( rINetFmt.GetValue() );
            OUString sToken = "#";
            sal_Int32 nPos = sText.indexOf(sToken);
            if (nPos==0)
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                if( ! xFactory.is() )
                    return sal_False;
                uno::Reference< com::sun::star::frame::XDesktop > xDesktop( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                    uno::UNO_QUERY );
                if( !xDesktop.is() )
                    return sal_False;
                uno::Reference< lang::XComponent > xComp;
                xComp = xDesktop->getCurrentComponent();
                if( !xComp.is() )
                    return sal_False;
                uno::Reference< com::sun::star::document::XLinkTargetSupplier >  xLTS(xComp, uno::UNO_QUERY);
                if ( !xLTS.is())
                    return sal_False;

                uno::Reference< ::com::sun::star::container::XNameAccess > xLinks = xLTS->getLinks();
                uno::Reference< ::com::sun::star::container::XNameAccess > xSubLinks;
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
                        return sal_True;
                }
            }
            else
                return sal_True;
        }
    }
    return sal_False;
}

void SwAccessibleHyperlink::Invalidate()
{
    SolarMutexGuard aGuard;
    xPara = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
