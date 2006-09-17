/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchemaSimpleTypeContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:30:29 $
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

#include "SchemaSimpleTypeContext.hxx"

#include "SchemaRestrictionContext.hxx"
#include <xmltoken.hxx>
#include <nmspmap.hxx>
#include <xmlnmspe.hxx>
#include <xmltkmap.hxx>
#include <xmluconv.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, NAME ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XSD, RESTRICTION ),
    XML_TOKEN_MAP_END
};


SchemaSimpleTypeContext::SchemaSimpleTypeContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XDataTypeRepository>& rRepository ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
        mxRepository( rRepository )
{
}

SchemaSimpleTypeContext::~SchemaSimpleTypeContext()
{
}

void SchemaSimpleTypeContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    if( nToken == XML_NAME )
    {
        msTypeName = rValue;
    }
}

SvXMLImportContext* SchemaSimpleTypeContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;
    switch( nToken )
    {
    case XML_RESTRICTION:
        pContext = new SchemaRestrictionContext( GetImport(),
                                                 nPrefix, rLocalName,
                                                 mxRepository, msTypeName );
        break;
    default:
        DBG_ERROR( "Booo!" );
    }

    return ( pContext != NULL )
        ? pContext
        : new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
