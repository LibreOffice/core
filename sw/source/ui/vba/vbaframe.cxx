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
#include "vbaframe.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFrame::SwVbaFrame( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const css::uno::Reference< frame::XModel >& rModel, const css::uno::Reference< text::XTextFrame >& xTextFrame ) throw ( css::uno::RuntimeException ) :
    SwVbaFrame_BASE( rParent, rContext ), mxModel( rModel ), mxTextFrame( xTextFrame )
{
}

SwVbaFrame::~SwVbaFrame()
{
}

void SAL_CALL SwVbaFrame::Select() throw ( uno::RuntimeException )
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( mxTextFrame ) );
}

rtl::OUString
SwVbaFrame::getServiceImplName()
{
    return rtl::OUString("SwVbaFrame");
}

uno::Sequence< rtl::OUString >
SwVbaFrame::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.Frame" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
