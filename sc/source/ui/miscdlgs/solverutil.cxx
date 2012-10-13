/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "solverutil.hxx"

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>

#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------

#define SCSOLVER_SERVICE "com.sun.star.sheet.Solver"

static uno::Reference<sheet::XSolver> lcl_CreateSolver( const uno::Reference<uno::XInterface>& xIntFac,
                                                 const uno::Reference<uno::XComponentContext>& xCtx )
{
    uno::Reference<sheet::XSolver> xSolver;

    uno::Reference<lang::XSingleComponentFactory> xCFac( xIntFac, uno::UNO_QUERY );
    uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
    if ( xCFac.is() )
    {
        try
        {
            uno::Reference<uno::XInterface> xInterface = xCFac->createInstanceWithContext(xCtx);
            xSolver = uno::Reference<sheet::XSolver>( xInterface, uno::UNO_QUERY );
        }
        catch(uno::Exception&)
        {
        }
    }
    if ( !xSolver.is() && xFac.is() )
    {
        try
        {
            uno::Reference<uno::XInterface> xInterface = xFac->createInstance();
            xSolver = uno::Reference<sheet::XSolver>( xInterface, uno::UNO_QUERY );
        }
        catch(uno::Exception&)
        {
        }
    }

    return xSolver;
}

void ScSolverUtil::GetImplementations( uno::Sequence<rtl::OUString>& rImplNames,
                                       uno::Sequence<rtl::OUString>& rDescriptions )
{
    rImplNames.realloc(0);      // clear
    rDescriptions.realloc(0);

    uno::Reference<uno::XComponentContext> xCtx(
        comphelper::getProcessComponentContext() );
    uno::Reference<lang::XMultiServiceFactory> xMSF(
        xCtx->getServiceManager(), uno::UNO_QUERY_THROW );

    uno::Reference<container::XContentEnumerationAccess> xEnAc( xMSF, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SCSOLVER_SERVICE)) );
        if ( xEnum.is() )
        {
            sal_Int32 nCount = 0;
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAny = xEnum->nextElement();
                uno::Reference<uno::XInterface> xIntFac;
                aAny >>= xIntFac;
                if ( xIntFac.is() )
                {
                    uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                    if ( xInfo.is() )
                    {
                        rtl::OUString sName = xInfo->getImplementationName();
                        rtl::OUString sDescription;

                        uno::Reference<sheet::XSolver> xSolver = lcl_CreateSolver( xIntFac, xCtx );
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

uno::Reference<sheet::XSolver> ScSolverUtil::GetSolver( const rtl::OUString& rImplName )
{
    uno::Reference<sheet::XSolver> xSolver;

    uno::Reference<uno::XComponentContext> xCtx(
        comphelper::getProcessComponentContext() );
    uno::Reference<lang::XMultiServiceFactory> xMSF(
        xCtx->getServiceManager(), uno::UNO_QUERY_THROW );

    uno::Reference<container::XContentEnumerationAccess> xEnAc( xMSF, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SCSOLVER_SERVICE)) );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() && !xSolver.is() )
            {
                uno::Any aAny = xEnum->nextElement();
                uno::Reference<uno::XInterface> xIntFac;
                aAny >>= xIntFac;
                if ( xIntFac.is() )
                {
                    uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                    if ( xInfo.is() )
                    {
                        rtl::OUString sName = xInfo->getImplementationName();
                        if ( sName == rImplName )
                            xSolver = lcl_CreateSolver( xIntFac, xCtx );
                    }
                }
            }
        }
    }

    OSL_ENSURE( xSolver.is(), "can't get solver" );
    return xSolver;
}

uno::Sequence<beans::PropertyValue> ScSolverUtil::GetDefaults( const rtl::OUString& rImplName )
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
