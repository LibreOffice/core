/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FrameOASISTContext.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:14:38 $
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

#ifndef _XMLOFF_FRAMEOASISTCONTEXT_HXX
#include "FrameOASISTContext.hxx"
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
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
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

TYPEINIT1( XMLFrameOASISTransformerContext, XMLTransformerContext );

sal_Bool XMLFrameOASISTransformerContext::IsLinkedEmbeddedObject(
            const OUString& rLocalName,
            const Reference< XAttributeList >& rAttrList )
{
    if( !(IsXMLToken( rLocalName, XML_OBJECT ) ||
          IsXMLToken( rLocalName, XML_OBJECT_OLE)  ) )
        return sal_False;

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString aAttrName( rAttrList->getNameByIndex( i ) );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( aAttrName,
                                                                 &aLocalName );
        if( XML_NAMESPACE_XLINK == nPrefix &&
            IsXMLToken( aLocalName, XML_HREF ) )
        {
            OUString sHRef( rAttrList->getValueByIndex( i ) );
            if (sHRef.getLength() == 0)
            {
                // When the href is empty then the object is not linked but
                // a placeholder.
                return sal_False;
            }
            GetTransformer().ConvertURIToOOo( sHRef, sal_True );
            return !(sHRef.getLength() && '#'==sHRef[0]);
        }
    }

    return sal_False;
}


XMLFrameOASISTransformerContext::XMLFrameOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_bIgnoreElement( false )
{
}

XMLFrameOASISTransformerContext::~XMLFrameOASISTransformerContext()
{
}

void XMLFrameOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if( (nPrefix == XML_NAMESPACE_PRESENTATION) && IsXMLToken( aLocalName, XML_CLASS ) )
        {
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            if( IsXMLToken( rAttrValue, XML_HEADER ) || IsXMLToken( rAttrValue, XML_FOOTER ) ||
                IsXMLToken( rAttrValue, XML_PAGE_NUMBER ) || IsXMLToken( rAttrValue, XML_DATE_TIME ) )
            {
                m_bIgnoreElement = true;
                break;
            }
        }
    }
}

XMLTransformerContext *XMLFrameOASISTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( m_bIgnoreElement )
    {
        // do not export the frame element and all of its children
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                sal_True, sal_True );
    }
    else
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( OASIS_FRAME_ELEM_ACTIONS );
        OSL_ENSURE( pActions, "go no actions" );
        XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
        XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

        if( !(aIter == pActions->end()) )
        {
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ETACTION_COPY:
                if( !m_aElemQName.getLength() &&
                    !IsLinkedEmbeddedObject( rLocalName, rAttrList ) )
                {
                    pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                sal_False, sal_False );
                    m_aElemQName = rQName;
                    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                        ->AppendAttributeList( rAttrList );
                    GetTransformer().ProcessAttrList( m_xAttrList,
                                                      OASIS_SHAPE_ACTIONS,
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
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    // default is copying
    if( !pContext )
        pContext = XMLTransformerContext::CreateChildContext( nPrefix,
                                                              rLocalName,
                                                              rQName,
                                                              rAttrList );

    return pContext;
}

void XMLFrameOASISTransformerContext::EndElement()
{
    if( !m_bIgnoreElement )
        GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLFrameOASISTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() && !m_bIgnoreElement )
        XMLTransformerContext::Characters( rChars );
}
