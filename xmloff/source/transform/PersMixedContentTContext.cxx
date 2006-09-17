/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PersMixedContentTContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:27:00 $
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

#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX
#include "PersMixedContentTContext.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

//------------------------------------------------------------------------------
class XMLPersTextTContext_Impl : public XMLTransformerContext
{
    ::rtl::OUString m_aCharacters;

public:
    TYPEINFO();

    XMLPersTextTContext_Impl( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rChars );
    virtual ~XMLPersTextTContext_Impl();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual sal_Bool IsPersistent() const;
    virtual void Export();
    const ::rtl::OUString& GetText() const { return m_aCharacters; }
};

TYPEINIT1( XMLPersTextTContext_Impl, XMLTransformerContext );

XMLPersTextTContext_Impl::XMLPersTextTContext_Impl(
        XMLTransformerBase& rImp,
        const OUString& rChars ) :
    XMLTransformerContext( rImp, OUString() ),
    m_aCharacters( rChars )
{
}

XMLPersTextTContext_Impl::~XMLPersTextTContext_Impl()
{
}

XMLTransformerContext *XMLPersTextTContext_Impl::CreateChildContext(
        sal_uInt16,
        const OUString&,
        const OUString&,
        const Reference< XAttributeList >& )
{
    OSL_ENSURE( !this, "illegal call to CreateChildContext" );
    return 0;
}

void XMLPersTextTContext_Impl::StartElement(
    const Reference< XAttributeList >& )
{
    OSL_ENSURE( !this, "illegal call to StartElement" );
}

void XMLPersTextTContext_Impl::EndElement()
{
    OSL_ENSURE( !this, "illegal call to EndElement" );
}

sal_Bool XMLPersTextTContext_Impl::IsPersistent() const
{
    return sal_True;
}

void XMLPersTextTContext_Impl::Characters( const OUString& rChars )
{
    m_aCharacters += rChars;
}

void XMLPersTextTContext_Impl::Export()
{
    GetTransformer().GetDocHandler()->characters( m_aCharacters );
}

//------------------------------------------------------------------------------

TYPEINIT1( XMLPersMixedContentTContext, XMLPersElemContentTContext );

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersElemContentTContext( rImp, rQName )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLPersElemContentTContext( rImp, rQName, nActionMap )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken, nActionMap )
{
}

XMLPersMixedContentTContext::~XMLPersMixedContentTContext()
{
}

void XMLPersMixedContentTContext::Characters( const OUString& rChars )
{
    AddContent( new XMLPersTextTContext_Impl( GetTransformer(), rChars ) );
}
