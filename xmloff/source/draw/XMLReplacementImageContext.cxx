/*************************************************************************
 *
 *  $RCSfile: XMLReplacementImageContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 12:59:48 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX
#include "XMLReplacementImageContext.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;

TYPEINIT1( XMLReplacementImageContext, SvXMLImportContext );

XMLReplacementImageContext::XMLReplacementImageContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        const Reference< XPropertySet > & rPropSet ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    m_xPropSet( rPropSet ),
    m_sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL"))
{
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        const OUString& rValue = rAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_FRAME_HREF:
            m_sHRef = rValue;
            break;
        }
    }
}

XMLReplacementImageContext::~XMLReplacementImageContext()
{
}

void XMLReplacementImageContext::EndElement()
{
    OSL_ENSURE( m_sHRef.getLength() > 0 || m_xBase64Stream.is(),
                "neither URL nor base64 image data given" );
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    OUString sHRef;
    if( m_sHRef.getLength() )
    {
        UniReference < XMLTextImportHelper > xTxtImport =
            GetImport().GetTextImport();
        sal_Bool bForceLoad = xTxtImport->IsInsertMode() ||
                              xTxtImport->IsBlockMode() ||
                              xTxtImport->IsStylesOnlyMode() ||
                              xTxtImport->IsOrganizerMode();
        sHRef = GetImport().ResolveGraphicObjectURL( m_sHRef, !bForceLoad );
    }
    else if( m_xBase64Stream.is() )
    {
        sHRef = GetImport().ResolveGraphicObjectURLFromBase64( m_xBase64Stream );
        m_xBase64Stream = 0;
    }

    Reference < XPropertySetInfo > xPropSetInfo =
        m_xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( m_sGraphicURL ) )
        m_xPropSet->setPropertyValue( m_sGraphicURL, makeAny( sHRef ) );
}

SvXMLImportContext *XMLReplacementImageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_OFFICE == nPrefix &&
        IsXMLToken( rLocalName, ::xmloff::token::XML_BINARY_DATA ) &&
        !m_xBase64Stream.is() )
    {
        m_xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
        if( m_xBase64Stream.is() )
            pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    m_xBase64Stream );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


