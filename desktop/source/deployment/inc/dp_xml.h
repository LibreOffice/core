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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_XML_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_XML_H

#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>


namespace ucbhelper
{
class Content;
}

namespace dp_misc
{


void xml_parse(
    css::uno::Reference< css::xml::sax::XDocumentHandler > const & xDocHandler,
    ::ucbhelper::Content & ucb_content,
    css::uno::Reference< css::uno::XComponentContext > const & xContext );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
