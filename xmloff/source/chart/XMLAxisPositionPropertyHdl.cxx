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
#include "XMLAxisPositionPropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::xmloff::token;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace com::sun::star;

XMLAxisPositionPropertyHdl::XMLAxisPositionPropertyHdl( bool bCrossingValue )
        : m_bCrossingValue( bCrossingValue )
{}

XMLAxisPositionPropertyHdl::~XMLAxisPositionPropertyHdl()
{}

sal_Bool XMLAxisPositionPropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = false;

    if( rStrImpValue.equals( GetXMLToken(XML_START) ) )
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_START;
            bResult = true;
        }
    }
    else if( rStrImpValue.equals( GetXMLToken(XML_END) ) )
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_END;
            bResult = true;
        }
    }
    else
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_VALUE;
            bResult = true;
        }
        else
        {
            double fDblValue=0.0;
            bResult = SvXMLUnitConverter::convertDouble( fDblValue, rStrImpValue );
            rValue <<= fDblValue;
        }
    }

    return bResult;
}

sal_Bool XMLAxisPositionPropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

    rtl::OUStringBuffer sValueBuffer;
    if( m_bCrossingValue )
    {
        if(rStrExpValue.getLength() == 0)
        {
            double fValue = 0.0;
            rValue >>= fValue;
            SvXMLUnitConverter::convertDouble( sValueBuffer, fValue );
            rStrExpValue = sValueBuffer.makeStringAndClear();
            bResult = true;
        }
    }
    else
    {
        ::com::sun::star::chart::ChartAxisPosition ePosition( ::com::sun::star::chart::ChartAxisPosition_ZERO );
        rValue >>= ePosition;
        switch(ePosition)
        {
            case ::com::sun::star::chart::ChartAxisPosition_START:
                rStrExpValue = GetXMLToken( XML_START );
                bResult = true;
                break;
            case ::com::sun::star::chart::ChartAxisPosition_END:
                rStrExpValue = GetXMLToken( XML_END );
                bResult = true;
                break;
            case ::com::sun::star::chart::ChartAxisPosition_ZERO:
                SvXMLUnitConverter::convertDouble( sValueBuffer, 0.0 );
                rStrExpValue = sValueBuffer.makeStringAndClear();
                bResult = true;
                break;
            default:
                break;
        }
    }
    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
