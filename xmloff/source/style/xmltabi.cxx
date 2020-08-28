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

#include <com/sun/star/style/TabAlign.hpp>
#include <sal/log.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/style/TabStop.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmltabi.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

enum SvXMLTokenMapAttrs
{
    XML_TOK_TABSTOP_POSITION,
    XML_TOK_TABSTOP_TYPE,
    XML_TOK_TABSTOP_CHAR,
    XML_TOK_TABSTOP_LEADER_STYLE,
    XML_TOK_TABSTOP_LEADER_TEXT
};

}

const SvXMLTokenMapEntry aTabsAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE, XML_POSITION,     XML_TOK_TABSTOP_POSITION },
    { XML_NAMESPACE_STYLE, XML_TYPE,         XML_TOK_TABSTOP_TYPE },
    { XML_NAMESPACE_STYLE, XML_CHAR,         XML_TOK_TABSTOP_CHAR },
    { XML_NAMESPACE_STYLE, XML_LEADER_TEXT,  XML_TOK_TABSTOP_LEADER_TEXT },
    { XML_NAMESPACE_STYLE, XML_LEADER_STYLE,  XML_TOK_TABSTOP_LEADER_STYLE },
    XML_TOKEN_MAP_END
};


class SvxXMLTabStopContext_Impl : public SvXMLImportContext
{
private:
     style::TabStop aTabStop;

public:

    SvxXMLTabStopContext_Impl( SvXMLImport& rImport, sal_Int32 nElement,
                               const uno::Reference< xml::sax::XFastAttributeList > & xAttrList );

    const style::TabStop& getTabStop() const { return aTabStop; }
};


SvxXMLTabStopContext_Impl::SvxXMLTabStopContext_Impl(
                               SvXMLImport& rImport, sal_Int32 /*nElement*/,
                               const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
: SvXMLImportContext( rImport )
{
    aTabStop.Position = 0;
    aTabStop.Alignment = style::TabAlign_LEFT;
    aTabStop.DecimalChar = ',';
    aTabStop.FillChar = ' ';
    sal_Unicode cTextFillChar = 0;

    static const SvXMLTokenMap aTokenMap( aTabsAttributesAttrTokenMap );

    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        const OUString sValue = aIter.toString();

        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(STYLE, XML_POSITION):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nVal, sValue))
            {
                aTabStop.Position = nVal;
            }
            break;
        case XML_ELEMENT(STYLE, XML_TYPE):
            if( IsXMLToken( sValue, XML_LEFT ) )
            {
                aTabStop.Alignment = style::TabAlign_LEFT;
            }
            else if( IsXMLToken( sValue, XML_RIGHT ) )
            {
                aTabStop.Alignment = style::TabAlign_RIGHT;
            }
            else if( IsXMLToken( sValue, XML_CENTER ) )
            {
                aTabStop.Alignment = style::TabAlign_CENTER;
            }
            else if( IsXMLToken( sValue, XML_CHAR ) )
            {
                aTabStop.Alignment = style::TabAlign_DECIMAL;
            }
            else if( IsXMLToken( sValue, XML_DEFAULT ) )
            {
                aTabStop.Alignment = style::TabAlign_DEFAULT;
            }
            break;
        case XML_ELEMENT(STYLE, XML_CHAR):
            if( !sValue.isEmpty() )
                aTabStop.DecimalChar = sValue[0];
            break;
        case XML_ELEMENT(STYLE, XML_LEADER_STYLE):
            if( IsXMLToken( sValue, XML_NONE ) )
                aTabStop.FillChar = ' ';
            else if( IsXMLToken( sValue, XML_DOTTED ) )
                aTabStop.FillChar = '.';
            else
                aTabStop.FillChar = '_';
            break;
        case XML_ELEMENT(STYLE, XML_LEADER_TEXT):
            if( !sValue.isEmpty() )
                cTextFillChar = sValue[0];
            break;
        default:
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }

    if( cTextFillChar != 0 && aTabStop.FillChar != ' ' )
        aTabStop.FillChar = cTextFillChar;
}


SvxXMLTabStopImportContext::SvxXMLTabStopImportContext(
                                SvXMLImport& rImport, sal_Int32 nElement,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps )
: XMLElementPropertyContext( rImport, nElement, rProp, rProps )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvxXMLTabStopImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    if( nElement == XML_ELEMENT(STYLE, XML_TAB_STOP) )
    {
        // create new tabstop import context
        const rtl::Reference<SvxXMLTabStopContext_Impl> xTabStopContext{
            new SvxXMLTabStopContext_Impl( GetImport(), nElement, xAttrList )};

        // add new tabstop to array of tabstops
        if( !mpTabStops )
            mpTabStops = std::make_unique<SvxXMLTabStopArray_Impl>();

        mpTabStops->push_back( xTabStopContext );

        return xTabStopContext.get();
    }
    else
        SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));

    return nullptr;
}

void SvxXMLTabStopImportContext::endFastElement(sal_Int32 nElement)
{
    sal_uInt16 nCount = mpTabStops ? mpTabStops->size() : 0;
    uno::Sequence< style::TabStop> aSeq( nCount );

    if( mpTabStops )
    {
        sal_uInt16 nNewCount = 0;

        style::TabStop* pTabStops = aSeq.getArray();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            SvxXMLTabStopContext_Impl *pTabStopContext = (*mpTabStops)[i].get();
            const style::TabStop& rTabStop = pTabStopContext->getTabStop();
            bool bDflt = style::TabAlign_DEFAULT == rTabStop.Alignment;
            if( !bDflt || 0==i )
            {
                *pTabStops++ = pTabStopContext->getTabStop();
                nNewCount++;
            }
            if( bDflt && 0==i )
                break;
        }

        if( nCount != nNewCount )
            aSeq.realloc( nNewCount );
    }
    aProp.maValue <<= aSeq;

    SetInsert( true );
    XMLElementPropertyContext::endFastElement(nElement);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
