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

#include "ChartOASISTContext.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/xmlnamespace.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include <osl/diagnose.h>
#include <xmloff/xmlimp.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLChartOASISTransformerContext::XMLChartOASISTransformerContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLChartOASISTransformerContext::~XMLChartOASISTransformerContext()
{
}

void XMLChartOASISTransformerContext::startFastElement(
    sal_Int32 nElement,
    const Reference< XFastAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_CHART_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    OUString aAddInName;
    Reference< XFastAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        XMLTransformerActions::key_type aKey( xAttrList->getTokenByIndex(i) );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( aIter != pActions->end() )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                        new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            OUString rAttrValue = xAttrList->getValueByIndex(i);
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_IN2INCH:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInWithInch(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::DecodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX:
                OSL_ENSURE( (xAttrList->getTokenByIndex(i) & TOKEN_MASK) == XML_CLASS,
                               "unexpected class token" );
                {
                    OUString aChartClass;
                    sal_uInt16 nValuePrefix =
                        GetTransformer().GetNamespaceMap().GetKeyByAttrName(
                            rAttrValue,
                            &aChartClass );
                    if( XML_NAMESPACE_CHART == nValuePrefix )
                    {
                        pMutableAttrList->SetValueByIndex( i, aChartClass );
                    }
                    else if ( XML_NAMESPACE_OOO == nValuePrefix )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                                GetXMLToken(XML_ADD_IN ) );
                        aAddInName = aChartClass;
                    }
                }
                break;
            default:
                OSL_ENSURE( false, "unknown action" );
                break;
            }
        }
    }

    if( !aAddInName.isEmpty() )
    {
        assert(pMutableAttrList && "coverity[var_deref_model] - pMutableAttrList should be assigned in a superset of the enclosing if condition entry logic");
        pMutableAttrList->AddAttribute( XML_ELEMENT(CHART,XML_ADD_IN_NAME), aAddInName );
    }

    XMLTransformerContext::startFastElement( nElement, xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
