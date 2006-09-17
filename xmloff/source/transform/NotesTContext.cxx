/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NotesTContext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:25:59 $
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
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_RENAMEELEMTCONTEXT_HXX
#include "RenameElemTContext.hxx"
#endif
#ifndef _XMLOFF_FLATTCONTEXT_HXX
#include "FlatTContext.hxx"
#endif

#ifndef _XMLOFF_NOTESCONTEXT_HXX
#include "NotesTContext.hxx"
#endif

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLNotesTransformerContext, XMLPersElemContentTContext );

XMLNotesTransformerContext::XMLNotesTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        XMLTokenEnum eToken, sal_Bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_bEndNote( sal_False ),
    m_bPersistent( bPersistent ),
    m_eTypeToken( eToken )
{
}

XMLNotesTransformerContext::~XMLNotesTransformerContext()
{
}

void XMLNotesTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_NOTES_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
        {
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            if( !pMutableAttrList )
            {
                pMutableAttrList =
                    new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_STYLE_FAMILY:
                {
                    if( IsXMLToken( rAttrValue, XML_FOOTNOTE ) )
                    {
                    }
                    else if( IsXMLToken( rAttrValue, XML_ENDNOTE ) )
                    {
                        m_bEndNote = sal_True;
                    }
                    pMutableAttrList->RemoveAttributeByIndex( i );
                    --i;
                    --nAttrCount;
                }
                break;
            case XML_ATACTION_DECODE_STYLE_NAME:
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().DecodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            }
        }
    }

    XMLTokenEnum eToken = XML_FOOTNOTE;
    switch( m_eTypeToken )
    {
    case XML_NOTE:
        eToken = (m_bEndNote ? XML_ENDNOTE : XML_FOOTNOTE);
        break;
    case XML_NOTES_CONFIGURATION:
        eToken = (m_bEndNote ? XML_ENDNOTES_CONFIGURATION
                             : XML_FOOTNOTES_CONFIGURATION);
        break;
    case XML_NOTE_REF:
        eToken = (m_bEndNote ? XML_ENDNOTE_REF : XML_FOOTNOTE_REF);
        break;
    default:
        OSL_ENSURE( XML_NOTE==m_eTypeToken, "invalid note type" );
        break;
    }

    SetExportQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_TEXT,
                            ::xmloff::token::GetXMLToken( eToken ) ) );
    if( m_bPersistent )
        XMLPersElemContentTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLNotesTransformerContext::EndElement()
{
    if( m_bPersistent )
    {
        XMLPersElemContentTContext::EndElement();
    }
    else
    {
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
    }
}

XMLTransformerContext *XMLNotesTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;
    if( XML_NOTE == m_eTypeToken )
    {
        if( XML_NAMESPACE_TEXT == nPrefix )
        {
            XMLTokenEnum eToken ( XML_TOKEN_INVALID );
            if( IsXMLToken( rLocalName, XML_NOTE_CITATION ) )
            {
                eToken = m_bEndNote ? XML_ENDNOTE_CITATION
                                  : XML_FOOTNOTE_CITATION;
            }
            else if( IsXMLToken( rLocalName, XML_NOTE_BODY ) )
            {
                eToken = m_bEndNote ? XML_ENDNOTE_BODY
                                  : XML_FOOTNOTE_BODY;
            }

            if( XML_TOKEN_INVALID != eToken )
            {
                if( m_bPersistent  )
                {
                    pContext = new XMLPersTextContentTContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_TEXT,
                                    eToken );
                    AddContent( pContext );

                }
                else
                {
                    pContext = new XMLRenameElemTransformerContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_TEXT,
                                    eToken );
                }
            }
        }
    }

    if( !pContext )
    {
        pContext = m_bPersistent
                        ? XMLPersElemContentTContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList )
                        : XMLTransformerContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList );
    }

    return pContext;
}

sal_Bool XMLNotesTransformerContext::IsPersistent() const
{
    return m_bPersistent;
}
