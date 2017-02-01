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

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "txtdrope.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::uno;

using namespace ::xmloff::token;


XMLTextDropCapExport::XMLTextDropCapExport( SvXMLExport& rExp ) :
    rExport(rExp)
{
}

XMLTextDropCapExport::~XMLTextDropCapExport()
{
}

void XMLTextDropCapExport::exportXML( const Any& rAny,
                                      bool bWholeWord,
                                      const OUString& rStyleName )
{
    DropCapFormat aFormat;
    rAny >>= aFormat;
    OUString sValue;
    OUStringBuffer sBuffer;
    if( aFormat.Lines > 1 )
    {
        SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

        // style:lines
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LINES,
                              OUString::number( aFormat.Lines ) );

        // style:length
        if( bWholeWord )
        {
            sValue = GetXMLToken(XML_WORD);
        }
        else if( aFormat.Count > 1 )
        {
            sValue = OUString::number(aFormat.Count);
        }
        if( !sValue.isEmpty() )
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LENGTH, sValue );

        // style:distance
        if( aFormat.Distance > 0 )
        {
            rUnitConv.convertMeasureToXML( sBuffer, aFormat.Distance );
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_DISTANCE,
                                  sBuffer.makeStringAndClear() );
        }

        // style:style-name
        if( !rStyleName.isEmpty() )
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_STYLE_NAME,
                                  rExport.EncodeStyleName( rStyleName ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_DROP_CAP,
                              false, false );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
