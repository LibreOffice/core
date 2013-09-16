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


#include "XMLAxisPositionPropertyHdl.hxx"

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/chart/ChartAxisPosition.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>


using namespace ::xmloff::token;

using namespace com::sun::star;

XMLAxisPositionPropertyHdl::XMLAxisPositionPropertyHdl( bool bCrossingValue )
        : m_bCrossingValue( bCrossingValue )
{}

XMLAxisPositionPropertyHdl::~XMLAxisPositionPropertyHdl()
{}

bool XMLAxisPositionPropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool bResult = false;

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
            bResult = ::sax::Converter::convertDouble(fDblValue, rStrImpValue);
            rValue <<= fDblValue;
        }
    }

    return bResult;
}

bool XMLAxisPositionPropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool bResult = sal_False;

    OUStringBuffer sValueBuffer;
    if( m_bCrossingValue )
    {
        if(rStrExpValue.isEmpty())
        {
            double fValue = 0.0;
            rValue >>= fValue;
            ::sax::Converter::convertDouble( sValueBuffer, fValue );
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
                ::sax::Converter::convertDouble( sValueBuffer, 0.0 );
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
