/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XFormsInstanceContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:31:12 $
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

#include "XFormsInstanceContext.hxx"

#include "DomBuilderContext.hxx"
#include "xformsapi.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <tools/debug.hxx>

#include <xmlnmspe.hxx>
#include <xmltoken.hxx>
#include <xmlimp.hxx>
#include <xmlerror.hxx>
#include <nmspmap.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Sequence;
using com::sun::star::xforms::XModel;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::PropertyValue;
using com::sun::star::xml::sax::XAttributeList;

using xmloff::token::IsXMLToken;
using xmloff::token::XML_INSTANCE;
using xmloff::token::XML_SRC;
using xmloff::token::XML_ID;

static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, SRC ),
    TOKEN_MAP_ENTRY( NONE, ID ),
    XML_TOKEN_MAP_END
};

XFormsInstanceContext::XFormsInstanceContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    Reference<XPropertySet> xModel ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aEmptyMap ),
        mxModel( Reference<XModel>( xModel, UNO_QUERY ) )
{
    DBG_ASSERT( mxModel.is(), "need model" );
}

XFormsInstanceContext::~XFormsInstanceContext()
{
}

SvXMLImportContext* XFormsInstanceContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;

    // only the first element child of an xforms:instance element
    // is used as an instance. The other children remainder must be
    // ignored.
    if( mxInstance.is() )
    {
        GetImport().SetError( XMLERROR_XFORMS_ONLY_ONE_INSTANCE_ELEMENT, rLocalName );
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    else
    {
        // create new DomBuilderContext. Save reference to tree in Model.
        DomBuilderContext* pInstance =
            new DomBuilderContext( GetImport(), nPrefix, rLocalName );
        mxInstance = pInstance->getTree();
        pContext = pInstance;
    }

    DBG_ASSERT( pContext != NULL, "no context!" );
    return pContext;

}

void XFormsInstanceContext::EndElement()
{
    Sequence<PropertyValue> aSequence( 3 );
    PropertyValue* pSequence = aSequence.getArray();
    pSequence[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Instance") );
    pSequence[0].Value <<= mxInstance;
    pSequence[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ID") );
    pSequence[1].Value <<= msId;
    pSequence[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("URL") );
    pSequence[2].Value <<= msURL;

    mxModel->getInstances()->insert( makeAny( aSequence ) );
}


void XFormsInstanceContext::HandleAttribute(
    sal_uInt16 nToken,
    const rtl::OUString& rValue )
{
    switch( nToken )
    {
    case XML_SRC:
        msURL = rValue;
        break;
    case XML_ID:
        msId = rValue;
        break;
    default:
        DBG_ERROR( "should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsInstanceContext::HandleChild(
    sal_uInt16,
    sal_uInt16,
    const OUString&,
    const Reference<XAttributeList>& )
{
    DBG_ERROR( "to be handled by CreateChildContext" );
    return NULL;
}
