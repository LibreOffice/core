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


#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_ 
#include <com/sun/star/xml/sax/XParser.hpp>
#endif


#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif





#ifndef _XMLOFF_XMLIMP_HXX 
#include <bf_xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX 
#include <bf_xmloff/xmlstyle.hxx>
#endif

#ifndef _SVX_EDITSOURCE_HXX
#include "editsource.hxx"
#endif

#ifndef _SVX_UNOTEXT_HXX 
#include "unotext.hxx"
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::rtl;
using namespace cppu;
using namespace xmloff::token;


///////////////////////////////////////////////////////////////////////

/*N*/ class SvxXMLTextImportContext : public SvXMLImportContext
/*N*/ {
/*N*/ public:
/*N*/ 	SvxXMLTextImportContext( SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, const Reference< XText >& xText );
/*N*/ 	virtual ~SvxXMLTextImportContext();
/*N*/ 
/*N*/ 	virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );
/*N*/ 
/*N*/ //	SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }
/*N*/ 
/*N*/ private:
/*N*/ 	const Reference< XText > mxText;
/*N*/ };
/*N*/ 
/*N*/ ///////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ SvxXMLTextImportContext::SvxXMLTextImportContext( SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, const Reference< XText >& xText )
/*N*/ : SvXMLImportContext( rImport, nPrfx, rLName ), mxText( xText )
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ SvxXMLTextImportContext::~SvxXMLTextImportContext()
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ SvXMLImportContext *SvxXMLTextImportContext::CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
/*N*/ {
/*N*/ 	SvXMLImportContext* pContext = NULL;
/*N*/ 	if(XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_BODY ) )
/*N*/ 	{
/*N*/ 		pContext = new SvxXMLTextImportContext( GetImport(), nPrefix, rLocalName, xAttrList, mxText );
/*N*/ 	}
/*N*/ 	else if( XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_AUTOMATIC_STYLES ) )
/*N*/ 	{
/*N*/ 		pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
/*N*/ 		GetImport().GetTextImport()->SetAutoStyles( (SvXMLStylesContext*)pContext );
/*N*/ 
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
/*N*/ 	}
/*N*/ 	
/*N*/ 	if( NULL == pContext )
/*N*/ 		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
/*N*/ 
/*N*/ 	return pContext;
/*N*/ }
}
