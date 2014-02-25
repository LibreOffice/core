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
#include "vbalisttemplate.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbalistlevels.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaListTemplate::SwVbaListTemplate( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType ) throw ( uno::RuntimeException ) : SwVbaListTemplate_BASE( rParent, rContext )
{
    pListHelper.reset( new SwVbaListHelper( xTextDoc, nGalleryType, nTemplateType ) );
}

SwVbaListTemplate::~SwVbaListTemplate()
{
}

uno::Any SAL_CALL
SwVbaListTemplate::ListLevels( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< XCollection > xCol( new SwVbaListLevels( mxParent, mxContext, pListHelper ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

void SwVbaListTemplate::applyListTemplate( uno::Reference< beans::XPropertySet >& xProps ) throw (uno::RuntimeException)
{
    uno::Reference< container::XIndexReplace > xNumberingRules = pListHelper->getNumberingRules();
    xProps->setPropertyValue("NumberingRules", uno::makeAny( xNumberingRules ) );
}

OUString
SwVbaListTemplate::getServiceImplName()
{
    return OUString("SwVbaListTemplate");
}

uno::Sequence< OUString >
SwVbaListTemplate::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.ListTemplate";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
