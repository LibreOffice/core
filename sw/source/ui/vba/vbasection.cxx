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
#include "vbasection.hxx"
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include "vbapagesetup.hxx"
#include "vbaheadersfooters.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaSection::SwVbaSection( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< frame::XModel >  xModel, uno::Reference< beans::XPropertySet >  xProps ) :
    SwVbaSection_BASE( rParent, rContext ), mxModel(std::move( xModel )), mxPageProps(std::move( xProps ))
{
}

SwVbaSection::~SwVbaSection()
{
}

sal_Bool SAL_CALL SwVbaSection::getProtectedForForms()
{
    return false;
}

void SAL_CALL SwVbaSection::setProtectedForForms( sal_Bool /*_protectedforforms*/ )
{
}

uno::Any SAL_CALL SwVbaSection::Headers( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaHeadersFooters( this, mxContext, mxModel, mxPageProps, true ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL SwVbaSection::Footers( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaHeadersFooters( this, mxContext, mxModel, mxPageProps, false ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaSection::PageSetup( )
{
    return uno::Any( uno::Reference< word::XPageSetup >( new SwVbaPageSetup( this, mxContext, mxModel, mxPageProps ) ) );
}

OUString
SwVbaSection::getServiceImplName()
{
    return u"SwVbaSection"_ustr;
}

uno::Sequence< OUString >
SwVbaSection::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Section"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
