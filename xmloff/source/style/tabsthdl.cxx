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

#include "tabsthdl.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/TabStop.hpp>

using namespace ::com::sun::star;


// class XMLFontFamilyNamePropHdl


XMLTabStopPropHdl::~XMLTabStopPropHdl()
{
    // Nothing to do
}

bool XMLTabStopPropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    uno::Sequence< style::TabStop> aSeq1;
    if( r1 >>= aSeq1 )
    {
        uno::Sequence< style::TabStop> aSeq2;
        if( r2 >>= aSeq2 )
        {
            return std::equal(aSeq1.begin(), aSeq1.end(), aSeq2.begin(), aSeq2.end(),
                [](const style::TabStop& a, const style::TabStop& b) {
                    return a.Position == b.Position
                        && a.Alignment == b.Alignment
                        && a.DecimalChar == b.DecimalChar
                        && a.FillChar == b.FillChar;
                });
        }
    }

    return false;
}

bool XMLTabStopPropHdl::importXML( const OUString&, css::uno::Any&, const SvXMLUnitConverter& ) const
{
    return false;
}

bool XMLTabStopPropHdl::exportXML( OUString&, const css::uno::Any&, const SvXMLUnitConverter& ) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
