/*************************************************************************
 *
 *  $RCSfile: MetaTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:53:11 $
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

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif

#ifndef _XMLOFF_METATCONTEXT_HXX
#include "MetaTContext.hxx"
#endif

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLTokenEnum aMetaTokens[] =
{
    XML_GENERATOR,
    XML_TITLE,
    XML_DESCRIPTION,
    XML_SUBJECT,
    XML_INITIAL_CREATOR,
    XML_CREATION_DATE,
    XML_CREATOR,
    XML_DATE,
    XML_PRINTED_BY,
    XML_PRINT_DATE,
    XML_KEYWORD,
    XML_LANGUAGE,
    XML_EDITING_CYCLES,
    XML_EDITING_DURATION,
    XML_HYPERLINK_BEHAVIOUR,
    XML_AUTO_RELOAD,
    XML_TEMPLATE,
    XML_USER_DEFINED,
    XML_DOCUMENT_STATISTIC,
    XML_TOKEN_END
};

TYPEINIT1( XMLMetaTransformerContext, XMLTransformerContext );

XMLMetaTransformerContext::XMLMetaTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLMetaTransformerContext::~XMLMetaTransformerContext()
{
}

XMLTransformerContext *XMLMetaTransformerContext::CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList )
{
    XMLPersTextContentTContext *pContext =
        new XMLPersTextContentTContext( GetTransformer(), rQName );
    XMLMetaContexts_Impl::value_type aVal( rLocalName, pContext );
    m_aContexts.insert( aVal );

    return pContext;
}

void XMLMetaTransformerContext::EndElement()
{
    // export everything in the correct order
    XMLTokenEnum *pToken = aMetaTokens;
    while( *pToken != XML_TOKEN_END )
    {
        const OUString& rToken = GetXMLToken( *pToken );
        XMLMetaContexts_Impl::const_iterator aIter =
            m_aContexts.find( rToken );
        if( aIter != m_aContexts.end() )
        {
            if( XML_KEYWORD == *pToken )
            {
                OUString aKeywordsQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_META, GetXMLToken(XML_KEYWORDS ) ) );
                Reference< XAttributeList > xAttrList =
                    new XMLMutableAttributeList;
                GetTransformer().GetDocHandler()->startElement( aKeywordsQName,
                                                            xAttrList );

                XMLMetaContexts_Impl::const_iterator aEndIter =
                    m_aContexts.upper_bound( rToken );
                while( aIter != aEndIter )
                {
                    (*aIter).second->Export();
                    ++aIter;
                }

                GetTransformer().GetDocHandler()->endElement( aKeywordsQName );
            }
            else
            {
                (*aIter).second->Export();
            }
        }
        pToken++;
    }

    GetTransformer().GetDocHandler()->endElement( GetQName() );
}

void XMLMetaTransformerContext::Characters( const OUString& rChars )
{
    // ignore them
}


