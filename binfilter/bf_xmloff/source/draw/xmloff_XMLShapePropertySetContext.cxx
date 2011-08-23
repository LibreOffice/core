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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _XMLOFF_XMLSHAPEPROPERTYSETCONTEXT_HXX_
#include "XMLShapePropertySetContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNUMI_HXX 
#include "xmlnumi.hxx"
#endif
#ifndef _XMLOFF_XMLTABI_HXX 
#include "xmltabi.hxx"
#endif
#ifndef _XMLOFF_TEXTPRMAP_HXX_ 
#include "txtprmap.hxx"
#endif

#include "sdpropls.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLShapePropertySetContext, SvXMLPropertySetContext );

XMLShapePropertySetContext::XMLShapePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps, rMap ),
    mnBulletIndex(-1)
{
}

XMLShapePropertySetContext::~XMLShapePropertySetContext()
{
}

void XMLShapePropertySetContext::EndElement()
{
    Reference< container::XIndexReplace > xNumRule;
    if( mxBulletStyle.Is() )
    {
        SvxXMLListStyleContext* pBulletStyle = (SvxXMLListStyleContext*)&mxBulletStyle;
        xNumRule = pBulletStyle->CreateNumRule( GetImport().GetModel() );
        pBulletStyle->FillUnoNumRule(xNumRule, NULL /* const SvI18NMap * ??? */ );
    }

    Any aAny;
    aAny <<= xNumRule;

    XMLPropertyState aPropState( mnBulletIndex, aAny );
    rProperties.push_back( aPropState );

    SvXMLPropertySetContext::EndElement();
}
    
SvXMLImportContext *XMLShapePropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_NUMBERINGRULES:
        mnBulletIndex = rProp.mnIndex;
        mxBulletStyle = pContext = new SvxXMLListStyleContext( GetImport(), nPrefix, rLocalName, xAttrList );
        break;
    case CTF_TABSTOP:
        pContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
                                                   rLocalName, rProp,
                                                   rProperties );
        break;
    }
    
    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, 
                                                            rProperties, rProp );
    
    return pContext;
}
}//end of namespace binfilter
