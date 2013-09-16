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

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>
#include "XMLIsPercentagePropertyHandler.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


XMLIsPercentagePropertyHandler::~XMLIsPercentagePropertyHandler()
{
}

bool XMLIsPercentagePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& ) const
{
    rValue <<= (sal_Bool)(rStrImpValue.indexOf( sal_Unicode('%') ) != -1);
    return true;
}

bool XMLIsPercentagePropertyHandler::exportXML(
    OUString&,
    const Any&,
    const SvXMLUnitConverter& ) const
{
    OSL_FAIL( "XMLIsPercentagePropertyHandler is not for export!" );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
