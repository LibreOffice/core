/*************************************************************************
 *
 *  $RCSfile: XFormsModelContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:14:43 $
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

#include "XFormsModelContext.hxx"

#include "XFormsBindContext.hxx"
#include "XFormsSubmissionContext.hxx"
#include "XFormsInstanceContext.hxx"
#include "SchemaContext.hxx"
#include "xformsapi.hxx"

#include "xmlimp.hxx"
#include "xmlnmspe.hxx"
#include "nmspmap.hxx"
#include "xmltoken.hxx"
#include "xmlerror.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/xforms/XModel.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::util::XUpdatable;
using namespace com::sun::star::uno;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, SCHEMA ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XFORMS, INSTANCE ),
    TOKEN_MAP_ENTRY( XFORMS, BIND ),
    TOKEN_MAP_ENTRY( XFORMS, SUBMISSION ),
    TOKEN_MAP_ENTRY( XSD,    SCHEMA ),
    XML_TOKEN_MAP_END
};


XFormsModelContext::XFormsModelContext( SvXMLImport& rImport,
                                        USHORT nPrefix,
                                        const OUString& rLocalName ) :
    TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
    mxModel( lcl_createXFormsModel() )
{
}

XFormsModelContext::~XFormsModelContext()
{
}


Reference<XPropertySet> XFormsModelContext::getModel()
{
    return mxModel;
}


void XFormsModelContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    switch( nToken )
    {
    case XML_ID:
        mxModel->setPropertyValue( OUSTRING("ID"), makeAny( rValue ) );
        break;
    case XML_SCHEMA:
        GetImport().SetError( XMLERROR_XFORMS_NO_SCHEMA_SUPPORT );
        break;
    default:
        DBG_ERROR( "this should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsModelContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    switch( nToken )
    {
    case XML_INSTANCE:
        pContext = new XFormsInstanceContext( GetImport(), nPrefix, rLocalName,
                                              mxModel );
        break;
    case XML_BIND:
        pContext = new XFormsBindContext( GetImport(), nPrefix, rLocalName,
                                          mxModel );
        break;
    case XML_SUBMISSION:
        pContext = new XFormsSubmissionContext( GetImport(), nPrefix,
                                                rLocalName, mxModel );
        break;
    case XML_SCHEMA:
        pContext = new SchemaContext(
            GetImport(), nPrefix, rLocalName,
            Reference<com::sun::star::xforms::XModel>( mxModel,
                                                       UNO_QUERY_THROW )
                ->getDataTypeRepository() );
        break;
    default:
        DBG_ERROR( "Boooo!" );
        break;
    }

    return pContext;
}

void XFormsModelContext::EndElement()
{
    // update before putting model into document
    Reference<XUpdatable> xUpdate( mxModel, UNO_QUERY );
    if( xUpdate.is() )
        xUpdate->update();

    GetImport().initXForms();
    lcl_addXFormsModel( GetImport().GetModel(), getModel() );
}
