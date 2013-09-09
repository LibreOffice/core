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

#include <sax/tools/converter.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;


enum SvXMLTokenMapDropAttrs
{
    XML_TOK_DROP_LINES,
    XML_TOK_DROP_LENGTH,
    XML_TOK_DROP_DISTANCE,
    XML_TOK_DROP_STYLE,
    XML_TOK_DROP_END=XML_TOK_UNKNOWN
};

static const SvXMLTokenMapEntry aDropAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_LINES,       XML_TOK_DROP_LINES  },
    { XML_NAMESPACE_STYLE, XML_LENGTH,      XML_TOK_DROP_LENGTH },
    { XML_NAMESPACE_STYLE, XML_DISTANCE,    XML_TOK_DROP_DISTANCE   },
    { XML_NAMESPACE_STYLE, XML_STYLE_NAME,  XML_TOK_DROP_STYLE  },
    XML_TOKEN_MAP_END
};

TYPEINIT1( XMLTextDropCapImportContext, XMLElementPropertyContext );
void XMLTextDropCapImportContext::ProcessAttrs(
        const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( aDropAttrTokenMap );

    DropCapFormat aFormat;
    sal_Bool bWholeWord = sal_False;

    sal_Int32 nTmp;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_DROP_LINES:
            if (::sax::Converter::convertNumber( nTmp, rValue, 0, 255 ))
            {
                aFormat.Lines = nTmp < 2 ? 0 : (sal_Int8)nTmp;
            }
            break;

        case XML_TOK_DROP_LENGTH:
            if( IsXMLToken( rValue, XML_WORD ) )
            {
                bWholeWord = sal_True;
            }
            else if (::sax::Converter::convertNumber( nTmp, rValue, 1, 255 ))
            {
                bWholeWord = sal_False;
                aFormat.Count = (sal_Int8)nTmp;
            }
            break;

        case XML_TOK_DROP_DISTANCE:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nTmp, rValue, 0 ))
            {
                aFormat.Distance = (sal_uInt16)nTmp;
            }
            break;

        case XML_TOK_DROP_STYLE:
            sStyleName = rValue;
            break;
        }
    }

    if( aFormat.Lines > 1 && aFormat.Count < 1 )
        aFormat.Count = 1;

    aProp.maValue <<= aFormat;

    aWholeWordProp.maValue.setValue( &bWholeWord, ::getBooleanCppuType() );
}

XMLTextDropCapImportContext::XMLTextDropCapImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nWholeWordIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
    aWholeWordProp( nWholeWordIdx )
{
    ProcessAttrs( xAttrList );
}

XMLTextDropCapImportContext::~XMLTextDropCapImportContext()
{
}

void XMLTextDropCapImportContext::EndElement()
{
    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

    if( -1 != aWholeWordProp.mnIndex )
        rProperties.push_back( aWholeWordProp );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
