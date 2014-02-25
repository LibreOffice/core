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
#include <swurl.hxx>
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>
#include <frmfmt.hxx>

#include <accnotexthyperlink.hxx>

#include <fmturl.hxx>

#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>

#include <accmap.hxx>
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SwAccessibleNoTextHyperlink::SwAccessibleNoTextHyperlink( SwAccessibleNoTextFrame *p, const SwFrm *aFrm, sal_uInt16 nIndex) :
    xFrame( p ),
    mpFrm( aFrm ),
    mnIndex(nIndex)
{
}

// XAccessibleAction
sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionCount()
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard g;

    SwFmtURL aURL( GetFmt()->GetURL() );
    ImageMap* pMap = aURL.GetMap();
    if( pMap != NULL )
    {
        return pMap->GetIMapObjectCount();
    }
    else if( !aURL.GetURL().isEmpty() )
    {
        return 1;
    }

    return 0;
}

sal_Bool SAL_CALL SwAccessibleNoTextHyperlink::doAccessibleAction( sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(nIndex < 0 || nIndex >= getAccessibleActionCount())
        throw lang::IndexOutOfBoundsException();

    sal_Bool bRet = sal_False;
    SwFmtURL aURL( GetFmt()->GetURL() );
    ImageMap* pMap = aURL.GetMap();
    if( pMap != NULL )
    {
        IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
        if (!pMapObj->GetURL().isEmpty())
        {
            SwViewShell *pVSh = xFrame->GetShell();
            if( pVSh )
            {
                LoadURL( *pVSh, pMapObj->GetURL(), URLLOAD_NOFILTER,
                         pMapObj->GetTarget() );
                bRet = sal_True;
            }
        }
    }
    else if (!aURL.GetURL().isEmpty())
    {
        SwViewShell *pVSh = xFrame->GetShell();
        if( pVSh )
        {
            LoadURL( *pVSh, aURL.GetURL(), URLLOAD_NOFILTER,
                     aURL.GetTargetFrameName() );
            bRet = sal_True;
        }
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionDescription(
        sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard g;

    OUString sDesc;

    if(nIndex < 0 || nIndex >= getAccessibleActionCount())
        throw lang::IndexOutOfBoundsException();

    SwFmtURL aURL( GetFmt()->GetURL() );
    ImageMap* pMap = aURL.GetMap();
    if( pMap != NULL )
    {
        IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
        if (!pMapObj->GetDesc().isEmpty())
            sDesc = pMapObj->GetDesc();
        else if (!pMapObj->GetURL().isEmpty())
            sDesc = pMapObj->GetURL();
    }
    else if( !aURL.GetURL().isEmpty() )
        sDesc = aURL.GetName();

    return sDesc;
}

Reference< XAccessibleKeyBinding > SAL_CALL
    SwAccessibleNoTextHyperlink::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard g;

    Reference< XAccessibleKeyBinding > xKeyBinding;

    if(nIndex < 0 || nIndex >= getAccessibleActionCount())
        throw lang::IndexOutOfBoundsException();

    bool bIsValid = false;
    SwFmtURL aURL( GetFmt()->GetURL() );
    ImageMap* pMap = aURL.GetMap();
    if( pMap != NULL )
    {
        IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
        if (!pMapObj->GetURL().isEmpty())
            bIsValid = true;
    }
    else if (!aURL.GetURL().isEmpty())
        bIsValid = true;

    if(bIsValid)
    {
        ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper =
            new ::comphelper::OAccessibleKeyBindingHelper();
        xKeyBinding = pKeyBindingHelper;

        ::com::sun::star::awt::KeyStroke aKeyStroke;
        aKeyStroke.Modifiers = 0;
        aKeyStroke.KeyCode = KEY_RETURN;
        aKeyStroke.KeyChar = 0;
        aKeyStroke.KeyFunc = 0;
        pKeyBindingHelper->AddKeyBinding( aKeyStroke );
    }

    return xKeyBinding;
}

// XAccessibleHyperlink
Any SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionAnchor(
        sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if(nIndex < 0 || nIndex >= getAccessibleActionCount())
        throw lang::IndexOutOfBoundsException();

    Any aRet;
    //SwFrm* pAnchor = ((SwFlyFrm*)mpFrm)->GetAnchor();
    Reference< XAccessible > xAnchor = xFrame->GetAccessibleMap()->GetContext(mpFrm, sal_True);
    //SwAccessibleNoTextFrame* pFrame = xFrame.get();
    //Reference< XAccessible > xAnchor = (XAccessible*)pFrame;
    aRet <<= xAnchor;
    return aRet;
}

Any SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionObject(
            sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if(nIndex < 0 || nIndex >= getAccessibleActionCount())
        throw lang::IndexOutOfBoundsException();

    SwFmtURL aURL( GetFmt()->GetURL() );
    OUString retText;
    ImageMap* pMap = aURL.GetMap();
    if( pMap != NULL )
    {
        IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
        if (!pMapObj->GetURL().isEmpty())
            retText = pMapObj->GetURL();
    }
    else if ( !aURL.GetURL().isEmpty() )
        retText = aURL.GetURL();

    Any aRet;
    aRet <<= retText;
    return aRet;
}

sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getStartIndex()
        throw (RuntimeException, std::exception)
{
    return 0;
}

sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getEndIndex()
        throw (RuntimeException, std::exception)
{
    return 0;
}

sal_Bool SAL_CALL SwAccessibleNoTextHyperlink::isValid(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    SwFmtURL aURL( GetFmt()->GetURL() );

    if( aURL.GetMap() || !aURL.GetURL().isEmpty() )
        return sal_True;
    return sal_False;
}
