/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PersAttrListTContext.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:15:26 $
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

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _XMLOFF_IGNORETCONTEXT_HXX
#include "IgnoreTContext.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersAttrListTContext, XMLTransformerContext );

void XMLPersAttrListTContext::AddAttribute(
        sal_uInt16 nAPrefix,
           ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken )
{
    OUString aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) );
    AddAttribute( nAPrefix, eAToken, aAttrValue );
}

void XMLPersAttrListTContext::AddAttribute(
    sal_uInt16 nAPrefix,
    ::xmloff::token::XMLTokenEnum eAToken,
    const ::rtl::OUString & rValue )
{
    OUString aAttrQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                nAPrefix, ::xmloff::token::GetXMLToken( eAToken ) ) );
    OUString aAttrValue( rValue );

    XMLMutableAttributeList *pMutableAttrList;
    if( m_xAttrList.is() )
    {
        pMutableAttrList =
            static_cast< XMLMutableAttributeList * >( m_xAttrList.get() );
    }
    else
    {
        pMutableAttrList = new XMLMutableAttributeList ;
        m_xAttrList = pMutableAttrList;
    }

    pMutableAttrList->AddAttribute( aAttrQName, aAttrValue );
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( nActionMap )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_nActionMap( nActionMap )
{
}

XMLPersAttrListTContext::~XMLPersAttrListTContext()
{
}

XMLTransformerContext *XMLPersAttrListTContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    // ignore all child elements
    return  new XMLIgnoreTransformerContext( GetTransformer(),
                                             rQName, sal_True, sal_True );
}

void XMLPersAttrListTContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLMutableAttributeList *pMutableAttrList = 0;

    Reference< XAttributeList > xAttrList( rAttrList );
    if( m_nActionMap != INVALID_ACTIONS )
    {
        pMutableAttrList =
            GetTransformer().ProcessAttrList( xAttrList, m_nActionMap,
                                                 sal_True );
    }

    if( m_xAttrList.is() )
    {
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( xAttrList );
    }
    else if( pMutableAttrList )
    {
        m_xAttrList = xAttrList;
    }
    else
    {
        m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
    }
}

void XMLPersAttrListTContext::EndElement()
{
    // ignore for now
}

void XMLPersAttrListTContext::Characters( const OUString& )
{
}

sal_Bool XMLPersAttrListTContext::IsPersistent() const
{
    return sal_True;
}

void XMLPersAttrListTContext::Export()
{
    GetTransformer().GetDocHandler()->startElement( m_aElemQName, m_xAttrList );
    ExportContent();
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLPersAttrListTContext::ExportContent()
{
    // nothing to export
}

Reference< XAttributeList > XMLPersAttrListTContext::GetAttrList() const
{
    return m_xAttrList;
}
