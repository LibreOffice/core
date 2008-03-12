/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLShapePropertySetContext.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:30:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLSHAPEPROPERTYSETCONTEXT_HXX_
#include "XMLShapePropertySetContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLNUMI_HXX
#include <xmloff/xmlnumi.hxx>
#endif
#ifndef _XMLOFF_XMLTABI_HXX
#include "xmltabi.hxx"
#endif
#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include <xmloff/txtprmap.hxx>
#endif

#include "sdpropls.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLShapePropertySetContext, SvXMLPropertySetContext );

XMLShapePropertySetContext::XMLShapePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 sal_uInt32 nFam,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap ) :
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
    if( mxBulletStyle.Is() )
    {
        SvxXMLListStyleContext* pBulletStyle = (SvxXMLListStyleContext*)&mxBulletStyle;
        xNumRule = pBulletStyle->CreateNumRule( GetImport().GetModel() );
        if( xNumRule.is() )
            pBulletStyle->FillUnoNumRule(xNumRule, NULL /* const SvI18NMap * ??? */ );
    }

    Any aAny;
    aAny <<= xNumRule;

    XMLPropertyState aPropState( mnBulletIndex, aAny );
    mrProperties.push_back( aPropState );

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

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
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
