/*************************************************************************
 *
 *  $RCSfile: PersAttrListTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:55:31 $
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
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersAttrListTContext, XMLTransformerContext );

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
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    // ignore all child elements
    return  new XMLIgnoreTransformerContext( GetTransformer(),
                                             rQName, sal_True, sal_True );
}

void XMLPersAttrListTContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    OSL_ENSURE( !m_xAttrList.is(), "attr list is already existing" );
    if( m_nActionMap != INVALID_ACTIONS )
    {
        Reference< XAttributeList > xAttrList( rAttrList );
        XMLMutableAttributeList *pMutableAttrList =
            GetTransformer().ProcessAttrList( xAttrList, m_nActionMap,
                                                 sal_True );
        if( pMutableAttrList  )
            m_xAttrList = pMutableAttrList;
    }

    if( !m_xAttrList.is() )
    {
        // clone the attribute list
        m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
    }
}

void XMLPersAttrListTContext::EndElement()
{
    // ignore for now
}

void XMLPersAttrListTContext::Characters( const OUString& rChars )
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
