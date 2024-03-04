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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/style/TabStop.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmltabi.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

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

    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(STYLE, XML_POSITION):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nVal, aIter.toView()))
            {
                aTabStop.Position = nVal;
            }
            break;
        case XML_ELEMENT(STYLE, XML_TYPE):
            if( IsXMLToken( aIter, XML_LEFT ) )
            {
                aTabStop.Alignment = style::TabAlign_LEFT;
            }
            else if( IsXMLToken( aIter, XML_RIGHT ) )
            {
                aTabStop.Alignment = style::TabAlign_RIGHT;
            }
            else if( IsXMLToken( aIter, XML_CENTER ) )
            {
                aTabStop.Alignment = style::TabAlign_CENTER;
            }
            else if( IsXMLToken( aIter, XML_CHAR ) )
            {
                aTabStop.Alignment = style::TabAlign_DECIMAL;
            }
            else if( IsXMLToken( aIter, XML_DEFAULT ) )
            {
                aTabStop.Alignment = style::TabAlign_DEFAULT;
            }
            break;
        case XML_ELEMENT(STYLE, XML_CHAR):
            if( !aIter.isEmpty() )
                aTabStop.DecimalChar = aIter.toString()[0];
            break;
        case XML_ELEMENT(STYLE, XML_LEADER_STYLE):
            if( IsXMLToken( aIter, XML_NONE ) )
                aTabStop.FillChar = ' ';
            else if( IsXMLToken( aIter, XML_DOTTED ) )
                aTabStop.FillChar = '.';
            else
                aTabStop.FillChar = '_';
            break;
        case XML_ELEMENT(STYLE, XML_LEADER_TEXT):
            if( !aIter.isEmpty() )
                cTextFillChar = aIter.toString()[0];
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
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
        maTabStops.push_back( xTabStopContext );

        return xTabStopContext;
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

void SvxXMLTabStopImportContext::endFastElement(sal_Int32 nElement)
{
    sal_uInt16 nCount = maTabStops.size();
    uno::Sequence< style::TabStop> aSeq( nCount );

    if( nCount )
    {
        sal_uInt16 nNewCount = 0;

        style::TabStop* pTabStops = aSeq.getArray();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            SvxXMLTabStopContext_Impl *pTabStopContext = maTabStops[i].get();
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
