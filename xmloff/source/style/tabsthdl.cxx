/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <tabsthdl.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/TabStop.hpp>

using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontFamilyNamePropHdl
//

XMLTabStopPropHdl::~XMLTabStopPropHdl()
{
    // Nothing to do
}

bool XMLTabStopPropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    sal_Bool bEqual = sal_False;

    uno::Sequence< style::TabStop> aSeq1;
    if( r1 >>= aSeq1 )
    {
        uno::Sequence< style::TabStop> aSeq2;
        if( r2 >>= aSeq2 )
        {
            if( aSeq1.getLength() == aSeq2.getLength() )
            {
                bEqual = sal_True;
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

sal_Bool XMLTabStopPropHdl::importXML( const ::rtl::OUString&, ::com::sun::star::uno::Any&, const SvXMLUnitConverter& ) const
{
    return sal_False;
}

sal_Bool XMLTabStopPropHdl::exportXML( ::rtl::OUString&, const ::com::sun::star::uno::Any&, const SvXMLUnitConverter& ) const
{
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
