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
#include <XMLTextColumnsContext.hxx>
#include <xmloff/XMLComplexColorContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmltabi.hxx>
#include <xmlsdtypes.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlimppr.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


XMLShapePropertySetContext::XMLShapePropertySetContext(
                 SvXMLImport& rImport, sal_Int32 nElement,
                 const Reference< xml::sax::XFastAttributeList > & xAttrList,
                 sal_uInt32 nFam,
                 ::std::vector< XMLPropertyState > &rProps,
                 SvXMLImportPropertyMapper* pMap ) :
    SvXMLPropertySetContext( rImport, nElement, xAttrList, nFam,
                             rProps, pMap ),
    mnBulletIndex(-1)
{
}

XMLShapePropertySetContext::~XMLShapePropertySetContext()
{
}

void XMLShapePropertySetContext::endFastElement(sal_Int32 )
{
    Reference< container::XIndexReplace > xNumRule;
    if( mxBulletStyle.is() )
    {
        xNumRule = SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() );
        if( xNumRule.is() )
            mxBulletStyle->FillUnoNumRule(xNumRule);
    }

    XMLPropertyState aPropState( mnBulletIndex, Any(xNumRule) );
    mrProperties.push_back( aPropState );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLShapePropertySetContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    switch( mpMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_NUMBERINGRULES:
        mnBulletIndex = rProp.mnIndex;
        mxBulletStyle = new SvxXMLListStyleContext( GetImport() );
        return mxBulletStyle;
    case CTF_TABSTOP:
        return new SvxXMLTabStopImportContext( GetImport(), nElement,
                                                   rProp,
                                                   rProperties );
    case CTF_TEXTCOLUMNS:
        return new XMLTextColumnsContext(GetImport(), nElement, xAttrList, rProp, rProperties);

    case CTF_COMPLEX_COLOR:
        return new XMLPropertyComplexColorContext(GetImport(), nElement, xAttrList, rProp, rProperties);
    }

    return SvXMLPropertySetContext::createFastChildContext( nElement,
                                                            xAttrList,
                                                            rProperties, rProp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
