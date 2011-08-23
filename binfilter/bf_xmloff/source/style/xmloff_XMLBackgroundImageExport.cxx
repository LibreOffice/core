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

#include <com/sun/star/style/GraphicLocation.hpp>

#include <xmlnmspe.hxx>


#include "xmlexp.hxx"
#include "XMLBackgroundImageExport.hxx"
#include "xmluconv.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::binfilter::xmloff::token;

XMLBackgroundImageExport::XMLBackgroundImageExport( SvXMLExport& rExp ) :
    rExport( rExp )
{
}

XMLBackgroundImageExport::~XMLBackgroundImageExport()
{
}

void XMLBackgroundImageExport::exportXML( const Any& rURL,
            const Any *pPos,
            const Any *pFilter,
            const Any *pTransparency,
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName )
{
    GraphicLocation ePos;
    if( !(pPos && ((*pPos) >>= ePos)) )
        ePos = GraphicLocation_AREA;

    OUString sURL;
    rURL >>= sURL;
    if( sURL.getLength() && GraphicLocation_NONE != ePos )
    {
        OUString sTempURL( GetExport().AddEmbeddedGraphicObject( sURL ) );
        if( sTempURL.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sTempURL );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, 
                                      XML_SIMPLE );
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE,
                                      XML_ONLOAD );
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
        }

        if( aOut.getLength() )
        {
            aOut.append( sal_Unicode( ' ' ) );

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
            }
        }
        if( aOut.getLength() )
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
        if( aOut.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REPEAT,
                          aOut.makeStringAndClear() );

        if( pFilter )
        {
            OUString sFilter;
            (*pFilter) >>= sFilter;
            if( sFilter.getLength() )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FILTER_NAME,
                                          sFilter );
        }

        if( pTransparency )
        {
            sal_Int8 nTransparency;
            if( (*pTransparency) >>= nTransparency )
            {
                OUStringBuffer aOut;
                SvXMLUnitConverter::convertPercent( aOut, nTransparency );
                GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_TRANSPARENCY,
                                          aOut.makeStringAndClear() );
            }
        }
    }

    {
        SvXMLElementExport aElem( GetExport(), nPrefix, rLocalName, sal_True, sal_True );
        if( sURL.getLength() && GraphicLocation_NONE != ePos )
        {
            // optional office:binary-data
            GetExport().AddEmbeddedGraphicObjectAsBase64( sURL );
        }
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
