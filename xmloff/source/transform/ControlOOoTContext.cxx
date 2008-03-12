/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlOOoTContext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:13:15 $
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

#ifndef _XMLOFF_CONTROLOOOTCONTEXT_HXX
#include "ControlOOoTContext.hxx"
#endif
#ifndef _XMLOFF_IGNORETCONTEXT_HXX
#include "IgnoreTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOOO_HXX
#include "ActionMapTypesOOo.hxx"
#endif
#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#include "ElemTransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLControlOOoTransformerContext, XMLTransformerContext );

XMLControlOOoTransformerContext::XMLControlOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLControlOOoTransformerContext::~XMLControlOOoTransformerContext()
{
}

void XMLControlOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLControlOOoTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( !m_aElemQName.getLength() )
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_False, sal_False );
        m_aElemQName = rQName;
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        GetTransformer().ProcessAttrList( m_xAttrList,
                                          OOO_FORM_CONTROL_ACTIONS,
                                          sal_False );
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        m_xAttrList );
    }
    else
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_True, sal_True );
    }
    return pContext;
}

void XMLControlOOoTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLControlOOoTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() )
        XMLTransformerContext::Characters( rChars );
}
