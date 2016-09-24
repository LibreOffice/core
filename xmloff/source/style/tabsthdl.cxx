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

#include <tabsthdl.hxx>
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
    bool bEqual = false;

    uno::Sequence< style::TabStop> aSeq1;
    if( r1 >>= aSeq1 )
    {
        uno::Sequence< style::TabStop> aSeq2;
        if( r2 >>= aSeq2 )
        {
            if( aSeq1.getLength() == aSeq2.getLength() )
            {
                bEqual = true;
                if( aSeq1.getLength() > 0 )
                {
                    const style::TabStop* pTabs1 = aSeq1.getConstArray();
                    const style::TabStop* pTabs2 = aSeq2.getConstArray();

                    int i=0;

                    do
                    {
                        bEqual = ( pTabs1[i].Position == pTabs2[i].Position       &&
                                   pTabs1[i].Alignment == pTabs2[i].Alignment     &&
                                   pTabs1[i].DecimalChar == pTabs2[i].DecimalChar &&
                                   pTabs1[i].FillChar == pTabs2[i].FillChar );
                        i++;

                    } while( bEqual && i < aSeq1.getLength() );
                }
            }
        }
    }

    return bEqual;
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
