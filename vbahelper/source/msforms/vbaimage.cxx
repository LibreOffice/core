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
#include "vbaimage.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


ScVbaImage::ScVbaImage( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : ImageImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

OUString
ScVbaImage::getServiceImplName()
{
    return u"ScVbaImage"_ustr;
}

uno::Sequence< OUString >
ScVbaImage::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.Image"_ustr
    };
    return aServiceNames;
}

sal_Int32 SAL_CALL ScVbaImage::getBackColor()
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaImage::setBackColor( sal_Int32 nBackColor )
{
    ScVbaControl::setBackColor( nBackColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
