/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormPropOOoTContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:45:33 $
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

#ifndef _XMLOFF_FORMPROPOOOTCONTEXT_HXX
#define _XMLOFF_FORMPROPOOOTCONTEXT_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _XMLOFF_TRANSFORMERCONTEXT_HXX
#include "TransformerContext.hxx"
#endif

class XMLFormPropValueTContext_Impl;

class XMLFormPropOOoTransformerContext : public XMLTransformerContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

    ::rtl::Reference< XMLFormPropValueTContext_Impl > m_xValueContext;

    ::rtl::OUString m_aElemQName;

    sal_Int16 m_nValueTypeAttr;

    ::xmloff::token::XMLTokenEnum m_eValueToken;
    ::xmloff::token::XMLTokenEnum m_eValueTypeToken;

    sal_Bool m_bIsList;

public:
    TYPEINFO();

    XMLFormPropOOoTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    virtual ~XMLFormPropOOoTransformerContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

#endif  //  _XMLOFF_FORMPROPOOOTCONTEXT_HXX

