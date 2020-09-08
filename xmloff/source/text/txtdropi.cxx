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


#include "txtdropi.hxx"

#include <com/sun/star/style/DropCapFormat.hpp>

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

namespace {

enum SvXMLTokenMapDropAttrs
{
    XML_TOK_DROP_LINES,
    XML_TOK_DROP_LENGTH,
    XML_TOK_DROP_DISTANCE,
    XML_TOK_DROP_STYLE
};

}

const SvXMLTokenMapEntry aDropAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_LINES,       XML_TOK_DROP_LINES  },
    { XML_NAMESPACE_STYLE, XML_LENGTH,      XML_TOK_DROP_LENGTH },
    { XML_NAMESPACE_STYLE, XML_DISTANCE,    XML_TOK_DROP_DISTANCE   },
    { XML_NAMESPACE_STYLE, XML_STYLE_NAME,  XML_TOK_DROP_STYLE  },
    XML_TOKEN_MAP_END
};

void XMLTextDropCapImportContext::ProcessAttrs(
        const Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    DropCapFormat aFormat;
    bool bWholeWord = false;

    sal_Int32 nTmp;
    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        const OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
        case XML_ELEMENT(STYLE, XML_LINES):
            if (::sax::Converter::convertNumber( nTmp, sValue, 0, 255 ))
            {
                aFormat.Lines = nTmp < 2 ? 0 : static_cast<sal_Int8>(nTmp);
            }
            break;

        case XML_ELEMENT(STYLE, XML_LENGTH):
            if( IsXMLToken( sValue, XML_WORD ) )
            {
                bWholeWord = true;
            }
            else if (::sax::Converter::convertNumber( nTmp, sValue, 1, 255 ))
            {
                bWholeWord = false;
                aFormat.Count = static_cast<sal_Int8>(nTmp);
            }
            break;

        case XML_ELEMENT(STYLE, XML_DISTANCE):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nTmp, sValue, 0 ))
            {
                aFormat.Distance = static_cast<sal_uInt16>(nTmp);
            }
            break;

        case XML_ELEMENT(STYLE, XML_STYLE_NAME):
            sStyleName = sValue;
            break;

        default:
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }

    if( aFormat.Lines > 1 && aFormat.Count < 1 )
        aFormat.Count = 1;

    aProp.maValue <<= aFormat;

    aWholeWordProp.maValue <<= bWholeWord;
}

XMLTextDropCapImportContext::XMLTextDropCapImportContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nWholeWordIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nElement, rProp, rProps ),
    aWholeWordProp( nWholeWordIdx )
{
    ProcessAttrs( xAttrList );
}

XMLTextDropCapImportContext::~XMLTextDropCapImportContext()
{
}

void XMLTextDropCapImportContext::endFastElement(sal_Int32 nElement)
{
    SetInsert( true );
    XMLElementPropertyContext::endFastElement(nElement);

    if( -1 != aWholeWordProp.mnIndex )
        rProperties.push_back( aWholeWordProp );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
