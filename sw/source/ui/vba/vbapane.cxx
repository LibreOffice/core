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
#include "vbapane.hxx"
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include "vbaview.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaPane::SwVbaPane( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    uno::Reference< frame::XModel > xModel ) :
    SwVbaPane_BASE( rParent, rContext ), mxModel(std::move( xModel ))
{
}

SwVbaPane::~SwVbaPane()
{
}

uno::Any SAL_CALL
SwVbaPane::View()
{
    return uno::Any( uno::Reference< word::XView >( new SwVbaView( this,  mxContext, mxModel ) ) );
}

void SAL_CALL
SwVbaPane::Close( )
{
    dispatchRequests( mxModel,u".uno:CloseWin"_ustr );
}

OUString
SwVbaPane::getServiceImplName()
{
    return u"SwVbaPane"_ustr;
}

uno::Sequence< OUString >
SwVbaPane::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Pane"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
