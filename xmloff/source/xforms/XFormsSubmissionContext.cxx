/*************************************************************************
 *
 *  $RCSfile: XFormsSubmissionContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:15:18 $
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

#include "XFormsSubmissionContext.hxx"

#include "xformsapi.hxx"

#include "xmlimp.hxx"
#include "xmlerror.hxx"
#include "xmltoken.hxx"
#include "xmltkmap.hxx"
#include "xmlnmspe.hxx"
#include "nmspmap.hxx"
#include "xmluconv.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel.hpp>

#include <tools/debug.hxx>

using rtl::OUString;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xforms::XModel;

using namespace com::sun::star::uno;
using namespace xmloff::token;




static struct SvXMLTokenMapEntry aAttributeMap[] =
{
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, BIND ),
    TOKEN_MAP_ENTRY( NONE, REF ),
    TOKEN_MAP_ENTRY( NONE, ACTION ),
    TOKEN_MAP_ENTRY( NONE, METHOD ),
    TOKEN_MAP_ENTRY( NONE, VERSION ),
    TOKEN_MAP_ENTRY( NONE, INDENT ),
    TOKEN_MAP_ENTRY( NONE, MEDIATYPE ),
    TOKEN_MAP_ENTRY( NONE, ENCODING ),
    TOKEN_MAP_ENTRY( NONE, OMIT_XML_DECLARATION ),
    TOKEN_MAP_ENTRY( NONE, STANDALONE ),
    TOKEN_MAP_ENTRY( NONE, CDATA_SECTION_ELEMENTS ),
    TOKEN_MAP_ENTRY( NONE, REPLACE ),
    TOKEN_MAP_ENTRY( NONE, SEPARATOR ),
    TOKEN_MAP_ENTRY( NONE, INCLUDENAMESPACEPREFIXES ),
    XML_TOKEN_MAP_END
};

// helper function; see below
void lcl_fillNamespaceContainer( const SvXMLNamespaceMap&,
                                 Reference<XNameContainer>& );

XFormsSubmissionContext::XFormsSubmissionContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XPropertySet>& xModel ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributeMap, aEmptyMap ),
        mxSubmission()
{
    // register submission with model
    DBG_ASSERT( xModel.is(), "need model" );
    Reference<XModel> xXModel( xModel, UNO_QUERY );
    DBG_ASSERT( xXModel.is(), "need XModel" );
    mxSubmission = xXModel->createSubmission().get();
    DBG_ASSERT( mxSubmission.is(), "can't create submission" );
    xXModel->getSubmissions()->insert( makeAny( mxSubmission ) );
}

XFormsSubmissionContext::~XFormsSubmissionContext()
{
}

Any toBool( const OUString& rValue )
{
    Any aValue;
    sal_Bool bValue;
    if( SvXMLUnitConverter::convertBool( bValue, rValue ) )
    {
        aValue <<= ( bValue ? true : false );
    }
    return aValue;
}

void XFormsSubmissionContext::HandleAttribute( sal_uInt16 nToken,
                                               const OUString& rValue )
{
    switch( nToken )
    {
    case XML_ID:
        lcl_setValue( mxSubmission, OUSTRING("ID"), rValue );
        break;
    case XML_BIND:
        lcl_setValue( mxSubmission, OUSTRING("Bind"), rValue );
        break;
    case XML_REF:
        lcl_setValue( mxSubmission, OUSTRING("Ref"), rValue );
        break;
    case XML_ACTION:
        lcl_setValue( mxSubmission, OUSTRING("Action"), rValue );
        break;
    case XML_METHOD:
        lcl_setValue( mxSubmission, OUSTRING("Method"), rValue );
        break;
    case XML_VERSION:
        lcl_setValue( mxSubmission, OUSTRING("Version"), rValue );
        break;
    case XML_INDENT:
        lcl_setValue( mxSubmission, OUSTRING("Indent"), toBool( rValue ) );
        break;
    case XML_MEDIATYPE:
        lcl_setValue( mxSubmission, OUSTRING("MediaType"), rValue );
        break;
    case XML_ENCODING:
        lcl_setValue( mxSubmission, OUSTRING("Encoding"), rValue );
        break;
    case XML_OMIT_XML_DECLARATION:
        lcl_setValue( mxSubmission, OUSTRING("OmitXmlDeclaration"),
                      toBool( rValue ) );
        break;
    case XML_STANDALONE:
        lcl_setValue( mxSubmission, OUSTRING("Standalone"), toBool( rValue ) );
        break;
    case XML_CDATA_SECTION_ELEMENTS:
        lcl_setValue( mxSubmission, OUSTRING("CDataSectionElement"), rValue );
        break;
    case XML_REPLACE:
        lcl_setValue( mxSubmission, OUSTRING("Replace"), rValue );
        break;
    case XML_SEPARATOR:
        lcl_setValue( mxSubmission, OUSTRING("Separator"), rValue );
        break;
    case XML_INCLUDENAMESPACEPREFIXES:
        lcl_setValue( mxSubmission, OUSTRING("IncludeNamespacePrefixes"), rValue );
        break;
    default:
        DBG_ERROR( "unknown attribute" );
        break;
    }
}

/** will be called for each child element */
SvXMLImportContext* XFormsSubmissionContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nNamespace,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    DBG_ERROR( "no children supported" );
    return NULL;
}
