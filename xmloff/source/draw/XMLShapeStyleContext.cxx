/*************************************************************************
 *
 *  $RCSfile: XMLShapeStyleContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-12-01 19:19:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
#include "xmlkywd.hxx"

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include "xmlprcon.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmlnmspe.hxx>
#endif

#include "sdpropls.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//////////////////////////////////////////////////////////////////////////////

class XMLShapePropertySetContext : public SvXMLPropertySetContext
{
    SvXMLImportContextRef mxBulletStyle;
    sal_Int32 mnBulletIndex;

public:
    XMLShapePropertySetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                 const ::com::sun::star::uno::Reference<
                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap );

    virtual ~XMLShapePropertySetContext();

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp);
};
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
    }

    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLShapeStyleContext, XMLPropStyleContext );

XMLShapeStyleContext::XMLShapeStyleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    SvXMLStylesContext& rStyles,
    sal_uInt16 nFamily)
:   XMLPropStyleContext(rImport, nPrfx, rLName, xAttrList, rStyles)
{
    // set Family
    SetFamily(nFamily);
}

XMLShapeStyleContext::~XMLShapeStyleContext()
{
}

SvXMLImportContext *XMLShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.compareToAscii( sXML_properties ) == 0 )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new XMLShapePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap );
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLShapeStyleContext::Filter()
{
    const UniReference< XMLPropertySetMapper >&rMapper = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

    ::std::vector< XMLPropertyState > &rProperties = GetProperties();
    std::vector< XMLPropertyState >::iterator end( rProperties.end() );

    for( std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != end;
         property++ )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        if( rMapper->GetEntryContextId( property->mnIndex ) == CTF_NUMBERINGRULES_NAME )
        {
            OUString sName;
            if( property->maValue >>= sName )
            {
                uno::Reference< container::XIndexReplace > xNumRule;
                const SvxXMLListStyleContext *pListStyle = GetImport().GetTextImport()->FindAutoListStyle( sName );

                if( pListStyle )
                {
                    xNumRule = pListStyle->CreateNumRule( GetImport().GetModel() );
                    pListStyle->FillUnoNumRule(xNumRule, NULL /* const SvI18NMap * ??? */ );
                }
                property->maValue <<= xNumRule;
            }
            break;
        }
    }
}
