/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLReplacementImageContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:25:36 $
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


