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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFrame::SwVbaFrame( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, css::uno::Reference< frame::XModel > xModel, css::uno::Reference< text::XTextFrame >  xTextFrame ) :
    SwVbaFrame_BASE( rParent, rContext ), mxModel(std::move( xModel )), mxTextFrame(std::move( xTextFrame ))
{
}

SwVbaFrame::~SwVbaFrame()
{
}

void SAL_CALL SwVbaFrame::Select()
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::Any( mxTextFrame ) );
}

OUString
SwVbaFrame::getServiceImplName()
{
    return u"SwVbaFrame"_ustr;
}

uno::Sequence< OUString >
SwVbaFrame::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Frame"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
