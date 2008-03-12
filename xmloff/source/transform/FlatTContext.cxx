/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FlatTContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:14:22 $
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

#ifndef _XMLOFF_FLATTCONTEXT_HXX
#include "FlatTContext.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersTextContentTContext, XMLPersAttrListTContext );

XMLPersTextContentTContext::XMLPersTextContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersAttrListTContext( rImp, rQName )
{
}

XMLPersTextContentTContext::XMLPersTextContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nActionMap )
{
}

XMLPersTextContentTContext::XMLPersTextContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken )
{
}

XMLPersTextContentTContext::XMLPersTextContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken, nActionMap )
{
}

XMLPersTextContentTContext::~XMLPersTextContentTContext()
{
}

void XMLPersTextContentTContext::Characters( const OUString& rChars )
{
    m_aCharacters += rChars;
}

void XMLPersTextContentTContext::ExportContent()
{
    GetTransformer().GetDocHandler()->characters( m_aCharacters );
}
