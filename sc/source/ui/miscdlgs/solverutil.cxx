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

#include "solverutil.hxx"

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>

#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

#define SCSOLVER_SERVICE "com.sun.star.sheet.Solver"

void ScSolverUtil::GetImplementations( uno::Sequence<OUString>& rImplNames,
                                       uno::Sequence<OUString>& rDescriptions )
{
    rImplNames.realloc(0);      // clear
    rDescriptions.realloc(0);

    uno::Reference<uno::XComponentContext> xCtx(
        comphelper::getProcessComponentContext() );

    uno::Reference<container::XContentEnumerationAccess> xEnAc(
            xCtx->getServiceManager(), uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration( SCSOLVER_SERVICE );
        if ( xEnum.is() )
        {
            sal_Int32 nCount = 0;
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAny = xEnum->nextElement();
                uno::Reference<lang::XServiceInfo> xInfo;
                aAny >>= xInfo;
                if ( xInfo.is() )
                {
                    uno::Reference<lang::XSingleComponentFactory> xCFac( xInfo, uno::UNO_QUERY );
                    if ( xCFac.is() )
                    {
                        OUString sName = xInfo->getImplementationName();
                        OUString sDescription;

                        uno::Reference<sheet::XSolver> xSolver(
                                xCFac->createInstanceWithContext(xCtx), uno::UNO_QUERY );
                        uno::Reference<sheet::XSolverDescription> xDesc( xSolver, uno::UNO_QUERY );
                        if ( xDesc.is() )
                            sDescription = xDesc->getComponentDescription();

                        if ( sDescription.isEmpty() )
                            sDescription = sName;          // use implementation name if no description available

                        rImplNames.realloc( nCount+1 );
                        rImplNames[nCount] = sName;
                        rDescriptions.realloc( nCount+1 );
                        rDescriptions[nCount] = sDescription;
                        ++nCount;
                    }
                }
            }
        }
    }
}

uno::Reference<sheet::XSolver> ScSolverUtil::GetSolver( const OUString& rImplName )
{
    uno::Reference<sheet::XSolver> xSolver;

    uno::Reference<uno::XComponentContext> xCtx(
        comphelper::getProcessComponentContext() );

    uno::Reference<container::XContentEnumerationAccess> xEnAc(
            xCtx->getServiceManager(), uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration( SCSOLVER_SERVICE );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() && !xSolver.is() )
            {
                uno::Any aAny = xEnum->nextElement();
                uno::Reference<lang::XServiceInfo> xInfo;
                aAny >>= xInfo;
                if ( xInfo.is() )
                {
                    uno::Reference<lang::XSingleComponentFactory> xCFac( xInfo, uno::UNO_QUERY );
                    if ( xCFac.is() )
                    {
                        OUString sName = xInfo->getImplementationName();
                        if ( sName == rImplName )
                            xSolver.set( xCFac->createInstanceWithContext(xCtx), uno::UNO_QUERY );
                    }
                }
            }
        }
    }

    SAL_WARN_IF( !xSolver.is(), "sc.ui", "can't get solver" );
    return xSolver;
}

uno::Sequence<beans::PropertyValue> ScSolverUtil::GetDefaults( const OUString& rImplName )
{
    uno::Sequence<beans::PropertyValue> aDefaults;

    uno::Reference<sheet::XSolver> xSolver = GetSolver( rImplName );
    uno::Reference<beans::XPropertySet> xPropSet( xSolver, uno::UNO_QUERY );
    if ( !xPropSet.is() )
    {
        // no XPropertySet - no options
        return aDefaults;
    }

    // fill maProperties

    uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    OSL_ENSURE( xInfo.is(), "can't get property set info" );
    if ( !xInfo.is() )
        return aDefaults;

    uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
    const sal_Int32 nSize = aPropSeq.getLength();
    aDefaults.realloc(nSize);
    sal_Int32 nValid = 0;
    for (sal_Int32 nPos=0; nPos<nSize; ++nPos)
    {
        const beans::Property& rProp = aPropSeq[nPos];
        uno::Any aValue = xPropSet->getPropertyValue( rProp.Name );
        uno::TypeClass eClass = aValue.getValueTypeClass();
        // only use properties of supported types
        if ( eClass == uno::TypeClass_BOOLEAN || eClass == uno::TypeClass_LONG || eClass == uno::TypeClass_DOUBLE )
            aDefaults[nValid++] = beans::PropertyValue( rProp.Name, -1, aValue, beans::PropertyState_DIRECT_VALUE );
    }
    aDefaults.realloc(nValid);

    //! get user-visible names, sort by them

    return aDefaults;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
