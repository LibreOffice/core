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
#include "vbalistgallery.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbalisttemplates.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaListGallery::SwVbaListGallery( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& xTextDoc, sal_Int32 nType ) throw ( uno::RuntimeException ) : SwVbaListGallery_BASE( rParent, rContext ), mxTextDocument( xTextDoc ), mnType( nType )
{
}

SwVbaListGallery::~SwVbaListGallery()
{
}

uno::Any SAL_CALL
SwVbaListGallery::ListTemplates( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< XCollection > xCol( new SwVbaListTemplates( mxParent, mxContext, mxTextDocument, mnType ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

OUString
SwVbaListGallery::getServiceImplName()
{
    return OUString("SwVbaListGallery");
}

uno::Sequence< OUString >
SwVbaListGallery::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.ListGallery";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
