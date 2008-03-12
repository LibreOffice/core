/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProcAddAttrTContext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:15:43 $
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

#ifndef _XMLOFF_PROCADDATTRTCONTEXT_HXX
#include "ProcAddAttrTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLProcAddAttrTransformerContext, XMLProcAttrTransformerContext);

XMLProcAddAttrTransformerContext::XMLProcAddAttrTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nActionMap,
        sal_uInt16 nAPrefix,
           ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLProcAttrTransformerContext( rImp, rQName, nActionMap ),
    m_aAttrQName( rImp.GetNamespaceMap().GetQNameByKey( nAPrefix,
                                    ::xmloff::token::GetXMLToken( eAToken ) ) ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLProcAddAttrTransformerContext::XMLProcAddAttrTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
        sal_uInt16 nActionMap,
        sal_uInt16 nAPrefix,
           ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLProcAttrTransformerContext( rImp, rQName, nPrefix,  eToken, nActionMap ),
    m_aAttrQName( rImp.GetNamespaceMap().GetQNameByKey( nAPrefix,
                                    ::xmloff::token::GetXMLToken( eAToken ) ) ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLProcAddAttrTransformerContext::~XMLProcAddAttrTransformerContext()
{
}

void XMLProcAddAttrTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList =
        GetTransformer().ProcessAttrList( xAttrList, GetActionMap(),
                                          sal_False );
    if( !pMutableAttrList )
    {
        pMutableAttrList = new XMLMutableAttributeList;
        xAttrList = pMutableAttrList;
    }
    pMutableAttrList->AddAttribute( m_aAttrQName, m_aAttrValue );
    GetTransformer().GetDocHandler()->startElement( GetElemQName(), xAttrList );
}

