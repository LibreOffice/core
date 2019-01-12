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


#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltabe.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry<style::TabAlign> const pXML_tabstop_style[] =
{
    { XML_LEFT,     style::TabAlign_LEFT    },
    { XML_CENTER,   style::TabAlign_CENTER  },
    { XML_RIGHT,    style::TabAlign_RIGHT   },
    { XML_CHAR,     style::TabAlign_DECIMAL },
    { XML_DEFAULT,  style::TabAlign_DEFAULT  }, // ?????????????????????????????????????
    { XML_TOKEN_INVALID,        style::TabAlign(0) }
};

void SvxXMLTabStopExport::exportTabStop( const css::style::TabStop* pTabStop )
{
    SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

    // text:level
    OUStringBuffer sBuffer;

    // position attribute
    rUnitConv.convertMeasureToXML( sBuffer, pTabStop->Position );
    rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_POSITION,
                           sBuffer.makeStringAndClear() );

    // type attribute
    if( style::TabAlign_LEFT != pTabStop->Alignment )
    {
        SvXMLUnitConverter::convertEnum( sBuffer, pTabStop->Alignment,
                                   pXML_tabstop_style );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_TYPE,
                               sBuffer.makeStringAndClear() );
    }

    // char
    if( style::TabAlign_DECIMAL == pTabStop->Alignment &&
        pTabStop->DecimalChar != 0 )
    {
        sBuffer.append( pTabStop->DecimalChar );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_CHAR,
                               sBuffer.makeStringAndClear() );
    }

    // leader-char
    if( ' ' != pTabStop->FillChar && 0 != pTabStop->FillChar )
    {
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEADER_STYLE,
                      GetXMLToken('.' == pTabStop->FillChar ? XML_DOTTED
                                                               : XML_SOLID) );

        sBuffer.append( pTabStop->FillChar );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEADER_TEXT,
                               sBuffer.makeStringAndClear() );
    }

    SvXMLElementExport rElem( rExport, XML_NAMESPACE_STYLE, XML_TAB_STOP,
                              true, true );
}


SvxXMLTabStopExport::SvxXMLTabStopExport(
    SvXMLExport& rExp)
    : rExport( rExp )
{
}

SvxXMLTabStopExport::~SvxXMLTabStopExport()
{
}

void SvxXMLTabStopExport::Export( const uno::Any& rAny )
{
    uno::Sequence< css::style::TabStop> aSeq;
    if(!(rAny >>= aSeq))
    {
        OSL_FAIL( "SvxXMLTabStopExport needs a Sequence css::style::TabStop>" );
    }
    else
    {
        const css::style::TabStop* pTabs = aSeq.getConstArray();
        const sal_Int32 nTabs   = aSeq.getLength();

        SvXMLElementExport rElem( rExport, XML_NAMESPACE_STYLE, XML_TAB_STOPS,
                                  true, true );

        for( sal_Int32 nIndex = 0; nIndex < nTabs; nIndex++ )
        {
            if( style::TabAlign_DEFAULT != pTabs[nIndex].Alignment )
                exportTabStop( &(pTabs[nIndex]) );
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
