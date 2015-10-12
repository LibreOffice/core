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
#include "vbapivottables.hxx"
#include "vbapivottable.hxx"
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <ooo/vba/excel/XPivotTable.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

uno::Any DataPilotToPivotTable( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext )
{
    uno::Reference< sheet::XDataPilotTable > xTable( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< excel::XPivotTable > ( new ScVbaPivotTable( xContext, xTable ) ) );
}

class PivotTableEnumeration : public EnumerationHelperImpl
{
public:
    PivotTableEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        return DataPilotToPivotTable( m_xEnumeration->nextElement(),  m_xContext );
    }

};

ScVbaPivotTables::ScVbaPivotTables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess  ):  ScVbaPivotTables_BASE( xParent, xContext, xIndexAccess )
{
}

uno::Reference< container::XEnumeration >
ScVbaPivotTables::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new PivotTableEnumeration( mxParent, mxContext, xEnumAccess->createEnumeration() );
}

uno::Any
ScVbaPivotTables::createCollectionObject( const css::uno::Any& aSource )
{
    return DataPilotToPivotTable( aSource,  mxContext );
}

uno::Type
ScVbaPivotTables::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<excel::XPivotTable>::get();
}

OUString
ScVbaPivotTables::getServiceImplName()
{
    return OUString("ScVbaPivotTables");
}

css::uno::Sequence<OUString>
ScVbaPivotTables::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.excel.PivotTables";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
