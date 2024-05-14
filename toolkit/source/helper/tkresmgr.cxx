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

#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <vcl/image.hxx>

#include <helper/tkresmgr.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicProvider;
using namespace ::com::sun::star;

Image TkResMgr::getImageFromURL(const OUString& i_rImageURL)
{
    if (i_rImageURL.isEmpty())
        return Image();

    try
    {
        Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());
        Reference<XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));
        ::comphelper::NamedValueCollection aMediaProperties;
        aMediaProperties.put(u"URL"_ustr, i_rImageURL);
        Reference<XGraphic> xGraphic
            = xProvider->queryGraphic(aMediaProperties.getPropertyValues());
        return Image(xGraphic);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("toolkit");
    }
    return Image();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
