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


#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlexp.hxx>
#include <XMLBackgroundImageExport.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

XMLBackgroundImageExport::XMLBackgroundImageExport( SvXMLExport& rExp ) :
    rExport( rExp )
{
}

XMLBackgroundImageExport::~XMLBackgroundImageExport()
{
}

void XMLBackgroundImageExport::exportXML( const Any& rGraphicAny,
            const Any *pPos,
            const Any *pFilter,
            const Any *pTransparency,
            sal_uInt16 nPrefix,
            const OUString& rLocalName )
{
    GraphicLocation ePos;
    if( !(pPos && ((*pPos) >>= ePos)) )
        ePos = GraphicLocation_AREA;

    uno::Reference<graphic::XGraphic> xGraphic;
    if (rGraphicAny.has<uno::Reference<graphic::XGraphic>>())
        xGraphic = rGraphicAny.get<uno::Reference<graphic::XGraphic>>();

    if (xGraphic.is() && GraphicLocation_NONE != ePos)
    {
        OUString sUsedMimeType;
        OUString sInternalURL(GetExport().AddEmbeddedXGraphic(xGraphic, sUsedMimeType));

        if (!sInternalURL.isEmpty())
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);
        }

        OUStringBuffer aOut;
        switch( ePos )
        {
        case GraphicLocation_LEFT_TOP:
        case GraphicLocation_MIDDLE_TOP:
        case GraphicLocation_RIGHT_TOP:
            aOut.append( GetXMLToken(XML_TOP) );
            break;
        case GraphicLocation_LEFT_MIDDLE:
        case GraphicLocation_MIDDLE_MIDDLE:
        case GraphicLocation_RIGHT_MIDDLE:
            aOut.append( GetXMLToken(XML_CENTER) );
            break;
        case GraphicLocation_LEFT_BOTTOM:
        case GraphicLocation_MIDDLE_BOTTOM:
        case GraphicLocation_RIGHT_BOTTOM:
            aOut.append( GetXMLToken(XML_BOTTOM) );
            break;
        default:
            break;
        }

        if( !aOut.isEmpty() )
        {
            aOut.append( ' ' );

            switch( ePos )
            {
            case GraphicLocation_LEFT_TOP:
            case GraphicLocation_LEFT_BOTTOM:
            case GraphicLocation_LEFT_MIDDLE:
                aOut.append( GetXMLToken(XML_LEFT) );
                break;
            case GraphicLocation_MIDDLE_TOP:
            case GraphicLocation_MIDDLE_MIDDLE:
            case GraphicLocation_MIDDLE_BOTTOM:
                aOut.append( GetXMLToken(XML_CENTER) );
                break;
            case GraphicLocation_RIGHT_MIDDLE:
            case GraphicLocation_RIGHT_TOP:
            case GraphicLocation_RIGHT_BOTTOM:
                aOut.append( GetXMLToken(XML_RIGHT) );
                break;
            default:
                break;
            }
        }
        if( !aOut.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                  XML_POSITION, aOut.makeStringAndClear() );

        if( GraphicLocation_AREA == ePos )
        {
            aOut.append( GetXMLToken(XML_BACKGROUND_STRETCH) );
        }
        else if( GraphicLocation_NONE != ePos && GraphicLocation_TILED != ePos  )
        {
            aOut.append( GetXMLToken(XML_BACKGROUND_NO_REPEAT) );
        }
        if( !aOut.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REPEAT,
                          aOut.makeStringAndClear() );

        if( pFilter )
        {
            OUString sFilter;
            (*pFilter) >>= sFilter;
            if( !sFilter.isEmpty() )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FILTER_NAME,
                                          sFilter );
        }

        if( pTransparency )
        {
            sal_Int8 nTransparency = sal_Int8();
            if( (*pTransparency) >>= nTransparency )
            {
                OUStringBuffer aTransOut;
                ::sax::Converter::convertPercent(aTransOut, 100-nTransparency);
                GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_OPACITY,
                                          aTransOut.makeStringAndClear() );
            }
        }
    }

    {
        SvXMLElementExport aElem(GetExport(), nPrefix, rLocalName, true, true);
        if (xGraphic.is() && GraphicLocation_NONE != ePos)
        {
            // optional office:binary-data
            GetExport().AddEmbeddedXGraphicAsBase64(xGraphic);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
