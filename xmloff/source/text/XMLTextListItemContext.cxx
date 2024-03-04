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

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "txtparai.hxx"
#include <txtlists.hxx>
#include "XMLTextListBlockContext.hxx"
#include <xmloff/txtimp.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "XMLTextListItemContext.hxx"
#include <sal/log.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;


XMLTextListItemContext::XMLTextListItemContext(
                        SvXMLImport& rImport,
                        XMLTextImportHelper& rTxtImp,
                        const Reference< xml::sax::XFastAttributeList > & xAttrList,
                        const bool bIsHeader )
    : SvXMLImportContext( rImport ),
      rTxtImport( rTxtImp ),
      nStartValue( -1 ),
      mnSubListCount( 0 )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( !bIsHeader && aIter.getToken() == XML_ELEMENT(TEXT, XML_START_VALUE) )
        {
            sal_Int32 nTmp = aIter.toInt32();
            if( nTmp >= 0 && nTmp <= SHRT_MAX )
                nStartValue = static_cast<sal_Int16>(nTmp);
        }
        else if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_OVERRIDE) )
        {
            OUString sListStyleOverrideName = aIter.toString();
            if ( !sListStyleOverrideName.isEmpty() )
            {
                OUString sDisplayStyleName(
                        GetImport().GetStyleDisplayName( XmlStyleFamily::TEXT_LIST,
                                                         sListStyleOverrideName ) );
                const Reference < container::XNameContainer >& rNumStyles =
                                                    rTxtImp.GetNumberingStyles();
                if( rNumStyles.is() && rNumStyles->hasByName( sDisplayStyleName ) )
                {
                    Reference < style::XStyle > xStyle;
                    Any aAny = rNumStyles->getByName( sDisplayStyleName );
                    aAny >>= xStyle;

                    uno::Reference< beans::XPropertySet > xPropSet( xStyle, UNO_QUERY );
                    aAny = xPropSet->getPropertyValue("NumberingRules");
                    aAny >>= mxNumRulesOverride;
                }
                else
                {
                    const SvxXMLListStyleContext* pListStyle =
                                        rTxtImp.FindAutoListStyle( sListStyleOverrideName );
                    if( pListStyle )
                    {
                        mxNumRulesOverride = pListStyle->GetNumRules();
                        if( !mxNumRulesOverride.is() )
                        {
                            pListStyle->CreateAndInsertAuto();
                            mxNumRulesOverride = pListStyle->GetNumRules();
                        }
                    }
                }
            }
        }
        else if ( aIter.getToken() == XML_ELEMENT(XML, XML_ID) )
        {
//FIXME: there is no UNO API for list items
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    // If this is a <text:list-item> element, then remember it as a sign
    // that a bullet has to be generated.
    if( !bIsHeader ) {
        rTxtImport.GetTextListHelper().SetListItem( this );
    }

}

XMLTextListItemContext::~XMLTextListItemContext()
{
}

void XMLTextListItemContext::endFastElement(sal_Int32 )
{
    // finish current list item
    rTxtImport.GetTextListHelper().SetListItem( nullptr );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextListItemContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
    case XML_ELEMENT(TEXT, XML_H):
    case XML_ELEMENT(TEXT, XML_P):
    case XML_ELEMENT(LO_EXT, XML_P):
        pContext = new XMLParaContext( GetImport(), nElement,
                                       xAttrList );
        if (rTxtImport.IsProgress())
            GetImport().GetProgressBarHelper()->Increment();

        break;
    case XML_ELEMENT(TEXT, XML_LIST):
        ++mnSubListCount;
        pContext = new XMLTextListBlockContext( GetImport(), rTxtImport,
                                                xAttrList,
                                                (mnSubListCount > 1) );
        break;
    default:
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
