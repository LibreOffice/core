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

#include <XMLShapePropertySetContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmltabi.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlimppr.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>

#include "sdpropls.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


XMLShapePropertySetContext::XMLShapePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 sal_uInt32 nFam,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFam,
                             rProps, rMap ),
    mnBulletIndex(-1)
{
}

XMLShapePropertySetContext::~XMLShapePropertySetContext()
{
}

void XMLShapePropertySetContext::EndElement()
{
    Reference< container::XIndexReplace > xNumRule;
    if( mxBulletStyle.is() )
    {
        SvxXMLListStyleContext* pBulletStyle = static_cast<SvxXMLListStyleContext*>(mxBulletStyle.get());
        xNumRule = SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() );
        if( xNumRule.is() )
            pBulletStyle->FillUnoNumRule(xNumRule);
    }

    XMLPropertyState aPropState( mnBulletIndex, Any(xNumRule) );
    mrProperties.push_back( aPropState );

    SvXMLPropertySetContext::EndElement();
}

SvXMLImportContextRef XMLShapePropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_NUMBERINGRULES:
        mnBulletIndex = rProp.mnIndex;
        mxBulletStyle = xContext = new SvxXMLListStyleContext( GetImport(), nPrefix, rLocalName, xAttrList );
        break;
    case CTF_TABSTOP:
        xContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
                                                   rLocalName, rProp,
                                                   rProperties );
        break;
    }

    if (!xContext)
        xContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
