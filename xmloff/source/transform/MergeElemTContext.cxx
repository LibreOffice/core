/*************************************************************************
 *
 *  $RCSfile: MergeElemTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:52:31 $
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

#ifndef _XMLOFF_MERGEELEMTCONTEXT_HXX
#include "MergeElemTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLMergeElemTransformerContext, XMLTransformerContext );

void XMLMergeElemTransformerContext::ExportStartElement()
{
    XMLPersTextContentTContextVector::iterator aIter = m_aChildContexts.begin();

    for( ; aIter != m_aChildContexts.end(); ++aIter )
    {
        XMLPersTextContentTContext *pContext = (*aIter).get();
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( pContext->GetExportQName(),
                            pContext->GetTextContent() );
    }
    XMLTransformerContext::StartElement( m_xAttrList );

    m_bStartElementExported = sal_True;
}

XMLMergeElemTransformerContext::XMLMergeElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap ),
    m_bStartElementExported( sal_False )
{
}

XMLMergeElemTransformerContext::~XMLMergeElemTransformerContext()
{
}

void XMLMergeElemTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLMergeElemTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( !m_bStartElementExported )
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( m_nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
        if( pActions )
        {
            XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
            XMLTransformerActions::const_iterator aIter =
                pActions->find( aKey );

            if( !(aIter == pActions->end()) )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ATACTION_MOVE_FROM_ELEM:
                    {
                        XMLPersTextContentTContext *pTC =
                            new XMLPersTextContentTContext(
                                    GetTransformer(), rQName,
                                    (*aIter).second.GetQNamePrefixFromParam1(),
                                    (*aIter).second.GetQNameTokenFromParam1() );
                        XMLPersTextContentTContextVector::value_type aVal(pTC);
                        m_aChildContexts.push_back( aVal );
                        pContext = pTC;
                    }
                    break;
                default:
                    OSL_ENSURE( !this, "unknown action" );
                    break;
                }
            }
        }
    }

    // default is copying
    if( !pContext )
    {
        if( !m_bStartElementExported )
            ExportStartElement();
        pContext = XMLTransformerContext::CreateChildContext( nPrefix,
                                                              rLocalName,
                                                              rQName,
                                                              rAttrList );
    }

    return pContext;
}

void XMLMergeElemTransformerContext::EndElement()
{
    if( !m_bStartElementExported )
        ExportStartElement();
    XMLTransformerContext::EndElement();
}

void XMLMergeElemTransformerContext::Characters( const OUString& )
{
    // ignore
}
