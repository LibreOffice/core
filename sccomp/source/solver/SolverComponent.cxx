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

#include "SolverComponent.hxx"
#include "solver.hrc"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <vector>

#include <tools/resmgr.hxx>

using namespace com::sun::star;


#define STR_NONNEGATIVE   "NonNegative"
#define STR_INTEGER       "Integer"
#define STR_TIMEOUT       "Timeout"
#define STR_EPSILONLEVEL  "EpsilonLevel"
#define STR_LIMITBBDEPTH  "LimitBBDepth"


//  Resources from tools are used for translated strings

ResMgr* SolverComponent::pSolverResMgr = nullptr;

OUString SolverComponent::GetResourceString( sal_uInt32 nId )
{
    if (!pSolverResMgr)
        pSolverResMgr = ResMgr::CreateResMgr("solver");

    return ResId(nId, *pSolverResMgr).toString();
}

size_t ScSolverCellHash::operator()( const css::table::CellAddress& rAddress ) const
{
    return ( rAddress.Sheet << 24 ) | ( rAddress.Column << 16 ) | rAddress.Row;
}

bool ScSolverCellEqual::operator()( const css::table::CellAddress& rAddr1, const css::table::CellAddress& rAddr2 ) const
{
    return AddressEqual( rAddr1, rAddr2 );
}

namespace
{
    enum
    {
        PROP_NONNEGATIVE,
        PROP_INTEGER,
        PROP_TIMEOUT,
        PROP_EPSILONLEVEL,
        PROP_LIMITBBDEPTH
    };
}

uno::Reference<table::XCell> SolverComponent::GetCell( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                                          const table::CellAddress& rPos )
{
    uno::Reference<container::XIndexAccess> xSheets( xDoc->getSheets(), uno::UNO_QUERY );
    uno::Reference<sheet::XSpreadsheet> xSheet( xSheets->getByIndex( rPos.Sheet ), uno::UNO_QUERY );
    return xSheet->getCellByPosition( rPos.Column, rPos.Row );
}

void SolverComponent::SetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                   const table::CellAddress& rPos, double fValue )
{
    SolverComponent::GetCell( xDoc, rPos )->setValue( fValue );
}

double SolverComponent::GetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                     const table::CellAddress& rPos )
{
    return SolverComponent::GetCell( xDoc, rPos )->getValue();
}

SolverComponent::SolverComponent() :
    OPropertyContainer( GetBroadcastHelper() ),
    mbMaximize( true ),
    mbNonNegative( false ),
    mbInteger( false ),
    mnTimeout( 100 ),
    mnEpsilonLevel( 0 ),
    mbLimitBBDepth( true ),
    mbSuccess( false ),
    mfResultValue( 0.0 )
{
    // for XPropertySet implementation:
    registerProperty( STR_NONNEGATIVE,  PROP_NONNEGATIVE,  0, &mbNonNegative,  cppu::UnoType<decltype(mbNonNegative)>::get()  );
    registerProperty( STR_INTEGER,      PROP_INTEGER,      0, &mbInteger,      cppu::UnoType<decltype(mbInteger)>::get()      );
    registerProperty( STR_TIMEOUT,      PROP_TIMEOUT,      0, &mnTimeout,      cppu::UnoType<decltype(mnTimeout)>::get()      );
    registerProperty( STR_EPSILONLEVEL, PROP_EPSILONLEVEL, 0, &mnEpsilonLevel, cppu::UnoType<decltype(mnEpsilonLevel)>::get() );
    registerProperty( STR_LIMITBBDEPTH, PROP_LIMITBBDEPTH, 0, &mbLimitBBDepth, cppu::UnoType<decltype(mbLimitBBDepth)>::get() );
}

SolverComponent::~SolverComponent()
{
}

IMPLEMENT_FORWARD_XINTERFACE2( SolverComponent, SolverComponent_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SolverComponent, SolverComponent_Base, OPropertyContainer )

cppu::IPropertyArrayHelper* SolverComponent::createArrayHelper() const
{
    uno::Sequence<beans::Property> aProps;
    describeProperties( aProps );
    return new cppu::OPropertyArrayHelper( aProps );
}

cppu::IPropertyArrayHelper& SAL_CALL SolverComponent::getInfoHelper()
{
    return *getArrayHelper();
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL SolverComponent::getPropertySetInfo() throw(uno::RuntimeException, std::exception)
{
    return createPropertySetInfo( getInfoHelper() );
}

// XSolverDescription

OUString SAL_CALL SolverComponent::getStatusDescription() throw (uno::RuntimeException, std::exception)
{
    return maStatus;
}

OUString SAL_CALL SolverComponent::getPropertyDescription( const OUString& rPropertyName ) throw (uno::RuntimeException, std::exception)
{
    sal_uInt32 nResId = 0;
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    switch (nHandle)
    {
        case PROP_NONNEGATIVE:
            nResId = RID_PROPERTY_NONNEGATIVE;
            break;
        case PROP_INTEGER:
            nResId = RID_PROPERTY_INTEGER;
            break;
        case PROP_TIMEOUT:
            nResId = RID_PROPERTY_TIMEOUT;
            break;
        case PROP_EPSILONLEVEL:
            nResId = RID_PROPERTY_EPSILONLEVEL;
            break;
        case PROP_LIMITBBDEPTH:
            nResId = RID_PROPERTY_LIMITBBDEPTH;
            break;
        default:
            {
                // unknown - leave empty
            }
    }
    OUString aRet;
    if ( nResId )
        aRet = SolverComponent::GetResourceString( nResId );
    return aRet;
}

// XSolver: settings

uno::Reference<sheet::XSpreadsheetDocument> SAL_CALL SolverComponent::getDocument() throw(uno::RuntimeException, std::exception)
{
    return mxDoc;
}

void SAL_CALL SolverComponent::setDocument( const uno::Reference<sheet::XSpreadsheetDocument>& _document )
                                throw(uno::RuntimeException, std::exception)
{
    mxDoc = _document;
}

table::CellAddress SAL_CALL SolverComponent::getObjective() throw(uno::RuntimeException, std::exception)
{
    return maObjective;
}

void SAL_CALL SolverComponent::setObjective( const table::CellAddress& _objective ) throw(uno::RuntimeException, std::exception)
{
    maObjective = _objective;
}

uno::Sequence<table::CellAddress> SAL_CALL SolverComponent::getVariables() throw(uno::RuntimeException, std::exception)
{
    return maVariables;
}

void SAL_CALL SolverComponent::setVariables( const uno::Sequence<table::CellAddress>& _variables )
                                throw(uno::RuntimeException, std::exception)
{
    maVariables = _variables;
}

uno::Sequence<sheet::SolverConstraint> SAL_CALL SolverComponent::getConstraints() throw(uno::RuntimeException, std::exception)
{
    return maConstraints;
}

void SAL_CALL SolverComponent::setConstraints( const uno::Sequence<sheet::SolverConstraint>& _constraints )
                                throw(uno::RuntimeException, std::exception)
{
    maConstraints = _constraints;
}

sal_Bool SAL_CALL SolverComponent::getMaximize() throw(uno::RuntimeException, std::exception)
{
    return mbMaximize;
}

void SAL_CALL SolverComponent::setMaximize( sal_Bool _maximize ) throw(uno::RuntimeException, std::exception)
{
    mbMaximize = _maximize;
}

// XSolver: get results

sal_Bool SAL_CALL SolverComponent::getSuccess() throw(uno::RuntimeException, std::exception)
{
    return mbSuccess;
}

double SAL_CALL SolverComponent::getResultValue() throw(uno::RuntimeException, std::exception)
{
    return mfResultValue;
}

uno::Sequence<double> SAL_CALL SolverComponent::getSolution() throw(uno::RuntimeException, std::exception)
{
    return maSolution;
}

// XServiceInfo

sal_Bool SAL_CALL SolverComponent::supportsService( const OUString& rServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SolverComponent::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aServiceNames { "com.sun.star.sheet.Solver" };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
